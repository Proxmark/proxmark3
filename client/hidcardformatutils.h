//-----------------------------------------------------------------------------
// Copyright (C) 2018 grauerfuchs
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// HID card format packing/unpacking support functions
//-----------------------------------------------------------------------------

#ifndef HIDCARDFORMATUTILS_H__
#define HIDCARDFORMATUTILS_H__

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

// Structure for packed HID messages
// Always align lowest value (last transmitted) bit to ordinal position 0 (lowest valued bit bottom)
typedef struct hidproxmessage_s{
  uint8_t Length; // Number of encoded bits in prox message (excluding headers and preamble)
  uint32_t top; // Bits in x<<64 positions
  uint32_t mid; // Bits in x<<32 positions
  uint32_t bot; // Lowest ordinal positions
} hidproxmessage_t;

// Structure for unpacked HID prox cards
typedef struct hidproxcard_s{
  uint32_t FacilityCode;
  uint64_t CardNumber;
  uint32_t IssueLevel;
  uint32_t OEM;
  bool ParityValid; // Only valid for responses
} hidproxcard_t;

bool get_bit_by_position(/* in */hidproxmessage_t* data, /* in */uint8_t pos);
bool set_bit_by_position(/* inout */hidproxmessage_t* data, /* in */bool value, /* in */uint8_t pos);

uint64_t get_linear_field(/* in */hidproxmessage_t* data, /* in */uint8_t firstBit, /* in */uint8_t length);
bool set_linear_field(/* inout */hidproxmessage_t* data, /* in */uint64_t value, /* in */uint8_t firstBit, /* in */uint8_t length);

uint64_t get_nonlinear_field(/* in */hidproxmessage_t* data, /* in */uint8_t numBits, /* in */uint8_t* bits);
bool set_nonlinear_field(/* inout */hidproxmessage_t* data, /* in */uint64_t value, /* in */uint8_t numBits, /* in */uint8_t* bits);

hidproxmessage_t initialize_proxmessage_object(/* in */uint32_t top, /* in */uint32_t mid, /* in */uint32_t bot);
bool add_HID_header(/* inout */hidproxmessage_t* data);
#endif
