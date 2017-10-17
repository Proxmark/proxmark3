//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Data utilities
//-----------------------------------------------------------------------------

#include <string.h>
#include <stdint.h>
#include "data.h"
#include "ui.h"
#include "proxmark3.h"
#include "cmdmain.h"

void GetFromBigBuf(pm3_connection* conn, uint8_t *dest, int bytes, int start_index)
{
	// FIXME: This instructs the proxmark3 hardware to write into a buffer, and then
	// implicitly trusts that it'll do the right thing in comms.c:UsbCommandReceived.
	conn->sample_buf = dest;
	UsbCommand c = {CMD_DOWNLOAD_RAW_ADC_SAMPLES_125K, {start_index, bytes, 0}};
	SendCommand(conn, &c);
}
