//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency commands
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include "proxmark3.h"
#include "cmdlf.h"
#include "lfdemod.h"     // for psk2TOpsk1
#include "util.h"        // for parsing cli command utils
#include "ui.h"          // for show graph controls
#include "graph.h"       // for graph data
#include "cmdparser.h"   // for getting cli commands included in cmdmain.h
#include "cmdmain.h"     // for sending cmds to device
#include "data.h"        // for GetFromBigBuf
#include "cmddata.h"     // for `lf search`
#include "cmdlfawid.h"   // for awid menu
#include "cmdlfem4x.h"   // for em4x menu
#include "cmdlfhid.h"    // for hid menu
#include "cmdlfhitag.h"  // for hitag menu
#include "cmdlfio.h"     // for ioprox menu
#include "cmdlft55xx.h"  // for t55xx menu
#include "cmdlfti.h"     // for ti menu
#include "cmdlfpresco.h" // for presco menu
#include "cmdlfpcf7931.h"// for pcf7931 menu
#include "cmdlfpyramid.h"// for pyramid menu
#include "cmdlfviking.h" // for viking menu
#include "cmdlfcotag.h"  // for COTAG menu
#include "cmdlfvisa2000.h"  // for VISA2000 menu
#include "cmdlfindala.h" // for indala menu
#include "cmdlfgproxii.h"// for gproxii menu
#include "cmdlffdx.h"    // for fdx-b menu
#include "cmdlfparadox.h"// for paradox menu
#include "cmdlfnexwatch.h"//for nexwatch menu
#include "cmdlfjablotron.h" //for jablotron menu
#include "cmdlfnoralsy.h"// for noralsy menu
#include "cmdlfsecurakey.h"//for securakey menu
#include "cmdlfpac.h"    // for pac menu

bool g_lf_threshold_set = false;
static int CmdHelp(const char *Cmd);



int usage_lf_cmdread(void)
{
	PrintAndLog("Usage: lf cmdread d <delay period> z <zero period> o <one period> c <cmdbytes> [H] ");
	PrintAndLog("Options:        ");
	PrintAndLog("       h             This help");
	PrintAndLog("       L             Low frequency (125 KHz)");
	PrintAndLog("       H             High frequency (134 KHz)");
	PrintAndLog("       d <delay>     delay OFF period");
	PrintAndLog("       z <zero>      time period ZERO");
	PrintAndLog("       o <one>       time period ONE");
	PrintAndLog("       c <cmd>       Command bytes");
	PrintAndLog("       ************* All periods in microseconds");
	PrintAndLog("Examples:");
	PrintAndLog("      lf cmdread d 80 z 100 o 200 c 11000");
	PrintAndLog("      lf cmdread d 80 z 100 o 100 c 11000 H");
	return 0;
}

/* send a command before reading */
int CmdLFCommandRead(const char *Cmd)
{
	static char dummy[3] = {0x20,0x00,0x00};
	UsbCommand c = {CMD_MOD_THEN_ACQUIRE_RAW_ADC_SAMPLES_125K};
	bool errors = false;
	//uint8_t divisor = 95; //125khz
	uint8_t cmdp = 0;
	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
			return usage_lf_cmdread();
		case 'H':
			//divisor = 88;
			dummy[1]='h';
			cmdp++;
			break;
		case 'L':
			cmdp++;
			break;
		case 'c':
			param_getstr(Cmd, cmdp+1, (char *)&c.d.asBytes);
			cmdp+=2;
			break;
		case 'd':
			c.arg[0] = param_get32ex(Cmd, cmdp+1, 0, 10);
			cmdp+=2;
			break;
		case 'z':
			c.arg[1] = param_get32ex(Cmd, cmdp+1, 0, 10);
			cmdp+=2;
			break;
		case 'o':
			c.arg[2] = param_get32ex(Cmd, cmdp+1, 0, 10);
			cmdp+=2;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = 1;
			break;
		}
		if(errors) break;
	}
	// No args
	if(cmdp == 0) errors = 1;

	//Validations
	if(errors) return usage_lf_cmdread();
	
	// in case they specified 'H'
	strcpy((char *)&c.d.asBytes + strlen((char *)c.d.asBytes), dummy);

	clearCommandBuffer();
	SendCommand(&c);
	return 0;
}

