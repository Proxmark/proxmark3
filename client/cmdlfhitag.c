//-----------------------------------------------------------------------------
// Copyright (C) 2012 Roel Verdult
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Hitag support
//-----------------------------------------------------------------------------

#include "cmdlfhitag.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comms.h"
#include "ui.h"
#include "cmdparser.h"
#include "common.h"
#include "util.h"
#include "parity.h"
#include "hitag.h"
#include "cmdmain.h"

static size_t nbytes(size_t nbits) {
	return (nbits/8) + ((nbits%8)>0);
}


static int CmdLFHitagList(const char *Cmd) {
	uint8_t *got = malloc(USB_CMD_DATA_SIZE);
	// Query for the actual size of the trace
	UsbCommand response;
	GetFromBigBuf(got, USB_CMD_DATA_SIZE, 0, &response, -1, false);
	uint16_t traceLen = response.arg[2];
	if (traceLen > USB_CMD_DATA_SIZE) {
		uint8_t *p = realloc(got, traceLen);
		if (p == NULL) {
			PrintAndLog("Cannot allocate memory for trace");
			free(got);
			return 2;
		}
		got = p;
		GetFromBigBuf(got, traceLen, 0, NULL, -1, false);
	}

	PrintAndLog("recorded activity (TraceLen = %d bytes):");
	PrintAndLog(" ETU     :nbits: who bytes");
	PrintAndLog("---------+-----+----+-----------");

	int j;
	int i = 0;
	int prev = -1;
	int len = strlen(Cmd);

	char filename[FILE_PATH_SIZE]  = { 0x00 };
	FILE* pf = NULL;

	if (len > FILE_PATH_SIZE)
		len = FILE_PATH_SIZE;
	memcpy(filename, Cmd, len);

	if (strlen(filename) > 0) {
		if ((pf = fopen(filename,"wb")) == NULL) {
			PrintAndLog("Error: Could not open file [%s]",filename);
			free(got);
			return 1;
		}
	}

	for (;;) {

		if(i > traceLen) { break; }

		bool isResponse;
		int timestamp = *((uint32_t *)(got+i));
		if (timestamp & 0x80000000) {
			timestamp &= 0x7fffffff;
			isResponse = 1;
		} else {
			isResponse = 0;
		}

		int parityBits = *((uint32_t *)(got+i+4));
		// 4 bytes of additional information...
		// maximum of 32 additional parity bit information
		//
		// TODO:
		// at each quarter bit period we can send power level (16 levels)
		// or each half bit period in 256 levels.

		int bits = got[i+8];
		int len = nbytes(bits);

		if (len > 100) {
			break;
		}
		if (i + len > traceLen) { break;}

		uint8_t *frame = (got+i+9);
/*
		int fillupBits = 8 - (bits % 8);
		byte_t framefilled[bits+fillupBits];
		byte_t* ff = framefilled;

		int response_bit[200] = {0};
		int z = 0;
		for (int y = 0; y < len; y++) {
			for (j = 0; j < 8; j++) {
				response_bit[z] = 0;
				if ((frame[y] & ((mask << 7) >> j)) != 0)
					response_bit[z] = 1;
				z++;
			}
		}
		z = 0;
		for (int y = 0; y < len; y++) {
			ff[y] = (response_bit[z] << 7) | (response_bit[z + 1] << 6)
					| (response_bit[z + 2] << 5) | (response_bit[z + 3] << 4)
					| (response_bit[z + 4] << 3) | (response_bit[z + 5] << 2)
					| (response_bit[z + 6] << 1) | response_bit[z + 7];
			z += 8;
		}
*/




		// Break and stick with current result if buffer was not completely full
		if (frame[0] == 0x44 && frame[1] == 0x44 && frame[3] == 0x44) { break; }

		char line[1000] = "";
		for (j = 0; j < len; j++) {
			//if((parityBits >> (len - j - 1)) & 0x01) {
			if (isResponse && (oddparity8(frame[j]) != ((parityBits >> (len - j - 1)) & 0x01))) {
				sprintf(line+(j*4), "%02x!  ", frame[j]);
			} else {
				sprintf(line+(j*4), "%02x   ", frame[j]);
			}
		}

		PrintAndLog(" +%7d:  %3d: %s %s",
			(prev < 0 ? 0 : (timestamp - prev)),
			bits,
			(isResponse ? "TAG" : "   "),
			line);

		if (pf) {
			fprintf(pf," +%7d:  %3d: %s %s\n",
				(prev < 0 ? 0 : (timestamp - prev)),
				bits,
				(isResponse ? "TAG" : "   "),
				line);
		}

		prev = timestamp;
		i += (len + 9);
	}

	if (pf) {
		fclose(pf);
		PrintAndLog("Recorded activity succesfully written to file: %s", filename);
	}

	free(got);
	return 0;
}


