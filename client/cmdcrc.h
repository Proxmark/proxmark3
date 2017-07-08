//-----------------------------------------------------------------------------
// Copyright (C) 2015 iceman <iceman at iuse.se>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// CRC Calculations from the software reveng commands
//-----------------------------------------------------------------------------

#ifndef CMDCRC_H__
#define CMDCRC_H__

#include "comms.h"

int CmdCrc(pm3_connection* conn, const char *Cmd);
int CmdrevengTest(pm3_connection* conn, const char *Cmd);
int CmdrevengTestC(pm3_connection* conn, const char *Cmd);
int CmdrevengSearch(pm3_connection* conn, const char *Cmd);
int GetModels(char *Models[], int *count, uint8_t *width);
int RunModel(char *inModel, char *inHexStr, bool reverse, char endian, char *result);
#endif