int CmdFlexdemod(const char *Cmd)
{
	int i;
	for (i = 0; i < GraphTraceLen; ++i) {
		if (GraphBuffer[i] < 0) {
			GraphBuffer[i] = -1;
		} else {
			GraphBuffer[i] = 1;
		}
	}

 #define LONG_WAIT 100
	int start;
	for (start = 0; start < GraphTraceLen - LONG_WAIT; start++) {
		int first = GraphBuffer[start];
		for (i = start; i < start + LONG_WAIT; i++) {
			if (GraphBuffer[i] != first) {
				break;
			}
		}
		if (i == (start + LONG_WAIT)) {
			break;
		}
	}
	if (start == GraphTraceLen - LONG_WAIT) {
		PrintAndLog("nothing to wait for");
		return 0;
	}

	GraphBuffer[start] = 2;
	GraphBuffer[start+1] = -2;
	uint8_t bits[64] = {0x00};

	int bit, sum;
	i = start;
	for (bit = 0; bit < 64; bit++) {
		sum = 0;
		for (int j = 0; j < 16; j++) {
			sum += GraphBuffer[i++];
		}

		bits[bit] = (sum > 0) ? 1 : 0;

		PrintAndLog("bit %d sum %d", bit, sum);
	}

	for (bit = 0; bit < 64; bit++) {
		int j;
		int sum = 0;
		for (j = 0; j < 16; j++) {
			sum += GraphBuffer[i++];
		}
		if (sum > 0 && bits[bit] != 1) {
			PrintAndLog("oops1 at %d", bit);
		}
		if (sum < 0 && bits[bit] != 0) {
			PrintAndLog("oops2 at %d", bit);
		}
	}

	// HACK writing back to graphbuffer.
	GraphTraceLen = 32*64;
	i = 0;
	int phase = 0;
	for (bit = 0; bit < 64; bit++) {
	
		phase = (bits[bit] == 0) ? 0 : 1;
		
		int j;
		for (j = 0; j < 32; j++) {
			GraphBuffer[i++] = phase;
			phase = !phase;
		}
	}

	RepaintGraphWindow();
	return 0;
}	

int usage_lf_read(void)
{
	PrintAndLog("Usage: lf read");
	PrintAndLog("Options:        ");
	PrintAndLog("       h            This help");
	PrintAndLog("       s            silent run no printout");
	PrintAndLog("       [# samples]  # samples to collect (optional)");	
	PrintAndLog("Use 'lf config' to set parameters.");
	return 0;
}
int usage_lf_snoop(void)
{
	PrintAndLog("Usage: lf snoop");
	PrintAndLog("Options:        ");
	PrintAndLog("       h            This help");
	PrintAndLog("This function takes no arguments. ");
	PrintAndLog("Use 'lf config' to set parameters.");
	return 0;
}

int usage_lf_config(void)
{
	PrintAndLog("Usage: lf config [H|<divisor>] [b <bps>] [d <decim>] [a 0|1]");
	PrintAndLog("Options:        ");
	PrintAndLog("       h             This help");
	PrintAndLog("       L             Low frequency (125 KHz)");
	PrintAndLog("       H             High frequency (134 KHz)");
	PrintAndLog("       q <divisor>   Manually set divisor. 88-> 134KHz, 95-> 125 Hz");
	PrintAndLog("       b <bps>       Sets resolution of bits per sample. Default (max): 8");
	PrintAndLog("       d <decim>     Sets decimation. A value of N saves only 1 in N samples. Default: 1");
	PrintAndLog("       a [0|1]       Averaging - if set, will average the stored sample value when decimating. Default: 1");
	PrintAndLog("       t <threshold> Sets trigger threshold. 0 means no threshold (range: 0-128)");
	PrintAndLog("Examples:");
	PrintAndLog("      lf config b 8 L");
	PrintAndLog("                    Samples at 125KHz, 8bps.");
	PrintAndLog("      lf config H b 4 d 3");
	PrintAndLog("                    Samples at 134KHz, averages three samples into one, stored with ");
	PrintAndLog("                    a resolution of 4 bits per sample.");
	PrintAndLog("      lf read");
	PrintAndLog("                    Performs a read (active field)");
	PrintAndLog("      lf snoop");
	PrintAndLog("                    Performs a snoop (no active field)");
	return 0;
}

int CmdLFSetConfig(const char *Cmd)
{

	uint8_t divisor =  0;//Frequency divisor
	uint8_t bps = 0; // Bits per sample
	uint8_t decimation = 0; //How many to keep
	bool averaging = 1; // Defaults to true
	bool errors = false;
	int trigger_threshold =-1;//Means no change
	uint8_t unsigned_trigg = 0;

	uint8_t cmdp =0;
	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
			return usage_lf_config();
		case 'H':
			divisor = 88;
			cmdp++;
			break;
		case 'L':
			divisor = 95;
			cmdp++;
			break;
		case 'q':
			errors |= param_getdec(Cmd,cmdp+1,&divisor);
			cmdp+=2;
			break;
		case 't':
			errors |= param_getdec(Cmd,cmdp+1,&unsigned_trigg);
			cmdp+=2;
			if(!errors) {
				trigger_threshold = unsigned_trigg;
				if (trigger_threshold > 0) g_lf_threshold_set = true;
			}
			break;
		case 'b':
			errors |= param_getdec(Cmd,cmdp+1,&bps);
			cmdp+=2;
			break;
		case 'd':
			errors |= param_getdec(Cmd,cmdp+1,&decimation);
			cmdp+=2;
			break;
		case 'a':
			averaging = param_getchar(Cmd,cmdp+1) == '1';
			cmdp+=2;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = 1;
			break;
		}
		if(errors) break;
	}
	if(cmdp == 0)
	{
		errors = 1;// No args
	}

	//Validations
	if(errors)
	{
		return usage_lf_config();
	}
	//Bps is limited to 8, so fits in lower half of arg1
	if(bps >> 4) bps = 8;

	sample_config config = {
		decimation,bps,averaging,divisor,trigger_threshold
	};
	//Averaging is a flag on high-bit of arg[1]
	UsbCommand c = {CMD_SET_LF_SAMPLING_CONFIG};
	memcpy(c.d.asBytes,&config,sizeof(sample_config));
	clearCommandBuffer();
	SendCommand(&c);
	return 0;
}

