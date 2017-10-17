//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Securakey tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFSECURAKEY_H__
#define CMDLFSECURAKEY_H__

#include "comms.h"

extern int CmdLFSecurakey(pm3_connection* conn, const char *Cmd);
extern int CmdSecurakeyClone(pm3_connection* conn, const char *Cmd);
extern int CmdSecurakeySim(pm3_connection* conn, const char *Cmd);
extern int CmdSecurakeyRead(pm3_connection* conn, const char *Cmd);
extern int CmdSecurakeyDemod(pm3_connection* conn, const char *Cmd);

#endif

