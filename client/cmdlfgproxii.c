//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency G Prox II tag commands
// Biphase, rf/ , 96 bits  (unknown key calc + some bits)
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "cmdlfgproxii.h"
#include "proxmark3.h"
#include "ui.h"
#include "util.h"
#include "graph.h"
#include "cmdparser.h"
#include "cmddata.h"
#include "cmdmain.h"
#include "cmdlf.h"
#include "lfdemod.h"
static int CmdHelp(const char *Cmd);

//by marshmellow
//attempts to demodulate and identify a G_Prox_II verex/chubb card
//WARNING: if it fails during some points it will destroy the DemodBuffer data
// but will leave the GraphBuffer intact.
//if successful it will push askraw data back to demod buffer ready for emulation
int CmdG_Prox_II_Demod(const char *Cmd)
{
	if (!ASKbiphaseDemod(Cmd, false)){
		if (g_debugMode) PrintAndLog("Error gProxII: ASKbiphaseDemod failed 1st try");
		return 0;
	}
	size_t size = DemodBufferLen;
	//call lfdemod.c demod for gProxII
	int ans = gProxII_Demod(DemodBuffer, &size);
	if (ans < 0){
		if (g_debugMode) PrintAndLog("Error gProxII_Demod");
		return 0;
	}
	//got a good demod of 96 bits
	uint8_t ByteStream[8] = {0x00};
	uint8_t xorKey=0;
	size_t startIdx = ans + 6; //start after 6 bit preamble

	uint8_t bits_no_spacer[90];
	//so as to not mess with raw DemodBuffer copy to a new sample array
	memcpy(bits_no_spacer, DemodBuffer + startIdx, 90);
	// remove the 18 (90/5=18) parity bits (down to 72 bits (96-6-18=72))
	size_t bitLen = removeParity(bits_no_spacer, 0, 5, 3, 90); //source, startloc, paritylen, ptype, length_to_run
	if (bitLen != 72) {
		if (g_debugMode) PrintAndLog("Error gProxII: spacer removal did not produce 72 bits: %u, start: %u", bitLen, startIdx);
		return 0;
	}
	// get key and then get all 8 bytes of payload decoded
	xorKey = (uint8_t)bytebits_to_byteLSBF(bits_no_spacer, 8);
	for (size_t idx = 0; idx < 8; idx++) {
		ByteStream[idx] = ((uint8_t)bytebits_to_byteLSBF(bits_no_spacer+8 + (idx*8),8)) ^ xorKey;
		if (g_debugMode) PrintAndLog("byte %u after xor: %02x", (unsigned int)idx, ByteStream[idx]);
	}
	//now ByteStream contains 8 Bytes (64 bits) of decrypted raw tag data
	// 
	uint8_t fmtLen = ByteStream[0]>>2;
	uint32_t FC = 0;
	uint32_t Card = 0;
	//get raw 96 bits to print
	uint32_t raw1 = bytebits_to_byte(DemodBuffer+ans,32);
	uint32_t raw2 = bytebits_to_byte(DemodBuffer+ans+32, 32);
	uint32_t raw3 = bytebits_to_byte(DemodBuffer+ans+64, 32);

	if (fmtLen==36){
		FC = ((ByteStream[3] & 0x7F)<<7) | (ByteStream[4]>>1);
		Card = ((ByteStream[4]&1)<<19) | (ByteStream[5]<<11) | (ByteStream[6]<<3) | (ByteStream[7]>>5);
		PrintAndLog("G-Prox-II Found: FmtLen %d, FC %u, Card %u", (int)fmtLen, FC, Card);
	} else if(fmtLen==26){
		FC = ((ByteStream[3] & 0x7F)<<1) | (ByteStream[4]>>7);
		Card = ((ByteStream[4]&0x7F)<<9) | (ByteStream[5]<<1) | (ByteStream[6]>>7);
		PrintAndLog("G-Prox-II Found: FmtLen %d, FC %u, Card %u", (int)fmtLen, FC, Card);
	} else {
		PrintAndLog("Unknown G-Prox-II Fmt Found: FmtLen %d",(int)fmtLen);
		PrintAndLog("Decoded Raw: %s", sprint_hex(ByteStream, 8)); 
	}
	PrintAndLog("Raw: %08x%08x%08x", raw1,raw2,raw3);
	setDemodBuf(DemodBuffer, 96, ans);
	setClockGrid(g_DemodClock, g_DemodStartIdx + (ans*g_DemodClock));

	return 1;
}
//by marshmellow
//see ASKDemod for what args are accepted
int CmdG_Prox_II_Read(const char *Cmd) {
	// read lf silently
	lf_read(true, 10000);
	// demod and output viking ID	
	return CmdG_Prox_II_Demod(Cmd);
}

static command_t CommandTable[] = {
	{"help",  CmdHelp,            1, "This help"},
	{"demod", CmdG_Prox_II_Demod, 1, "Demodulate a G Prox II tag from the GraphBuffer"},
	{"read",  CmdG_Prox_II_Read,  0, "Attempt to read and Extract tag data from the antenna"},
	{NULL, NULL, 0, NULL}
};

int CmdLF_G_Prox_II(const char *Cmd) {
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd) {
	CmdsHelp(CommandTable);
	return 0;
}
