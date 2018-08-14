//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Main command parser entry point
//-----------------------------------------------------------------------------

#include "cmdmain.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cmdparser.h"
#include "proxmark3.h"
#include "usb_cmd.h"
#include "ui.h"
#include "cmdhf.h"
#include "cmddata.h"
#include "cmdhw.h"
#include "cmdlf.h"
#include "util.h"
#include "util_posix.h"
#include "cmdscript.h"


static int CmdHelp(const char *Cmd);
static int CmdQuit(const char *Cmd);


static command_t CommandTable[] = 
{
  {"help",  CmdHelp,  1, "This help. Use '<command> help' for details of a particular command."},
  {"data",  CmdData,  1, "{ Plot window / data buffer manipulation... }"},
  {"hf",    CmdHF,    1, "{ High Frequency commands... }"},
  {"hw",    CmdHW,    1, "{ Hardware commands... }"},
  {"lf",    CmdLF,    1, "{ Low Frequency commands... }"},
  {"script",CmdScript,1, "{ Scripting commands }"},
  {"quit",  CmdQuit,  1, "Exit program"},
  {"exit",  CmdQuit,  1, "Exit program"},
  {NULL, NULL, 0, NULL}
};

command_t* getTopLevelCommandTable()
{
  return CommandTable;
}

static int CmdHelp(const char *Cmd)
{
  CmdsHelp(CommandTable);
  return 0;
}

static int CmdQuit(const char *Cmd)
{
  return 99;
}

//-----------------------------------------------------------------------------
// Entry point into our code: called whenever the user types a command and
// then presses Enter, which the full command line that they typed.
//-----------------------------------------------------------------------------
int CommandReceived(char *Cmd) {
	return CmdsParse(CommandTable, Cmd);
}

