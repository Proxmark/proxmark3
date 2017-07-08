//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Paradox tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFPARADOX_H__
#define CMDLFPARADOX_H__

#include "comms.h"

extern int CmdLFParadox(pm3_connection* conn, const char *Cmd);
extern int CmdFSKdemodParadox(pm3_connection* conn, const char *Cmd);
extern int CmdParadoxRead(pm3_connection* conn, const char *Cmd);
#endif