static int CmdLFHitagSnoop(const char *Cmd) {
	UsbCommand c = {CMD_SNOOP_HITAG};
	SendCommand(&c);
	return 0;
}


static int CmdLFHitagSim(const char *Cmd) {

	UsbCommand c = {CMD_SIMULATE_HITAG};
	char filename[FILE_PATH_SIZE] = { 0x00 };
	FILE* pf;
	bool tag_mem_supplied;
	int len = strlen(Cmd);
	if (len > FILE_PATH_SIZE) len = FILE_PATH_SIZE;
	memcpy(filename, Cmd, len);

	if (strlen(filename) > 0) {
		if ((pf = fopen(filename,"rb+")) == NULL) {
			PrintAndLog("Error: Could not open file [%s]",filename);
			return 1;
		}
		tag_mem_supplied = true;
		if (fread(c.d.asBytes,1,48,pf) != 48) {
			PrintAndLog("Error: File reading error");
			fclose(pf);
			return 1;
		}
		fclose(pf);
	} else {
		tag_mem_supplied = false;
	}

	// Does the tag comes with memory
	c.arg[0] = (uint32_t)tag_mem_supplied;

	SendCommand(&c);
	return 0;
}


static bool getHitagUid(uint32_t *uid) {
	// ToDo: this is for Hitag2 only (??)
	
	UsbCommand c = {CMD_READER_HITAG, {RHT2F_UID_ONLY}};

	SendCommand(&c);

	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 2500)) {
		PrintAndLogEx(WARNING, "timeout while waiting for reply.");
		return false;
	}

	if (resp.arg[0] == false) {
		PrintAndLogEx(DEBUG, "DEBUG: Error - failed getting UID");
		return false;
	}

	if (uid)
		*uid = bytes_to_num(resp.d.asBytes, 4);

	return true;
}


static int CmdLFHitagInfo(const char *Cmd) {
	char ctmp = param_getchar(Cmd, 0);
	if (ctmp != '\0') {
		PrintAndLog("Usage:   lf hitag info [h]");
		PrintAndLog("Options:");
		PrintAndLog("       h          This help");
		PrintAndLog("Examples:");
		PrintAndLog("         lf hitag info");
		return 0;
	}

	// read UID
	uint32_t uid = 0;
	if (getHitagUid(&uid) == false)
		return 1;

	PrintAndLogEx(SUCCESS, "UID: %08X", uid);

	// how to detemine Hitag types?
	// read block3,  get configuration byte.
	// PrintAndLogEx(FAILED, _RED_("TODO: This is a hardcoded example!"));

	// common configurations.
	// printHitagConfiguration(0x06);
	//printHitagConfiguration( 0x0E );
	//printHitagConfiguration( 0x02 );
	//printHitagConfiguration( 0x00 );
	//printHitagConfiguration( 0x04 );
	return 0;
}


