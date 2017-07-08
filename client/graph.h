//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Graph utilities
//-----------------------------------------------------------------------------

#ifndef GRAPH_H__
#define GRAPH_H__
#include <stdint.h>

void AppendGraph(pm3_connection* conn, int redraw, int clock, int bit);
int ClearGraph(pm3_connection* conn, int redraw);
//int DetectClock(int peak);
size_t getFromGraphBuf(pm3_connection* conn, uint8_t *buff);
int GetAskClock(pm3_connection* conn, const char str[], bool printAns, bool verbose);
int GetPskClock(pm3_connection* conn, const char str[], bool printAns, bool verbose);
uint8_t GetPskCarrier(pm3_connection* conn, const char str[], bool printAns, bool verbose);
uint8_t GetNrzClock(pm3_connection* conn, const char str[], bool printAns, bool verbose);
uint8_t GetFskClock(pm3_connection* conn, const char str[], bool printAns, bool verbose);
uint8_t fskClocks(pm3_connection* conn, uint8_t *fc1, uint8_t *fc2, uint8_t *rf1, bool verbose, int *firstClockEdge);
//uint8_t fskClocks(uint8_t *fc1, uint8_t *fc2, uint8_t *rf1, bool verbose);
bool graphJustNoise(int *BitStream, int size);
void setGraphBuf(pm3_connection* conn, uint8_t *buff, size_t size);
void save_restoreGB(pm3_connection* conn, uint8_t saveOpt);

bool HasGraphData();
void DetectHighLowInGraph(pm3_connection* conn, int *high, int *low, bool addFuzz); 

#define GRAPH_SAVE 1
#define GRAPH_RESTORE 0

#endif
