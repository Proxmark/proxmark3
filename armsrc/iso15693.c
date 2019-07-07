//-----------------------------------------------------------------------------
// Jonathan Westhues, split Nov 2006
// Modified by Greg Jones, Jan 2009
// Modified by Adrian Dabrowski "atrox", Mar-Sept 2010,Oct 2011
// Modified by piwi, Oct 2018 
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support ISO 15693. This includes both the reader software and
// the `fake tag' modes.
//-----------------------------------------------------------------------------

// The ISO 15693 describes two transmission modes from reader to tag, and four
// transmission modes from tag to reader. As of Oct 2018 this code supports
// both reader modes and the high speed variant with one subcarrier from card to reader.
// As long as the card fully support ISO 15693 this is no problem, since the
// reader chooses both data rates, but some non-standard tags do not. 
// For card simulation, the code supports both high and low speed modes with one subcarrier.
//
// VCD (reader) -> VICC (tag)
// 1 out of 256:
// 	data rate: 1,66 kbit/s (fc/8192)
// 	used for long range
// 1 out of 4:
// 	data rate: 26,48 kbit/s (fc/512)
//	used for short range, high speed
//
// VICC (tag) -> VCD (reader)
// Modulation:
//		ASK / one subcarrier (423,75 khz)
//		FSK / two subcarriers (423,75 khz && 484,28 khz)
// Data Rates / Modes:
// 	low ASK: 6,62 kbit/s
// 	low FSK: 6.67 kbit/s
// 	high ASK: 26,48 kbit/s
// 	high FSK: 26,69 kbit/s
//-----------------------------------------------------------------------------


// Random Remarks:
// *) UID is always used "transmission order" (LSB), which is reverse to display order

// TODO / BUGS / ISSUES:
// *) signal decoding is unable to detect collisions.
// *) add anti-collision support for inventory-commands
// *) read security status of a block
// *) sniffing and simulation do not support two subcarrier modes.
// *) remove or refactor code under "deprecated"
// *) document all the functions

#include "iso15693.h"

#include "proxmark3.h"
#include "util.h"
#include "apps.h"
#include "string.h"
#include "iso15693tools.h"
#include "protocols.h"
#include "cmd.h"
#include "BigBuf.h"
#include "fpgaloader.h"

#define arraylen(x) (sizeof(x)/sizeof((x)[0]))

static int DEBUG = 0;

///////////////////////////////////////////////////////////////////////
// ISO 15693 Part 2 - Air Interface
// This section basicly contains transmission and receiving of bits
///////////////////////////////////////////////////////////////////////

#define Crc(data,datalen)     Iso15693Crc(data,datalen)
#define AddCrc(data,datalen)  Iso15693AddCrc(data,datalen)
#define sprintUID(target,uid)	Iso15693sprintUID(target,uid)

// buffers
#define ISO15693_DMA_BUFFER_SIZE        2048 // must be a power of 2
#define ISO15693_MAX_RESPONSE_LENGTH     36 // allows read single block with the maximum block size of 256bits. Read multiple blocks not supported yet
#define ISO15693_MAX_COMMAND_LENGTH      45 // allows write single block with the maximum block size of 256bits. Write multiple blocks not supported yet

// timing. Delays in SSP_CLK ticks. 
// SSP_CLK runs at 13,56MHz / 32 = 423.75kHz when simulating a tag
#define DELAY_READER_TO_ARM_SIM           8
#define DELAY_ARM_TO_READER_SIM           1
#define DELAY_ISO15693_VCD_TO_VICC_SIM    132  // 132/423.75kHz = 311.5us from end of command EOF to start of tag response
//SSP_CLK runs at 13.56MHz / 4 = 3,39MHz when acting as reader
#define DELAY_ISO15693_VCD_TO_VICC_READER 1056 // 1056/3,39MHz = 311.5us from end of command EOF to start of tag response
#define DELAY_ISO15693_VICC_TO_VCD_READER 1017 // 1017/3.39MHz = 300us between end of tag response and next reader command

// ---------------------------
// Signal Processing
// ---------------------------

// prepare data using "1 out of 4" code for later transmission
// resulting data rate is 26.48 kbit/s (fc/512)
// cmd ... data
// n ... length of data
static void CodeIso15693AsReader(uint8_t *cmd, int n)
{
	int i, j;

	ToSendReset();

	// Give it a bit of slack at the beginning
	for(i = 0; i < 24; i++) {
		ToSendStuffBit(1);
	}

	// SOF for 1of4
	ToSendStuffBit(0);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(0);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	for(i = 0; i < n; i++) {
		for(j = 0; j < 8; j += 2) {
			int these = (cmd[i] >> j) & 3;
			switch(these) {
				case 0:
					ToSendStuffBit(1);
					ToSendStuffBit(0);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					break;
				case 1:
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(0);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					break;
				case 2:
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(0);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					break;
				case 3:
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(1);
					ToSendStuffBit(0);
					break;
			}
		}
	}
	// EOF
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(0);
	ToSendStuffBit(1);

	// Fill remainder of last byte with 1
	for(i = 0; i < 4; i++) {
		ToSendStuffBit(1);
	}
	
	ToSendMax++;
}

// encode data using "1 out of 256" scheme
// data rate is 1,66 kbit/s (fc/8192)
// is designed for more robust communication over longer distances
static void CodeIso15693AsReader256(uint8_t *cmd, int n)
{
	int i, j;

	ToSendReset();

	// Give it a bit of slack at the beginning
	for(i = 0; i < 24; i++) {
		ToSendStuffBit(1);
	}

	// SOF for 1of256
	ToSendStuffBit(0);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(0);

	for(i = 0; i < n; i++) {
		for (j = 0; j<=255; j++) {
			if (cmd[i]==j) {
				ToSendStuffBit(1);
				ToSendStuffBit(0);
			} else {
				ToSendStuffBit(1);
				ToSendStuffBit(1);
			}
		}
	}
	// EOF
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(0);
	ToSendStuffBit(1);

	// Fill remainder of last byte with 1
	for(i = 0; i < 4; i++) {
		ToSendStuffBit(1);
	}

	ToSendMax++;
}


static void CodeIso15693AsTag(uint8_t *cmd, int n)
{
	ToSendReset();

	// SOF
	ToSendStuffBit(0);
	ToSendStuffBit(0);
	ToSendStuffBit(0);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(0);
	ToSendStuffBit(1);

	// data
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < 8; j++) {
			if ((cmd[i] >> j) & 0x01) {
					ToSendStuffBit(0);
					ToSendStuffBit(1);
			} else {
					ToSendStuffBit(1);
					ToSendStuffBit(0);
			}
		}
	}

	// EOF
	ToSendStuffBit(1);
	ToSendStuffBit(0);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(1);
	ToSendStuffBit(0);
	ToSendStuffBit(0);
	ToSendStuffBit(0);

	ToSendMax++;
}


// Transmit the command (to the tag) that was placed in cmd[].
static void TransmitTo15693Tag(const uint8_t *cmd, int len, uint32_t start_time)
{
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SEND_FULL_MOD);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);

	while (GetCountSspClk() < start_time) ;

	LED_B_ON();
	for(int c = 0; c < len; c++) {
		uint8_t data = cmd[c];
		for (int i = 0; i < 8; i++) {
			uint16_t send_word = (data & 0x80) ? 0x0000 : 0xffff;
			while (!(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_TXRDY))) ;
			AT91C_BASE_SSC->SSC_THR = send_word;
			while (!(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_TXRDY))) ;
			AT91C_BASE_SSC->SSC_THR = send_word;
			data <<= 1;
		}
		WDT_HIT();
	}
	LED_B_OFF();
}


