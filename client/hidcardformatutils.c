//-----------------------------------------------------------------------------
// Copyright (C) 2018 grauerfuchs
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// HID card format packing/unpacking support functions
//-----------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hidcardformatutils.h"
#include "ui.h"

bool get_bit_by_position(/* in */hidproxmessage_t* data, /* in */uint8_t pos){
  if (pos >= data->Length) return false;
  pos = (data->Length - pos) - 1; // invert ordering; Indexing goes from 0 to 1. Subtract 1 for weight of bit.
  bool result = false;
  if (pos > 95) 
    result = false;
  else if (pos > 63)
    result = (data->top >> (pos - 64)) & 1;
  else if (pos > 31)
    result = (data->mid >> (pos - 32)) & 1;
  else
    result = (data->bot >> pos) & 1;
  return result;
}
bool set_bit_by_position(/* inout */hidproxmessage_t* data, /* in */bool value, /* in */uint8_t pos){
  if (pos >= data->Length) return false;
  pos = (data->Length - pos) - 1; // invert ordering; Indexing goes from 0 to 1. Subtract 1 for weight of bit.
  if (pos > 95) {
    return false;
  } else if (pos > 63) {
    if (value)
      data->top |= (1 << (pos - 64));
    else
      data->top &= ~(1 << (pos - 64));
    return true;
  } else if (pos > 31) {
    if (value)
      data->mid |= (1 << (pos - 32));
    else
      data->mid &= ~(1 << (pos - 32));
    return true;
  } else {
    if (value)
      data->bot |= (1 << pos);
    else
      data->bot &= ~(1 << pos);
    return true;
  }
}
/**
 * Safeguard the data by doing a manual deep copy
 * 
 * At the time of the initial writing, the struct does not contain pointers. That doesn't
 * mean it won't eventually contain one, however. To prevent memory leaks and erroneous
 * aliasing, perform the copy function manually instead. Hence, this function.
 * 
 * If the definition of the hidproxmessage struct changes, this function must also
 * be updated to match.
 */
void proxmessage_datacopy(/*in*/hidproxmessage_t* src, /*out*/hidproxmessage_t* dest){
  dest->bot = src->bot;
  dest->mid = src->mid;
  dest->top = src->top;
  dest->Length = src->Length;
}
/**
 *
 * Yes, this is horribly inefficient for linear data. 
 * The current code is a temporary measure to have a working function in place
 * until all the bugs shaken from the block/chunk version of the code.
 *
 */
uint64_t get_linear_field(/* in */hidproxmessage_t* data, uint8_t firstBit, uint8_t length){
  uint64_t result = 0;
  for (uint8_t i = 0; i < length; i++ ) {
    result = (result << 1) | get_bit_by_position(data, firstBit + i);
  }
  return result;
}
bool set_linear_field(/* inout */hidproxmessage_t* data, uint64_t value, uint8_t firstBit, uint8_t length){
  hidproxmessage_t tmpdata;
  proxmessage_datacopy(data, &tmpdata);
  bool result = true;
  for (int i = 0; i < length; i++){
    result &= set_bit_by_position(&tmpdata, (value >> ((length - i) - 1)) & 1, firstBit + i);
  }
  if (result) proxmessage_datacopy(&tmpdata, data);
  return result;
}

uint64_t get_nonlinear_field(/* in */hidproxmessage_t* data, uint8_t numBits, uint8_t* bits){
  uint64_t result = 0;
  for (int i = 0; i < numBits; i++){
    result = (result << 1) | (get_bit_by_position(data, *(bits+i)) & 1);
  }
  return result;
}
bool set_nonlinear_field(/* inout */hidproxmessage_t* data, uint64_t value, uint8_t numBits, uint8_t* bits){
  hidproxmessage_t tmpdata;
  proxmessage_datacopy(data, &tmpdata);
  bool result = true;
  for (int i = 0; i < numBits; i++){
    result &= set_bit_by_position(&tmpdata, (value >> ((numBits - i) - 1)) & 1, *(bits + i));
  }
  if (result) proxmessage_datacopy(&tmpdata, data);
  return result;
}

uint8_t get_length_from_header(/* inout */hidproxmessage_t* data) {
  uint8_t len = 0;
	
  uint32_t hFmt; // for calculating card length
  if ((data->top & 0x000FFFFF) > 0) { // > 64 bits
    hFmt = data->top & 0x000FFFFF;
    len = 64;
  } else if ((data->mid & 0xFFFFFFC0) > 0) { // < 63-38 bits
    hFmt = data->mid & 0xFFFFFFC0;
    len = 32;
  } else if ((data->mid & 0x00000020) == 0) { // 37 bits
    hFmt = 0;
    len = 37;
  } else if ((data->mid & 0x0000001F) > 0){ // 36-32 bits
    hFmt = data->mid & 0x0000001F;
    len = 32;
  } else { // <32 bits
    hFmt = data->bot;
    len = 0;
  }

  while (hFmt > 1) {
    hFmt >>= 1;
    len++;
  }
  return len;
}

hidproxmessage_t initialize_proxmessage_object(uint32_t top, uint32_t mid, uint32_t bot){
  struct hidproxmessage_s result;
  memset(&result, 0, sizeof(hidproxmessage_t));
  result.top = top;
  result.mid = mid;
  result.bot = bot;
  result.Length = get_length_from_header(&result);
  return result;
}
bool add_HID_header(/* inout */hidproxmessage_t* data){
  if (data->Length > 84 || data->Length == 0) return false; // Invalid value

  if (data->Length >= 64){
    data->top |= 1 << (data->Length - 64); // leading 1: start bit
    data->top |= 0x09e00000; // Extended-length header
  } else if (data->Length > 37){
    data->mid |= 1 << (data->Length - 32); // leading 1: start bit
    data->top |= 0x09e00000; // Extended-length header
  } else if (data->Length == 37){
    // No header bits added to 37-bit cards
  } else if (data->Length >= 32){
    data->mid |= 0x20; // Bit 37; standard header
    data->mid |= 1 << (data->Length - 32); // leading 1: start bit
  } else {
    data->mid |= 0x20; // Bit 37; standard header
    data->bot |= 1 << data->Length; // leading 1: start bit
  }
  return true;
}
