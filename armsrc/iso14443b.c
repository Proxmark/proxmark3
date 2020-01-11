//-----------------------------------------------------------------------------
// Jonathan Westhues, split Nov 2006
// piwi 2018
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support ISO 14443B. This includes both the reader software and
// the `fake tag' modes.
//-----------------------------------------------------------------------------

#include "iso14443b.h"

#include "proxmark3.h"
#include "apps.h"
#include "usb_cdc.h"
#include "util.h"
#include "string.h"
#include "iso14443crc.h"
#include "fpgaloader.h"
#include "BigBuf.h"

#define RECEIVE_SAMPLES_TIMEOUT 64 // TR0 max is 256/fs = 256/(848kHz) = 302us or 64 samples from FPGA
#define ISO14443B_DMA_BUFFER_SIZE 128

// PCB Block number for APDUs
static uint8_t pcb_blocknum = 0;

//=============================================================================
// An ISO 14443 Type B tag. We listen for commands from the reader, using
// a UART kind of thing that's implemented in software. When we get a
// frame (i.e., a group of bytes between SOF and EOF), we check the CRC.
// If it's good, then we can do something appropriate with it, and send
// a response.
//=============================================================================

//-----------------------------------------------------------------------------
// Code up a string of octets at layer 2 (including CRC, we don't generate
// that here) so that they can be transmitted to the reader. Doesn't transmit
// them yet, just leaves them ready to send in ToSend[].
//-----------------------------------------------------------------------------
static void CodeIso14443bAsTag(const uint8_t *cmd, int len)
{
	int i;

	ToSendReset();

	// Transmit a burst of ones, as the initial thing that lets the
	// reader get phase sync. This (TR1) must be > 80/fs, per spec,
	// but tag that I've tried (a Paypass) exceeds that by a fair bit,
	// so I will too.
	for(i = 0; i < 20; i++) {
		ToSendStuffBit(1);
		ToSendStuffBit(1);
		ToSendStuffBit(1);
		ToSendStuffBit(1);
	}

	// Send SOF.
	for(i = 0; i < 10; i++) {
		ToSendStuffBit(0);
		ToSendStuffBit(0);
		ToSendStuffBit(0);
		ToSendStuffBit(0);
	}
	for(i = 0; i < 2; i++) {
		ToSendStuffBit(1);
		ToSendStuffBit(1);
		ToSendStuffBit(1);
		ToSendStuffBit(1);
	}

	for(i = 0; i < len; i++) {
		int j;
		uint8_t b = cmd[i];

		// Start bit
		ToSendStuffBit(0);
		ToSendStuffBit(0);
		ToSendStuffBit(0);
		ToSendStuffBit(0);

		// Data bits
		for(j = 0; j < 8; j++) {
			if(b & 1) {
				ToSendStuffBit(1);
				ToSendStuffBit(1);
				ToSendStuffBit(1);
				ToSendStuffBit(1);
			} else {
				ToSendStuffBit(0);
				ToSendStuffBit(0);
				ToSendStuffBit(0);
				ToSendStuffBit(0);
			}
			b >>= 1;
		}

		// Stop bit
		ToSendStuffBit(1);
		ToSendStuffBit(1);
		ToSendStuffBit(1);
		ToSendStuffBit(1);
	}

	// Send EOF.
	for(i = 0; i < 10; i++) {
		ToSendStuffBit(0);
		ToSendStuffBit(0);
		ToSendStuffBit(0);
		ToSendStuffBit(0);
	}
	for(i = 0; i < 2; i++) {
		ToSendStuffBit(1);
		ToSendStuffBit(1);
		ToSendStuffBit(1);
		ToSendStuffBit(1);
	}

	// Convert from last byte pos to length
	ToSendMax++;
}

//-----------------------------------------------------------------------------
// The software UART that receives commands from the reader, and its state
// variables.
//-----------------------------------------------------------------------------
static struct {
	enum {
		STATE_UNSYNCD,
		STATE_GOT_FALLING_EDGE_OF_SOF,
		STATE_AWAITING_START_BIT,
		STATE_RECEIVING_DATA
	}       state;
	uint16_t    shiftReg;
	int     bitCnt;
	int     byteCnt;
	int     byteCntMax;
	int     posCnt;
	uint8_t   *output;
} Uart;

/* Receive & handle a bit coming from the reader.
 *
 * This function is called 4 times per bit (every 2 subcarrier cycles).
 * Subcarrier frequency fs is 848kHz, 1/fs = 1,18us, i.e. function is called every 2,36us
 *
 * LED handling:
 * LED A -> ON once we have received the SOF and are expecting the rest.
 * LED A -> OFF once we have received EOF or are in error state or unsynced
 *
 * Returns: true if we received a EOF
 *          false if we are still waiting for some more
 */