//-----------------------------------------------------------------------------
// Transmit the tag response (to the reader) that was placed in cmd[].
//-----------------------------------------------------------------------------
static void TransmitTo15693Reader(const uint8_t *cmd, size_t len, uint32_t start_time, bool slow)
{
	// don't use the FPGA_HF_SIMULATOR_MODULATE_424K_8BIT minor mode. It would spoil GetCountSspClk()
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_MODULATE_424K);

	uint8_t shift_delay = start_time & 0x00000007;
	uint8_t bitmask = 0x00;
	for (int i = 0; i < shift_delay; i++) {
		bitmask |= (0x01 << i);
	}

	while (GetCountSspClk() < (start_time & 0xfffffff8)) ;

	AT91C_BASE_SSC->SSC_THR = 0x00; // clear TXRDY

	LED_C_ON();
	uint8_t bits_to_shift = 0x00;
    for(size_t c = 0; c <= len; c++) {
		uint8_t bits_to_send = bits_to_shift << (8 - shift_delay) | (c==len?0x00:cmd[c]) >> shift_delay;
		bits_to_shift = cmd[c] & bitmask;
		for (int i = 7; i >= 0; i--) {
			for (int j = 0; j < (slow?4:1); ) {
				if (AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXRDY) {
					if (bits_to_send >> i & 0x01) {
						AT91C_BASE_SSC->SSC_THR = 0xff;
					} else {
						AT91C_BASE_SSC->SSC_THR = 0x00;
					}
					j++;
				}
				WDT_HIT();
			}
        }
    }
	LED_C_OFF();
}


//=============================================================================
// An ISO 15693 decoder for tag responses (one subcarrier only).
// Uses cross correlation to identify each bit and EOF.
// This function is called 8 times per bit (every 2 subcarrier cycles).
// Subcarrier frequency fs is 424kHz, 1/fs = 2,36us,
// i.e. function is called every 4,72us
// LED handling:
//    LED C -> ON once we have received the SOF and are expecting the rest.
//    LED C -> OFF once we have received EOF or are unsynced
//
// Returns: true if we received a EOF
//          false if we are still waiting for some more
//=============================================================================

#define NOISE_THRESHOLD    160      // don't try to correlate noise

typedef struct DecodeTag {
	enum {
		STATE_TAG_SOF_LOW,
		STATE_TAG_SOF_HIGH,
		STATE_TAG_SOF_HIGH_END,
		STATE_TAG_RECEIVING_DATA,
		STATE_TAG_EOF
	}         state;
	int       bitCount;
	int       posCount;
	enum {
		LOGIC0,
		LOGIC1,
		SOF_PART1,
		SOF_PART2
	}         lastBit;
	uint16_t  shiftReg;
	uint16_t  max_len;
	uint8_t   *output;
	int       len;
	int       sum1, sum2;
} DecodeTag_t;


static int inline __attribute__((always_inline)) Handle15693SamplesFromTag(uint16_t amplitude, DecodeTag_t *DecodeTag)
{
	switch(DecodeTag->state) {
		case STATE_TAG_SOF_LOW: 
			// waiting for 12 times low (11 times low is accepted as well)
			if (amplitude < NOISE_THRESHOLD) {
				DecodeTag->posCount++;
			} else {
				if (DecodeTag->posCount > 10) {
					DecodeTag->posCount = 1;
					DecodeTag->sum1 = 0;
					DecodeTag->state = STATE_TAG_SOF_HIGH;
				} else {
					DecodeTag->posCount = 0;
				}
			}
			break;
			
		case STATE_TAG_SOF_HIGH:
			// waiting for 10 times high. Take average over the last 8
			if (amplitude > NOISE_THRESHOLD) {
				DecodeTag->posCount++;
				if (DecodeTag->posCount > 2) {
					DecodeTag->sum1 += amplitude; // keep track of average high value
				}
				if (DecodeTag->posCount == 10) {
					DecodeTag->sum1 >>= 4;        // calculate half of average high value (8 samples)
					DecodeTag->state = STATE_TAG_SOF_HIGH_END;
				}
			} else { // high phase was too short
				DecodeTag->posCount = 1;
				DecodeTag->state = STATE_TAG_SOF_LOW;
			}
			break;

		case STATE_TAG_SOF_HIGH_END:
			// waiting for a falling edge
			if (amplitude < DecodeTag->sum1) {   // signal drops below 50% average high: a falling edge
				DecodeTag->lastBit = SOF_PART1;  // detected 1st part of SOF (12 samples low and 12 samples high)
				DecodeTag->shiftReg = 0;
				DecodeTag->bitCount = 0;
				DecodeTag->len = 0;
				DecodeTag->sum1 = amplitude;
				DecodeTag->sum2 = 0;
				DecodeTag->posCount = 2;
				DecodeTag->state = STATE_TAG_RECEIVING_DATA;
				LED_C_ON();
			} else {
				DecodeTag->posCount++;
				if (DecodeTag->posCount > 13) { // high phase too long
					DecodeTag->posCount = 0;
					DecodeTag->state = STATE_TAG_SOF_LOW;
					LED_C_OFF();
				}
			}
			break;

		case STATE_TAG_RECEIVING_DATA:
			if (DecodeTag->posCount == 1) {
				DecodeTag->sum1 = 0;
				DecodeTag->sum2 = 0;
			}
			if (DecodeTag->posCount <= 4) {
				DecodeTag->sum1 += amplitude;
			} else {
				DecodeTag->sum2 += amplitude;
			}
			if (DecodeTag->posCount == 8) {
				int32_t corr_1 = DecodeTag->sum2 - DecodeTag->sum1;
				int32_t corr_0 = -corr_1;
				int32_t corr_EOF = (DecodeTag->sum1 + DecodeTag->sum2) / 2;
				if (corr_EOF > corr_0 && corr_EOF > corr_1) {
					if (DecodeTag->lastBit == LOGIC0) {  // this was already part of EOF
						DecodeTag->state = STATE_TAG_EOF;
					} else {
						DecodeTag->posCount = 0;
						DecodeTag->state = STATE_TAG_SOF_LOW;
						LED_C_OFF();
					}
				} else if (corr_1 > corr_0) {
					// logic 1
					if (DecodeTag->lastBit == SOF_PART1) { // still part of SOF
						DecodeTag->lastBit = SOF_PART2;    // SOF completed
					} else {
						DecodeTag->lastBit = LOGIC1;
						DecodeTag->shiftReg >>= 1;
						DecodeTag->shiftReg |= 0x80;
						DecodeTag->bitCount++;
						if (DecodeTag->bitCount == 8) {
							DecodeTag->output[DecodeTag->len] = DecodeTag->shiftReg;
							DecodeTag->len++;
							if (DecodeTag->len > DecodeTag->max_len) {
								// buffer overflow, give up
								DecodeTag->posCount = 0;
								DecodeTag->state = STATE_TAG_SOF_LOW;
								LED_C_OFF();
							}
							DecodeTag->bitCount = 0;
							DecodeTag->shiftReg = 0;
						}
					}
				} else {
					// logic 0
					if (DecodeTag->lastBit == SOF_PART1) { // incomplete SOF
						DecodeTag->posCount = 0;
						DecodeTag->state = STATE_TAG_SOF_LOW;
						LED_C_OFF();
					} else {
						DecodeTag->lastBit = LOGIC0;
						DecodeTag->shiftReg >>= 1;
						DecodeTag->bitCount++;
						if (DecodeTag->bitCount == 8) {
							DecodeTag->output[DecodeTag->len] = DecodeTag->shiftReg;
							DecodeTag->len++;
							if (DecodeTag->len > DecodeTag->max_len) {
								// buffer overflow, give up
								DecodeTag->posCount = 0;
								DecodeTag->state = STATE_TAG_SOF_LOW;
								LED_C_OFF();
							}
							DecodeTag->bitCount = 0;
							DecodeTag->shiftReg = 0;
						}
					}
				}
				DecodeTag->posCount = 0;
			}
			DecodeTag->posCount++;
			break;

		case STATE_TAG_EOF:
			if (DecodeTag->posCount == 1) {
				DecodeTag->sum1 = 0;
				DecodeTag->sum2 = 0;
			}
			if (DecodeTag->posCount <= 4) {
				DecodeTag->sum1 += amplitude;
			} else {
				DecodeTag->sum2 += amplitude;
			}
			if (DecodeTag->posCount == 8) {
				int32_t corr_1 = DecodeTag->sum2 - DecodeTag->sum1;
				int32_t corr_0 = -corr_1;
				int32_t corr_EOF = (DecodeTag->sum1 + DecodeTag->sum2) / 2;
				if (corr_EOF > corr_0 || corr_1 > corr_0) {
					DecodeTag->posCount = 0;
					DecodeTag->state = STATE_TAG_SOF_LOW;
					LED_C_OFF();
				} else {
					LED_C_OFF();
					return true;
				}
			}
			DecodeTag->posCount++;
			break;

	}

	return false;
}


