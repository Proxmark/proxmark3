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

#include <stdint.h>
#include <stdbool.h>

int CmdLFHID(const char *Cmd);
int CmdFSKdemodHID(const char *Cmd);
int CmdHIDReadDemod(const char *Cmd);
int CmdHIDSim(const char *Cmd);
int CmdHIDClone(const char *Cmd);
int CmdHIDDecode(const char *Cmd);
int CmdHIDEncode(const char *Cmd);
int CmdHIDWrite(const char *Cmd);
#endif
