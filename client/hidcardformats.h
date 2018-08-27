//-----------------------------------------------------------------------------
// Copyright (C) 2018 grauerfuchs
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// HID card format packing/unpacking routines
//-----------------------------------------------------------------------------

#ifndef HIDCARDFORMATS_H__
#define HIDCARDFORMATS_H__

#include <stdint.h>
#include <stdbool.h>
#include "hidcardformatutils.h"

// Structure for defined HID card formats available for packing/unpacking
typedef struct hidcardformat_s{
  const char* Name;
  bool (*Pack)(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed);
  bool (*Unpack)(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card);
  const char* Descrp;
} hidcardformat_t;

void HIDListFormats();
int HIDFindCardFormat(const char *format);
hidcardformat_t HIDGetCardFormat(int idx);
bool HIDPack(/* in */int FormatIndex, /* in */hidproxcard_t* card, /* out */hidproxmessage_t* packed);
bool HIDTryUnpack(/* in */hidproxmessage_t* packed);

#endif