static void DecodeTagInit(DecodeTag_t *DecodeTag, uint8_t *data, uint16_t max_len)
{
	DecodeTag->posCount = 0;
	DecodeTag->state = STATE_TAG_SOF_LOW;
	DecodeTag->output = data;
	DecodeTag->max_len = max_len;
}


static void DecodeTagReset(DecodeTag_t *DecodeTag)
{
	DecodeTag->posCount = 0;
	DecodeTag->state = STATE_TAG_SOF_LOW;
}


/*
 *  Receive and decode the tag response, also log to tracebuffer
 */
static int GetIso15693AnswerFromTag(uint8_t* response, uint16_t max_len, int timeout)
{
	int samples = 0;
	bool gotFrame = false;

	uint16_t *dmaBuf = (uint16_t*)BigBuf_malloc(ISO15693_DMA_BUFFER_SIZE*sizeof(uint16_t));
	
	// the Decoder data structure
	DecodeTag_t DecodeTag = { 0 };
	DecodeTagInit(&DecodeTag, response, max_len);

	// wait for last transfer to complete
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXEMPTY));

	// And put the FPGA in the appropriate mode
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_SUBCARRIER_424_KHZ | FPGA_HF_READER_MODE_RECEIVE_AMPLITUDE);

	// Setup and start DMA.
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);
	uint16_t *upTo = dmaBuf;

	for(;;) {
		uint16_t behindBy = ((uint16_t*)AT91C_BASE_PDC_SSC->PDC_RPR - upTo) & (ISO15693_DMA_BUFFER_SIZE-1);

		if (behindBy == 0) continue;

		uint16_t tagdata = *upTo++;

		if(upTo >= dmaBuf + ISO15693_DMA_BUFFER_SIZE) {                // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                             // start reading the circular buffer from the beginning
			if(behindBy > (9*ISO15693_DMA_BUFFER_SIZE/10)) {
				Dbprintf("About to blow circular buffer - aborted! behindBy=%d", behindBy);
				break;
			}
		}
		if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
			AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
			AT91C_BASE_PDC_SSC->PDC_RNCR = ISO15693_DMA_BUFFER_SIZE;   // DMA Next Counter registers
		}

		samples++;

		if (Handle15693SamplesFromTag(tagdata, &DecodeTag)) {
			gotFrame = true;
			break;
		}

		if (samples > timeout && DecodeTag.state < STATE_TAG_RECEIVING_DATA) {
			DecodeTag.len = 0;
			break;
		}

	}

	FpgaDisableSscDma();
	BigBuf_free();
	
	if (DEBUG) Dbprintf("samples = %d, gotFrame = %d, Decoder: state = %d, len = %d, bitCount = %d, posCount = %d",
	                    samples, gotFrame, DecodeTag.state, DecodeTag.len, DecodeTag.bitCount, DecodeTag.posCount);

	if (DecodeTag.len > 0) {
		LogTrace(DecodeTag.output, DecodeTag.len, 0, 0, NULL, false);
	}

	return DecodeTag.len;
}


//=============================================================================
// An ISO15693 decoder for reader commands.
//
// This function is called 4 times per bit (every 2 subcarrier cycles).
// Subcarrier frequency fs is 848kHz, 1/fs = 1,18us, i.e. function is called every 2,36us
// LED handling:
//    LED B -> ON once we have received the SOF and are expecting the rest.
//    LED B -> OFF once we have received EOF or are in error state or unsynced
//
// Returns: true  if we received a EOF
//          false if we are still waiting for some more
//=============================================================================

typedef struct DecodeReader {
	enum {
		STATE_READER_UNSYNCD,
		STATE_READER_AWAIT_1ST_RISING_EDGE_OF_SOF,
		STATE_READER_AWAIT_2ND_FALLING_EDGE_OF_SOF,
		STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF,
		STATE_READER_AWAIT_END_OF_SOF_1_OUT_OF_4,
		STATE_READER_RECEIVE_DATA_1_OUT_OF_4,
		STATE_READER_RECEIVE_DATA_1_OUT_OF_256
	}           state;
	enum {
		CODING_1_OUT_OF_4,
		CODING_1_OUT_OF_256
	}           Coding;
	uint8_t     shiftReg;
	uint8_t     bitCount;
	int         byteCount;
	int         byteCountMax;
	int         posCount;
	int			sum1, sum2;
	uint8_t     *output;
} DecodeReader_t;


static void DecodeReaderInit(DecodeReader_t* DecodeReader, uint8_t *data, uint16_t max_len)
{
	DecodeReader->output = data;
	DecodeReader->byteCountMax = max_len;
	DecodeReader->state = STATE_READER_UNSYNCD;
	DecodeReader->byteCount = 0;
	DecodeReader->bitCount = 0;
	DecodeReader->posCount = 1;
	DecodeReader->shiftReg = 0;
}


static void DecodeReaderReset(DecodeReader_t* DecodeReader)
{
	DecodeReader->state = STATE_READER_UNSYNCD;
}


