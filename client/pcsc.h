//-----------------------------------------------------------------------------
// Copyright (C) 2019 piwi
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// PCSC functions to use alternative Smartcard Readers
//-----------------------------------------------------------------------------

#ifndef PCSC_H__
#define PCSC_H__

#include <stdint.h>
#include <stdbool.h>
#include "smartcard.h"

uint8_t *pcsc_get_trace_addr(void);
uint32_t pcsc_get_traceLen(void);
char *getAlternativeSmartcardReader(void);
bool pcscCheckForCardReaders(void);
bool pcscSelectAlternativeCardReader(const char *readername);
bool pcscGetATR(smart_card_atr_t *card);
void pcscTransmit(uint8_t *data, uint32_t data_len, uint32_t flags, uint8_t *response, int *response_len);

#endif
