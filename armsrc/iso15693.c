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
// *) remove or refactor code under "depricated"
// *) document all the functions


#include "proxmark3.h"
#include "util.h"
#include "apps.h"
#include "string.h"
#include "iso15693tools.h"
#include "protocols.h"
#include "cmd.h"

#define arraylen(x) (sizeof(x)/sizeof((x)[0]))

static int DEBUG = 0;

///////////////////////////////////////////////////////////////////////
// ISO 15693 Part 2 - Air Interface
// This section basicly contains transmission and receiving of bits
///////////////////////////////////////////////////////////////////////

#define FrameSOF              Iso15693FrameSOF
#define Logic0                Iso15693Logic0
#define Logic1                Iso15693Logic1
#define FrameEOF              Iso15693FrameEOF

#define Crc(data,datalen)     Iso15693Crc(data,datalen)
#define AddCrc(data,datalen)  Iso15693AddCrc(data,datalen)
#define sprintUID(target,uid)	Iso15693sprintUID(target,uid)

// approximate amplitude=sqrt(ci^2+cq^2) by amplitude = max(|ci|,|cq|) + 1/2*min(|ci|,|cq|)
#define AMPLITUDE(ci, cq) (MAX(ABS(ci), ABS(cq)) + MIN(ABS(ci), ABS(cq))/2)

// buffers
#define ISO15693_DMA_BUFFER_SIZE     128
#define ISO15693_MAX_RESPONSE_LENGTH  36 // allows read single block with the maximum block size of 256bits. Read multiple blocks not supported yet
#define ISO15693_MAX_COMMAND_LENGTH   45 // allows write single block with the maximum block size of 256bits. Write multiple blocks not supported yet

