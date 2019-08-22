//-----------------------------------------------------------------------------
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// HitagS emulation (preliminary test version)
//
// (c) 2016 Oguzhan Cicek, Hendrik Schwartke, Ralf Spenneberg
//     <info@os-s.de>
//-----------------------------------------------------------------------------
// Some code was copied from Hitag2.c
//-----------------------------------------------------------------------------


#include "hitagS.h"

#include <stdlib.h>
#include "proxmark3.h"
#include "apps.h"
#include "util.h"
#include "hitag.h"
#include "string.h"
#include "BigBuf.h"
#include "fpgaloader.h"

#define CRC_PRESET 0xFF
#define CRC_POLYNOM 0x1D

static bool bQuiet;
static bool bSuccessful;
static struct hitagS_tag tag;
static byte_t page_to_be_written = 0;
static int block_data_left = 0;
typedef enum modulation {
	AC2K = 0, AC4K, MC4K, MC8K
} MOD;
static MOD m = AC2K;            //used modulation
static uint32_t temp_uid;
static int temp2 = 0;
static int sof_bits;            //number of start-of-frame bits
static byte_t pwdh0, pwdl0, pwdl1;  //password bytes
static uint32_t rnd = 0x74124485;   //randomnumber
static int test = 0;
size_t blocknr;
bool end=false;

// Single bit Hitag2 functions:
#define i4(x,a,b,c,d)   ((uint32_t)((((x)>>(a))&1)+(((x)>>(b))&1)*2+(((x)>>(c))&1)*4+(((x)>>(d))&1)*8))
static const uint32_t ht2_f4a = 0x2C79;          // 0010 1100 0111 1001
static const uint32_t ht2_f4b = 0x6671;          // 0110 0110 0111 0001
static const uint32_t ht2_f5c = 0x7907287B;          // 0111 1001 0000 0111 0010 1000 0111 1011
#define ht2bs_4a(a,b,c,d)   (~(((a|b)&c)^(a|d)^b))
#define ht2bs_4b(a,b,c,d)   (~(((d|c)&(a^b))^(d|a|b)))
#define ht2bs_5c(a,b,c,d,e) (~((((((c^e)|d)&a)^b)&(c^b))^(((d^e)|a)&((d^b)|c))))
#define uf20bs              uint32_t

static uint32_t f20(const uint64_t x) {
	uint32_t i5;

	i5 = ((ht2_f4a >> i4(x, 1, 2, 4, 5)) & 1) * 1
			+ ((ht2_f4b >> i4(x, 7, 11, 13, 14)) & 1) * 2
			+ ((ht2_f4b >> i4(x, 16, 20, 22, 25)) & 1) * 4
			+ ((ht2_f4b >> i4(x, 27, 28, 30, 32)) & 1) * 8
			+ ((ht2_f4a >> i4(x, 33, 42, 43, 45)) & 1) * 16;

	return (ht2_f5c >> i5) & 1;
}

static uint64_t hitag2_init(const uint64_t key, const uint32_t serial, const uint32_t IV) {
	uint32_t i;
	uint64_t x = ((key & 0xFFFF) << 32) + serial;
	for (i = 0; i < 32; i++) {
		x >>= 1;
		x += (uint64_t) (f20(x) ^ (((IV >> i) ^ (key >> (i + 16))) & 1)) << 47;
	}
	return x;
}

static uint64_t hitag2_round(uint64_t *state) {
	uint64_t x = *state;

	x = (x >> 1)
			+ ((((x >> 0) ^ (x >> 2) ^ (x >> 3) ^ (x >> 6) ^ (x >> 7) ^ (x >> 8)
					^ (x >> 16) ^ (x >> 22) ^ (x >> 23) ^ (x >> 26) ^ (x >> 30)
					^ (x >> 41) ^ (x >> 42) ^ (x >> 43) ^ (x >> 46) ^ (x >> 47))
					& 1) << 47);

	*state = x;
	return f20(x);
}

static uint32_t hitag2_byte(uint64_t *x) {
	uint32_t i, c;

	for (i = 0, c = 0; i < 8; i++)
		c += (uint32_t) hitag2_round(x) << (i ^ 7);
	return c;
}

// Sam7s has several timers, we will use the source TIMER_CLOCK1 (aka AT91C_TC_CLKS_TIMER_DIV1_CLOCK)
// TIMER_CLOCK1 = MCK/2, MCK is running at 48 MHz, Timer is running at 48/2 = 24 MHz
// Hitag units (T0) have duration of 8 microseconds (us), which is 1/125000 per second (carrier)
// T0 = TIMER_CLOCK1 / 125000 = 192
#define T0                          192

#define SHORT_COIL()                        LOW(GPIO_SSC_DOUT)
#define OPEN_COIL()                         HIGH(GPIO_SSC_DOUT)

#define HITAG_FRAME_LEN                     20
#define HITAG_T_STOP                        36  /* T_EOF should be > 36 */
#define HITAG_T_LOW                     8   /* T_LOW should be 4..10 */
#define HITAG_T_0_MIN                       15  /* T[0] should be 18..22 */
#define HITAG_T_1_MIN                       25  /* T[1] should be 26..30 */
//#define HITAG_T_EOF   40 /* T_EOF should be > 36 */
#define HITAG_T_EOF                         80   /* T_EOF should be > 36 */
#define HITAG_T_WAIT_1                      200  /* T_wresp should be 199..206 */
#define HITAG_T_WAIT_2                      90   /* T_wresp should be 199..206 */
#define HITAG_T_WAIT_MAX                    300  /* bit more than HITAG_T_WAIT_1 + HITAG_T_WAIT_2 */

#define HITAG_T_TAG_ONE_HALF_PERIOD             10
#define HITAG_T_TAG_TWO_HALF_PERIOD             25
#define HITAG_T_TAG_THREE_HALF_PERIOD               41
#define HITAG_T_TAG_FOUR_HALF_PERIOD                57

#define HITAG_T_TAG_HALF_PERIOD                 16
#define HITAG_T_TAG_FULL_PERIOD                 32

#define HITAG_T_TAG_CAPTURE_ONE_HALF                13
#define HITAG_T_TAG_CAPTURE_TWO_HALF                25
#define HITAG_T_TAG_CAPTURE_THREE_HALF              41
#define HITAG_T_TAG_CAPTURE_FOUR_HALF               57

#define DEBUG 0

/*
 * Implementation of the crc8 calculation from Hitag S
 * from http://www.proxmark.org/files/Documents/125%20kHz%20-%20Hitag/HitagS.V11.pdf
 */
void calc_crc(unsigned char * crc, unsigned char data, unsigned char Bitcount) {
	*crc ^= data; // crc = crc (exor) data
	do {
		if (*crc & 0x80) // if (MSB-CRC == 1)
				{
			*crc <<= 1; // CRC = CRC Bit-shift left
			*crc ^= CRC_POLYNOM; // CRC = CRC (exor) CRC_POLYNOM
		} else {
			*crc <<= 1; // CRC = CRC Bit-shift left
		}
	} while (--Bitcount);
}


static void hitag_send_bit(int bit) {
	LED_A_ON();
	// Reset clock for the next bit
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;

	switch (m) {
		case AC2K:
			if (bit == 0) {
				// AC Coding --__
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 32)
					;
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 64)
					;
			} else {
				// AC coding -_-_
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 16)
					;
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 32)
					;
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 48)
					;
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 64)
					;;
			}
			LED_A_OFF();
			break;
		case AC4K:
			if (bit == 0) {
				// AC Coding --__
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * HITAG_T_TAG_HALF_PERIOD)
					;
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * HITAG_T_TAG_FULL_PERIOD)
					;
			} else {
				// AC coding -_-_
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 8)
					;
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 16)
					;
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 24)
					;
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 32)
					;
			}
			LED_A_OFF();
			break;
		case MC4K:
			if (bit == 0) {
				// Manchester: Unloaded, then loaded |__--|
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 16)
					;
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 32)
					;
			} else {
				// Manchester: Loaded, then unloaded |--__|
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 16)
					;
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 32)
					;
			}
			LED_A_OFF();
			break;
		case MC8K:
			if (bit == 0) {
				// Manchester: Unloaded, then loaded |__--|
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 8)
					;
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 16)
					;
			} else {
				// Manchester: Loaded, then unloaded |--__|
				HIGH(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 8)
					;
				LOW(GPIO_SSC_DOUT);
				while (AT91C_BASE_TC0->TC_CV < T0 * 16)
					;
			}
			LED_A_OFF();
			break;
		default:
			break;
	}
}

static void hitag_tag_send_frame(const byte_t* frame, size_t frame_len) {
// Send start of frame
	for (size_t i = 0; i < sof_bits; i++) {
		hitag_send_bit(1);
	}

// Send the content of the frame
	for (size_t i = 0; i < frame_len; i++) {
		hitag_send_bit((frame[i / 8] >> (7 - (i % 8))) & 1);
	}
// Drop the modulation
	LOW(GPIO_SSC_DOUT);
}

static void hitag_reader_send_bit(int bit) {
//Dbprintf("BIT: %d",bit);
	LED_A_ON();
// Reset clock for the next bit
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;

// Binary puls length modulation (BPLM) is used to encode the data stream
// This means that a transmission of a one takes longer than that of a zero

// Enable modulation, which means, drop the the field
	HIGH(GPIO_SSC_DOUT);
	if (test == 1) {
		// Wait for 4-10 times the carrier period
		while (AT91C_BASE_TC0->TC_CV < T0 * 6)
			;
		//  SpinDelayUs(8*8);

		// Disable modulation, just activates the field again
		LOW(GPIO_SSC_DOUT);

		if (bit == 0) {
			// Zero bit: |_-|
			while (AT91C_BASE_TC0->TC_CV < T0 * 11)
				;
			//      SpinDelayUs(16*8);
		} else {
			// One bit: |_--|
			while (AT91C_BASE_TC0->TC_CV < T0 * 14)
				;
			//      SpinDelayUs(22*8);
		}
	} else {
		// Wait for 4-10 times the carrier period
		while (AT91C_BASE_TC0->TC_CV < T0 * 6)
			;
		//  SpinDelayUs(8*8);

		// Disable modulation, just activates the field again
		LOW(GPIO_SSC_DOUT);

		if (bit == 0) {
			// Zero bit: |_-|
			while (AT91C_BASE_TC0->TC_CV < T0 * 22)
				;
			//      SpinDelayUs(16*8);
		} else {
			// One bit: |_--|
			while (AT91C_BASE_TC0->TC_CV < T0 * 28)
				;
			//      SpinDelayUs(22*8);
		}
	}

	LED_A_OFF();
}

static void hitag_reader_send_frame(const byte_t* frame, size_t frame_len) {
	// Send the content of the frame
	for (size_t i = 0; i < frame_len; i++) {
		if (frame[0] == 0xf8) {
			//Dbprintf("BIT: %d",(frame[i / 8] >> (7 - (i % 8))) & 1);
		}
		hitag_reader_send_bit(((frame[i / 8] >> (7 - (i % 8))) & 1));
	}
// Send EOF
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;
// Enable modulation, which means, drop the the field
	HIGH(GPIO_SSC_DOUT);
// Wait for 4-10 times the carrier period
	while (AT91C_BASE_TC0->TC_CV < T0 * 6)
		;
// Disable modulation, just activates the field again
	LOW(GPIO_SSC_DOUT);
}

