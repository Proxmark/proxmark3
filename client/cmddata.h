//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Data and Graph commands
//-----------------------------------------------------------------------------

#ifndef CMDDATA_H__
#define CMDDATA_H__

#include <stdlib.h>  //size_t
#include <stdint.h>  //uint_32+
#include <stdbool.h> //bool

#include "cmdparser.h" // for command_t

command_t * CmdDataCommands();

int CmdData(pm3_connection* conn, const char *Cmd);
void printDemodBuff(pm3_connection* conn);
void setDemodBuf(pm3_connection* conn, uint8_t *buff, size_t size, size_t startIdx);
bool getDemodBuf(pm3_connection* conn, uint8_t *buff, size_t *size);
// option '1' to save DemodBuffer any other to restore
void save_restoreDB(pm3_connection* conn, uint8_t saveOpt);

int CmdPrintDemodBuff(pm3_connection* conn, const char *Cmd);
int Cmdaskrawdemod(pm3_connection* conn, const char *Cmd);
int Cmdaskmandemod(pm3_connection* conn, const char *Cmd);
int AutoCorrelate(pm3_connection* conn, const int *in, int *out, size_t len, int window, bool SaveGrph, bool verbose);
int CmdAutoCorr(pm3_connection* conn, const char *Cmd);
int CmdBiphaseDecodeRaw(pm3_connection* conn, const char *Cmd);
int CmdBitsamples(pm3_connection* conn, const char *Cmd);
int CmdBuffClear(pm3_connection* conn, const char *Cmd);
int CmdDec(pm3_connection* conn, const char *Cmd);
int CmdDetectClockRate(pm3_connection* conn, const char *Cmd);
int CmdFSKrawdemod(pm3_connection* conn, const char *Cmd);
int CmdPSK1rawDemod(pm3_connection* conn, const char *Cmd);
int CmdPSK2rawDemod(pm3_connection* conn, const char *Cmd);
int CmdGrid(pm3_connection* conn, const char *Cmd);
int CmdGetBitStream(pm3_connection* conn, const char *Cmd);
int CmdHexsamples(pm3_connection* conn, const char *Cmd);
int CmdHide(pm3_connection* conn, const char *Cmd);
int CmdHpf(pm3_connection* conn, const char *Cmd);
int CmdLoad(pm3_connection* conn, const char *Cmd);
int CmdLtrim(pm3_connection* conn, const char *Cmd);
int CmdRtrim(pm3_connection* conn, const char *Cmd);
int Cmdmandecoderaw(pm3_connection* conn, const char *Cmd);
int CmdNorm(pm3_connection* conn, const char *Cmd);
int CmdNRZrawDemod(pm3_connection* conn, const char *Cmd);
int CmdPlot(pm3_connection* conn, const char *Cmd);
int CmdPrintDemodBuff(pm3_connection* conn, const char *Cmd);
int CmdRawDemod(pm3_connection* conn, const char *Cmd);
int CmdSamples(pm3_connection* conn, const char *Cmd);
int CmdTuneSamples(pm3_connection* conn, const char *Cmd);
int CmdSave(pm3_connection* conn, const char *Cmd);
int CmdScale(pm3_connection* conn, const char *Cmd);
int CmdDirectionalThreshold(pm3_connection* conn, const char *Cmd);
int CmdZerocrossings(pm3_connection* conn, const char *Cmd);
int ASKbiphaseDemod(pm3_connection* conn, const char *Cmd, bool verbose);
int ASKDemod(pm3_connection* conn, const char *Cmd, bool verbose, bool emSearch, uint8_t askType);
int ASKDemod_ext(pm3_connection* conn, const char *Cmd, bool verbose, bool emSearch, uint8_t askType, bool *stCheck);
int FSKrawDemod(pm3_connection* conn, const char *Cmd, bool verbose);
int PSKDemod(pm3_connection* conn, const char *Cmd, bool verbose);
int NRZrawDemod(pm3_connection* conn, const char *Cmd, bool verbose);
int getSamples(pm3_connection* conn, int n, bool silent);
void setClockGrid(pm3_connection* conn, int clk, int offset);
int directionalThreshold(const int* in, int *out, size_t len, int8_t up, int8_t down);
extern int AskEdgeDetect(const int *in, int *out, int len, int threshold);
//int autoCorr(const int* in, int *out, size_t len, int window);

#endif
