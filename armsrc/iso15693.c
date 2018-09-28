//-----------------------------------------------------------------------------
// Jonathan Westhues, split Nov 2006
// Modified by Greg Jones, Jan 2009
// Modified by Adrian Dabrowski "atrox", Mar-Sept 2010,Oct 2011
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support ISO 15693. This includes both the reader software and
// the `fake tag' modes, but at the moment I've implemented only the reader
// stuff, and that barely.
// Modified to perform modulation onboard in arm rather than on PC
// Also added additional reader commands (SELECT, READ etc.)
//-----------------------------------------------------------------------------
// The ISO 15693 describes two transmission modes from reader to tag, and 4 
// transmission modes from tag to reader. As of Mar 2010 this code only 
// supports one of each: "1of4" mode from reader to tag, and the highspeed 
// variant with one subcarrier from card to reader.
// As long, as the card fully support ISO 15693 this is no problem, since the 
// reader chooses both data rates, but some non-standard tags do not. Further for 
// the simulation to work, we will need to support all data rates.
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
// added "1 out of 256" mode (for VCD->PICC) - atrox 20100911


// Random Remarks:
// *) UID is always used "transmission order" (LSB), which is reverse to display order

// TODO / BUGS / ISSUES:
// *) writing to tags takes longer: we miss the answer from the tag in most cases
//    -> tweak the read-timeout times
// *) signal decoding from the card is still a bit shaky. 
// *) signal decoding is unable to detect collissions.
// *) add anti-collission support for inventory-commands 
// *) read security status of a block
// *) sniffing and simulation do only support one transmission mode. need to support 
//		all 8 transmission combinations
//	*) remove or refactor code under "depricated"
// *) document all the functions


#include "proxmark3.h"
#include "util.h"
#include "apps.h"
#include "string.h"
#include "iso15693tools.h"
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

// approximate amplitude=sqrt(ci^2+cq^2) 
#define AMPLITUDE(ci, cq) (MAX(ABS(ci), ABS(cq)) + MIN(ABS(ci), ABS(cq))/2)

// DMA buffer
#define ISO15693_DMA_BUFFER_SIZE 128

// ---------------------------
// Signal Processing 
// ---------------------------

// prepare data using "1 out of 4" code for later transmission
// resulting data rate is 26,48 kbit/s (fc/512)
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

	// And slack at the end, too.
	for(i = 0; i < 24; i++) {
		ToSendStuffBit(1);
	}
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
// Transmit the command (to the reader) that was placed in cmd[].
//-----------------------------------------------------------------------------
static void TransmitTo15693Reader(const uint8_t *cmd, int len)
{
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_MODULATE_424K);

	LED_C_ON();
    for(int c = 0; c < len; ) {
        if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_TXRDY)) {
            AT91C_BASE_SSC->SSC_THR = cmd[c];
            c++;
        }
        WDT_HIT();
    }
	LED_C_OFF();
}


//=============================================================================
// An ISO 15693 demodulator (one subcarrier only). Uses cross correlation to 
// identify the SOF, each bit, and EOF.
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

