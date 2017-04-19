//-----------------------------------------------------------------------------
// Authored by Craig Young <cyoung@tripwire.com> based on cmdlfhid.c structure
//
// cmdlfhid.c is Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency AWID26 commands
// FSK2a, RF/50, 96 bits (complete)
//-----------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>      // sscanf
#include "proxmark3.h"  // Definitions, USB controls, etc
#include "cmdlfawid.h"
#include "ui.h"         // PrintAndLog
#include "cmdparser.h"  // CmdsParse, CmdsHelp
#include "lfdemod.h"    // parityTest +
#include "util.h"       // weigandparity
#include "protocols.h"  // for T55xx config register definitions
#include "cmddata.h"    // for printDemod and demodbuffer commands
#include "graph.h"      // for getFromGraphBuff cmds
#include "cmdmain.h"

static int CmdHelp(const char *Cmd);

int usage_lf_awid_read(void) {
	PrintAndLog("Enables AWID26 compatible reader mode printing details of scanned AWID26 tags.");
	PrintAndLog("By default, values are printed and logged until the button is pressed or another USB command is issued.");
	PrintAndLog("If the ['1'] option is provided, reader mode is exited after reading a single AWID26 card.");
	PrintAndLog("");
	PrintAndLog("Usage:  lf awid read ['1']");
	PrintAndLog("Options : ");
	PrintAndLog("  1 : (optional) stop after reading a single card");
	PrintAndLog("");
	PrintAndLog("Samples : lf awid read");
	PrintAndLog("        : lf awid read 1");
	return 0;
}

int usage_lf_awid_sim(void) {
	PrintAndLog("Enables simulation of AWID26 card with specified facility-code and card number.");
	PrintAndLog("Simulation runs until the button is pressed or another USB command is issued.");
	PrintAndLog("Per AWID26 format, the facility-code is 8-bit and the card number is 16-bit.  Larger values are truncated.");
	PrintAndLog("");
	PrintAndLog("Usage:  lf awid sim <Facility-Code> <Card-Number>");
	PrintAndLog("Options : ");
	PrintAndLog("  <Facility-Code> : 8-bit value representing the AWID facility code");
	PrintAndLog("  <Card Number>   : 16-bit value representing the AWID card number");
	PrintAndLog("");
	PrintAndLog("Sample : lf awid sim 224 1337");
	return 0;
}

int usage_lf_awid_clone(void) {
	PrintAndLog("Enables cloning of AWID26 card with specified facility-code and card number onto T55x7.");
	PrintAndLog("The T55x7 must be on the antenna when issuing this command.  T55x7 blocks are calculated and printed in the process.");
	PrintAndLog("Per AWID26 format, the facility-code is 8-bit and the card number is 16-bit.  Larger values are truncated.");
	PrintAndLog("");
	PrintAndLog("Usage:  lf awid clone <Facility-Code> <Card-Number>");
	PrintAndLog("Options : ");
	PrintAndLog("  <Facility-Code> : 8-bit value representing the AWID facility code");
	PrintAndLog("  <Card Number>   : 16-bit value representing the AWID card number");
	PrintAndLog("  Q5              : optional - clone to Q5 (T5555) instead of T55x7 chip");
	PrintAndLog("");
	PrintAndLog("Sample  : lf awid clone 224 1337");
	return 0;
}

