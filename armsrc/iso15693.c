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
//  data rate: 1,66 kbit/s (fc/8192)
//  used for long range
// 1 out of 4:
//  data rate: 26,48 kbit/s (fc/512)
//  used for short range, high speed
//
// VICC (tag) -> VCD (reader)
// Modulation:
//      ASK / one subcarrier (423,75 khz)
//      FSK / two subcarriers (423,75 khz && 484,28 khz)
// Data Rates / Modes:
//  low ASK: 6,62 kbit/s
//  low FSK: 6.67 kbit/s
//  high ASK: 26,48 kbit/s
//  high FSK: 26,69 kbit/s
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
#include "usb_cdc.h"
#include "BigBuf.h"
#include "fpgaloader.h"

#define arraylen(x) (sizeof(x)/sizeof((x)[0]))

// Delays in SSP_CLK ticks.
// SSP_CLK runs at 13,56MHz / 32 = 423.75kHz when simulating a tag
#define DELAY_READER_TO_ARM               8
#define DELAY_ARM_TO_READER               0
//SSP_CLK runs at 13.56MHz / 4 = 3,39MHz when acting as reader. All values should be multiples of 16
#define DELAY_ARM_TO_TAG                 16
#define DELAY_TAG_TO_ARM                 32
//SSP_CLK runs at 13.56MHz / 4 = 3,39MHz when snooping. All values should be multiples of 16
#define DELAY_TAG_TO_ARM_SNOOP           32
#define DELAY_READER_TO_ARM_SNOOP        32

// times in samples @ 212kHz when acting as reader
//#define ISO15693_READER_TIMEOUT              80 // 80/212kHz = 378us, nominal t1_max=313,9us
#define ISO15693_READER_TIMEOUT             330 // 330/212kHz = 1558us, should be even enough for iClass tags responding to ACTALL
#define ISO15693_READER_TIMEOUT_WRITE      4700 // 4700/212kHz = 22ms, nominal 20ms


static int DEBUG = 0;


///////////////////////////////////////////////////////////////////////
// ISO 15693 Part 2 - Air Interface
// This section basically contains transmission and receiving of bits
///////////////////////////////////////////////////////////////////////

// buffers
#define ISO15693_DMA_BUFFER_SIZE        256 // must be a power of 2
#define ISO15693_MAX_RESPONSE_LENGTH     36 // allows read single block with the maximum block size of 256bits. Read multiple blocks not supported yet
#define ISO15693_MAX_COMMAND_LENGTH      45 // allows write single block with the maximum block size of 256bits. Write multiple blocks not supported yet


// specific LogTrace function for ISO15693: the duration needs to be scaled because otherwise it won't fit into a uint16_t
bool LogTrace_ISO15693(const uint8_t *btBytes, uint16_t iLen, uint32_t timestamp_start, uint32_t timestamp_end, uint8_t *parity, bool readerToTag) {
	uint32_t duration = timestamp_end - timestamp_start;
	duration /= 32;
	timestamp_end = timestamp_start + duration;
	return LogTrace(btBytes, iLen, timestamp_start, timestamp_end, parity, readerToTag);
}


// ---------------------------
// Signal Processing
// ---------------------------

// prepare data using "1 out of 4" code for later transmission
// resulting data rate is 26.48 kbit/s (fc/512)
// cmd ... data
// n ... length of data
void CodeIso15693AsReader(uint8_t *cmd, int n) {

	ToSendReset();

	// SOF for 1of4
	ToSend[++ToSendMax] = 0x84; //10000100

	// data
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < 8; j += 2) {
			int these = (cmd[i] >> j) & 0x03;
			switch(these) {
				case 0:
					ToSend[++ToSendMax] = 0x40; //01000000
					break;
				case 1:
					ToSend[++ToSendMax] = 0x10; //00010000
					break;
				case 2:
					ToSend[++ToSendMax] = 0x04; //00000100
					break;
				case 3:
					ToSend[++ToSendMax] = 0x01; //00000001
					break;
			}
		}
	}

	// EOF
	ToSend[++ToSendMax] = 0x20; //0010 + 0000 padding

	ToSendMax++;
}


// Encode EOF only
static void CodeIso15693AsReaderEOF() {
	ToSendReset();
	ToSend[++ToSendMax] = 0x20;
	ToSendMax++;
}


// encode data using "1 out of 256" scheme
// data rate is 1,66 kbit/s (fc/8192)
// is designed for more robust communication over longer distances
static void CodeIso15693AsReader256(uint8_t *cmd, int n)
{
	ToSendReset();

	// SOF for 1of256
	ToSend[++ToSendMax] = 0x81; //10000001

	// data
	for(int i = 0; i < n; i++) {
		for (int j = 0; j <= 255; j++) {
			if (cmd[i] == j) {
				ToSendStuffBit(0);
				ToSendStuffBit(1);
			} else {
				ToSendStuffBit(0);
				ToSendStuffBit(0);
			}
		}
	}

	// EOF
	ToSend[++ToSendMax] = 0x20; //0010 + 0000 padding

	ToSendMax++;
}


// static uint8_t encode4Bits(const uint8_t b) {
	// uint8_t c = b & 0xF;
	// // OTA, the least significant bits first
	// //         The columns are
	// //               1 - Bit value to send
	// //               2 - Reversed (big-endian)
	// //               3 - Manchester Encoded
	// //               4 - Hex values

	// switch(c){
	// //                          1       2         3         4
	  // case 15: return 0x55; // 1111 -> 1111 -> 01010101 -> 0x55
	  // case 14: return 0x95; // 1110 -> 0111 -> 10010101 -> 0x95
	  // case 13: return 0x65; // 1101 -> 1011 -> 01100101 -> 0x65
	  // case 12: return 0xa5; // 1100 -> 0011 -> 10100101 -> 0xa5
	  // case 11: return 0x59; // 1011 -> 1101 -> 01011001 -> 0x59
	  // case 10: return 0x99; // 1010 -> 0101 -> 10011001 -> 0x99
	  // case 9:  return 0x69; // 1001 -> 1001 -> 01101001 -> 0x69
	  // case 8:  return 0xa9; // 1000 -> 0001 -> 10101001 -> 0xa9
	  // case 7:  return 0x56; // 0111 -> 1110 -> 01010110 -> 0x56
	  // case 6:  return 0x96; // 0110 -> 0110 -> 10010110 -> 0x96
	  // case 5:  return 0x66; // 0101 -> 1010 -> 01100110 -> 0x66
	  // case 4:  return 0xa6; // 0100 -> 0010 -> 10100110 -> 0xa6
	  // case 3:  return 0x5a; // 0011 -> 1100 -> 01011010 -> 0x5a
	  // case 2:  return 0x9a; // 0010 -> 0100 -> 10011010 -> 0x9a
	  // case 1:  return 0x6a; // 0001 -> 1000 -> 01101010 -> 0x6a
	  // default: return 0xaa; // 0000 -> 0000 -> 10101010 -> 0xaa

	// }
// }

static const uint8_t encode_4bits[16] = { 0xaa, 0x6a, 0x9a, 0x5a, 0xa6, 0x66, 0x96, 0x56, 0xa9, 0x69, 0x99, 0x59, 0xa5, 0x65, 0x95, 0x55 };

