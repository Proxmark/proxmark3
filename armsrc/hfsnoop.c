//-----------------------------------------------------------------------------
// piwi, 2019
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to get sample data from FPGA.
//-----------------------------------------------------------------------------

#include "hfsnoop.h"

#include "proxmark3.h"
#include "BigBuf.h"
#include "util.h"
#include "apps.h"
#include "usb_cdc.h"
#include "fpga.h"
#include "fpgaloader.h"

static void RAMFUNC optimizedSnoop(void)
{
	int n = BigBuf_max_traceLen() / sizeof(uint16_t); // take all memory

	uint16_t *dest = (uint16_t *)BigBuf_get_addr();
	uint16_t *destend = dest + n;

	AT91C_BASE_SSC->SSC_RFMR = SSC_FRAME_MODE_BITS_IN_WORD(16); // Setting Frame mode, 16 bits per word
	// Reading data loop
	while(dest <= destend)
	{
		if(AT91C_BASE_SSC->SSC_SR & AT91C_SSC_RXRDY)
		{
			*dest = (uint16_t)(AT91C_BASE_SSC->SSC_RHR);
			dest++;
		}
	}
	//Resetting Frame mode (First set in fpgaloader.c)
	AT91C_BASE_SSC->SSC_RFMR = SSC_FRAME_MODE_BITS_IN_WORD(8) | AT91C_SSC_MSBF | SSC_FRAME_MODE_WORDS_PER_TRANSFER(0);
}

void HfSnoop(int samplesToSkip, int triggersToSkip)
{
	BigBuf_free(); BigBuf_Clear();
	
	Dbprintf("Skipping first %d sample pairs, Skipping %d triggers.\n", samplesToSkip, triggersToSkip);
	int trigger_cnt;
	LED_D_ON();
	// Select correct configs
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	// Set up the synchronous serial port
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_SNOOP);
	// connect Demodulated Signal to ADC:
	SetAdcMuxFor(GPIO_MUXSEL_HIPKD);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_SNOOP);
	SpinDelay(100);

	AT91C_BASE_SSC->SSC_RFMR = SSC_FRAME_MODE_BITS_IN_WORD(16); // Setting Frame Mode For better performance on high speed data transfer.

	trigger_cnt = 0;
	uint16_t r = 0;
	while(!BUTTON_PRESS() && !usb_poll_validate_length()) {
		WDT_HIT();
		if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_RXRDY)) {
			r = (uint16_t)AT91C_BASE_SSC->SSC_RHR;
			r = MAX(r & 0xff, r >> 8); 
			if (r >= 240) {
				if (++trigger_cnt > triggersToSkip)
					break;
			} 
		}
	}

	if(!BUTTON_PRESS()) {
		int waitcount = samplesToSkip; // lets wait 40000 ticks of pck0
		while(waitcount != 0) {
			if(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_RXRDY))
				waitcount--;
		}
		optimizedSnoop();
		Dbprintf("Trigger kicked! Value: %d, Dumping Samples Hispeed now.", r);
	}

	DbpString("HF Snoop end");
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();
}

void HfPlot(void)
{
	uint8_t *buf = ToSend;
	uint8_t *this_buf = buf;
	
	FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
	FpgaSetupSsc(FPGA_MAJOR_MODE_HF_GET_TRACE);
	AT91C_BASE_PDC_SSC->PDC_PTCR = AT91C_PDC_RXTDIS;   // Disable DMA Transfer
	AT91C_BASE_PDC_SSC->PDC_RPR = (uint32_t) this_buf; // start transfer to this memory address
	AT91C_BASE_PDC_SSC->PDC_RCR = USB_CMD_DATA_SIZE;   // transfer this many samples
	buf[0] = (uint8_t)AT91C_BASE_SSC->SSC_RHR;         // clear receive register
	AT91C_BASE_PDC_SSC->PDC_PTCR = AT91C_PDC_RXTEN;    // Start DMA transfer
	FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_GET_TRACE);   // let FPGA transfer its internal Block-RAM

	LED_B_ON();
	for(size_t i = 0; i < FPGA_TRACE_SIZE; i += USB_CMD_DATA_SIZE) {
		// prepare next DMA transfer:
		uint8_t *next_buf = buf + ((i + USB_CMD_DATA_SIZE) % (2 * USB_CMD_DATA_SIZE));
		AT91C_BASE_PDC_SSC->PDC_RNPR = (uint32_t)next_buf;
		AT91C_BASE_PDC_SSC->PDC_RNCR = USB_CMD_DATA_SIZE;
		size_t len = MIN(FPGA_TRACE_SIZE - i, USB_CMD_DATA_SIZE);
		while (!(AT91C_BASE_SSC->SSC_SR & (AT91C_SSC_ENDRX))) ; // wait for DMA transfer to complete
		cmd_send(CMD_DOWNLOADED_RAW_ADC_SAMPLES_125K, i, len, FPGA_TRACE_SIZE, this_buf, len);
		this_buf = next_buf;
	}
	// Trigger a finish downloading signal with an ACK frame
	cmd_send(CMD_ACK, 1, 0, FPGA_TRACE_SIZE, 0, 0);
	LED_B_OFF();
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
}
