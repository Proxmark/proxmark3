//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Farpoint / Pyramid commands
//-----------------------------------------------------------------------------
#ifndef CMDLFPYRAMID_H__
#define CMDLFPYRAMID_H__

#include "comms.h"

extern int CmdLFPyramid(pm3_connection* conn, const char *Cmd);
extern int CmdPyramidClone(pm3_connection* conn, const char *Cmd);
extern int CmdPyramidSim(pm3_connection* conn, const char *Cmd);
extern int CmdFSKdemodPyramid(pm3_connection* conn, const char *Cmd);
extern int CmdPyramidRead(pm3_connection* conn, const char *Cmd);
#endif