void CodeIso15693AsTag(uint8_t *cmd, size_t len) {
	/*
	 * SOF comprises 3 parts;
	 * * An unmodulated time of 56.64 us
	 * * 24 pulses of 423.75 kHz (fc/32)
	 * * A logic 1, which starts with an unmodulated time of 18.88us
	 *   followed by 8 pulses of 423.75kHz (fc/32)
	 *
	 * EOF comprises 3 parts:
	 * - A logic 0 (which starts with 8 pulses of fc/32 followed by an unmodulated
	 *   time of 18.88us.
	 * - 24 pulses of fc/32
	 * - An unmodulated time of 56.64 us
	 *
	 * A logic 0 starts with 8 pulses of fc/32
	 * followed by an unmodulated time of 256/fc (~18,88us).
	 *
	 * A logic 0 starts with unmodulated time of 256/fc (~18,88us) followed by
	 * 8 pulses of fc/32 (also 18.88us)
	 *
	 * A bit here becomes 8 pulses of fc/32. Therefore:
	 * The SOF can be written as 00011101 = 0x1D
	 * The EOF can be written as 10111000 = 0xb8
	 * A logic 1 is 01
	 * A logic 0 is 10
	 *
	 * */

	ToSendReset();

	// SOF
	ToSend[++ToSendMax] = 0x1D;  // 00011101

	// data
	for (int i = 0; i < len; i++) {
		ToSend[++ToSendMax] = encode_4bits[cmd[i] & 0xF];
		ToSend[++ToSendMax] = encode_4bits[cmd[i] >> 4];
	}

	// EOF
	ToSend[++ToSendMax] = 0xB8; // 10111000

	ToSendMax++;
}


// Transmit the command (to the tag) that was placed in cmd[].
void TransmitTo15693Tag(const uint8_t *cmd, int len, uint32_t *start_time) {

	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SEND_FULL_MOD);

	if (*start_time < DELAY_ARM_TO_TAG) {
		*start_time = DELAY_ARM_TO_TAG;
	}

	*start_time = (*start_time - DELAY_ARM_TO_TAG) & 0xfffffff0;

	if (GetCountSspClk() > *start_time) { // we may miss the intended time
		*start_time = (GetCountSspClk() + 16) & 0xfffffff0; // next possible time
	}

	while (GetCountSspClk() < *start_time)
		/* wait */ ;

	LED_B_ON();
	for (int c = 0; c < len; c++) {
		uint8_t data = cmd[c];
		for (int i = 0; i < 8; i++) {
			uint16_t send_word = (data & 0x80) ? 0xffff : 0x0000;
			while (!(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_TXRDY))) ;
			AT91C_BASE_SSC->SSC_THR = send_word;
			while (!(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_TXRDY))) ;
			AT91C_BASE_SSC->SSC_THR = send_word;
			data <<= 1;
		}
		WDT_HIT();
	}
	LED_B_OFF();

	*start_time = *start_time + DELAY_ARM_TO_TAG;
}


