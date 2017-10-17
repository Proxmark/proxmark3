//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency Legic commands
//-----------------------------------------------------------------------------

#ifndef CMDHFLEGIC_H__
#define CMDHFLEGIC_H__

#include "comms.h"

int CmdHFLegic(pm3_connection* conn, const char *Cmd);

int CmdLegicRFRead(pm3_connection* conn, const char *Cmd);
int CmdLegicDecode(pm3_connection* conn, const char *Cmd);
int CmdLegicLoad(pm3_connection* conn, const char *Cmd);
int CmdLegicSave(pm3_connection* conn, const char *Cmd);
int CmdLegicRfSim(pm3_connection* conn, const char *Cmd);
int CmdLegicRfWrite(pm3_connection* conn, const char *Cmd);
int CmdLegicRfFill(pm3_connection* conn, const char *Cmd);

#endif