int CmdLFHitagReader(const char *Cmd) {
	UsbCommand c = {CMD_READER_HITAG};
	hitag_data* htd = (hitag_data*)c.d.asBytes;
	hitag_function htf = param_get32ex(Cmd, 0, 0, 10);

	switch (htf) {
		case 01: { //RHTSF_CHALLENGE
			c = (UsbCommand){ CMD_READ_HITAG_S };
			num_to_bytes(param_get32ex(Cmd, 1, 0, 16), 4, htd->auth.NrAr);
			num_to_bytes(param_get32ex(Cmd, 2, 0, 16), 4, htd->auth.NrAr+4);
			c.arg[1] = param_get64ex(Cmd, 3, 0, 0); //firstpage
			c.arg[2] = param_get64ex(Cmd, 4, 0, 0); //tag mode
		} break;
		case 02: { //RHTSF_KEY
			c = (UsbCommand){ CMD_READ_HITAG_S };
			num_to_bytes(param_get64ex(Cmd, 1, 0, 16), 6, htd->crypto.key);
			c.arg[1] = param_get64ex(Cmd, 2, 0, 0); //firstpage
			c.arg[2] = param_get64ex(Cmd, 3, 0, 0); //tag mode
		} break;
		case 03: { //RHTSF_CHALLENGE BLOCK
			c = (UsbCommand){ CMD_READ_HITAG_S_BLK };
			num_to_bytes(param_get32ex(Cmd, 1, 0, 16), 4, htd->auth.NrAr);
			num_to_bytes(param_get32ex(Cmd, 2, 0, 16), 4, htd->auth.NrAr+4);
			c.arg[1] = param_get64ex(Cmd, 3, 0, 0); //firstpage
			c.arg[2] = param_get64ex(Cmd, 4, 0, 0); //tag mode
		} break;
		case 04: { //RHTSF_KEY BLOCK
			c = (UsbCommand){ CMD_READ_HITAG_S_BLK };
			num_to_bytes(param_get64ex(Cmd, 1, 0, 16), 6, htd->crypto.key);
			c.arg[1] = param_get64ex(Cmd, 2, 0, 0); //firstpage
			c.arg[2] = param_get64ex(Cmd, 3, 0, 0); //tag mode
		} break;
		case RHT2F_PASSWORD: {
			num_to_bytes(param_get32ex(Cmd, 1, 0, 16), 4, htd->pwd.password);
		} break;
		case RHT2F_AUTHENTICATE: {
			num_to_bytes(param_get32ex(Cmd, 1, 0, 16), 4, htd->auth.NrAr);
			num_to_bytes(param_get32ex(Cmd, 2, 0, 16), 4, htd->auth.NrAr+4);
		} break;
		case RHT2F_CRYPTO: {
			num_to_bytes(param_get64ex(Cmd, 1, 0, 16), 6, htd->crypto.key);
			//          num_to_bytes(param_get32ex(Cmd,2,0,16),4,htd->auth.NrAr+4);
		} break;
		case RHT2F_TEST_AUTH_ATTEMPTS: {
			// No additional parameters needed
		} break;
		case RHT2F_UID_ONLY: {
			// No additional parameters needed
		} break;
		default: {
			PrintAndLog("\nError: unkown reader function %d",htf);
			PrintAndLog("");
			PrintAndLog("Usage: hitag reader <Reader Function #>");
			PrintAndLog("Reader Functions:");
			PrintAndLog("  HitagS (0*):");
			PrintAndLog("    01 <nr> <ar> (Challenge) <firstPage> <tagmode> read all pages from a Hitag S tag");
			PrintAndLog("    02 <key> (set to 0 if no authentication is needed) <firstPage> <tagmode> read all pages from a Hitag S tag");
			PrintAndLog("    03 <nr> <ar> (Challenge) <firstPage> <tagmode> read all blocks from a Hitag S tag");
			PrintAndLog("    04 <key> (set to 0 if no authentication is needed) <firstPage> <tagmode> read all blocks from a Hitag S tag");
			PrintAndLog("    Valid tagmodes are 0=STANDARD, 1=ADVANCED, 2=FAST_ADVANCED (default is ADVANCED)");
			PrintAndLog("  Hitag1 (1*):");
			PrintAndLog("    (not yet implemented)");
			PrintAndLog("  Hitag2 (2*):");
			PrintAndLog("    21 <password> (password mode)");
			PrintAndLog("    22 <nr> <ar> (authentication)");
			PrintAndLog("    23 <key> (authentication) key is in format: ISK high + ISK low");
			PrintAndLog("    25 (test recorded authentications)");
			PrintAndLog("    26 just read UID");
			return 1;
		} break;
	}

	// Copy the hitag2 function into the first argument
	c.arg[0] = htf;

	// Send the command to the proxmark
	clearCommandBuffer();
	SendCommand(&c);

	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4000)) {
		PrintAndLogEx(WARNING, "timeout while waiting for reply.");
		return 1;
	}

	// Check the return status, stored in the first argument
	if (resp.arg[0] == false)  {
		PrintAndLogEx(DEBUG, "DEBUG: Error - hitag failed");
		return 1;
	}

	uint32_t id = bytes_to_num(resp.d.asBytes,4);

	if (htf == RHT2F_UID_ONLY){
		PrintAndLog("Valid Hitag2 tag found - UID: %08x",id);
	} else {
		char filename[256];
		FILE* pf = NULL;

		sprintf(filename,"%08x_%04x.ht2",id,(rand() & 0xffff));
		if ((pf = fopen(filename,"wb")) == NULL) {
		  PrintAndLog("Error: Could not open file [%s]",filename);
		  return 1;
		}

		// Write the 48 tag memory bytes to file and finalize
		fwrite(resp.d.asBytes,1,48,pf);
		fclose(pf);

		PrintAndLog("Succesfully saved tag memory to [%s]",filename);
	}

	return 0;
}


