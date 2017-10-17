//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Indala commands
//-----------------------------------------------------------------------------

#ifndef CMDLFINDALA_H__
#define CMDLFINDALA_H__
#include "comms.h"

extern int CmdLFINDALA(pm3_connection* conn, const char *Cmd);
extern int CmdIndalaDecode(pm3_connection* conn, const char *Cmd);
extern int CmdIndalaRead(pm3_connection* conn, const char *Cmd);
extern int CmdIndalaClone(pm3_connection* conn, const char *Cmd);

#endif
