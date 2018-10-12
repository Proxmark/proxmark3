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

static int CmdHelp(const char *Cmd);

int CmdHFMFPInfo(const char *cmd) {
	// info about 14a part
	CmdHF14AInfo("");
	
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
				uint8_t cmd[16 + 5] = {0x0a, 0x00, 0xa8, 0x90, 0x90, 0x00};
				int res = ExchangeRAW14a(cmd, sizeof(cmd), false, false, data, sizeof(data), &datalen);
				if (!res && datalen > 2 && data[2] == 0x09) {
					SLmode = 0;
				}
			}
		}
		
		if (SLmode != 0xff)
			PrintAndLog("Mifare Plus SL mode: SL%d", SLmode);
		else
			PrintAndLog("Mifare Plus SL mode: unknown(");
	};
	
	DropField();
	
	return 0;
}

static command_t CommandTable[] =
{
  {"help",             CmdHelp,					1, "This help"},
  {"info",  	       CmdHFMFPInfo,			0, "Info about Mifare Plus tag"},
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