// timing. Delays in SSP_CLK ticks.
#define DELAY_READER_TO_ARM            8
#define DELAY_ARM_TO_READER            1
#define DELAY_ISO15693_VCD_TO_VICC   132 // 132/423.75kHz = 311.5us from end of EOF to start of tag response

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
static void TransmitTo15693Tag(const uint8_t *cmd, int len)
{
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER_TX);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_TX);

	LED_B_ON();
    for(int c = 0; c < len; ) {
        if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_TXRDY)) {
            AT91C_BASE_SSC->SSC_THR = ~cmd[c];
            c++;
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
// Uses cross correlation to identify the SOF, each bit, and EOF.
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

#define SUBCARRIER_DETECT_THRESHOLD	2
#define SOF_CORRELATOR_LEN (1<<5)

typedef struct DecodeTag {
	enum {
		STATE_TAG_UNSYNCD,
		STATE_TAG_AWAIT_SOF_1,
		STATE_TAG_AWAIT_SOF_2,
		STATE_TAG_RECEIVING_DATA,
		STATE_TAG_AWAIT_EOF
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
	uint8_t   *output;
	int       len;
	int       sum1, sum2;
	uint8_t   SOF_low;
	uint8_t   SOF_high;
	uint8_t   SOF_last;
	int32_t   SOF_corr;
	int32_t   SOF_corr_prev;
	uint8_t   SOF_correlator[SOF_CORRELATOR_LEN];
} DecodeTag_t;

static int Handle15693SamplesFromTag(int8_t ci, int8_t cq, DecodeTag_t *DecodeTag)
{
	switch(DecodeTag->state) {
		case STATE_TAG_UNSYNCD:
			// initialize SOF correlator. We are looking for 12 samples low and 12 samples high.
			DecodeTag->SOF_low = 0;
			DecodeTag->SOF_high = 12;
			DecodeTag->SOF_last = 23;
			memset(DecodeTag->SOF_correlator, 0x00, DecodeTag->SOF_last + 1);
			DecodeTag->SOF_correlator[DecodeTag->SOF_last] = AMPLITUDE(ci,cq);
			DecodeTag->SOF_corr = DecodeTag->SOF_correlator[DecodeTag->SOF_last];
			DecodeTag->SOF_corr_prev = DecodeTag->SOF_corr;
			// initialize Decoder
			DecodeTag->posCount = 0;
			DecodeTag->bitCount = 0;
			DecodeTag->len = 0;
			DecodeTag->state = STATE_TAG_AWAIT_SOF_1;
			break;

		case STATE_TAG_AWAIT_SOF_1:
			// calculate the correlation in real time. Look at differences only.
			DecodeTag->SOF_corr += DecodeTag->SOF_correlator[DecodeTag->SOF_low++];
			DecodeTag->SOF_corr -= 2*DecodeTag->SOF_correlator[DecodeTag->SOF_high++];
			DecodeTag->SOF_last++;
			DecodeTag->SOF_low &= (SOF_CORRELATOR_LEN-1);
			DecodeTag->SOF_high &= (SOF_CORRELATOR_LEN-1);
			DecodeTag->SOF_last &= (SOF_CORRELATOR_LEN-1);
			DecodeTag->SOF_correlator[DecodeTag->SOF_last] = AMPLITUDE(ci,cq);
			DecodeTag->SOF_corr += DecodeTag->SOF_correlator[DecodeTag->SOF_last];

			// if correlation increases for 10 consecutive samples, we are close to maximum correlation
			if (DecodeTag->SOF_corr > DecodeTag->SOF_corr_prev + SUBCARRIER_DETECT_THRESHOLD) {
				DecodeTag->posCount++;
			} else {
				DecodeTag->posCount = 0;
			}

			if (DecodeTag->posCount == 10) {	// correlation increased 10 times
				DecodeTag->state = STATE_TAG_AWAIT_SOF_2;
			}

			DecodeTag->SOF_corr_prev = DecodeTag->SOF_corr;

			break;

		case STATE_TAG_AWAIT_SOF_2:
			// calculate the correlation in real time. Look at differences only.
			DecodeTag->SOF_corr += DecodeTag->SOF_correlator[DecodeTag->SOF_low++];
			DecodeTag->SOF_corr -= 2*DecodeTag->SOF_correlator[DecodeTag->SOF_high++];
			DecodeTag->SOF_last++;
			DecodeTag->SOF_low &= (SOF_CORRELATOR_LEN-1);
			DecodeTag->SOF_high &= (SOF_CORRELATOR_LEN-1);
			DecodeTag->SOF_last &= (SOF_CORRELATOR_LEN-1);
			DecodeTag->SOF_correlator[DecodeTag->SOF_last] = AMPLITUDE(ci,cq);
			DecodeTag->SOF_corr += DecodeTag->SOF_correlator[DecodeTag->SOF_last];

			if (DecodeTag->SOF_corr >= DecodeTag->SOF_corr_prev) { // we are looking for the maximum correlation
				DecodeTag->SOF_corr_prev = DecodeTag->SOF_corr;
			} else {
				DecodeTag->lastBit = SOF_PART1;  // detected 1st part of SOF
				DecodeTag->sum1 = DecodeTag->SOF_correlator[DecodeTag->SOF_last];
				DecodeTag->sum2 = 0;
				DecodeTag->posCount = 2;
				DecodeTag->state = STATE_TAG_RECEIVING_DATA;
				LED_C_ON();
			}

			break;

		case STATE_TAG_RECEIVING_DATA:
			if (DecodeTag->posCount == 1) {
				DecodeTag->sum1 = 0;
				DecodeTag->sum2 = 0;
			}

			if (DecodeTag->posCount <= 4) {
				DecodeTag->sum1 += AMPLITUDE(ci, cq);
			} else {
				DecodeTag->sum2 += AMPLITUDE(ci, cq);
			}

			if (DecodeTag->posCount == 8) {
				int16_t corr_1 = (DecodeTag->sum2 - DecodeTag->sum1) / 4;
				int16_t corr_0 = (DecodeTag->sum1 - DecodeTag->sum2) / 4;
				int16_t corr_EOF = (DecodeTag->sum1 + DecodeTag->sum2) / 8;
				if (corr_EOF > corr_0 && corr_EOF > corr_1) {
					DecodeTag->state = STATE_TAG_AWAIT_EOF;
				} else if (corr_1 > corr_0) {
					// logic 1
					if (DecodeTag->lastBit == SOF_PART1) { // still part of SOF
						DecodeTag->lastBit = SOF_PART2;
					} else {
						DecodeTag->lastBit = LOGIC1;
						DecodeTag->shiftReg >>= 1;
						DecodeTag->shiftReg |= 0x80;
						DecodeTag->bitCount++;
						if (DecodeTag->bitCount == 8) {
							DecodeTag->output[DecodeTag->len] = DecodeTag->shiftReg;
							DecodeTag->len++;
							DecodeTag->bitCount = 0;
							DecodeTag->shiftReg = 0;
						}
					}
				} else {
					// logic 0
					if (DecodeTag->lastBit == SOF_PART1) { // incomplete SOF
						DecodeTag->state = STATE_TAG_UNSYNCD;
						LED_C_OFF();
					} else {
						DecodeTag->lastBit = LOGIC0;
						DecodeTag->shiftReg >>= 1;
						DecodeTag->bitCount++;
						if (DecodeTag->bitCount == 8) {
							DecodeTag->output[DecodeTag->len] = DecodeTag->shiftReg;
							DecodeTag->len++;
							DecodeTag->bitCount = 0;
							DecodeTag->shiftReg = 0;
						}
					}
				}
				DecodeTag->posCount = 0;
			}
			DecodeTag->posCount++;
			break;

		case STATE_TAG_AWAIT_EOF:
			if (DecodeTag->lastBit == LOGIC0) {  // this was already part of EOF
				LED_C_OFF();
				return true;
			} else {
				DecodeTag->state = STATE_TAG_UNSYNCD;
				LED_C_OFF();
			}
			break;

		default:
			DecodeTag->state = STATE_TAG_UNSYNCD;
			LED_C_OFF();
			break;
	}

	return false;
}


static void DecodeTagInit(DecodeTag_t *DecodeTag, uint8_t *data)
{
	DecodeTag->output = data;
	DecodeTag->state = STATE_TAG_UNSYNCD;
}

/*
 *  Receive and decode the tag response, also log to tracebuffer
 */
static int GetIso15693AnswerFromTag(uint8_t* response, int timeout)
{
	int maxBehindBy = 0;
	int lastRxCounter, samples = 0;
	int8_t ci, cq;
	bool gotFrame = false;

	uint16_t dmaBuf[ISO15693_DMA_BUFFER_SIZE];

	// the Decoder data structure
	DecodeTag_t DecodeTag;
	DecodeTagInit(&DecodeTag, response);

	// wait for last transfer to complete
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXEMPTY));

	// And put the FPGA in the appropriate mode
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);

	// Setup and start DMA.
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);
	FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);
	uint16_t *upTo = dmaBuf;
	lastRxCounter = ISO15693_DMA_BUFFER_SIZE;

	for(;;) {
		int behindBy = (lastRxCounter - AT91C_BASE_PDC_SSC->PDC_RCR) & (ISO15693_DMA_BUFFER_SIZE-1);
		if(behindBy > maxBehindBy) {
			maxBehindBy = behindBy;
		}

		if (behindBy < 1) continue;

		ci = (int8_t)(*upTo >> 8);
		cq = (int8_t)(*upTo & 0xff);

		upTo++;
		lastRxCounter--;
		if(upTo >= dmaBuf + ISO15693_DMA_BUFFER_SIZE) {                // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                             // start reading the circular buffer from the beginning
			lastRxCounter += ISO15693_DMA_BUFFER_SIZE;
		}
		if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
			AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
			AT91C_BASE_PDC_SSC->PDC_RNCR = ISO15693_DMA_BUFFER_SIZE;   // DMA Next Counter registers
		}
		samples++;

		if (Handle15693SamplesFromTag(ci, cq, &DecodeTag)) {
			gotFrame = true;
			break;
		}

		if(samples > timeout && DecodeTag.state < STATE_TAG_RECEIVING_DATA) {
			DecodeTag.len = 0;
			break;
		}

	}

	FpgaDisableSscDma();

	if (DEBUG) Dbprintf("max behindby = %d, samples = %d, gotFrame = %d, Decoder: state = %d, len = %d, bitCount = %d, posCount = %d",
	                    maxBehindBy, samples, gotFrame, DecodeTag.state, DecodeTag.len, DecodeTag.bitCount, DecodeTag.posCount);

	if (tracing && DecodeTag.len > 0) {
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


static int Handle15693SampleFromReader(uint8_t bit, DecodeReader_t* DecodeReader)
{
	switch(DecodeReader->state) {
		case STATE_READER_UNSYNCD:
			if(!bit) {
				// we went low, so this could be the beginning of a SOF
				DecodeReader->state = STATE_READER_AWAIT_1ST_RISING_EDGE_OF_SOF;
				DecodeReader->posCount = 1;
			}
			break;

		case STATE_READER_AWAIT_1ST_RISING_EDGE_OF_SOF:
			DecodeReader->posCount++;
			if(bit) { // detected rising edge
				if(DecodeReader->posCount < 4) { // rising edge too early (nominally expected at 5)
					DecodeReader->state = STATE_READER_UNSYNCD;
				} else { // SOF
					DecodeReader->state = STATE_READER_AWAIT_2ND_FALLING_EDGE_OF_SOF;
				}
			} else {
				if(DecodeReader->posCount > 5) { // stayed low for too long
					DecodeReader->state = STATE_READER_UNSYNCD;
				} else {
					// do nothing, keep waiting
				}
			}
			break;

		case STATE_READER_AWAIT_2ND_FALLING_EDGE_OF_SOF:
			DecodeReader->posCount++;
			if(!bit) { // detected a falling edge
				if (DecodeReader->posCount < 20) {         // falling edge too early (nominally expected at 21 earliest)
					DecodeReader->state = STATE_READER_UNSYNCD;
				} else if (DecodeReader->posCount < 23) {  // SOF for 1 out of 4 coding
					DecodeReader->Coding = CODING_1_OUT_OF_4;
					DecodeReader->state = STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF;
				} else if (DecodeReader->posCount < 28) {  // falling edge too early (nominally expected at 29 latest)
					DecodeReader->state = STATE_READER_UNSYNCD;
				} else {                                 // SOF for 1 out of 4 coding
					DecodeReader->Coding = CODING_1_OUT_OF_256;
					DecodeReader->state = STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF;
				}
			} else {
				if(DecodeReader->posCount > 29) { // stayed high for too long
					DecodeReader->state = STATE_READER_UNSYNCD;
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
						DecodeReader->state = STATE_READER_UNSYNCD;
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
						DecodeReader->state = STATE_READER_UNSYNCD;
					} else {
						DecodeReader->state = STATE_READER_AWAIT_END_OF_SOF_1_OUT_OF_4;
					}
				}
			} else {
				if (DecodeReader->Coding == CODING_1_OUT_OF_256) {
					if (DecodeReader->posCount > 34) { // signal stayed low for too long
						DecodeReader->state = STATE_READER_UNSYNCD;
					} else {
						// do nothing, keep waiting
					}
				} else { // CODING_1_OUT_OF_4
					if (DecodeReader->posCount > 26) { // signal stayed low for too long
						DecodeReader->state = STATE_READER_UNSYNCD;
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
				DecodeReader->state = STATE_READER_UNSYNCD;
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
					DecodeReader->state = STATE_READER_UNSYNCD;
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
						DecodeReader->state = STATE_READER_UNSYNCD;
					}
					DecodeReader->bitCount = 0;
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
					DecodeReader->state = STATE_READER_UNSYNCD;
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
						DecodeReader->state = STATE_READER_UNSYNCD;
					}
				}
				DecodeReader->bitCount++;
			}
			break;

		default:
			LED_B_OFF();
			DecodeReader->state = STATE_READER_UNSYNCD;
			break;
	}

	return false;
}