static RAMFUNC int Handle14443bUartBit(uint8_t bit)
{
	switch(Uart.state) {
		case STATE_UNSYNCD:
			if(!bit) {
				// we went low, so this could be the beginning
				// of an SOF
				Uart.state = STATE_GOT_FALLING_EDGE_OF_SOF;
				Uart.posCnt = 0;
				Uart.bitCnt = 0;
			}
			break;

		case STATE_GOT_FALLING_EDGE_OF_SOF:
			Uart.posCnt++;
			if(Uart.posCnt == 2) {	// sample every 4 1/fs in the middle of a bit
				if(bit) {
					if(Uart.bitCnt > 9) {
						// we've seen enough consecutive
						// zeros that it's a valid SOF
						Uart.posCnt = 0;
						Uart.byteCnt = 0;
						Uart.state = STATE_AWAITING_START_BIT;
						LED_A_ON(); // Indicate we got a valid SOF
					} else {
						// didn't stay down long enough
						// before going high, error
						Uart.state = STATE_UNSYNCD;
					}
				} else {
					// do nothing, keep waiting
				}
				Uart.bitCnt++;
			}
			if(Uart.posCnt >= 4) Uart.posCnt = 0;
			if(Uart.bitCnt > 12) {
				// Give up if we see too many zeros without
				// a one, too.
				LED_A_OFF();
				Uart.state = STATE_UNSYNCD;
			}
			break;

		case STATE_AWAITING_START_BIT:
			Uart.posCnt++;
			if(bit) {
				if(Uart.posCnt > 50/2) {	// max 57us between characters = 49 1/fs, max 3 etus after low phase of SOF = 24 1/fs
					// stayed high for too long between
					// characters, error
					Uart.state = STATE_UNSYNCD;
				}
			} else {
				// falling edge, this starts the data byte
				Uart.posCnt = 0;
				Uart.bitCnt = 0;
				Uart.shiftReg = 0;
				Uart.state = STATE_RECEIVING_DATA;
			}
			break;

		case STATE_RECEIVING_DATA:
			Uart.posCnt++;
			if(Uart.posCnt == 2) {
				// time to sample a bit
				Uart.shiftReg >>= 1;
				if(bit) {
					Uart.shiftReg |= 0x200;
				}
				Uart.bitCnt++;
			}
			if(Uart.posCnt >= 4) {
				Uart.posCnt = 0;
			}
			if(Uart.bitCnt == 10) {
				if((Uart.shiftReg & 0x200) && !(Uart.shiftReg & 0x001))
				{
					// this is a data byte, with correct
					// start and stop bits
					Uart.output[Uart.byteCnt] = (Uart.shiftReg >> 1) & 0xff;
					Uart.byteCnt++;

					if(Uart.byteCnt >= Uart.byteCntMax) {
						// Buffer overflowed, give up
						LED_A_OFF();
						Uart.state = STATE_UNSYNCD;
					} else {
						// so get the next byte now
						Uart.posCnt = 0;
						Uart.state = STATE_AWAITING_START_BIT;
					}
				} else if (Uart.shiftReg == 0x000) {
					// this is an EOF byte
					LED_A_OFF(); // Finished receiving
					Uart.state = STATE_UNSYNCD;
					if (Uart.byteCnt != 0) {
						return true;
					}
				} else {
					// this is an error
					LED_A_OFF();
					Uart.state = STATE_UNSYNCD;
				}
			}
			break;

		default:
			LED_A_OFF();
			Uart.state = STATE_UNSYNCD;
			break;
	}

	return false;
}


static void UartReset()
{
	Uart.byteCntMax = MAX_FRAME_SIZE;
	Uart.state = STATE_UNSYNCD;
	Uart.byteCnt = 0;
	Uart.bitCnt = 0;
}


