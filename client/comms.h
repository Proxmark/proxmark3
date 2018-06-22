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

#ifndef COMMS_H_
#define COMMS_H_

#include <stdbool.h>
#include <pthread.h>

#include "usb_cmd.h"
#include "uart.h"

#ifndef CMD_BUFFER_SIZE
#define CMD_BUFFER_SIZE 50
#endif

void SetOffline(bool new_offline);
bool IsOffline();

bool OpenProxmark(void *port, bool wait_for_port, int timeout, bool flash_mode);
void CloseProxmark(void);

void SendCommand(UsbCommand *c);

void clearCommandBuffer();
bool WaitForResponseTimeoutW(uint32_t cmd, UsbCommand* response, size_t ms_timeout, bool show_warning);
bool WaitForResponseTimeout(uint32_t cmd, UsbCommand* response, size_t ms_timeout);
bool WaitForResponse(uint32_t cmd, UsbCommand* response);
bool GetFromBigBuf(uint8_t *dest, int bytes, int start_index, UsbCommand *response, size_t ms_timeout, bool show_warning);

#endif // COMMS_H_