static int CmdLFHitagSimS(const char *Cmd) {
	UsbCommand c = { CMD_SIMULATE_HITAG_S };
	char filename[FILE_PATH_SIZE] = { 0x00 };
	FILE* pf;
	bool tag_mem_supplied;
	int len = strlen(Cmd);
	if (len > FILE_PATH_SIZE)
		len = FILE_PATH_SIZE;
	memcpy(filename, Cmd, len);

	if (strlen(filename) > 0) {
		if ((pf = fopen(filename, "rb+")) == NULL) {
			PrintAndLog("Error: Could not open file [%s]", filename);
			return 1;
		}
		tag_mem_supplied = true;
		if (fread(c.d.asBytes, 1, 4*64, pf) != 4*64) {
			PrintAndLog("Error: File reading error");
			fclose(pf);
			return 1;
		}
		fclose(pf);
	} else {
		tag_mem_supplied = false;
	}

	// Does the tag comes with memory
	c.arg[0] = (uint32_t) tag_mem_supplied;

	SendCommand(&c);
	return 0;
}


static int CmdLFHitagCheckChallenges(const char *Cmd) {
	UsbCommand c = { CMD_TEST_HITAGS_TRACES };
	char filename[FILE_PATH_SIZE] = { 0x00 };
	FILE* pf;
	bool file_given;
	int len = strlen(Cmd);
	if (len > FILE_PATH_SIZE) len = FILE_PATH_SIZE;
	memcpy(filename, Cmd, len);

	if (strlen(filename) > 0) {
		if ((pf = fopen(filename,"rb+")) == NULL) {
			PrintAndLog("Error: Could not open file [%s]",filename);
			return 1;
		}
		file_given = true;
		if (fread(c.d.asBytes,1,8*60,pf) != 8*60) {
			PrintAndLog("Error: File reading error");
			fclose(pf);
			return 1;
		}
		fclose(pf);
	} else {
		file_given = false;
	}

	//file with all the challenges to try
	c.arg[0] = (uint32_t)file_given;
	c.arg[1] = param_get64ex(Cmd,2,0,0); //get mode

	SendCommand(&c);
	return 0;
}