static int inline __attribute__((always_inline)) Handle15693SampleFromReader(uint8_t bit, DecodeReader_t *restrict DecodeReader)
{
	switch(DecodeReader->state) {
		case STATE_READER_UNSYNCD:
			if(!bit) {
				// we went low, so this could be the beginning of a SOF
				DecodeReader->posCount = 1;
				DecodeReader->state = STATE_READER_AWAIT_1ST_RISING_EDGE_OF_SOF;
			}
			break;

		case STATE_READER_AWAIT_1ST_RISING_EDGE_OF_SOF:
			DecodeReader->posCount++;
			if(bit) { // detected rising edge
				if(DecodeReader->posCount < 4) { // rising edge too early (nominally expected at 5)
					DecodeReaderReset(DecodeReader);
				} else { // SOF
					DecodeReader->state = STATE_READER_AWAIT_2ND_FALLING_EDGE_OF_SOF;
				}
			} else {
				if(DecodeReader->posCount > 5) { // stayed low for too long
					DecodeReaderReset(DecodeReader);
				} else {
					// do nothing, keep waiting
				}
			}
			break;

		case STATE_READER_AWAIT_2ND_FALLING_EDGE_OF_SOF:
			DecodeReader->posCount++;
			if(!bit) { // detected a falling edge
				if (DecodeReader->posCount < 20) {         // falling edge too early (nominally expected at 21 earliest)
					DecodeReaderReset(DecodeReader);
				} else if (DecodeReader->posCount < 23) {  // SOF for 1 out of 4 coding
					DecodeReader->Coding = CODING_1_OUT_OF_4;
					DecodeReader->state = STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF;
				} else if (DecodeReader->posCount < 28) {  // falling edge too early (nominally expected at 29 latest)
					DecodeReaderReset(DecodeReader);
				} else {                                 // SOF for 1 out of 4 coding
					DecodeReader->Coding = CODING_1_OUT_OF_256;
					DecodeReader->state = STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF;
				}
			} else {
				if(DecodeReader->posCount > 29) { // stayed high for too long
					DecodeReaderReset(DecodeReader);
				} else {
					// do nothing, keep waiting
				}
			}
			break;

		case STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF:
			DecodeReader->posCount++;
			if (bit) { // detected rising edge
				if (DecodeReader->Coding == CODING_1_OUT_OF_256) {
					if (DecodeReader->posCount < 32) { // rising edge too early (nominally expected at 33)
					DecodeReaderReset(DecodeReader);
					} else {
						DecodeReader->posCount = 1;
						DecodeReader->bitCount = 0;
						DecodeReader->byteCount = 0;
						DecodeReader->sum1 = 1;
						DecodeReader->state = STATE_READER_RECEIVE_DATA_1_OUT_OF_256;
						LED_B_ON();
					}
				} else { // CODING_1_OUT_OF_4
					if (DecodeReader->posCount < 24) { // rising edge too early (nominally expected at 25)
					DecodeReaderReset(DecodeReader);
					} else {
						DecodeReader->state = STATE_READER_AWAIT_END_OF_SOF_1_OUT_OF_4;
					}
				}
			} else {
				if (DecodeReader->Coding == CODING_1_OUT_OF_256) {
					if (DecodeReader->posCount > 34) { // signal stayed low for too long
					DecodeReaderReset(DecodeReader);
					} else {
						// do nothing, keep waiting
					}
				} else { // CODING_1_OUT_OF_4
					if (DecodeReader->posCount > 26) { // signal stayed low for too long
					DecodeReaderReset(DecodeReader);
					} else {
						// do nothing, keep waiting
					}
				}
			}
			break;

		case STATE_READER_AWAIT_END_OF_SOF_1_OUT_OF_4:
			DecodeReader->posCount++;
			if (bit) {
				if (DecodeReader->posCount == 33) {
					DecodeReader->posCount = 1;
					DecodeReader->bitCount = 0;
					DecodeReader->byteCount = 0;
					DecodeReader->sum1 = 1;
					DecodeReader->state = STATE_READER_RECEIVE_DATA_1_OUT_OF_4;
					LED_B_ON();
				} else {
					// do nothing, keep waiting
				}
			} else { // unexpected falling edge
					DecodeReaderReset(DecodeReader);
			}
			break;

		case STATE_READER_RECEIVE_DATA_1_OUT_OF_4:
			DecodeReader->posCount++;
			if (DecodeReader->posCount == 1) {
				DecodeReader->sum1 = bit;
			} else if (DecodeReader->posCount <= 4) {
				DecodeReader->sum1 += bit;
			} else if (DecodeReader->posCount == 5) {
				DecodeReader->sum2 = bit;
			} else {
				DecodeReader->sum2 += bit;
			}
			if (DecodeReader->posCount == 8) {
				DecodeReader->posCount = 0;
				int corr10 = DecodeReader->sum1 - DecodeReader->sum2;
				int corr01 = DecodeReader->sum2 - DecodeReader->sum1;
				int corr11 = (DecodeReader->sum1 + DecodeReader->sum2) / 2;
				if (corr01 > corr11 && corr01 > corr10) { // EOF
					LED_B_OFF(); // Finished receiving
					DecodeReaderReset(DecodeReader);
					if (DecodeReader->byteCount != 0) {
						return true;
					}
				}
				if (corr10 > corr11) { // detected a 2bit position
					DecodeReader->shiftReg >>= 2;
					DecodeReader->shiftReg |= (DecodeReader->bitCount << 6);
				}
				if (DecodeReader->bitCount == 15) { // we have a full byte
					DecodeReader->output[DecodeReader->byteCount++] = DecodeReader->shiftReg;
					if (DecodeReader->byteCount > DecodeReader->byteCountMax) {
						// buffer overflow, give up
						LED_B_OFF();
						DecodeReaderReset(DecodeReader);
					}
					DecodeReader->bitCount = 0;
					DecodeReader->shiftReg = 0;
				} else {
					DecodeReader->bitCount++;
				}
			}
			break;

		case STATE_READER_RECEIVE_DATA_1_OUT_OF_256:
			DecodeReader->posCount++;
			if (DecodeReader->posCount == 1) {
				DecodeReader->sum1 = bit;
			} else if (DecodeReader->posCount <= 4) {
				DecodeReader->sum1 += bit;
			} else if (DecodeReader->posCount == 5) {
				DecodeReader->sum2 = bit;
			} else {
				DecodeReader->sum2 += bit;
			}
			if (DecodeReader->posCount == 8) {
				DecodeReader->posCount = 0;
				int corr10 = DecodeReader->sum1 - DecodeReader->sum2;
				int corr01 = DecodeReader->sum2 - DecodeReader->sum1;
				int corr11 = (DecodeReader->sum1 + DecodeReader->sum2) / 2;
				if (corr01 > corr11 && corr01 > corr10) { // EOF
					LED_B_OFF(); // Finished receiving
					DecodeReaderReset(DecodeReader);
					if (DecodeReader->byteCount != 0) {
						return true;
					}
				}
				if (corr10 > corr11) { // detected the bit position
					DecodeReader->shiftReg = DecodeReader->bitCount;
				}
				if (DecodeReader->bitCount == 255) { // we have a full byte
					DecodeReader->output[DecodeReader->byteCount++] = DecodeReader->shiftReg;
					if (DecodeReader->byteCount > DecodeReader->byteCountMax) {
						// buffer overflow, give up
						LED_B_OFF();
						DecodeReaderReset(DecodeReader);
					}
				}
				DecodeReader->bitCount++;
			}
			break;

		default:
			LED_B_OFF();
			DecodeReaderReset(DecodeReader);
			break;
	}

	return false;
}


//-----------------------------------------------------------------------------
// Receive a command (from the reader to us, where we are the simulated tag),
// and store it in the given buffer, up to the given maximum length. Keeps
// spinning, waiting for a well-framed command, until either we get one
// (returns true) or someone presses the pushbutton on the board (false).
//
// Assume that we're called with the SSC (to the FPGA) and ADC path set
// correctly.
//-----------------------------------------------------------------------------

