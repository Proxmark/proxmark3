// version 2.1.0
// add readkeyfile,writekeyfile,modify dump,iso write and calcEkey functions.
//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
// Copyright (C) 2011 Gerhard de Koning Gans
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency iClass support
//-----------------------------------------------------------------------------

#ifndef CMDHFICLASS_H__
#define CMDHFICLASS_H__

int CmdHFiClass(const char *Cmd);

int CmdHFiClassSnoop(const char *Cmd);
int CmdHFiClassSim(const char *Cmd);
int CmdHFiClassList(const char *Cmd);
int HFiClassReader(const char *Cmd, bool loop, bool verbose);
int CmdHFiClassReader(const char *Cmd);
int CmdHFiClassReader_Replay(const char *Cmd);
int CmdHFiClassReadKeyFile(const char *filename);
int CmdHFiClassWriteKeyFile(const char *Cmd);
int CmdHFiClass_iso14443A_write(const char *Cmd);
int CmdHFiClassCalcEKey(const char *Cmd);
int CmdHFiClassRestore(const char *cmd);
int CmdHFiClass_TestMac(const char *Cmd);
#endif
