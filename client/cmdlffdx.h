//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency fdx-b tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFFDX_H__
#define CMDLFFDX_H__
#include "comms.h"

extern int CmdLFFdx(pm3_connection* conn, const char *Cmd);
extern int CmdFdxClone(pm3_connection* conn, const char *Cmd);
extern int CmdFdxSim(pm3_connection* conn, const char *Cmd);
extern int CmdFdxRead(pm3_connection* conn, const char *Cmd);
extern int CmdFdxDemod(pm3_connection* conn, const char *Cmd);
#endif
