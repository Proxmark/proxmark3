//-----------------------------------------------------------------------------
// Copyright (C) 2011 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency MIFARE commands
//-----------------------------------------------------------------------------

#ifndef CMDHFMF_H__
#define CMDHFMF_H__
#include "comms.h"

extern int CmdHFMF(pm3_connection* conn, const char *Cmd);

extern int CmdHF14AMfDbg(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfRdBl(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfURdBl(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfRdSc(pm3_connection* conn, const char* cmd);
extern int CmdHF14SMfURdCard(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfDump(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfRestore(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfWrBl(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfUWrBl(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfChk(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMifare(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfNested(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfSniff(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMf1kSim(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfEClear(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfEGet(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfESet(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfELoad(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfESave(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfECFill(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfEKeyPrn(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfCSetUID(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfCSetBlk(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfCGetBlk(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfCGetSc(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfCLoad(pm3_connection* conn, const char* cmd);
extern int CmdHF14AMfCSave(pm3_connection* conn, const char* cmd);

#endif

