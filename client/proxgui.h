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
#include "uart.h"
#include "comms.h"

void ShowGraphWindow(pm3_connection* conn);
void HideGraphWindow(pm3_connection* conn);
void RepaintGraphWindow(pm3_connection* conn);
void MainGraphics(void);
void InitGraphics(int argc, char **argv, char *script_cmds_file, bool usb_present, serial_port* port, bool flush_after_write);
void ExitGraphics(void);

//Operations defined in data_operations
//extern int autoCorr(const int* in, int *out, size_t len, int window);
extern int AskEdgeDetect(const int *in, int *out, int len, int threshold);
extern int AutoCorrelate(const int *in, int *out, size_t len, int window, bool SaveGrph, bool verbose);
extern int directionalThreshold(const int* in, int *out, size_t len, int8_t up, int8_t down);


#ifdef __cplusplus
}
#endif