static void DecodeReaderInit(uint8_t *data, uint16_t max_len, DecodeReader_t* DecodeReader)
{
	DecodeReader->output = data;
	DecodeReader->byteCountMax = max_len;
	DecodeReader->state = STATE_READER_UNSYNCD;
	DecodeReader->byteCount = 0;
	DecodeReader->bitCount = 0;
	DecodeReader->posCount = 0;
	DecodeReader->shiftReg = 0;
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
	int maxBehindBy = 0;
	int lastRxCounter, samples = 0;
	bool gotFrame = false;
	uint8_t b;

	uint8_t dmaBuf[ISO15693_DMA_BUFFER_SIZE];

	// the decoder data structure
	DecodeReader_t DecodeReader = {0};
	DecodeReaderInit(received, max_len, &DecodeReader);

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
	lastRxCounter = ISO15693_DMA_BUFFER_SIZE;

	for(;;) {
		int behindBy = (lastRxCounter - AT91C_BASE_PDC_SSC->PDC_RCR) & (ISO15693_DMA_BUFFER_SIZE-1);
		if(behindBy > maxBehindBy) {
			maxBehindBy = behindBy;
		}

		if (behindBy < 1) continue;

		b = *upTo++;
		lastRxCounter--;
		if(upTo >= dmaBuf + ISO15693_DMA_BUFFER_SIZE) {                // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                             // start reading the circular buffer from the beginning
			lastRxCounter += ISO15693_DMA_BUFFER_SIZE;
		}
		if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
			AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
			AT91C_BASE_PDC_SSC->PDC_RNCR = ISO15693_DMA_BUFFER_SIZE;   // DMA Next Counter registers
		}

		for (int i = 7; i >= 0; i--) {
			if (Handle15693SampleFromReader((b >> i) & 0x01, &DecodeReader)) {
				*eof_time = bit_time + samples - DELAY_READER_TO_ARM; // end of EOF
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

	if (DEBUG) Dbprintf("max behindby = %d, samples = %d, gotFrame = %d, Decoder: state = %d, len = %d, bitCount = %d, posCount = %d",
	                    maxBehindBy, samples, gotFrame, DecodeReader.state, DecodeReader.byteCount, DecodeReader.bitCount, DecodeReader.posCount);

	if (tracing && DecodeReader.byteCount > 0) {
		LogTrace(DecodeReader.output, DecodeReader.byteCount, 0, 0, NULL, true);
	}

	return DecodeReader.byteCount;
}


static void BuildIdentifyRequest(void);
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
	BuildIdentifyRequest();

	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	// Give the tags time to energize
	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);
	SpinDelay(100);

	// Now send the command
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER_TX);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_TX);

	LED_B_ON();
	for(int c = 0; c < ToSendMax; ) {
		if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_TXRDY)) {
			AT91C_BASE_SSC->SSC_THR = ~ToSend[c];
			c++;
		}
		WDT_HIT();
	}
	LED_B_OFF();

	// wait for last transfer to complete
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXEMPTY));

	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);

	for(int c = 0; c < 4000; ) {
		if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_RXRDY)) {
			uint16_t iq = AT91C_BASE_SSC->SSC_RHR;
			// The samples are correlations against I and Q versions of the
			// tone that the tag AM-modulates. We just want power,
			// so abs(I) + abs(Q) is close to what we want.
			int8_t i = (int8_t)(iq >> 8);
			int8_t q = (int8_t)(iq & 0xff);
			uint8_t r = AMPLITUDE(i, q);
			dest[c++] = r;
		}
	}

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LEDsoff();
}