static void hitag_decode_frame_MC(int bitRate, int sofBits, byte_t* rx, size_t* rxlenOrg, int* response, int rawMod[], int rawLen) {
	size_t rxlen = 0;
	bool bSkip = true;
	int lastbit = 1;
	int tag_sof = 0;
	int timing = 1;
	if (bitRate == 8) {
		timing = 2;
	}

	for (int i=0; i < rawLen; i++) {
		int ra = rawMod[i];
		if (ra >= HITAG_T_EOF) {
			if (rxlen != 0) {
				//DbpString("wierd1?");
			}
			tag_sof = sofBits;

			// Capture the T0 periods that have passed since last communication or field drop (reset)
			// We always recieve a 'one' first, which has the falling edge after a half period |-_|
			*response = ra - HITAG_T_TAG_HALF_PERIOD;
		} else if (ra >= HITAG_T_TAG_CAPTURE_FOUR_HALF / timing) {
			tag_sof=0;
			// Manchester coding example |-_|_-|-_| (101)
			rx[rxlen / 8] |= 0 << (7 - (rxlen % 8));
			rxlen++;
			rx[rxlen / 8] |= 1 << (7 - (rxlen % 8));
			rxlen++;
		} else if (ra >= HITAG_T_TAG_CAPTURE_THREE_HALF / timing) {
			tag_sof=0;
			// Manchester coding example |_-|...|_-|-_| (0...01)
			rx[rxlen / 8] |= 0 << (7 - (rxlen % 8));
			rxlen++;
			// We have to skip this half period at start and add the 'one' the second time
			if (!bSkip) {
				rx[rxlen / 8] |= 1 << (7 - (rxlen % 8));
				rxlen++;
			}
			lastbit = !lastbit;
			bSkip = !bSkip;
		} else if (ra >= HITAG_T_TAG_CAPTURE_TWO_HALF / timing) {
			// Manchester coding example |_-|_-| (00) or |-_|-_| (11)
			if (tag_sof) {
				// Ignore bits that are transmitted during SOF
				tag_sof--;
			} else {
				// bit is same as last bit
				rx[rxlen / 8] |= lastbit << (7 - (rxlen % 8));
				rxlen++;
			}
		} else {
			// Ignore wierd value, is to small to mean anything
		}
	}
	*rxlenOrg = rxlen;
}

/*
static void hitag_decode_frame_AC2K_rising(byte_t* rx, size_t* rxlenOrg, int* response, int rawMod[], int rawLen) {
	int tag_sof = 1; //skip start of frame
	size_t rxlen = 0;

	for (int i=0; i < rawLen; i++) {
		int ra = rawMod[i];
		if (ra >= HITAG_T_EOF) {
			if (rxlen != 0) {
				//DbpString("wierd1?");
			}
			// Capture the T0 periods that have passed since last communication or field drop (reset)
			// We always recieve a 'one' first, which has the falling edge after a half period |-_|
			tag_sof = 1;
			*response = ra - HITAG_T_TAG_HALF_PERIOD;
		} else if (ra >= HITAG_T_TAG_CAPTURE_FOUR_HALF) {
			// AC coding example |--__|--__| means 0
			rx[rxlen / 8] |= 0 << (7 - (rxlen % 8));
			rxlen++;
			if (rawMod[i+1] == 0) { //TODO: this is weird - may we miss one capture with current configuration
				rx[rxlen / 8] |= 0 << (7 - (rxlen % 8));
				rxlen++;
				i++; //drop next capture
			}
		} else if (ra >= HITAG_T_TAG_CAPTURE_TWO_HALF) {
			if (tag_sof) {
				// Ignore bits that are transmitted during SOF
				tag_sof--;
			} else {
				// AC coding example |-_-_|-_-_| which means 1
				//check if another high is coming (only -_-_ = 1) except end of the frame (support 0)
				if (rawMod[i+1] == 0 || rawMod[i+1] >= HITAG_T_TAG_CAPTURE_TWO_HALF) {
					rx[rxlen / 8] |= 1 << (7 - (rxlen % 8));
					rxlen++;
					i++; //drop next capture
				} else {
					Dbprintf("got weird high - %d,%d", ra, rawMod[i+1]);
				}
			}
		} else {
			// Ignore wierd value, is to small to mean anything
		}
	}
	*rxlenOrg = rxlen;
}
*/

static void hitag_decode_frame_AC(int bitRate, int sofBits, byte_t* rx, size_t* rxlenOrg, int* response, int rawMod[], int rawLen) {
	int tag_sof = 1;
	size_t rxlen = 0;
	int timing = 1;
	if (bitRate == 4) {
		timing = 2;
	}


	for (int i=0; i < rawLen; i++) {
		int ra = rawMod[i];
		if (ra >= HITAG_T_EOF) {
			if (rxlen != 0) {
				//DbpString("wierd1?");
			}

			// Capture the T0 periods that have passed since last communication or field drop (reset)
			// We always recieve a 'one' first, which has the falling edge after a half period |-_|
			tag_sof = sofBits;
			*response = ra - HITAG_T_TAG_HALF_PERIOD;
		} else if (ra >= HITAG_T_TAG_CAPTURE_FOUR_HALF / timing) {
			tag_sof=0;

			// AC coding example |--__|--__| means 0
			rx[rxlen / 8] |= 0 << (7 - (rxlen % 8));
			rxlen++;
		} else if (ra >= HITAG_T_TAG_CAPTURE_THREE_HALF / timing) {
			tag_sof=0;

			if (rawMod[i-1] >= HITAG_T_TAG_CAPTURE_THREE_HALF / timing) {
				//treat like HITAG_T_TAG_CAPTURE_TWO_HALF
				if (rawMod[i+1] >= HITAG_T_TAG_CAPTURE_TWO_HALF / timing) {
					rx[rxlen / 8] |= 1 << (7 - (rxlen % 8));
					rxlen++;
					i++; //drop next capture
				} else {
					Dbprintf("got weird value - %d,%d", ra, rawMod[i+1]);
				}
			} else {
				//treat like HITAG_T_TAG_CAPTURE_FOUR_HALF
				rx[rxlen / 8] |= 0 << (7 - (rxlen % 8));
				rxlen++;
			}
		} else if (ra >= HITAG_T_TAG_CAPTURE_TWO_HALF / timing) {
			if (tag_sof) {
				// Ignore bits that are transmitted during SOF
				tag_sof--;
			} else {
				// AC coding example |-_-_|-_-_| which means 1
				//check if another high is coming (only -_-_ = 1) except end of the frame (support 0)
				if (rawMod[i+1] == 0 || rawMod[i+1] >= HITAG_T_TAG_CAPTURE_TWO_HALF / timing) {
					rx[rxlen / 8] |= 1 << (7 - (rxlen % 8));
					rxlen++;
					i++; //drop next capture
				} else {
					Dbprintf("got weird value - %d,%d", ra, rawMod[i+1]);
				}
			}
		} else {
			// Ignore wierd value, is to small to mean anything
		}
	}
	*rxlenOrg = rxlen;
}

static void hitag_receive_frame(byte_t* rx, size_t* rxlen, int* response) {
	int rawMod[200] = {0};
	int rawLen = 0;
	int i = 0;
	int sofBits = 0;

	m = MC4K;
	if (tag.pstate == READY) {
		switch (tag.mode) {
			case STANDARD:
				m = AC2K;
				sofBits = 1;
				break;
			case ADVANCED:
				m = AC2K;
				sofBits = 5; //3 sof bits but 5 captures
				break;
			case FAST_ADVANCED:
				m = AC4K;
				sofBits = 5; //3 sof bits but 5 captures
				break;
			default:
				break;
		}
	} else {
		switch (tag.mode) {
			case STANDARD:
				m = MC4K;
				sofBits = 0; //in theory 1
				break;
			case ADVANCED:
				m = MC4K;
				sofBits = 5; //in theory 6
				break;
			case FAST_ADVANCED:
				m = MC8K;
				sofBits = 5; //in theory 6
				break;
			default:
				break;
		}
	}

	//rising AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK | AT91C_TC_ETRGEDG_RISING | AT91C_TC_ABETRG | AT91C_TC_LDRA_RISING;
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK | AT91C_TC_ETRGEDG_FALLING | AT91C_TC_ABETRG | AT91C_TC_LDRA_FALLING;


	//first capture timing values
	while (AT91C_BASE_TC1->TC_CV < T0 * HITAG_T_WAIT_MAX ) {
		// Check if rising edge in modulation is detected
		if (AT91C_BASE_TC1->TC_SR & AT91C_TC_LDRAS) {
			// Retrieve the new timing values
			int ra = (AT91C_BASE_TC1->TC_RA / T0);

			LED_B_ON();
			// Reset timer every frame, we have to capture the last edge for timing
			AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
			//AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;

			if (rawLen >= 200) { //avoid exception
				break;
			}
			rawMod[rawLen] = ra;
			rawLen++;

			// We can break this loop if we received the last bit from a frame
			if (AT91C_BASE_TC1->TC_CV > T0 * HITAG_T_EOF) {
				if (rawLen > 2) {
					if (DEBUG >= 2) { Dbprintf("AT91C_BASE_TC1->TC_CV > T0 * HITAG_T_EOF breaking (%d)", rawLen); }
					break;
				}
			}
		}
	}

	if (DEBUG >= 2) {
		for (i=0; i < rawLen; i+=20) {
			Dbprintf("raw modulation: - %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				rawMod[i],rawMod[i+1],rawMod[i+2],rawMod[i+3], rawMod[i+4],rawMod[i+5],rawMod[i+6],rawMod[i+7],
				rawMod[i+8],rawMod[i+9],rawMod[i+10],rawMod[i+11], rawMod[i+12],rawMod[i+13],rawMod[i+14],rawMod[i+15],
				rawMod[i+16],rawMod[i+17],rawMod[i+18],rawMod[i+19]
			);
		}
	}

	switch (m) {
		// DATA           |  1 |  0 |  1 |  1 |  0 |
		// Manchester     |--__|__--|--__|--__|__--|
		// Anti Collision |-_-_|--__|-_-_|-_-_|--__|
		//                |<-->|
		//                | T  |
		case AC2K:
			if (DEBUG >= 2) { Dbprintf("decoding frame with modulation AC2K"); }
			hitag_decode_frame_AC(2, sofBits, rx, rxlen, response, rawMod, rawLen);
			break;
		case AC4K:
			if (DEBUG >= 2) { Dbprintf("decoding frame with modulation AC4K"); }
			hitag_decode_frame_AC(4, sofBits, rx, rxlen, response, rawMod, rawLen);
			break;
		case MC4K:
			if (DEBUG >= 2) { Dbprintf("decoding frame with modulation MC4K"); }
			hitag_decode_frame_MC(4, sofBits, rx, rxlen, response, rawMod, rawLen);
			break;
		case MC8K:
			if (DEBUG >= 2) { Dbprintf("decoding frame with modulation MC8K"); }
			hitag_decode_frame_MC(8, sofBits, rx, rxlen, response, rawMod, rawLen);
			break;
	}

	LED_B_OFF();
	if (DEBUG >= 2) {
		int rb[200] = {0}; int z = 0;
		for (i = 0; i < 16; i++) { for (int j = 0; j < 8; j++) {
			rb[z] = 0;
			if ((rx[i] & ((1 << 7) >> j)) != 0) { rb[z] = 1; }
			z++;
		} }
		for (i=0; i < z; i+=8) {
			Dbprintf("raw bit: - %d%d%d%d%d%d%d%d", rb[i],rb[i+1],rb[i+2],rb[i+3],rb[i+4],rb[i+5],rb[i+6],rb[i+7] );
		}
	}
}