static int CmdLFHitagWriter(const char *Cmd) {
	UsbCommand c = { CMD_WR_HITAG_S };
	hitag_data* htd = (hitag_data*)c.d.asBytes;
	hitag_function htf = param_get32ex(Cmd,0,0,10);
	switch (htf) {
		case WHTSF_CHALLENGE: {
			num_to_bytes(param_get64ex(Cmd,1,0,16),8,htd->auth.NrAr);
			c.arg[2]= param_get32ex(Cmd, 2, 0, 10);
			num_to_bytes(param_get32ex(Cmd,3,0,16),4,htd->auth.data);
		} break;
		case WHTSF_KEY:
		case WHT2F_CRYPTO: {
			num_to_bytes(param_get64ex(Cmd,1,0,16),6,htd->crypto.key);
			c.arg[2]= param_get32ex(Cmd, 2, 0, 10);
			num_to_bytes(param_get32ex(Cmd,3,0,16),4,htd->crypto.data);
		} break;
		case WHT2F_PASSWORD: {
			num_to_bytes(param_get64ex(Cmd, 1, 0, 16), 4, htd->pwd.password);
			c.arg[2] = param_get32ex(Cmd, 2, 0, 10);
			num_to_bytes(param_get32ex(Cmd, 3, 0, 16), 4, htd->crypto.data);
		} break;
		default: {
			PrintAndLog("Error: unkown writer function %d",htf);
			PrintAndLog("Hitag writer functions");
			PrintAndLog("  HitagS (0*):");
			PrintAndLog("    03 <nr,ar> (Challenge) <page> <byte0...byte3> write page on a Hitag S tag");
			PrintAndLog("    04 <key> (set to 0 if no authentication is needed) <page> <byte0...byte3> write page on a Hitag S tag");
			PrintAndLog("  Hitag1 (1*)");
			PrintAndLog("    (not yet implemented)");
			PrintAndLog("  Hitag2 (2*):");
			PrintAndLog("    24 <key> (set to 0 if no authentication is needed) <page> <byte0...byte3> write page on a Hitag S tag");
			PrintAndLog("    27 <password> <page> <byte0...byte3> write page on a Hitag2 tag");
			return 1;
		} break;
	}
	// Copy the hitag function into the first argument
	c.arg[0] = htf;

	// Send the command to the proxmark
	SendCommand(&c);

	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4000)) {
		PrintAndLogEx(WARNING, "timeout while waiting for reply.");
		return 1;
	}

	// Check the return status, stored in the first argument
	if (resp.arg[0] == false) {
		PrintAndLogEx(DEBUG, "DEBUG: Error - hitag write failed");
		return 1;
	}
	return 0;
}


static int CmdHelp(const char *Cmd);

static command_t CommandTable[] =
{
	{"help",             CmdHelp,                   1, "This help"},
	{"list",             CmdLFHitagList,            0, "<outfile> List Hitag trace history"},
	{"info",             CmdLFHitagInfo,            0, "Tag information" },
	{"reader",           CmdLFHitagReader,          0, "Act like a Hitag Reader"},
	{"sim",              CmdLFHitagSim,             0, "Simulate Hitag transponder"},
	{"snoop",            CmdLFHitagSnoop,           0, "Eavesdrop Hitag communication"},
	{"writer",           CmdLFHitagWriter,          0, "Act like a Hitag Writer" },
	{"simS",             CmdLFHitagSimS,            0, "Simulate HitagS transponder" },
	{"checkChallenges",  CmdLFHitagCheckChallenges, 0, "Test challenges from a file" },
	{ NULL,              NULL,                      0, NULL }
};


static int CmdHelp(const char *Cmd) {
	CmdsHelp(CommandTable);
	return 0;
}


int CmdLFHitag(const char *Cmd) {
	CmdsParse(CommandTable, Cmd);
	return 0;
}
