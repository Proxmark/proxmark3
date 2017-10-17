//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency HID commands
//-----------------------------------------------------------------------------

#ifndef CMDLFHID_H__
#define CMDLFHID_H__
#include "comms.h"

int CmdLFHID(pm3_connection* conn, const char *Cmd);
int CmdFSKdemodHID(pm3_connection* conn, const char *Cmd);
int CmdHIDReadDemod(pm3_connection* conn, const char *Cmd);
int CmdHIDSim(pm3_connection* conn, const char *Cmd);
int CmdHIDClone(pm3_connection* conn, const char *Cmd);

#endif
