//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency TI commands
//-----------------------------------------------------------------------------

#ifndef CMDLFTI_H__
#define CMDLFTI_H__

#include "comms.h"

int CmdLFTI(pm3_connection* conn, const char *Cmd);

int CmdTIDemod(pm3_connection* conn, const char *Cmd);
int CmdTIRead(pm3_connection* conn, const char *Cmd);
int CmdTIWrite(pm3_connection* conn, const char *Cmd);

#endif
