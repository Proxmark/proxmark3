//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency viking tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFVIKING_H__
#define CMDLFVIKING_H__

#include "comms.h"

extern int CmdLFViking(pm3_connection* conn, const char *Cmd);
extern int CmdVikingDemod(pm3_connection* conn, const char *Cmd);
extern int CmdVikingRead(pm3_connection* conn, const char *Cmd);
extern int CmdVikingClone(pm3_connection* conn, const char *Cmd);
extern int CmdVikingSim(pm3_connection* conn, const char *Cmd);
#endif