static void hitag_start_auth(byte_t* tx, size_t* txlen) {
	*txlen = 5;
	switch (tag.mode) {
		case STANDARD:
			//00110 - 0x30 - STANDARD MODE
			memcpy(tx, "\x30", nbytes(*txlen));
			break;
		case ADVANCED:
			//11000 - 0xc0 - Advance Mode
			memcpy(tx, "\xc0", nbytes(*txlen));
			break;
		case FAST_ADVANCED:
			//TODO!
			break;
		default: //STANDARD MODE
			memcpy(tx, "\x30", nbytes(*txlen));
			break;
	}
	tag.pstate = READY;
	tag.tstate = NO_OP;
}

static int hitag_read_page(hitag_function htf, uint64_t key, byte_t* rx, size_t* rxlen, byte_t* tx, size_t* txlen, int pageNum) {
	int i, j, z;
	int response_bit[200];
	unsigned char mask = 1;
	unsigned char crc;
	unsigned char pageData[32];

	if (pageNum >= tag.max_page) {
		return -1;
	}
	if (tag.pstate == SELECTED && tag.tstate == NO_OP && *rxlen > 0) {
		//send read request
		tag.tstate = READING_PAGE;
		*txlen = 20;
		crc = CRC_PRESET;
		tx[0] = 0xc0 + (pageNum / 16);
		calc_crc(&crc, tx[0], 8);
		calc_crc(&crc, 0x00 + ((pageNum % 16) * 16), 4);
		tx[1] = 0x00 + ((pageNum % 16) * 16) + (crc / 16);
		tx[2] = 0x00 + (crc % 16) * 16;
	} else if (tag.pstate == SELECTED && tag.tstate == READING_PAGE && *rxlen > 0) {
		//save received data
		z = 0;
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 8; j++) {
				response_bit[z] = 0;
				if ((rx[i] & ((mask << 7) >> j)) != 0) {
					response_bit[z] = 1;
				}
				if (z < 32) {
					pageData[z] = response_bit[z];
				}

				z++;
			}
		}
		for (i = 0; i < 4; i++) {
			tag.pages[pageNum][i] = 0x0;
		}
		for (i = 0; i < 4; i++) {
			tag.pages[pageNum][i] += ((pageData[i * 8] << 7) | (pageData[1 + (i * 8)] << 6) |
					(pageData[2 + (i * 8)] << 5) | (pageData[3 + (i * 8)] << 4) |
					(pageData[4 + (i * 8)] << 3) | (pageData[5 + (i * 8)] << 2) |
					(pageData[6 + (i * 8)]
					 << 1) | pageData[7 + (i * 8)]);
		}
		if (tag.auth && tag.LKP && pageNum == 1) {
			Dbprintf("Page[%2d]: %02X %02X %02X %02X", pageNum, pwdh0,
					tag.pages[pageNum][2], tag.pages[pageNum][1], tag.pages[pageNum][0]);
		} else {
			Dbprintf("Page[%2d]: %02X %02X %02X %02X", pageNum,
					tag.pages[pageNum][3], tag.pages[pageNum][2],
					tag.pages[pageNum][1], tag.pages[pageNum][0]);
		}


		//display key and password if possible
		if (pageNum == 1 && tag.auth == 1 && tag.LKP) {
			if (htf == 02) { //RHTS_KEY
				Dbprintf("Page[ 2]: %02X %02X %02X %02X",
						(byte_t)(key >> 8) & 0xff,
						(byte_t) key & 0xff, pwdl1, pwdl0);
				Dbprintf("Page[ 3]: %02X %02X %02X %02X",
						(byte_t)(key >> 40) & 0xff,
						(byte_t)(key >> 32) & 0xff,
						(byte_t)(key >> 24) & 0xff,
						(byte_t)(key >> 16) & 0xff);
			} else {
				//if the authentication is done with a challenge the key and password are unknown
				Dbprintf("Page[ 2]: __ __ __ __");
				Dbprintf("Page[ 3]: __ __ __ __");
			}
		}

		*txlen = 20;
		crc = CRC_PRESET;
		tx[0] = 0xc0 + ((pageNum+1) / 16);
		calc_crc(&crc, tx[0], 8);
		calc_crc(&crc, 0x00 + (((pageNum+1) % 16) * 16), 4);
		tx[1] = 0x00 + (((pageNum+1) % 16) * 16) + (crc / 16);
		tx[2] = 0x00 + (crc % 16) * 16;

		return 1;
	}
	return 0;
}

static int hitag_read_block(hitag_function htf, uint64_t key, byte_t* rx, size_t* rxlen, byte_t* tx, size_t* txlen, int blockNum) {
	int i, j, z;
	int response_bit[200];
	unsigned char mask = 1;
	unsigned char crc;
	unsigned char blockData[128];

	if (blockNum+4 >= tag.max_page) { //block always = 4 pages
		return -1;
	}

	if (tag.pstate == SELECTED && tag.tstate == NO_OP && *rxlen > 0) {
		//send read request
		tag.tstate = READING_BLOCK;
		*txlen = 20;
		crc = CRC_PRESET;
		tx[0] = 0xd0 + (blockNum / 16);
		calc_crc(&crc, tx[0], 8);
		calc_crc(&crc, 0x00 + ((blockNum % 16) * 16), 4);
		tx[1] = 0x00 + ((blockNum % 16) * 16) + (crc / 16);
		tx[2] = 0x00 + (crc % 16) * 16;
	} else if (tag.pstate == SELECTED && tag.tstate == READING_BLOCK && *rxlen > 0) {
		//save received data
		z = 0;
		for (i = 0; i < 16; i++) {
			for (j = 0; j < 8; j++) {
				response_bit[z] = 0;
				if ((rx[i] & ((mask << 7) >> j)) != 0) {
					response_bit[z] = 1;
				}
				if (z < 128) {
					blockData[z] = response_bit[z];
				}
				z++;
			}
		}

		for (z = 0; z < 4; z++) { //4 pages
			for (i = 0; i < 4; i++) {
				tag.pages[blockNum+z][i] = 0x0;
			}
		}
		for (z = 0; z < 4; z++) { //4 pages
			for (i = 0; i < 4; i++) {
				j = (i * 8) + (z*32); //bit in page + pageStart
				tag.pages[blockNum+z][i] = ((blockData[j] << 7) | (blockData[1 + j] << 6) |
					(blockData[2 + j] << 5) | (blockData[3 + j] << 4) |
					(blockData[4 + j] << 3) | (blockData[5 + j] << 2) |
					(blockData[6 + j] << 1) | blockData[7 + j]);
			}
		}
		if (DEBUG) {
			for (z = 0; z < 4; z++) {
				Dbprintf("Page[%2d]: %02X %02X %02X %02X", blockNum+z,
						tag.pages[blockNum+z][3], tag.pages[blockNum+z][2],
						tag.pages[blockNum+z][1], tag.pages[blockNum+z][0]);
			}
		}
		Dbprintf("Block[%2d]: %02X %02X %02X %02X - %02X %02X %02X %02X - %02X %02X %02X %02X - %02X %02X %02X %02X", blockNum,
					tag.pages[blockNum][3],   tag.pages[blockNum][2],    tag.pages[blockNum][1],   tag.pages[blockNum][0],
					tag.pages[blockNum+1][3], tag.pages[blockNum+1][2],  tag.pages[blockNum+1][1], tag.pages[blockNum+1][0],
					tag.pages[blockNum+2][3], tag.pages[blockNum+2][2],  tag.pages[blockNum+2][1], tag.pages[blockNum+2][0],
					tag.pages[blockNum+3][3], tag.pages[blockNum+3][2],  tag.pages[blockNum+3][1], tag.pages[blockNum+3][0]);

		*txlen = 20;
		crc = CRC_PRESET;
		tx[0] = 0xd0 + ((blockNum+4) / 16);
		calc_crc(&crc, tx[0], 8);
		calc_crc(&crc, 0x00 + (((blockNum+4) % 16) * 16), 4);
		tx[1] = 0x00 + (((blockNum+4) % 16) * 16) + (crc / 16);
		tx[2] = 0x00 + (crc % 16) * 16;

		return 1;
	}
	return 0;
}


/*
 * to check if the right uid was selected
 */
static int check_select(byte_t* rx, uint32_t uid) {
	unsigned char resp[48];
	int i;
	uint32_t ans = 0x0;
	for (i = 0; i < 48; i++)
		resp[i] = (rx[i / 8] >> (7 - (i % 8))) & 0x1;
	for (i = 0; i < 32; i++)
		ans += resp[5 + i] << (31 - i);
	/*if (rx[0] == 0x01 && rx[1] == 0x15 && rx[2] == 0xc1 && rx[3] == 0x14
	 && rx[4] == 0x65 && rx[5] == 0x38)
	 Dbprintf("got uid %X", ans);*/
	temp_uid = ans;
	if (ans == tag.uid)
		return 1;
	return 0;
}

/*
 * handles all commands from a reader
 */