//-----------------------------------------------------------------------------
// Transmit the tag response (to the reader) that was placed in cmd[].
//-----------------------------------------------------------------------------
void TransmitTo15693Reader(const uint8_t *cmd, size_t len, uint32_t *start_time, uint32_t slot_time, bool slow) {
	// don't use the FPGA_HF_SIMULATOR_MODULATE_424K_8BIT minor mode. It would spoil GetCountSspClk()
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_MODULATE_424K);

	uint32_t modulation_start_time = *start_time - DELAY_ARM_TO_READER + 3 * 8;  // no need to transfer the unmodulated start of SOF

	while (GetCountSspClk() > (modulation_start_time & 0xfffffff8) + 3) { // we will miss the intended time
		if (slot_time) {
			modulation_start_time += slot_time; // use next available slot
		} else {
			modulation_start_time = (modulation_start_time & 0xfffffff8) + 8; // next possible time
		}
	}

	while (GetCountSspClk() < (modulation_start_time & 0xfffffff8))
		/* wait */ ;

	uint8_t shift_delay = modulation_start_time & 0x00000007;

	*start_time = modulation_start_time + DELAY_ARM_TO_READER - 3 * 8;

	LED_C_ON();
	uint8_t bits_to_shift = 0x00;
	uint8_t bits_to_send = 0x00;
	for (size_t c = 0; c < len; c++) {
		for (int i = (c==0?4:7); i >= 0; i--) {
			uint8_t cmd_bits = ((cmd[c] >> i) & 0x01) ? 0xff : 0x00;
			for (int j = 0; j < (slow?4:1); ) {
				if (AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXRDY) {
					bits_to_send = bits_to_shift << (8 - shift_delay) | cmd_bits >> shift_delay;
					AT91C_BASE_SSC->SSC_THR = bits_to_send;
					bits_to_shift = cmd_bits;
					j++;
				}
			}
		}
		WDT_HIT();
	}
	// send the remaining bits, padded with 0:
	bits_to_send = bits_to_shift << (8 - shift_delay);
	for ( ; ; ) {
		if (AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXRDY) {
			AT91C_BASE_SSC->SSC_THR = bits_to_send;
			break;
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

#define NOISE_THRESHOLD          80                   // don't try to correlate noise
#define MAX_PREVIOUS_AMPLITUDE   (-1 - NOISE_THRESHOLD)

typedef struct DecodeTag {
	enum {
		STATE_TAG_SOF_LOW,
		STATE_TAG_SOF_RISING_EDGE,
		STATE_TAG_SOF_HIGH,
		STATE_TAG_SOF_HIGH_END,
		STATE_TAG_RECEIVING_DATA,
		STATE_TAG_EOF,
		STATE_TAG_EOF_TAIL
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
	int       threshold_sof;
	int       threshold_half;
	uint16_t  previous_amplitude;
} DecodeTag_t;


static int inline __attribute__((always_inline)) Handle15693SamplesFromTag(uint16_t amplitude, DecodeTag_t *DecodeTag) {
	switch (DecodeTag->state) {
		case STATE_TAG_SOF_LOW:
			// waiting for a rising edge
			if (amplitude > NOISE_THRESHOLD + DecodeTag->previous_amplitude) {
				if (DecodeTag->posCount > 10) {
					DecodeTag->threshold_sof = amplitude - DecodeTag->previous_amplitude; // to be divided by 2
					DecodeTag->threshold_half = 0;
					DecodeTag->state = STATE_TAG_SOF_RISING_EDGE;
				} else {
					DecodeTag->posCount = 0;
				}
			} else {
				DecodeTag->posCount++;
				DecodeTag->previous_amplitude = amplitude;
			}
			break;

		case STATE_TAG_SOF_RISING_EDGE:
			if (amplitude > DecodeTag->threshold_sof + DecodeTag->previous_amplitude) { // edge still rising
				if (amplitude > DecodeTag->threshold_sof + DecodeTag->threshold_sof) { // steeper edge, take this as time reference
					DecodeTag->posCount = 1;
				} else {
					DecodeTag->posCount = 2;
				}
				DecodeTag->threshold_sof = (amplitude - DecodeTag->previous_amplitude) / 2;
			} else {
				DecodeTag->posCount = 2;
				DecodeTag->threshold_sof = DecodeTag->threshold_sof/2;
			}
			// DecodeTag->posCount = 2;
			DecodeTag->state = STATE_TAG_SOF_HIGH;
			break;

		case STATE_TAG_SOF_HIGH:
			// waiting for 10 times high. Take average over the last 8
			if (amplitude > DecodeTag->threshold_sof) {
				DecodeTag->posCount++;
				if (DecodeTag->posCount > 2) {
					DecodeTag->threshold_half += amplitude; // keep track of average high value
				}
				if (DecodeTag->posCount == 10) {
					DecodeTag->threshold_half >>= 2; // (4 times 1/2 average)
					DecodeTag->state = STATE_TAG_SOF_HIGH_END;
				}
			} else { // high phase was too short
				DecodeTag->posCount = 1;
				DecodeTag->previous_amplitude = amplitude;
				DecodeTag->state = STATE_TAG_SOF_LOW;
			}
			break;

		case STATE_TAG_SOF_HIGH_END:
			// check for falling edge
			if (DecodeTag->posCount == 13 && amplitude < DecodeTag->threshold_sof) {
				DecodeTag->lastBit = SOF_PART1;  // detected 1st part of SOF (12 samples low and 12 samples high)
				DecodeTag->shiftReg = 0;
				DecodeTag->bitCount = 0;
				DecodeTag->len = 0;
				DecodeTag->sum1 = amplitude;
				DecodeTag->sum2 = 0;
				DecodeTag->posCount = 2;
				DecodeTag->state = STATE_TAG_RECEIVING_DATA;
				// FpgaDisableTracing(); // DEBUGGING
				// Dbprintf("amplitude = %d, threshold_sof = %d, threshold_half/4 = %d, previous_amplitude = %d",
					// amplitude,
					// DecodeTag->threshold_sof,
					// DecodeTag->threshold_half/4,
					// DecodeTag->previous_amplitude); // DEBUGGING
				LED_C_ON();
			} else {
				DecodeTag->posCount++;
				if (DecodeTag->posCount > 13) { // high phase too long
					DecodeTag->posCount = 0;
					DecodeTag->previous_amplitude = amplitude;
					DecodeTag->state = STATE_TAG_SOF_LOW;
					LED_C_OFF();
				}
			}
			break;

		case STATE_TAG_RECEIVING_DATA:
				// FpgaDisableTracing(); // DEBUGGING
				// Dbprintf("amplitude = %d, threshold_sof = %d, threshold_half/4 = %d, previous_amplitude = %d",
					// amplitude,
					// DecodeTag->threshold_sof,
					// DecodeTag->threshold_half/4,
					// DecodeTag->previous_amplitude); // DEBUGGING
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
				if (DecodeTag->sum1 > DecodeTag->threshold_half && DecodeTag->sum2 > DecodeTag->threshold_half) { // modulation in both halves
					if (DecodeTag->lastBit == LOGIC0) {  // this was already part of EOF
						DecodeTag->state = STATE_TAG_EOF;
					} else {
						DecodeTag->posCount = 0;
						DecodeTag->previous_amplitude = amplitude;
						DecodeTag->state = STATE_TAG_SOF_LOW;
						LED_C_OFF();
					}
				} else if (DecodeTag->sum1 < DecodeTag->threshold_half && DecodeTag->sum2 > DecodeTag->threshold_half) { // modulation in second half
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
							// if (DecodeTag->shiftReg == 0x12 && DecodeTag->len == 1) FpgaDisableTracing(); // DEBUGGING
							if (DecodeTag->len > DecodeTag->max_len) {
								// buffer overflow, give up
								LED_C_OFF();
								return true;
							}
							DecodeTag->bitCount = 0;
							DecodeTag->shiftReg = 0;
						}
					}
				} else if (DecodeTag->sum1 > DecodeTag->threshold_half && DecodeTag->sum2 < DecodeTag->threshold_half) { // modulation in first half
					// logic 0
					if (DecodeTag->lastBit == SOF_PART1) { // incomplete SOF
						DecodeTag->posCount = 0;
						DecodeTag->previous_amplitude = amplitude;
						DecodeTag->state = STATE_TAG_SOF_LOW;
						LED_C_OFF();
					} else {
						DecodeTag->lastBit = LOGIC0;
						DecodeTag->shiftReg >>= 1;
						DecodeTag->bitCount++;
						if (DecodeTag->bitCount == 8) {
							DecodeTag->output[DecodeTag->len] = DecodeTag->shiftReg;
							DecodeTag->len++;
							// if (DecodeTag->shiftReg == 0x12 && DecodeTag->len == 1) FpgaDisableTracing(); // DEBUGGING
							if (DecodeTag->len > DecodeTag->max_len) {
								// buffer overflow, give up
								DecodeTag->posCount = 0;
								DecodeTag->previous_amplitude = amplitude;
								DecodeTag->state = STATE_TAG_SOF_LOW;
								LED_C_OFF();
							}
							DecodeTag->bitCount = 0;
							DecodeTag->shiftReg = 0;
						}
					}
				} else { // no modulation
					if (DecodeTag->lastBit == SOF_PART2) { // only SOF (this is OK for iClass)
						LED_C_OFF();
						return true;
					} else {
						DecodeTag->posCount = 0;
						DecodeTag->state = STATE_TAG_SOF_LOW;
						LED_C_OFF();
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
				if (DecodeTag->sum1 > DecodeTag->threshold_half && DecodeTag->sum2 < DecodeTag->threshold_half) { // modulation in first half
					DecodeTag->posCount = 0;
					DecodeTag->state = STATE_TAG_EOF_TAIL;
				} else {
					DecodeTag->posCount = 0;
					DecodeTag->previous_amplitude = amplitude;
					DecodeTag->state = STATE_TAG_SOF_LOW;
					LED_C_OFF();
				}
			}
			DecodeTag->posCount++;
			break;

		case STATE_TAG_EOF_TAIL:
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
				if (DecodeTag->sum1 < DecodeTag->threshold_half && DecodeTag->sum2 < DecodeTag->threshold_half) { // no modulation in both halves
					LED_C_OFF();
					return true;
				} else {
					DecodeTag->posCount = 0;
					DecodeTag->previous_amplitude = amplitude;
					DecodeTag->state = STATE_TAG_SOF_LOW;
					LED_C_OFF();
				}
			}
			DecodeTag->posCount++;
			break;
	}

	return false;
}


static void DecodeTagInit(DecodeTag_t *DecodeTag, uint8_t *data, uint16_t max_len) {
	DecodeTag->previous_amplitude = MAX_PREVIOUS_AMPLITUDE;
	DecodeTag->posCount = 0;
	DecodeTag->state = STATE_TAG_SOF_LOW;
	DecodeTag->output = data;
	DecodeTag->max_len = max_len;
}


static void DecodeTagReset(DecodeTag_t *DecodeTag) {
	DecodeTag->posCount = 0;
	DecodeTag->state = STATE_TAG_SOF_LOW;
	DecodeTag->previous_amplitude = MAX_PREVIOUS_AMPLITUDE;
}


/*
 *  Receive and decode the tag response, also log to tracebuffer
 */
int GetIso15693AnswerFromTag(uint8_t* response, uint16_t max_len, uint16_t timeout, uint32_t *eof_time) {

	int samples = 0;
	int ret = 0;

	uint16_t dmaBuf[ISO15693_DMA_BUFFER_SIZE];

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
	uint32_t dma_start_time = 0;
	uint16_t *upTo = dmaBuf;

	for(;;) {
		uint16_t behindBy = ((uint16_t*)AT91C_BASE_PDC_SSC->PDC_RPR - upTo) & (ISO15693_DMA_BUFFER_SIZE-1);

		if (behindBy == 0) continue;

		samples++;
		if (samples == 1) {
			// DMA has transferred the very first data
			dma_start_time = GetCountSspClk() & 0xfffffff0;
		}

		uint16_t tagdata = *upTo++;

		if(upTo >= dmaBuf + ISO15693_DMA_BUFFER_SIZE) {                // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                             // start reading the circular buffer from the beginning
			if (behindBy > (9*ISO15693_DMA_BUFFER_SIZE/10)) {
				Dbprintf("About to blow circular buffer - aborted! behindBy=%d", behindBy);
				ret = -1;
				break;
			}
		}
		if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
			AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
			AT91C_BASE_PDC_SSC->PDC_RNCR = ISO15693_DMA_BUFFER_SIZE;   // DMA Next Counter registers
		}

		if (Handle15693SamplesFromTag(tagdata, &DecodeTag)) {
			*eof_time = dma_start_time + samples*16 - DELAY_TAG_TO_ARM; // end of EOF
			if (DecodeTag.lastBit == SOF_PART2) {
				*eof_time -= 8*16; // needed 8 additional samples to confirm single SOF (iCLASS)
			}
			if (DecodeTag.len > DecodeTag.max_len) {
				ret = -2; // buffer overflow
			}
			break;
		}

		if (samples > timeout && DecodeTag.state < STATE_TAG_RECEIVING_DATA) {
			ret = -1;   // timeout
			break;
		}

	}

	FpgaDisableSscDma();

	if (DEBUG) Dbprintf("samples = %d, ret = %d, Decoder: state = %d, lastBit = %d, len = %d, bitCount = %d, posCount = %d",
						samples, ret, DecodeTag.state, DecodeTag.lastBit, DecodeTag.len, DecodeTag.bitCount, DecodeTag.posCount);

	if (ret < 0) {
		return ret;
	}

	uint32_t sof_time = *eof_time
						- DecodeTag.len * 8 * 8 * 16 // time for byte transfers
						- 32 * 16  // time for SOF transfer
						- (DecodeTag.lastBit != SOF_PART2?32*16:0); // time for EOF transfer

	if (DEBUG) Dbprintf("timing: sof_time = %d, eof_time = %d", sof_time, *eof_time);

	LogTrace_ISO15693(DecodeTag.output, DecodeTag.len, sof_time*4, *eof_time*4, NULL, false);

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
		STATE_READER_AWAIT_1ST_FALLING_EDGE_OF_SOF,
		STATE_READER_AWAIT_1ST_RISING_EDGE_OF_SOF,
		STATE_READER_AWAIT_2ND_FALLING_EDGE_OF_SOF,
		STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF,
		STATE_READER_AWAIT_END_OF_SOF_1_OUT_OF_4,
		STATE_READER_RECEIVE_DATA_1_OUT_OF_4,
		STATE_READER_RECEIVE_DATA_1_OUT_OF_256,
		STATE_READER_RECEIVE_JAMMING
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
	int         sum1, sum2;
	uint8_t     *output;
	uint8_t     jam_search_len;
	uint8_t     *jam_search_string;
} DecodeReader_t;


