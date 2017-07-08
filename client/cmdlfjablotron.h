//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency jablotron tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFJABLOTRON_H__
#define CMDLFJABLOTRON_H__

#include "comms.h"

extern int CmdLFJablotron(pm3_connection* conn, const char *Cmd);
extern int CmdJablotronClone(pm3_connection* conn, const char *Cmd);
extern int CmdJablotronSim(pm3_connection* conn, const char *Cmd);
extern int CmdJablotronRead(pm3_connection* conn, const char *Cmd);
extern int CmdJablotronDemod(pm3_connection* conn, const char *Cmd);

#endif

