//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Graph utilities
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ui.h"
#include "graph.h"
#include "lfdemod.h"
#include "cmddata.h" // SetClockGrid

/* write a manchester bit to the graph */
void AppendGraph(pm3_connection* conn, int redraw, int clock, int bit)
{
  int i;
  //set first half the clock bit (all 1's or 0's for a 0 or 1 bit) 
  for (i = 0; i < (int)(clock / 2); ++i)
    conn->GraphBuffer[conn->GraphTraceLen++] = bit ;
  //set second half of the clock bit (all 0's or 1's for a 0 or 1 bit)
  for (i = (int)(clock / 2); i < clock; ++i)
    conn->GraphBuffer[conn->GraphTraceLen++] = bit ^ 1;

  if (redraw)
    RepaintGraphWindow(conn);
}

// clear out our graph window
int ClearGraph(pm3_connection* conn, int redraw)
{
  int gtl = conn->GraphTraceLen;
  memset(conn->GraphBuffer, 0x00, sizeof(conn->GraphBuffer));

  conn->GraphTraceLen = 0;

  if (redraw)
    RepaintGraphWindow(conn);

  return gtl;
}
// option '1' to save GraphBuffer any other to restore
void save_restoreGB(pm3_connection* conn, uint8_t saveOpt)
{
	// FIXME: This function only allows a buffer of 1 graph to be stored.
	// FIXME: This function is not thread-safe.
	static int SavedGB[MAX_GRAPH_TRACE_LEN];
	static int SavedGBlen=0;
	static bool GB_Saved = false;
	static int SavedGridOffsetAdj=0;

	if (saveOpt == GRAPH_SAVE) { //save
		memcpy(SavedGB, conn->GraphBuffer, MAX_GRAPH_TRACE_LEN);
		SavedGBlen = conn->GraphTraceLen;
		GB_Saved=true;
		SavedGridOffsetAdj = conn->GridOffset;
	} else if (GB_Saved) { //restore
		memcpy(conn->GraphBuffer, SavedGB, MAX_GRAPH_TRACE_LEN);
		conn->GraphTraceLen = SavedGBlen;
		conn->GridOffset = SavedGridOffsetAdj;
		RepaintGraphWindow(conn);
	}
	return;
}

// DETECT CLOCK NOW IN LFDEMOD.C

void setGraphBuf(pm3_connection* conn, uint8_t *buff, size_t size)
{
	if ( buff == NULL ) return;
	
	uint16_t i = 0;  
	if ( size > MAX_GRAPH_TRACE_LEN )
		size = MAX_GRAPH_TRACE_LEN;
	ClearGraph(conn, 0);
	for (; i < size; ++i){
		conn->GraphBuffer[i]=buff[i]-128;
	}
	conn->GraphTraceLen=size;
	RepaintGraphWindow(conn);
	return;
}

size_t getFromGraphBuf(pm3_connection* conn, uint8_t *buff)
{
	if (buff == NULL ) return 0;
	uint32_t i;
	for (i=0;i<conn->GraphTraceLen;++i){
		if (conn->GraphBuffer[i]>127) conn->GraphBuffer[i]=127; //trim
		if (conn->GraphBuffer[i]<-127) conn->GraphBuffer[i]=-127; //trim
		buff[i]=(uint8_t)(conn->GraphBuffer[i]+128);
	}
	return i;
}

// A simple test to see if there is any data inside Graphbuffer. 
bool HasGraphData(pm3_connection* conn){
	if (conn->GraphTraceLen <= 0) {
		PrintAndLog("No data available, try reading something first");
		return false;
	}
	return true;	
}

// Detect high and lows in Grapbuffer.
// Only loops the first 256 values. 
void DetectHighLowInGraph(pm3_connection* conn, int *high, int *low, bool addFuzz) {

	uint8_t loopMax = 255;
	if ( loopMax > conn->GraphTraceLen)
		loopMax = conn->GraphTraceLen;
  
	for (uint8_t i = 0; i < loopMax; ++i) {
		if (conn->GraphBuffer[i] > *high)
			*high = conn->GraphBuffer[i];
		else if (conn->GraphBuffer[i] < *low)
			*low = conn->GraphBuffer[i];
	}
	
	//12% fuzz in case highs and lows aren't clipped
	if (addFuzz) {
		*high = (int)(*high * .88);
		*low  = (int)(*low  * .88);
	}
}

// Get or auto-detect ask clock rate
int GetAskClock(pm3_connection* conn, const char str[], bool printAns, bool verbose)
{
	int clock;
	// FIXME: Use atoi instead
	sscanf(str, "%i", &clock);
	if (!strcmp(str, ""))
		clock = 0;

	if (clock != 0) 
		return clock;
	// Auto-detect clock
	uint8_t grph[MAX_GRAPH_TRACE_LEN]={0};
	size_t size = getFromGraphBuf(conn, grph);
	if (size == 0) {
		if (verbose)
			PrintAndLog("Failed to copy from graphbuffer");
		return -1;
	}
	//, size_t *ststart, size_t *stend
	size_t ststart = 0, stend = 0;
	bool st = DetectST(grph, &size, &clock, &ststart, &stend);
	int start = stend;
	if (st == false) {
		start = DetectASKClock(grph, size, &clock, 20);
	}
	setClockGrid(conn, clock, start);
	// Only print this message if we're not looping something
	if (printAns || g_debugMode) {
		PrintAndLog("Auto-detected clock rate: %d, Best Starting Position: %d", clock, start);
	}
	return clock;
}

