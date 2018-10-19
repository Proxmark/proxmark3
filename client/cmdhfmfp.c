//-----------------------------------------------------------------------------
// Copyright (C) 2018 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency MIFARE  Plus commands
//-----------------------------------------------------------------------------

#include "cmdhfmfp.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "comms.h"
#include "cmdmain.h"
#include "util.h"
#include "ui.h"
#include "cmdhf14a.h"
#include "mifare.h"
#include "cliparser/cliparser.h"

static const uint8_t DefaultKey[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

typedef struct {
	uint8_t Code;
	const char *Description;
} PlusErrorsElm;

static const PlusErrorsElm PlusErrors[] = {
	{0xFF, ""},
	{0x00, "Unknown error"},
	{0x09, "Invalid block number"},
	{0x0b, "Command code error"},
	{0x0c, "Length error"},
	{0x90, "OK"},
};
int PlusErrorsLen = sizeof(PlusErrors) / sizeof(PlusErrorsElm);

const char * GetErrorDescription(uint8_t errorCode) {
	for(int i = 0; i < PlusErrorsLen; i++)
		if (errorCode == PlusErrors[i].Code)
			return PlusErrors[i].Description;
		
	return PlusErrors[0].Description;
}

static int CmdHelp(const char *Cmd);

static bool VerboseMode = false;
void SetVerboseMode(bool verbose) {
	VerboseMode = verbose;
}

int intExchangeRAW14aPlus(uint8_t *datain, int datainlen, bool activateField, bool leaveSignalON, uint8_t *dataout, int maxdataoutlen, int *dataoutlen) {
	if(VerboseMode)
		PrintAndLog(">>> %s", sprint_hex(datain, datainlen));
	
	int res = ExchangeRAW14a(datain, datainlen, activateField, leaveSignalON, dataout, maxdataoutlen, dataoutlen);

	if(VerboseMode)
		PrintAndLog("<<< %s", sprint_hex(dataout, *dataoutlen));
	
	return res;
}

int MFPWritePerso(uint8_t *keyNum, uint8_t *key, bool activateField, bool leaveSignalON, uint8_t *dataout, int maxdataoutlen, int *dataoutlen) {
	uint8_t rcmd[3 + 16] = {0xa8, keyNum[1], keyNum[0], 0x00};
	memmove(&rcmd[3], key, 16);
	
	return intExchangeRAW14aPlus(rcmd, sizeof(rcmd), activateField, leaveSignalON, dataout, maxdataoutlen, dataoutlen);
}

int MFPCommitPerso(bool activateField, bool leaveSignalON, uint8_t *dataout, int maxdataoutlen, int *dataoutlen) {
	uint8_t rcmd[1] = {0xaa};
	
	return intExchangeRAW14aPlus(rcmd, sizeof(rcmd), activateField, leaveSignalON, dataout, maxdataoutlen, dataoutlen);
}

int CmdHFMFPInfo(const char *cmd) {
	
	if (cmd && strlen(cmd) > 0)
		PrintAndLog("WARNING: command don't have any parameters.\n");
	
	// info about 14a part
	CmdHF14AInfo("");

	// Mifare Plus info
	UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_CONNECT | ISO14A_NO_DISCONNECT, 0, 0}};
	SendCommand(&c);

	UsbCommand resp;
	WaitForResponse(CMD_ACK,&resp);
	
	iso14a_card_select_t card;
	memcpy(&card, (iso14a_card_select_t *)resp.d.asBytes, sizeof(iso14a_card_select_t));

	uint64_t select_status = resp.arg[0];		// 0: couldn't read, 1: OK, with ATS, 2: OK, no ATS, 3: proprietary Anticollision
	
	if (select_status == 1 || select_status == 2) {
		PrintAndLog("----------------------------------------------");
		PrintAndLog("Mifare Plus info:");
		
		// MIFARE Type Identification Procedure
		// https://www.nxp.com/docs/en/application-note/AN10833.pdf
		uint16_t ATQA = card.atqa[0] + (card.atqa[1] << 8);
		if (ATQA == 0x0004) PrintAndLog("ATQA: Mifare Plus 2k 4bUID");
		if (ATQA == 0x0002) PrintAndLog("ATQA: Mifare Plus 4k 4bUID");
		if (ATQA == 0x0044) PrintAndLog("ATQA: Mifare Plus 2k 7bUID");
		if (ATQA == 0x0042) PrintAndLog("ATQA: Mifare Plus 4k 7bUID");
		
		uint8_t SLmode = 0xff;
		if (card.sak == 0x08) {
			PrintAndLog("SAK: Mifare Plus 2k 7bUID");
			if (select_status == 2) SLmode = 1;
		}
		if (card.sak == 0x18) {
			PrintAndLog("SAK: Mifare Plus 4k 7bUID");
			if (select_status == 2) SLmode = 1;
		}
		if (card.sak == 0x10) {
			PrintAndLog("SAK: Mifare Plus 2k");
			if (select_status == 2) SLmode = 2;
		}
		if (card.sak == 0x11) {
			PrintAndLog("SAK: Mifare Plus 4k");
			if (select_status == 2) SLmode = 2;
		}
		if (card.sak == 0x20) {
			PrintAndLog("SAK: Mifare Plus SL0/SL3 or Mifare desfire");
			if (card.ats_len > 0) {
				SLmode = 3;

				// check SL0
				uint8_t data[250] = {0};
				int datalen = 0;
				// https://github.com/Proxmark/proxmark3/blob/master/client/scripts/mifarePlus.lua#L161
				uint8_t cmd[3 + 16] = {0xa8, 0x90, 0x90, 0x00};
				int res = ExchangeRAW14a(cmd, sizeof(cmd), false, false, data, sizeof(data), &datalen);
				if (!res && datalen > 1 && data[0] == 0x09) {
					SLmode = 0;
				}
			}
		}
		
		if (SLmode != 0xff)
			PrintAndLog("Mifare Plus SL mode: SL%d", SLmode);
		else
			PrintAndLog("Mifare Plus SL mode: unknown(");
	} else {
		PrintAndLog("Mifare Plus info not available.");
	}
	
	DropField();
	
	return 0;
}

