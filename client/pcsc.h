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

#include <stdbool.h>

char *getAlternativeSmartcardReader(void);
bool pcscCheckForCardReaders(void);
bool pcscSelectAlternativeCardReader(const char *readername);

#endif
