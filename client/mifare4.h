//-----------------------------------------------------------------------------
// Copyright (C) 2018 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// iso14443-4 mifare commands
//-----------------------------------------------------------------------------

#ifndef MIFARE4_H
#define MIFARE4_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
	bool Authenticated;
	uint16_t KeyNum;
	uint8_t Rnd1[16];
	uint8_t Rnd2[16];
	
}mf4Session;

extern int MifareAuth4(mf4Session *session, uint8_t *keyn, uint8_t *key, bool activateField, bool leaveSignalON, bool verbose);



#endif // mifare4.h