static int GetIso15693CommandFromReader(uint8_t *received, size_t max_len, uint32_t *eof_time)
{
	int samples = 0;
	bool gotFrame = false;
	uint8_t b;

	uint8_t *dmaBuf = BigBuf_malloc(ISO15693_DMA_BUFFER_SIZE);

	// the decoder data structure
	DecodeReader_t DecodeReader = {0};
	DecodeReaderInit(&DecodeReader, received, max_len);

	// wait for last transfer to complete
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXEMPTY));

	LED_D_OFF();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_NO_MODULATION);

	// clear receive register and wait for next transfer
	uint32_t temp = AT91C_BASE_SSC->SSC_RHR;
	(void) temp;
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_RXRDY)) ;

	uint32_t bit_time = GetCountSspClk() & 0xfffffff8;

	// Setup and start DMA.
	FpgaSetupSscDma(dmaBuf, ISO15693_DMA_BUFFER_SIZE);
	uint8_t *upTo = dmaBuf;

	for(;;) {
		uint16_t behindBy = ((uint8_t*)AT91C_BASE_PDC_SSC->PDC_RPR - upTo) & (ISO15693_DMA_BUFFER_SIZE-1);

		if (behindBy == 0) continue;

		b = *upTo++;
		if(upTo >= dmaBuf + ISO15693_DMA_BUFFER_SIZE) {                // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                             // start reading the circular buffer from the beginning
			if(behindBy > (9*ISO15693_DMA_BUFFER_SIZE/10)) {
				Dbprintf("About to blow circular buffer - aborted! behindBy=%d", behindBy);
				break;
			}
		}
		if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
			AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
			AT91C_BASE_PDC_SSC->PDC_RNCR = ISO15693_DMA_BUFFER_SIZE;   // DMA Next Counter registers
		}

		for (int i = 7; i >= 0; i--) {
			if (Handle15693SampleFromReader((b >> i) & 0x01, &DecodeReader)) {
				*eof_time = bit_time + samples - DELAY_READER_TO_ARM_SIM; // end of EOF
				gotFrame = true;
				break;
			}
			samples++;
		}

		if (gotFrame) {
			break;
		}

		if (BUTTON_PRESS()) {
			DecodeReader.byteCount = 0;
			break;
		}

		WDT_HIT();
	}


	FpgaDisableSscDma();
	BigBuf_free_keep_EM();
	
	if (DEBUG) Dbprintf("samples = %d, gotFrame = %d, Decoder: state = %d, len = %d, bitCount = %d, posCount = %d",
	                    samples, gotFrame, DecodeReader.state, DecodeReader.byteCount, DecodeReader.bitCount, DecodeReader.posCount);

	if (DecodeReader.byteCount > 0) {
		LogTrace(DecodeReader.output, DecodeReader.byteCount, 0, *eof_time, NULL, true);
	}

	return DecodeReader.byteCount;
}


// Encode (into the ToSend buffers) an identify request, which is the first
// thing that you must send to a tag to get a response.
static void BuildIdentifyRequest(void)
{
	uint8_t cmd[5];

	uint16_t crc;
	// one sub-carrier, inventory, 1 slot, fast rate
	// AFI is at bit 5 (1<<4) when doing an INVENTORY
	cmd[0] = (1 << 2) | (1 << 5) | (1 << 1);
	// inventory command code
	cmd[1] = 0x01;
	// no mask
	cmd[2] = 0x00;
	//Now the CRC
	crc = Crc(cmd, 3);
	cmd[3] = crc & 0xff;
	cmd[4] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}


//-----------------------------------------------------------------------------
// Start to read an ISO 15693 tag. We send an identify request, then wait
// for the response. The response is not demodulated, just left in the buffer
// so that it can be downloaded to a PC and processed there.
//-----------------------------------------------------------------------------
void AcquireRawAdcSamplesIso15693(void)
{
	LEDsoff();
	LED_A_ON();

	uint8_t *dest = BigBuf_get_addr();

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	BuildIdentifyRequest();

	// Give the tags time to energize
	LED_D_ON();
	SpinDelay(100);

	// Now send the command
	TransmitTo15693Tag(ToSend, ToSendMax, 0);

	// wait for last transfer to complete
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXEMPTY)) ;

	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_SUBCARRIER_424_KHZ | FPGA_HF_READER_MODE_RECEIVE_AMPLITUDE);

	for(int c = 0; c < 4000; ) {
		if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_RXRDY)) {
			uint16_t r = AT91C_BASE_SSC->SSC_RHR;
			dest[c++] = r >> 5;
		}
	}

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LEDsoff();
}


void SnoopIso15693(void)
{
	LED_A_ON();
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	BigBuf_free();

	clear_trace();
	set_tracing(true);

	// The DMA buffer, used to stream samples from the FPGA
	uint16_t* dmaBuf = (uint16_t*)BigBuf_malloc(ISO15693_DMA_BUFFER_SIZE*sizeof(uint16_t));
	uint16_t *upTo;

	// Count of samples received so far, so that we can include timing
	// information in the trace buffer.
	int samples = 0;

	DecodeTag_t DecodeTag = {0};
	uint8_t response[ISO15693_MAX_RESPONSE_LENGTH];
	DecodeTagInit(&DecodeTag, response, sizeof(response));

	DecodeReader_t DecodeReader = {0};;
	uint8_t cmd[ISO15693_MAX_COMMAND_LENGTH];
	DecodeReaderInit(&DecodeReader, cmd, sizeof(cmd));

	// Print some debug information about the buffer sizes
	if (DEBUG) {
		Dbprintf("Snooping buffers initialized:");
		Dbprintf("  Trace:         %i bytes", BigBuf_max_traceLen());
		Dbprintf("  Reader -> tag: %i bytes", ISO15693_MAX_COMMAND_LENGTH);
		Dbprintf("  tag -> Reader: %i bytes", ISO15693_MAX_RESPONSE_LENGTH);
		Dbprintf("  DMA:           %i bytes", ISO15693_DMA_BUFFER_SIZE * sizeof(uint16_t));
	}
	Dbprintf("Snoop started. Press PM3 Button to stop.");
	
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SNOOP_AMPLITUDE);
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	// Setup for the DMA.
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	upTo = dmaBuf;
	FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);

	bool TagIsActive = false;
	bool ReaderIsActive = false;
	bool ExpectTagAnswer = false;

	// And now we loop, receiving samples.
	for(;;) {
		uint16_t behindBy = ((uint16_t*)AT91C_BASE_PDC_SSC->PDC_RPR - upTo) & (ISO15693_DMA_BUFFER_SIZE-1);

		if (behindBy == 0) continue;

		uint16_t snoopdata = *upTo++;

		if(upTo >= dmaBuf + ISO15693_DMA_BUFFER_SIZE) {                    // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                                 // start reading the circular buffer from the beginning
			if(behindBy > (9*ISO15693_DMA_BUFFER_SIZE/10)) {
				Dbprintf("About to blow circular buffer - aborted! behindBy=%d, samples=%d", behindBy, samples);
				break;
			}
			if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
				AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
				AT91C_BASE_PDC_SSC->PDC_RNCR = ISO15693_DMA_BUFFER_SIZE;   // DMA Next Counter registers
				WDT_HIT();
				if(BUTTON_PRESS()) {
					DbpString("Snoop stopped.");
					break;
				}
			}
		}
		samples++;
		
		if (!TagIsActive) {                                            // no need to try decoding reader data if the tag is sending
			if (Handle15693SampleFromReader(snoopdata & 0x02, &DecodeReader)) {
				FpgaDisableSscDma();
				ExpectTagAnswer = true;
				LogTrace(DecodeReader.output, DecodeReader.byteCount, samples, samples, NULL, true);
				/* And ready to receive another command. */
				DecodeReaderReset(&DecodeReader);
				/* And also reset the demod code, which might have been */
				/* false-triggered by the commands from the reader. */
				DecodeTagReset(&DecodeTag);
				upTo = dmaBuf;
				FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);
			}
			if (Handle15693SampleFromReader(snoopdata & 0x01, &DecodeReader)) {
				FpgaDisableSscDma();
				ExpectTagAnswer = true;
				LogTrace(DecodeReader.output, DecodeReader.byteCount, samples, samples, NULL, true);
				/* And ready to receive another command. */
				DecodeReaderReset(&DecodeReader);
				/* And also reset the demod code, which might have been */
				/* false-triggered by the commands from the reader. */
				DecodeTagReset(&DecodeTag);
				upTo = dmaBuf;
				FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);
			}
			ReaderIsActive = (DecodeReader.state >= STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF);
		}

		if (!ReaderIsActive && ExpectTagAnswer) {						// no need to try decoding tag data if the reader is currently sending or no answer expected yet
			if (Handle15693SamplesFromTag(snoopdata >> 2, &DecodeTag)) {
				FpgaDisableSscDma();
				//Use samples as a time measurement
				LogTrace(DecodeTag.output, DecodeTag.len, samples, samples, NULL, false);
				// And ready to receive another response.
				DecodeTagReset(&DecodeTag);
				DecodeReaderReset(&DecodeReader);
				ExpectTagAnswer = false;
				upTo = dmaBuf;
				FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);
			}
			TagIsActive = (DecodeTag.state >= STATE_TAG_RECEIVING_DATA);
		}

	}

	FpgaDisableSscDma();
	BigBuf_free();
	
	LEDsoff();

	DbpString("Snoop statistics:");
	Dbprintf("  ExpectTagAnswer: %d", ExpectTagAnswer);
	Dbprintf("  DecodeTag State: %d", DecodeTag.state);
	Dbprintf("  DecodeTag byteCnt: %d", DecodeTag.len);
	Dbprintf("  DecodeReader State: %d", DecodeReader.state);
	Dbprintf("  DecodeReader byteCnt: %d", DecodeReader.byteCount);
	Dbprintf("  Trace length: %d", BigBuf_get_traceLen());
}