static void UartInit(uint8_t *data)
{
	Uart.output = data;
	UartReset();
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
static int GetIso14443bCommandFromReader(uint8_t *received, uint16_t *len)
{
	// Set FPGA mode to "simulated ISO 14443B tag", no modulation (listen
	// only, since we are receiving, not transmitting).
	// Signal field is off with the appropriate LED
	LED_D_OFF();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_NO_MODULATION);

	// Now run a `software UART' on the stream of incoming samples.
	UartInit(received);

	for(;;) {
		WDT_HIT();

		if(BUTTON_PRESS()) return false;

		if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_RXRDY)) {
			uint8_t b = (uint8_t)AT91C_BASE_SSC->SSC_RHR;
			for(uint8_t mask = 0x80; mask != 0x00; mask >>= 1) {
				if(Handle14443bUartBit(b & mask)) {
					*len = Uart.byteCnt;
					return true;
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Main loop of simulated tag: receive commands from reader, decide what
// response to send, and send it.
//-----------------------------------------------------------------------------
void SimulateIso14443bTag(void)
{
	LED_A_ON();
	// the only commands we understand is WUPB, AFI=0, Select All, N=1:
	static const uint8_t cmd1[] = { 0x05, 0x00, 0x08, 0x39, 0x73 }; // WUPB
	// ... and REQB, AFI=0, Normal Request, N=1:
	static const uint8_t cmd2[] = { 0x05, 0x00, 0x00, 0x71, 0xFF }; // REQB
	// ... and HLTB
	static const uint8_t cmd3[] = { 0x50, 0xff, 0xff, 0xff, 0xff }; // HLTB
	// ... and ATTRIB
	static const uint8_t cmd4[] = { 0x1D, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // ATTRIB

	// ... and we always respond with ATQB, PUPI = 820de174, Application Data = 0x20381922,
	// supports only 106kBit/s in both directions, max frame size = 32Bytes,
	// supports ISO14443-4, FWI=8 (77ms), NAD supported, CID not supported:
	static const uint8_t response1[] = {
		0x50, 0x82, 0x0d, 0xe1, 0x74, 0x20, 0x38, 0x19, 0x22,
		0x00, 0x21, 0x85, 0x5e, 0xd7
	};
	// response to HLTB and ATTRIB
	static const uint8_t response2[] = {0x00, 0x78, 0xF0};


	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);

	clear_trace();
	set_tracing(true);

	const uint8_t *resp;
	uint8_t *respCode;
	uint16_t respLen, respCodeLen;

	// allocate command receive buffer
	BigBuf_free();
	uint8_t *receivedCmd = BigBuf_malloc(MAX_FRAME_SIZE);

	uint16_t len;
	uint16_t cmdsRecvd = 0;

	// prepare the (only one) tag answer:
	CodeIso14443bAsTag(response1, sizeof(response1));
	uint8_t *resp1Code = BigBuf_malloc(ToSendMax);
	memcpy(resp1Code, ToSend, ToSendMax);
	uint16_t resp1CodeLen = ToSendMax;

	// prepare the (other) tag answer:
	CodeIso14443bAsTag(response2, sizeof(response2));
	uint8_t *resp2Code = BigBuf_malloc(ToSendMax);
	memcpy(resp2Code, ToSend, ToSendMax);
	uint16_t resp2CodeLen = ToSendMax;

	// We need to listen to the high-frequency, peak-detected path.
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_SIMULATOR);

	cmdsRecvd = 0;

	for(;;) {

		if(!GetIso14443bCommandFromReader(receivedCmd, &len)) {
			Dbprintf("button pressed, received %d commands", cmdsRecvd);
			break;
		}

		LogTrace(receivedCmd, len, 0, 0, NULL, true);

		// Good, look at the command now.
		if ( (len == sizeof(cmd1) && memcmp(receivedCmd, cmd1, len) == 0)
			|| (len == sizeof(cmd2) && memcmp(receivedCmd, cmd2, len) == 0) ) {
			resp = response1;
			respLen = sizeof(response1);
			respCode = resp1Code;
			respCodeLen = resp1CodeLen;
		} else if ( (len == sizeof(cmd3) && receivedCmd[0] == cmd3[0])
			|| (len == sizeof(cmd4) && receivedCmd[0] == cmd4[0]) ) {
			resp = response2;
			respLen = sizeof(response2);
			respCode = resp2Code;
			respCodeLen = resp2CodeLen;
		} else {
			Dbprintf("new cmd from reader: len=%d, cmdsRecvd=%d", len, cmdsRecvd);
			// And print whether the CRC fails, just for good measure
			uint8_t b1, b2;
			if (len >= 3){ // if crc exists
				ComputeCrc14443(CRC_14443_B, receivedCmd, len-2, &b1, &b2);
				if(b1 != receivedCmd[len-2] || b2 != receivedCmd[len-1]) {
					// Not so good, try again.
					DbpString("+++CRC fail");
		
				} else {
					DbpString("CRC passes");
				}
			}
			//get rid of compiler warning
			respCodeLen = 0;
			resp = response1;
			respLen	= 0;
			respCode = resp1Code;
			//don't crash at new command just wait and see if reader will send other new cmds.
			//break;
		}

		cmdsRecvd++;

		if(cmdsRecvd > 0x30) {
			DbpString("many commands later...");
			break;
		}

		if(respCodeLen <= 0) continue;

		// Modulate BPSK
		// Signal field is off with the appropriate LED
		LED_D_OFF();
		FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SIMULATOR | FPGA_HF_SIMULATOR_MODULATE_BPSK);
		AT91C_BASE_SSC->SSC_THR = 0xff;
		FpgaSetupSsc(FPGA_MAJOR_MODE_HF_SIMULATOR);

		// Transmit the response.
		uint16_t i = 0;
		for(;;) {
			if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_TXRDY)) {
				uint8_t b = respCode[i];

				AT91C_BASE_SSC->SSC_THR = b;

				i++;
				if(i > respCodeLen) {
					break;
				}
			}
			if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_RXRDY)) {
				volatile uint8_t b = (uint8_t)AT91C_BASE_SSC->SSC_RHR;
				(void)b;
			}
		}

		// trace the response:
		LogTrace(resp, respLen, 0, 0, NULL, false);

	}
	
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_A_OFF();
}

//=============================================================================
// An ISO 14443 Type B reader. We take layer two commands, code them
// appropriately, and then send them to the tag. We then listen for the
// tag's response, which we leave in the buffer to be demodulated on the
// PC side.
//=============================================================================

static struct {
	enum {
		DEMOD_UNSYNCD,
		DEMOD_PHASE_REF_TRAINING,
		DEMOD_AWAITING_FALLING_EDGE_OF_SOF,
		DEMOD_GOT_FALLING_EDGE_OF_SOF,
		DEMOD_AWAITING_START_BIT,
		DEMOD_RECEIVING_DATA
	}       state;
	int     bitCount;
	int     posCount;
	int     thisBit;
/* this had been used to add RSSI (Received Signal Strength Indication) to traces. Currently not implemented.
	int     metric;
	int     metricN;
*/
	uint16_t    shiftReg;
	uint8_t   *output;
	int     len;
	int     sumI;
	int     sumQ;
} Demod;

/*
 * Handles reception of a bit from the tag
 *
 * This function is called 2 times per bit (every 4 subcarrier cycles).
 * Subcarrier frequency fs is 848kHz, 1/fs = 1,18us, i.e. function is called every 4,72us
 *
 * LED handling:
 * LED C -> ON once we have received the SOF and are expecting the rest.
 * LED C -> OFF once we have received EOF or are unsynced
 *
 * Returns: true if we received a EOF
 *          false if we are still waiting for some more
 *
 */