bool lf_read(bool silent, uint32_t samples) {
	if (offline) return false;
	UsbCommand c = {CMD_ACQUIRE_RAW_ADC_SAMPLES_125K, {silent,samples,0}};
	clearCommandBuffer();
	//And ship it to device
	SendCommand(&c);

	UsbCommand resp;
	if (g_lf_threshold_set) {
		WaitForResponse(CMD_ACK,&resp);
	} else {
		if ( !WaitForResponseTimeout(CMD_ACK,&resp,2500) ) {
			PrintAndLog("command execution time out");
			return false;
		}
	}
	// resp.arg[0] is bits read not bytes read.
	getSamples(resp.arg[0]/8, silent);

	return true;
}

int CmdLFRead(const char *Cmd)
{
	uint8_t cmdp = 0;
	bool silent = false;
	if (param_getchar(Cmd, cmdp) == 'h')
	{
		return usage_lf_read();
	}
	if (param_getchar(Cmd, cmdp) == 's') {
		silent = true; //suppress print
		cmdp++;
	}
	uint32_t samples = param_get32ex(Cmd, cmdp, 0, 10);
	return lf_read(silent, samples);
}

int CmdLFSnoop(const char *Cmd)
{
	uint8_t cmdp =0;
	if(param_getchar(Cmd, cmdp) == 'h')
	{
		return usage_lf_snoop();
	}

	UsbCommand c = {CMD_LF_SNOOP_RAW_ADC_SAMPLES};
	clearCommandBuffer();
	SendCommand(&c);
	WaitForResponse(CMD_ACK,NULL);
	getSamples(0, true);

	return 0;
}

static void ChkBitstream(const char *str)
{
	int i;
 
	/* convert to bitstream if necessary */
	for (i = 0; i < (int)(GraphTraceLen / 2); i++){
		if (GraphBuffer[i] > 1 || GraphBuffer[i] < 0) {
			CmdGetBitStream("");
			break;
		}
	}
}
//Attempt to simulate any wave in buffer (one bit per output sample)
// converts GraphBuffer to bitstream (based on zero crossings) if needed.
int CmdLFSim(const char *Cmd)
{
	int i,j;
	static int gap;

	sscanf(Cmd, "%i", &gap);

	// convert to bitstream if necessary
	ChkBitstream(Cmd);

	//can send only 512 bits at a time (1 byte sent per bit...)
	printf("Sending [%d bytes]", GraphTraceLen);
	for (i = 0; i < GraphTraceLen; i += USB_CMD_DATA_SIZE) {
		UsbCommand c = {CMD_DOWNLOADED_SIM_SAMPLES_125K, {i, 0, 0}};

		for (j = 0; j < USB_CMD_DATA_SIZE; j++) {
			c.d.asBytes[j] = GraphBuffer[i+j];
		}
		SendCommand(&c);
		WaitForResponse(CMD_ACK,NULL);
		printf(".");
	}

	printf("\n");
	PrintAndLog("Starting to simulate");
	UsbCommand c = {CMD_SIMULATE_TAG_125K, {GraphTraceLen, gap, 0}};
	clearCommandBuffer();
	SendCommand(&c);
	return 0;
}

int usage_lf_simfsk(void)
{
	//print help
	PrintAndLog("Usage: lf simfsk [c <clock>] [i] [H <fcHigh>] [L <fcLow>] [d <hexdata>]");
	PrintAndLog("Options:        ");
	PrintAndLog("       h              This help");
	PrintAndLog("       c <clock>      Manually set clock - can autodetect if using DemodBuffer");
	PrintAndLog("       i              invert data");
	PrintAndLog("       H <fcHigh>     Manually set the larger Field Clock");
	PrintAndLog("       L <fcLow>      Manually set the smaller Field Clock");
	//PrintAndLog("       s              TBD- -to enable a gap between playback repetitions - default: no gap");
	PrintAndLog("       d <hexdata>    Data to sim as hex - omit to sim from DemodBuffer");
	PrintAndLog("\n  NOTE: if you set one clock manually set them all manually");
	return 0;
}

