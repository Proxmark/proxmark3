//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
// Merlok - 2017
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency commands
//-----------------------------------------------------------------------------

#include "cmdhf.h"

#include <math.h>
#include "usb_cmd.h"
#include "comms.h"
#include "ui.h"
#include "cmdparser.h"
#include "cliparser/cliparser.h"
#include "cmdhf14a.h"
#include "cmdhf14b.h"
#include "cmdhf15.h"
#include "cmdhfepa.h"
#include "cmdhflegic.h"
#include "cmdhficlass.h"
#include "cmdhfmf.h"
#include "cmdhfmfp.h"
#include "cmdhfmfu.h"
#include "cmdhftopaz.h"
#include "cmdhflist.h"
#include "cmdhffido.h"
#include "cmddata.h"
#include "graph.h"
#include "fpga.h"

static int CmdHelp(const char *Cmd);

int CmdHFTune(const char *Cmd)
{
  UsbCommand c={CMD_MEASURE_ANTENNA_TUNING_HF};
  SendCommand(&c);
  return 0;
}

int CmdHFSearch(const char *Cmd){
	int ans = 0;
	PrintAndLog("");
	ans = CmdHF14AInfo("s");
	if (ans > 0) {
		PrintAndLog("\nValid ISO14443A Tag Found - Quiting Search\n");
		return ans;
	}
	ans = HFiClassReader(false, false);
	if (ans) {
		PrintAndLog("\nValid iClass Tag (or PicoPass Tag) Found - Quiting Search\n");
		return ans;
	}
	ans = HF15Reader("", false);
	if (ans) {
		PrintAndLog("\nValid ISO15693 Tag Found - Quiting Search\n");
		return ans;
	}
	//14b is longest test currently (and rarest chip type) ... put last
	ans = infoHF14B(false);
	if (ans) {
		PrintAndLog("\nValid ISO14443B Tag Found - Quiting Search\n");
		return ans;
	}
	ans = CmdLegicRFRead("");
	if (ans == 0) {
		PrintAndLog("\nValid Legic Tag Found - Quiting Search\n");
		return ans;
	}
	PrintAndLog("\nno known/supported 13.56 MHz tags found\n");
	return 0;
}

int CmdHFSnoop(const char *Cmd)
{
	char * pEnd;
	UsbCommand c = {CMD_HF_SNIFFER, {strtol(Cmd, &pEnd,0),strtol(pEnd, &pEnd,0),0}};
	SendCommand(&c);
	return 0;
}


// static void InterpolateShannon(int *source, size_t source_len, int *dest, size_t dest_len)
// {
	// int *buf = (int*)malloc(source_len * sizeof(int));
	// memcpy(buf, source, source_len * sizeof(int));
	// for (int i = 0; i < source_len; i++) {
		// buf[i] += 128;
	// }
	// for (int i = 0; i < dest_len; i++) {
		// float value = 0.0;
		// for (int j = 0; j < source_len; j++) {
			// if (i * source_len == j * dest_len) { // sin(0) / 0 = 1
				// value += (float)buf[j];
			// } else {
				// value += (float)buf[j] * sin(((float)i*source_len/dest_len-j)*3.1415) / (((float)i*source_len/dest_len-j)*3.1415);
			// }
		// }
		// dest[i] = value - 128;
	// }
	// free(buf);
// }


static int CmdHFPlot(const char *Cmd)
{
	CLIParserInit("hf plot",
		"Plots HF signal after RF signal path and A/D conversion.",
		"This can be used after any hf command and will show the last few milliseconds of the HF signal.\n"
		"Note: If the last hf command terminated because of a timeout you will most probably see nothing.\n");
	void* argtable[] = {
		arg_param_begin,
		arg_param_end
	};
	CLIExecWithReturn(Cmd, argtable, true);

	uint8_t buf[FPGA_TRACE_SIZE];

	if (GetFromFpgaRAM(buf, FPGA_TRACE_SIZE)) {
		for (size_t i = 0; i < FPGA_TRACE_SIZE; i++) {
			GraphBuffer[i] = (int)buf[i] - 128;
		}
		GraphTraceLen = FPGA_TRACE_SIZE;
		// InterpolateShannon(GraphBuffer, FPGA_TRACE_SIZE, GraphBuffer, FPGA_TRACE_SIZE*8/7);
		// GraphTraceLen = FPGA_TRACE_SIZE*8/7;
		ShowGraphWindow();
		RepaintGraphWindow();
	}
	return 0;
}


static command_t CommandTable[] =
{
	{"help",    CmdHelp,        1, "This help"},
	{"14a",     CmdHF14A,       0, "{ ISO14443A RFIDs... }"},
	{"14b",     CmdHF14B,       0, "{ ISO14443B RFIDs... }"},
	{"15",      CmdHF15,        1, "{ ISO15693 RFIDs... }"},
	{"epa",     CmdHFEPA,       0, "{ German Identification Card... }"},
	{"legic",   CmdHFLegic,     0, "{ LEGIC RFIDs... }"},
	{"iclass",  CmdHFiClass,    1, "{ ICLASS RFIDs... }"},
	{"mf",      CmdHFMF,        1, "{ MIFARE RFIDs... }"},
	{"mfu",     CmdHFMFUltra,   1, "{ MIFARE Ultralight RFIDs... }"},
	{"mfp",     CmdHFMFP,       0, "{ MIFARE Plus RFIDs... }"},
	{"topaz",   CmdHFTopaz,     0, "{ TOPAZ (NFC Type 1) RFIDs... }"},
	{"fido",    CmdHFFido,      0, "{ FIDO and FIDO2 authenticators... }"},
	{"tune",    CmdHFTune,      0, "Continuously measure HF antenna tuning"},
	{"list",    CmdHFList,      1, "List protocol data in trace buffer"},
	{"plot",    CmdHFPlot,      0, "Plot signal"},
	{"search",  CmdHFSearch,    0, "Search for known HF tags [preliminary]"},
	{"snoop",   CmdHFSnoop,     0, "<samples to skip (10000)> <triggers to skip (1)> Generic HF Snoop"},
	{NULL,      NULL,           0, NULL}
};

int CmdHF(const char *Cmd)
{
  CmdsParse(CommandTable, Cmd);
  return 0;
}

int CmdHelp(const char *Cmd)
{
  CmdsHelp(CommandTable);
  return 0;
}