static RAMFUNC int Handle14443bSamplesDemod(int ci, int cq)
{
	int v;

// The soft decision on the bit uses an estimate of just the
// quadrant of the reference angle, not the exact angle.
#define MAKE_SOFT_DECISION() { \
		if(Demod.sumI > 0) { \
			v = ci; \
		} else { \
			v = -ci; \
		} \
		if(Demod.sumQ > 0) { \
			v += cq; \
		} else { \
			v -= cq; \
		} \
	}

#define SUBCARRIER_DETECT_THRESHOLD	8

// Subcarrier amplitude v = sqrt(ci^2 + cq^2), approximated here by max(abs(ci),abs(cq)) + 1/2*min(abs(ci),abs(cq)))
#define AMPLITUDE(ci,cq) (MAX(ABS(ci),ABS(cq)) + (MIN(ABS(ci),ABS(cq))/2))
	switch(Demod.state) {
		case DEMOD_UNSYNCD:
			if(AMPLITUDE(ci,cq) > SUBCARRIER_DETECT_THRESHOLD) {	// subcarrier detected
				Demod.state = DEMOD_PHASE_REF_TRAINING;
				Demod.sumI = ci;
				Demod.sumQ = cq;
				Demod.posCount = 1;
				}
			break;

		case DEMOD_PHASE_REF_TRAINING:
			if(Demod.posCount < 8) {
				if (AMPLITUDE(ci,cq) > SUBCARRIER_DETECT_THRESHOLD) {
					// set the reference phase (will code a logic '1') by averaging over 32 1/fs.
					// note: synchronization time > 80 1/fs
					Demod.sumI += ci;
					Demod.sumQ += cq;
					Demod.posCount++;
				} else {		// subcarrier lost
					Demod.state = DEMOD_UNSYNCD;
				}
			} else {
				Demod.state = DEMOD_AWAITING_FALLING_EDGE_OF_SOF;
			}
			break;

		case DEMOD_AWAITING_FALLING_EDGE_OF_SOF:
			MAKE_SOFT_DECISION();
			if(v < 0) {	// logic '0' detected
				Demod.state = DEMOD_GOT_FALLING_EDGE_OF_SOF;
				Demod.posCount = 0;	// start of SOF sequence
			} else {
				if(Demod.posCount > 200/4) {	// maximum length of TR1 = 200 1/fs
					Demod.state = DEMOD_UNSYNCD;
				}
			}
			Demod.posCount++;
			break;

		case DEMOD_GOT_FALLING_EDGE_OF_SOF:
			Demod.posCount++;
			MAKE_SOFT_DECISION();
			if(v > 0) {
				if(Demod.posCount < 9*2) { // low phase of SOF too short (< 9 etu). Note: spec is >= 10, but FPGA tends to "smear" edges
					Demod.state = DEMOD_UNSYNCD;
				} else {
					LED_C_ON(); // Got SOF
					Demod.posCount = 0;
					Demod.bitCount = 0;
					Demod.len = 0;
					Demod.state = DEMOD_AWAITING_START_BIT;
/* this had been used to add RSSI (Received Signal Strength Indication) to traces. Currently not implemented.
					Demod.metricN = 0;
					Demod.metric = 0;
*/
				}
			} else {
				if(Demod.posCount > 12*2) { // low phase of SOF too long (> 12 etu)
					Demod.state = DEMOD_UNSYNCD;
					LED_C_OFF();
				}
			}
			break;

		case DEMOD_AWAITING_START_BIT:
			Demod.posCount++;
			MAKE_SOFT_DECISION();
			if (v > 0) {
				if (Demod.posCount > 3*2) { 		// max 19us between characters = 16 1/fs, max 3 etu after low phase of SOF = 24 1/fs
					LED_C_OFF();
					if (Demod.bitCount == 0 && Demod.len == 0) { // received SOF only, this is valid for iClass/Picopass
						return true;
					} else {
						Demod.state = DEMOD_UNSYNCD;
					}
				}
			} else {							// start bit detected
				Demod.posCount = 1;				// this was the first half
				Demod.thisBit = v;
				Demod.shiftReg = 0;
				Demod.state = DEMOD_RECEIVING_DATA;
			}
			break;

		case DEMOD_RECEIVING_DATA:
			MAKE_SOFT_DECISION();
			if (Demod.posCount == 0) { 			// first half of bit
				Demod.thisBit = v;
				Demod.posCount = 1;
			} else {							// second half of bit
				Demod.thisBit += v;

/* this had been used to add RSSI (Received Signal Strength Indication) to traces. Currently not implemented.
				if(Demod.thisBit > 0) {
					Demod.metric += Demod.thisBit;
				} else {
					Demod.metric -= Demod.thisBit;
				}
				(Demod.metricN)++;
*/

				Demod.shiftReg >>= 1;
				if (Demod.thisBit > 0) {	// logic '1'
					Demod.shiftReg |= 0x200;
				}

				Demod.bitCount++;
				if (Demod.bitCount == 10) {
					uint16_t s = Demod.shiftReg;
					if ((s & 0x200) && !(s & 0x001)) { // stop bit == '1', start bit == '0'
						uint8_t b = (s >> 1);
						Demod.output[Demod.len] = b;
						Demod.len++;
						Demod.bitCount = 0;
						Demod.state = DEMOD_AWAITING_START_BIT;
					} else {
						Demod.state = DEMOD_UNSYNCD;
						LED_C_OFF();
						if (s == 0x000) {
							// This is EOF (start, stop and all data bits == '0'
							return true;
						}
					}
				}
				Demod.posCount = 0;
			}
			break;

		default:
			Demod.state = DEMOD_UNSYNCD;
			LED_C_OFF();
			break;
	}

	return false;
}