int CmdHFMFPWritePerso(const char *cmd) {
	uint8_t keyNum[64] = {0};
	int keyNumLen = 0;
	uint8_t key[64] = {0};
	int keyLen = 0;

	CLIParserInit("hf mfp wrp", 
		"Executes Write Perso command. Can be used in SL0 mode only.", 
		"Usage:\n\thf mfp wrp 4000 000102030405060708090a0b0c0d0e0f -> write key (00..0f) to key number 4000 \n"
			"\thf mfp wrp 4000 -> write default key(0xff..0xff) to key number 4000");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("vV",  "verbose", "show internal data."),
		arg_str1(NULL,  NULL,      "<HEX key number (2b)>", NULL),
		arg_strx0(NULL,  NULL,     "<HEX key (16b)>", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool verbose = arg_get_lit(1);
	CLIGetHexWithReturn(2, keyNum, &keyNumLen);
	CLIGetHexWithReturn(3, key, &keyLen);
	CLIParserFree();
	
	SetVerboseMode(verbose);
	
	if (!keyLen) {
		memmove(key, DefaultKey, 16);
		keyLen = 16;
	}
	
	if (keyNumLen != 2) {
		PrintAndLog("Key number length must be 2 bytes instead of: %d", keyNumLen);
		return 1;
	}
	if (keyLen != 16) {
		PrintAndLog("Key length must be 16 bytes instead of: %d", keyLen);
		return 1;
	}

	uint8_t data[250] = {0};
	int datalen = 0;

	int res = MFPWritePerso(keyNum, key, true, false, data, sizeof(data), &datalen);
	if (res) {
		PrintAndLog("Exchange error: %d", res);
		return res;
	}
	
	if (datalen != 3) {
		PrintAndLog("Command must return 3 bytes instead of: %d", datalen);
		return 1;
	}

	if (data[0] != 0x90) {
		PrintAndLog("Command error: %02x %s", data[0], GetErrorDescription(data[0]));
		return 1;
	}
	PrintAndLog("Write OK.");
	
	return 0;
}

uint16_t CardAddresses[] = {0x9000, 0x9001, 0x9003, 0x9004, 0xA000, 0xA001, 0xA080, 0xA081, 0xC000, 0xC001};

int CmdHFMFPInitPerso(const char *cmd) {
	int res;
	uint8_t key[256] = {0};
	int keyLen = 0;
	uint8_t keyNum[2] = {0};
	uint8_t data[250] = {0};
	int datalen = 0;

	CLIParserInit("hf mfp initp", 
		"Executes Write Perso command for all card's keys. Can be used in SL0 mode only.", 
		"Usage:\n\thf mfp initp 000102030405060708090a0b0c0d0e0f -> fill all the keys with key (00..0f)\n"
			"\thf mfp initp -vv -> fill all the keys with default key(0xff..0xff) and show all the data exchange");

	void* argtable[] = {
		arg_param_begin,
		arg_litn("vV",  "verbose", 0, 2, "show internal data."),
		arg_strx0(NULL,  NULL,      "<HEX key (16b)>", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool verbose = arg_get_lit(1);
	bool verbose2 = arg_get_lit(1) > 1;
	CLIGetHexWithReturn(2, key, &keyLen);
	CLIParserFree();

	if (keyLen && keyLen != 16) {
		PrintAndLog("Key length must be 16 bytes instead of: %d", keyLen);
		return 1;
	}
	
	if (!keyLen)
		memmove(key, DefaultKey, 16);

	SetVerboseMode(verbose2);
	for (uint16_t sn = 0x4000; sn < 0x4050; sn++) {
		keyNum[0] = sn >> 8;
		keyNum[1] = sn & 0xff;
		res = MFPWritePerso(keyNum, key, (sn == 0x4000), true, data, sizeof(data), &datalen);
		if (!res && (datalen == 3) && data[0] == 0x09) {
			PrintAndLog("2k card detected.");
			break;
		}
		if (res || (datalen != 3) || data[0] != 0x90) {
			PrintAndLog("Write error on address %04x", sn);
			break;
		}
	}
	
	SetVerboseMode(verbose);
	for (int i = 0; i < sizeof(CardAddresses) / 2; i++) {
		keyNum[0] = CardAddresses[i] >> 8;
		keyNum[1] = CardAddresses[i] & 0xff;
		res = MFPWritePerso(keyNum, key, false, true, data, sizeof(data), &datalen);
		if (!res && (datalen == 3) && data[0] == 0x09) {
			PrintAndLog("Skipped[%04x]...", CardAddresses[i]);
		} else {
			if (res || (datalen != 3) || data[0] != 0x90) {
				PrintAndLog("Write error on address %04x", CardAddresses[i]);
				break;
			}
		}
	}
	
	DropField();
	
	if (res)
		return res;
	
	PrintAndLog("Done.");
	
	return 0;
}

int CmdHFMFPCommitPerso(const char *cmd) {
	CLIParserInit("hf mfp commitp", 
		"Executes Commit Perso command. Can be used in SL0 mode only.", 
		"Usage:\n\thf mfp commitp ->  \n");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("vV",  "verbose", "show internal data."),
		arg_int0(NULL,  NULL,      "SL mode", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool verbose = arg_get_lit(1);
	CLIParserFree();
	
	SetVerboseMode(verbose);
	
	uint8_t data[250] = {0};
	int datalen = 0;

	int res = MFPCommitPerso(true, false, data, sizeof(data), &datalen);
	if (res) {
		PrintAndLog("Exchange error: %d", res);
		return res;
	}
	
	if (datalen != 3) {
		PrintAndLog("Command must return 3 bytes instead of: %d", datalen);
		return 1;
	}

	if (data[0] != 0x90) {
		PrintAndLog("Command error: %02x %s", data[0], GetErrorDescription(data[0]));
		return 1;
	}
	PrintAndLog("Switch level OK.");

	return 0;
}

static command_t CommandTable[] =
{
  {"help",             CmdHelp,					1, "This help"},
  {"info",  	       CmdHFMFPInfo,			0, "Info about Mifare Plus tag"},
  {"wrp",	  	       CmdHFMFPWritePerso,		0, "Write Perso command"},
  {"initp",  	       CmdHFMFPInitPerso,		0, "Fills all the card's keys"},
  {"commitp",  	       CmdHFMFPCommitPerso,		0, "Move card to SL1 or SL3 mode"},
  {NULL,               NULL,					0, NULL}
};

int CmdHFMFP(const char *Cmd) {
	(void)WaitForResponseTimeout(CMD_ACK,NULL,100);
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd) {
  CmdsHelp(CommandTable);
  return 0;
}
