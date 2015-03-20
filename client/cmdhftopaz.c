//-----------------------------------------------------------------------------
// Copyright (C) 2015 Piwi
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency Topaz (NFC Type 1) commands
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cmdmain.h"
#include "cmdparser.h"
#include "cmdhftopaz.h"
#include "cmdhf14a.h"
#include "ui.h"
#include "mifare.h"
#include "proxmark3.h"
#include "iso14443crc.h"
#include "protocols.h"



static void topaz_switch_on_field(void)
{
	UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_CONNECT | ISO14A_NO_SELECT | ISO14A_NO_DISCONNECT | ISO14A_TOPAZMODE, 0, 0}};
	SendCommand(&c);

	UsbCommand resp;
	WaitForResponse(CMD_ACK, &resp);
}


static void topaz_switch_off_field(void)
{
	UsbCommand c = {CMD_READER_ISO_14443a, {0, 0, 0}};
	SendCommand(&c);

	UsbCommand resp;
	WaitForResponse(CMD_ACK, &resp);
}


static void topaz_send_cmd_raw(uint8_t *cmd, uint8_t len, uint8_t *response)
{
	UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_RAW | ISO14A_NO_DISCONNECT | ISO14A_TOPAZMODE, len, 0}};
	memcpy(c.d.asBytes, cmd, len);
	SendCommand(&c);

	UsbCommand resp;
	WaitForResponse(CMD_ACK, &resp);

	memcpy(response, resp.d.asBytes, resp.arg[0]);
}


static void topaz_send_cmd(uint8_t *cmd, uint8_t len, uint8_t *response)
{
	if (len > 1) {
        uint8_t first, second;
		ComputeCrc14443(CRC_14443_B, cmd, len, &first, &second);
        cmd[len] = first;
        cmd[len+1] = second;
	}

	topaz_send_cmd_raw(cmd, len+2, response);
}


static void topaz_select(uint8_t *atqa, uint8_t *uid)
{
	// ToDo: implement anticollision
	uint8_t rid_cmd[] = {TOPAZ_RID, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t wupa_cmd[] = {TOPAZ_WUPA};
	
	topaz_switch_on_field();
	topaz_send_cmd(wupa_cmd, sizeof(wupa_cmd), atqa);
	topaz_send_cmd(rid_cmd, sizeof(rid_cmd) - 2, uid);
	topaz_switch_off_field();
}


int CmdHFTopazReader(const char *Cmd)
{
	PrintAndLog("not yet implemented");
	return 0;
}


int CmdHFTopazSim(const char *Cmd)
{
	PrintAndLog("not yet implemented");
	return 0;
}


int CmdHFTopazCmdRaw(const char *Cmd)
{
	PrintAndLog("not yet implemented");
	return 0;
}


static int CmdHelp(const char *Cmd);


static command_t CommandTable[] = 
{
	{"help",	CmdHelp,			1, "This help"},
	{"reader",	CmdHFTopazReader,	0, "Act like a Topaz reader"},
	{"sim",		CmdHFTopazSim,		0, "<UID> -- Simulate Topaz tag"},
	{"snoop",	CmdHF14ASnoop,		0, "Eavesdrop a Topaz reader-tag communication"},
	{"raw",		CmdHFTopazCmdRaw,	0, "Send raw hex data to tag"},
	{NULL,		NULL,				0, NULL}
};


int CmdHFTopaz(const char *Cmd) {
	// flush
	WaitForResponseTimeout(CMD_ACK,NULL,100);

	// parse
	CmdsParse(CommandTable, Cmd);
	return 0;
}

static int CmdHelp(const char *Cmd)
{
	CmdsHelp(CommandTable);
	return 0;
}