static void DemodReset()
{
	// Clear out the state of the "UART" that receives from the tag.
	Demod.len = 0;
	Demod.state = DEMOD_UNSYNCD;
	Demod.posCount = 0;
	memset(Demod.output, 0x00, MAX_FRAME_SIZE);
}


static void DemodInit(uint8_t *data)
{
	Demod.output = data;
	DemodReset();
}


/*
 *  Demodulate the samples we received from the tag, also log to tracebuffer
 *  quiet: set to 'true' to disable debug output
 */
static int GetSamplesFor14443bDemod(int timeout, bool quiet) {
	int ret = 0;
	int maxBehindBy = 0;
	bool gotFrame = false;
	int lastRxCounter, samples = 0;
	int8_t ci, cq;
	
	// Allocate memory from BigBuf for some buffers
	// free all previous allocations first
	BigBuf_free();

	// The response (tag -> reader) that we're receiving.
	uint8_t *receivedResponse = BigBuf_malloc(MAX_FRAME_SIZE);

	// The DMA buffer, used to stream samples from the FPGA
	uint16_t *dmaBuf = (uint16_t*) BigBuf_malloc(ISO14443B_DMA_BUFFER_SIZE * sizeof(uint16_t));

	// Set up the demodulator for tag -> reader responses.
	DemodInit(receivedResponse);

	// wait for last transfer to complete
	while (!(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_TXEMPTY))

	// Setup and start DMA.
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	FpgaSetupSscDma((uint8_t*) dmaBuf, ISO14443B_DMA_BUFFER_SIZE);

	uint16_t *upTo = dmaBuf;
	lastRxCounter = ISO14443B_DMA_BUFFER_SIZE;

	// Signal field is ON with the appropriate LED:
	LED_D_ON();
	// And put the FPGA in the appropriate mode
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_SUBCARRIER_848_KHZ | FPGA_HF_READER_MODE_RECEIVE_IQ);

	for(;;) {
		int behindBy = (lastRxCounter - AT91C_BASE_PDC_SSC->PDC_RCR) & (ISO14443B_DMA_BUFFER_SIZE-1);
		if(behindBy > maxBehindBy) {
			maxBehindBy = behindBy;
		}

		if(behindBy < 1) continue;

		ci = *upTo >> 8;
		cq = *upTo;
		upTo++;
		lastRxCounter--;
		if(upTo >= dmaBuf + ISO14443B_DMA_BUFFER_SIZE) {               // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                             // start reading the circular buffer from the beginning
			lastRxCounter += ISO14443B_DMA_BUFFER_SIZE;
		}
		if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
			AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
			AT91C_BASE_PDC_SSC->PDC_RNCR = ISO14443B_DMA_BUFFER_SIZE;  // DMA Next Counter registers
		}
		samples++;

		if (Handle14443bSamplesDemod(ci, cq)) {
			ret = Demod.len;
			gotFrame = true;
			break;
		}

		if(samples > timeout && Demod.state < DEMOD_PHASE_REF_TRAINING) {
			ret = -1;
			LED_C_OFF();
			break;
		}
	}

	FpgaDisableSscDma();

	if (!quiet) Dbprintf("max behindby = %d, samples = %d, gotFrame = %d, Demod.len = %d, Demod.sumI = %d, Demod.sumQ = %d", maxBehindBy, samples, gotFrame, Demod.len, Demod.sumI, Demod.sumQ);

	if (ret < 0) {
		return ret;
	}
 	//Tracing
	LogTrace(Demod.output, Demod.len, 0, 0, NULL, false);

	return ret;
}