// Initialize the proxmark as iso15k reader
static void Iso15693InitReader() {
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	// Setup SSC
	// FpgaSetupSsc();

	// Start from off (no field generated)
	LED_D_OFF();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	SpinDelay(10);

	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);

	// Give the tags time to energize
	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER);
	SpinDelay(250);
}

///////////////////////////////////////////////////////////////////////
// ISO 15693 Part 3 - Air Interface
// This section basically contains transmission and receiving of bits
///////////////////////////////////////////////////////////////////////


// uid is in transmission order (which is reverse of display order)
static void BuildReadBlockRequest(uint8_t *uid, uint8_t blockNumber )
{
	uint8_t cmd[13];

	uint16_t crc;
	// If we set the Option_Flag in this request, the VICC will respond with the security status of the block
	// followed by the block data
	cmd[0] = ISO15693_REQ_OPTION | ISO15693_REQ_ADDRESS | ISO15693_REQ_DATARATE_HIGH; 
	// READ BLOCK command code
	cmd[1] = ISO15693_READBLOCK;
	// UID may be optionally specified here
	// 64-bit UID
	cmd[2] = uid[0];
	cmd[3] = uid[1];
	cmd[4] = uid[2];
	cmd[5] = uid[3];
	cmd[6] = uid[4];
	cmd[7] = uid[5];
	cmd[8] = uid[6];
	cmd[9] = uid[7]; // 0xe0; // always e0 (not exactly unique)
	// Block number to read
	cmd[10] = blockNumber;
	//Now the CRC
	crc = Crc(cmd, 11); // the crc needs to be calculated over 11 bytes
	cmd[11] = crc & 0xff;
	cmd[12] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}


// Now the VICC>VCD responses when we are simulating a tag
static void BuildInventoryResponse(uint8_t *uid)
{
	uint8_t cmd[12];

	uint16_t crc;

	cmd[0] = 0; // No error, no protocol format extension
	cmd[1] = 0; // DSFID (data storage format identifier).  0x00 = not supported
	// 64-bit UID
	cmd[2] = uid[7]; //0x32;
	cmd[3] = uid[6]; //0x4b;
	cmd[4] = uid[5]; //0x03;
	cmd[5] = uid[4]; //0x01;
	cmd[6] = uid[3]; //0x00;
	cmd[7] = uid[2]; //0x10;
	cmd[8] = uid[1]; //0x05;
	cmd[9] = uid[0]; //0xe0;
	//Now the CRC
	crc = Crc(cmd, 10);
	cmd[10] = crc & 0xff;
	cmd[11] = crc >> 8;

	CodeIso15693AsTag(cmd, sizeof(cmd));
}

// Universal Method for sending to and recv bytes from a tag
// 	init ... should we initialize the reader?
// 	speed ... 0 low speed, 1 hi speed
// 	*recv will contain the tag's answer
//	return: lenght of received data
int SendDataTag(uint8_t *send, int sendlen, bool init, int speed, uint8_t *recv, uint16_t max_recv_len, uint32_t start_time) {

	LED_A_ON();
	LED_B_OFF();
	LED_C_OFF();

	if (init) Iso15693InitReader();

	int answerLen=0;

	if (!speed) {
		// low speed (1 out of 256)
		CodeIso15693AsReader256(send, sendlen);
	} else {
		// high speed (1 out of 4)
		CodeIso15693AsReader(send, sendlen);
	}

	TransmitTo15693Tag(ToSend, ToSendMax, start_time);

	// Now wait for a response
	if (recv != NULL) {
		answerLen = GetIso15693AnswerFromTag(recv, max_recv_len, DELAY_ISO15693_VCD_TO_VICC_READER * 2);
	}

	LED_A_OFF();

	return answerLen;
}


// --------------------------------------------------------------------
// Debug Functions
// --------------------------------------------------------------------

// Decodes a message from a tag and displays its metadata and content
#define DBD15STATLEN 48
void DbdecodeIso15693Answer(int len, uint8_t *d) {
	char status[DBD15STATLEN+1]={0};
	uint16_t crc;

	if (len > 3) {
		if (d[0] & ISO15693_RES_EXT)
			strncat(status,"ProtExt ", DBD15STATLEN);
		if (d[0] & ISO15693_RES_ERROR) {
			// error
			strncat(status,"Error ", DBD15STATLEN);
			switch (d[1]) {
				case 0x01:
					strncat(status,"01:notSupp", DBD15STATLEN);
					break;
				case 0x02:
					strncat(status,"02:notRecog", DBD15STATLEN);
					break;
				case 0x03:
					strncat(status,"03:optNotSupp", DBD15STATLEN);
					break;
				case 0x0f:
					strncat(status,"0f:noInfo", DBD15STATLEN);
					break;
				case 0x10:
					strncat(status,"10:doesn'tExist", DBD15STATLEN);
					break;
				case 0x11:
					strncat(status,"11:lockAgain", DBD15STATLEN);
					break;
				case 0x12:
					strncat(status,"12:locked", DBD15STATLEN);
					break;
				case 0x13:
					strncat(status,"13:progErr", DBD15STATLEN);
					break;
				case 0x14:
					strncat(status,"14:lockErr", DBD15STATLEN);
					break;
				default:
					strncat(status,"unknownErr", DBD15STATLEN);
			}
			strncat(status," ", DBD15STATLEN);
		} else {
			strncat(status,"NoErr ", DBD15STATLEN);
		}

		crc=Crc(d,len-2);
		if ( (( crc & 0xff ) == d[len-2]) && (( crc >> 8 ) == d[len-1]) )
			strncat(status,"CrcOK",DBD15STATLEN);
		else
			strncat(status,"CrcFail!",DBD15STATLEN);

		Dbprintf("%s",status);
	}
}



///////////////////////////////////////////////////////////////////////
// Functions called via USB/Client
///////////////////////////////////////////////////////////////////////

void SetDebugIso15693(uint32_t debug) {
	DEBUG=debug;
	Dbprintf("Iso15693 Debug is now %s",DEBUG?"on":"off");
	return;
}