static void hitagS_handle_reader_command(byte_t* rx, const size_t rxlen,
		byte_t* tx, size_t* txlen) {
	byte_t rx_air[HITAG_FRAME_LEN];
	byte_t page;
	int i;
	uint64_t state;
	unsigned char crc;

	// Copy the (original) received frame how it is send over the air
	memcpy(rx_air, rx, nbytes(rxlen));
	// Reset the transmission frame length
	*txlen = 0;
	// Try to find out which command was send by selecting on length (in bits)
	switch (rxlen) {
		case 5: {
			//UID request with a selected response protocol mode
			tag.pstate = READY;
			tag.tstate = NO_OP;
			if ((rx[0] & 0xf0) == 0x30) {
				Dbprintf("recieved uid request in Standard Mode");
				tag.mode = STANDARD;
				sof_bits = 1;
				m = AC2K;
			}
			if ((rx[0] & 0xf0) == 0xc0) {
				Dbprintf("recieved uid request in ADVANCE Mode");
				tag.mode = ADVANCED;
				sof_bits = 3;
				m = AC2K;
			}
			if ((rx[0] & 0xf0) == 0xd0) {
				Dbprintf("recieved uid request in FAST_ADVANCE Mode");
				tag.mode = FAST_ADVANCED;
				sof_bits = 3;
				m = AC4K;
			}
			//send uid as a response
			*txlen = 32;
			for (i = 0; i < 4; i++) {
				tx[i] = (tag.uid >> (24 - (i * 8))) & 0xff;
			}
		}
		break;
		case 45: {
			//select command from reader received
			if (check_select(rx, tag.uid) == 1) {
				//if the right tag was selected
				*txlen = 32;
				switch (tag.mode) {
					case STANDARD:
						Dbprintf("uid selected in Standard Mode");
						sof_bits = 1;
						m = MC4K;
						break;
					case ADVANCED:
						Dbprintf("uid selected in ADVANCE Mode");
						sof_bits = 6;
						m = MC4K;
						break;
					case FAST_ADVANCED:
						Dbprintf("uid selected in FAST_ADVANCE Mode");
						sof_bits = 6;
						m = MC8K;
						break;
					default:
						break;
				}

				//send configuration
				tx[0] = tag.pages[1][3];
				tx[1] = tag.pages[1][2];
				tx[2] = tag.pages[1][1];
				tx[3] = 0xff;
				if (tag.mode != STANDARD) {
					*txlen = 40;
					crc = CRC_PRESET;
					for (i = 0; i < 4; i++)
						calc_crc(&crc, tx[i], 8);
					tx[4] = crc;
				}
			}
		}
		break;
	case 64: {
		switch (tag.mode) {
			case STANDARD:
				sof_bits = 1;
				m = MC4K;
				break;
			case ADVANCED:
				sof_bits = 6;
				m = MC4K;
				break;
			case FAST_ADVANCED:
				sof_bits = 6;
				m = MC8K;
				break;
			default:
				break;
		}
		//challenge message received
		Dbprintf("Challenge for UID: %X", temp_uid);
		temp2++;
		state = hitag2_init(REV64(tag.key), REV32(tag.pages[0][0]),
				REV32(((rx[3] << 24) + (rx[2] << 16) + (rx[1] << 8) + rx[0])));
		Dbprintf(
				",{0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}",
				rx[0], rx[1], rx[2], rx[3], rx[4], rx[5], rx[6], rx[7]);

		for (i = 0; i < 4; i++) {
			hitag2_byte(&state);
		}

		*txlen = 32;
		//send con2,pwdh0,pwdl0,pwdl1 encrypted as a response
		tx[0] = hitag2_byte(&state) ^ tag.pages[1][1];
		tx[1] = hitag2_byte(&state) ^ tag.pwdh0;
		tx[2] = hitag2_byte(&state) ^ tag.pwdl0;
		tx[3] = hitag2_byte(&state) ^ tag.pwdl1;
		if (tag.mode != STANDARD) {
			//add crc8
			*txlen = 40;
			crc = CRC_PRESET;
			calc_crc(&crc, tag.pages[1][1], 8);
			calc_crc(&crc, tag.pwdh0, 8);
			calc_crc(&crc, tag.pwdl0, 8);
			calc_crc(&crc, tag.pwdl1, 8);
			tx[4] = (crc ^ hitag2_byte(&state));
		}
	}
	case 40:
		//data received to be written
		if (tag.tstate == WRITING_PAGE_DATA) {
			tag.tstate = NO_OP;
			tag.pages[page_to_be_written][0] = rx[3];
			tag.pages[page_to_be_written][1] = rx[2];
			tag.pages[page_to_be_written][2] = rx[1];
			tag.pages[page_to_be_written][3] = rx[0];

			//send ack
			*txlen = 2;
			tx[0] = 0x40;
			page_to_be_written = 0;
			switch (tag.mode) {
			case STANDARD:
				sof_bits = 1;
				m = MC4K;
				break;
			case ADVANCED:
				sof_bits = 6;
				m = MC4K;
				break;
			case FAST_ADVANCED:
				sof_bits = 6;
				m = MC8K;
				break;
			default:
				break;
			}
		} else if (tag.tstate == WRITING_BLOCK_DATA) {
			tag.pages[page_to_be_written][0] = rx[0];
			tag.pages[page_to_be_written][1] = rx[1];
			tag.pages[page_to_be_written][2] = rx[2];
			tag.pages[page_to_be_written][3] = rx[3];

			//send ack
			*txlen = 2;
			tx[0] = 0x40;
			switch (tag.mode) {
			case STANDARD:
				sof_bits = 1;
				m = MC4K;
				break;
			case ADVANCED:
				sof_bits = 6;
				m = MC4K;
				break;
			case FAST_ADVANCED:
				sof_bits = 6;
				m = MC8K;
				break;
			default:
				break;
			}
			page_to_be_written++;
			block_data_left--;
			if (block_data_left == 0) {
				tag.tstate = NO_OP;
				page_to_be_written = 0;
			}
		}
		break;
	case 20: {
		//write page, write block, read page or read block command received
		if ((rx[0] & 0xf0) == 0xc0) { //read page
			//send page data
			page = ((rx[0] & 0x0f) * 16) + ((rx[1] & 0xf0) / 16);
			Dbprintf("reading page %d", page);
			*txlen = 32;
			tx[0] = tag.pages[page][0];
			tx[1] = tag.pages[page][1];
			tx[2] = tag.pages[page][2];
			tx[3] = tag.pages[page][3];

			if (tag.LKP && page == 1)
				tx[3] = 0xff;

			switch (tag.mode) {
			case STANDARD:
				sof_bits = 1;
				m = MC4K;
				break;
			case ADVANCED:
				sof_bits = 6;
				m = MC4K;
				break;
			case FAST_ADVANCED:
				sof_bits = 6;
				m = MC8K;
				break;
			default:
				break;
			}

			if (tag.mode != STANDARD) {
				//add crc8
				*txlen = 40;
				crc = CRC_PRESET;
				for (i = 0; i < 4; i++)
					calc_crc(&crc, tx[i], 8);
				tx[4] = crc;
			}

			if (tag.LKP && (page == 2 || page == 3)) {
				//if reader asks for key or password and the LKP-mark is set do not respond
				sof_bits = 0;
				*txlen = 0;
			}
		} else if ((rx[0] & 0xf0) == 0xd0) { //read block
			page = ((rx[0] & 0x0f) * 16) + ((rx[1] & 0xf0) / 16);
			Dbprintf("reading block %d", page);
			*txlen = 32 * 4;
			//send page,...,page+3 data
			for (i = 0; i < 4; i++) {
				tx[0 + (i * 4)] = tag.pages[page][0];
				tx[1 + (i * 4)] = tag.pages[page][1];
				tx[2 + (i * 4)] = tag.pages[page][2];
				tx[3 + (i * 4)] = tag.pages[page][3];
				page++;
			}

			switch (tag.mode) {
				case STANDARD:
					sof_bits = 1;
					m = MC4K;
					break;
				case ADVANCED:
					sof_bits = 6;
					m = MC4K;
					break;
				case FAST_ADVANCED:
					sof_bits = 6;
					m = MC8K;
					break;
				default:
					break;
			}

			if (tag.mode != STANDARD) {
				//add crc8
				*txlen = 32 * 4 + 8;
				crc = CRC_PRESET;
				for (i = 0; i < 16; i++)
					calc_crc(&crc, tx[i], 8);
				tx[16] = crc;
			}

			if ((page - 4) % 4 != 0 || (tag.LKP && (page - 4) == 0)) {
				sof_bits = 0;
				*txlen = 0;
			}
		} else if ((rx[0] & 0xf0) == 0x80) { //write page
			page = ((rx[0] & 0x0f) * 16) + ((rx[1] & 0xf0) / 16);

			switch (tag.mode) {
			case STANDARD:
				sof_bits = 1;
				m = MC4K;
				break;
			case ADVANCED:
				sof_bits = 6;
				m = MC4K;
				break;
			case FAST_ADVANCED:
				sof_bits = 6;
				m = MC8K;
				break;
			default:
				break;
			}
			if ((tag.LCON && page == 1)
					|| (tag.LKP && (page == 2 || page == 3))) {
				//deny
				*txlen = 0;
			} else {
				//allow
				*txlen = 2;
				tx[0] = 0x40;
				page_to_be_written = page;
				tag.tstate = WRITING_PAGE_DATA;
			}

		} else if ((rx[0] & 0xf0) == 0x90) { //write block
			page = ((rx[0] & 0x0f) * 6) + ((rx[1] & 0xf0) / 16);
			switch (tag.mode) {
			case STANDARD:
				sof_bits = 1;
				m = MC4K;
				break;
			case ADVANCED:
				sof_bits = 6;
				m = MC4K;
				break;
			case FAST_ADVANCED:
				sof_bits = 6;
				m = MC8K;
				break;
			default:
				break;
			}
			if (page % 4 != 0 || page == 0) {
				//deny
				*txlen = 0;
			} else {
				//allow
				*txlen = 2;
				tx[0] = 0x40;
				page_to_be_written = page;
				block_data_left = 4;
				tag.tstate = WRITING_BLOCK_DATA;
			}
		}
	}
		break;
	default:

		break;
	}
}


/*
 * to autenticate to a tag with the given key or challenge
 */
