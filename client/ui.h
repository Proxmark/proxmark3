//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// UI utilities
//-----------------------------------------------------------------------------

#ifndef UI_H__
#define UI_H__

#include <stdbool.h>
#include <stdint.h>

#define MAX_PRINT_BUFFER 2048
typedef enum logLevel {NORMAL, SUCCESS, INFO, FAILED, WARNING, ERR, DEBUG} logLevel_t;

void ShowGui(void);
void HideGraphWindow(void);
void ShowGraphWindow(void);
void RepaintGraphWindow(void);
void PrintAndLog(char *fmt, ...);
void PrintAndLogEx(logLevel_t level, char *fmt, ...);
void SetLogFilename(char *fn);
void SetFlushAfterWrite(bool flush_after_write);

extern double CursorScaleFactor;
extern int PlotGridX, PlotGridY, PlotGridXdefault, PlotGridYdefault, CursorCPos, CursorDPos, GridOffset;
extern bool GridLocked;
extern bool showDemod;

#endif
