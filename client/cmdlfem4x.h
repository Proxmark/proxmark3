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

int CmdLFEM4X(const char *Cmd);
void printEM410x(uint32_t hi, uint64_t id);
int CmdEMdemodASK(const char *Cmd);
int CmdAskEM410xDemod(const char *Cmd);
int AskEm410xDecode(bool verbose, uint32_t *hi, uint64_t *lo );
int AskEm410xDemod(const char *Cmd, uint32_t *hi, uint64_t *lo, bool verbose);
int CmdEM410xSim(const char *Cmd);
int CmdEM410xBrute(const char *Cmd);
int CmdEM410xWatch(const char *Cmd);
int CmdEM410xWatchnSpoof(const char *Cmd);
int CmdEM410xWrite(const char *Cmd);
bool EM4x05Block0Test(uint32_t *wordData);
int CmdEM4x05info(const char *Cmd);
int EM4x05WriteWord(uint8_t addr, uint32_t data, uint32_t pwd, bool usePwd, bool swap, bool invert);
int CmdEM4x05WriteWord(const char *Cmd);
int CmdEM4x05dump(const char *Cmd);
int CmdEM4x05ReadWord(const char *Cmd);
int EM4x05ReadWord_ext(uint8_t addr, uint32_t pwd, bool usePwd, uint32_t *wordData);
int EM4x50Read(const char *Cmd, bool verbose);
int CmdEM4x50Read(const char *Cmd);


#endif
