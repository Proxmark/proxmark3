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

int CmdHFiClass(pm3_connection* conn, const char *Cmd);

int CmdHFiClassCalcNewKey(pm3_connection* conn, const char *Cmd);
int CmdHFiClassCloneTag(pm3_connection* conn, const char *Cmd);
int CmdHFiClassDecrypt(pm3_connection* conn, const char *Cmd);
int CmdHFiClassEncryptBlk(pm3_connection* conn, const char *Cmd);
int CmdHFiClassELoad(pm3_connection* conn, const char *Cmd);
int CmdHFiClassList(pm3_connection* conn, const char *Cmd);
int HFiClassReader(pm3_connection* conn, const char *Cmd, bool loop, bool verbose);
int CmdHFiClassReader(pm3_connection* conn, const char *Cmd);
int CmdHFiClassReader_Dump(pm3_connection* conn, const char *Cmd);
int CmdHFiClassReader_Replay(pm3_connection* conn, const char *Cmd);
int CmdHFiClassReadKeyFile(pm3_connection* conn, const char *filename);
int CmdHFiClassReadTagFile(pm3_connection* conn, const char *Cmd);
int CmdHFiClass_ReadBlock(pm3_connection* conn, const char *Cmd);
int CmdHFiClass_TestMac(pm3_connection* conn, const char *Cmd);
int CmdHFiClassManageKeys(pm3_connection* conn, const char *Cmd);
int CmdHFiClass_loclass(pm3_connection* conn, const char *Cmd);
int CmdHFiClassSnoop(pm3_connection* conn, const char *Cmd);
int CmdHFiClassSim(pm3_connection* conn, const char *Cmd);
int CmdHFiClassWriteKeyFile(pm3_connection* conn, const char *Cmd);
int CmdHFiClass_WriteBlock(pm3_connection* conn, const char *Cmd);
void printIclassDumpContents(uint8_t *iclass_dump, uint8_t startblock, uint8_t endblock, size_t filesize);
void HFiClassCalcDivKey(uint8_t	*CSN, uint8_t	*KEY, uint8_t *div_key, bool elite);
#endif