//---------------------------------------------------------------------------------------
// Simulate an ISO15693 reader, perform anti-collision and then attempt to read a sector.
// all demodulation performed in arm rather than host. - greg
//---------------------------------------------------------------------------------------
void ReaderIso15693(uint32_t parameter)
{
	LEDsoff();
	LED_A_ON();

	set_tracing(true);
	
	int answerLen = 0;
	uint8_t TagUID[8] = {0x00};

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);

	uint8_t answer[ISO15693_MAX_RESPONSE_LENGTH];

	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	// Setup SSC
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);

	// Start from off (no field generated)
   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
   	SpinDelay(200);

	// Give the tags time to energize
	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER);
	SpinDelay(200);
	StartCountSspClk();


	// FIRST WE RUN AN INVENTORY TO GET THE TAG UID
	// THIS MEANS WE CAN PRE-BUILD REQUESTS TO SAVE CPU TIME

	// Now send the IDENTIFY command
	BuildIdentifyRequest();
	TransmitTo15693Tag(ToSend, ToSendMax, 0);
	
	// Now wait for a response
	answerLen = GetIso15693AnswerFromTag(answer, sizeof(answer), DELAY_ISO15693_VCD_TO_VICC_READER * 2) ;
	uint32_t start_time = GetCountSspClk() + DELAY_ISO15693_VICC_TO_VCD_READER;

	if (answerLen >=12) // we should do a better check than this
	{
		TagUID[0] = answer[2];
		TagUID[1] = answer[3];
		TagUID[2] = answer[4];
		TagUID[3] = answer[5];
		TagUID[4] = answer[6];
		TagUID[5] = answer[7];
		TagUID[6] = answer[8]; // IC Manufacturer code
		TagUID[7] = answer[9]; // always E0

	}

	Dbprintf("%d octets read from IDENTIFY request:", answerLen);
	DbdecodeIso15693Answer(answerLen, answer);
	Dbhexdump(answerLen, answer, false);

	// UID is reverse
	if (answerLen >= 12)
		Dbprintf("UID = %02hX%02hX%02hX%02hX%02hX%02hX%02hX%02hX",
			TagUID[7],TagUID[6],TagUID[5],TagUID[4],
			TagUID[3],TagUID[2],TagUID[1],TagUID[0]);


	// Dbprintf("%d octets read from SELECT request:", answerLen2);
	// DbdecodeIso15693Answer(answerLen2,answer2);
	// Dbhexdump(answerLen2,answer2,true);

	// Dbprintf("%d octets read from XXX request:", answerLen3);
	// DbdecodeIso15693Answer(answerLen3,answer3);
	// Dbhexdump(answerLen3,answer3,true);

	// read all pages
	if (answerLen >= 12 && DEBUG) {
		for (int i = 0; i < 32; i++) {  // sanity check, assume max 32 pages
			BuildReadBlockRequest(TagUID, i);
			TransmitTo15693Tag(ToSend, ToSendMax, start_time);
			int answerLen = GetIso15693AnswerFromTag(answer, sizeof(answer), DELAY_ISO15693_VCD_TO_VICC_READER * 2);
			start_time = GetCountSspClk() + DELAY_ISO15693_VICC_TO_VCD_READER;
			if (answerLen > 0) {
				Dbprintf("READ SINGLE BLOCK %d returned %d octets:", i, answerLen);
				DbdecodeIso15693Answer(answerLen, answer);
				Dbhexdump(answerLen, answer, false);
				if ( *((uint32_t*) answer) == 0x07160101 ) break; // exit on NoPageErr
			}
		}
	}

	// for the time being, switch field off to protect rdv4.0
	// note: this prevents using hf 15 cmd with s option - which isn't implemented yet anyway
   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();

	LED_A_OFF();
}


// Simulate an ISO15693 TAG.
// For Inventory command: print command and send Inventory Response with given UID
// TODO: interpret other reader commands and send appropriate response
void SimTagIso15693(uint32_t parameter, uint8_t *uid)
{
	LEDsoff();
	LED_A_ON();

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
   	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_NO_MODULATION);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_SIMULATOR);

	StartCountSspClk();

	uint8_t cmd[ISO15693_MAX_COMMAND_LENGTH];

	// Build a suitable response to the reader INVENTORY command
	BuildInventoryResponse(uid);

	// Listen to reader
	while (!BUTTON_PRESS()) {
		uint32_t eof_time = 0, start_time = 0;
		int cmd_len = GetIso15693CommandFromReader(cmd, sizeof(cmd), &eof_time);

		if ((cmd_len >= 5) && (cmd[0] & ISO15693_REQ_INVENTORY) && (cmd[1] == ISO15693_INVENTORY)) { // TODO: check more flags
			bool slow = !(cmd[0] & ISO15693_REQ_DATARATE_HIGH);
			start_time = eof_time + DELAY_ISO15693_VCD_TO_VICC_SIM - DELAY_ARM_TO_READER_SIM;
			TransmitTo15693Reader(ToSend, ToSendMax, start_time, slow);
		}

		Dbprintf("%d bytes read from reader:", cmd_len);
		Dbhexdump(cmd_len, cmd, false);
	}

   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LEDsoff();
}


// Since there is no standardized way of reading the AFI out of a tag, we will brute force it
// (some manufactures offer a way to read the AFI, though)
void BruteforceIso15693Afi(uint32_t speed)
{
	LEDsoff();
	LED_A_ON();

	uint8_t data[6];
	uint8_t recv[ISO15693_MAX_RESPONSE_LENGTH];
	
	int datalen=0, recvlen=0;

	Iso15693InitReader();
	StartCountSspClk();
	
	// first without AFI
	// Tags should respond without AFI and with AFI=0 even when AFI is active

	data[0] = ISO15693_REQ_DATARATE_HIGH | ISO15693_REQ_INVENTORY | ISO15693_REQINV_SLOT1;
	data[1] = ISO15693_INVENTORY;
	data[2] = 0; // mask length
	datalen = AddCrc(data,3);
	recvlen = SendDataTag(data, datalen, false, speed, recv, sizeof(recv), 0);
	uint32_t start_time = GetCountSspClk() + DELAY_ISO15693_VICC_TO_VCD_READER;
	WDT_HIT();
	if (recvlen>=12) {
		Dbprintf("NoAFI UID=%s", sprintUID(NULL, &recv[2]));
	}

	// now with AFI

	data[0] = ISO15693_REQ_DATARATE_HIGH | ISO15693_REQ_INVENTORY | ISO15693_REQINV_AFI | ISO15693_REQINV_SLOT1;
	data[1] = ISO15693_INVENTORY;
	data[2] = 0; // AFI
	data[3] = 0; // mask length

	for (int i = 0; i < 256; i++) {
		data[2] = i & 0xFF;
		datalen = AddCrc(data,4);
		recvlen = SendDataTag(data, datalen, false, speed, recv, sizeof(recv), start_time);
		start_time = GetCountSspClk() + DELAY_ISO15693_VICC_TO_VCD_READER;
		WDT_HIT();
		if (recvlen >= 12) {
			Dbprintf("AFI=%i UID=%s", i, sprintUID(NULL, &recv[2]));
		}
	}
	Dbprintf("AFI Bruteforcing done.");

   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LEDsoff();
}

// Allows to directly send commands to the tag via the client
void DirectTag15693Command(uint32_t datalen, uint32_t speed, uint32_t recv, uint8_t data[]) {

	int recvlen = 0;
	uint8_t recvbuf[ISO15693_MAX_RESPONSE_LENGTH];

	LED_A_ON();

	if (DEBUG) {
		Dbprintf("SEND:");
		Dbhexdump(datalen, data, false);
	}

	recvlen = SendDataTag(data, datalen, true, speed, (recv?recvbuf:NULL), sizeof(recvbuf), 0);

	if (recv) {
		if (DEBUG) {
			Dbprintf("RECV:");
			Dbhexdump(recvlen, recvbuf, false);
			DbdecodeIso15693Answer(recvlen, recvbuf);
		}

		cmd_send(CMD_ACK, recvlen>ISO15693_MAX_RESPONSE_LENGTH?ISO15693_MAX_RESPONSE_LENGTH:recvlen, 0, 0, recvbuf, ISO15693_MAX_RESPONSE_LENGTH);

	}

	// for the time being, switch field off to protect rdv4.0
	// note: this prevents using hf 15 cmd with s option - which isn't implemented yet anyway
   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();

	LED_A_OFF();
}

