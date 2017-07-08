//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Securakey tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFPAC_H__
#define CMDLFPAC_H__

#include "comms.h"

extern int CmdLFPac(pm3_connection* conn, const char *Cmd);
extern int CmdPacRead(pm3_connection* conn, const char *Cmd);
extern int CmdPacDemod(pm3_connection* conn, const char *Cmd);

#endif

