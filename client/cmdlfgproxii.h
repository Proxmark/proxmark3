//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency G Prox II tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFGPROXII_H__
#define CMDLFGPROXII_H__
#include "comms.h"

extern int CmdLF_G_Prox_II(pm3_connection* conn, const char *Cmd);
extern int CmdG_Prox_II_Demod(pm3_connection* conn, const char *Cmd);
extern int CmdG_Prox_II_Read(pm3_connection* conn, const char *Cmd);
#endif
