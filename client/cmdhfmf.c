//-----------------------------------------------------------------------------
// Copyright (C) 2011,2012 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency MIFARE commands
//-----------------------------------------------------------------------------

#include "cmdhfmf.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "proxmark3.h"
#include "cmdmain.h"
#include "cmdhfmfhard.h"
#include "util.h"
#include "util_posix.h"
#include "usb_cmd.h"
#include "ui.h"
#include "mifarehost.h"
#include "mifare.h"
#include "mfkey.h"

#define NESTED_SECTOR_RETRY     10			// how often we try mfested() until we give up

static int CmdHelp(const char *Cmd);

int CmdHF14AMifare(const char *Cmd)
{
	int isOK = 0;
	uint64_t key = 0;
	isOK = mfDarkside(&key);
	switch (isOK) {
		case -1 : PrintAndLog("Button pressed. Aborted."); return 1;
		case -2 : PrintAndLog("Card is not vulnerable to Darkside attack (doesn't send NACK on authentication requests)."); return 1;
		case -3 : PrintAndLog("Card is not vulnerable to Darkside attack (its random number generator is not predictable)."); return 1;
		case -4 : PrintAndLog("Card is not vulnerable to Darkside attack (its random number generator seems to be based on the wellknown");
				  PrintAndLog("generating polynomial with 16 effective bits only, but shows unexpected behaviour."); return 1;
		case -5 : PrintAndLog("Aborted via keyboard.");  return 1;
		default : PrintAndLog("Found valid key:%012" PRIx64 "\n", key);
	}

	PrintAndLog("");
	return 0;
}


