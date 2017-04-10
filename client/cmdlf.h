//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency commands
//-----------------------------------------------------------------------------

#ifndef CMDLF_H__
#define CMDLF_H__

#include <stdbool.h>
#include <stdint.h>

extern int CmdLF(const char *Cmd);

extern int CmdLFCommandRead(const char *Cmd);
extern int CmdFlexdemod(const char *Cmd);
extern int CmdIndalaDemod(const char *Cmd);
extern int CmdIndalaClone(const char *Cmd);
extern int CmdLFRead(const char *Cmd);
extern int CmdLFSim(const char *Cmd);
extern int CmdLFaskSim(const char *Cmd);
extern int CmdLFfskSim(const char *Cmd);
extern int CmdLFpskSim(const char *Cmd);
extern int CmdLFSimBidir(const char *Cmd);
extern int CmdLFSnoop(const char *Cmd);
extern int CmdVchDemod(const char *Cmd);
extern int CmdLFfind(const char *Cmd);
extern bool lf_read(bool silent, uint32_t samples);

#endif
