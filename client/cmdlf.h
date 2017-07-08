//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency commands
//-----------------------------------------------------------------------------

#ifndef CMDLF_H__
#define CMDLF_H__

#include <stdbool.h>
#include <stdint.h>
#include "comms.h"

extern int CmdLF(pm3_connection* conn, const char *Cmd);

extern int CmdLFCommandRead(pm3_connection* conn, const char *Cmd);
extern int CmdFlexdemod(pm3_connection* conn, const char *Cmd);
extern int CmdIndalaDemod(pm3_connection* conn, const char *Cmd);
extern int CmdIndalaClone(pm3_connection* conn, const char *Cmd);
extern int CmdLFRead(pm3_connection* conn, const char *Cmd);
extern int CmdLFSim(pm3_connection* conn, const char *Cmd);
extern int CmdLFaskSim(pm3_connection* conn, const char *Cmd);
extern int CmdLFfskSim(pm3_connection* conn, const char *Cmd);
extern int CmdLFpskSim(pm3_connection* conn, const char *Cmd);
extern int CmdLFSimBidir(pm3_connection* conn, const char *Cmd);
extern int CmdLFSnoop(pm3_connection* conn, const char *Cmd);
extern int CmdVchDemod(pm3_connection* conn, const char *Cmd);
extern int CmdLFfind(pm3_connection* conn, const char *Cmd);
extern bool lf_read(pm3_connection* conn, bool silent, uint32_t samples);

#endif