static void DecodeReaderInit(DecodeReader_t* DecodeReader, uint8_t *data, uint16_t max_len, uint8_t jam_search_len, uint8_t *jam_search_string) {
	DecodeReader->output = data;
	DecodeReader->byteCountMax = max_len;
	DecodeReader->state = STATE_READER_UNSYNCD;
	DecodeReader->byteCount = 0;
	DecodeReader->bitCount = 0;
	DecodeReader->posCount = 1;
	DecodeReader->shiftReg = 0;
	DecodeReader->jam_search_len = jam_search_len;
	DecodeReader->jam_search_string = jam_search_string;
}


static void DecodeReaderReset(DecodeReader_t* DecodeReader) {
	DecodeReader->state = STATE_READER_UNSYNCD;
}


static int inline __attribute__((always_inline)) Handle15693SampleFromReader(bool bit, DecodeReader_t *DecodeReader) {
	switch (DecodeReader->state) {
		case STATE_READER_UNSYNCD:
			// wait for unmodulated carrier
			if (bit) {
				DecodeReader->state = STATE_READER_AWAIT_1ST_FALLING_EDGE_OF_SOF;
			}
			break;

		case STATE_READER_AWAIT_1ST_FALLING_EDGE_OF_SOF:
			if (!bit) {
				// we went low, so this could be the beginning of a SOF
				DecodeReader->posCount = 1;
				DecodeReader->state = STATE_READER_AWAIT_1ST_RISING_EDGE_OF_SOF;
			}
			break;

		case STATE_READER_AWAIT_1ST_RISING_EDGE_OF_SOF:
			DecodeReader->posCount++;
			if (bit) { // detected rising edge
				if (DecodeReader->posCount < 4) { // rising edge too early (nominally expected at 5)
					DecodeReader->state = STATE_READER_AWAIT_1ST_FALLING_EDGE_OF_SOF;
				} else { // SOF
					DecodeReader->state = STATE_READER_AWAIT_2ND_FALLING_EDGE_OF_SOF;
				}
			} else {
				if (DecodeReader->posCount > 5) { // stayed low for too long
					DecodeReaderReset(DecodeReader);
				} else {
					// do nothing, keep waiting
				}
			}
			break;

		case STATE_READER_AWAIT_2ND_FALLING_EDGE_OF_SOF:
			DecodeReader->posCount++;
			if (!bit) { // detected a falling edge
				if (DecodeReader->posCount < 20) {         // falling edge too early (nominally expected at 21 earliest)
					DecodeReaderReset(DecodeReader);
				} else if (DecodeReader->posCount < 23) {  // SOF for 1 out of 4 coding
					DecodeReader->Coding = CODING_1_OUT_OF_4;
					DecodeReader->state = STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF;
				} else if (DecodeReader->posCount < 28) {  // falling edge too early (nominally expected at 29 latest)
					DecodeReaderReset(DecodeReader);
				} else {                                   // SOF for 1 out of 256 coding
					DecodeReader->Coding = CODING_1_OUT_OF_256;
					DecodeReader->state = STATE_READER_AWAIT_2ND_RISING_EDGE_OF_SOF;
				}
			} else {
				if (DecodeReader->posCount > 29) { // stayed high for too long
					DecodeReader->state = STATE_READER_AWAIT_1ST_FALLING_EDGE_OF_SOF;
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
						DecodeReader->state = STATE_READER_AWAIT_1ST_FALLING_EDGE_OF_SOF;
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
						DecodeReader->state = STATE_READER_AWAIT_1ST_FALLING_EDGE_OF_SOF;
					} else {
						DecodeReader->posCount = 1;
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
				if (DecodeReader->posCount == 9) {
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
				DecodeReader->sum1 = bit?1:0;
			} else if (DecodeReader->posCount <= 4) {
				if (bit) DecodeReader->sum1++;
			} else if (DecodeReader->posCount == 5) {
				DecodeReader->sum2 = bit?1:0;
			} else {
				if (bit) DecodeReader->sum2++;
			}
			if (DecodeReader->posCount == 8) {
				DecodeReader->posCount = 0;
				if (DecodeReader->sum1 <= 1 && DecodeReader->sum2 >= 3) { // EOF
					LED_B_OFF(); // Finished receiving
					DecodeReaderReset(DecodeReader);
					if (DecodeReader->byteCount != 0) {
						return true;
					}
				} else if (DecodeReader->sum1 >= 3 && DecodeReader->sum2 <= 1) { // detected a 2bit position
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
					if (DecodeReader->byteCount == DecodeReader->jam_search_len) {
						if (!memcmp(DecodeReader->output, DecodeReader->jam_search_string, DecodeReader->jam_search_len)) {
							LED_D_ON();
							FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SEND_JAM);
							DecodeReader->state = STATE_READER_RECEIVE_JAMMING;
						}
					}
				} else {
					DecodeReader->bitCount++;
				}
			}
			break;

		case STATE_READER_RECEIVE_DATA_1_OUT_OF_256:
			DecodeReader->posCount++;
			if (DecodeReader->posCount == 1) {
				DecodeReader->sum1 = bit?1:0;
			} else if (DecodeReader->posCount <= 4) {
				if (bit) DecodeReader->sum1++;
			} else if (DecodeReader->posCount == 5) {
				DecodeReader->sum2 = bit?1:0;
			} else if (bit) {
				DecodeReader->sum2++;
			}
			if (DecodeReader->posCount == 8) {
				DecodeReader->posCount = 0;
				if (DecodeReader->sum1 <= 1 && DecodeReader->sum2 >= 3) { // EOF
					LED_B_OFF(); // Finished receiving
					DecodeReaderReset(DecodeReader);
					if (DecodeReader->byteCount != 0) {
						return true;
					}
				} else if (DecodeReader->sum1 >= 3 && DecodeReader->sum2 <= 1) { // detected the bit position
					DecodeReader->shiftReg = DecodeReader->bitCount;
				}
				if (DecodeReader->bitCount == 255) { // we have a full byte
					DecodeReader->output[DecodeReader->byteCount++] = DecodeReader->shiftReg;
					if (DecodeReader->byteCount > DecodeReader->byteCountMax) {
						// buffer overflow, give up
						LED_B_OFF();
						DecodeReaderReset(DecodeReader);
					}
					if (DecodeReader->byteCount == DecodeReader->jam_search_len) {
						if (!memcmp(DecodeReader->output, DecodeReader->jam_search_string, DecodeReader->jam_search_len)) {
							LED_D_ON();
							FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SEND_JAM);
							DecodeReader->state = STATE_READER_RECEIVE_JAMMING;
						}
					}
				}
				DecodeReader->bitCount++;
			}
			break;

		case STATE_READER_RECEIVE_JAMMING:
			DecodeReader->posCount++;
			if (DecodeReader->Coding == CODING_1_OUT_OF_4) {
				if (DecodeReader->posCount == 7*16) { // 7 bits jammed
					FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SNOOP_AMPLITUDE); // stop jamming
					// FpgaDisableTracing();
					LED_D_OFF();
				} else if (DecodeReader->posCount == 8*16) {
					DecodeReader->posCount = 0;
					DecodeReader->output[DecodeReader->byteCount++] = 0x00;
					DecodeReader->state = STATE_READER_RECEIVE_DATA_1_OUT_OF_4;
				}
			} else {
				if (DecodeReader->posCount == 7*256) { // 7 bits jammend
					FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SNOOP_AMPLITUDE); // stop jamming
					LED_D_OFF();
				} else if (DecodeReader->posCount == 8*256) {
					DecodeReader->posCount = 0;
					DecodeReader->output[DecodeReader->byteCount++] = 0x00;
					DecodeReader->state = STATE_READER_RECEIVE_DATA_1_OUT_OF_256;
				}
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
// (returns len) or someone presses the pushbutton on the board (returns -1).
//
// Assume that we're called with the SSC (to the FPGA) and ADC path set
// correctly.
//-----------------------------------------------------------------------------

int GetIso15693CommandFromReader(uint8_t *received, size_t max_len, uint32_t *eof_time) {
	int samples = 0;
	bool gotFrame = false;
	uint8_t b;

	uint8_t dmaBuf[ISO15693_DMA_BUFFER_SIZE];

	// the decoder data structure
	DecodeReader_t DecodeReader = {0};
	DecodeReaderInit(&DecodeReader, received, max_len, 0, NULL);

	// wait for last transfer to complete
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXEMPTY));

	LED_D_OFF();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_NO_MODULATION);

	// clear receive register and wait for next transfer
	uint32_t temp = AT91C_BASE_SSC->SSC_RHR;
	(void) temp;
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_RXRDY)) ;

	uint32_t dma_start_time = GetCountSspClk() & 0xfffffff8;

	// Setup and start DMA.
	FpgaSetupSscDma(dmaBuf, ISO15693_DMA_BUFFER_SIZE);
	uint8_t *upTo = dmaBuf;

	for (;;) {
		uint16_t behindBy = ((uint8_t*)AT91C_BASE_PDC_SSC->PDC_RPR - upTo) & (ISO15693_DMA_BUFFER_SIZE-1);

		if (behindBy == 0) continue;

		b = *upTo++;
		if (upTo >= dmaBuf + ISO15693_DMA_BUFFER_SIZE) {               // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                             // start reading the circular buffer from the beginning
			if (behindBy > (9*ISO15693_DMA_BUFFER_SIZE/10)) {
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
				*eof_time = dma_start_time + samples - DELAY_READER_TO_ARM; // end of EOF
				gotFrame = true;
				break;
			}
			samples++;
		}

		if (gotFrame) {
			break;
		}

		if (BUTTON_PRESS()) {
			DecodeReader.byteCount = -1;
			break;
		}

		WDT_HIT();
	}

	FpgaDisableSscDma();

	if (DEBUG) Dbprintf("samples = %d, gotFrame = %d, Decoder: state = %d, len = %d, bitCount = %d, posCount = %d",
						samples, gotFrame, DecodeReader.state, DecodeReader.byteCount, DecodeReader.bitCount, DecodeReader.posCount);

	if (DecodeReader.byteCount > 0) {
		uint32_t sof_time = *eof_time
						- DecodeReader.byteCount * (DecodeReader.Coding==CODING_1_OUT_OF_4?128:2048) // time for byte transfers
						- 32  // time for SOF transfer
						- 16; // time for EOF transfer
		LogTrace_ISO15693(DecodeReader.output, DecodeReader.byteCount, sof_time*32, *eof_time*32, NULL, true);
	}

	return DecodeReader.byteCount;
}