static int hitagS_handle_tag_auth(hitag_function htf,uint64_t key, uint64_t NrAr, byte_t* rx,
		const size_t rxlen, byte_t* tx, size_t* txlen) {
	byte_t rx_air[HITAG_FRAME_LEN];
	int response_bit[200] = {0};
	int i, j, z, k;
	unsigned char mask = 1;
	unsigned char uid[32];
	byte_t uid1 = 0x00, uid2 = 0x00, uid3 = 0x00, uid4 = 0x00;
	unsigned char crc;
	uint64_t state;
	byte_t auth_ks[4];
	byte_t conf_pages[3];
	memcpy(rx_air, rx, nbytes(rxlen));
	*txlen = 0;

	if (DEBUG) {
		Dbprintf("START hitagS_handle_tag_auth - rxlen: %d, tagstate=%d", rxlen, (int)tag.pstate);
	}

	if (tag.pstate == READY && rxlen >= 32) {
		//received uid
		if(end==true) {
			Dbprintf("authentication failed!");
			return -1;
		}
		z = 0;
		for (i = 0; i < 10; i++) {
			for (j = 0; j < 8; j++) {
				response_bit[z] = 0;
				if ((rx[i] & ((mask << 7) >> j)) != 0)
					response_bit[z] = 1;
				z++;
			}
		}
		for (i = 0; i < 32; i++) {
			uid[i] = response_bit[i];
		}

		uid1 = (uid[0] << 7) | (uid[1] << 6) | (uid[2] << 5) | (uid[3] << 4)
				| (uid[4] << 3) | (uid[5] << 2) | (uid[6] << 1) | uid[7];
		uid2 = (uid[8] << 7) | (uid[9] << 6) | (uid[10] << 5) | (uid[11] << 4)
				| (uid[12] << 3) | (uid[13] << 2) | (uid[14] << 1) | uid[15];
		uid3 = (uid[16] << 7) | (uid[17] << 6) | (uid[18] << 5) | (uid[19] << 4)
				| (uid[20] << 3) | (uid[21] << 2) | (uid[22] << 1) | uid[23];
		uid4 = (uid[24] << 7) | (uid[25] << 6) | (uid[26] << 5) | (uid[27] << 4)
				| (uid[28] << 3) | (uid[29] << 2) | (uid[30] << 1) | uid[31];
		Dbprintf("UID: %02X %02X %02X %02X", uid1, uid2, uid3, uid4);
		tag.uid = (uid4 << 24 | uid3 << 16 | uid2 << 8 | uid1);

		//select uid
		crc = CRC_PRESET;
		calc_crc(&crc, 0x00, 5);
		calc_crc(&crc, uid1, 8);
		calc_crc(&crc, uid2, 8);
		calc_crc(&crc, uid3, 8);
		calc_crc(&crc, uid4, 8);
		Dbprintf("crc: %02X", crc);

		//resetting response bit
		for (i = 0; i < 100; i++) {
			response_bit[i] = 0;
		}

		//skip the first 5
		for (i = 5; i < 37; i++) {
			response_bit[i] = uid[i - 5];
		}
		//add crc value
		for (j = 0; j < 8; j++) {
			response_bit[i] = 0;
			if ((crc & ((mask << 7) >> j)) != 0)
				response_bit[i] = 1;
			i++;
		}

		k = 0;
		for (i = 0; i < 6; i++) {
			tx[i] = (response_bit[k] << 7) | (response_bit[k + 1] << 6)
					| (response_bit[k + 2] << 5) | (response_bit[k + 3] << 4)
					| (response_bit[k + 4] << 3) | (response_bit[k + 5] << 2)
					| (response_bit[k + 6] << 1) | response_bit[k + 7];
			k += 8;
		}
		*txlen = 45;
		tag.pstate = INIT;
	} else if (tag.pstate == INIT && rxlen > 24) {
		// received configuration after select command
		z = 0;
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 8; j++) {
				response_bit[z] = 0;
				if ((rx[i] & ((mask << 7) >> j)) != 0) {
					response_bit[z] = 1;
				}
				z++;
			}
		}

		//check wich memorysize this tag has
		//CON0
		if (response_bit[6] == 0 && response_bit[7] == 0)
			tag.max_page = 32 / 32;
		if (response_bit[6] == 0 && response_bit[7] == 1)
			tag.max_page = 256 / 32;
		if (response_bit[6] == 1 && response_bit[7] == 0)
			tag.max_page = 2048 / 32;
		if (response_bit[6] == 1 && response_bit[7] == 1) //reserved but some tags got this setting
			tag.max_page = 2048 / 32;

		//CON1
		tag.auth = response_bit[8];
		tag.TTFC = response_bit[9];
		//tag.TTFDR in response_bit[10] and response_bit[11]
		//tag.TTFM in response_bit[12] and response_bit[13]
		tag.LCON = response_bit[14];
		tag.LKP = response_bit[15];

		//CON2
		tag.LCK7 = response_bit[16];
		tag.LCK6 = response_bit[17];
		tag.LCK5 = response_bit[18];
		tag.LCK4 = response_bit[19];
		tag.LCK3 = response_bit[20];
		tag.LCK2 = response_bit[21];
		tag.LCK1 = response_bit[22];
		tag.LCK0 = response_bit[23];

		if (DEBUG) {
			conf_pages[0] = ((response_bit[0] << 7) | (response_bit[1] << 6)
				| (response_bit[2] << 5) | (response_bit[3] << 4)
				| (response_bit[4] << 3) | (response_bit[5] << 2)
				| (response_bit[6] << 1) | response_bit[7]);
			conf_pages[1] = ((response_bit[8] << 7) | (response_bit[9] << 6)
				| (response_bit[10] << 5) | (response_bit[11] << 4)
				| (response_bit[12] << 3) | (response_bit[13] << 2)
				| (response_bit[14] << 1) | response_bit[15]);
			conf_pages[2] = ((response_bit[16] << 7) | (response_bit[17] << 6)
				| (response_bit[18] << 5) | (response_bit[19] << 4)
				| (response_bit[20] << 3) | (response_bit[21] << 2)
				| (response_bit[22] << 1) | response_bit[23]);
			Dbprintf("conf0: %02X conf1: %02X conf2: %02X", conf_pages[0], conf_pages[1], conf_pages[2]);
			Dbprintf("tag.max_page: %d, tag.auth: %d", tag.max_page, tag.auth);
		}

		if (tag.auth == 1) {
			//if the tag is in authentication mode try the key or challenge
			*txlen = 64;
			if(end!=true){
				if(htf==02||htf==04){ //RHTS_KEY //WHTS_KEY
					state = hitag2_init(REV64(key), REV32(tag.uid), REV32(rnd));
					/*
					Dbprintf("key: %02X %02X\n\n", key, REV64(key));
					Dbprintf("tag.uid: %02X %02X\n\n", tag.uid, REV32(tag.uid));
					Dbprintf("rnd: %02X %02X\n\n", rnd, REV32(rnd));
					*/
					for (i = 0; i < 4; i++) {
						auth_ks[i] = hitag2_byte(&state) ^ 0xff;
					}
					*txlen = 64;
					tx[0] = rnd & 0xff;
					tx[1] = (rnd >> 8) & 0xff;
					tx[2] = (rnd >> 16) & 0xff;
					tx[3] = (rnd >> 24) & 0xff;

					tx[4] = auth_ks[0];
					tx[5] = auth_ks[1];
					tx[6] = auth_ks[2];
					tx[7] = auth_ks[3];
					if (DEBUG)
						Dbprintf("%02X %02X %02X %02X %02X %02X %02X %02X", tx[0],
								tx[1], tx[2], tx[3], tx[4], tx[5], tx[6], tx[7]);
				} else if(htf==01 || htf==03) { //RHTS_CHALLENGE //WHTS_CHALLENGE
					for (i = 0; i < 8; i++)
						tx[i]=((NrAr>>(56-(i*8)))&0xff);
				}
				end=true;
				tag.pstate = AUTHENTICATE;
			} else {
				Dbprintf("authentication failed!");
				return -1;
			}
		} else if (tag.auth == 0) {
			tag.pstate = SELECTED;
		}

	} else if (tag.pstate == AUTHENTICATE && rxlen >= 32) {
		//encrypted con2,password received.
		if (DEBUG) {
			Dbprintf("UID:::%X", tag.uid);
			Dbprintf("RND:::%X", rnd);
		}

		//decrypt password
		pwdh0=0;
		pwdl0=0;
		pwdl1=0;
		if(htf==02 || htf==04) { //RHTS_KEY //WHTS_KEY
			state = hitag2_init(REV64(key), REV32(tag.uid), REV32(rnd));
			for (i = 0; i < 5; i++)  {
				hitag2_byte(&state);
			}
			pwdh0 = ((rx[1] & 0x0f) * 16 + ((rx[2] & 0xf0) / 16)) ^ hitag2_byte(&state);
			pwdl0 = ((rx[2] & 0x0f) * 16 + ((rx[3] & 0xf0) / 16)) ^ hitag2_byte(&state);
			pwdl1 = ((rx[3] & 0x0f) * 16 + ((rx[4] & 0xf0) / 16)) ^ hitag2_byte(&state);
			if (DEBUG) {
				Dbprintf("pwdh0 %02X pwdl0 %02X pwdl1 %02X", pwdh0, pwdl0, pwdl1);
			}
		}
		tag.pstate = SELECTED; //tag is now ready for read/write commands
	}

	if (DEBUG) {
		Dbprintf("END hitagS_handle_tag_auth - tagstate=%d", (int)tag.pstate);
	}

	return 0;
}

/*
 * Emulates a Hitag S Tag with the given data from the .hts file
 */
