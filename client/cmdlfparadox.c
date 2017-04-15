//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Paradox tag commands
// FSK2a, rf/50, 96 bits (completely known)
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "cmdlfparadox.h"
#include "proxmark3.h"
#include "ui.h"
#include "util.h"
#include "graph.h"
#include "cmdparser.h"
#include "cmddata.h"
#include "cmdlf.h"
#include "lfdemod.h"
static int CmdHelp(const char *Cmd);

//by marshmellow
//Paradox Prox demod - FSK RF/50 with preamble of 00001111 (then manchester encoded)
//print full Paradox Prox ID and some bit format details if found
int CmdFSKdemodParadox(const char *Cmd)
{
	//raw fsk demod no manchester decoding no start bit finding just get binary from wave
	uint32_t hi2=0, hi=0, lo=0;

	uint8_t BitStream[MAX_GRAPH_TRACE_LEN]={0};
	size_t BitLen = getFromGraphBuf(BitStream);
	if (BitLen==0) return 0;
	int waveIdx=0;
	//get binary from fsk wave
	int idx = ParadoxdemodFSK(BitStream,&BitLen,&hi2,&hi,&lo,&waveIdx);
	if (idx<0){
		if (g_debugMode){
			if (idx==-1){
				PrintAndLog("DEBUG: Just Noise Detected");     
			} else if (idx == -2) {
				PrintAndLog("DEBUG: Error demoding fsk");
			} else if (idx == -3) {
				PrintAndLog("DEBUG: Preamble not found");
			} else if (idx == -4) {
				PrintAndLog("DEBUG: Error in Manchester data");
			} else {
				PrintAndLog("DEBUG: Error demoding fsk %d", idx);
			}
		}
		return 0;
	}
	if (hi2==0 && hi==0 && lo==0){
		if (g_debugMode) PrintAndLog("DEBUG: Error - no value found");
		return 0;
	}
	uint32_t fc = ((hi & 0x3)<<6) | (lo>>26);
	uint32_t cardnum = (lo>>10)&0xFFFF;
	uint32_t rawLo = bytebits_to_byte(BitStream+idx+64,32);
	uint32_t rawHi = bytebits_to_byte(BitStream+idx+32,32);
	uint32_t rawHi2 = bytebits_to_byte(BitStream+idx,32);

	PrintAndLog("Paradox TAG ID: %x%08x - FC: %d - Card: %d - Checksum: %02x - RAW: %08x%08x%08x",
		hi>>10, (hi & 0x3)<<26 | (lo>>10), fc, cardnum, (lo>>2) & 0xFF, rawHi2, rawHi, rawLo);
	setDemodBuf(BitStream,BitLen,idx);
	setClockGrid(50, waveIdx + (idx*50));
	if (g_debugMode){ 
		PrintAndLog("DEBUG: idx: %d, len: %d, Printing Demod Buffer:", idx, BitLen);
		printDemodBuff();
	}
	return 1;
}
//by marshmellow
//see ASKDemod for what args are accepted
int CmdParadoxRead(const char *Cmd) {
	// read lf silently
	lf_read(true, 10000);
	// demod and output viking ID	
	return CmdFSKdemodParadox(Cmd);
}

static command_t CommandTable[] = {
	{"help",  CmdHelp,            1, "This help"},
	{"demod", CmdFSKdemodParadox, 1, "Demodulate a Paradox FSK tag from the GraphBuffer"},
	{"read",  CmdParadoxRead,     0, "Attempt to read and Extract tag data from the antenna"},
	{NULL, NULL, 0, NULL}
};

int CmdLFParadox(const char *Cmd) {
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd) {
	CmdsHelp(CommandTable);
	return 0;
}
