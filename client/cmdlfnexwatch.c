//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Honeywell NexWatch tag commands
// PSK1 RF/16, RF/2, 128 bits long (known)
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include "cmdlfnexwatch.h"
#include "proxmark3.h"
#include "ui.h"
#include "util.h"
#include "graph.h"
#include "cmdparser.h"
#include "cmddata.h"
#include "cmdlf.h"
#include "lfdemod.h"

static int CmdHelp(pm3_connection* conn, const char *Cmd);

int CmdPSKNexWatch(pm3_connection* conn, const char *Cmd)
{
	if (!PSKDemod(conn, "", false)) return 0;
	uint8_t preamble[28] = {0,0,0,0,0,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	size_t startIdx = 0, size = conn->DemodBufferLen; 
	bool invert = false;
	if (!preambleSearch(conn->DemodBuffer, preamble, sizeof(preamble), &size, &startIdx)){
		// if didn't find preamble try again inverting
		if (!PSKDemod(conn, "1", false)) return 0; 
		size = conn->DemodBufferLen;
		if (!preambleSearch(conn->DemodBuffer, preamble, sizeof(preamble), &size, &startIdx)) return 0;
		invert = true;
	}
	if (size != 128) return 0;
	setDemodBuf(conn, conn->DemodBuffer, size, startIdx+4);
	setClockGrid(conn, conn->g_DemodClock, conn->g_DemodStartIdx + ((startIdx+4)*conn->g_DemodClock));
	startIdx = 8+32; // 8 = preamble, 32 = reserved bits (always 0)
	//get ID
	uint32_t ID = 0;
	for (uint8_t wordIdx=0; wordIdx<4; wordIdx++){
		for (uint8_t idx=0; idx<8; idx++){
			ID = (ID << 1) | conn->DemodBuffer[startIdx+wordIdx+(idx*4)];
		}	
	}
	//parity check (TBD)

	//checksum check (TBD)

	//output
	PrintAndLog("NexWatch ID: %d", ID);
	if (invert){
		PrintAndLog("Had to Invert - probably NexKey");
		for (uint8_t idx=0; idx<size; idx++)
			conn->DemodBuffer[idx] ^= 1;
	} 

	CmdPrintDemodBuff(conn, "x");
	return 1;
}

//by marshmellow
//see ASKDemod for what args are accepted
int CmdNexWatchRead(pm3_connection* conn, const char *Cmd) {
	// read lf silently
	lf_read(conn, true, 10000);
	// demod and output viking ID	
	return CmdPSKNexWatch(conn, Cmd);
}

static command_t CommandTable[] = {
	{"help",  CmdHelp,          1, "This help"},
	{"demod", CmdPSKNexWatch,   1, "Demodulate a NexWatch tag (nexkey, quadrakey) from the GraphBuffer"},
	{"read",  CmdNexWatchRead,  0, "Attempt to Read and Extract tag data from the antenna"},
	{NULL, NULL, 0, NULL}
};

int CmdLFNexWatch(pm3_connection* conn, const char *Cmd) {
	CmdsParse(conn, CommandTable, Cmd);
	return 0;
}

int CmdHelp(pm3_connection* conn, const char *Cmd) {
	CmdsHelp(conn, CommandTable);
	return 0;
}
