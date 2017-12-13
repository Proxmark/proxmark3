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

void ShowGui(void);
void HideGraphWindow(void);
void ShowGraphWindow(void);
void RepaintGraphWindow(void);
void PrintAndLog(char *fmt, ...);
void SetLogFilename(char *fn);

double CursorScaleFactor;
int PlotGridX, PlotGridY, PlotGridXdefault, PlotGridYdefault, CursorCPos, CursorDPos, GridOffset;
int offline;
int flushAfterWrite;   //buzzy
bool GridLocked;
bool showDemod;

#endif