//-----------------------------------------------------------------------------
// Transmit the command (to the tag) that was placed in ToSend[].
//-----------------------------------------------------------------------------
static void TransmitFor14443b(void)
{
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SEND_SHALLOW_MOD);
	LED_B_ON();
	for(int c = 0; c < ToSendMax; c++) {
		uint8_t data = ToSend[c];
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
// Code a layer 2 command (string of octets, including CRC) into ToSend[],
// so that it is ready to transmit to the tag using TransmitFor14443b().
//-----------------------------------------------------------------------------
static void CodeIso14443bAsReader(const uint8_t *cmd, int len)
{
	int i, j;
	uint8_t b;

	ToSendReset();

	// Send SOF
	for(i = 0; i < 10; i++) {
		ToSendStuffBit(0);
	}
	ToSendStuffBit(1);
	ToSendStuffBit(1);

	for(i = 0; i < len; i++) {
		// Start bit
		ToSendStuffBit(0);
		// Data bits
		b = cmd[i];
		for(j = 0; j < 8; j++) {
			if(b & 1) {
				ToSendStuffBit(1);
			} else {
				ToSendStuffBit(0);
			}
			b >>= 1;
		}
		// Stop bit
		ToSendStuffBit(1);
	}

	// Send EOF
	for(i = 0; i < 10; i++) {
		ToSendStuffBit(0);
	}
	ToSendStuffBit(1);

	// ensure that last byte is filled up
	for(i = 0; i < 8; i++) {
		ToSendStuffBit(1);
	}

	// Convert from last character reference to length
	ToSendMax++;
}


/**
  Convenience function to encode, transmit and trace iso 14443b comms
  **/
static void CodeAndTransmit14443bAsReader(const uint8_t *cmd, int len)
{
	CodeIso14443bAsReader(cmd, len);
	TransmitFor14443b();
	LogTrace(cmd,len, 0, 0, NULL, true);
}

/* Sends an APDU to the tag
 * TODO: check CRC and preamble
 */
int iso14443b_apdu(uint8_t const *message, size_t message_length, uint8_t *response) {
	LED_A_ON();
	uint8_t message_frame[message_length + 4];
	// PCB
	message_frame[0] = 0x0A | pcb_blocknum;
	pcb_blocknum ^= 1;
	// CID
	message_frame[1] = 0;
	// INF
	memcpy(message_frame + 2, message, message_length);
	// EDC (CRC)
	ComputeCrc14443(CRC_14443_B, message_frame, message_length + 2, &message_frame[message_length + 2], &message_frame[message_length + 3]);
	// send
	CodeAndTransmit14443bAsReader(message_frame, message_length + 4);
	// get response
	int ret = GetSamplesFor14443bDemod(RECEIVE_SAMPLES_TIMEOUT, true);
	FpgaDisableTracing();
	if (ret < 3) {
		LED_A_OFF();
		return 0;
	}
	// TODO: Check CRC
	// copy response contents
	if (response != NULL) {
		memcpy(response, Demod.output, Demod.len);
	}
	LED_A_OFF();
	return ret;
}

/* Perform the ISO 14443 B Card Selection procedure
 * Currently does NOT do any collision handling.
 * It expects 0-1 cards in the device's range.
 * TODO: Support multiple cards (perform anticollision)
 * TODO: Verify CRC checksums
 */
int iso14443b_select_card()
{
	// WUPB command (including CRC)
	// Note: WUPB wakes up all tags, REQB doesn't wake up tags in HALT state
	static const uint8_t wupb[] = { 0x05, 0x00, 0x08, 0x39, 0x73 };
	// ATTRIB command (with space for CRC)
	uint8_t attrib[] = { 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};

	// first, wake up the tag
	CodeAndTransmit14443bAsReader(wupb, sizeof(wupb));
	int ret = GetSamplesFor14443bDemod(RECEIVE_SAMPLES_TIMEOUT, true);
	// ATQB too short?
	if (ret < 14) {
		return 2;
	}

    // select the tag
    // copy the PUPI to ATTRIB
    memcpy(attrib + 1, Demod.output + 1, 4);
    /* copy the protocol info from ATQB (Protocol Info -> Protocol_Type) into
    ATTRIB (Param 3) */
    attrib[7] = Demod.output[10] & 0x0F;
    ComputeCrc14443(CRC_14443_B, attrib, 9, attrib + 9, attrib + 10);
    CodeAndTransmit14443bAsReader(attrib, sizeof(attrib));
    ret = GetSamplesFor14443bDemod(RECEIVE_SAMPLES_TIMEOUT, true);
    // Answer to ATTRIB too short?
    if (ret < 3) {
		return 2;
	}
	// reset PCB block number
	pcb_blocknum = 0;
	return 1;
}

// Set up ISO 14443 Type B communication (similar to iso14443a_setup)
void iso14443b_setup() {
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	// Set up the synchronous serial port
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	// connect Demodulated Signal to ADC:
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	// Signal field is on with the appropriate LED
    LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SEND_SHALLOW_MOD);

	DemodReset();
	UartReset();
}

