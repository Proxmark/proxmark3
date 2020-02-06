//-----------------------------------------------------------------------------
// Copyright (C) 2009 Michael Gernoth <michael at gernoth.net>
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Code for communicating with the proxmark3 hardware.
//-----------------------------------------------------------------------------

#ifndef COMMS_H__
#define COMMS_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "usb_cmd.h"

extern void SetOffline(bool new_offline);
extern bool IsOffline();
extern bool OpenProxmark(void *port, bool wait_for_port, int timeout);
extern void CloseProxmark(void);
extern void SendCommand(UsbCommand *c);
extern void clearCommandBuffer();
extern bool WaitForResponseTimeoutW(uint32_t cmd, UsbCommand* response, size_t ms_timeout, bool show_warning);
extern bool WaitForResponseTimeout(uint32_t cmd, UsbCommand* response, size_t ms_timeout);
extern bool WaitForResponse(uint32_t cmd, UsbCommand* response);
extern bool GetFromBigBuf(uint8_t *dest, int bytes, int start_index, UsbCommand *response, size_t ms_timeout, bool show_warning);
extern bool GetFromFpgaRAM(uint8_t *dest, int bytes);

#endif // COMMS_H__