typedef struct Demod {
	enum {
		DEMOD_UNSYNCD,
		DEMOD_AWAIT_SOF_1,
		DEMOD_AWAIT_SOF_2,
		DEMOD_RECEIVING_DATA,
		DEMOD_AWAIT_EOF
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
} Demod_t;

static RAMFUNC int Handle15693SamplesDemod(int8_t ci, int8_t cq, Demod_t *Demod)
{
	switch(Demod->state) {
		case DEMOD_UNSYNCD:
			// initialize SOF correlator. We are looking for 12 samples low and 12 samples high.
			Demod->SOF_low = 0;
			Demod->SOF_high = 12;
			Demod->SOF_last = 23;
			memset(Demod->SOF_correlator, 0x00, Demod->SOF_last + 1);
			Demod->SOF_correlator[Demod->SOF_last] = AMPLITUDE(ci,cq);
			Demod->SOF_corr = Demod->SOF_correlator[Demod->SOF_last];
			Demod->SOF_corr_prev = Demod->SOF_corr;
			// initialize Demodulator
			Demod->posCount = 0;
			Demod->bitCount = 0;
			Demod->len = 0;
			Demod->state = DEMOD_AWAIT_SOF_1;
			break;
			
		case DEMOD_AWAIT_SOF_1:
			// calculate the correlation in real time. Look at differences only.
			Demod->SOF_corr += Demod->SOF_correlator[Demod->SOF_low++];
			Demod->SOF_corr -= 2*Demod->SOF_correlator[Demod->SOF_high++];
			Demod->SOF_last++;
			Demod->SOF_low &= (SOF_CORRELATOR_LEN-1);
			Demod->SOF_high &= (SOF_CORRELATOR_LEN-1);
			Demod->SOF_last &= (SOF_CORRELATOR_LEN-1);
			Demod->SOF_correlator[Demod->SOF_last] = AMPLITUDE(ci,cq);
			Demod->SOF_corr += Demod->SOF_correlator[Demod->SOF_last];

			// if correlation increases for 10 consecutive samples, we are close to maximum correlation
			if (Demod->SOF_corr > Demod->SOF_corr_prev + SUBCARRIER_DETECT_THRESHOLD) {
				Demod->posCount++;
			} else {
				Demod->posCount = 0;
			}

			if (Demod->posCount == 10) {	// correlation increased 10 times
				Demod->state = DEMOD_AWAIT_SOF_2;
			}
				
			Demod->SOF_corr_prev = Demod->SOF_corr;
				
			break;

		case DEMOD_AWAIT_SOF_2:
			// calculate the correlation in real time. Look at differences only.
			Demod->SOF_corr += Demod->SOF_correlator[Demod->SOF_low++];
			Demod->SOF_corr -= 2*Demod->SOF_correlator[Demod->SOF_high++];
			Demod->SOF_last++;
			Demod->SOF_low &= (SOF_CORRELATOR_LEN-1);
			Demod->SOF_high &= (SOF_CORRELATOR_LEN-1);
			Demod->SOF_last &= (SOF_CORRELATOR_LEN-1);
			Demod->SOF_correlator[Demod->SOF_last] = AMPLITUDE(ci,cq);
			Demod->SOF_corr += Demod->SOF_correlator[Demod->SOF_last];

			if (Demod->SOF_corr >= Demod->SOF_corr_prev) { // we are looking for the maximum correlation
				Demod->SOF_corr_prev = Demod->SOF_corr;
			} else {
				Demod->lastBit = SOF_PART1;  // detected 1st part of SOF
				Demod->sum1 = Demod->SOF_correlator[Demod->SOF_last];
				Demod->sum2 = 0;
				Demod->posCount = 2;
				Demod->state = DEMOD_RECEIVING_DATA;
				LED_C_ON();
			}
			
			break;

		case DEMOD_RECEIVING_DATA:
			if (Demod->posCount == 1) {
				Demod->sum1 = 0;
				Demod->sum2 = 0;
			}

			if (Demod->posCount <= 4) {
				Demod->sum1 += AMPLITUDE(ci, cq);
			} else {
				Demod->sum2 += AMPLITUDE(ci, cq);
			}

			if (Demod->posCount == 8) {
				int16_t corr_1 = (Demod->sum2 - Demod->sum1) / 4;
				int16_t corr_0 = (Demod->sum1 - Demod->sum2) / 4;
				int16_t corr_EOF = (Demod->sum1 + Demod->sum2) / 8;
				if (corr_EOF > corr_0 && corr_EOF > corr_1) {
					Demod->state = DEMOD_AWAIT_EOF;
				} else if (corr_1 > corr_0) {
					// logic 1
					if (Demod->lastBit == SOF_PART1) { // still part of SOF
						Demod->lastBit = SOF_PART2;
					} else {
						Demod->lastBit = LOGIC1;
						Demod->shiftReg >>= 1;
						Demod->shiftReg |= 0x80;
						Demod->bitCount++;
						if (Demod->bitCount == 8) {
							Demod->output[Demod->len] = Demod->shiftReg;
							Demod->len++;
							Demod->bitCount = 0;
							Demod->shiftReg = 0;
						}
					}
				} else {
					// logic 0
					if (Demod->lastBit == SOF_PART1) { // incomplete SOF
						Demod->state = DEMOD_UNSYNCD;
						LED_C_OFF();
					} else {
						Demod->lastBit = LOGIC0;
						Demod->shiftReg >>= 1;
						Demod->bitCount++;
						if (Demod->bitCount == 8) {
							Demod->output[Demod->len] = Demod->shiftReg;
							Demod->len++;
							Demod->bitCount = 0;
							Demod->shiftReg = 0;
						}
					}
				}
				Demod->posCount = 0;
			}
			Demod->posCount++;
			break;
		
		case DEMOD_AWAIT_EOF:
			if (Demod->lastBit == LOGIC0) {  // this was already part of EOF 
				LED_C_OFF();
				return true;
			} else {
				Demod->state = DEMOD_UNSYNCD;
				LED_C_OFF();
			}
			break;

		default:
			Demod->state = DEMOD_UNSYNCD;
			LED_C_OFF();
			break;
	}

	return false;
}


static void DemodInit(Demod_t* Demod, uint8_t* data)
{
	Demod->output = data;
	Demod->state = DEMOD_UNSYNCD;
}


/*
 *  Demodulate the samples we received from the tag, also log to tracebuffer
 */
static int GetIso15693AnswerFromTag(uint8_t* response, int timeout)
{
	int maxBehindBy = 0;
	int lastRxCounter, samples = 0;
	int8_t ci, cq;
	bool gotFrame = false;
	
	// Allocate memory from BigBuf for some buffers
	// free all previous allocations first
	BigBuf_free();

	// The DMA buffer, used to stream samples from the FPGA
	uint16_t* dmaBuf = (uint16_t*) BigBuf_malloc(ISO15693_DMA_BUFFER_SIZE * sizeof(uint16_t));

	// the Demodulatur data structure
	Demod_t* Demod = (Demod_t*) BigBuf_malloc(sizeof(Demod_t));
	
	// Set up the demodulator for tag -> reader responses.
	DemodInit(Demod, response);

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

		if (Handle15693SamplesDemod(ci, cq, Demod)) {
			gotFrame = true;
			break;
		}

		if(samples > timeout && Demod->state < DEMOD_RECEIVING_DATA) {
			Demod->len = 0;
			break;
		}
	}

	FpgaDisableSscDma();

	if (DEBUG) Dbprintf("max behindby = %d, samples = %d, gotFrame = %d, Demod.state = %d, Demod.len = %d, Demod.bitCount = %d, Demod.posCount = %d", 
	                    maxBehindBy, samples, gotFrame, Demod->state, Demod->len, Demod->bitCount, Demod->posCount);

	if (tracing && Demod->len > 0) {
		uint8_t parity[MAX_PARITY_SIZE];
		LogTrace(Demod->output, Demod->len, 0, 0, parity, false);
	}

	return Demod->len;
}


