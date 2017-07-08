//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// This module handles the main entry point into the command parser for the
// proxmark3 CLI.
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cmdparser.h"
#include "proxmark3.h"
#include "data.h"
#include "usb_cmd.h"
#include "ui.h"
#include "cmdhf.h"
#include "cmddata.h"
#include "cmdhw.h"
#include "cmdlf.h"
#include "cmdmain.h"
#include "util.h"
#include "util_posix.h"
#include "cmdscript.h"
#include "cmdcrc.h"

static int CmdHelp(pm3_connection* conn, const char *Cmd);
static int CmdQuit(pm3_connection* conn, const char *Cmd);
static int CmdRev(pm3_connection* conn, const char *Cmd);

static command_t CommandTable[] = 
{
  {"help",  CmdHelp,  1, "This help. Use '<command> help' for details of a particular command."},
  {"data",  CmdData,  1, "{ Plot window / data buffer manipulation... }"},
  {"hf",    CmdHF,    1, "{ High Frequency commands... }"},
  {"hw",    CmdHW,    1, "{ Hardware commands... }"},
  {"lf",    CmdLF,    1, "{ Low Frequency commands... }"},
  {"reveng",CmdRev,   1, "Crc calculations from the software reveng1-30"},
  {"script",CmdScript,1, "{ Scripting commands }"},
  {"quit",  CmdQuit,  1, "Exit program"},
  {"exit",  CmdQuit,  1, "Exit program"},
  {NULL, NULL, 0, NULL}
};

command_t* getTopLevelCommandTable()
{
  return CommandTable;
}

int CmdHelp(pm3_connection* conn, const char *Cmd)
{
  CmdsHelp(conn, CommandTable);
  return 0;
}

int CmdQuit(pm3_connection* conn, const char *Cmd)
{
  return 99;
}

int CmdRev(pm3_connection* conn, const char *Cmd)
{
  CmdCrc(conn, Cmd);
  return 0;
}

//-----------------------------------------------------------------------------
// Entry point into our code: called whenever the user types a command and
// then presses Enter, which the full command line that they typed.
//-----------------------------------------------------------------------------
int CommandReceived(pm3_connection* conn, char *Cmd) {
	return CmdsParse(conn, CommandTable, Cmd);
}

