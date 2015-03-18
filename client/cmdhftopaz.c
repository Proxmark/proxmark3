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


