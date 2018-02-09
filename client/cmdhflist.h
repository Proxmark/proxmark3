//-----------------------------------------------------------------------------
// Copyright (C) Merlok - 2017
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Command: hf mf list. It shows data from arm buffer.
//-----------------------------------------------------------------------------
#ifndef CMDHFLIST_H
#define CMDHFLIST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

extern uint8_t iso14443A_CRC_check(bool isResponse, uint8_t* data, uint8_t len);
extern uint8_t mifare_CRC_check(bool isResponse, uint8_t* data, uint8_t len);
extern void annotateIso14443a(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize);
extern void annotateMifare(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize, bool isResponse);


#endif // CMDHFLIST