// Now the GetISO15693 message from sniffing command
// TODO: fix it. This cannot work for several reasons: 
//   1. Carrier is switched on during sniffing?
//   2. We most probable miss the next reader command when demodulating
static int GetIso15693AnswerFromSniff(uint8_t *receivedResponse, int maxLen, int *samples, int *elapsed)
{
	uint8_t *dest = BigBuf_get_addr();

// NOW READ RESPONSE
	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);
	//spindelay(60);	// greg - experiment to get rid of some of the 0 byte/failed reads
	for(int c = 0; c < BIGBUF_SIZE; ) {
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

	//////////////////////////////////////////
	/////////// DEMODULATE ///////////////////
	//////////////////////////////////////////

	int i, j;
	int max = 0, maxPos=0;

	int skip = 2;

	// First, correlate for SOF
	for(i = 0; i < 38000; i++) {
		int corr = 0;
		for(j = 0; j < arraylen(FrameSOF); j += skip) {
			corr += FrameSOF[j]*dest[i+(j/skip)];
		}
		if(corr > max) {
			max = corr;
			maxPos = i;
		}
	}

	if (DEBUG) Dbprintf("SOF at %d, correlation %d", maxPos,max/(arraylen(FrameSOF)/skip));

	int k = 0; // this will be our return value

	// greg - If correlation is less than 1 then there's little point in continuing
	if ((max/(arraylen(FrameSOF)/skip)) >= 1)	// THIS SHOULD BE 1
	{
	
		i = maxPos + arraylen(FrameSOF)/skip;
	
		uint8_t outBuf[20];
		memset(outBuf, 0, sizeof(outBuf));
		uint8_t mask = 0x01;
		for(;;) {
			int corr0 = 0, corr00 = 0, corr01 = 0, corr1 = 0, corrEOF = 0;
			for(j = 0; j < arraylen(Logic0); j += skip) {
				corr0 += Logic0[j]*dest[i+(j/skip)];
			}
			corr01 = corr00 = corr0;
			for(j = 0; j < arraylen(Logic0); j += skip) {
				corr00 += Logic0[j]*dest[i+arraylen(Logic0)/skip+(j/skip)];
				corr01 += Logic1[j]*dest[i+arraylen(Logic0)/skip+(j/skip)];
			}
			for(j = 0; j < arraylen(Logic1); j += skip) {
				corr1 += Logic1[j]*dest[i+(j/skip)];
			}
			for(j = 0; j < arraylen(FrameEOF); j += skip) {
				corrEOF += FrameEOF[j]*dest[i+(j/skip)];
			}
			// Even things out by the length of the target waveform.
			corr00 *= 2;
			corr01 *= 2;
			corr0 *= 4;
			corr1 *= 4;
	
			if(corrEOF > corr1 && corrEOF > corr00 && corrEOF > corr01) {
				if (DEBUG) Dbprintf("EOF at %d, correlation %d (corr01: %d, corr00: %d, corr1: %d, corr0: %d)", 
				         i, corrEOF, corr01, corr00, corr1, corr0);
				break;
			} else if(corr1 > corr0) {
				i += arraylen(Logic1)/skip;
				outBuf[k] |= mask;
			} else {
				i += arraylen(Logic0)/skip;
			}
			mask <<= 1;
			if(mask == 0) {
				k++;
				mask = 0x01;
			}
			if((i+(int)arraylen(FrameEOF)/skip) >= BIGBUF_SIZE) {
				DbpString("ran off end!");
				break;
			}
		}
		if(mask != 0x01) {
			DbpString("sniff: error, uneven octet! (discard extra bits!)");
	///		DbpString("   mask=%02x", mask);
		}
	//	uint8_t str1 [8];
	//	itoa(k,str1);
	//	strncat(str1," octets read",8);
	
	//	DbpString(  str1);    // DbpString("%d octets", k);
	
	//	for(i = 0; i < k; i+=3) {
	//		//DbpString("# %2d: %02x ", i, outBuf[i]);
	//		DbpIntegers(outBuf[i],outBuf[i+1],outBuf[i+2]);
	//	}
	
		for(i = 0; i < k; i++) {
			receivedResponse[i] = outBuf[i];
		}
	} // "end if correlation > 0" 	(max/(arraylen(FrameSOF)/skip))
	return k; // return the number of bytes demodulated

///	DbpString("CRC=%04x", Iso15693Crc(outBuf, k-2));
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
	crc = Crc(cmd, 11); // the crc needs to be calculated over 12 bytes
	cmd[11] = crc & 0xff;
	cmd[12] = crc >> 8;

	CodeIso15693AsReader(cmd, sizeof(cmd));
}