// TODO: there is no trigger condition. The 14000 samples represent a time frame of 66ms.
// It is unlikely that we get something meaningful.
// TODO: Currently we only record tag answers. Add tracing of reader commands.
// TODO: would we get something at all? The carrier is switched on...
void RecordRawAdcSamplesIso15693(void)
{
	LEDsoff();
	LED_A_ON();

	uint8_t *dest = BigBuf_get_addr();

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	// Setup SSC
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);

	// Start from off (no field generated)
   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
   	SpinDelay(200);

	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	SpinDelay(100);

	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);

	for(int c = 0; c < 14000;) {
		if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_RXRDY)) {
			uint16_t iq = AT91C_BASE_SSC->SSC_RHR;
			// The samples are correlations against I and Q versions of the
			// tone that the tag AM-modulates. We just want power,
			// so abs(I) + abs(Q) is close to what we want.
			int8_t i = (int8_t)(iq >> 8);
			int8_t q = (int8_t)(iq & 0xff);
			uint8_t r = AMPLITUDE(i, q);
			dest[c++] = r;
		}
	}

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();
	Dbprintf("finished recording");
	LED_A_OFF();
}


// Initialize the proxmark as iso15k reader
// (this might produces glitches that confuse some tags
static void Iso15693InitReader() {
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	// Setup SSC
	// FpgaSetupSsc();

	// Start from off (no field generated)
	LED_D_OFF();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	SpinDelay(10);

	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);

	// Give the tags time to energize
	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);
	SpinDelay(250);
}