int CmdHF14AMfWrBl(const char *Cmd)
{
	uint8_t blockNo = 0;
	uint8_t keyType = 0;
	uint8_t key[6] = {0, 0, 0, 0, 0, 0};
	uint8_t bldata[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	char cmdp	= 0x00;

	if (strlen(Cmd)<3) {
		PrintAndLog("Usage:  hf mf wrbl    <block number> <key A/B> <key (12 hex symbols)> <block data (32 hex symbols)>");
		PrintAndLog("        sample: hf mf wrbl 0 A FFFFFFFFFFFF 000102030405060708090A0B0C0D0E0F");
		return 0;
	}

	blockNo = param_get8(Cmd, 0);
	cmdp = param_getchar(Cmd, 1);
	if (cmdp == 0x00) {
		PrintAndLog("Key type must be A or B");
		return 1;
	}
	if (cmdp != 'A' && cmdp != 'a') keyType = 1;
	if (param_gethex(Cmd, 2, key, 12)) {
		PrintAndLog("Key must include 12 HEX symbols");
		return 1;
	}
	if (param_gethex(Cmd, 3, bldata, 32)) {
		PrintAndLog("Block data must include 32 HEX symbols");
		return 1;
	}
	PrintAndLog("--block no:%d, key type:%c, key:%s", blockNo, keyType?'B':'A', sprint_hex(key, 6));
	PrintAndLog("--data: %s", sprint_hex(bldata, 16));

  UsbCommand c = {CMD_MIFARE_WRITEBL, {blockNo, keyType, 0}};
	memcpy(c.d.asBytes, key, 6);
	memcpy(c.d.asBytes + 10, bldata, 16);
  SendCommand(&c);

	UsbCommand resp;
	if (WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
		uint8_t isOK  = resp.arg[0] & 0xff;
		PrintAndLog("isOk:%02x", isOK);
	} else {
		PrintAndLog("Command execute timeout");
	}

	return 0;
}

int CmdHF14AMfRdBl(const char *Cmd)
{
	uint8_t blockNo = 0;
	uint8_t keyType = 0;
	uint8_t key[6] = {0, 0, 0, 0, 0, 0};

	char cmdp	= 0x00;


	if (strlen(Cmd)<3) {
		PrintAndLog("Usage:  hf mf rdbl    <block number> <key A/B> <key (12 hex symbols)>");
		PrintAndLog("        sample: hf mf rdbl 0 A FFFFFFFFFFFF ");
		return 0;
	}

	blockNo = param_get8(Cmd, 0);
	cmdp = param_getchar(Cmd, 1);
	if (cmdp == 0x00) {
		PrintAndLog("Key type must be A or B");
		return 1;
	}
	if (cmdp != 'A' && cmdp != 'a') keyType = 1;
	if (param_gethex(Cmd, 2, key, 12)) {
		PrintAndLog("Key must include 12 HEX symbols");
		return 1;
	}
	PrintAndLog("--block no:%d, key type:%c, key:%s ", blockNo, keyType?'B':'A', sprint_hex(key, 6));

  UsbCommand c = {CMD_MIFARE_READBL, {blockNo, keyType, 0}};
	memcpy(c.d.asBytes, key, 6);
  SendCommand(&c);

	UsbCommand resp;
	if (WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
		uint8_t isOK  = resp.arg[0] & 0xff;
		uint8_t *data = resp.d.asBytes;

		if (isOK)
			PrintAndLog("isOk:%02x data:%s", isOK, sprint_hex(data, 16));
		else
			PrintAndLog("isOk:%02x", isOK);
	} else {
		PrintAndLog("Command execute timeout");
	}

  return 0;
}

int CmdHF14AMfRdSc(const char *Cmd)
{
	int i;
	uint8_t sectorNo = 0;
	uint8_t keyType = 0;
	uint8_t key[6] = {0, 0, 0, 0, 0, 0};
	uint8_t isOK  = 0;
	uint8_t *data  = NULL;
	char cmdp	= 0x00;

	if (strlen(Cmd)<3) {
		PrintAndLog("Usage:  hf mf rdsc    <sector number> <key A/B> <key (12 hex symbols)>");
		PrintAndLog("        sample: hf mf rdsc 0 A FFFFFFFFFFFF ");
		return 0;
	}

	sectorNo = param_get8(Cmd, 0);
	if (sectorNo > 39) {
		PrintAndLog("Sector number must be less than 40");
		return 1;
	}
	cmdp = param_getchar(Cmd, 1);
	if (cmdp != 'a' && cmdp != 'A' && cmdp != 'b' && cmdp != 'B') {
		PrintAndLog("Key type must be A or B");
		return 1;
	}
	if (cmdp != 'A' && cmdp != 'a') keyType = 1;
	if (param_gethex(Cmd, 2, key, 12)) {
		PrintAndLog("Key must include 12 HEX symbols");
		return 1;
	}
	PrintAndLog("--sector no:%d key type:%c key:%s ", sectorNo, keyType?'B':'A', sprint_hex(key, 6));

	UsbCommand c = {CMD_MIFARE_READSC, {sectorNo, keyType, 0}};
	memcpy(c.d.asBytes, key, 6);
	SendCommand(&c);
	PrintAndLog(" ");

	UsbCommand resp;
	if (WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
		isOK  = resp.arg[0] & 0xff;
		data  = resp.d.asBytes;

		PrintAndLog("isOk:%02x", isOK);
		if (isOK) {
			for (i = 0; i < (sectorNo<32?3:15); i++) {
				PrintAndLog("data   : %s", sprint_hex(data + i * 16, 16));
			}
			PrintAndLog("trailer: %s", sprint_hex(data + (sectorNo<32?3:15) * 16, 16));
		}
	} else {
		PrintAndLog("Command execute timeout");
	}

  return 0;
}

uint8_t FirstBlockOfSector(uint8_t sectorNo)
{
	if (sectorNo < 32) {
		return sectorNo * 4;
	} else {
		return 32 * 4 + (sectorNo - 32) * 16;
	}
}

uint8_t NumBlocksPerSector(uint8_t sectorNo)
{
	if (sectorNo < 32) {
		return 4;
	} else {
		return 16;
	}
}

static int ParamCardSizeSectors(const char c) {
	int numBlocks = 16;
	switch (c) {
		case '0' : numBlocks = 5; break;
		case '2' : numBlocks = 32; break;
		case '4' : numBlocks = 40; break;
		default:   numBlocks = 16;
	}
	return numBlocks;
}

static int ParamCardSizeBlocks(const char c) {
	int numBlocks = 16 * 4;
	switch (c) {
		case '0' : numBlocks = 5 * 4; break;
		case '2' : numBlocks = 32 * 4; break;
		case '4' : numBlocks = 32 * 4 + 8 * 16; break;
		default:   numBlocks = 16 * 4;
	}
	return numBlocks;
}

int CmdHF14AMfDump(const char *Cmd)
{
	uint8_t sectorNo, blockNo;

	uint8_t keyA[40][6];
	uint8_t keyB[40][6];
	uint8_t rights[40][4];
	uint8_t carddata[256][16];
	uint8_t numSectors = 16;

	FILE *fin;
	FILE *fout;

	UsbCommand resp;

	char cmdp = param_getchar(Cmd, 0);
	numSectors = ParamCardSizeSectors(cmdp);

	if (strlen(Cmd) > 1 || cmdp == 'h' || cmdp == 'H') {
		PrintAndLog("Usage:   hf mf dump [card memory]");
		PrintAndLog("  [card memory]: 0 = 320 bytes (Mifare Mini), 1 = 1K (default), 2 = 2K, 4 = 4K");
		PrintAndLog("");
		PrintAndLog("Samples: hf mf dump");
		PrintAndLog("         hf mf dump 4");
		return 0;
	}

	if ((fin = fopen("dumpkeys.bin","rb")) == NULL) {
		PrintAndLog("Could not find file dumpkeys.bin");
		return 1;
	}

	// Read keys A from file
	for (sectorNo=0; sectorNo<numSectors; sectorNo++) {
		size_t bytes_read = fread(keyA[sectorNo], 1, 6, fin);
		if (bytes_read != 6) {
			PrintAndLog("File reading error.");
			fclose(fin);
			return 2;
		}
	}

	// Read keys B from file
	for (sectorNo=0; sectorNo<numSectors; sectorNo++) {
		size_t bytes_read = fread(keyB[sectorNo], 1, 6, fin);
		if (bytes_read != 6) {
			PrintAndLog("File reading error.");
			fclose(fin);
			return 2;
		}
	}

	fclose(fin);

	PrintAndLog("|-----------------------------------------|");
	PrintAndLog("|------ Reading sector access bits...-----|");
	PrintAndLog("|-----------------------------------------|");
	uint8_t tries = 0;
	for (sectorNo = 0; sectorNo < numSectors; sectorNo++) {
		for (tries = 0; tries < 3; tries++) {
			UsbCommand c = {CMD_MIFARE_READBL, {FirstBlockOfSector(sectorNo) + NumBlocksPerSector(sectorNo) - 1, 0, 0}};
			memcpy(c.d.asBytes, keyA[sectorNo], 6);
			SendCommand(&c);

			if (WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
				uint8_t isOK  = resp.arg[0] & 0xff;
				uint8_t *data  = resp.d.asBytes;
				if (isOK){
					rights[sectorNo][0] = ((data[7] & 0x10)>>2) | ((data[8] & 0x1)<<1) | ((data[8] & 0x10)>>4); // C1C2C3 for data area 0
					rights[sectorNo][1] = ((data[7] & 0x20)>>3) | ((data[8] & 0x2)<<0) | ((data[8] & 0x20)>>5); // C1C2C3 for data area 1
					rights[sectorNo][2] = ((data[7] & 0x40)>>4) | ((data[8] & 0x4)>>1) | ((data[8] & 0x40)>>6); // C1C2C3 for data area 2
					rights[sectorNo][3] = ((data[7] & 0x80)>>5) | ((data[8] & 0x8)>>2) | ((data[8] & 0x80)>>7); // C1C2C3 for sector trailer
					break;
				} else if (tries == 2) { // on last try set defaults
					PrintAndLog("Could not get access rights for sector %2d. Trying with defaults...", sectorNo);
					rights[sectorNo][0] = rights[sectorNo][1] = rights[sectorNo][2] = 0x00;
					rights[sectorNo][3] = 0x01;
				}
			} else {
				PrintAndLog("Command execute timeout when trying to read access rights for sector %2d. Trying with defaults...", sectorNo);
				rights[sectorNo][0] = rights[sectorNo][1] = rights[sectorNo][2] = 0x00;
				rights[sectorNo][3] = 0x01;
			}
		}
	}

	PrintAndLog("|-----------------------------------------|");
	PrintAndLog("|----- Dumping all blocks to file... -----|");
	PrintAndLog("|-----------------------------------------|");

	bool isOK = true;
	for (sectorNo = 0; isOK && sectorNo < numSectors; sectorNo++) {
		for (blockNo = 0; isOK && blockNo < NumBlocksPerSector(sectorNo); blockNo++) {
			bool received = false;
			for (tries = 0; tries < 3; tries++) {
				if (blockNo == NumBlocksPerSector(sectorNo) - 1) {		// sector trailer. At least the Access Conditions can always be read with key A.
					UsbCommand c = {CMD_MIFARE_READBL, {FirstBlockOfSector(sectorNo) + blockNo, 0, 0}};
					memcpy(c.d.asBytes, keyA[sectorNo], 6);
					SendCommand(&c);
					received = WaitForResponseTimeout(CMD_ACK,&resp,1500);
				} else {												// data block. Check if it can be read with key A or key B
					uint8_t data_area = sectorNo<32?blockNo:blockNo/5;
					if ((rights[sectorNo][data_area] == 0x03) || (rights[sectorNo][data_area] == 0x05)) {	// only key B would work
						UsbCommand c = {CMD_MIFARE_READBL, {FirstBlockOfSector(sectorNo) + blockNo, 1, 0}};
						memcpy(c.d.asBytes, keyB[sectorNo], 6);
						SendCommand(&c);
						received = WaitForResponseTimeout(CMD_ACK,&resp,1500);
					} else if (rights[sectorNo][data_area] == 0x07) {										// no key would work
						isOK = false;
						PrintAndLog("Access rights do not allow reading of sector %2d block %3d", sectorNo, blockNo);
						tries = 2;
					} else {																				// key A would work
						UsbCommand c = {CMD_MIFARE_READBL, {FirstBlockOfSector(sectorNo) + blockNo, 0, 0}};
						memcpy(c.d.asBytes, keyA[sectorNo], 6);
						SendCommand(&c);
						received = WaitForResponseTimeout(CMD_ACK,&resp,1500);
					}
				}
				if (received) {
					isOK  = resp.arg[0] & 0xff;
					if (isOK) break;
				}
			}

			if (received) {
				isOK  = resp.arg[0] & 0xff;
				uint8_t *data  = resp.d.asBytes;
				if (blockNo == NumBlocksPerSector(sectorNo) - 1) {		// sector trailer. Fill in the keys.
					data[0]  = (keyA[sectorNo][0]);
					data[1]  = (keyA[sectorNo][1]);
					data[2]  = (keyA[sectorNo][2]);
					data[3]  = (keyA[sectorNo][3]);
					data[4]  = (keyA[sectorNo][4]);
					data[5]  = (keyA[sectorNo][5]);
					data[10] = (keyB[sectorNo][0]);
					data[11] = (keyB[sectorNo][1]);
					data[12] = (keyB[sectorNo][2]);
					data[13] = (keyB[sectorNo][3]);
					data[14] = (keyB[sectorNo][4]);
					data[15] = (keyB[sectorNo][5]);
				}
				if (isOK) {
					memcpy(carddata[FirstBlockOfSector(sectorNo) + blockNo], data, 16);
                    PrintAndLog("Successfully read block %2d of sector %2d.", blockNo, sectorNo);
				} else {
					PrintAndLog("Could not read block %2d of sector %2d", blockNo, sectorNo);
					break;
				}
			}
			else {
				isOK = false;
				PrintAndLog("Command execute timeout when trying to read block %2d of sector %2d.", blockNo, sectorNo);
				break;
			}
		}
	}

	if (isOK) {
		if ((fout = fopen("dumpdata.bin","wb")) == NULL) {
			PrintAndLog("Could not create file name dumpdata.bin");
			return 1;
		}
		uint16_t numblocks = FirstBlockOfSector(numSectors - 1) + NumBlocksPerSector(numSectors - 1);
		fwrite(carddata, 1, 16*numblocks, fout);
		fclose(fout);
		PrintAndLog("Dumped %d blocks (%d bytes) to file dumpdata.bin", numblocks, 16*numblocks);
	}

	return 0;
}

int CmdHF14AMfRestore(const char *Cmd)
{
	uint8_t sectorNo,blockNo;
	uint8_t keyType = 0;
	uint8_t key[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	uint8_t bldata[16] = {0x00};
	uint8_t keyA[40][6];
	uint8_t keyB[40][6];
	uint8_t numSectors;

	FILE *fdump;
	FILE *fkeys;

	char cmdp = param_getchar(Cmd, 0);
	switch (cmdp) {
		case '0' : numSectors = 5; break;
		case '1' :
		case '\0': numSectors = 16; break;
		case '2' : numSectors = 32; break;
		case '4' : numSectors = 40; break;
		default:   numSectors = 16;
	}

	if (strlen(Cmd) > 1 || cmdp == 'h' || cmdp == 'H') {
		PrintAndLog("Usage:   hf mf restore [card memory]");
		PrintAndLog("  [card memory]: 0 = 320 bytes (Mifare Mini), 1 = 1K (default), 2 = 2K, 4 = 4K");
		PrintAndLog("");
		PrintAndLog("Samples: hf mf restore");
		PrintAndLog("         hf mf restore 4");
		return 0;
	}

	if ((fkeys = fopen("dumpkeys.bin","rb")) == NULL) {
		PrintAndLog("Could not find file dumpkeys.bin");
		return 1;
	}

	for (sectorNo = 0; sectorNo < numSectors; sectorNo++) {
		size_t bytes_read = fread(keyA[sectorNo], 1, 6, fkeys);
		if (bytes_read != 6) {
			PrintAndLog("File reading error (dumpkeys.bin).");
			fclose(fkeys);
			return 2;
		}
	}

	for (sectorNo = 0; sectorNo < numSectors; sectorNo++) {
		size_t bytes_read = fread(keyB[sectorNo], 1, 6, fkeys);
		if (bytes_read != 6) {
			PrintAndLog("File reading error (dumpkeys.bin).");
			fclose(fkeys);
			return 2;
		}
	}

	fclose(fkeys);

	if ((fdump = fopen("dumpdata.bin","rb")) == NULL) {
		PrintAndLog("Could not find file dumpdata.bin");
		return 1;
	}
	PrintAndLog("Restoring dumpdata.bin to card");

	for (sectorNo = 0; sectorNo < numSectors; sectorNo++) {
		for(blockNo = 0; blockNo < NumBlocksPerSector(sectorNo); blockNo++) {
			UsbCommand c = {CMD_MIFARE_WRITEBL, {FirstBlockOfSector(sectorNo) + blockNo, keyType, 0}};
			memcpy(c.d.asBytes, key, 6);

			size_t bytes_read = fread(bldata, 1, 16, fdump);
			if (bytes_read != 16) {
				PrintAndLog("File reading error (dumpdata.bin).");
				fclose(fdump);
				return 2;
			}

			if (blockNo == NumBlocksPerSector(sectorNo) - 1) {	// sector trailer
				bldata[0]  = (keyA[sectorNo][0]);
				bldata[1]  = (keyA[sectorNo][1]);
				bldata[2]  = (keyA[sectorNo][2]);
				bldata[3]  = (keyA[sectorNo][3]);
				bldata[4]  = (keyA[sectorNo][4]);
				bldata[5]  = (keyA[sectorNo][5]);
				bldata[10] = (keyB[sectorNo][0]);
				bldata[11] = (keyB[sectorNo][1]);
				bldata[12] = (keyB[sectorNo][2]);
				bldata[13] = (keyB[sectorNo][3]);
				bldata[14] = (keyB[sectorNo][4]);
				bldata[15] = (keyB[sectorNo][5]);
			}

			PrintAndLog("Writing to block %3d: %s", FirstBlockOfSector(sectorNo) + blockNo, sprint_hex(bldata, 16));

			memcpy(c.d.asBytes + 10, bldata, 16);
			SendCommand(&c);

			UsbCommand resp;
			if (WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
				uint8_t isOK  = resp.arg[0] & 0xff;
				PrintAndLog("isOk:%02x", isOK);
			} else {
				PrintAndLog("Command execute timeout");
			}
		}
	}

	fclose(fdump);
	return 0;
}

//----------------------------------------------
//   Nested
//----------------------------------------------
# define NESTED_KEY_COUNT 15

static void parseParamTDS(const char *Cmd, const uint8_t indx, bool *paramT, bool *paramD, uint8_t *timeout) {
	char ctmp3[3] = {0};
	int len = param_getlength(Cmd, indx);
	if (len > 0 && len < 4){
		param_getstr(Cmd, indx, ctmp3);
		
		*paramT |= (ctmp3[0] == 't' || ctmp3[0] == 'T');
		*paramD |= (ctmp3[0] == 'd' || ctmp3[0] == 'D');
		bool paramS1 = *paramT || *paramD;

		// slow and very slow
		if (ctmp3[0] == 's' || ctmp3[0] == 'S' || ctmp3[1] == 's' || ctmp3[1] == 'S') {
			*timeout = 11; // slow
		
			if (!paramS1 && (ctmp3[1] == 's' || ctmp3[1] == 'S')) {
				*timeout = 53; // very slow
			}
			if (paramS1 && (ctmp3[2] == 's' || ctmp3[2] == 'S')) {
				*timeout = 53; // very slow
			}
		}
	}
}

int CmdHF14AMfNested(const char *Cmd)
{
	int i, j, res, iterations;
	sector_t *e_sector = NULL;
	uint8_t blockNo = 0;
	uint8_t keyType = 0;
	uint8_t trgBlockNo = 0;
	uint8_t trgKeyType = 0;
	uint8_t SectorsCnt = 0;
	uint8_t key[6] = {0, 0, 0, 0, 0, 0};
	uint8_t keyBlock[NESTED_KEY_COUNT * 6];
	uint64_t key64 = 0;
	// timeout in units. (ms * 106)/10 or us*0.0106
	uint8_t btimeout14a = MF_CHKKEYS_DEFTIMEOUT; // fast by default
	
	bool autosearchKey = false;

	bool transferToEml = false;
	bool createDumpFile = false;
	FILE *fkeys;
	uint8_t standart[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t tempkey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	char cmdp, ctmp;

	if (strlen(Cmd)<3) {
		PrintAndLog("Usage:");
		PrintAndLog(" all sectors:  hf mf nested  <card memory> <block number> <key A/B> <key (12 hex symbols)> [t|d|s|ss]");
		PrintAndLog(" all sectors autosearch key:  hf mf nested  <card memory> * [t|d|s|ss]");
		PrintAndLog(" one sector:   hf mf nested  o <block number> <key A/B> <key (12 hex symbols)>");
		PrintAndLog("               <target block number> <target key A/B> [t]");
		PrintAndLog(" ");
		PrintAndLog("card memory - 0 - MINI(320 bytes), 1 - 1K, 2 - 2K, 4 - 4K, <other> - 1K");
		PrintAndLog("t - transfer keys to emulator memory");
		PrintAndLog("d - write keys to binary file dumpkeys.bin");
		PrintAndLog("s - Slow (1ms) check keys (required by some non standard cards)");
		PrintAndLog("ss - Very slow (5ms) check keys");
		PrintAndLog(" ");
		PrintAndLog("      sample1: hf mf nested 1 0 A FFFFFFFFFFFF ");
		PrintAndLog("      sample2: hf mf nested 1 0 A FFFFFFFFFFFF t ");
		PrintAndLog("      sample3: hf mf nested 1 0 A FFFFFFFFFFFF d ");
		PrintAndLog("      sample4: hf mf nested o 0 A FFFFFFFFFFFF 4 A");
		PrintAndLog("      sample5: hf mf nested 1 * t");
		PrintAndLog("      sample6: hf mf nested 1 * ss");
		return 0;
	}

	// <card memory>
	cmdp = param_getchar(Cmd, 0);
	if (cmdp == 'o' || cmdp == 'O') {
		cmdp = 'o';
		SectorsCnt = 1;
	} else {
		SectorsCnt = ParamCardSizeSectors(cmdp);
	}
		
	// <block number>. number or autosearch key (*)
	if (param_getchar(Cmd, 1) == '*') {
		autosearchKey = true;

		parseParamTDS(Cmd, 2, &transferToEml, &createDumpFile, &btimeout14a);

		PrintAndLog("--nested. sectors:%2d, block no:*, eml:%c, dmp=%c checktimeout=%d us", 
			SectorsCnt, transferToEml?'y':'n', createDumpFile?'y':'n', ((int)btimeout14a * 10000) / 106);
	} else {
		blockNo = param_get8(Cmd, 1);

		ctmp = param_getchar(Cmd, 2);
		if (ctmp != 'a' && ctmp != 'A' && ctmp != 'b' && ctmp != 'B') {
			PrintAndLog("Key type must be A or B");
			return 1;
		}

		if (ctmp != 'A' && ctmp != 'a')
			keyType = 1;

		if (param_gethex(Cmd, 3, key, 12)) {
			PrintAndLog("Key must include 12 HEX symbols");
			return 1;
		}

		// check if we can authenticate to sector
		res = mfCheckKeys(blockNo, keyType, true, 1, key, &key64);
		if (res) {
			PrintAndLog("Can't authenticate to block:%3d key type:%c key:%s", blockNo, keyType?'B':'A', sprint_hex(key, 6));
			return 3;
		}

		// one sector nested
		if (cmdp == 'o') { 
			trgBlockNo = param_get8(Cmd, 4);

			ctmp = param_getchar(Cmd, 5);
			if (ctmp != 'a' && ctmp != 'A' && ctmp != 'b' && ctmp != 'B') {
				PrintAndLog("Target key type must be A or B");
				return 1;
			}
			if (ctmp != 'A' && ctmp != 'a')
				trgKeyType = 1;

			parseParamTDS(Cmd, 6, &transferToEml, &createDumpFile, &btimeout14a);
		} else {
			parseParamTDS(Cmd, 4, &transferToEml, &createDumpFile, &btimeout14a);
		}

		PrintAndLog("--nested. sectors:%2d, block no:%3d, key type:%c, eml:%c, dmp=%c checktimeout=%d us", 
			SectorsCnt, blockNo, keyType?'B':'A', transferToEml?'y':'n', createDumpFile?'y':'n', ((int)btimeout14a * 10000) / 106);
	}

	// one-sector nested
	if (cmdp == 'o') { // ------------------------------------  one sector working
		PrintAndLog("--target block no:%3d, target key type:%c ", trgBlockNo, trgKeyType?'B':'A');
		int16_t isOK = mfnested(blockNo, keyType, key, trgBlockNo, trgKeyType, keyBlock, true);
		if (isOK) {
			switch (isOK) {
				case -1 : PrintAndLog("Error: No response from Proxmark.\n"); break;
				case -2 : PrintAndLog("Button pressed. Aborted.\n"); break;
				case -3 : PrintAndLog("Tag isn't vulnerable to Nested Attack (random numbers are not predictable).\n"); break;
				default : PrintAndLog("Unknown Error.\n");
			}
			return 2;
		}
		key64 = bytes_to_num(keyBlock, 6);
		if (key64) {
			PrintAndLog("Found valid key:%012" PRIx64, key64);

			// transfer key to the emulator
			if (transferToEml) {
				uint8_t sectortrailer;
				if (trgBlockNo < 32*4) { 	// 4 block sector
					sectortrailer = (trgBlockNo & 0x03) + 3;
				} else {					// 16 block sector
					sectortrailer = (trgBlockNo & 0x0f) + 15;
				}
				mfEmlGetMem(keyBlock, sectortrailer, 1);

				if (!trgKeyType)
					num_to_bytes(key64, 6, keyBlock);
				else
					num_to_bytes(key64, 6, &keyBlock[10]);
				mfEmlSetMem(keyBlock, sectortrailer, 1);
				PrintAndLog("Key transferred to emulator memory.");
			}
		} else {
			PrintAndLog("No valid key found");
		}
	}
	else { // ------------------------------------  multiple sectors working
		uint64_t msclock1;
		msclock1 = msclock();

		e_sector = calloc(SectorsCnt, sizeof(sector_t));
		if (e_sector == NULL) return 1;

		//test current key and additional standard keys first
		for (int defaultKeyCounter = 0; defaultKeyCounter < MifareDefaultKeysSize; defaultKeyCounter++){
			num_to_bytes(MifareDefaultKeys[defaultKeyCounter], 6, (uint8_t*)(keyBlock + defaultKeyCounter * 6));
		}

		PrintAndLog("Testing known keys. Sector count=%d", SectorsCnt);
		mfCheckKeysSec(SectorsCnt, 2, btimeout14a, true, NESTED_KEY_COUNT, keyBlock, e_sector);
		
		// get known key from array
		bool keyFound = false;
		if (autosearchKey) {
			for (i = 0; i < SectorsCnt; i++) {
				for (j = 0; j < 2; j++) {
					if (e_sector[i].foundKey[j]) {
						// get known key
						blockNo = i * 4;
						keyType = j;
						num_to_bytes(e_sector[i].Key[j], 6, key);
						
						keyFound = true;
						break;
					}
				}
				if (keyFound) break;
			}		

			// Can't found a key....
			if (!keyFound) {
				PrintAndLog("Can't found any of the known keys.");
				return 4;
			}
			PrintAndLog("--auto key. block no:%3d, key type:%c key:%s", blockNo, keyType?'B':'A', sprint_hex(key, 6));
		}

		// nested sectors
		iterations = 0;
		PrintAndLog("nested...");
		bool calibrate = true;
		for (i = 0; i < NESTED_SECTOR_RETRY; i++) {
			for (uint8_t sectorNo = 0; sectorNo < SectorsCnt; sectorNo++) {
				for (trgKeyType = 0; trgKeyType < 2; trgKeyType++) {
					if (e_sector[sectorNo].foundKey[trgKeyType]) continue;
					PrintAndLog("-----------------------------------------------");
					int16_t isOK = mfnested(blockNo, keyType, key, FirstBlockOfSector(sectorNo), trgKeyType, keyBlock, calibrate);
					if(isOK) {
						switch (isOK) {
							case -1 : PrintAndLog("Error: No response from Proxmark.\n"); break;
							case -2 : PrintAndLog("Button pressed. Aborted.\n"); break;
							case -3 : PrintAndLog("Tag isn't vulnerable to Nested Attack (random numbers are not predictable).\n"); break;
							default : PrintAndLog("Unknown Error.\n");
						}
						free(e_sector);
						return 2;
					} else {
						calibrate = false;
					}

					iterations++;

					key64 = bytes_to_num(keyBlock, 6);
					if (key64) {
						PrintAndLog("Found valid key:%012" PRIx64, key64);
						e_sector[sectorNo].foundKey[trgKeyType] = 1;
						e_sector[sectorNo].Key[trgKeyType] = key64;
						
						// try to check this key as a key to the other sectors
						mfCheckKeysSec(SectorsCnt, 2, btimeout14a, true, 1, keyBlock, e_sector);
					}
				}
			}
		}

		// print nested statistic
		PrintAndLog("\n\n-----------------------------------------------\nNested statistic:\nIterations count: %d", iterations);
		PrintAndLog("Time in nested: %1.3f (%1.3f sec per key)", ((float)(msclock() - msclock1))/1000.0, ((float)(msclock() - msclock1))/iterations/1000.0);
		
		// print result
		PrintAndLog("|---|----------------|---|----------------|---|");
		PrintAndLog("|sec|key A           |res|key B           |res|");
		PrintAndLog("|---|----------------|---|----------------|---|");
		for (i = 0; i < SectorsCnt; i++) {
			PrintAndLog("|%03d|  %012" PRIx64 "  | %d |  %012" PRIx64 "  | %d |", i,
				e_sector[i].Key[0], e_sector[i].foundKey[0], e_sector[i].Key[1], e_sector[i].foundKey[1]);
		}
		PrintAndLog("|---|----------------|---|----------------|---|");

		// transfer keys to the emulator memory
		if (transferToEml) {
			for (i = 0; i < SectorsCnt; i++) {
				mfEmlGetMem(keyBlock, FirstBlockOfSector(i) + NumBlocksPerSector(i) - 1, 1);
				if (e_sector[i].foundKey[0])
					num_to_bytes(e_sector[i].Key[0], 6, keyBlock);
				if (e_sector[i].foundKey[1])
					num_to_bytes(e_sector[i].Key[1], 6, &keyBlock[10]);
				mfEmlSetMem(keyBlock, FirstBlockOfSector(i) + NumBlocksPerSector(i) - 1, 1);
			}
			PrintAndLog("Keys transferred to emulator memory.");
		}

		// Create dump file
		if (createDumpFile) {
			if ((fkeys = fopen("dumpkeys.bin","wb")) == NULL) {
				PrintAndLog("Could not create file dumpkeys.bin");
				free(e_sector);
				return 1;
			}
			PrintAndLog("Printing keys to binary file dumpkeys.bin...");
			for(i=0; i<SectorsCnt; i++) {
				if (e_sector[i].foundKey[0]){
					num_to_bytes(e_sector[i].Key[0], 6, tempkey);
					fwrite ( tempkey, 1, 6, fkeys );
				}
				else{
					fwrite ( &standart, 1, 6, fkeys );
				}
			}
			for(i=0; i<SectorsCnt; i++) {
				if (e_sector[i].foundKey[1]){
					num_to_bytes(e_sector[i].Key[1], 6, tempkey);
					fwrite ( tempkey, 1, 6, fkeys );
				}
				else{
					fwrite ( &standart, 1, 6, fkeys );
				}
			}
			fclose(fkeys);
		}

		free(e_sector);
	}
	return 0;
}


int CmdHF14AMfNestedHard(const char *Cmd)
{
	uint8_t blockNo = 0;
	uint8_t keyType = 0;
	uint8_t trgBlockNo = 0;
	uint8_t trgKeyType = 0;
	uint8_t key[6] = {0, 0, 0, 0, 0, 0};
	uint8_t trgkey[6] = {0, 0, 0, 0, 0, 0};

	char ctmp;
	ctmp = param_getchar(Cmd, 0);

	if (ctmp != 'R' && ctmp != 'r' && ctmp != 'T' && ctmp != 't' && strlen(Cmd) < 20) {
		PrintAndLog("Usage:");
		PrintAndLog("      hf mf hardnested <block number> <key A|B> <key (12 hex symbols)>");
		PrintAndLog("                       <target block number> <target key A|B> [known target key (12 hex symbols)] [w] [s]");
		PrintAndLog("  or  hf mf hardnested r [known target key]");
		PrintAndLog(" ");
		PrintAndLog("Options: ");
		PrintAndLog("      w: Acquire nonces and write them to binary file nonces.bin");
		PrintAndLog("      s: Slower acquisition (required by some non standard cards)");
		PrintAndLog("      r: Read nonces.bin and start attack");
		PrintAndLog(" ");
		PrintAndLog("      sample1: hf mf hardnested 0 A FFFFFFFFFFFF 4 A");
		PrintAndLog("      sample2: hf mf hardnested 0 A FFFFFFFFFFFF 4 A w");
		PrintAndLog("      sample3: hf mf hardnested 0 A FFFFFFFFFFFF 4 A w s");
		PrintAndLog("      sample4: hf mf hardnested r");
		PrintAndLog(" ");
		PrintAndLog("Add the known target key to check if it is present in the remaining key space:");
		PrintAndLog("      sample5: hf mf hardnested 0 A A0A1A2A3A4A5 4 A FFFFFFFFFFFF");
		return 0;
	}

	bool know_target_key = false;
	bool nonce_file_read = false;
	bool nonce_file_write = false;
	bool slow = false;
	int tests = 0;


	if (ctmp == 'R' || ctmp == 'r') {
		nonce_file_read = true;
		if (!param_gethex(Cmd, 1, trgkey, 12)) {
			know_target_key = true;
		}
	} else if (ctmp == 'T' || ctmp == 't') {
		tests = param_get32ex(Cmd, 1, 100, 10);
		if (!param_gethex(Cmd, 2, trgkey, 12)) {
			know_target_key = true;
		}
	} else {
		blockNo = param_get8(Cmd, 0);
		ctmp = param_getchar(Cmd, 1);
		if (ctmp != 'a' && ctmp != 'A' && ctmp != 'b' && ctmp != 'B') {
			PrintAndLog("Key type must be A or B");
			return 1;
		}
		if (ctmp != 'A' && ctmp != 'a') {
			keyType = 1;
		}

		if (param_gethex(Cmd, 2, key, 12)) {
			PrintAndLog("Key must include 12 HEX symbols");
			return 1;
		}

		trgBlockNo = param_get8(Cmd, 3);
		ctmp = param_getchar(Cmd, 4);
		if (ctmp != 'a' && ctmp != 'A' && ctmp != 'b' && ctmp != 'B') {
			PrintAndLog("Target key type must be A or B");
			return 1;
		}
		if (ctmp != 'A' && ctmp != 'a') {
			trgKeyType = 1;
		}

		uint16_t i = 5;

		if (!param_gethex(Cmd, 5, trgkey, 12)) {
			know_target_key = true;
			i++;
		}

		while ((ctmp = param_getchar(Cmd, i))) {
			if (ctmp == 's' || ctmp == 'S') {
				slow = true;
			} else if (ctmp == 'w' || ctmp == 'W') {
				nonce_file_write = true;
			} else {
				PrintAndLog("Possible options are w and/or s");
				return 1;
			}
			i++;
		}
	}

	PrintAndLog("--target block no:%3d, target key type:%c, known target key: 0x%02x%02x%02x%02x%02x%02x%s, file action: %s, Slow: %s, Tests: %d ",
			trgBlockNo,
			trgKeyType?'B':'A',
			trgkey[0], trgkey[1], trgkey[2], trgkey[3], trgkey[4], trgkey[5],
			know_target_key?"":" (not set)",
			nonce_file_write?"write":nonce_file_read?"read":"none",
			slow?"Yes":"No",
			tests);

	int16_t isOK = mfnestedhard(blockNo, keyType, key, trgBlockNo, trgKeyType, know_target_key?trgkey:NULL, nonce_file_read, nonce_file_write, slow, tests);

	if (isOK) {
		switch (isOK) {
			case 1 : PrintAndLog("Error: No response from Proxmark.\n"); break;
			case 2 : PrintAndLog("Button pressed. Aborted.\n"); break;
			default : break;
		}
		return 2;
	}

	return 0;
}


int CmdHF14AMfChk(const char *Cmd)
{
	if (strlen(Cmd)<3) {
		PrintAndLog("Usage:  hf mf chk <block number>|<*card memory> <key type (A/B/?)> [t|d|s|ss] [<key (12 hex symbols)>] [<dic (*.dic)>]");
		PrintAndLog("          * - all sectors");
		PrintAndLog("card memory - 0 - MINI(320 bytes), 1 - 1K, 2 - 2K, 4 - 4K, <other> - 1K");
		PrintAndLog("d - write keys to binary file\n");
		PrintAndLog("t - write keys to emulator memory");
		PrintAndLog("s - slow execute. timeout 1ms");
		PrintAndLog("ss- very slow execute. timeout 5ms");
		PrintAndLog("      sample: hf mf chk 0 A 1234567890ab keys.dic");
		PrintAndLog("              hf mf chk *1 ? t");
		PrintAndLog("              hf mf chk *1 ? d");
		PrintAndLog("              hf mf chk *1 ? s");
		PrintAndLog("              hf mf chk *1 ? dss");
		return 0;
	}

	FILE * f;
	char filename[FILE_PATH_SIZE]={0};
	char buf[13];
	uint8_t *keyBlock = NULL, *p;
	uint16_t stKeyBlock = 20;

	int i, res;
	int	keycnt = 0;
	char ctmp	= 0x00;
	char ctmp3[3]	= {0x00};
	uint8_t blockNo = 0;
	uint8_t SectorsCnt = 0;
	uint8_t keyType = 0;
	uint64_t key64 = 0;
	uint32_t timeout14a = 0; // timeout in us
	bool param3InUse = false;

	int transferToEml = 0;
	int createDumpFile = 0;
	
	sector_t *e_sector = NULL;

	keyBlock = calloc(stKeyBlock, 6);
	if (keyBlock == NULL) return 1;

	int defaultKeysSize = MifareDefaultKeysSize;
	for (int defaultKeyCounter = 0; defaultKeyCounter < defaultKeysSize; defaultKeyCounter++){
		num_to_bytes(MifareDefaultKeys[defaultKeyCounter], 6, (uint8_t*)(keyBlock + defaultKeyCounter * 6));
	}

	if (param_getchar(Cmd, 0)=='*') {
		SectorsCnt = ParamCardSizeSectors(param_getchar(Cmd + 1, 0));
	}
	else
		blockNo = param_get8(Cmd, 0);

	ctmp = param_getchar(Cmd, 1);
	switch (ctmp) {
	case 'a': case 'A':
		keyType = 0;
		break;
	case 'b': case 'B':
		keyType = 1;
		break;
	case '?':
		keyType = 2;
		break;
	default:
		PrintAndLog("Key type must be A , B or ?");
		free(keyBlock);
		return 1;
	};

	// transfer to emulator & create dump file
	ctmp = param_getchar(Cmd, 2);
	if (ctmp == 't' || ctmp == 'T') transferToEml = 1;
	if (ctmp == 'd' || ctmp == 'D') createDumpFile = 1;
	
	param3InUse = transferToEml | createDumpFile;
	
	timeout14a = 500; // fast by default
	// double parameters - ts, ds
	int clen = param_getlength(Cmd, 2);
	if (clen == 2 || clen == 3){
		param_getstr(Cmd, 2, ctmp3);
		ctmp = ctmp3[1];
	}
	//parse
	if (ctmp == 's' || ctmp == 'S') {
		timeout14a = 1000; // slow
		if (!param3InUse && clen == 2 && (ctmp3[1] == 's' || ctmp3[1] == 'S')) {
			timeout14a = 5000; // very slow
		}
		if (param3InUse && clen == 3 && (ctmp3[2] == 's' || ctmp3[2] == 'S')) {
			timeout14a = 5000; // very slow
		}
		param3InUse = true;
	}

	for (i = param3InUse; param_getchar(Cmd, 2 + i); i++) {
		if (!param_gethex(Cmd, 2 + i, keyBlock + 6 * keycnt, 12)) {
			if ( stKeyBlock - keycnt < 2) {
				p = realloc(keyBlock, 6*(stKeyBlock+=10));
				if (!p) {
					PrintAndLog("Cannot allocate memory for Keys");
					free(keyBlock);
					return 2;
				}
				keyBlock = p;
			}
			PrintAndLog("chk key[%2d] %02x%02x%02x%02x%02x%02x", keycnt,
			(keyBlock + 6*keycnt)[0],(keyBlock + 6*keycnt)[1], (keyBlock + 6*keycnt)[2],
			(keyBlock + 6*keycnt)[3], (keyBlock + 6*keycnt)[4],	(keyBlock + 6*keycnt)[5], 6);
			keycnt++;
		} else {
			// May be a dic file
			if ( param_getstr(Cmd, 2 + i,filename) >= FILE_PATH_SIZE ) {
				PrintAndLog("File name too long");
				free(keyBlock);
				return 2;
			}

			if ( (f = fopen( filename , "r")) ) {
				while( fgets(buf, sizeof(buf), f) ){
					if (strlen(buf) < 12 || buf[11] == '\n')
						continue;

					while (fgetc(f) != '\n' && !feof(f)) ;  //goto next line

					if( buf[0]=='#' ) continue;	//The line start with # is comment, skip

					if (!isxdigit(buf[0])){
						PrintAndLog("File content error. '%s' must include 12 HEX symbols",buf);
						continue;
					}

					buf[12] = 0;

					if ( stKeyBlock - keycnt < 2) {
						p = realloc(keyBlock, 6*(stKeyBlock+=10));
						if (!p) {
							PrintAndLog("Cannot allocate memory for defKeys");
							free(keyBlock);
							fclose(f);
							return 2;
						}
						keyBlock = p;
					}
					memset(keyBlock + 6 * keycnt, 0, 6);
					num_to_bytes(strtoll(buf, NULL, 16), 6, keyBlock + 6*keycnt);
					PrintAndLog("chk custom key[%2d] %012" PRIx64 , keycnt, bytes_to_num(keyBlock + 6*keycnt, 6));
					keycnt++;
					memset(buf, 0, sizeof(buf));
				}
				fclose(f);
			} else {
				PrintAndLog("File: %s: not found or locked.", filename);
				free(keyBlock);
				return 1;

			}
		}
	}

	// fill with default keys
	if (keycnt == 0) {
		PrintAndLog("No key specified, trying default keys");
		for (;keycnt < defaultKeysSize; keycnt++)
			PrintAndLog("chk default key[%2d] %02x%02x%02x%02x%02x%02x", keycnt,
				(keyBlock + 6*keycnt)[0],(keyBlock + 6*keycnt)[1], (keyBlock + 6*keycnt)[2],
				(keyBlock + 6*keycnt)[3], (keyBlock + 6*keycnt)[4],	(keyBlock + 6*keycnt)[5], 6);
	}

	// initialize storage for found keys
	e_sector = calloc(SectorsCnt, sizeof(sector_t));
	if (e_sector == NULL) return 1;
	for (uint8_t keyAB = 0; keyAB < 2; keyAB++) {
		for (uint16_t sectorNo = 0; sectorNo < SectorsCnt; sectorNo++) {
			e_sector[sectorNo].Key[keyAB] = 0xffffffffffff;
			e_sector[sectorNo].foundKey[keyAB] = 0;
		}
	}
	printf("\n");

	bool foundAKey = false;
	uint32_t max_keys = keycnt > USB_CMD_DATA_SIZE / 6 ? USB_CMD_DATA_SIZE / 6 : keycnt;
	if (SectorsCnt) {
		PrintAndLog("To cancel this operation press the button on the proxmark...");
		printf("--");
		for (uint32_t c = 0; c < keycnt; c += max_keys) {

			uint32_t size = keycnt-c > max_keys ? max_keys : keycnt-c;
			res = mfCheckKeysSec(SectorsCnt, keyType, timeout14a * 1.06 / 100, true, size, &keyBlock[6 * c], e_sector); // timeout is (ms * 106)/10 or us*0.0106

			if (res != 1) {
				if (!res) {
					printf("o");
					foundAKey = true;
				} else {
					printf(".");
				}
			} else {
				printf("\n");
				PrintAndLog("Command execute timeout");
			}
		}
	} else {
		int keyAB = keyType;
		do {
			for (uint32_t c = 0; c < keycnt; c+=max_keys) {

				uint32_t size = keycnt-c > max_keys ? max_keys : keycnt-c;
				res = mfCheckKeys(blockNo, keyAB & 0x01, true, size, &keyBlock[6 * c], &key64); 

				if (res != 1) {
					if (!res) {
						PrintAndLog("Found valid key:[%d:%c]%012" PRIx64, blockNo, (keyAB & 0x01)?'B':'A', key64);
						foundAKey = true;
					}
				} else {
					PrintAndLog("Command execute timeout");
				}
			}
		} while(--keyAB > 0);
	}
	
	// print result
	if (foundAKey) {
		if (SectorsCnt) {
			PrintAndLog("");
			PrintAndLog("|---|----------------|---|----------------|---|");
			PrintAndLog("|sec|key A           |res|key B           |res|");
			PrintAndLog("|---|----------------|---|----------------|---|");
			for (i = 0; i < SectorsCnt; i++) {
				PrintAndLog("|%03d|  %012" PRIx64 "  | %d |  %012" PRIx64 "  | %d |", i,
					e_sector[i].Key[0], e_sector[i].foundKey[0], e_sector[i].Key[1], e_sector[i].foundKey[1]);
			}
			PrintAndLog("|---|----------------|---|----------------|---|");
		}
	} else {
		PrintAndLog("");
		PrintAndLog("No valid keys found.");
	}	
	
	if (transferToEml) {
		uint8_t block[16];
		for (uint16_t sectorNo = 0; sectorNo < SectorsCnt; sectorNo++) {
			if (e_sector[sectorNo].foundKey[0] || e_sector[sectorNo].foundKey[1]) {
				mfEmlGetMem(block, FirstBlockOfSector(sectorNo) + NumBlocksPerSector(sectorNo) - 1, 1);
				for (uint16_t t = 0; t < 2; t++) {
					if (e_sector[sectorNo].foundKey[t]) {
						num_to_bytes(e_sector[sectorNo].Key[t], 6, block + t * 10);
					}
				}
				mfEmlSetMem(block, FirstBlockOfSector(sectorNo) + NumBlocksPerSector(sectorNo) - 1, 1);
			}
		}
		PrintAndLog("Found keys have been transferred to the emulator memory");
	}

	if (createDumpFile) {
		FILE *fkeys = fopen("dumpkeys.bin","wb");
		if (fkeys == NULL) {
			PrintAndLog("Could not create file dumpkeys.bin");
			free(e_sector);
			free(keyBlock);
			return 1;
		}
		uint8_t mkey[6];
		for (uint8_t t = 0; t < 2; t++) {
			for (uint8_t sectorNo = 0; sectorNo < SectorsCnt; sectorNo++) {
				num_to_bytes(e_sector[sectorNo].Key[t], 6, mkey);
				fwrite(mkey, 1, 6, fkeys);
			}
		}
		fclose(fkeys);
		PrintAndLog("Found keys have been dumped to file dumpkeys.bin. 0xffffffffffff has been inserted for unknown keys.");
	}

	free(e_sector);
	free(keyBlock);
	PrintAndLog("");
	return 0;
}

void readerAttack(nonces_t ar_resp[], bool setEmulatorMem, bool doStandardAttack) {
	#define ATTACK_KEY_COUNT 7 // keep same as define in iso14443a.c -> Mifare1ksim()
	                           // cannot be more than 7 or it will overrun c.d.asBytes(512)
	uint64_t key = 0;
	typedef struct {
			uint64_t keyA;
			uint64_t keyB;
	} st_t;
	st_t sector_trailer[ATTACK_KEY_COUNT];
	memset(sector_trailer, 0x00, sizeof(sector_trailer));

	uint8_t	stSector[ATTACK_KEY_COUNT];
	memset(stSector, 0x00, sizeof(stSector));
	uint8_t key_cnt[ATTACK_KEY_COUNT];
	memset(key_cnt, 0x00, sizeof(key_cnt));

	for (uint8_t i = 0; i<ATTACK_KEY_COUNT; i++) {
		if (ar_resp[i].ar2 > 0) {
			//PrintAndLog("DEBUG: Trying sector %d, cuid %08x, nt %08x, ar %08x, nr %08x, ar2 %08x, nr2 %08x",ar_resp[i].sector, ar_resp[i].cuid,ar_resp[i].nonce,ar_resp[i].ar,ar_resp[i].nr,ar_resp[i].ar2,ar_resp[i].nr2);
			if (doStandardAttack && mfkey32(ar_resp[i], &key)) {
				PrintAndLog("  Found Key%s for sector %02d: [%04x%08x]", (ar_resp[i].keytype) ? "B" : "A", ar_resp[i].sector, (uint32_t) (key>>32), (uint32_t) (key &0xFFFFFFFF));

				for (uint8_t ii = 0; ii<ATTACK_KEY_COUNT; ii++) {
					if (key_cnt[ii]==0 || stSector[ii]==ar_resp[i].sector) {
						if (ar_resp[i].keytype==0) {
							//keyA
							sector_trailer[ii].keyA = key;
							stSector[ii] = ar_resp[i].sector;
							key_cnt[ii]++;
							break;
						} else {
							//keyB
							sector_trailer[ii].keyB = key;
							stSector[ii] = ar_resp[i].sector;
							key_cnt[ii]++;
							break;
						}
					}
				}
			} else if (mfkey32_moebius(ar_resp[i+ATTACK_KEY_COUNT], &key)) {
				uint8_t sectorNum = ar_resp[i+ATTACK_KEY_COUNT].sector;
				uint8_t keyType = ar_resp[i+ATTACK_KEY_COUNT].keytype;

				PrintAndLog("M-Found Key%s for sector %02d: [%012" PRIx64 "]"
					, keyType ? "B" : "A"
					, sectorNum
					, key
				);

				for (uint8_t ii = 0; ii<ATTACK_KEY_COUNT; ii++) {
					if (key_cnt[ii]==0 || stSector[ii]==sectorNum) {
						if (keyType==0) {
							//keyA
							sector_trailer[ii].keyA = key;
							stSector[ii] = sectorNum;
							key_cnt[ii]++;
							break;
						} else {
							//keyB
							sector_trailer[ii].keyB = key;
							stSector[ii] = sectorNum;
							key_cnt[ii]++;
							break;
						}
					}
				}
				continue;
			}
		}
	}
	//set emulator memory for keys
	if (setEmulatorMem) {
		for (uint8_t i = 0; i<ATTACK_KEY_COUNT; i++) {
			if (key_cnt[i]>0) {
				uint8_t	memBlock[16];
				memset(memBlock, 0x00, sizeof(memBlock));
				char cmd1[36];
				memset(cmd1,0x00,sizeof(cmd1));
				snprintf(cmd1,sizeof(cmd1),"%04x%08xFF078069%04x%08x",(uint32_t) (sector_trailer[i].keyA>>32), (uint32_t) (sector_trailer[i].keyA &0xFFFFFFFF),(uint32_t) (sector_trailer[i].keyB>>32), (uint32_t) (sector_trailer[i].keyB &0xFFFFFFFF));
				PrintAndLog("Setting Emulator Memory Block %02d: [%s]",stSector[i]*4+3, cmd1);
				if (param_gethex(cmd1, 0, memBlock, 32)) {
					PrintAndLog("block data must include 32 HEX symbols");
					return;
				}

				UsbCommand c = {CMD_MIFARE_EML_MEMSET, {(stSector[i]*4+3), 1, 0}};
				memcpy(c.d.asBytes, memBlock, 16);
				clearCommandBuffer();
				SendCommand(&c);
			}
		}
	}
	/*
	//un-comment to use as well moebius attack
	for (uint8_t i = ATTACK_KEY_COUNT; i<ATTACK_KEY_COUNT*2; i++) {
		if (ar_resp[i].ar2 > 0) {
			if (tryMfk32_moebius(ar_resp[i], &key)) {
				PrintAndLog("M-Found Key%s for sector %02d: [%04x%08x]", (ar_resp[i].keytype) ? "B" : "A", ar_resp[i].sector, (uint32_t) (key>>32), (uint32_t) (key &0xFFFFFFFF));
			}
		}
	}*/
}

int usage_hf14_mf1ksim(void) {
	PrintAndLog("Usage:  hf mf sim h u <uid (8, 14, or 20 hex symbols)> n <numreads> i x");
	PrintAndLog("options:");
	PrintAndLog("      h    this help");
	PrintAndLog("      u    (Optional) UID 4,7 or 10 bytes. If not specified, the UID 4B from emulator memory will be used");
	PrintAndLog("      n    (Optional) Automatically exit simulation after <numreads> blocks have been read by reader. 0 = infinite");
	PrintAndLog("      i    (Optional) Interactive, means that console will not be returned until simulation finishes or is aborted");
	PrintAndLog("      x    (Optional) Crack, performs the 'reader attack', nr/ar attack against a legitimate reader, fishes out the key(s)");
	PrintAndLog("      e    (Optional) set keys found from 'reader attack' to emulator memory (implies x and i)");
	PrintAndLog("      f    (Optional) get UIDs to use for 'reader attack' from file 'f <filename.txt>' (implies x and i)");
	PrintAndLog("      r    (Optional) Generate random nonces instead of sequential nonces. Standard reader attack won't work with this option, only moebius attack works.");
	PrintAndLog("samples:");
	PrintAndLog("           hf mf sim u 0a0a0a0a");
	PrintAndLog("           hf mf sim u 11223344556677");
	PrintAndLog("           hf mf sim u 112233445566778899AA");
	PrintAndLog("           hf mf sim f uids.txt");
	PrintAndLog("           hf mf sim u 0a0a0a0a e");

	return 0;
}

int CmdHF14AMf1kSim(const char *Cmd) {
	UsbCommand resp;
	uint8_t uid[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t exitAfterNReads = 0;
	uint8_t flags = 0;
	int uidlen = 0;
	uint8_t pnr = 0;
	bool setEmulatorMem = false;
	bool attackFromFile = false;
	FILE *f;
	char filename[FILE_PATH_SIZE];
	memset(filename, 0x00, sizeof(filename));
	int len = 0;
	char buf[64];

	uint8_t cmdp = 0;
	bool errors = false;

	while(param_getchar(Cmd, cmdp) != 0x00) {
		switch(param_getchar(Cmd, cmdp)) {
		case 'e':
		case 'E':
			setEmulatorMem = true;
			//implies x and i
			flags |= FLAG_INTERACTIVE;
			flags |= FLAG_NR_AR_ATTACK;
			cmdp++;
			break;
		case 'f':
		case 'F':
			len = param_getstr(Cmd, cmdp+1, filename);
			if (len < 1) {
				PrintAndLog("error no filename found");
				return 0;
			}
			attackFromFile = true;
			//implies x and i
			flags |= FLAG_INTERACTIVE;
			flags |= FLAG_NR_AR_ATTACK;
			cmdp += 2;
			break;
		case 'h':
		case 'H':
			return usage_hf14_mf1ksim();
		case 'i':
		case 'I':
			flags |= FLAG_INTERACTIVE;
			cmdp++;
			break;
		case 'n':
		case 'N':
			exitAfterNReads = param_get8(Cmd, pnr+1);
			cmdp += 2;
			break;
		case 'r':
		case 'R':
			flags |= FLAG_RANDOM_NONCE;
			cmdp++;
			break;
		case 'u':
		case 'U':
			param_gethex_ex(Cmd, cmdp+1, uid, &uidlen);
			switch(uidlen) {
				case 20: flags = FLAG_10B_UID_IN_DATA;	break; //not complete
				case 14: flags = FLAG_7B_UID_IN_DATA; break;
				case  8: flags = FLAG_4B_UID_IN_DATA; break;
				default: return usage_hf14_mf1ksim();
			}
			cmdp += 2;
			break;
		case 'x':
		case 'X':
			flags |= FLAG_NR_AR_ATTACK;
			cmdp++;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if(errors) break;
	}
	//Validations
	if(errors) return usage_hf14_mf1ksim();

	//get uid from file
	if (attackFromFile) {
		int count = 0;
		// open file
		f = fopen(filename, "r");
		if (f == NULL) {
			PrintAndLog("File %s not found or locked", filename);
			return 1;
		}
		PrintAndLog("Loading file and simulating. Press keyboard to abort");
		while(!feof(f) && !ukbhit()){
			memset(buf, 0, sizeof(buf));
			memset(uid, 0, sizeof(uid));

			if (fgets(buf, sizeof(buf), f) == NULL) {
				if (count > 0) break;

				PrintAndLog("File reading error.");
				fclose(f);
				return 2;
			}
			if(!strlen(buf) && feof(f)) break;

			uidlen = strlen(buf)-1;
			switch(uidlen) {
				case 20: flags |= FLAG_10B_UID_IN_DATA;	break; //not complete
				case 14: flags |= FLAG_7B_UID_IN_DATA; break;
				case  8: flags |= FLAG_4B_UID_IN_DATA; break;
				default:
					PrintAndLog("uid in file wrong length at %d (length: %d) [%s]",count, uidlen, buf);
					fclose(f);
					return 2;
			}

			for (uint8_t i = 0; i < uidlen; i += 2) {
				sscanf(&buf[i], "%02x", (unsigned int *)&uid[i / 2]);
			}

			PrintAndLog("mf 1k sim uid: %s, numreads:%d, flags:%d (0x%02x) - press button to abort",
					flags & FLAG_4B_UID_IN_DATA ? sprint_hex(uid,4):
						flags & FLAG_7B_UID_IN_DATA	? sprint_hex(uid,7):
							flags & FLAG_10B_UID_IN_DATA ? sprint_hex(uid,10): "N/A"
					, exitAfterNReads, flags, flags);

			UsbCommand c = {CMD_SIMULATE_MIFARE_CARD, {flags, exitAfterNReads,0}};
			memcpy(c.d.asBytes, uid, sizeof(uid));
			clearCommandBuffer();
			SendCommand(&c);

			while(! WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
				//We're waiting only 1.5 s at a time, otherwise we get the
				// annoying message about "Waiting for a response... "
			}
			//got a response
			nonces_t ar_resp[ATTACK_KEY_COUNT*2];
			memcpy(ar_resp, resp.d.asBytes, sizeof(ar_resp));
			// We can skip the standard attack if we have RANDOM_NONCE set.
			readerAttack(ar_resp, setEmulatorMem, !(flags & FLAG_RANDOM_NONCE));
			if ((bool)resp.arg[1]) {
				PrintAndLog("Device button pressed - quitting");
				fclose(f);
				return 4;
			}
			count++;
		}
		fclose(f);
	} else { //not from file

		PrintAndLog("mf 1k sim uid: %s, numreads:%d, flags:%d (0x%02x) ",
				flags & FLAG_4B_UID_IN_DATA ? sprint_hex(uid,4):
					flags & FLAG_7B_UID_IN_DATA	? sprint_hex(uid,7):
						flags & FLAG_10B_UID_IN_DATA ? sprint_hex(uid,10): "N/A"
				, exitAfterNReads, flags, flags);

		UsbCommand c = {CMD_SIMULATE_MIFARE_CARD, {flags, exitAfterNReads,0}};
		memcpy(c.d.asBytes, uid, sizeof(uid));
		clearCommandBuffer();
		SendCommand(&c);

		if(flags & FLAG_INTERACTIVE) {
			PrintAndLog("Press pm3-button to abort simulation");
			while(! WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
				//We're waiting only 1.5 s at a time, otherwise we get the
				// annoying message about "Waiting for a response... "
			}
			//got a response
			if (flags & FLAG_NR_AR_ATTACK) {
				nonces_t ar_resp[ATTACK_KEY_COUNT*2];
				memcpy(ar_resp, resp.d.asBytes, sizeof(ar_resp));
				// We can skip the standard attack if we have RANDOM_NONCE set.
				readerAttack(ar_resp, setEmulatorMem, !(flags & FLAG_RANDOM_NONCE));
			}
		}
	}

	return 0;
}

int CmdHF14AMfDbg(const char *Cmd)
{
	int dbgMode = param_get32ex(Cmd, 0, 0, 10);
	if (dbgMode > 4) {
		PrintAndLog("Max debug mode parameter is 4 \n");
	}

	if (strlen(Cmd) < 1 || !param_getchar(Cmd, 0) || dbgMode > 4) {
		PrintAndLog("Usage:  hf mf dbg  <debug level>");
		PrintAndLog(" 0 - no debug messages");
		PrintAndLog(" 1 - error messages");
		PrintAndLog(" 2 - plus information messages");
		PrintAndLog(" 3 - plus debug messages");
		PrintAndLog(" 4 - print even debug messages in timing critical functions");
		PrintAndLog("     Note: this option therefore may cause malfunction itself");
		return 0;
	}

  UsbCommand c = {CMD_MIFARE_SET_DBGMODE, {dbgMode, 0, 0}};
  SendCommand(&c);

  return 0;
}

int CmdHF14AMfEGet(const char *Cmd)
{
	uint8_t blockNo = 0;
	uint8_t data[16] = {0x00};

	if (strlen(Cmd) < 1 || param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("Usage:  hf mf eget <block number>");
		PrintAndLog(" sample: hf mf eget 0 ");
		return 0;
	}

	blockNo = param_get8(Cmd, 0);

	PrintAndLog(" ");
	if (!mfEmlGetMem(data, blockNo, 1)) {
		PrintAndLog("data[%3d]:%s", blockNo, sprint_hex(data, 16));
	} else {
		PrintAndLog("Command execute timeout");
	}

  return 0;
}

int CmdHF14AMfEClear(const char *Cmd)
{
	if (param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("Usage:  hf mf eclr");
		PrintAndLog("It set card emulator memory to empty data blocks and key A/B FFFFFFFFFFFF \n");
		return 0;
	}

  UsbCommand c = {CMD_MIFARE_EML_MEMCLR, {0, 0, 0}};
  SendCommand(&c);
  return 0;
}


int CmdHF14AMfESet(const char *Cmd)
{
	uint8_t memBlock[16];
	uint8_t blockNo = 0;

	memset(memBlock, 0x00, sizeof(memBlock));

	if (strlen(Cmd) < 3 || param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("Usage:  hf mf eset <block number> <block data (32 hex symbols)>");
		PrintAndLog(" sample: hf mf eset 1 000102030405060708090a0b0c0d0e0f ");
		return 0;
	}

	blockNo = param_get8(Cmd, 0);

	if (param_gethex(Cmd, 1, memBlock, 32)) {
		PrintAndLog("block data must include 32 HEX symbols");
		return 1;
	}

	//  1 - blocks count
	UsbCommand c = {CMD_MIFARE_EML_MEMSET, {blockNo, 1, 0}};
	memcpy(c.d.asBytes, memBlock, 16);
	SendCommand(&c);
	return 0;
}


int CmdHF14AMfELoad(const char *Cmd)
{
	FILE * f;
	char filename[FILE_PATH_SIZE];
	char *fnameptr = filename;
	char buf[64] = {0x00};
	uint8_t buf8[64] = {0x00};
	int i, len, blockNum, numBlocks;
	int nameParamNo = 1;

	char ctmp = param_getchar(Cmd, 0);

	if ( ctmp == 'h' || ctmp == 0x00) {
		PrintAndLog("It loads emul dump from the file `filename.eml`");
		PrintAndLog("Usage:  hf mf eload [card memory] <file name w/o `.eml`>");
		PrintAndLog("  [card memory]: 0 = 320 bytes (Mifare Mini), 1 = 1K (default), 2 = 2K, 4 = 4K");
		PrintAndLog("");
		PrintAndLog(" sample: hf mf eload filename");
		PrintAndLog("         hf mf eload 4 filename");
		return 0;
	}

	switch (ctmp) {
		case '0' : numBlocks = 5*4; break;
		case '1' :
		case '\0': numBlocks = 16*4; break;
		case '2' : numBlocks = 32*4; break;
		case '4' : numBlocks = 256; break;
		default:  {
			numBlocks = 16*4;
			nameParamNo = 0;
		}
	}

	len = param_getstr(Cmd,nameParamNo,filename);

	if (len > FILE_PATH_SIZE - 5) len = FILE_PATH_SIZE - 5;

	fnameptr += len;

	sprintf(fnameptr, ".eml");

	// open file
	f = fopen(filename, "r");
	if (f == NULL) {
		PrintAndLog("File %s not found or locked", filename);
		return 1;
	}

	blockNum = 0;
	while(!feof(f)){
		memset(buf, 0, sizeof(buf));

		if (fgets(buf, sizeof(buf), f) == NULL) {

			if (blockNum >= numBlocks) break;

			PrintAndLog("File reading error.");
			fclose(f);
			return 2;
		}

		if (strlen(buf) < 32){
			if(strlen(buf) && feof(f))
				break;
			PrintAndLog("File content error. Block data must include 32 HEX symbols");
			fclose(f);
			return 2;
		}

		for (i = 0; i < 32; i += 2) {
			sscanf(&buf[i], "%02x", (unsigned int *)&buf8[i / 2]);
		}

		if (mfEmlSetMem(buf8, blockNum, 1)) {
			PrintAndLog("Cant set emul block: %3d", blockNum);
			fclose(f);
			return 3;
		}
		printf(".");
		blockNum++;

		if (blockNum >= numBlocks) break;
	}
	fclose(f);
	printf("\n");

	if ((blockNum != numBlocks)) {
		PrintAndLog("File content error. Got %d must be %d blocks.",blockNum, numBlocks);
		return 4;
	}
	PrintAndLog("Loaded %d blocks from file: %s", blockNum, filename);
	return 0;
}


int CmdHF14AMfESave(const char *Cmd)
{
	FILE * f;
	char filename[FILE_PATH_SIZE];
	char * fnameptr = filename;
	uint8_t buf[64];
	int i, j, len, numBlocks;
	int nameParamNo = 1;

	memset(filename, 0, sizeof(filename));
	memset(buf, 0, sizeof(buf));

	char ctmp = param_getchar(Cmd, 0);

	if ( ctmp == 'h' || ctmp == 'H') {
		PrintAndLog("It saves emul dump into the file `filename.eml` or `cardID.eml`");
		PrintAndLog(" Usage:  hf mf esave [card memory] [file name w/o `.eml`]");
		PrintAndLog("  [card memory]: 0 = 320 bytes (Mifare Mini), 1 = 1K (default), 2 = 2K, 4 = 4K");
		PrintAndLog("");
		PrintAndLog(" sample: hf mf esave ");
		PrintAndLog("         hf mf esave 4");
		PrintAndLog("         hf mf esave 4 filename");
		return 0;
	}

	switch (ctmp) {
		case '0' : numBlocks = 5*4; break;
		case '1' :
		case '\0': numBlocks = 16*4; break;
		case '2' : numBlocks = 32*4; break;
		case '4' : numBlocks = 256; break;
		default:  {
			numBlocks = 16*4;
			nameParamNo = 0;
		}
	}

	len = param_getstr(Cmd,nameParamNo,filename);

	if (len > FILE_PATH_SIZE - 5) len = FILE_PATH_SIZE - 5;

	// user supplied filename?
	if (len < 1) {
		// get filename (UID from memory)
		if (mfEmlGetMem(buf, 0, 1)) {
			PrintAndLog("Can\'t get UID from block: %d", 0);
			len = sprintf(fnameptr, "dump");
			fnameptr += len;
		}
		else {
			for (j = 0; j < 7; j++, fnameptr += 2)
				sprintf(fnameptr, "%02X", buf[j]);
		}
	} else {
		fnameptr += len;
	}

	// add file extension
	sprintf(fnameptr, ".eml");

	// open file
	f = fopen(filename, "w+");

	if ( !f ) {
		PrintAndLog("Can't open file %s ", filename);
		return 1;
	}

	// put hex
	for (i = 0; i < numBlocks; i++) {
		if (mfEmlGetMem(buf, i, 1)) {
			PrintAndLog("Cant get block: %d", i);
			break;
		}
		for (j = 0; j < 16; j++)
			fprintf(f, "%02X", buf[j]);
		fprintf(f,"\n");
	}
	fclose(f);

	PrintAndLog("Saved %d blocks to file: %s", numBlocks, filename);

  return 0;
}


int CmdHF14AMfECFill(const char *Cmd)
{
	uint8_t keyType = 0;
	uint8_t numSectors = 16;

	if (strlen(Cmd) < 1 || param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("Usage:  hf mf ecfill <key A/B> [card memory]");
		PrintAndLog("  [card memory]: 0 = 320 bytes (Mifare Mini), 1 = 1K (default), 2 = 2K, 4 = 4K");
		PrintAndLog("");
		PrintAndLog("samples:  hf mf ecfill A");
		PrintAndLog("          hf mf ecfill A 4");
		PrintAndLog("Read card and transfer its data to emulator memory.");
		PrintAndLog("Keys must be laid in the emulator memory. \n");
		return 0;
	}

	char ctmp = param_getchar(Cmd, 0);
	if (ctmp != 'a' && ctmp != 'A' && ctmp != 'b' && ctmp != 'B') {
		PrintAndLog("Key type must be A or B");
		return 1;
	}
	if (ctmp != 'A' && ctmp != 'a') keyType = 1;

	ctmp = param_getchar(Cmd, 1);
	switch (ctmp) {
		case '0' : numSectors = 5; break;
		case '1' :
		case '\0': numSectors = 16; break;
		case '2' : numSectors = 32; break;
		case '4' : numSectors = 40; break;
		default:   numSectors = 16;
	}

	printf("--params: numSectors: %d, keyType:%d", numSectors, keyType);
	UsbCommand c = {CMD_MIFARE_EML_CARDLOAD, {numSectors, keyType, 0}};
	SendCommand(&c);
	return 0;
}

int CmdHF14AMfEKeyPrn(const char *Cmd)
{
	int i;
	uint8_t numSectors;
	uint8_t data[16];
	uint64_t keyA, keyB;

	if (param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("It prints the keys loaded in the emulator memory");
		PrintAndLog("Usage:  hf mf ekeyprn [card memory]");
		PrintAndLog("  [card memory]: 0 = 320 bytes (Mifare Mini), 1 = 1K (default), 2 = 2K, 4 = 4K");
		PrintAndLog("");
		PrintAndLog(" sample: hf mf ekeyprn 1");
		return 0;
	}

	char cmdp = param_getchar(Cmd, 0);

	switch (cmdp) {
		case '0' : numSectors = 5; break;
		case '1' :
		case '\0': numSectors = 16; break;
		case '2' : numSectors = 32; break;
		case '4' : numSectors = 40; break;
		default:   numSectors = 16;
	}

	PrintAndLog("|---|----------------|----------------|");
	PrintAndLog("|sec|key A           |key B           |");
	PrintAndLog("|---|----------------|----------------|");
	for (i = 0; i < numSectors; i++) {
		if (mfEmlGetMem(data, FirstBlockOfSector(i) + NumBlocksPerSector(i) - 1, 1)) {
			PrintAndLog("error get block %d", FirstBlockOfSector(i) + NumBlocksPerSector(i) - 1);
			break;
		}
		keyA = bytes_to_num(data, 6);
		keyB = bytes_to_num(data + 10, 6);
		PrintAndLog("|%03d|  %012" PRIx64 "  |  %012" PRIx64 "  |", i, keyA, keyB);
	}
	PrintAndLog("|---|----------------|----------------|");

	return 0;
}

int CmdHF14AMfCSetUID(const char *Cmd)
{
	uint8_t uid[8] = {0x00};
	uint8_t oldUid[8] = {0x00};
	uint8_t atqa[2] = {0x00};
	uint8_t sak[1] = {0x00};
	uint8_t atqaPresent = 0;
	int res;

	uint8_t needHelp = 0;
	char cmdp = 1;
	
	if (param_getchar(Cmd, 0) && param_gethex(Cmd, 0, uid, 8)) {
		PrintAndLog("UID must include 8 HEX symbols");
		return 1;
	}

	if (param_getlength(Cmd, 1) > 1 && param_getlength(Cmd, 2) >  1) {
		atqaPresent = 1;
		cmdp = 3;
		
		if (param_gethex(Cmd, 1, atqa, 4)) {
			PrintAndLog("ATQA must include 4 HEX symbols");
			return 1;
		}
				
		if (param_gethex(Cmd, 2, sak, 2)) {
			PrintAndLog("SAK must include 2 HEX symbols");
			return 1;
		}
	}

	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
		case 'H':
			needHelp = 1;
			break;
		default:
			PrintAndLog("ERROR: Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			needHelp = 1;
			break;
		}
		cmdp++;
	}

	if (strlen(Cmd) < 1 || needHelp) {
		PrintAndLog("");
		PrintAndLog("Usage:  hf mf csetuid <UID 8 hex symbols> [ATQA 4 hex symbols SAK 2 hex symbols]");
		PrintAndLog("sample:  hf mf csetuid 01020304");
		PrintAndLog("sample:  hf mf csetuid 01020304 0004 08");
		PrintAndLog("Set UID, ATQA, and SAK for magic Chinese card (only works with such cards)");
		return 0;
	}

	PrintAndLog("uid:%s", sprint_hex(uid, 4));
	if (atqaPresent) {
		PrintAndLog("--atqa:%s sak:%02x", sprint_hex(atqa, 2), sak[0]);
	}

	res = mfCSetUID(uid, (atqaPresent)?atqa:NULL, (atqaPresent)?sak:NULL, oldUid);
	if (res) {
			PrintAndLog("Can't set UID. Error=%d", res);
			return 1;
		}

	PrintAndLog("old UID:%s", sprint_hex(oldUid, 4));
	PrintAndLog("new UID:%s", sprint_hex(uid, 4));
	return 0;
}

int CmdHF14AMfCWipe(const char *Cmd)
{
	int res, gen = 0;
	int numBlocks = 16 * 4;
	bool wipeCard = false;
	bool fillCard = false;
	
	if (strlen(Cmd) < 1 || param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("Usage:  hf mf cwipe [card size] [w] [p]");
		PrintAndLog("sample:  hf mf cwipe 1 w s");
		PrintAndLog("[card size]: 0 = 320 bytes (Mifare Mini), 1 = 1K (default), 2 = 2K, 4 = 4K");
		PrintAndLog("w - Wipe magic Chinese card (only works with gen:1a cards)");
		PrintAndLog("f - Fill the card with default data and keys (works with gen:1a and gen:1b cards only)");
		return 0;
	}

	gen = mfCIdentify();
	if ((gen != 1) && (gen != 2)) 
		return 1;
	
	numBlocks = ParamCardSizeBlocks(param_getchar(Cmd, 0));

	char cmdp = 0;
	while(param_getchar(Cmd, cmdp) != 0x00){
		switch(param_getchar(Cmd, cmdp)) {
		case 'w':
		case 'W':
			wipeCard = 1;
			break;
		case 'f':
		case 'F':
			fillCard = 1;
			break;
		default:
			break;
		}
		cmdp++;
	}

	if (!wipeCard && !fillCard) 
		wipeCard = true;

	PrintAndLog("--blocks count:%2d wipe:%c fill:%c", numBlocks, (wipeCard)?'y':'n', (fillCard)?'y':'n');

	if (gen == 2) {
		/* generation 1b magic card */
		if (wipeCard) {
			PrintAndLog("WARNING: can't wipe magic card 1b generation");
		}
		res = mfCWipe(numBlocks, true, false, fillCard); 
	} else {
		/* generation 1a magic card by default */
		res = mfCWipe(numBlocks, false, wipeCard, fillCard); 
	}

	if (res) {
		PrintAndLog("Can't wipe. error=%d", res);
		return 1;
	}
	PrintAndLog("OK");
	return 0;
}

int CmdHF14AMfCSetBlk(const char *Cmd)
{
	uint8_t memBlock[16] = {0x00};
	uint8_t blockNo = 0;
	bool wipeCard = false;
	int res, gen = 0;

	if (strlen(Cmd) < 1 || param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("Usage:  hf mf csetblk <block number> <block data (32 hex symbols)> [w]");
		PrintAndLog("sample:  hf mf csetblk 1 01020304050607080910111213141516");
		PrintAndLog("Set block data for magic Chinese card (only works with such cards)");
		PrintAndLog("If you also want wipe the card then add 'w' at the end of the command line");
		return 0;
	}

	gen = mfCIdentify();
	if ((gen != 1) && (gen != 2)) 
		return 1;

	blockNo = param_get8(Cmd, 0);

	if (param_gethex(Cmd, 1, memBlock, 32)) {
		PrintAndLog("block data must include 32 HEX symbols");
		return 1;
	}

	char ctmp = param_getchar(Cmd, 2);
	wipeCard = (ctmp == 'w' || ctmp == 'W');
	PrintAndLog("--block number:%2d data:%s", blockNo, sprint_hex(memBlock, 16));

	if (gen == 2) {
		/* generation 1b magic card */
		res = mfCSetBlock(blockNo, memBlock, NULL, wipeCard, CSETBLOCK_SINGLE_OPER | CSETBLOCK_MAGIC_1B);
	} else {
		/* generation 1a magic card by default */
		res = mfCSetBlock(blockNo, memBlock, NULL, wipeCard, CSETBLOCK_SINGLE_OPER);
	}

	if (res) {
		PrintAndLog("Can't write block. error=%d", res);
		return 1;
	}
	return 0;
}


int CmdHF14AMfCLoad(const char *Cmd)
{
	FILE * f;
	char filename[FILE_PATH_SIZE] = {0x00};
	char * fnameptr = filename;
	char buf[256] = {0x00};
	uint8_t buf8[256] = {0x00};
	uint8_t fillFromEmulator = 0;
	int i, len, blockNum, flags = 0, gen = 0, numblock = 64;

	if (param_getchar(Cmd, 0) == 'h' || param_getchar(Cmd, 0)== 0x00) {
		PrintAndLog("It loads magic Chinese card from the file `filename.eml`");
		PrintAndLog("or from emulator memory (option `e`). 4K card: (option `4`)");
		PrintAndLog("Usage:  hf mf cload [file name w/o `.eml`][e][4]");
		PrintAndLog("   or:  hf mf cload e [4]");
		PrintAndLog("Sample: hf mf cload filename");
		PrintAndLog("        hf mf cload filname 4");
		PrintAndLog("        hf mf cload e");
		PrintAndLog("        hf mf cload e 4");
		return 0;
	}

	char ctmp = param_getchar(Cmd, 0);
	if (ctmp == 'e' || ctmp == 'E') fillFromEmulator = 1;
	ctmp = param_getchar(Cmd, 1);
	if (ctmp == '4') numblock = 256;

	gen = mfCIdentify();
	PrintAndLog("Loading magic mifare %dK", numblock == 256 ? 4:1);

	if (fillFromEmulator) {
		for (blockNum = 0; blockNum < numblock; blockNum += 1) {
			if (mfEmlGetMem(buf8, blockNum, 1)) {
				PrintAndLog("Cant get block: %d", blockNum);
				return 2;
			}
			if (blockNum == 0) flags = CSETBLOCK_INIT_FIELD + CSETBLOCK_WUPC;				// switch on field and send magic sequence
			if (blockNum == 1) flags = 0;													// just write
			if (blockNum == numblock - 1) flags = CSETBLOCK_HALT + CSETBLOCK_RESET_FIELD;		// Done. Magic Halt and switch off field.

			if (gen == 2)
				/* generation 1b magic card */
				flags |= CSETBLOCK_MAGIC_1B;
			if (mfCSetBlock(blockNum, buf8, NULL, 0, flags)) {
				PrintAndLog("Cant set magic card block: %d", blockNum);
				return 3;
			}
		}
		return 0;
	} else {
		param_getstr(Cmd, 0, filename);

		len = strlen(filename);
		if (len > FILE_PATH_SIZE - 5) len = FILE_PATH_SIZE - 5;

		//memcpy(filename, Cmd, len);
		fnameptr += len;

		sprintf(fnameptr, ".eml");

		// open file
		f = fopen(filename, "r");
		if (f == NULL) {
			PrintAndLog("File not found or locked.");
			return 1;
		}

		blockNum = 0;
		while(!feof(f)){

			memset(buf, 0, sizeof(buf));

			if (fgets(buf, sizeof(buf), f) == NULL) {
				fclose(f);
				PrintAndLog("File reading error.");
				return 2;
			}

			if (strlen(buf) < 32) {
				if(strlen(buf) && feof(f))
					break;
				PrintAndLog("File content error. Block data must include 32 HEX symbols");
				fclose(f);
				return 2;
			}
			for (i = 0; i < 32; i += 2)
				sscanf(&buf[i], "%02x", (unsigned int *)&buf8[i / 2]);

			if (blockNum == 0) flags = CSETBLOCK_INIT_FIELD + CSETBLOCK_WUPC;				// switch on field and send magic sequence
			if (blockNum == 1) flags = 0;													// just write
			if (blockNum == numblock - 1) flags = CSETBLOCK_HALT + CSETBLOCK_RESET_FIELD;		// Done. Switch off field.

			if (gen == 2)
				/* generation 1b magic card */
				flags |= CSETBLOCK_MAGIC_1B;
			if (mfCSetBlock(blockNum, buf8, NULL, 0, flags)) {
				PrintAndLog("Can't set magic card block: %d", blockNum);
				fclose(f);
				return 3;
			}
			blockNum++;

			if (blockNum >= numblock) break;  // magic card type - mifare 1K 64 blocks, mifare 4k 256 blocks
		}
		fclose(f);

		//if (blockNum != 16 * 4 && blockNum != 32 * 4 + 8 * 16){
		if (blockNum != numblock){
			PrintAndLog("File content error. There must be %d blocks", numblock);
			return 4;
		}
		PrintAndLog("Loaded from file: %s", filename);
		return 0;
	}
	return 0;
}

int CmdHF14AMfCGetBlk(const char *Cmd) {
	uint8_t memBlock[16];
	uint8_t blockNo = 0;
	int res, gen = 0;
	memset(memBlock, 0x00, sizeof(memBlock));

	if (strlen(Cmd) < 1 || param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("Usage:  hf mf cgetblk <block number>");
		PrintAndLog("sample:  hf mf cgetblk 1");
		PrintAndLog("Get block data from magic Chinese card (only works with such cards)\n");
		return 0;
	}

	gen = mfCIdentify();

	blockNo = param_get8(Cmd, 0);

	PrintAndLog("--block number:%2d ", blockNo);

	if (gen == 2) {
		/* generation 1b magic card */
		res = mfCGetBlock(blockNo, memBlock, CSETBLOCK_SINGLE_OPER | CSETBLOCK_MAGIC_1B);
	} else {
		/* generation 1a magic card by default */
		res = mfCGetBlock(blockNo, memBlock, CSETBLOCK_SINGLE_OPER);
	}
	if (res) {
			PrintAndLog("Can't read block. error=%d", res);
			return 1;
		}

	PrintAndLog("block data:%s", sprint_hex(memBlock, 16));
	return 0;
}

int CmdHF14AMfCGetSc(const char *Cmd) {
	uint8_t memBlock[16] = {0x00};
	uint8_t sectorNo = 0;
	int i, res, flags, gen = 0, baseblock = 0, sect_size = 4;

	if (strlen(Cmd) < 1 || param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("Usage:  hf mf cgetsc <sector number>");
		PrintAndLog("sample:  hf mf cgetsc 0");
		PrintAndLog("Get sector data from magic Chinese card (only works with such cards)\n");
		return 0;
	}

	sectorNo = param_get8(Cmd, 0);

	if (sectorNo > 39) {
		PrintAndLog("Sector number must be in [0..15] in MIFARE classic 1k and [0..39] in MIFARE classic 4k.");
		return 1;
	}

	PrintAndLog("--sector number:%d ", sectorNo);

	gen = mfCIdentify();

	flags = CSETBLOCK_INIT_FIELD + CSETBLOCK_WUPC;
	if (sectorNo < 32 ) {
		baseblock = sectorNo * 4;
	} else {
		baseblock = 128 + 16 * (sectorNo - 32);

	}
	if (sectorNo > 31) sect_size = 16;

	for (i = 0; i < sect_size; i++) {
		if (i == 1) flags = 0;
		if (i == sect_size - 1) flags = CSETBLOCK_HALT + CSETBLOCK_RESET_FIELD;

		if (gen == 2)
			/* generation 1b magic card */
			flags |= CSETBLOCK_MAGIC_1B;

		res = mfCGetBlock(baseblock + i, memBlock, flags);
		if (res) {
			PrintAndLog("Can't read block. %d error=%d", baseblock + i, res);
			return 1;
		}

		PrintAndLog("block %3d data:%s", baseblock + i, sprint_hex(memBlock, 16));
	}
	return 0;
}


int CmdHF14AMfCSave(const char *Cmd) {

	FILE * f;
	char filename[FILE_PATH_SIZE] = {0x00};
	char * fnameptr = filename;
	uint8_t fillFromEmulator = 0;
	uint8_t buf[256] = {0x00};
	int i, j, len, flags, gen = 0, numblock = 64;

	// memset(filename, 0, sizeof(filename));
	// memset(buf, 0, sizeof(buf));

	if (param_getchar(Cmd, 0) == 'h') {
		PrintAndLog("It saves `magic Chinese` card dump into the file `filename.eml` or `cardID.eml`");
		PrintAndLog("or into emulator memory (option `e`). 4K card: (option `4`)");
		PrintAndLog("Usage:  hf mf csave [file name w/o `.eml`][e][4]");
		PrintAndLog("Sample: hf mf csave ");
		PrintAndLog("        hf mf csave filename");
		PrintAndLog("        hf mf csave e");
		PrintAndLog("        hf mf csave 4");
		PrintAndLog("        hf mf csave filename 4");
		PrintAndLog("        hf mf csave e 4");
		return 0;
	}

	char ctmp = param_getchar(Cmd, 0);
	if (ctmp == 'e' || ctmp == 'E') fillFromEmulator = 1;
	if (ctmp == '4') numblock = 256;
	ctmp = param_getchar(Cmd, 1);
	if (ctmp == '4') numblock = 256;

	gen = mfCIdentify();
	PrintAndLog("Saving magic mifare %dK", numblock == 256 ? 4:1);

	if (fillFromEmulator) {
		// put into emulator
		flags = CSETBLOCK_INIT_FIELD + CSETBLOCK_WUPC;
		for (i = 0; i < numblock; i++) {
			if (i == 1) flags = 0;
			if (i == numblock - 1) flags = CSETBLOCK_HALT + CSETBLOCK_RESET_FIELD;

			if (gen == 2)
				/* generation 1b magic card */
				flags |= CSETBLOCK_MAGIC_1B;

			if (mfCGetBlock(i, buf, flags)) {
				PrintAndLog("Cant get block: %d", i);
				break;
			}

			if (mfEmlSetMem(buf, i, 1)) {
				PrintAndLog("Cant set emul block: %d", i);
				return 3;
			}
		}
		return 0;
	} else {
		param_getstr(Cmd, 0, filename);

		len = strlen(filename);
		if (len > FILE_PATH_SIZE - 5) len = FILE_PATH_SIZE - 5;

		ctmp = param_getchar(Cmd, 0);
		if (len < 1 || (ctmp == '4')) {
			// get filename

			flags = CSETBLOCK_SINGLE_OPER;
			if (gen == 2)
				/* generation 1b magic card */
				flags |= CSETBLOCK_MAGIC_1B;
			if (mfCGetBlock(0, buf, flags)) {
				PrintAndLog("Cant get block: %d", 0);
				len = sprintf(fnameptr, "dump");
				fnameptr += len;
			}
			else {
				for (j = 0; j < 7; j++, fnameptr += 2)
					sprintf(fnameptr, "%02x", buf[j]);
			}
		} else {
			//memcpy(filename, Cmd, len);
			fnameptr += len;
		}

		sprintf(fnameptr, ".eml");

		// open file
		f = fopen(filename, "w+");

		if (f == NULL) {
			PrintAndLog("File not found or locked.");
			return 1;
		}

		// put hex
		flags = CSETBLOCK_INIT_FIELD + CSETBLOCK_WUPC;
		for (i = 0; i < numblock; i++) {
			if (i == 1) flags = 0;
			if (i == numblock - 1) flags = CSETBLOCK_HALT + CSETBLOCK_RESET_FIELD;

			if (gen == 2)
				/* generation 1b magic card */
				flags |= CSETBLOCK_MAGIC_1B;
			if (mfCGetBlock(i, buf, flags)) {
				PrintAndLog("Cant get block: %d", i);
				break;
			}
			for (j = 0; j < 16; j++)
				fprintf(f, "%02x", buf[j]);
			fprintf(f,"\n");
		}
		fclose(f);

		PrintAndLog("Saved to file: %s", filename);

		return 0;
	}
}


int CmdHF14AMfSniff(const char *Cmd){

	bool wantLogToFile = 0;
	bool wantDecrypt = 0;
	//bool wantSaveToEml = 0; TODO
	bool wantSaveToEmlFile = 0;

	//var
	int res = 0;
	int len = 0;
	int blockLen = 0;
	int pckNum = 0;
	int num = 0;
	uint8_t uid[7];
	uint8_t uid_len;
	uint8_t atqa[2] = {0x00};
	uint8_t sak;
	bool isTag;
	uint8_t *buf = NULL;
	uint16_t bufsize = 0;
	uint8_t *bufPtr = NULL;

	char ctmp = param_getchar(Cmd, 0);
	if ( ctmp == 'h' || ctmp == 'H' ) {
		PrintAndLog("It continuously gets data from the field and saves it to: log, emulator, emulator file.");
		PrintAndLog("You can specify:");
		PrintAndLog("    l - save encrypted sequence to logfile `uid.log`");
		PrintAndLog("    d - decrypt sequence and put it to log file `uid.log`");
		PrintAndLog(" n/a   e - decrypt sequence, collect read and write commands and save the result of the sequence to emulator memory");
		PrintAndLog("    f - decrypt sequence, collect read and write commands and save the result of the sequence to emulator dump file `uid.eml`");
		PrintAndLog("Usage:  hf mf sniff [l][d][e][f]");
		PrintAndLog("  sample: hf mf sniff l d e");
		return 0;
	}

	for (int i = 0; i < 4; i++) {
		ctmp = param_getchar(Cmd, i);
		if (ctmp == 'l' || ctmp == 'L') wantLogToFile = true;
		if (ctmp == 'd' || ctmp == 'D') wantDecrypt = true;
		//if (ctmp == 'e' || ctmp == 'E') wantSaveToEml = true; TODO
		if (ctmp == 'f' || ctmp == 'F') wantSaveToEmlFile = true;
	}

	printf("-------------------------------------------------------------------------\n");
	printf("Executing command. \n");
	printf("Press the key on the proxmark3 device to abort both proxmark3 and client.\n");
	printf("Press the key on pc keyboard to abort the client.\n");
	printf("-------------------------------------------------------------------------\n");

	UsbCommand c = {CMD_MIFARE_SNIFFER, {0, 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);

	// wait cycle
	while (true) {
		printf(".");
		fflush(stdout);
		if (ukbhit()) {
			getchar();
			printf("\naborted via keyboard!\n");
			break;
		}

		UsbCommand resp;
		if (WaitForResponseTimeout(CMD_ACK,&resp,2000)) {
			res = resp.arg[0] & 0xff;
			uint16_t traceLen = resp.arg[1];
			len = resp.arg[2];

			if (res == 0) {								// we are done
				free(buf);
				return 0;
			}

			if (res == 1) {								// there is (more) data to be transferred
				if (pckNum == 0) {						// first packet, (re)allocate necessary buffer
					if (traceLen > bufsize || buf == NULL) {
						uint8_t *p;
						if (buf == NULL) {				// not yet allocated
							p = malloc(traceLen);
						} else {						// need more memory
							p = realloc(buf, traceLen);
						}
						if (p == NULL) {
							PrintAndLog("Cannot allocate memory for trace");
							free(buf);
							return 2;
						}
						buf = p;
					}
					bufPtr = buf;
					bufsize = traceLen;
					memset(buf, 0x00, traceLen);
				}
				memcpy(bufPtr, resp.d.asBytes, len);
				bufPtr += len;
				pckNum++;
			}

			if (res == 2) {								// received all data, start displaying
				blockLen = bufPtr - buf;
				bufPtr = buf;
				printf(">\n");
				PrintAndLog("received trace len: %d packages: %d", blockLen, pckNum);
				while (bufPtr - buf < blockLen) {
					bufPtr += 6;						// skip (void) timing information
					len = *((uint16_t *)bufPtr);
					if(len & 0x8000) {
						isTag = true;
						len &= 0x7fff;
					} else {
						isTag = false;
					}
					bufPtr += 2;
					if ((len == 14) && (bufPtr[0] == 0xff) && (bufPtr[1] == 0xff) && (bufPtr[12] == 0xff) && (bufPtr[13] == 0xff)) {
						memcpy(uid, bufPtr + 2, 7);
						memcpy(atqa, bufPtr + 2 + 7, 2);
						uid_len = (atqa[0] & 0xC0) == 0x40 ? 7 : 4;
						sak = bufPtr[11];
						PrintAndLog("tag select uid:%s atqa:0x%02x%02x sak:0x%02x",
							sprint_hex(uid + (7 - uid_len), uid_len),
							atqa[1],
							atqa[0],
							sak);
						if (wantLogToFile || wantDecrypt) {
							FillFileNameByUID(logHexFileName, uid + (7 - uid_len), ".log", uid_len);
							AddLogCurrentDT(logHexFileName);
						}
						if (wantDecrypt)
							mfTraceInit(uid, atqa, sak, wantSaveToEmlFile);
					} else {
						PrintAndLog("%s(%d):%s", isTag ? "TAG":"RDR", num, sprint_hex(bufPtr, len));
						if (wantLogToFile)
							AddLogHex(logHexFileName, isTag ? "TAG: ":"RDR: ", bufPtr, len);
						if (wantDecrypt)
							mfTraceDecode(bufPtr, len, wantSaveToEmlFile);
						num++;
					}
					bufPtr += len;
					bufPtr += ((len-1)/8+1);	// ignore parity
				}
				pckNum = 0;
			}
		} // resp not NULL
	} // while (true)

	free(buf);
	return 0;
}

//needs nt, ar, at, Data to decrypt
int CmdDecryptTraceCmds(const char *Cmd){
	uint8_t data[50];
	int len = 0;
	param_gethex_ex(Cmd,3,data,&len);
	return tryDecryptWord(param_get32ex(Cmd,0,0,16),param_get32ex(Cmd,1,0,16),param_get32ex(Cmd,2,0,16),data,len/2);
}

static command_t CommandTable[] =
{
  {"help",             CmdHelp,                 1, "This help"},
  {"dbg",              CmdHF14AMfDbg,           0, "Set default debug mode"},
  {"rdbl",             CmdHF14AMfRdBl,          0, "Read MIFARE classic block"},
  {"rdsc",             CmdHF14AMfRdSc,          0, "Read MIFARE classic sector"},
  {"dump",             CmdHF14AMfDump,          0, "Dump MIFARE classic tag to binary file"},
  {"restore",  	       CmdHF14AMfRestore,       0, "Restore MIFARE classic binary file to BLANK tag"},
  {"wrbl",             CmdHF14AMfWrBl,          0, "Write MIFARE classic block"},
  {"chk",              CmdHF14AMfChk,           0, "Test block keys"},
  {"mifare",           CmdHF14AMifare,          0, "Read parity error messages."},
  {"hardnested",       CmdHF14AMfNestedHard,    0, "Nested attack for hardened Mifare cards"},
  {"nested",           CmdHF14AMfNested,        0, "Test nested authentication"},
  {"sniff",            CmdHF14AMfSniff,         0, "Sniff card-reader communication"},
  {"sim",              CmdHF14AMf1kSim,         0, "Simulate MIFARE card"},
  {"eclr",             CmdHF14AMfEClear,        0, "Clear simulator memory block"},
  {"eget",             CmdHF14AMfEGet,          0, "Get simulator memory block"},
  {"eset",             CmdHF14AMfESet,          0, "Set simulator memory block"},
  {"eload",            CmdHF14AMfELoad,         0, "Load from file emul dump"},
  {"esave",            CmdHF14AMfESave,         0, "Save to file emul dump"},
  {"ecfill",           CmdHF14AMfECFill,        0, "Fill simulator memory with help of keys from simulator"},
  {"ekeyprn",          CmdHF14AMfEKeyPrn,       0, "Print keys from simulator memory"},
  {"cwipe",            CmdHF14AMfCWipe,         0, "Wipe magic Chinese card"},
  {"csetuid",          CmdHF14AMfCSetUID,       0, "Set UID for magic Chinese card"},
  {"csetblk",          CmdHF14AMfCSetBlk,       0, "Write block - Magic Chinese card"},
  {"cgetblk",          CmdHF14AMfCGetBlk,       0, "Read block - Magic Chinese card"},
  {"cgetsc",           CmdHF14AMfCGetSc,        0, "Read sector - Magic Chinese card"},
  {"cload",            CmdHF14AMfCLoad,         0, "Load dump into magic Chinese card"},
  {"csave",            CmdHF14AMfCSave,         0, "Save dump from magic Chinese card into file or emulator"},
  {"decrypt",          CmdDecryptTraceCmds,     1, "[nt] [ar_enc] [at_enc] [data] - to decrypt snoop or trace"},
  {NULL,               NULL,                    0, NULL}
};

int CmdHFMF(const char *Cmd)
{
	// flush
	WaitForResponseTimeout(CMD_ACK,NULL,100);

  CmdsParse(CommandTable, Cmd);
  return 0;
}

int CmdHelp(const char *Cmd)
{
  CmdsHelp(CommandTable);
  return 0;
}
