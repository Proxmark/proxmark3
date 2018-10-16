//-----------------------------------------------------------------------------
// Copyright (C) 2018 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency MIFARE  Plus commands
//-----------------------------------------------------------------------------
//
//  Documentation here:
//
// FIDO Alliance specifications
// https://fidoalliance.org/download/
// FIDO NFC Protocol Specification v1.0
// https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/fido-u2f-nfc-protocol-v1.2-ps-20170411.html
// FIDO U2F Raw Message Formats
// https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/fido-u2f-raw-message-formats-v1.2-ps-20170411.html
//-----------------------------------------------------------------------------


#include "cmdhffido.h"

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
	
	if (cmd && strlen(cmd) > 0)
		PrintAndLog("WARNING: command don't have any parameters.\n");
	
	// info about 14a part
	CmdHF14AInfo("");

	// FIDO info
	
	DropField();
	
	return 0;
}

static command_t CommandTable[] =
{
  {"help",             CmdHelp,					1, "This help"},
  {"info",  	       CmdHFMFPInfo,			0, "Info about FIDO tag"},
  {NULL,               NULL,					0, NULL}
};

int CmdHFFido(const char *Cmd) {
	(void)WaitForResponseTimeout(CMD_ACK,NULL,100);
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd) {
  CmdsHelp(CommandTable);
  return 0;
}
