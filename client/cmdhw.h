//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Hardware commands
//-----------------------------------------------------------------------------

#ifndef CMDHW_H__
#define CMDHW_H__

#include "comms.h"

int CmdHW(pm3_connection *conn, const char *Cmd);

int CmdDetectReader(pm3_connection *conn, const char *Cmd);
int CmdFPGAOff(pm3_connection *conn, const char *Cmd);
int CmdLCD(pm3_connection *conn, const char *Cmd);
int CmdLCDReset(pm3_connection *conn, const char *Cmd);
int CmdReadmem(pm3_connection *conn, const char *Cmd);
int CmdReset(pm3_connection *conn, const char *Cmd);
int CmdSetDivisor(pm3_connection *conn, const char *Cmd);
int CmdSetMux(pm3_connection *conn, const char *Cmd);
int CmdTune(pm3_connection *conn, const char *Cmd);
int CmdVersion(pm3_connection *conn, const char *Cmd);

#endif