void SimulateHitagSTag(bool tag_mem_supplied, byte_t* data) {
	int frame_count;
	int response;
	int overflow;
	int i, j;
	byte_t rx[HITAG_FRAME_LEN];
	size_t rxlen = 0;
	bQuiet = false;
	byte_t txbuf[HITAG_FRAME_LEN];
	byte_t* tx = txbuf;
	size_t txlen = 0;
	uint8_t con0, con1, con2;
	BigBuf_free();

// Clean up trace and prepare it for storing frames
	set_tracing(true);
	clear_trace();

	DbpString("Starting HitagS simulation");
	LED_D_ON();

	tag.pstate = READY;
	tag.tstate = NO_OP;
	//read tag data into memory
	if (tag_mem_supplied) {
		DbpString("Loading hitagS memory...");
		for (i = 0; i < 64; i++) {
			for (j = 0; j < 4; j++) {
				tag.pages[i][j] = 0x0;
			}
		}

		for (i = 0; i < 64; i++) {
			for (j = 0; j < 4; j++) {
				tag.pages[i][j] = data[(i*4)+j];
			}
		}
	}
	tag.uid = (tag.pages[0][3] << 24 | tag.pages[0][2] << 16 | tag.pages[0][1] << 8 | tag.pages[0][0]);
	con0 = tag.pages[1][3];
	con1 = tag.pages[1][2];
	con2 = tag.pages[1][1];
	Dbprintf("UID: %X", tag.uid);
	Dbprintf("Hitag S simulation started");

	//0x01 plain mode - Reserved, CON2, CON1, CON0
	//0x01 auth  mode - PWDH 0,   CON2, CON1, CON0
	//0x02 auth  mode - KEYH 1, KEYH 0, PWDL 1, PWDL 0
	//0x03 auth  mode - KEYL 3, KEYL 2, KEYL 1, KEYL 0

	//con0
	tag.max_page = 2048 / 32;
	if ((con0 & 0x2) == 0 && (con0 & 0x1) == 1)
		tag.max_page = 256 / 32;
	if ((con0 & 0x2) == 0 && (con0 & 0x1) == 0)
		tag.max_page = 32 / 32;

	//CON1
	tag.auth = ((con1 & 0x80) == 0x80) ? 1 : 0;
	tag.TTFC = ((con1 & 0x40) == 0x40) ? 1 : 0;
	//tag.TTFDR in response_bit[10] and response_bit[11]
	//tag.TTFM in response_bit[12] and response_bit[13]
	tag.LCON = ((con1 & 0x2) == 0x2) ? 1 : 0;
	tag.LKP  = ((con1 & 0x1) == 0x1) ? 1 : 0;

	//CON2
	tag.LCK7 = ((con2 & 0x80) == 0x80) ? 1 : 0;
	tag.LCK6 = ((con2 & 0x40) == 0x40) ? 1 : 0;
	tag.LCK5 = ((con2 & 0x20) == 0x20) ? 1 : 0;
	tag.LCK4 = ((con2 & 0x10) == 0x10) ? 1 : 0;
	tag.LCK3 = ((con2 & 0x8) == 0x8) ? 1 : 0;
	tag.LCK2 = ((con2 & 0x4) == 0x4) ? 1 : 0;
	tag.LCK1 = ((con2 & 0x2) == 0x2) ? 1 : 0;
	tag.LCK0 = ((con2 & 0x1) == 0x1) ? 1 : 0;

	if (tag.auth == 1) {
		//TODO check if this working :D
		tag.key=(intptr_t)tag.pages[3];
		tag.key<<=16;
		tag.key+=((tag.pages[2][0])<<8)+tag.pages[2][1];
		tag.pwdl0=tag.pages[2][3];
		tag.pwdl1=tag.pages[2][2];
		tag.pwdh0=tag.pages[1][0];
	}

	// Set up simulator mode, frequency divisor which will drive the FPGA
	// and analog mux selection.
	FpgaDownloadAndGo(FPGA_BITSTREAM_LF);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_LF_EDGE_DETECT);
	FpgaSendCommand(FPGA_CMD_SET_DIVISOR, 95); //125Khz
	SetAdcMuxFor(GPIO_MUXSEL_LOPKD);
	RELAY_OFF();

	// Configure output pin that is connected to the FPGA (for modulating)
	AT91C_BASE_PIOA->PIO_OER = GPIO_SSC_DOUT;
	AT91C_BASE_PIOA->PIO_PER = GPIO_SSC_DOUT;

	// Disable modulation at default, which means release resistance
	LOW(GPIO_SSC_DOUT);

	// Enable Peripheral Clock for TIMER_CLOCK0, used to measure exact timing before answering
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);

	// Enable Peripheral Clock for TIMER_CLOCK1, used to capture edges of the reader frames
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC1);
	AT91C_BASE_PIOA->PIO_BSR = GPIO_SSC_FRAME;

	// Disable timer during configuration
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;

	// TC0: Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), no triggers
	AT91C_BASE_TC0->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK;

	// TC1: Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), TIOA is external trigger,
	// external trigger rising edge, load RA on rising edge of TIOA.
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK | AT91C_TC_ETRGEDG_RISING | AT91C_TC_ABETRG | AT91C_TC_LDRA_RISING;

	// Reset the received frame, frame count and timing info
	memset(rx, 0x00, sizeof(rx));
	frame_count = 0;
	response = 0;
	overflow = 0;

	// Enable and reset counter
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

	while (!BUTTON_PRESS()) {
		// Watchdog hit
		WDT_HIT();

		// Receive frame, watch for at most T0*EOF periods
		//HITAG_T_WAIT_MAX
		while (AT91C_BASE_TC1->TC_CV < T0 * HITAG_T_EOF) {
			// Check if rising edge in modulation is detected
			if (AT91C_BASE_TC1->TC_SR & AT91C_TC_LDRAS) {
				// Retrieve the new timing values
				int ra = (AT91C_BASE_TC1->TC_RA / T0) + overflow;
				overflow = 0;

				// Reset timer every frame, we have to capture the last edge for timing
				AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

				LED_B_ON();

				// Capture reader frame
				if (ra >= HITAG_T_STOP) {
					if (rxlen != 0) {
						//DbpString("wierd0?");
					}
					// Capture the T0 periods that have passed since last communication or field drop (reset)
					response = (ra - HITAG_T_LOW);
				} else if (ra >= HITAG_T_1_MIN) {
					// '1' bit
					rx[rxlen / 8] |= 1 << (7 - (rxlen % 8));
					rxlen++;
				} else if (ra >= HITAG_T_0_MIN) {
					// '0' bit
					rx[rxlen / 8] |= 0 << (7 - (rxlen % 8));
					rxlen++;
				} else {
					// Ignore wierd value, is to small to mean anything
				}
			}
		}

		// Check if frame was captured
		if (rxlen > 0) {
			frame_count++;
			if (!bQuiet) {
				if (!LogTraceHitag(rx, rxlen, response, 0, true)) {
					DbpString("Trace full");
					clear_trace();
				}
			}

			// Disable timer 1 with external trigger to avoid triggers during our own modulation
			AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;

			// Process the incoming frame (rx) and prepare the outgoing frame (tx)
			hitagS_handle_reader_command(rx, rxlen, tx, &txlen);

			// Wait for HITAG_T_WAIT_1 carrier periods after the last reader bit,
			// not that since the clock counts since the rising edge, but T_Wait1 is
			// with respect to the falling edge, we need to wait actually (T_Wait1 - T_Low)
			// periods. The gap time T_Low varies (4..10). All timer values are in
			// terms of T0 units
			while (AT91C_BASE_TC0->TC_CV < T0 * (HITAG_T_WAIT_1 - HITAG_T_LOW)) { }

			// Send and store the tag answer (if there is any)
			if (txlen > 0) {
				// Transmit the tag frame
				hitag_tag_send_frame(tx, txlen);

				// Store the frame in the trace
				if (!bQuiet) {
					if (!LogTraceHitag(tx, txlen, 0, 0, false)) {
						DbpString("Trace full");
						clear_trace();
					}
				}
			}

			// Enable and reset external trigger in timer for capturing future frames
			AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

			// Reset the received frame and response timing info
			memset(rx, 0x00, sizeof(rx));
			response = 0;

			LED_B_OFF();
		}
		// Reset the frame length
		rxlen = 0;
		// Save the timer overflow, will be 0 when frame was received
		overflow += (AT91C_BASE_TC1->TC_CV / T0);
		// Reset the timer to restart while-loop that receives frames
		AT91C_BASE_TC1->TC_CCR = AT91C_TC_SWTRG;
	}
	Dbprintf("Hitag S simulation stopped");
	LED_B_OFF();
	LED_D_OFF();
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
}

/*
 * Authenticates to the Tag with the given key or challenge.
 * If the key was given the password will be decrypted.
 * Reads every page of a hitag S transpoder.
 */
void ReadHitagSintern(hitag_function htf, hitag_data* htd, stype tagMode, int startPage, bool readBlock) {
	int frame_count;
	int i;
	int sendNum = startPage;
	tag.mode = tagMode;

	//int i, j, z;
	//int response_bit[200];
	//unsigned char mask = 1;

	int response;
	byte_t rx[HITAG_FRAME_LEN];
	size_t rxlen = 0;
	byte_t txbuf[HITAG_FRAME_LEN];
	byte_t* tx = txbuf;
	size_t txlen = 0;
	int lastbit;
	int t_wait = HITAG_T_WAIT_MAX;
	bool bStop;
	bool bQuitTraceFull = false;

	page_to_be_written = 0;

	//read given key/challenge
	byte_t NrAr_[8];
	uint64_t key=0;
	uint64_t NrAr=0;
	byte_t key_[6];
	switch(htf) {
		case 01:
		case 03: { //RHTS_CHALLENGE
			DbpString("Authenticating using nr,ar pair:");
			memcpy(NrAr_,htd->auth.NrAr,8);
			Dbhexdump(8,NrAr_,false);
			NrAr=NrAr_[7] | ((uint64_t)NrAr_[6]) << 8 | ((uint64_t)NrAr_[5]) << 16 | ((uint64_t)NrAr_[4]) << 24 | ((uint64_t)NrAr_[3]) << 32 |
						((uint64_t)NrAr_[2]) << 40| ((uint64_t)NrAr_[1]) << 48 | ((uint64_t)NrAr_[0]) << 56;
		} break;
		case 02:
		case 04: { //RHTS_KEY
			DbpString("Authenticating using key:");
			memcpy(key_,htd->crypto.key,6);
			Dbhexdump(6,key_,false);
			key=key_[5] | ((uint64_t)key_[4]) << 8 | ((uint64_t)key_[3]) << 16 | ((uint64_t)key_[2]) << 24 | ((uint64_t)key_[1]) << 32 | ((uint64_t)key_[0]) << 40;
		} break;
		default: {
			Dbprintf("Error , unknown function: %d",htf);
			return;
		} break;
	}



	FpgaDownloadAndGo(FPGA_BITSTREAM_LF);
	// Reset the return status
	bSuccessful = false;

	// Clean up trace and prepare it for storing frames
	set_tracing(true);
	clear_trace();

	bQuiet = false;

	LED_D_ON();

	// Configure output and enable pin that is connected to the FPGA (for modulating)
	AT91C_BASE_PIOA->PIO_OER = GPIO_SSC_DOUT;
	AT91C_BASE_PIOA->PIO_PER = GPIO_SSC_DOUT;

	// Set fpga in edge detect with reader field, we can modulate as reader now
	FpgaWriteConfWord(FPGA_MAJOR_MODE_LF_EDGE_DETECT | FPGA_LF_EDGE_DETECT_READER_FIELD);

	// Set Frequency divisor which will drive the FPGA and analog mux selection
	FpgaSendCommand(FPGA_CMD_SET_DIVISOR, 95);                  //125Khz
	SetAdcMuxFor(GPIO_MUXSEL_LOPKD);
	RELAY_OFF();

	// Disable modulation at default, which means enable the field
	LOW(GPIO_SSC_DOUT);

	// Give it a bit of time for the resonant antenna to settle.
	SpinDelay(30);

	// Enable Peripheral Clock for TIMER_CLOCK0, used to measure exact timing before answering
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);

	// Enable Peripheral Clock for TIMER_CLOCK1, used to capture edges of the tag frames
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC1);
	AT91C_BASE_PIOA->PIO_BSR = GPIO_SSC_FRAME;

	// Disable timer during configuration
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;

	// TC0: Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), no triggers
	AT91C_BASE_TC0->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK;

	// TC1: Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), TIOA is external trigger,
	// external trigger rising edge, load RA on falling edge of TIOA.
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK | AT91C_TC_ETRGEDG_FALLING | AT91C_TC_ABETRG | AT91C_TC_LDRA_FALLING;

	// Enable and reset counters
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

	// Reset the received frame, frame count and timing info
	frame_count = 0;
	response = 0;
	lastbit = 1;
	bStop = false;
	t_wait = 200;

	while (!bStop && !BUTTON_PRESS()) {
		// Watchdog hit
		WDT_HIT();


		// Add transmitted frame to total count
		if (txlen > 0) {
			frame_count++;

			if (tag.pstate == READY && rxlen < 1) {
				//skip logging starting auths if no response
			} else {
				if (!bQuiet) {
					// Store the frame in the trace
					if (!LogTraceHitag(tx, txlen, HITAG_T_WAIT_2, 0, true)) {
						if (bQuitTraceFull) {
							DbpString("Trace full");
							break;
						} else {
							bQuiet = true;
						}
					}
				}
			}
		}

		// Check if frame was captured and store it
		if (rxlen > 0) {
			frame_count++;
			if (!bQuiet) {
				if (!LogTraceHitag(rx, rxlen, response, 0, false)) {
					DbpString("Trace full");
					if (bQuitTraceFull) {
						break;
					} else {
						bQuiet = true;
					}
				}
			}
		}

		// By default reset the transmission buffer
		tx = txbuf;
		txlen = 0;

		if (DEBUG >= 2) {
			Dbprintf("FRO %d rxlen: %d, pstate=%d, tstate=%d", frame_count, rxlen, (int)tag.pstate,  (int)tag.tstate);
		}

		if (rxlen == 0) {
			//start authentication
			hitag_start_auth(tx, &txlen);
		} else if (tag.pstate != SELECTED) {
			if (hitagS_handle_tag_auth(htf, key,NrAr,rx, rxlen, tx, &txlen) == -1) {
				Dbprintf("hitagS_handle_tag_auth - bStop = !false");
				bStop = !false;
			}
		}



		if (readBlock && tag.pstate == SELECTED && (tag.tstate == READING_BLOCK || tag.tstate == NO_OP) && rxlen > 0) {
			i = hitag_read_block(htf, key, rx, &rxlen, tx, &txlen, sendNum);
			if (i > 0) { sendNum+=4; }
			if (sendNum+4 >= tag.max_page) {
				bStop = !false;
			}
		} else if (!readBlock && tag.pstate == SELECTED && (tag.tstate == READING_PAGE || tag.tstate == NO_OP) && rxlen > 0) {
			i = hitag_read_page(htf, key, rx, &rxlen, tx, &txlen, sendNum);
			if (i > 0) { sendNum++; }
			if (sendNum >= tag.max_page) {
				bStop = !false;
			}
		}

		// Send and store the reader command
		// Disable timer 1 with external trigger to avoid triggers during our own modulation
		AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;

		// Wait for HITAG_T_WAIT_2 carrier periods after the last tag bit before transmitting,
		// Since the clock counts since the last falling edge, a 'one' means that the
		// falling edge occured halfway the period. with respect to this falling edge,
		// we need to wait (T_Wait2 + half_tag_period) when the last was a 'one'.
		// All timer values are in terms of T0 units

		while (AT91C_BASE_TC0->TC_CV < T0 * (t_wait + (HITAG_T_TAG_HALF_PERIOD * lastbit))) { }

		// Transmit the reader frame
		hitag_reader_send_frame(tx, txlen);


		// Enable and reset external trigger in timer for capturing future frames
		AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;


		// Reset values for receiving frames
		memset(rx, 0x00, sizeof(rx));
		rxlen = 0;
		lastbit = 1;
		response = 0;

		// get tag id in anti-collision mode (proprietary data format, so switch off manchester and read at double the data rate, for 4 x the data bits)
		hitag_receive_frame(rx, &rxlen, &response);
	}
	end=false;
	LED_B_OFF();
	LED_D_OFF();
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	cmd_send(CMD_ACK, bSuccessful, 0, 0, 0, 0);
}