int usage_lf_simask(void)
{
	//print help
	PrintAndLog("Usage: lf simask [c <clock>] [i] [b|m|r] [s] [d <raw hex to sim>]");
	PrintAndLog("Options:        ");
	PrintAndLog("       h              This help");
	PrintAndLog("       c <clock>      Manually set clock - can autodetect if using DemodBuffer");
	PrintAndLog("       i              invert data");
	PrintAndLog("       b              sim ask/biphase");
	PrintAndLog("       m              sim ask/manchester - Default");
	PrintAndLog("       r              sim ask/raw");
	PrintAndLog("       s              add t55xx Sequence Terminator gap - default: no gaps (only manchester)");
	PrintAndLog("       d <hexdata>    Data to sim as hex - omit to sim from DemodBuffer");
	return 0;
}

int usage_lf_simpsk(void)
{
	//print help
	PrintAndLog("Usage: lf simpsk [1|2|3] [c <clock>] [i] [r <carrier>] [d <raw hex to sim>]");
	PrintAndLog("Options:        ");
	PrintAndLog("       h              This help");
	PrintAndLog("       c <clock>      Manually set clock - can autodetect if using DemodBuffer");
	PrintAndLog("       i              invert data");
	PrintAndLog("       1              set PSK1 (default)");
	PrintAndLog("       2              set PSK2");
	PrintAndLog("       3              set PSK3");
	PrintAndLog("       r <carrier>    2|4|8 are valid carriers: default = 2");
	PrintAndLog("       d <hexdata>    Data to sim as hex - omit to sim from DemodBuffer");
	return 0;
}

// by marshmellow - sim fsk data given clock, fcHigh, fcLow, invert 
// - allow pull data from DemodBuffer
int CmdLFfskSim(const char *Cmd)
{
	//might be able to autodetect FCs and clock from Graphbuffer if using demod buffer
	// otherwise will need FChigh, FClow, Clock, and bitstream
	uint8_t fcHigh=0, fcLow=0, clk=0;
	uint8_t invert=0;
	bool errors = false;
	char hexData[32] = {0x00}; // store entered hex data
	uint8_t data[255] = {0x00}; 
	int dataLen = 0;
	uint8_t cmdp = 0;
	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
			return usage_lf_simfsk();
		case 'i':
			invert = 1;
			cmdp++;
			break;
		case 'c':
			errors |= param_getdec(Cmd,cmdp+1,&clk);
			cmdp+=2;
			break;
		case 'H':
			errors |= param_getdec(Cmd,cmdp+1,&fcHigh);
			cmdp+=2;
			break;
		case 'L':
			errors |= param_getdec(Cmd,cmdp+1,&fcLow);
			cmdp+=2;
			break;
		//case 's':
		//  separator=1;
		//  cmdp++;
		//  break;
		case 'd':
			dataLen = param_getstr(Cmd, cmdp+1, hexData);
			if (dataLen==0) {
				errors=true; 
			} else {
				dataLen = hextobinarray((char *)data, hexData);
			}   
			if (dataLen==0) errors=true; 
			if (errors) PrintAndLog ("Error getting hex data");
			cmdp+=2;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if(errors) break;
	}
	if(cmdp == 0 && DemodBufferLen == 0)
	{
		errors = true;// No args
	}

	//Validations
	if(errors)
	{
		return usage_lf_simfsk();
	}
	int firstClockEdge = 0;
	if (dataLen == 0){ //using DemodBuffer 
		if (clk==0 || fcHigh==0 || fcLow==0){ //manual settings must set them all
			uint8_t ans = fskClocks(&fcHigh, &fcLow, &clk, 0, &firstClockEdge);
			if (ans==0){
				if (!fcHigh) fcHigh=10;
				if (!fcLow) fcLow=8;
				if (!clk) clk=50;
			}
		}
	} else {
		setDemodBuf(data, dataLen, 0);
	}

	//default if not found
	if (clk == 0) clk = 50;
	if (fcHigh == 0) fcHigh = 10;
	if (fcLow == 0) fcLow = 8;

	uint16_t arg1, arg2;
	arg1 = fcHigh << 8 | fcLow;
	arg2 = invert << 8 | clk;
	size_t size = DemodBufferLen;
	if (size > USB_CMD_DATA_SIZE) {
		PrintAndLog("DemodBuffer too long for current implementation - length: %d - max: %d", size, USB_CMD_DATA_SIZE);
		size = USB_CMD_DATA_SIZE;
	} 
	UsbCommand c = {CMD_FSK_SIM_TAG, {arg1, arg2, size}};

	memcpy(c.d.asBytes, DemodBuffer, size);
	clearCommandBuffer();
	SendCommand(&c);
	return 0;
}

