//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Noralsy tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFNORALSY_H__
#define CMDLFNORALSY_H__

#include "comms.h"

extern int CmdLFNoralsy(pm3_connection* conn, const char *Cmd);
extern int CmdNoralsyClone(pm3_connection* conn, const char *Cmd);
extern int CmdNoralsySim(pm3_connection* conn, const char *Cmd);
extern int CmdNoralsyRead(pm3_connection* conn, const char *Cmd);
extern int CmdNoralsyDemod(pm3_connection* conn, const char *Cmd);

#endif