// Construct an identify (Inventory) request, which is the first
// thing that you must send to a tag to get a response.
static void BuildIdentifyRequest(uint8_t *cmd) {
	uint16_t crc;
	// one sub-carrier, inventory, 1 slot, fast rate
	cmd[0] = ISO15693_REQ_INVENTORY | ISO15693_REQINV_SLOT1 | ISO15693_REQ_DATARATE_HIGH;
	// inventory command code
	cmd[1] = 0x01;
	// no mask
	cmd[2] = 0x00;
	//Now the CRC
	crc = Iso15693Crc(cmd, 3);
	cmd[3] = crc & 0xff;
	cmd[4] = crc >> 8;
}


//-----------------------------------------------------------------------------
// Start to read an ISO 15693 tag. We send an identify request, then wait
// for the response. The response is not demodulated, just left in the buffer
// so that it can be downloaded to a PC and processed there.
//-----------------------------------------------------------------------------
void AcquireRawAdcSamplesIso15693(void) {
	LED_A_ON();

	uint8_t *dest = BigBuf_get_addr();

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER);
	LED_D_ON();
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	uint8_t cmd[5];
	BuildIdentifyRequest(cmd);
	CodeIso15693AsReader(cmd, sizeof(cmd));

	// Give the tags time to energize
	SpinDelay(100);

	// Now send the command
	uint32_t start_time = 0;
	TransmitTo15693Tag(ToSend, ToSendMax, &start_time);

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


