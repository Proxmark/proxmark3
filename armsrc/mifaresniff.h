//-----------------------------------------------------------------------------
// Merlok - June 2012
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support mifare classic sniffer.
//-----------------------------------------------------------------------------

#ifndef __MIFARESNIFF_H
#define __MIFARESNIFF_H

#include <stdint.h>
#include <stdbool.h>

bool MfSniffInit(void);
bool RAMFUNC MfSniffLogic(const uint8_t *data, uint16_t len, uint8_t *parity, uint16_t bitCnt, bool reader);
bool RAMFUNC MfSniffSend(uint16_t maxTimeoutMs);
bool intMfSniffSend();
bool MfSniffEnd(void);

#endif