// by marshmellow - sim ask data given clock, invert, manchester or raw, separator 
// - allow pull data from DemodBuffer
int CmdLFaskSim(const char *Cmd)
{
	//autodetect clock from Graphbuffer if using demod buffer
	// needs clock, invert, manchester/raw as m or r, separator as s, and bitstream
	uint8_t encoding = 1, separator = 0;
	uint8_t clk=0, invert=0;
	bool errors = false;
	char hexData[32] = {0x00}; 
	uint8_t data[255]= {0x00}; // store entered hex data
	int dataLen = 0;
	uint8_t cmdp = 0;
	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
			return usage_lf_simask();
		case 'i':
			invert = 1;
			cmdp++;
			break;
		case 'c':
			errors |= param_getdec(Cmd,cmdp+1,&clk);
			cmdp+=2;
			break;
		case 'b':
			encoding=2; //biphase
			cmdp++;
			break;
		case 'm':
			encoding=1;
			cmdp++;
			break;
		case 'r':
			encoding=0;
			cmdp++;
			break;
		case 's':
			separator=1;
			cmdp++;
			break;
		case 'd':
			dataLen = param_getstr(Cmd, cmdp+1, hexData);
			if (dataLen==0) {
				errors=true; 
			} else {
				dataLen = hextobinarray((char *)data, hexData);
			}
			if (dataLen==0) errors=true; 
			if (errors) PrintAndLog ("Error getting hex data, datalen: %d",dataLen);
				cmdp+=2;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if(errors) break;
	}
	if(cmdp == 0 && DemodBufferLen == 0)
	{
		errors = true;// No args
	}

	//Validations
	if(errors)
	{
		return usage_lf_simask();
	}
	if (dataLen == 0){ //using DemodBuffer
		if (clk == 0) clk = GetAskClock("0", false, false);
	} else {
		setDemodBuf(data, dataLen, 0);
	}
	if (clk == 0) clk = 64;
	if (encoding == 0) clk = clk/2; //askraw needs to double the clock speed
	uint16_t arg1, arg2;
	size_t size=DemodBufferLen;
	arg1 = clk << 8 | encoding;
	arg2 = invert << 8 | separator;
	if (size > USB_CMD_DATA_SIZE) {
		PrintAndLog("DemodBuffer too long for current implementation - length: %d - max: %d", size, USB_CMD_DATA_SIZE);
		size = USB_CMD_DATA_SIZE;
	}
	UsbCommand c = {CMD_ASK_SIM_TAG, {arg1, arg2, size}};
	PrintAndLog("preparing to sim ask data: %d bits", size);
	memcpy(c.d.asBytes, DemodBuffer, size);
	clearCommandBuffer();
	SendCommand(&c);
	return 0;
}

// by marshmellow - sim psk data given carrier, clock, invert 
// - allow pull data from DemodBuffer or parameters
int CmdLFpskSim(const char *Cmd)
{
	//might be able to autodetect FC and clock from Graphbuffer if using demod buffer
	//will need carrier, Clock, and bitstream
	uint8_t carrier=0, clk=0;
	uint8_t invert=0;
	bool errors = false;
	char hexData[32] = {0x00}; // store entered hex data
	uint8_t data[255] = {0x00}; 
	int dataLen = 0;
	uint8_t cmdp = 0;
	uint8_t pskType = 1;
	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
			return usage_lf_simpsk();
		case 'i':
			invert = 1;
			cmdp++;
			break;
		case 'c':
			errors |= param_getdec(Cmd,cmdp+1,&clk);
			cmdp+=2;
			break;
		case 'r':
			errors |= param_getdec(Cmd,cmdp+1,&carrier);
			cmdp+=2;
			break;
		case '1':
			pskType=1;
			cmdp++;
			break;
		case '2':
			pskType=2;
			cmdp++;
			break;
		case '3':
			pskType=3;
			cmdp++;
			break;
		case 'd':
			dataLen = param_getstr(Cmd, cmdp+1, hexData);
			if (dataLen==0) {
				errors=true; 
			} else {
				dataLen = hextobinarray((char *)data, hexData);
			}    
			if (dataLen==0) errors=true; 
			if (errors) PrintAndLog ("Error getting hex data");
			cmdp+=2;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if (errors) break;
	}
	if (cmdp == 0 && DemodBufferLen == 0)
	{
		errors = true;// No args
	}

	//Validations
	if (errors)
	{
		return usage_lf_simpsk();
	}
	if (dataLen == 0){ //using DemodBuffer
		PrintAndLog("Getting Clocks");
		if (clk==0) clk = GetPskClock("", false, false);
		PrintAndLog("clk: %d",clk);
		if (!carrier) carrier = GetPskCarrier("", false, false); 
		PrintAndLog("carrier: %d", carrier);
	} else {
		setDemodBuf(data, dataLen, 0);
	}

	if (clk <= 0) clk = 32;
	if (carrier == 0) carrier = 2;
	if (pskType != 1){
		if (pskType == 2){
			//need to convert psk2 to psk1 data before sim
			psk2TOpsk1(DemodBuffer, DemodBufferLen);
		} else {
			PrintAndLog("Sorry, PSK3 not yet available");
		}
	}
	uint16_t arg1, arg2;
	arg1 = clk << 8 | carrier;
	arg2 = invert;
	size_t size=DemodBufferLen;
	if (size > USB_CMD_DATA_SIZE) {
		PrintAndLog("DemodBuffer too long for current implementation - length: %d - max: %d", size, USB_CMD_DATA_SIZE);
		size=USB_CMD_DATA_SIZE;
	}
	UsbCommand c = {CMD_PSK_SIM_TAG, {arg1, arg2, size}};
	PrintAndLog("DEBUG: Sending DemodBuffer Length: %d", size);
	memcpy(c.d.asBytes, DemodBuffer, size);
	clearCommandBuffer();
	SendCommand(&c);
	
	return 0;
}