//-----------------------------------------------------------------------------
// Read a SRI512 ISO 14443B tag.
//
// SRI512 tags are just simple memory tags, here we're looking at making a dump
// of the contents of the memory. No anticollision algorithm is done, we assume
// we have a single tag in the field.
//
// I tried to be systematic and check every answer of the tag, every CRC, etc...
//-----------------------------------------------------------------------------
void ReadSTMemoryIso14443b(uint32_t dwLast)
{
	LED_A_ON();
	uint8_t i = 0x00;

	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	// Make sure that we start from off, since the tags are stateful;
	// confusing things will happen if we don't reset them between reads.
	LED_D_OFF();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	SpinDelay(200);

	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);

	// Now give it time to spin up.
	// Signal field is on with the appropriate LED
	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SEND_SHALLOW_MOD);
	SpinDelay(200);

	clear_trace();
	set_tracing(true);

	// First command: wake up the tag using the INITIATE command
	uint8_t cmd1[] = {0x06, 0x00, 0x97, 0x5b};
	CodeAndTransmit14443bAsReader(cmd1, sizeof(cmd1));
	int ret = GetSamplesFor14443bDemod(RECEIVE_SAMPLES_TIMEOUT, true);

	if (ret < 0) {
		FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
		DbpString("No response from tag");
		LEDsoff();
		return;
	} else {
		Dbprintf("Randomly generated Chip ID (+ 2 byte CRC): %02x %02x %02x",
				Demod.output[0], Demod.output[1], Demod.output[2]);
	}

	// There is a response, SELECT the uid
	DbpString("Now SELECT tag:");
	cmd1[0] = 0x0E; // 0x0E is SELECT
	cmd1[1] = Demod.output[0];
	ComputeCrc14443(CRC_14443_B, cmd1, 2, &cmd1[2], &cmd1[3]);
	CodeAndTransmit14443bAsReader(cmd1, sizeof(cmd1));
	ret = GetSamplesFor14443bDemod(RECEIVE_SAMPLES_TIMEOUT, true);
	if (Demod.len != 3) {
		FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
		Dbprintf("Expected 3 bytes from tag, got %d", Demod.len);
		LEDsoff();
		return;
	}
	// Check the CRC of the answer:
	ComputeCrc14443(CRC_14443_B, Demod.output, 1 , &cmd1[2], &cmd1[3]);
	if(cmd1[2] != Demod.output[1] || cmd1[3] != Demod.output[2]) {
		FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
		DbpString("CRC Error reading select response.");
		LEDsoff();
		return;
	}
	// Check response from the tag: should be the same UID as the command we just sent:
	if (cmd1[1] != Demod.output[0]) {
		FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
		Dbprintf("Bad response to SELECT from Tag, aborting: %02x %02x", cmd1[1], Demod.output[0]);
		LEDsoff();
		return;
	}

	// Tag is now selected,
	// First get the tag's UID:
	cmd1[0] = 0x0B;
	ComputeCrc14443(CRC_14443_B, cmd1, 1 , &cmd1[1], &cmd1[2]);
	CodeAndTransmit14443bAsReader(cmd1, 3); // Only first three bytes for this one
	ret = GetSamplesFor14443bDemod(RECEIVE_SAMPLES_TIMEOUT, true);
	if (ret != 10) {
		FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
		Dbprintf("Expected 10 bytes from tag, got %d", Demod.len);
		LEDsoff();
		return;
	}
	// The check the CRC of the answer (use cmd1 as temporary variable):
	ComputeCrc14443(CRC_14443_B, Demod.output, 8, &cmd1[2], &cmd1[3]);
	if(cmd1[2] != Demod.output[8] || cmd1[3] != Demod.output[9]) {
		Dbprintf("CRC Error reading block! Expected: %04x got: %04x",
				(cmd1[2]<<8)+cmd1[3], (Demod.output[8]<<8)+Demod.output[9]);
		// Do not return;, let's go on... (we should retry, maybe ?)
	}
	Dbprintf("Tag UID (64 bits): %08x %08x",
			(Demod.output[7]<<24) + (Demod.output[6]<<16) + (Demod.output[5]<<8) + Demod.output[4],
			(Demod.output[3]<<24) + (Demod.output[2]<<16) + (Demod.output[1]<<8) + Demod.output[0]);

	// Now loop to read all 16 blocks, address from 0 to last block
	Dbprintf("Tag memory dump, block 0 to %d", dwLast);
	cmd1[0] = 0x08;
	i = 0x00;
	dwLast++;
	for (;;) {
		if (i == dwLast) {
			DbpString("System area block (0xff):");
			i = 0xff;
		}
		cmd1[1] = i;
		ComputeCrc14443(CRC_14443_B, cmd1, 2, &cmd1[2], &cmd1[3]);
		CodeAndTransmit14443bAsReader(cmd1, sizeof(cmd1));
		ret = GetSamplesFor14443bDemod(RECEIVE_SAMPLES_TIMEOUT, true);
		if (ret != 6) { // Check if we got an answer from the tag
			FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
			DbpString("Expected 6 bytes from tag, got less...");
			LEDsoff();
			return;
		}
		// The check the CRC of the answer (use cmd1 as temporary variable):
		ComputeCrc14443(CRC_14443_B, Demod.output, 4, &cmd1[2], &cmd1[3]);
		if (cmd1[2] != Demod.output[4] || cmd1[3] != Demod.output[5]) {
			Dbprintf("CRC Error reading block! Expected: %04x got: %04x",
					(cmd1[2]<<8)+cmd1[3], (Demod.output[4]<<8)+Demod.output[5]);
			// Do not return;, let's go on... (we should retry, maybe ?)
		}
		// Now print out the memory location:
		Dbprintf("Address=%02x, Contents=%08x, CRC=%04x", i,
				(Demod.output[3]<<24) + (Demod.output[2]<<16) + (Demod.output[1]<<8) + Demod.output[0],
				(Demod.output[4]<<8)+Demod.output[5]);
		if (i == 0xff) {
			break;
		}
		i++;
	}
	
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LEDsoff();
}


//=============================================================================
// Finally, the `sniffer' combines elements from both the reader and
// simulated tag, to show both sides of the conversation.
//=============================================================================

//-----------------------------------------------------------------------------
// Record the sequence of commands sent by the reader to the tag, with
// triggering so that we start recording at the point that the tag is moved
// near the reader.
//-----------------------------------------------------------------------------
/*
 * Memory usage for this function, (within BigBuf)
 * Last Received command (reader->tag) - MAX_FRAME_SIZE
 * Last Received command (tag->reader) - MAX_FRAME_SIZE
 * DMA Buffer - ISO14443B_DMA_BUFFER_SIZE
 * Demodulated samples received - all the rest
 */
