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
void InitGraphics(int argc, char **argv, char *script_cmds_file, char *script_cmd, bool usb_present);
void ExitGraphics(void);

#define MAX_GRAPH_TRACE_LEN (40000*8)
int GraphBuffer[MAX_GRAPH_TRACE_LEN];
int GraphTraceLen;
int s_Buff[MAX_GRAPH_TRACE_LEN];

double CursorScaleFactor;
int PlotGridX, PlotGridY, PlotGridXdefault, PlotGridYdefault, CursorCPos, CursorDPos, GridOffset;
int CommandFinished;
int offline;
bool GridLocked;

//Operations defined in data_operations
//extern int autoCorr(const int* in, int *out, size_t len, int window);
int AskEdgeDetect(const int *in, int *out, int len, int threshold);
int AutoCorrelate(const int *in, int *out, size_t len, int window, bool SaveGrph, bool verbose);
int directionalThreshold(const int* in, int *out, size_t len, int8_t up, int8_t down);
void save_restoreGB(uint8_t saveOpt);

#define GRAPH_SAVE 1
#define GRAPH_RESTORE 0
#define MAX_DEMOD_BUF_LEN (1024*128)
uint8_t DemodBuffer[MAX_DEMOD_BUF_LEN];
size_t DemodBufferLen;
size_t g_DemodStartIdx;
bool showDemod;
uint8_t g_debugMode;

#ifdef __cplusplus
}
#endif