//-----------------------------------------------------------------------------
// Work with "magic Chinese" card.
//
//-----------------------------------------------------------------------------

// Set the UID to the tag (based on Iceman work).
void SetTag15693Uid(uint8_t *uid)
{
    uint8_t cmd[4][9] = {0x00};

    uint16_t crc;

    int recvlen = 0;
    uint8_t recvbuf[ISO15693_MAX_RESPONSE_LENGTH];

    LED_A_ON();

    // Command 1 : 02213E00000000
    cmd[0][0] = 0x02;
    cmd[0][1] = 0x21;
    cmd[0][2] = 0x3e;
    cmd[0][3] = 0x00;
    cmd[0][4] = 0x00;
    cmd[0][5] = 0x00;
    cmd[0][6] = 0x00;

    // Command 2 : 02213F69960000
    cmd[1][0] = 0x02;
    cmd[1][1] = 0x21;
    cmd[1][2] = 0x3f;
    cmd[1][3] = 0x69;
    cmd[1][4] = 0x96;
    cmd[1][5] = 0x00;
    cmd[1][6] = 0x00;

    // Command 3 : 022138u8u7u6u5 (where uX = uid byte X)
    cmd[2][0] = 0x02;
    cmd[2][1] = 0x21;
    cmd[2][2] = 0x38;
    cmd[2][3] = uid[7];
    cmd[2][4] = uid[6];
    cmd[2][5] = uid[5];
    cmd[2][6] = uid[4];

    // Command 4 : 022139u4u3u2u1 (where uX = uid byte X)
    cmd[3][0] = 0x02;
    cmd[3][1] = 0x21;
    cmd[3][2] = 0x39;
    cmd[3][3] = uid[3];
    cmd[3][4] = uid[2];
    cmd[3][5] = uid[1];
    cmd[3][6] = uid[0];

    for (int i=0; i<4; i++) {
        // Add the CRC
        crc = Crc(cmd[i], 7);
        cmd[i][7] = crc & 0xff;
        cmd[i][8] = crc >> 8;

        if (DEBUG) {
            Dbprintf("SEND:");
            Dbhexdump(sizeof(cmd[i]), cmd[i], false);
        }

        recvlen = SendDataTag(cmd[i], sizeof(cmd[i]), true, 1, recvbuf, sizeof(recvbuf), 0);

        if (DEBUG) {
            Dbprintf("RECV:");
            Dbhexdump(recvlen, recvbuf, false);
            DbdecodeIso15693Answer(recvlen, recvbuf);
        }

        cmd_send(CMD_ACK, recvlen>ISO15693_MAX_RESPONSE_LENGTH?ISO15693_MAX_RESPONSE_LENGTH:recvlen, 0, 0, recvbuf, ISO15693_MAX_RESPONSE_LENGTH);
    }
    
    LED_D_OFF();

    LED_A_OFF();
}



// --------------------------------------------------------------------
// -- Misc & deprecated functions
// --------------------------------------------------------------------

/*

// do not use; has a fix UID
static void __attribute__((unused)) BuildSysInfoRequest(uint8_t *uid)
{
	uint8_t cmd[12];

	uint16_t crc;
	// If we set the Option_Flag in this request, the VICC will respond with the security status of the block
	// followed by the block data
	// one sub-carrier, inventory, 1 slot, fast rate
	cmd[0] =  (1 << 5) | (1 << 1); // no SELECT bit
	// System Information command code
	cmd[1] = 0x2B;
	// UID may be optionally specified here
	// 64-bit UID
	cmd[2] = 0x32;
	cmd[3]= 0x4b;
	cmd[4] = 0x03;
	cmd[5] = 0x01;
	cmd[6] = 0x00;
	cmd[7] = 0x10;
	cmd[8] = 0x05;
	cmd[9]= 0xe0; // always e0 (not exactly unique)
	//Now the CRC
	crc = Crc(cmd, 10); // the crc needs to be calculated over 2 bytes
	cmd[10] = crc & 0xff;
	cmd[11] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}


// do not use; has a fix UID
static void __attribute__((unused)) BuildReadMultiBlockRequest(uint8_t *uid)
{
	uint8_t cmd[14];

	uint16_t crc;
	// If we set the Option_Flag in this request, the VICC will respond with the security status of the block
	// followed by the block data
	// one sub-carrier, inventory, 1 slot, fast rate
	cmd[0] =  (1 << 5) | (1 << 1); // no SELECT bit
	// READ Multi BLOCK command code
	cmd[1] = 0x23;
	// UID may be optionally specified here
	// 64-bit UID
	cmd[2] = 0x32;
	cmd[3]= 0x4b;
	cmd[4] = 0x03;
	cmd[5] = 0x01;
	cmd[6] = 0x00;
	cmd[7] = 0x10;
	cmd[8] = 0x05;
	cmd[9]= 0xe0; // always e0 (not exactly unique)
	// First Block number to read
	cmd[10] = 0x00;
	// Number of Blocks to read
	cmd[11] = 0x2f; // read quite a few
	//Now the CRC
	crc = Crc(cmd, 12); // the crc needs to be calculated over 2 bytes
	cmd[12] = crc & 0xff;
	cmd[13] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}

// do not use; has a fix UID
static void __attribute__((unused)) BuildArbitraryRequest(uint8_t *uid,uint8_t CmdCode)
{
	uint8_t cmd[14];

	uint16_t crc;
	// If we set the Option_Flag in this request, the VICC will respond with the security status of the block
	// followed by the block data
	// one sub-carrier, inventory, 1 slot, fast rate
	cmd[0] =   (1 << 5) | (1 << 1); // no SELECT bit
	// READ BLOCK command code
	cmd[1] = CmdCode;
	// UID may be optionally specified here
	// 64-bit UID
	cmd[2] = 0x32;
	cmd[3]= 0x4b;
	cmd[4] = 0x03;
	cmd[5] = 0x01;
	cmd[6] = 0x00;
	cmd[7] = 0x10;
	cmd[8] = 0x05;
	cmd[9]= 0xe0; // always e0 (not exactly unique)
	// Parameter
	cmd[10] = 0x00;
	cmd[11] = 0x0a;

//	cmd[12] = 0x00;
//	cmd[13] = 0x00;	//Now the CRC
	crc = Crc(cmd, 12); // the crc needs to be calculated over 2 bytes
	cmd[12] = crc & 0xff;
	cmd[13] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}

// do not use; has a fix UID
static void __attribute__((unused)) BuildArbitraryCustomRequest(uint8_t uid[], uint8_t CmdCode)
{
	uint8_t cmd[14];

	uint16_t crc;
	// If we set the Option_Flag in this request, the VICC will respond with the security status of the block
	// followed by the block data
	// one sub-carrier, inventory, 1 slot, fast rate
	cmd[0] =   (1 << 5) | (1 << 1); // no SELECT bit
	// READ BLOCK command code
	cmd[1] = CmdCode;
	// UID may be optionally specified here
	// 64-bit UID
	cmd[2] = 0x32;
	cmd[3]= 0x4b;
	cmd[4] = 0x03;
	cmd[5] = 0x01;
	cmd[6] = 0x00;
	cmd[7] = 0x10;
	cmd[8] = 0x05;
	cmd[9]= 0xe0; // always e0 (not exactly unique)
	// Parameter
	cmd[10] = 0x05; // for custom codes this must be manufacturer code
	cmd[11] = 0x00;

//	cmd[12] = 0x00;
//	cmd[13] = 0x00;	//Now the CRC
	crc = Crc(cmd, 12); // the crc needs to be calculated over 2 bytes
	cmd[12] = crc & 0xff;
	cmd[13] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}




*/