///////////////////////////////////////////////////////////////////////
// ISO 15693 Part 3 - Air Interface
// This section basically contains transmission and receiving of bits
///////////////////////////////////////////////////////////////////////

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

// uid is in transmission order (which is reverse of display order)
static void BuildReadBlockRequest(uint8_t *uid, uint8_t blockNumber )
{
	uint8_t cmd[13];

	uint16_t crc;
	// If we set the Option_Flag in this request, the VICC will respond with the secuirty status of the block
	// followed by teh block data
	// one sub-carrier, inventory, 1 slot, fast rate
	cmd[0] = (1 << 6)| (1 << 5) | (1 << 1); // no SELECT bit, ADDR bit, OPTION bit
	// READ BLOCK command code
	cmd[1] = 0x20;
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
	cmd[10] = blockNumber;//0x00;
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
// 	**recv will return you a pointer to the received data
// 	If you do not need the answer use NULL for *recv[]
//	return: lenght of received data
int SendDataTag(uint8_t *send, int sendlen, bool init, int speed, uint8_t **recv) {

	LED_A_ON();
	LED_B_OFF();
	LED_C_OFF();

	if (init) Iso15693InitReader();

	int answerLen=0;
	uint8_t *answer = BigBuf_get_addr() + 4000;
	if (recv != NULL) memset(answer, 0, 100);

	if (!speed) {
		// low speed (1 out of 256)
		CodeIso15693AsReader256(send, sendlen);
	} else {
		// high speed (1 out of 4)
		CodeIso15693AsReader(send, sendlen);
	}

	TransmitTo15693Tag(ToSend,ToSendMax);
	// Now wait for a response
	if (recv!=NULL) {
		answerLen = GetIso15693AnswerFromTag(answer, 100);
		*recv=answer;
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

	if (len>3) {
		if (d[0]&(1<<3))
			strncat(status,"ProtExt ",DBD15STATLEN);
		if (d[0]&1) {
			// error
			strncat(status,"Error ",DBD15STATLEN);
			switch (d[1]) {
				case 0x01:
					strncat(status,"01:notSupp",DBD15STATLEN);
					break;
				case 0x02:
					strncat(status,"02:notRecog",DBD15STATLEN);
					break;
				case 0x03:
					strncat(status,"03:optNotSupp",DBD15STATLEN);
					break;
				case 0x0f:
					strncat(status,"0f:noInfo",DBD15STATLEN);
					break;
				case 0x10:
					strncat(status,"10:dontExist",DBD15STATLEN);
					break;
				case 0x11:
					strncat(status,"11:lockAgain",DBD15STATLEN);
					break;
				case 0x12:
					strncat(status,"12:locked",DBD15STATLEN);
					break;
				case 0x13:
					strncat(status,"13:progErr",DBD15STATLEN);
					break;
				case 0x14:
					strncat(status,"14:lockErr",DBD15STATLEN);
					break;
				default:
					strncat(status,"unknownErr",DBD15STATLEN);
			}
			strncat(status," ",DBD15STATLEN);
		} else {
			strncat(status,"NoErr ",DBD15STATLEN);
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

//-----------------------------------------------------------------------------
// Simulate an ISO15693 reader, perform anti-collision and then attempt to read a sector
// all demodulation performed in arm rather than host. - greg
//-----------------------------------------------------------------------------
void ReaderIso15693(uint32_t parameter)
{
	LEDsoff();
	LED_A_ON();

	int answerLen1 = 0;
	uint8_t TagUID[8] = {0x00};

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);

	uint8_t *answer1 = BigBuf_get_addr() + 4000;
	memset(answer1, 0x00, 200);

	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	// Setup SSC
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);

	// Start from off (no field generated)
   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
   	SpinDelay(200);

	// Give the tags time to energize
	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);
	SpinDelay(200);

	// FIRST WE RUN AN INVENTORY TO GET THE TAG UID
	// THIS MEANS WE CAN PRE-BUILD REQUESTS TO SAVE CPU TIME

	// Now send the IDENTIFY command
	BuildIdentifyRequest();

	TransmitTo15693Tag(ToSend,ToSendMax);

	// Now wait for a response
	answerLen1 = GetIso15693AnswerFromTag(answer1, 100) ;

	if (answerLen1 >=12) // we should do a better check than this
	{
		TagUID[0] = answer1[2];
		TagUID[1] = answer1[3];
		TagUID[2] = answer1[4];
		TagUID[3] = answer1[5];
		TagUID[4] = answer1[6];
		TagUID[5] = answer1[7];
		TagUID[6] = answer1[8]; // IC Manufacturer code
		TagUID[7] = answer1[9]; // always E0

	}

	Dbprintf("%d octets read from IDENTIFY request:", answerLen1);
	DbdecodeIso15693Answer(answerLen1, answer1);
	Dbhexdump(answerLen1, answer1, false);

	// UID is reverse
	if (answerLen1 >= 12)
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
	if (answerLen1 >= 12 && DEBUG) {
		uint8_t *answer2 = BigBuf_get_addr() + 4100;
		int i = 0;
		while (i < 32) {  // sanity check, assume max 32 pages
			BuildReadBlockRequest(TagUID, i);
			TransmitTo15693Tag(ToSend, ToSendMax);
			int answerLen2 = GetIso15693AnswerFromTag(answer2, 100);
			if (answerLen2 > 0) {
				Dbprintf("READ SINGLE BLOCK %d returned %d octets:", i, answerLen2);
				DbdecodeIso15693Answer(answerLen2, answer2);
				Dbhexdump(answerLen2, answer2, false);
				if ( *((uint32_t*) answer2) == 0x07160101 ) break; // exit on NoPageErr
			}
			i++;
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
			start_time = eof_time + DELAY_ISO15693_VCD_TO_VICC - DELAY_ARM_TO_READER;
			TransmitTo15693Reader(ToSend, ToSendMax, start_time, slow);
		}

		Dbprintf("%d bytes read from reader:", cmd_len);
		Dbhexdump(cmd_len, cmd, false);
	}

	LEDsoff();
}


// Since there is no standardized way of reading the AFI out of a tag, we will brute force it
// (some manufactures offer a way to read the AFI, though)
void BruteforceIso15693Afi(uint32_t speed)
{
	LEDsoff();
	LED_A_ON();

	uint8_t data[20];
	uint8_t *recv=data;
	int datalen=0, recvlen=0;

	Iso15693InitReader();

	// first without AFI
	// Tags should respond without AFI and with AFI=0 even when AFI is active

	data[0] = ISO15693_REQ_DATARATE_HIGH | ISO15693_REQ_INVENTORY | ISO15693_REQINV_SLOT1;
	data[1] = ISO15693_INVENTORY;
	data[2] = 0; // mask length
	datalen = AddCrc(data,3);
	recvlen = SendDataTag(data, datalen, false, speed, &recv);
	WDT_HIT();
	if (recvlen>=12) {
		Dbprintf("NoAFI UID=%s",sprintUID(NULL,&recv[2]));
	}

	// now with AFI

	data[0] = ISO15693_REQ_DATARATE_HIGH | ISO15693_REQ_INVENTORY | ISO15693_REQINV_AFI | ISO15693_REQINV_SLOT1;
	data[1] = ISO15693_INVENTORY;
	data[2] = 0; // AFI
	data[3] = 0; // mask length

	for (int i=0;i<256;i++) {
		data[2]=i & 0xFF;
		datalen=AddCrc(data,4);
		recvlen=SendDataTag(data, datalen, false, speed, &recv);
		WDT_HIT();
		if (recvlen>=12) {
			Dbprintf("AFI=%i UID=%s", i, sprintUID(NULL,&recv[2]));
		}
	}
	Dbprintf("AFI Bruteforcing done.");

   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LEDsoff();
}

// Allows to directly send commands to the tag via the client
void DirectTag15693Command(uint32_t datalen, uint32_t speed, uint32_t recv, uint8_t data[]) {

	int recvlen=0;
	uint8_t *recvbuf = BigBuf_get_addr();

	LED_A_ON();

	if (DEBUG) {
		Dbprintf("SEND");
		Dbhexdump(datalen, data, false);
	}

	recvlen = SendDataTag(data, datalen, true, speed, (recv?&recvbuf:NULL));

	if (recv) {
		cmd_send(CMD_ACK, recvlen>48?48:recvlen, 0, 0, recvbuf, 48);

		if (DEBUG) {
			Dbprintf("RECV");
			DbdecodeIso15693Answer(recvlen,recvbuf);
			Dbhexdump(recvlen, recvbuf, false);
		}
	}

	// for the time being, switch field off to protect rdv4.0
	// note: this prevents using hf 15 cmd with s option - which isn't implemented yet anyway
   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
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
	// If we set the Option_Flag in this request, the VICC will respond with the secuirty status of the block
	// followed by teh block data
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
	// If we set the Option_Flag in this request, the VICC will respond with the secuirty status of the block
	// followed by teh block data
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
	// If we set the Option_Flag in this request, the VICC will respond with the secuirty status of the block
	// followed by teh block data
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
	// If we set the Option_Flag in this request, the VICC will respond with the secuirty status of the block
	// followed by teh block data
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
	cmd[10] = 0x05; // for custom codes this must be manufcturer code
	cmd[11] = 0x00;

//	cmd[12] = 0x00;
//	cmd[13] = 0x00;	//Now the CRC
	crc = Crc(cmd, 12); // the crc needs to be calculated over 2 bytes
	cmd[12] = crc & 0xff;
	cmd[13] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}




*/


