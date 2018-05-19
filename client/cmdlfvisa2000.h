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
extern int CmdLFVisa2k(const char *Cmd);
extern int CmdVisa2kClone(const char *Cmd);
extern int CmdVisa2kSim(const char *Cmd);
extern int CmdVisa2kRead(const char *Cmd);
extern int CmdVisa2kDemod(const char *Cmd);

#endif