void SnoopIso15693(uint8_t jam_search_len, uint8_t *jam_search_string) {

	LED_A_ON();

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);

	clear_trace();
	set_tracing(true);

	// The DMA buffer, used to stream samples from the FPGA
	uint16_t dmaBuf[ISO15693_DMA_BUFFER_SIZE];

	// Count of samples received so far, so that we can include timing
	// information in the trace buffer.
	int samples = 0;

	DecodeTag_t DecodeTag = {0};
	uint8_t response[ISO15693_MAX_RESPONSE_LENGTH];
	DecodeTagInit(&DecodeTag, response, sizeof(response));

	DecodeReader_t DecodeReader = {0};
	uint8_t cmd[ISO15693_MAX_COMMAND_LENGTH];
	DecodeReaderInit(&DecodeReader, cmd, sizeof(cmd), jam_search_len, jam_search_string);

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
	LED_D_OFF();
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	StartCountSspClk();
	FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);

	bool TagIsActive = false;
	bool ReaderIsActive = false;
	bool ExpectTagAnswer = false;
	uint32_t dma_start_time = 0;
	uint16_t *upTo = dmaBuf;

	uint16_t max_behindBy = 0;
	
	// And now we loop, receiving samples.
	for(;;) {
		uint16_t behindBy = ((uint16_t*)AT91C_BASE_PDC_SSC->PDC_RPR - upTo) & (ISO15693_DMA_BUFFER_SIZE-1);
		if (behindBy > max_behindBy) {
			max_behindBy = behindBy;
		}
		
		if (behindBy == 0) continue;

		samples++;
		if (samples == 1) {
			// DMA has transferred the very first data
			dma_start_time = GetCountSspClk() & 0xfffffff0;
		}

		uint16_t snoopdata = *upTo++;

		if (upTo >= dmaBuf + ISO15693_DMA_BUFFER_SIZE) {                   // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                                 // start reading the circular buffer from the beginning
			if (behindBy > (9*ISO15693_DMA_BUFFER_SIZE/10)) {
				// FpgaDisableTracing();
				Dbprintf("About to blow circular buffer - aborted! behindBy=%d, samples=%d", behindBy, samples);
				break;
			}
			if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
				AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
				AT91C_BASE_PDC_SSC->PDC_RNCR = ISO15693_DMA_BUFFER_SIZE;   // DMA Next Counter registers
				WDT_HIT();
				if (BUTTON_PRESS()) {
					DbpString("Snoop stopped.");
					break;
				}
			}
		}

		if (!TagIsActive) {                                                // no need to try decoding reader data if the tag is sending
			if (Handle15693SampleFromReader(snoopdata & 0x02, &DecodeReader)) {
				// FpgaDisableSscDma();
				uint32_t eof_time = dma_start_time + samples*16 + 8 - DELAY_READER_TO_ARM_SNOOP; // end of EOF
				if (DecodeReader.byteCount > 0) {
					uint32_t sof_time = eof_time
									- DecodeReader.byteCount * (DecodeReader.Coding==CODING_1_OUT_OF_4?128*16:2048*16) // time for byte transfers
									- 32*16  // time for SOF transfer
									- 16*16; // time for EOF transfer
					LogTrace_ISO15693(DecodeReader.output, DecodeReader.byteCount, sof_time*4, eof_time*4, NULL, true);
				}
				/* And ready to receive another command. */
				DecodeReaderReset(&DecodeReader);
				/* And also reset the demod code, which might have been */
				/* false-triggered by the commands from the reader. */
				DecodeTagReset(&DecodeTag);
				ReaderIsActive = false;
				ExpectTagAnswer = true;
				// upTo = dmaBuf;
				// samples = 0;
				// FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);
				// continue;
			} else if (Handle15693SampleFromReader(snoopdata & 0x01, &DecodeReader)) {
				// FpgaDisableSscDma();
				uint32_t eof_time = dma_start_time + samples*16 + 16 - DELAY_READER_TO_ARM_SNOOP; // end of EOF
				if (DecodeReader.byteCount > 0) {
					uint32_t sof_time = eof_time
									- DecodeReader.byteCount * (DecodeReader.Coding==CODING_1_OUT_OF_4?128*16:2048*16) // time for byte transfers
									- 32*16  // time for SOF transfer
									- 16*16; // time for EOF transfer
					LogTrace_ISO15693(DecodeReader.output, DecodeReader.byteCount, sof_time*4, eof_time*4, NULL, true);
				}
				/* And ready to receive another command. */
				DecodeReaderReset(&DecodeReader);
				/* And also reset the demod code, which might have been */
				/* false-triggered by the commands from the reader. */
				DecodeTagReset(&DecodeTag);
				ReaderIsActive = false;
				ExpectTagAnswer = true;
				// upTo = dmaBuf;
				// samples = 0;
				// FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);
				// continue;
			} else {
				ReaderIsActive = (DecodeReader.state >= STATE_READER_RECEIVE_DATA_1_OUT_OF_4);
			}
		}

		if (!ReaderIsActive && ExpectTagAnswer) {                       // no need to try decoding tag data if the reader is currently sending or no answer expected yet
			if (Handle15693SamplesFromTag(snoopdata >> 2, &DecodeTag)) {
				// FpgaDisableSscDma();
				uint32_t eof_time = dma_start_time + samples*16 - DELAY_TAG_TO_ARM_SNOOP; // end of EOF
				if (DecodeTag.lastBit == SOF_PART2) {
					eof_time -= 8*16; // needed 8 additional samples to confirm single SOF (iCLASS)
				}
				uint32_t sof_time = eof_time
									- DecodeTag.len * 8 * 8 * 16 // time for byte transfers
									- 32 * 16  // time for SOF transfer
									- (DecodeTag.lastBit != SOF_PART2?32*16:0); // time for EOF transfer
				LogTrace_ISO15693(DecodeTag.output, DecodeTag.len, sof_time*4, eof_time*4, NULL, false);
				// And ready to receive another response.
				DecodeTagReset(&DecodeTag);
				DecodeReaderReset(&DecodeReader);
				ExpectTagAnswer = false;
				TagIsActive = false;
				// upTo = dmaBuf;
				// samples = 0;
				// FpgaSetupSscDma((uint8_t*) dmaBuf, ISO15693_DMA_BUFFER_SIZE);
				// continue;
			} else {
				TagIsActive = (DecodeTag.state >= STATE_TAG_RECEIVING_DATA);
			}
		}

	}

	FpgaDisableSscDma();

	DbpString("Snoop statistics:");
	Dbprintf("  ExpectTagAnswer: %d, TagIsActive: %d, ReaderIsActive: %d", ExpectTagAnswer, TagIsActive, ReaderIsActive);
	Dbprintf("  DecodeTag State: %d", DecodeTag.state);
	Dbprintf("  DecodeTag byteCnt: %d", DecodeTag.len);
	Dbprintf("  DecodeTag posCount: %d", DecodeTag.posCount);
	Dbprintf("  DecodeReader State: %d", DecodeReader.state);
	Dbprintf("  DecodeReader byteCnt: %d", DecodeReader.byteCount);
	Dbprintf("  DecodeReader posCount: %d", DecodeReader.posCount);
	Dbprintf("  Trace length: %d", BigBuf_get_traceLen());
	Dbprintf("  Max behindBy: %d", max_behindBy);
}


