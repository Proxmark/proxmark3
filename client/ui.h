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
#include <pthread.h>
#include "comms.h"

// a global mutex to prevent interlaced printing from different threads
pthread_mutex_t print_lock;
extern uint8_t g_debugMode;

void ShowGui(void);
void HideGraphWindow(pm3_connection* conn);
void ShowGraphWindow(pm3_connection* conn);
void RepaintGraphWindow(pm3_connection* conn);
void PrintAndLog(char *fmt, ...);
void SetLogFilename(char *fn);
void SetFlushAfterWrite(bool flush_after_write);

#endif