void ReadHitagSCmd(hitag_function htf, hitag_data* htd, uint64_t startPage, uint64_t tagMode, bool readBlock) {
	if (tagMode == 1) {
		Dbprintf("ReadHitagS in mode=ADVANCED, blockRead=%d, startPage=%d", readBlock, startPage);
		ReadHitagSintern(htf, htd, ADVANCED, (int)startPage, readBlock);
	} else if (tagMode == 2) {
		Dbprintf("ReadHitagS in mode=FAST_ADVANCED, blockRead=%d, startPage=%d", readBlock, startPage);
		ReadHitagSintern(htf, htd, FAST_ADVANCED, (int)startPage, readBlock);
	} else {
		Dbprintf("ReadHitagS in mode=STANDARD, blockRead=%d, startPage=%d", readBlock, startPage);
		ReadHitagSintern(htf, htd, STANDARD, (int)startPage, readBlock);
	}
}



/*
 * Authenticates to the Tag with the given Key or Challenge.
 * Writes the given 32Bit data into page_
 */
void WritePageHitagS(hitag_function htf, hitag_data* htd,int page_) {
	int frame_count;
	int response;
	byte_t rx[HITAG_FRAME_LEN];
	size_t rxlen = 0;
	byte_t txbuf[HITAG_FRAME_LEN];
	byte_t* tx = txbuf;
	size_t txlen = 0;
	int lastbit;
	int t_wait = HITAG_T_WAIT_MAX;
	bool bStop;
	bool bQuitTraceFull = false;
	int page = page_;
	unsigned char crc;
	byte_t data[4]= {0,0,0,0};

	//read given key/challenge, the page and the data
	byte_t NrAr_[8];
	uint64_t key=0;
	uint64_t NrAr=0;
	byte_t key_[6];
	switch(htf) {
		case 03: { //WHTS_CHALLENGE
			memcpy(data,htd->auth.data,4);
			DbpString("Authenticating using nr,ar pair:");
			memcpy(NrAr_,htd->auth.NrAr,8);
			Dbhexdump(8,NrAr_,false);
			NrAr=NrAr_[7] | ((uint64_t)NrAr_[6]) << 8 | ((uint64_t)NrAr_[5]) << 16 | ((uint64_t)NrAr_[4]) << 24 | ((uint64_t)NrAr_[3]) << 32 |
						((uint64_t)NrAr_[2]) << 40| ((uint64_t)NrAr_[1]) << 48 | ((uint64_t)NrAr_[0]) << 56;
		} break;
		case 04: { //WHTS_KEY
			memcpy(data,htd->crypto.data,4);
			DbpString("Authenticating using key:");
			memcpy(key_,htd->crypto.key,6);
			Dbhexdump(6,key_,false);
			key=key_[5] | ((uint64_t)key_[4]) << 8 | ((uint64_t)key_[3]) << 16 | ((uint64_t)key_[2]) << 24 | ((uint64_t)key_[1]) << 32 | ((uint64_t)key_[0]) << 40;
		} break;
		default: {
			Dbprintf("Error , unknown function: %d",htf);
			return;
		} break;
	}

	Dbprintf("Page: %d",page_);
	Dbprintf("DATA: %02X %02X %02X %02X", data[0], data[1], data[2], data[3]);
	FpgaDownloadAndGo(FPGA_BITSTREAM_LF);
	// Reset the return status
	bSuccessful = false;

	tag.pstate = READY;
	tag.tstate = NO_OP;

	// Clean up trace and prepare it for storing frames
	set_tracing(true);
	clear_trace();

	bQuiet = false;

	LED_D_ON();

	// Configure output and enable pin that is connected to the FPGA (for modulating)
	AT91C_BASE_PIOA->PIO_OER = GPIO_SSC_DOUT;
	AT91C_BASE_PIOA->PIO_PER = GPIO_SSC_DOUT;

	// Set fpga in edge detect with reader field, we can modulate as reader now
	FpgaWriteConfWord(FPGA_MAJOR_MODE_LF_EDGE_DETECT | FPGA_LF_EDGE_DETECT_READER_FIELD);

	// Set Frequency divisor which will drive the FPGA and analog mux selection
	FpgaSendCommand(FPGA_CMD_SET_DIVISOR, 95); //125Khz
	SetAdcMuxFor(GPIO_MUXSEL_LOPKD);
	RELAY_OFF();

	// Disable modulation at default, which means enable the field
	LOW(GPIO_SSC_DOUT);

	// Give it a bit of time for the resonant antenna to settle.
	SpinDelay(30);

	// Enable Peripheral Clock for TIMER_CLOCK0, used to measure exact timing before answering
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);

	// Enable Peripheral Clock for TIMER_CLOCK1, used to capture edges of the tag frames
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC1);
	AT91C_BASE_PIOA->PIO_BSR = GPIO_SSC_FRAME;

	// Disable timer during configuration
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;

	// TC0: Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), no triggers
	AT91C_BASE_TC0->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK;

	// TC1: Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), TIOA is external trigger,
	// external trigger rising edge, load RA on falling edge of TIOA.
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK
			| AT91C_TC_ETRGEDG_FALLING | AT91C_TC_ABETRG
			| AT91C_TC_LDRA_FALLING;

	// Enable and reset counters
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

	// Reset the received frame, frame count and timing info
	frame_count = 0;
	response = 0;
	lastbit = 1;
	bStop = false;
	t_wait = 200;

	while (!bStop && !BUTTON_PRESS()) {
		// Watchdog hit
		WDT_HIT();

		// Check if frame was captured and store it
		if (rxlen > 0) {
			frame_count++;
			if (!bQuiet) {
				if (!LogTraceHitag(rx, rxlen, response, 0, false)) {
					DbpString("Trace full");
					if (bQuitTraceFull) {
						break;
					} else {
						bQuiet = true;
					}
				}
			}
		}

		//check for valid input
		/*
		if (page == 0) {
			Dbprintf("usage: lf hitag writer [03 | 04] [CHALLENGE | KEY] [page] [byte0] [byte1] [byte2] [byte3]");
			bStop = !false;
		}*/


		// By default reset the transmission buffer
		tx = txbuf;
		txlen = 0;

		if (rxlen == 0 && tag.tstate == WRITING_PAGE_ACK) {
			//no write access on this page
			Dbprintf("no write access on page %d", page_);
			bStop = !false;
		} else if (rxlen == 0 && tag.tstate != WRITING_PAGE_DATA) {
			//start the authetication
			//tag.mode = ADVANCED;
			tag.mode = STANDARD;
			hitag_start_auth(tx, &txlen);
			m = AC2K;
		} else if (tag.pstate != SELECTED) {
			//try to authenticate with the given key or challenge
			if (hitagS_handle_tag_auth(htf,key,NrAr,rx, rxlen, tx, &txlen) == -1) {
				bStop = !false;
			}
			m = MC4K;
		}

		if (tag.pstate == SELECTED && tag.tstate == NO_OP && rxlen > 0) {
			//check if the given page exists
			if (page > tag.max_page) {
				Dbprintf("page number too big");
				bStop = !false;
			}
			//ask Tag for write permission
			tag.tstate = WRITING_PAGE_ACK;
			txlen = 20;
			crc = CRC_PRESET;
			tx[0] = 0x90 + (page / 16);
			calc_crc(&crc, tx[0], 8);
			calc_crc(&crc, 0x00 + ((page % 16) * 16), 4);
			tx[1] = 0x00 + ((page % 16) * 16) + (crc / 16);
			tx[2] = 0x00 + (crc % 16) * 16;
		} else if (tag.pstate == SELECTED && tag.tstate == WRITING_PAGE_ACK
				&& rxlen == 2 && rx[0] == 0x40) {
			//ACK recieved to write the page. send data
			tag.tstate = WRITING_PAGE_DATA;
			txlen = 40;
			crc = CRC_PRESET;
			calc_crc(&crc, data[3], 8);
			calc_crc(&crc, data[2], 8);
			calc_crc(&crc, data[1], 8);
			calc_crc(&crc, data[0], 8);
			tx[0] = data[3];
			tx[1] = data[2];
			tx[2] = data[1];
			tx[3] = data[0];
			tx[4] = crc;
		} else if (tag.pstate == SELECTED && tag.tstate == WRITING_PAGE_DATA
				&& rxlen == 2 && rx[0] == 0x40) {
			//received ACK
			Dbprintf("Successful!");
			bStop = !false;
		}

		// Send and store the reader command
		// Disable timer 1 with external trigger to avoid triggers during our own modulation
		AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;

		// Wait for HITAG_T_WAIT_2 carrier periods after the last tag bit before transmitting,
		// Since the clock counts since the last falling edge, a 'one' means that the
		// falling edge occured halfway the period. with respect to this falling edge,
		// we need to wait (T_Wait2 + half_tag_period) when the last was a 'one'.
		// All timer values are in terms of T0 units

		while (AT91C_BASE_TC0->TC_CV
				< T0 * (t_wait + (HITAG_T_TAG_HALF_PERIOD * lastbit)))
			;

		// Transmit the reader frame
		hitag_reader_send_frame(tx, txlen);

		// Enable and reset external trigger in timer for capturing future frames
		AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

		// Add transmitted frame to total count
		if (txlen > 0) {
			frame_count++;
			if (!bQuiet) {
				// Store the frame in the trace
				if (!LogTraceHitag(tx, txlen, HITAG_T_WAIT_2, 0, true)) {
					if (bQuitTraceFull) {
						DbpString("Trace full");
						break;
					} else {
						bQuiet = true;
					}
				}
			}
		}

		// Reset values for receiving frames
		memset(rx, 0x00, sizeof(rx));
		rxlen = 0;
		lastbit = 1;
		response = 0;

		hitag_receive_frame(rx, &rxlen, &response);
	}
	end=false;
	LED_B_OFF();
	LED_D_OFF();
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	cmd_send(CMD_ACK, bSuccessful, 0, 0, 0, 0);
}