// Initialize the proxmark as iso15k reader
void Iso15693InitReader(void) {
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);

	// switch field off and wait until tag resets
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();
	SpinDelay(10);

	// switch field on
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER);
	LED_D_ON();
	
	// initialize SSC and select proper AD input
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	// give tags some time to energize
	SpinDelay(250);
}

///////////////////////////////////////////////////////////////////////
// ISO 15693 Part 3 - Air Interface
// This section basically contains transmission and receiving of bits
///////////////////////////////////////////////////////////////////////


// uid is in transmission order (which is reverse of display order)
static void BuildReadBlockRequest(uint8_t *uid, uint8_t blockNumber, uint8_t *cmd) {
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
	crc = Iso15693Crc(cmd, 11); // the crc needs to be calculated over 11 bytes
	cmd[11] = crc & 0xff;
	cmd[12] = crc >> 8;

}


// Now the VICC>VCD responses when we are simulating a tag
static void BuildInventoryResponse(uint8_t *uid) {
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
	crc = Iso15693Crc(cmd, 10);
	cmd[10] = crc & 0xff;
	cmd[11] = crc >> 8;

	CodeIso15693AsTag(cmd, sizeof(cmd));
}

// Universal Method for sending to and recv bytes from a tag
//  init ... should we initialize the reader?
//  speed ... 0 low speed, 1 hi speed
//  *recv will contain the tag's answer
//  return: length of received data, or -1 for timeout
int SendDataTag(uint8_t *send, int sendlen, bool init, bool speed_fast, uint8_t *recv, uint16_t max_recv_len, uint32_t start_time, uint16_t timeout, uint32_t *eof_time) {

	if (init) {
		Iso15693InitReader();
		StartCountSspClk();
	}

	int answerLen = 0;

	if (speed_fast) {
		// high speed (1 out of 4)
		CodeIso15693AsReader(send, sendlen);
	} else {
		// low speed (1 out of 256)
		CodeIso15693AsReader256(send, sendlen);
	}

	TransmitTo15693Tag(ToSend, ToSendMax, &start_time);
	uint32_t end_time = start_time + 32*(8*ToSendMax-4); // substract the 4 padding bits after EOF
	LogTrace_ISO15693(send, sendlen, start_time*4, end_time*4, NULL, true);

	// Now wait for a response
	if (recv != NULL) {
		answerLen = GetIso15693AnswerFromTag(recv, max_recv_len, timeout, eof_time);
	}

	return answerLen;
}


int SendDataTagEOF(uint8_t *recv, uint16_t max_recv_len, uint32_t start_time, uint16_t timeout, uint32_t *eof_time) {

	int answerLen = 0;

	CodeIso15693AsReaderEOF();

	TransmitTo15693Tag(ToSend, ToSendMax, &start_time);
	uint32_t end_time = start_time + 32*(8*ToSendMax-4); // substract the 4 padding bits after EOF
	LogTrace_ISO15693(NULL, 0, start_time*4, end_time*4, NULL, true);

	// Now wait for a response
	if (recv != NULL) {
		answerLen = GetIso15693AnswerFromTag(recv, max_recv_len, timeout, eof_time);
	}

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

		crc=Iso15693Crc(d,len-2);
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
void ReaderIso15693(uint32_t parameter) {

	LED_A_ON();

	set_tracing(true);

	uint8_t TagUID[8] = {0x00};
	uint8_t answer[ISO15693_MAX_RESPONSE_LENGTH];

	// FIRST WE RUN AN INVENTORY TO GET THE TAG UID
	// THIS MEANS WE CAN PRE-BUILD REQUESTS TO SAVE CPU TIME

	// Now send the IDENTIFY command
	uint8_t cmd[5];
	BuildIdentifyRequest(cmd);
	uint32_t start_time = 0;
	uint32_t eof_time;
	int answerLen = SendDataTag(cmd, sizeof(cmd), true, true, answer, sizeof(answer), start_time, ISO15693_READER_TIMEOUT, &eof_time);
	start_time = eof_time + DELAY_ISO15693_VICC_TO_VCD_READER;

	if (answerLen >= 12) { // we should do a better check than this
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

	// read all pages
	if (answerLen >= 12 && DEBUG) {
		for (int i = 0; i < 32; i++) {  // sanity check, assume max 32 pages
			uint8_t cmd[13];
			BuildReadBlockRequest(TagUID, i, cmd);
			answerLen = SendDataTag(cmd, sizeof(cmd), false, true, answer, sizeof(answer), start_time, ISO15693_READER_TIMEOUT, &eof_time);
			start_time = eof_time + DELAY_ISO15693_VICC_TO_VCD_READER;
			if (answerLen > 0) {
				Dbprintf("READ SINGLE BLOCK %d returned %d octets:", i, answerLen);
				DbdecodeIso15693Answer(answerLen, answer);
				Dbhexdump(answerLen, answer, false);
				if ( *((uint32_t*) answer) == 0x07160101 ) break; // exit on NoPageErr
			}
		}
	}

	// for the time being, switch field off to protect RDV4
	// note: this prevents using hf 15 cmd with s option - which isn't implemented yet anyway
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();

	LED_A_OFF();
}


// Initialize the proxmark as iso15k tag
void Iso15693InitTag(void) {
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_NO_MODULATION);
	LED_D_OFF();
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_SIMULATOR);
	StartCountSspClk();
}


// Simulate an ISO15693 TAG.
// For Inventory command: print command and send Inventory Response with given UID
// TODO: interpret other reader commands and send appropriate response
void SimTagIso15693(uint32_t parameter, uint8_t *uid) {

	LED_A_ON();

	Iso15693InitTag();

	// Build a suitable response to the reader INVENTORY command
	BuildInventoryResponse(uid);

	// Listen to reader
	while (!BUTTON_PRESS()) {
		uint8_t cmd[ISO15693_MAX_COMMAND_LENGTH];
		uint32_t eof_time = 0, start_time = 0;
		int cmd_len = GetIso15693CommandFromReader(cmd, sizeof(cmd), &eof_time);

		if ((cmd_len >= 5) && (cmd[0] & ISO15693_REQ_INVENTORY) && (cmd[1] == ISO15693_INVENTORY)) { // TODO: check more flags
			bool slow = !(cmd[0] & ISO15693_REQ_DATARATE_HIGH);
			start_time = eof_time + DELAY_ISO15693_VCD_TO_VICC_SIM;
			TransmitTo15693Reader(ToSend, ToSendMax, &start_time, 0, slow);
		}

		Dbprintf("%d bytes read from reader:", cmd_len);
		Dbhexdump(cmd_len, cmd, false);
	}

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();
	LED_A_OFF();
}


