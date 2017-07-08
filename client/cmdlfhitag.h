//-----------------------------------------------------------------------------
// Copyright (C) 2012 Roel Verdult
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Hitag support
//-----------------------------------------------------------------------------

#ifndef CMDLFHITAG_H__
#define CMDLFHITAG_H__
#include "comms.h"

int CmdLFHitag(pm3_connection* conn, const char *Cmd);

int CmdLFHitagList(pm3_connection* conn, const char *Cmd);
int CmdLFHitagSnoop(pm3_connection* conn, const char *Cmd);
int CmdLFHitagSim(pm3_connection* conn, const char *Cmd);
int CmdLFHitagReader(pm3_connection* conn, const char *Cmd);

#endif