int CmdLFSimBidir(const char *Cmd)
{
	// Set ADC to twice the carrier for a slight supersampling
	// HACK: not implemented in ARMSRC.
	PrintAndLog("Not implemented yet.");
	UsbCommand c = {CMD_LF_SIMULATE_BIDIR, {47, 384, 0}};
	SendCommand(&c);
	return 0;
}

int CmdVchDemod(const char *Cmd)
{
	// Is this the entire sync pattern, or does this also include some
	// data bits that happen to be the same everywhere? That would be
	// lovely to know.
	static const int SyncPattern[] = {
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	};

	// So first, we correlate for the sync pattern, and mark that.
	int bestCorrel = 0, bestPos = 0;
	int i;
	// It does us no good to find the sync pattern, with fewer than
	// 2048 samples after it...
	for (i = 0; i < (GraphTraceLen-2048); i++) {
		int sum = 0;
		int j;
		for (j = 0; j < arraylen(SyncPattern); j++) {
			sum += GraphBuffer[i+j]*SyncPattern[j];
		}
		if (sum > bestCorrel) {
			bestCorrel = sum;
			bestPos = i;
		}
	}
	PrintAndLog("best sync at %d [metric %d]", bestPos, bestCorrel);

	char bits[257];
	bits[256] = '\0';

	int worst = INT_MAX;
	int worstPos = 0;

	for (i = 0; i < 2048; i += 8) {
		int sum = 0;
		int j;
		for (j = 0; j < 8; j++) {
			sum += GraphBuffer[bestPos+i+j];
		}
		if (sum < 0) {
			bits[i/8] = '.';
		} else {
			bits[i/8] = '1';
		}
		if(abs(sum) < worst) {
			worst = abs(sum);
			worstPos = i;
		}
	}
	PrintAndLog("bits:");
	PrintAndLog("%s", bits);
	PrintAndLog("worst metric: %d at pos %d", worst, worstPos);

	if (strcmp(Cmd, "clone")==0) {
		GraphTraceLen = 0;
		char *s;
		for(s = bits; *s; s++) {
			int j;
			for(j = 0; j < 16; j++) {
				GraphBuffer[GraphTraceLen++] = (*s == '1') ? 1 : 0;
			}
		}
		RepaintGraphWindow();
	}
	return 0;
}


//by marshmellow
int CheckChipType(char cmdp) {
	uint32_t wordData = 0;

	if (offline || cmdp == '1') return 0;

	save_restoreGB(GRAPH_SAVE);
	save_restoreDB(GRAPH_SAVE);
	//check for em4x05/em4x69 chips first
	if (EM4x05Block0Test(&wordData)) {
		PrintAndLog("\nValid EM4x05/EM4x69 Chip Found\nTry lf em 4x05... commands\n");
		save_restoreGB(GRAPH_RESTORE);
		save_restoreDB(GRAPH_RESTORE);
		return 1;
	}

	//check for t55xx chip...
	if (tryDetectP1(true)) {
		PrintAndLog("\nValid T55xx Chip Found\nTry lf t55xx ... commands\n");
		save_restoreGB(GRAPH_RESTORE);
		save_restoreDB(GRAPH_RESTORE);
		return 1;		
	}
	save_restoreGB(GRAPH_RESTORE);
	save_restoreDB(GRAPH_RESTORE);
	return 0;
}

