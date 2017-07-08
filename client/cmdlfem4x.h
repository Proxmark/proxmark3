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
#include "comms.h"

extern int CmdLFEM4X(pm3_connection* conn, const char *Cmd);
extern void printEM410x(uint32_t hi, uint64_t id);
extern int CmdEMdemodASK(pm3_connection* conn, const char *Cmd);
extern int CmdAskEM410xDemod(pm3_connection* conn, const char *Cmd);
extern int AskEm410xDecode(pm3_connection* conn, bool verbose, uint32_t *hi, uint64_t *lo );
extern int AskEm410xDemod(pm3_connection* conn, const char *Cmd, uint32_t *hi, uint64_t *lo, bool verbose);
extern int CmdEM410xSim(pm3_connection* conn, const char *Cmd);
extern int CmdEM410xBrute(pm3_connection* conn, const char *Cmd);
extern int CmdEM410xWatch(pm3_connection* conn, const char *Cmd);
extern int CmdEM410xWatchnSpoof(pm3_connection* conn, const char *Cmd);
extern int CmdEM410xWrite(pm3_connection* conn, const char *Cmd);
extern bool EM4x05Block0Test(pm3_connection* conn, uint32_t *wordData);
extern int CmdEM4x05info(pm3_connection* conn, const char *Cmd);
extern int EM4x05WriteWord(pm3_connection* conn, uint8_t addr, uint32_t data, uint32_t pwd, bool usePwd, bool swap, bool invert);
extern int CmdEM4x05WriteWord(pm3_connection* conn, const char *Cmd);
extern int CmdEM4x05dump(pm3_connection* conn, const char *Cmd);
extern int CmdEM4x05ReadWord(pm3_connection* conn, const char *Cmd);
extern int EM4x05ReadWord_ext(pm3_connection* conn, uint8_t addr, uint32_t pwd, bool usePwd, uint32_t *wordData);
extern int EM4x50Read(pm3_connection* conn, const char *Cmd, bool verbose);
extern int CmdEM4x50Read(pm3_connection* conn, const char *Cmd);


#endif
