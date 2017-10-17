//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency visa 2000 commands
//-----------------------------------------------------------------------------
#ifndef CMDLFVISA2000_H__
#define CMDLFVISA2000_H__

#include <inttypes.h>
#include "comms.h"

extern int CmdLFVisa2k(pm3_connection* conn, const char *Cmd);
extern int CmdVisa2kClone(pm3_connection* conn, const char *Cmd);
extern int CmdVisa2kSim(pm3_connection* conn, const char *Cmd);
extern int CmdVisa2kRead(pm3_connection* conn, const char *Cmd);
extern int CmdVisa2kDemod(pm3_connection* conn, const char *Cmd);

#endif