int CmdAWIDReadFSK(const char *Cmd) {
	int findone=0;
	if (Cmd[0] == 'h' || Cmd[0] == 'H') return usage_lf_awid_read();
	if (Cmd[0] == '1') findone = 1;

	UsbCommand c = {CMD_AWID_DEMOD_FSK, {findone, 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);
	return 0;   
}
//by marshmellow
//AWID Prox demod - FSK RF/50 with preamble of 00000001  (always a 96 bit data stream)
//print full AWID Prox ID and some bit format details if found
int CmdFSKdemodAWID(const char *Cmd)
{
	uint8_t BitStream[MAX_GRAPH_TRACE_LEN]={0};
	size_t size = getFromGraphBuf(BitStream);
	if (size==0) return 0;

	int waveIdx = 0;
	//get binary from fsk wave
	int idx = AWIDdemodFSK(BitStream, &size, &waveIdx);
	if (idx<=0){
		if (g_debugMode){
			if (idx == -1)
				PrintAndLog("DEBUG: Error - not enough samples");
			else if (idx == -2)
				PrintAndLog("DEBUG: Error - only noise found");
			else if (idx == -3)
				PrintAndLog("DEBUG: Error - problem during FSK demod");
			else if (idx == -4)
				PrintAndLog("DEBUG: Error - AWID preamble not found");
			else if (idx == -5)
				PrintAndLog("DEBUG: Error - Size not correct: %d", size);
			else
				PrintAndLog("DEBUG: Error %d",idx);
		}
		return 0;
	}

	// Index map
	// 0            10            20            30              40            50              60
	// |            |             |             |               |             |               |
	// 01234567 890 1 234 5 678 9 012 3 456 7 890 1 234 5 678 9 012 3 456 7 890 1 234 5 678 9 012 3 - to 96
	// -----------------------------------------------------------------------------
	// 00000001 000 1 110 1 101 1 011 1 101 1 010 0 000 1 000 1 010 0 001 0 110 1 100 0 000 1 000 1
	// premable bbb o bbb o bbw o fff o fff o ffc o ccc o ccc o ccc o ccc o ccc o wxx o xxx o xxx o - to 96
	//          |---26 bit---|    |-----117----||-------------142-------------|
	// b = format bit len, o = odd parity of last 3 bits
	// f = facility code, c = card number
	// w = wiegand parity
	// (26 bit format shown)
 
	//get raw ID before removing parities
	uint32_t rawLo = bytebits_to_byte(BitStream+idx+64,32);
	uint32_t rawHi = bytebits_to_byte(BitStream+idx+32,32);
	uint32_t rawHi2 = bytebits_to_byte(BitStream+idx,32);
	setDemodBuf(BitStream,96,idx);
	setClockGrid(50, waveIdx + (idx*50));

	size = removeParity(BitStream, idx+8, 4, 1, 88);
	if (size != 66){
		if (g_debugMode) PrintAndLog("DEBUG: Error - at parity check-tag size does not match AWID format");
		return 0;
	}
	// ok valid card found!

	// Index map
	// 0           10         20        30          40        50        60
	// |           |          |         |           |         |         |
	// 01234567 8 90123456 7890123456789012 3 456789012345678901234567890123456
	// -----------------------------------------------------------------------------
	// 00011010 1 01110101 0000000010001110 1 000000000000000000000000000000000
	// bbbbbbbb w ffffffff cccccccccccccccc w xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	// |26 bit|   |-117--| |-----142------|
	// b = format bit len, o = odd parity of last 3 bits
	// f = facility code, c = card number
	// w = wiegand parity
	// (26 bit format shown)

	uint32_t fc = 0;
	uint32_t cardnum = 0;
	uint32_t code1 = 0;
	uint32_t code2 = 0;
	uint8_t fmtLen = bytebits_to_byte(BitStream,8);
	if (fmtLen==26){
		fc = bytebits_to_byte(BitStream+9, 8);
		cardnum = bytebits_to_byte(BitStream+17, 16);
		code1 = bytebits_to_byte(BitStream+8,fmtLen);
		PrintAndLog("AWID Found - BitLength: %d, FC: %d, Card: %d - Wiegand: %x, Raw: %08x%08x%08x", fmtLen, fc, cardnum, code1, rawHi2, rawHi, rawLo);
	} else {
		cardnum = bytebits_to_byte(BitStream+8+(fmtLen-17), 16);
		if (fmtLen>32){
			code1 = bytebits_to_byte(BitStream+8,fmtLen-32);
			code2 = bytebits_to_byte(BitStream+8+(fmtLen-32),32);
			PrintAndLog("AWID Found - BitLength: %d -unknown BitLength- (%d) - Wiegand: %x%08x, Raw: %08x%08x%08x", fmtLen, cardnum, code1, code2, rawHi2, rawHi, rawLo);
		} else{
			code1 = bytebits_to_byte(BitStream+8,fmtLen);
			PrintAndLog("AWID Found - BitLength: %d -unknown BitLength- (%d) - Wiegand: %x, Raw: %08x%08x%08x", fmtLen, cardnum, code1, rawHi2, rawHi, rawLo);
		}
	}
	if (g_debugMode){
		PrintAndLog("DEBUG: idx: %d, Len: %d Printing Demod Buffer:", idx, 96);
		printDemodBuff();
	}
	//todo - convert hi2, hi, lo to demodbuffer for future sim/clone commands
	return 1;
}

//refactored by marshmellow
int getAWIDBits(uint32_t fc, uint32_t cn, uint8_t	*AWIDBits) {
	uint8_t pre[66];
	memset(pre, 0, sizeof(pre));
	AWIDBits[7]=1;
	num_to_bytebits(26, 8, pre);

	uint8_t wiegand[24];
	num_to_bytebits(fc, 8, wiegand);
	num_to_bytebits(cn, 16, wiegand+8);

	wiegand_add_parity(pre+8, wiegand, 24);

	size_t bitLen = addParity(pre, AWIDBits+8, 66, 4, 1);
	if (bitLen != 88) return 0;
	//for (uint8_t i = 0; i<3; i++){
	//	PrintAndLog("DEBUG: %08X", bytebits_to_byte(AWIDBits+(32*i),32));
	//}
	return 1;
}

int CmdAWIDSim(const char *Cmd) {
	uint32_t fcode = 0, cnum = 0, fc=0, cn=0;
	uint8_t BitStream[96];
	uint8_t *bs = BitStream;
	size_t size = sizeof(BitStream);
	memset(bs, 0, size);

	uint64_t arg1 = (10<<8) + 8; // fcHigh = 10, fcLow = 8
	uint64_t arg2 = 50; // clk RF/50 invert=0
	
	if (sscanf(Cmd, "%u %u", &fc, &cn ) != 2) return usage_lf_awid_sim();

	fcode = (fc & 0x000000FF);
	cnum = (cn & 0x0000FFFF);

	if (fc != fcode) PrintAndLog("Facility-Code (%u) truncated to 8-bits: %u",fc,fcode);
	if (cn != cnum) PrintAndLog("Card number (%u) truncated to 16-bits: %u",cn,cnum);

	PrintAndLog("Emulating AWID26 -- FC: %u; CN: %u\n",fcode,cnum);
	PrintAndLog("Press pm3-button to abort simulation or run another command");

	if (!getAWIDBits(fc, cn, bs)) {
		PrintAndLog("Error with tag bitstream generation.");
		return 1;
	}
	// AWID uses: fcHigh: 10, fcLow: 8, clk: 50, invert: 0
	UsbCommand c = {CMD_FSK_SIM_TAG, {arg1, arg2, size}};
	memcpy(c.d.asBytes, bs, size);
	clearCommandBuffer();
	SendCommand(&c);
	return 0;
}

int CmdAWIDClone(const char *Cmd) {
	uint32_t blocks[4] = {T55x7_MODULATION_FSK2a | T55x7_BITRATE_RF_50 | 3<<T55x7_MAXBLOCK_SHIFT, 0, 0, 0};
	uint32_t fc=0,cn=0;
	uint8_t BitStream[96];
	uint8_t *bs=BitStream;
	memset(bs,0,sizeof(BitStream));

	if (sscanf(Cmd, "%u %u", &fc, &cn ) != 2) return usage_lf_awid_clone();

	if (param_getchar(Cmd, 3) == 'Q' || param_getchar(Cmd, 3) == 'q')
		blocks[0] = T5555_MODULATION_FSK2 | T5555_INVERT_OUTPUT | ((50-2)>>1)<<T5555_BITRATE_SHIFT | 3<<T5555_MAXBLOCK_SHIFT;

	if ((fc & 0xFF) != fc) {
		fc &= 0xFF;
		PrintAndLog("Facility-Code Truncated to 8-bits (AWID26): %u", fc);
	}
	if ((cn & 0xFFFF) != cn) {
		cn &= 0xFFFF;
		PrintAndLog("Card Number Truncated to 16-bits (AWID26): %u", cn);
	}

	if ( !getAWIDBits(fc, cn, bs)) {
		PrintAndLog("Error with tag bitstream generation.");
		return 1;
	}

	blocks[1] = bytebits_to_byte(bs,32);
	blocks[2] = bytebits_to_byte(bs+32,32);
	blocks[3] = bytebits_to_byte(bs+64,32);

	PrintAndLog("Preparing to clone AWID26 to T55x7 with FC: %u, CN: %u", 
	    fc, cn);
	PrintAndLog("Blk | Data ");
	PrintAndLog("----+------------");
	PrintAndLog(" 00 | 0x%08x", blocks[0]);
	PrintAndLog(" 01 | 0x%08x", blocks[1]);
	PrintAndLog(" 02 | 0x%08x", blocks[2]);
	PrintAndLog(" 03 | 0x%08x", blocks[3]);

	UsbCommand resp;
	UsbCommand c = {CMD_T55XX_WRITE_BLOCK, {0,0,0}};

	for (uint8_t i=0; i<4; i++) {
		c.cmd = CMD_T55XX_WRITE_BLOCK;
		c.arg[0] = blocks[i];
		c.arg[1] = i;
		c.arg[2] = 0;
		clearCommandBuffer();
		SendCommand(&c);
		if (!WaitForResponseTimeout(CMD_ACK, &resp, 1000)){
			PrintAndLog("Error occurred, device did not respond during write operation.");
			return -1;
		}

	}
	return 0;
}

static command_t CommandTable[] = {
	{"help",      CmdHelp,         1, "This help"},
	{"demod",     CmdFSKdemodAWID, 1, "Demodulate an AWID FSK tag from the GraphBuffer"},
	{"read",      CmdAWIDReadFSK,  0, "['1'] Realtime AWID FSK read from the antenna (option '1' for one tag only)"},
	{"sim",       CmdAWIDSim,      0, "<Facility-Code> <Card Number> -- AWID tag simulator"},
	{"clone",     CmdAWIDClone,    0, "<Facility-Code> <Card Number> <Q5> -- Clone AWID to T55x7 (tag must be in range of antenna)"},
	{NULL, NULL, 0, NULL}
};

int CmdLFAWID(const char *Cmd) {
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd) {
	CmdsHelp(CommandTable);
	return 0;
}