// Now the VICC>VCD responses when we are simulating a tag
static void BuildInventoryResponse( uint8_t *uid)
{
	uint8_t cmd[12];

	uint16_t crc;
	// one sub-carrier, inventory, 1 slot, fast rate
	// AFI is at bit 5 (1<<4) when doing an INVENTORY
    //(1 << 2) | (1 << 5) | (1 << 1);
	cmd[0] = 0; // 
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

	CodeIso15693AsReader(cmd, sizeof(cmd));
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
	DbdecodeIso15693Answer(answerLen1,answer1);
	Dbhexdump(answerLen1,answer1,true);

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
		int i=0;			
		while (i<32) {  // sanity check, assume max 32 pages
			BuildReadBlockRequest(TagUID,i);
			TransmitTo15693Tag(ToSend,ToSendMax);  
			int answerLen2 = GetIso15693AnswerFromTag(answer2, 100);
			if (answerLen2>0) {
				Dbprintf("READ SINGLE BLOCK %d returned %d octets:",i,answerLen2);
				DbdecodeIso15693Answer(answerLen2,answer2);
				Dbhexdump(answerLen2,answer2,true);
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

// Simulate an ISO15693 TAG, perform anti-collision and then print any reader commands
// all demodulation performed in arm rather than host. - greg
void SimTagIso15693(uint32_t parameter, uint8_t *uid)
{
	LEDsoff();
	LED_A_ON();

	int answerLen1 = 0;
	int samples = 0;
	int elapsed = 0;

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);

	uint8_t *buf = BigBuf_get_addr() + 4000;
	memset(buf, 0x00, 100);
	
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);

	// Start from off (no field generated)
   	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	SpinDelay(200);

	// Listen to reader
	answerLen1 = GetIso15693AnswerFromSniff(buf, 100, &samples, &elapsed) ;

	if (answerLen1 >=1) // we should do a better check than this
	{
		// Build a suitable reponse to the reader INVENTORY cocmmand
		// not so obsvious, but in the call to BuildInventoryResponse,  the command is copied to the global ToSend buffer used below.
		
		BuildInventoryResponse(uid);
	
		TransmitTo15693Reader(ToSend,ToSendMax);
	}

	Dbprintf("%d octets read from reader command: %x %x %x %x %x %x %x %x %x", answerLen1,
		buf[0], buf[1], buf[2],	buf[3],
		buf[4], buf[5],	buf[6], buf[7], buf[8]);

	Dbprintf("Simulationg uid: %x %x %x %x %x %x %x %x",
		uid[0], uid[1], uid[2],	uid[3],
		uid[4], uid[5],	uid[6], uid[7]);

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
	// Tags should respond wihtout AFI and with AFI=0 even when AFI is active
	
	data[0]=ISO15_REQ_SUBCARRIER_SINGLE | ISO15_REQ_DATARATE_HIGH | 
	        ISO15_REQ_INVENTORY | ISO15_REQINV_SLOT1;
	data[1]=ISO15_CMD_INVENTORY;
	data[2]=0; // mask length
	datalen=AddCrc(data,3);
	recvlen=SendDataTag(data, datalen, false, speed, &recv);
	WDT_HIT();
	if (recvlen>=12) {
		Dbprintf("NoAFI UID=%s",sprintUID(NULL,&recv[2]));
	}
	
	// now with AFI
	
	data[0]=ISO15_REQ_SUBCARRIER_SINGLE | ISO15_REQ_DATARATE_HIGH | 
	        ISO15_REQ_INVENTORY | ISO15_REQINV_AFI | ISO15_REQINV_SLOT1;
	data[1]=ISO15_CMD_INVENTORY;
	data[2]=0; // AFI
	data[3]=0; // mask length
	
	for (int i=0;i<256;i++) {
		data[2]=i & 0xFF;
		datalen=AddCrc(data,4);
		recvlen=SendDataTag(data, datalen, false, speed, &recv);
		WDT_HIT();
		if (recvlen>=12) {
			Dbprintf("AFI=%i UID=%s",i,sprintUID(NULL,&recv[2]));
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
		Dbhexdump(datalen,data,true);
	}
	
	recvlen = SendDataTag(data, datalen, true, speed, (recv?&recvbuf:NULL));

	if (recv) { 
		cmd_send(CMD_ACK, recvlen>48?48:recvlen, 0, 0, recvbuf, 48);
		
		if (DEBUG) {
			Dbprintf("RECV");
			DbdecodeIso15693Answer(recvlen,recvbuf); 
			Dbhexdump(recvlen,recvbuf,true);
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


