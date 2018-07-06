//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency HID commands
//-----------------------------------------------------------------------------

#ifndef CMDLFHID_H__
#define CMDLFHID_H__

#include <stdint.h>
#include <stdbool.h>

// Structure for unpacked "short" (<38 bits) HID Prox tags.
typedef struct {
  // Format length, in bits.
  uint8_t fmtLen;

  // Facility code.
  uint32_t fc;

  // Card number.
  uint32_t cardnum;

  // Parity validity.
  //
  // When used with pack_short_hid, this determines if we should calculate
  // parity values for the ID.
  //
  // When used with unpack_short_hid, this indicates if we got valid parity
  // values for the ID.
  bool parityValid;
} short_hid_info;

bool pack_short_hid(/* out */ uint32_t *hi, /* out */ uint32_t *lo, /* in */ const short_hid_info *info);
bool unpack_short_hid(short_hid_info* out, uint32_t hi, uint32_t lo);

int CmdLFHID(const char *Cmd);
int CmdFSKdemodHID(const char *Cmd);
int CmdHIDReadDemod(const char *Cmd);
int CmdHIDSim(const char *Cmd);
int CmdHIDClone(const char *Cmd);
int CmdHIDPack(const char *Cmd);
int CmdHIDUnpack(const char *Cmd);

#endif
