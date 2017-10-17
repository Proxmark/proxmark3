//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency ISO15693 commands
//-----------------------------------------------------------------------------

#ifndef CMDHF15_H__
#define CMDHF15_H__

int CmdHF15(pm3_connection* conn, const char *Cmd);

int CmdHF15Demod(pm3_connection* conn, const char *Cmd);
int CmdHF15Read(pm3_connection* conn, const char *Cmd);
int HF15Reader(pm3_connection* conn, const char *Cmd, bool verbose);
int CmdHF15Reader(pm3_connection* conn, const char *Cmd);
int CmdHF15Sim(pm3_connection* conn, const char *Cmd);
int CmdHF15Record(pm3_connection* conn, const char *Cmd);
int CmdHF15Cmd(pm3_connection* conn, const char*Cmd);
int CmdHF15CmdHelp(pm3_connection* conn, const char*Cmd);
int CmdHF15Help(pm3_connection* conn, const char*Cmd);

#endif