void RAMFUNC SnoopIso14443b(void)
{
	LED_A_ON();
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	BigBuf_free();

	clear_trace();
	set_tracing(true);

	// The DMA buffer, used to stream samples from the FPGA
	uint16_t *dmaBuf = (uint16_t*) BigBuf_malloc(ISO14443B_DMA_BUFFER_SIZE * sizeof(uint16_t));
	int lastRxCounter;
	uint16_t *upTo;
	int8_t ci, cq;
	int maxBehindBy = 0;

	// Count of samples received so far, so that we can include timing
	// information in the trace buffer.
	int samples = 0;

	DemodInit(BigBuf_malloc(MAX_FRAME_SIZE));
	UartInit(BigBuf_malloc(MAX_FRAME_SIZE));

	// Print some debug information about the buffer sizes
	Dbprintf("Snooping buffers initialized:");
	Dbprintf("  Trace: %i bytes", BigBuf_max_traceLen());
	Dbprintf("  Reader -> tag: %i bytes", MAX_FRAME_SIZE);
	Dbprintf("  tag -> Reader: %i bytes", MAX_FRAME_SIZE);
	Dbprintf("  DMA: %i bytes", ISO14443B_DMA_BUFFER_SIZE);

	// Signal field is off
	LED_D_OFF();

	// And put the FPGA in the appropriate mode
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_SUBCARRIER_848_KHZ | FPGA_HF_READER_MODE_SNOOP_IQ);
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	// Setup for the DMA.
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	upTo = dmaBuf;
	lastRxCounter = ISO14443B_DMA_BUFFER_SIZE;
	FpgaSetupSscDma((uint8_t*) dmaBuf, ISO14443B_DMA_BUFFER_SIZE);

	bool TagIsActive = false;
	bool ReaderIsActive = false;
	// We won't start recording the frames that we acquire until we trigger.
	// A good trigger condition to get started is probably when we see a
	// reader command
	bool triggered = false;

	// And now we loop, receiving samples.
	for(;;) {
		int behindBy = (lastRxCounter - AT91C_BASE_PDC_SSC->PDC_RCR) & (ISO14443B_DMA_BUFFER_SIZE-1);
		if(behindBy > maxBehindBy) {
			maxBehindBy = behindBy;
		}

		if(behindBy < 1) continue;

		ci = *upTo>>8;
		cq = *upTo;
		upTo++;
		lastRxCounter--;
		if(upTo >= dmaBuf + ISO14443B_DMA_BUFFER_SIZE) {               // we have read all of the DMA buffer content.
			upTo = dmaBuf;                                             // start reading the circular buffer from the beginning again
			lastRxCounter += ISO14443B_DMA_BUFFER_SIZE;
			if(behindBy > (9*ISO14443B_DMA_BUFFER_SIZE/10)) { 
				Dbprintf("About to blow circular buffer - aborted! behindBy=%d", behindBy);
				break;
			}
		}
		if (AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX)) {              // DMA Counter Register had reached 0, already rotated.
			AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t) dmaBuf;          // refresh the DMA Next Buffer and
			AT91C_BASE_PDC_SSC->PDC_RNCR = ISO14443B_DMA_BUFFER_SIZE;  // DMA Next Counter registers
			WDT_HIT();
			if(BUTTON_PRESS()) {
				DbpString("cancelled");
				break;
			}
		}

		samples++;

		if (!TagIsActive) {							// no need to try decoding reader data if the tag is sending
			if(Handle14443bUartBit(ci & 0x01)) {
				triggered = true;
				LogTrace(Uart.output, Uart.byteCnt, samples, samples, NULL, true);
				/* And ready to receive another command. */
				UartReset();
				/* And also reset the demod code, which might have been */
				/* false-triggered by the commands from the reader. */
				DemodReset();
			}
			if(Handle14443bUartBit(cq & 0x01)) {
				triggered = true;
				LogTrace(Uart.output, Uart.byteCnt, samples, samples, NULL, true);
				/* And ready to receive another command. */
				UartReset();
				/* And also reset the demod code, which might have been */
				/* false-triggered by the commands from the reader. */
				DemodReset();
			}
			ReaderIsActive = (Uart.state > STATE_GOT_FALLING_EDGE_OF_SOF);
		}

		if (!ReaderIsActive && triggered) {						// no need to try decoding tag data if the reader is sending or not yet triggered
			if (Handle14443bSamplesDemod(ci/2, cq/2) >= 0) {
				//Use samples as a time measurement
				LogTrace(Demod.output, Demod.len, samples, samples, NULL, false);
				// And ready to receive another response.
				DemodReset();
			}
			TagIsActive = (Demod.state > DEMOD_GOT_FALLING_EDGE_OF_SOF);
		}

	}

	FpgaDisableSscDma();
	DbpString("Snoop statistics:");
	Dbprintf("  Max behind by: %i", maxBehindBy);
	Dbprintf("  Uart State: %x", Uart.state);
	Dbprintf("  Uart ByteCnt: %i", Uart.byteCnt);
	Dbprintf("  Uart ByteCntMax: %i", Uart.byteCntMax);
	Dbprintf("  Trace length: %i", BigBuf_get_traceLen());
	LEDsoff();
}


/*
 * Send raw command to tag ISO14443B
 * @Input
 * datalen     len of buffer data
 * recv        bool when true wait for data from tag and send to client
 * powerfield  bool leave the field on when true
 * data        buffer with byte to send
 *
 * @Output
 * none
 *
 */
void SendRawCommand14443B(uint32_t datalen, uint32_t recv, uint8_t powerfield, uint8_t data[])
{
	LED_A_ON();
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

	// switch field on and give tag some time to power up
	LED_D_ON();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER | FPGA_HF_READER_MODE_SEND_SHALLOW_MOD);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_READER);
	SpinDelay(10);

	if (datalen){
		set_tracing(true);
		
		CodeAndTransmit14443bAsReader(data, datalen);

		if (recv) {
			int ret = GetSamplesFor14443bDemod(5*RECEIVE_SAMPLES_TIMEOUT, true);
			FpgaDisableTracing();
			uint16_t iLen = MIN(Demod.len, USB_CMD_DATA_SIZE);
			cmd_send(CMD_ACK, ret, 0, 0, Demod.output, iLen);
		}

		FpgaDisableTracing();
	}

	if (!powerfield) {
		FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
		LED_D_OFF();
	}
	
	LED_A_OFF();
}