uint8_t GetPskCarrier(pm3_connection* conn, const char str[], bool printAns, bool verbose)
{
	uint8_t carrier=0;
	uint8_t grph[MAX_GRAPH_TRACE_LEN]={0};
	size_t size = getFromGraphBuf(conn, grph);
	if ( size == 0 ) {
		if (verbose) 
			PrintAndLog("Failed to copy from graphbuffer");
		return 0;
	}
	uint16_t fc = countFC(grph,size,0);
	carrier = fc & 0xFF;
	if (carrier != 2 && carrier != 4 && carrier != 8) return 0;
	if ((fc>>8) == 10 && carrier == 8) return 0;
	// Only print this message if we're not looping something
	if (printAns) {
		PrintAndLog("Auto-detected PSK carrier rate: %d", carrier);
	}
	return carrier;
}

int GetPskClock(pm3_connection* conn, const char str[], bool printAns, bool verbose)
{
	int clock;
	// FIXME: Use atoi instead.
	sscanf(str, "%i", &clock);
	if (!strcmp(str, "")) 
		clock = 0;

	if (clock!=0) 
		return clock;
	// Auto-detect clock
	uint8_t grph[MAX_GRAPH_TRACE_LEN]={0};
	size_t size = getFromGraphBuf(conn, grph);
	if ( size == 0 ) {
		if (verbose) 
			PrintAndLog("Failed to copy from graphbuffer");
		return -1;
	}
	size_t firstPhaseShiftLoc = 0;
	uint8_t curPhase = 0, fc = 0;
	clock = DetectPSKClock(grph, size, 0, &firstPhaseShiftLoc, &curPhase, &fc);
	setClockGrid(conn, clock, firstPhaseShiftLoc);
	// Only print this message if we're not looping something
	if (printAns){
		PrintAndLog("Auto-detected clock rate: %d", clock);
	}
	return clock;
}

uint8_t GetNrzClock(pm3_connection* conn, const char str[], bool printAns, bool verbose)
{
	int clock;
	sscanf(str, "%i", &clock);
	if (!strcmp(str, ""))
		clock = 0;

	if (clock!=0) 
		return clock;
	// Auto-detect clock
	uint8_t grph[MAX_GRAPH_TRACE_LEN]={0};
	size_t size = getFromGraphBuf(conn, grph);
	if ( size == 0 ) {
		if (verbose) 
			PrintAndLog("Failed to copy from graphbuffer");
		return -1;
	}
	size_t clkStartIdx = 0;
	clock = DetectNRZClock(grph, size, 0, &clkStartIdx);
	setClockGrid(conn, clock, clkStartIdx);
	// Only print this message if we're not looping something
	if (printAns){
		PrintAndLog("Auto-detected clock rate: %d", clock);
	}
	return clock;
}
//by marshmellow
//attempt to detect the field clock and bit clock for FSK
uint8_t GetFskClock(pm3_connection* conn, const char str[], bool printAns, bool verbose)
{
	int clock;
	sscanf(str, "%i", &clock);
	if (!strcmp(str, ""))
		clock = 0;
	if (clock != 0) return (uint8_t)clock;


	uint8_t fc1=0, fc2=0, rf1=0;
	int firstClockEdge = 0;
	uint8_t ans = fskClocks(conn, &fc1, &fc2, &rf1, verbose, &firstClockEdge);
	if (ans == 0) return 0;
	if ((fc1==10 && fc2==8) || (fc1==8 && fc2==5)){
		if (printAns) PrintAndLog("Detected Field Clocks: FC/%d, FC/%d - Bit Clock: RF/%d", fc1, fc2, rf1);
		setClockGrid(conn, rf1, firstClockEdge);
		return rf1;
	}
	if (verbose){
		PrintAndLog("DEBUG: unknown fsk field clock detected");
		PrintAndLog("Detected Field Clocks: FC/%d, FC/%d - Bit Clock: RF/%d", fc1, fc2, rf1);
	}
	return 0;
}
uint8_t fskClocks(pm3_connection* conn, uint8_t *fc1, uint8_t *fc2, uint8_t *rf1, bool verbose, int *firstClockEdge)
{
	uint8_t BitStream[MAX_GRAPH_TRACE_LEN]={0};
	size_t size = getFromGraphBuf(conn, BitStream);
	if (size==0) return 0;
	uint16_t ans = countFC(BitStream, size, 1); 
	if (ans==0) {
		if (verbose || g_debugMode) PrintAndLog("DEBUG: No data found");
		return 0;
	}
	*fc1 = (ans >> 8) & 0xFF;
	*fc2 = ans & 0xFF;
	//int firstClockEdge = 0;
	*rf1 = detectFSKClk(BitStream, size, *fc1, *fc2, firstClockEdge);
	if (*rf1==0) {
		if (verbose || g_debugMode) PrintAndLog("DEBUG: Clock detect error");
		return 0;
	}
	return 1;
}
bool graphJustNoise(int *BitStream, int size)
{
	static const uint8_t THRESHOLD = 15; //might not be high enough for noisy environments
	//test samples are not just noise
	bool justNoise1 = 1;
	for(int idx=0; idx < size && justNoise1 ;idx++){
		justNoise1 = BitStream[idx] < THRESHOLD;
	}
	return justNoise1;
}
