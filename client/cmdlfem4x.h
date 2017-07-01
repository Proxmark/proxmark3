//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency EM4x commands
//-----------------------------------------------------------------------------

#ifndef CMDLFEM4X_H__
#define CMDLFEM4X_H__

#include <stdbool.h>    // for bool
#include <inttypes.h>

extern int CmdLFEM4X(const char *Cmd);
extern void printEM410x(uint32_t hi, uint64_t id);
extern int CmdEMdemodASK(const char *Cmd);
extern int CmdAskEM410xDemod(const char *Cmd);
extern int AskEm410xDecode(bool verbose, uint32_t *hi, uint64_t *lo );
extern int AskEm410xDemod(const char *Cmd, uint32_t *hi, uint64_t *lo, bool verbose);
extern int CmdEM410xSim(const char *Cmd);
extern int CmdEM410xBrute(const char *Cmd);
extern int CmdEM410xWatch(const char *Cmd);
extern int CmdEM410xWatchnSpoof(const char *Cmd);
extern int CmdEM410xWrite(const char *Cmd);
extern bool EM4x05Block0Test(uint32_t *wordData);
extern int CmdEM4x05info(const char *Cmd);
extern int EM4x05WriteWord(uint8_t addr, uint32_t data, uint32_t pwd, bool usePwd, bool swap, bool invert);
extern int CmdEM4x05WriteWord(const char *Cmd);
extern int CmdEM4x05dump(const char *Cmd);
extern int CmdEM4x05ReadWord(const char *Cmd);
extern int EM4x05ReadWord_ext(uint8_t addr, uint32_t pwd, bool usePwd, uint32_t *wordData);
extern int EM4x50Read(const char *Cmd, bool verbose);
extern int CmdEM4x50Read(const char *Cmd);


#endif
