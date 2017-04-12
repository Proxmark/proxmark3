//-----------------------------------------------------------------------------
// Copyright (C) 2009 Michael Gernoth <michael at gernoth.net>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// GUI functions
//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

void ShowGraphWindow(void);
void HideGraphWindow(void);
void RepaintGraphWindow(void);
void MainGraphics(void);
void InitGraphics(int argc, char **argv);
void ExitGraphics(void);

#define MAX_GRAPH_TRACE_LEN (40000*8)
extern int GraphBuffer[MAX_GRAPH_TRACE_LEN];
extern int GraphTraceLen;
extern int s_Buff[MAX_GRAPH_TRACE_LEN];

extern double CursorScaleFactor;
extern int PlotGridX, PlotGridY, PlotGridXdefault, PlotGridYdefault, CursorCPos, CursorDPos, GridOffset;
extern int CommandFinished;
extern int offline;
extern bool GridLocked;

//Operations defined in data_operations
extern int autoCorr(const int* in, int *out, size_t len, int window);
extern int directionalThreshold(const int* in, int *out, size_t len, int8_t up, int8_t down);
extern void save_restoreGB(uint8_t saveOpt);

#define MAX_DEMOD_BUF_LEN (1024*128)
extern uint8_t DemodBuffer[MAX_DEMOD_BUF_LEN];
extern size_t DemodBufferLen;
extern size_t g_DemodStartIdx;
extern bool showDemod;

#ifdef __cplusplus
}
#endif