//by marshmellow
int CmdLFfind(const char *Cmd)
{
	uint32_t wordData = 0;
	int ans=0;
	size_t minLength = 1000;
	char cmdp = param_getchar(Cmd, 0);
	char testRaw = param_getchar(Cmd, 1);
	if (strlen(Cmd) > 3 || cmdp == 'h' || cmdp == 'H') {
		PrintAndLog("Usage:  lf search <0|1> [u]");
		PrintAndLog("     <use data from Graphbuffer> , if not set, try reading data from tag.");
		PrintAndLog("     [Search for Unknown tags] , if not set, reads only known tags.");
		PrintAndLog("");
		PrintAndLog("    sample: lf search     = try reading data from tag & search for known tags");
		PrintAndLog("          : lf search 1   = use data from GraphBuffer & search for known tags");
		PrintAndLog("          : lf search u   = try reading data from tag & search for known and unknown tags");
		PrintAndLog("          : lf search 1 u = use data from GraphBuffer & search for known and unknown tags");

		return 0;
	}

	if (!offline && (cmdp != '1')) {
		lf_read(true, 30000);
	} else if (GraphTraceLen < minLength) {
		PrintAndLog("Data in Graphbuffer was too small.");
		return 0;
	}
	if (cmdp == 'u' || cmdp == 'U') testRaw = 'u';

	PrintAndLog("NOTE: some demods output possible binary\n  if it finds something that looks like a tag");
	PrintAndLog("False Positives ARE possible\n");  
	PrintAndLog("\nChecking for known tags:\n");

	size_t testLen = minLength;
	// only run if graphbuffer is just noise as it should be for hitag/cotag
	if (graphJustNoise(GraphBuffer, testLen)) {
		// only run these tests if we are in online mode 
		if (!offline && (cmdp != '1')) {
			// test for em4x05 in reader talk first mode.
			if (EM4x05Block0Test(&wordData)) {
				PrintAndLog("\nValid EM4x05/EM4x69 Chip Found\nUse lf em 4x05readword/dump commands to read\n");
				return 1;
			}
			ans=CmdLFHitagReader("26");
			if (ans==0) {
				return 1;
			}
			ans=CmdCOTAGRead("");
			if (ans>0) {
				PrintAndLog("\nValid COTAG ID Found!");
				return 1;
			}
		}
		return 0;
	}

	// TODO test for modulation then only test formats that use that modulation

	ans=CmdFSKdemodIO("");
	if (ans>0) {
		PrintAndLog("\nValid IO Prox ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdFSKdemodPyramid("");
	if (ans>0) {
		PrintAndLog("\nValid Pyramid ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdFSKdemodParadox("");
	if (ans>0) {
		PrintAndLog("\nValid Paradox ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdFSKdemodAWID("");
	if (ans>0) {
		PrintAndLog("\nValid AWID ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdFSKdemodHID("");
	if (ans>0) {
		PrintAndLog("\nValid HID Prox ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdAskEM410xDemod("");
	if (ans>0) {
		PrintAndLog("\nValid EM410x ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdVisa2kDemod("");
	if (ans>0) {
		PrintAndLog("\nValid Visa2000 ID Found!");
		return CheckChipType(cmdp);		
	}
	
	ans=CmdG_Prox_II_Demod("");
	if (ans>0) {
		PrintAndLog("\nValid G Prox II ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdFdxDemod(""); //biphase
	if (ans>0) {
		PrintAndLog("\nValid FDX-B ID Found!");
		return CheckChipType(cmdp);
	}

	ans=EM4x50Read("", false); //ask
	if (ans>0) {
		PrintAndLog("\nValid EM4x50 ID Found!");
		return 1;
	}

	ans=CmdJablotronDemod("");
	if (ans>0) {
		PrintAndLog("\nValid Jablotron ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdNoralsyDemod("");
	if (ans>0) {
		PrintAndLog("\nValid Noralsy ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdSecurakeyDemod("");
	if (ans>0) {
		PrintAndLog("\nValid Securakey ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdVikingDemod("");
	if (ans>0) {
		PrintAndLog("\nValid Viking ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdIndalaDecode(""); //psk
	if (ans>0) {
		PrintAndLog("\nValid Indala ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdPSKNexWatch("");
	if (ans>0) {
		PrintAndLog("\nValid NexWatch ID Found!");
		return CheckChipType(cmdp);
	}

	ans=CmdPacDemod("");
	if (ans>0) {
		PrintAndLog("\nValid PAC/Stanley ID Found!");
		return CheckChipType(cmdp);		
	}

	PrintAndLog("\nNo Known Tags Found!\n");
	if (testRaw=='u' || testRaw=='U') {
		//ans=CheckChipType(cmdp);
		//test unknown tag formats (raw mode)0
		PrintAndLog("\nChecking for Unknown tags:\n");
		ans=AutoCorrelate(GraphBuffer, GraphBuffer, GraphTraceLen, 4000, false, false);
		if (ans > 0) PrintAndLog("Possible Auto Correlation of %d repeating samples",ans);
		ans=GetFskClock("",false,false); 
		if (ans != 0) { //fsk
			ans=FSKrawDemod("",true);
			if (ans>0) {
				PrintAndLog("\nUnknown FSK Modulated Tag Found!");
				return CheckChipType(cmdp);
			}
		}
		bool st = true;
		ans=ASKDemod_ext("0 0 0",true,false,1,&st);
		if (ans>0) {
			PrintAndLog("\nUnknown ASK Modulated and Manchester encoded Tag Found!");
			PrintAndLog("\nif it does not look right it could instead be ASK/Biphase - try 'data rawdemod ab'");
			return CheckChipType(cmdp);
		}
		ans=CmdPSK1rawDemod("");
		if (ans>0) {
			PrintAndLog("Possible unknown PSK1 Modulated Tag Found above!\n\nCould also be PSK2 - try 'data rawdemod p2'");
			PrintAndLog("\nCould also be PSK3 - [currently not supported]");
			PrintAndLog("\nCould also be NRZ - try 'data rawdemod nr'");
			return CheckChipType(cmdp);
		}
		ans = CheckChipType(cmdp);
		PrintAndLog("\nNo Data Found!\n");
	}
	return 0;
}

static command_t CommandTable[] = 
{
	{"help",        CmdHelp,            1, "This help"},
	{"awid",        CmdLFAWID,          1, "{ AWID RFIDs...              }"},
	{"cotag",       CmdLFCOTAG,         1, "{ COTAG CHIPs...             }"},
	{"em",          CmdLFEM4X,          1, "{ EM4X CHIPs & RFIDs...      }"},
	{"fdx",         CmdLFFdx,           1, "{ FDX-B RFIDs...             }"},
	{"gproxii",     CmdLF_G_Prox_II,    1, "{ G Prox II RFIDs...         }"},
	{"hid",         CmdLFHID,           1, "{ HID RFIDs...               }"},
	{"hitag",       CmdLFHitag,         1, "{ Hitag CHIPs...             }"},
	{"io",          CmdLFIO,            1, "{ ioProx RFIDs...            }"},
	{"indala",      CmdLFINDALA,        1, "{ Indala RFIDs...            }"},
	{"jablotron",   CmdLFJablotron,     1, "{ Jablotron RFIDs...         }"},
	{"nexwatch",    CmdLFNexWatch,      1, "{ NexWatch RFIDs...          }"},
	{"noralsy",     CmdLFNoralsy,       1, "{ Noralsy RFIDs...           }"},
	{"pac",         CmdLFPac,           1, "{ PAC/Stanley RFIDs...       }"},
	{"paradox",     CmdLFParadox,       1, "{ Paradox RFIDs...           }"},
	{"presco",      CmdLFPresco,        1, "{ Presco RFIDs...            }"},
	{"pcf7931",     CmdLFPCF7931,       1, "{ PCF7931 CHIPs...           }"},
	{"pyramid",     CmdLFPyramid,       1, "{ Farpointe/Pyramid RFIDs... }"},
	{"securakey",   CmdLFSecurakey,     1, "{ Securakey RFIDs...         }"},
	{"t55xx",       CmdLFT55XX,         1, "{ T55xx CHIPs...             }"},
	{"ti",          CmdLFTI,            1, "{ TI CHIPs...                }"},
	{"viking",      CmdLFViking,        1, "{ Viking RFIDs...            }"},
	{"visa2000",    CmdLFVisa2k,        1, "{ Visa2000 RFIDs...          }"},
	{"cmdread",     CmdLFCommandRead,   0, "<d period> <z period> <o period> <c command> ['H'] -- Modulate LF reader field to send command before read (all periods in microseconds) (option 'H' for 134)"},
	{"config",      CmdLFSetConfig,     0, "Set config for LF sampling, bit/sample, decimation, frequency"},
	{"flexdemod",   CmdFlexdemod,       1, "Demodulate samples for FlexPass"},
	{"read",        CmdLFRead,          0, "['s' silent] Read 125/134 kHz LF ID-only tag. Do 'lf read h' for help"},
	{"search",      CmdLFfind,          1, "[offline] ['u'] Read and Search for valid known tag (in offline mode it you can load first then search) - 'u' to search for unknown tags"},
	{"sim",         CmdLFSim,           0, "[GAP] -- Simulate LF tag from buffer with optional GAP (in microseconds)"},
	{"simask",      CmdLFaskSim,        0, "[clock] [invert <1|0>] [biphase/manchester/raw <'b'|'m'|'r'>] [msg separator 's'] [d <hexdata>] -- Simulate LF ASK tag from demodbuffer or input"},
	{"simfsk",      CmdLFfskSim,        0, "[c <clock>] [i] [H <fcHigh>] [L <fcLow>] [d <hexdata>] -- Simulate LF FSK tag from demodbuffer or input"},
	{"simpsk",      CmdLFpskSim,        0, "[1|2|3] [c <clock>] [i] [r <carrier>] [d <raw hex to sim>] -- Simulate LF PSK tag from demodbuffer or input"},
	{"simbidir",    CmdLFSimBidir,      0, "Simulate LF tag (with bidirectional data transmission between reader and tag)"},
	{"snoop",       CmdLFSnoop,         0, "['l'|'h'|<divisor>] [trigger threshold]-- Snoop LF (l:125khz, h:134khz)"},
	{"vchdemod",    CmdVchDemod,        1, "['clone'] -- Demodulate samples for VeriChip"},
	{NULL, NULL, 0, NULL}
};

int CmdLF(const char *Cmd)
{
	CmdsParse(CommandTable, Cmd);
	return 0; 
}

int CmdHelp(const char *Cmd)
{
	CmdsHelp(CommandTable);
	return 0;
}