/*
 * Tries to authenticate to a Hitag S Transponder with the given challenges from a .cc file.
 * Displays all Challenges that failed.
 * When collecting Challenges to break the key it is possible that some data
 * is not received correctly due to Antenna problems. This function
 * detects these challenges.
 */
void check_challenges_cmd(bool file_given, byte_t* data, uint64_t tagMode) {
	int i, j, z, k;
	byte_t uid_byte[4];
	int frame_count;
	int response;
	byte_t rx[HITAG_FRAME_LEN];
	byte_t unlocker[60][8];
	int u1 = 0;
	size_t rxlen = 0;
	byte_t txbuf[HITAG_FRAME_LEN];
	byte_t* tx = txbuf;
	size_t txlen = 0;
	int lastbit;
	int t_wait = HITAG_T_WAIT_MAX;
	int STATE = 0;
	bool bStop;
	bool bQuitTraceFull = false;
	int response_bit[200];
	unsigned char mask = 1;
	unsigned char uid[32];
	unsigned char crc;

	if (tagMode == 1) {
		Dbprintf("check_challenges in mode=ADVANCED");
		tag.mode = ADVANCED;
	} else if (tagMode == 2) {
		Dbprintf("check_challenges in mode=FAST_ADVANCED");
		tag.mode = FAST_ADVANCED;
	} else {
		Dbprintf("check_challenges in mode=STANDARD");
		tag.mode = STANDARD;
	}


	FpgaDownloadAndGo(FPGA_BITSTREAM_LF);
	// Reset the return status
	bSuccessful = false;

	// Clean up trace and prepare it for storing frames
	set_tracing(true);
	clear_trace();

	bQuiet = false;

	LED_D_ON();

	// Configure output and enable pin that is connected to the FPGA (for modulating)
	AT91C_BASE_PIOA->PIO_OER = GPIO_SSC_DOUT;
	AT91C_BASE_PIOA->PIO_PER = GPIO_SSC_DOUT;

	// Set fpga in edge detect with reader field, we can modulate as reader now
	FpgaWriteConfWord(
	FPGA_MAJOR_MODE_LF_EDGE_DETECT | FPGA_LF_EDGE_DETECT_READER_FIELD);

	// Set Frequency divisor which will drive the FPGA and analog mux selection
	FpgaSendCommand(FPGA_CMD_SET_DIVISOR, 95);                  //125Khz
	SetAdcMuxFor(GPIO_MUXSEL_LOPKD);
	RELAY_OFF();

	// Disable modulation at default, which means enable the field
	LOW(GPIO_SSC_DOUT);

	// Give it a bit of time for the resonant antenna to settle.
	SpinDelay(30);

	// Enable Peripheral Clock for TIMER_CLOCK0, used to measure exact timing before answering
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);

	// Enable Peripheral Clock for TIMER_CLOCK1, used to capture edges of the tag frames
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC1);
	AT91C_BASE_PIOA->PIO_BSR = GPIO_SSC_FRAME;

	// Disable timer during configuration
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;

	// TC0: Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), no triggers
	AT91C_BASE_TC0->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK;

	// TC1: Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), TIOA is external trigger,
	// external trigger rising edge, load RA on falling edge of TIOA.
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK | AT91C_TC_ETRGEDG_FALLING | AT91C_TC_ABETRG | AT91C_TC_LDRA_FALLING;

	// Enable and reset counters
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

	// Reset the received frame, frame count and timing info
	frame_count = 0;
	response = 0;
	lastbit = 1;
	bStop = false;

	t_wait = 200;

	if (file_given) {
		DbpString("Loading challenges...");
		memcpy((byte_t*)unlocker,data,60*8);
	}

	while (file_given && !bStop && !BUTTON_PRESS()) {
		// Watchdog hit
		WDT_HIT();

		// Check if frame was captured and store it
		if (rxlen > 0) {
			frame_count++;
			if (!bQuiet) {
				if (!LogTraceHitag(rx, rxlen, response, 0, false)) {
					DbpString("Trace full");
					if (bQuitTraceFull) {
						break;
					} else {
						bQuiet = true;
					}
				}
			}
		}

		tx = txbuf;
		txlen = 0;
		if (rxlen == 0) {
			if (STATE == 2) {
				// challenge failed
				Dbprintf("Challenge failed: %02X %02X %02X %02X %02X %02X %02X %02X",
						unlocker[u1 - 1][0], unlocker[u1 - 1][1],
						unlocker[u1 - 1][2], unlocker[u1 - 1][3],
						unlocker[u1 - 1][4], unlocker[u1 - 1][5],
						unlocker[u1 - 1][6], unlocker[u1 - 1][7]);
			}
			STATE = 0;
			hitag_start_auth(tx, &txlen);
		} else if (rxlen >= 32 && STATE == 0) {
			//received uid
			z = 0;
			for (i = 0; i < 10; i++) {
				for (j = 0; j < 8; j++) {
					response_bit[z] = 0;
					if ((rx[i] & ((mask << 7) >> j)) != 0)
						response_bit[z] = 1;
					z++;
				}
			}
			for (i = 0; i < 32; i++) {
				uid[i] = response_bit[i];
			}

			uid_byte[0] = (uid[0] << 7) | (uid[1] << 6) | (uid[2] << 5)
					| (uid[3] << 4) | (uid[4] << 3) | (uid[5] << 2)
					| (uid[6] << 1) | uid[7];
			uid_byte[1] = (uid[8] << 7) | (uid[9] << 6) | (uid[10] << 5)
					| (uid[11] << 4) | (uid[12] << 3) | (uid[13] << 2)
					| (uid[14] << 1) | uid[15];
			uid_byte[2] = (uid[16] << 7) | (uid[17] << 6) | (uid[18] << 5)
					| (uid[19] << 4) | (uid[20] << 3) | (uid[21] << 2)
					| (uid[22] << 1) | uid[23];
			uid_byte[3] = (uid[24] << 7) | (uid[25] << 6) | (uid[26] << 5)
					| (uid[27] << 4) | (uid[28] << 3) | (uid[29] << 2)
					| (uid[30] << 1) | uid[31];
			//Dbhexdump(10, rx, rxlen);
			STATE = 1;
			txlen = 45;
			crc = CRC_PRESET;
			calc_crc(&crc, 0x00, 5);
			calc_crc(&crc, uid_byte[0], 8);
			calc_crc(&crc, uid_byte[1], 8);
			calc_crc(&crc, uid_byte[2], 8);
			calc_crc(&crc, uid_byte[3], 8);
			for (i = 0; i < 100; i++) {
				response_bit[i] = 0;
			}
			for (i = 0; i < 5; i++) {
				response_bit[i] = 0;
			}
			for (i = 5; i < 37; i++) {
				response_bit[i] = uid[i - 5];
			}
			for (j = 0; j < 8; j++) {
				response_bit[i] = 0;
				if ((crc & ((mask << 7) >> j)) != 0)
					response_bit[i] = 1;
				i++;
			}
			k = 0;
			for (i = 0; i < 6; i++) {
				tx[i] = (response_bit[k] << 7) | (response_bit[k + 1] << 6)
						| (response_bit[k + 2] << 5)
						| (response_bit[k + 3] << 4)
						| (response_bit[k + 4] << 3)
						| (response_bit[k + 5] << 2)
						| (response_bit[k + 6] << 1) | response_bit[k + 7];
				k += 8;
			}


			tag.pstate = INIT;
		} else if (STATE == 1 && rxlen > 24) {
			//received configuration
			STATE = 2;
			z = 0;
			for (i = 0; i < 6; i++) {
				for (j = 0; j < 8; j++) {
					response_bit[z] = 0;
					if ((rx[i] & ((mask << 7) >> j)) != 0)
						response_bit[z] = 1;
					z++;
				}
			}
			txlen = 64;

			if (u1 >= (sizeof(unlocker) / sizeof(unlocker[0])))
				bStop = !false;
			for (i = 0; i < 8; i++)
				tx[i] = unlocker[u1][i];
			u1++;

			tag.pstate = SELECTED;
		} else if (STATE == 2 && rxlen >= 32) {
			STATE = 0;
		}

		// Send and store the reader command
		// Disable timer 1 with external trigger to avoid triggers during our own modulation
		AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;

		// Wait for HITAG_T_WAIT_2 carrier periods after the last tag bit before transmitting,
		// Since the clock counts since the last falling edge, a 'one' means that the
		// falling edge occured halfway the period. with respect to this falling edge,
		// we need to wait (T_Wait2 + half_tag_period) when the last was a 'one'.
		// All timer values are in terms of T0 units
		while (AT91C_BASE_TC0->TC_CV < T0 * (t_wait + (HITAG_T_TAG_HALF_PERIOD * lastbit))) { }

		// Transmit the reader frame
		hitag_reader_send_frame(tx, txlen);

		// Enable and reset external trigger in timer for capturing future frames
		AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

		// Add transmitted frame to total count
		if (txlen > 0) {
			frame_count++;
			if (!bQuiet) {
				// Store the frame in the trace
				if (!LogTraceHitag(tx, txlen, HITAG_T_WAIT_2, 0, true)) {
					if (bQuitTraceFull) {
						DbpString("Trace full");
						break;
					} else {
						bQuiet = true;
					}
				}
			}
		}

		// Reset values for receiving frames
		memset(rx, 0x00, sizeof(rx));
		rxlen = 0;
		lastbit = 1;
		response = 0;

		hitag_receive_frame(rx, &rxlen, &response);
	}
	LED_B_OFF();
	LED_D_OFF();
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	cmd_send(CMD_ACK, bSuccessful, 0, 0, 0, 0);
}

/**
Backward compatibility
*/
void check_challenges(bool file_given, byte_t* data) {
	check_challenges_cmd(file_given, data, 1);
}

void ReadHitagS(hitag_function htf, hitag_data* htd) {
	ReadHitagSintern(htf, htd, ADVANCED, 0, false);
}
