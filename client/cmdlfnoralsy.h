//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency Noralsy tag commands
//-----------------------------------------------------------------------------
#ifndef CMDLFNORALSY_H__
#define CMDLFNORALSY_H__

int CmdLFNoralsy(const char *Cmd);
int CmdNoralsyClone(const char *Cmd);
int CmdNoralsySim(const char *Cmd);
int CmdNoralsyRead(const char *Cmd);
int CmdNoralsyDemod(const char *Cmd);

#endif