// Since there is no standardized way of reading the AFI out of a tag, we will brute force it
// (some manufactures offer a way to read the AFI, though)
void BruteforceIso15693Afi(uint32_t speed) {
	LED_A_ON();

	uint8_t data[6];
	uint8_t recv[ISO15693_MAX_RESPONSE_LENGTH];
	int datalen = 0, recvlen = 0;
	uint32_t eof_time;

	// first without AFI
	// Tags should respond without AFI and with AFI=0 even when AFI is active

	data[0] = ISO15693_REQ_DATARATE_HIGH | ISO15693_REQ_INVENTORY | ISO15693_REQINV_SLOT1;
	data[1] = ISO15693_INVENTORY;
	data[2] = 0; // mask length
	datalen = Iso15693AddCrc(data,3);
	uint32_t start_time = GetCountSspClk();
	recvlen = SendDataTag(data, datalen, true, speed, recv, sizeof(recv), 0, ISO15693_READER_TIMEOUT, &eof_time);
	start_time = eof_time + DELAY_ISO15693_VICC_TO_VCD_READER;
	WDT_HIT();
	if (recvlen>=12) {
		Dbprintf("NoAFI UID=%s", Iso15693sprintUID(NULL, &recv[2]));
	}

	// now with AFI

	data[0] = ISO15693_REQ_DATARATE_HIGH | ISO15693_REQ_INVENTORY | ISO15693_REQINV_AFI | ISO15693_REQINV_SLOT1;
	data[1] = ISO15693_INVENTORY;
	data[2] = 0; // AFI
	data[3] = 0; // mask length

	for (int i = 0; i < 256; i++) {
		data[2] = i & 0xFF;
		datalen = Iso15693AddCrc(data,4);
		recvlen = SendDataTag(data, datalen, false, speed, recv, sizeof(recv), start_time, ISO15693_READER_TIMEOUT, &eof_time);
		start_time = eof_time + DELAY_ISO15693_VICC_TO_VCD_READER;
		WDT_HIT();
		if (recvlen >= 12) {
			Dbprintf("AFI=%i UID=%s", i, Iso15693sprintUID(NULL, &recv[2]));
		}
	}
	Dbprintf("AFI Bruteforcing done.");

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();
	LED_A_OFF();

}

// Allows to directly send commands to the tag via the client
void DirectTag15693Command(uint32_t datalen, uint32_t speed, uint32_t recv, uint8_t data[]) {

	LED_A_ON();

	int recvlen = 0;
	uint8_t recvbuf[ISO15693_MAX_RESPONSE_LENGTH];
	uint32_t eof_time;

	uint16_t timeout;
    bool request_answer = false;
	
	switch (data[1]) {
		case ISO15693_WRITEBLOCK:
		case ISO15693_LOCKBLOCK:
		case ISO15693_WRITE_MULTI_BLOCK:
		case ISO15693_WRITE_AFI:
		case ISO15693_LOCK_AFI:
		case ISO15693_WRITE_DSFID:
		case ISO15693_LOCK_DSFID:
			timeout = ISO15693_READER_TIMEOUT_WRITE;
			request_answer = data[0] & ISO15693_REQ_OPTION;
			break;
		default:
			timeout = ISO15693_READER_TIMEOUT;
	}		

	if (DEBUG) {
		Dbprintf("SEND:");
		Dbhexdump(datalen, data, false);
	}

	recvlen = SendDataTag(data, datalen, true, speed, (recv?recvbuf:NULL), sizeof(recvbuf), 0, timeout, &eof_time);

	if (request_answer) { // send a single EOF to get the tag response
		recvlen = SendDataTagEOF((recv?recvbuf:NULL), sizeof(recvbuf), 0, ISO15693_READER_TIMEOUT, &eof_time);
	}
	
	// for the time being, switch field off to protect rdv4.0
	// note: this prevents using hf 15 cmd with s option - which isn't implemented yet anyway
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();

	if (recv) {
		if (DEBUG) {
			Dbprintf("RECV:");
			if (recvlen > 0) {
				Dbhexdump(recvlen, recvbuf, false);
				DbdecodeIso15693Answer(recvlen, recvbuf);
			}
		}
		if (recvlen > ISO15693_MAX_RESPONSE_LENGTH) {
			recvlen = ISO15693_MAX_RESPONSE_LENGTH;
		}
		cmd_send(CMD_ACK, recvlen, 0, 0, recvbuf, ISO15693_MAX_RESPONSE_LENGTH);
	}

	LED_A_OFF();
}

//-----------------------------------------------------------------------------
// Work with "magic Chinese" card.
//
//-----------------------------------------------------------------------------

// Set the UID on Magic ISO15693 tag (based on Iceman's LUA-script).
void SetTag15693Uid(uint8_t *uid) {

	LED_A_ON();

	uint8_t cmd[4][9] = {
		{ISO15693_REQ_DATARATE_HIGH, ISO15693_WRITEBLOCK, 0x3e, 0x00, 0x00, 0x00, 0x00},
		{ISO15693_REQ_DATARATE_HIGH, ISO15693_WRITEBLOCK, 0x3f, 0x69, 0x96, 0x00, 0x00},
		{ISO15693_REQ_DATARATE_HIGH, ISO15693_WRITEBLOCK, 0x38},
		{ISO15693_REQ_DATARATE_HIGH, ISO15693_WRITEBLOCK, 0x39}
	};

	uint16_t crc;

	int recvlen = 0;
	uint8_t recvbuf[ISO15693_MAX_RESPONSE_LENGTH];
	uint32_t eof_time;

	// Command 3 : 022138u8u7u6u5 (where uX = uid byte X)
	cmd[2][3] = uid[7];
	cmd[2][4] = uid[6];
	cmd[2][5] = uid[5];
	cmd[2][6] = uid[4];

	// Command 4 : 022139u4u3u2u1 (where uX = uid byte X)
	cmd[3][3] = uid[3];
	cmd[3][4] = uid[2];
	cmd[3][5] = uid[1];
	cmd[3][6] = uid[0];

	uint32_t start_time = 0;
	
	for (int i = 0; i < 4; i++) {
		// Add the CRC
		crc = Iso15693Crc(cmd[i], 7);
		cmd[i][7] = crc & 0xff;
		cmd[i][8] = crc >> 8;

		recvlen = SendDataTag(cmd[i], sizeof(cmd[i]), i==0?true:false, true, recvbuf, sizeof(recvbuf), start_time, ISO15693_READER_TIMEOUT_WRITE, &eof_time);
		start_time = eof_time + DELAY_ISO15693_VICC_TO_VCD_READER;
		if (DEBUG) {
			Dbprintf("SEND:");
			Dbhexdump(sizeof(cmd[i]), cmd[i], false);
			Dbprintf("RECV:");
			if (recvlen > 0) {
				Dbhexdump(recvlen, recvbuf, false);
				DbdecodeIso15693Answer(recvlen, recvbuf);
			}
		}
		// Note: need to know if we expect an answer from one of the magic commands
		// if (recvlen < 0) {
			// break;
		// }
	}

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();

	cmd_send(CMD_ACK, recvlen, 0, 0, recvbuf, recvlen);
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
	crc = Iso15693Crc(cmd, 10); // the crc needs to be calculated over 2 bytes
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
	crc = Iso15693Crc(cmd, 12); // the crc needs to be calculated over 2 bytes
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

//  cmd[12] = 0x00;
//  cmd[13] = 0x00; //Now the CRC
	crc = Iso15693Crc(cmd, 12); // the crc needs to be calculated over 2 bytes
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

//  cmd[12] = 0x00;
//  cmd[13] = 0x00; //Now the CRC
	crc = Iso15693Crc(cmd, 12); // the crc needs to be calculated over 2 bytes
	cmd[12] = crc & 0xff;
	cmd[13] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}




*/


