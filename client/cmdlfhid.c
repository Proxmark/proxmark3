//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency HID commands (known)
//
// Useful resources:
// RF interface, programming a T55x7 clone, 26-bit HID H10301 encoding:
// http://www.proxmark.org/files/Documents/125%20kHz%20-%20HID/HID_format_example.pdf
//
// "Understanding Card Data Formats"
// https://www.hidglobal.com/sites/default/files/hid-understanding_card_data_formats-wp-en.pdf
//
// "What Format Do You Need?"
// https://www.hidglobal.com/sites/default/files/resource_files/hid-prox-br-en.pdf
//-----------------------------------------------------------------------------

#include "cmdlfhid.h"

#include <stdio.h>
#include <string.h>
#include "comms.h"
#include "ui.h"
#include "graph.h"
#include "cmdparser.h"
#include "cmddata.h"  //for g_debugMode, demodbuff cmds
#include "lfdemod.h" // for HIDdemodFSK
#include "parity.h" // for parity
#include "util.h" // for param_get8,32,64

static int CmdHelp(const char *Cmd);

/**
 * Packs an HID ID from component parts.
 *
 * This only works with 26, 34, 35, 37, and 48 bit card IDs.
 *
 * NOTE: Parity calculation is supported only on 26, 35, and 48 bit IDs.
 *
 * Returns false on invalid inputs.
 */
bool pack_hid(/* out */ uint32_t *hi2, /* out */ uint32_t *hi, /* out */ uint32_t *lo, /* in */ const hid_info *info) {
  uint32_t higher = 0, high = 0, low = 0;

  switch (info->fmtLen) {
    case 26: // HID H10301
      low |= (info->cardnum & 0xffff) << 1;
      low |= (info->fc & 0xff) << 17;

      if (info->parityValid) {
        // Calculate parity
        low |= oddparity32((low >> 1) & 0xfff) & 1;
        low |= (evenparity32((low >> 13) & 0xfff) & 1) << 25;
      }
    break;

  case 34:
    low |= (info->cardnum & 0xffff) << 1;
    low |= (info->fc & 0x7fff) << 17;
    high |= (info->fc & 0x8000) >> 15;
    // TODO: Calculate parity
    break;

  case 35:
    low |= (info->cardnum & 0xfffff) << 1;
    low |= (info->fc & 0x7ff) << 21;
    high |= (info->fc & 0x800) >> 11;
    if (info->parityValid) {
      high |= (evenparity32((high & 0x00000001) ^ (low & 0xB6DB6DB6)) & 1) << 1;
      low |=  (oddparity32( (high & 0x00000003) ^ (low & 0x6DB6DB6C)) & 1);
      high |= (oddparity32( (high & 0x00000003) ^ (low & 0xFFFFFFFF)) & 1) << 2;
    }
    break;

  case 37:
    low |= (info->cardnum & 0x7ffff) << 1;
    low |= (info->fc & 0xfff) << 20;
    high |= (info->fc & 0xf000) >> 12;
    // TODO: Calculate parity
    break;

  case 48:
    low |= (info->cardnum & 0x7FFFFF) << 1;
    low |= (info->fc & 0xff) << 24;
    high |= (info->fc & 0x3FFF00) >> 8;
    if (info->parityValid) {
      high |= (evenparity32((high & 0x00001B6D) ^ (low & 0xB6DB6DB6)) & 1) << 14;
      low |=  (oddparity32( (high & 0x000036DB) ^ (low & 0x6DB6DB6C)) & 1);
      high |= (oddparity32( (high & 0x00007FFF) ^ (low & 0xFFFFFFFF)) & 1) << 15;
    }
    break;

  default:
    // Invalid / unsupported length
    return false;
  }
  
  // Set the format length bits
  if (info->fmtLen < 37) {
    // Bit 37 is always set
    high |= 0x20;

    // Set the bit corresponding to the length.
    if (info->fmtLen < 32)
      low |= 1 << info->fmtLen;
    else 
      high |= 1 << (info->fmtLen - 32);
    
  } else if (info->fmtLen > 37){
    if (info->fmtLen < 64) 
      high |= 1 << (info->fmtLen - 32);
    else 
      higher |= 1 << (info->fmtLen - 64);
  }
  // Return result only if successful.
  *hi2 = higher;
  *hi = high;
  *lo = low;
  return true;
}

/**
 * Unpacks an HID ID into its component parts.
 *
 * This only works with 26, 34, 35, 37, and 48 bit card IDs.
 *
 * NOTE: Parity checking is only supported on 26, 35, and 48 bit tags.
 *
 * Returns false on invalid inputs.
 */
bool unpack_hid(hid_info *out, uint32_t hi2, uint32_t hi, uint32_t lo) {
  memset(out, 0, sizeof(hid_info));
  uint8_t fmtLen = 0;
	
  uint32_t hFmt; // for calculating card length
  if ((hi2 & 0x000FFFFF) > 0) { // > 64 bits
    hFmt = hi2 & 0x000FFFFF;
    fmtLen = 64;
  } else if ((hi & 0xFFFFFFC0) > 0) { // < 63-38 bits
    hFmt = hi & 0xFFFFFFC0;
    fmtLen = 32;
  } else if ((hi & 0x00000020) == 0) { // 37 bits
    hFmt = 0;
    fmtLen = 37;
  } else if ((hi & 0x0000001F) > 0){ // 36-32 bits
    hFmt = hi & 0x0000001F;
    fmtLen = 32;
  } else { // <32 bits
    hFmt = lo;
    fmtLen = 0;
  }

  while (hFmt > 1) {
    hFmt >>= 1;
    fmtLen++;
  }
  out->fmtLen = fmtLen;
  switch (out->fmtLen) {
    case 26: // HID H10301
      out->cardnum = (lo >> 1) & 0xFFFF;
      out->fc = (lo >> 17) & 0xFF;

      if (g_debugMode) {
        PrintAndLog("oddparity : input=%x, calculated=%d, provided=%d",
          (lo >> 1) & 0xFFF, oddparity32((lo >> 1) & 0xFFF), lo & 1);
        PrintAndLog("evenparity: input=%x, calculated=%d, provided=%d",
          (lo >> 13) & 0xFFF, evenparity32((lo >> 13) & 0xFFF) & 1, (lo >> 25) & 1);
      }

      out->parityValid =
        (oddparity32((lo >> 1) & 0xFFF) == (lo & 1)) &&
        ((evenparity32((lo >> 13) & 0xFFF) & 1) == ((lo >> 25) & 1));
      break;

    case 34:
      out->cardnum = (lo >> 1) & 0xFFFF;
      out->fc = ((hi & 1) << 15) | (lo >> 17);
      // TODO: Calculate parity
      break;

    case 35:
      out->cardnum = (lo >> 1) & 0xFFFFF;
      out->fc = ((hi & 1) << 11) | (lo >> 21);
      out->parityValid = 
        (evenparity32((hi & 0x00000001) ^ (lo & 0xB6DB6DB6)) == ((hi >> 1) & 1)) &&
        (oddparity32( (hi & 0x00000003) ^ (lo & 0x6DB6DB6C)) == ((lo >> 0) & 1)) &&
        (oddparity32( (hi & 0x00000003) ^ (lo & 0xFFFFFFFF)) == ((hi >> 2) & 1));
      if (g_debugMode) {
        PrintAndLog("Parity check: %d, %d, %d vs bits %d, %d, %d",
          evenparity32((hi & 0x00000001) ^ (lo & 0xB6DB6DB6)),
          oddparity32( (hi & 0x00000003) ^ (lo & 0x6DB6DB6C)),
          oddparity32( (hi & 0x00000003) ^ (lo & 0xFFFFFFFF)),
          ((hi >> 1) & 1),
          ((lo >> 0) & 1),
          ((hi >> 2) & 1)
        );
      }
      break;
    case 37:
      out->fmtLen = 37;
      out->cardnum = (lo >> 1) & 0x7FFFF;
      out->fc = ((hi & 0xF) << 12) | (lo >> 20);
      // TODO: Calculate parity
      break;
    case 48:
      out->cardnum = (lo >> 1) & 0x7FFFFF;  //Start 24, 23 length
      out->fc = ((hi & 0x3FFF) << 8 ) | (lo >> 24);  //Start 2, 22 length
      out->parityValid = 
        (evenparity32((hi & 0x00001B6D) ^ (lo & 0xB6DB6DB6)) == ((hi >> 14) & 1)) &&
        (oddparity32( (hi & 0x000036DB) ^ (lo & 0x6DB6DB6C)) == ((lo >> 0) & 1)) &&
        (oddparity32( (hi & 0x00007FFF) ^ (lo & 0xFFFFFFFF)) == ((hi >> 15) & 1));
      if (g_debugMode) {
        PrintAndLog("Parity check: %d, %d, %d vs bits %d, %d, %d",
          evenparity32((hi & 0x00001B6D) ^ (lo & 0xB6DB6DB6)),
          oddparity32( (hi & 0x000036DB) ^ (lo & 0x6DB6DB6C)),
          oddparity32( (hi & 0x00007FFF) ^ (lo & 0xFFFFFFFF)),
          ((hi >> 14) & 1),
          ((lo >> 0) & 1),
          ((hi >> 15) & 1)
        );
      }
      break;

    default:
      return false;
  }
  return true;
}

/**
 * Converts a hex string to component "hi2", "hi" and "lo" 32-bit integers, one nibble
 * at a time.
 *
 * Returns the number of nibbles (4 bits) entered.
 */
int hexstring_to_int96(/* out */ uint32_t* hi2,/* out */ uint32_t* hi, /* out */ uint32_t* lo, const char* str) {
  // TODO: Replace this with param_gethex when it supports arbitrary length
  // inputs.
  int n = 0, i = 0;

  while (sscanf(&str[i++], "%1x", &n ) == 1) {
    *hi2 = (*hi2 << 4) | (*hi >> 28);
    *hi = (*hi << 4) | (*lo >> 28);
    *lo = (*lo << 4) | (n & 0xf);
  }

  return i - 1;
}

//by marshmellow (based on existing demod + holiman's refactor)
//HID Prox demod - FSK RF/50 with preamble of 00011101 (then manchester encoded)
//print full HID Prox ID and some bit format details if found
int CmdFSKdemodHID(const char *Cmd)
{
  //raw fsk demod no manchester decoding no start bit finding just get binary from wave
  uint32_t hi2=0, hi=0, lo=0;

  uint8_t BitStream[MAX_GRAPH_TRACE_LEN]={0};
  size_t BitLen = getFromGraphBuf(BitStream);
  if (BitLen==0) return 0;
  //get binary from fsk wave
  int waveIdx = 0;
  int idx = HIDdemodFSK(BitStream,&BitLen,&hi2,&hi,&lo, &waveIdx);
  if (idx<0){
    if (g_debugMode){
      if (idx==-1){
        PrintAndLog("DEBUG: Just Noise Detected");
      } else if (idx == -2) {
        PrintAndLog("DEBUG: Error demoding fsk");
      } else if (idx == -3) {
        PrintAndLog("DEBUG: Preamble not found");
      } else if (idx == -4) {
        PrintAndLog("DEBUG: Error in Manchester data, SIZE: %d", BitLen);
      } else {
        PrintAndLog("DEBUG: Error demoding fsk %d", idx);
      }   
    }
    return 0;
  }
  if (hi2==0 && hi==0 && lo==0) {
    if (g_debugMode) PrintAndLog("DEBUG: Error - no values found");
    return 0;
  }
  
  hid_info card_info;
  bool ret = unpack_hid(&card_info, (uint32_t)hi2, (uint32_t)hi, (uint32_t)lo);
  
  if (hi2 != 0)
    PrintAndLog("HID Prox TAG ID: %x%08x%08x (%d) - Format Len: %u bits - FC: %u - Card: %u",
      (unsigned int) hi2, (unsigned int) hi, (unsigned int) lo, (unsigned int) (lo>>1) & 0xFFFF,
      card_info.fmtLen, card_info.fc, card_info.cardnum);
  else
    PrintAndLog("HID Prox TAG ID: %x%08x (%d) - Format Len: %u bits - FC: %u - Card: %u",
      (unsigned int) hi, (unsigned int) lo, (unsigned int) (lo>>1) & 0xFFFF,
      card_info.fmtLen, card_info.fc, card_info.cardnum);

  if (card_info.fmtLen == 26 || card_info.fmtLen == 35 || card_info.fmtLen == 48) {
    PrintAndLog("Parity: %s", card_info.parityValid ? "valid" : "invalid");
  }
  if (!ret) {
    PrintAndLog("Invalid or unsupported tag length.");
  }
  setDemodBuf(BitStream,BitLen,idx);
  setClockGrid(50, waveIdx + (idx*50));
  if (g_debugMode){ 
    PrintAndLog("DEBUG: idx: %d, Len: %d, Printing Demod Buffer:", idx, BitLen);
    printDemodBuff();
  }
  return 1;
}

int CmdHIDReadFSK(const char *Cmd)
{
  int findone=0;
  if(Cmd[0]=='1') findone=1;
  UsbCommand c={CMD_HID_DEMOD_FSK};
  c.arg[0]=findone;
  SendCommand(&c);
  return 0;
}

int CmdHIDSim(const char *Cmd)
{
  uint32_t hi2 = 0, hi = 0, lo = 0;
  hexstring_to_int96(&hi2, &hi, &lo, Cmd);
  if (hi >= 0x40 || hi2 != 0) {
    PrintAndLog("This looks like a long tag ID. Use 'lf simfsk' for long tags. Aborting!");
    return 0;
  }

  PrintAndLog("Emulating tag with ID %x%08x", hi, lo);
  PrintAndLog("Press pm3-button to abort simulation");

  UsbCommand c = {CMD_HID_SIM_TAG, {hi, lo, 0}};
  SendCommand(&c);
  return 0;
}

int CmdHIDClone(const char *Cmd)
{
  unsigned int hi2 = 0, hi = 0, lo = 0;
  UsbCommand c;
  hexstring_to_int96(&hi2, &hi, &lo, Cmd);
    
  if (hi >= 0x40 || hi2 != 0) {
    PrintAndLog("Cloning tag with long ID %x%08x%08x", hi2, hi, lo);
    c.d.asBytes[0] = 1;
  } else {
    PrintAndLog("Cloning tag with ID %x%08x", hi, lo);
    c.d.asBytes[0] = 0;
  }

  c.cmd = CMD_HID_CLONE_TAG;
  c.arg[0] = hi2;
  c.arg[1] = hi;
  c.arg[2] = lo;

  SendCommand(&c);
  return 0;
}


int CmdHIDPack(const char *Cmd) {
  uint32_t hi2 = 0, hi = 0, lo = 0;
  
  if (strlen(Cmd)<3) {
    PrintAndLog("Usage:  lf hid pack <length> <facility code (decimal)> <card number (decimal)>");
    PrintAndLog("        sample: lf hid pack 26 123 4567");
    return 0;
  }
  uint8_t fmtLen = param_get8(Cmd, 0);

  // TODO
  if (fmtLen != 26 && fmtLen != 35 && fmtLen != 48) {
    PrintAndLog("Warning: Parity bits are only calculated for 26, 35, and 48 bit IDs -- this may be invalid!");
  }

  hid_info card_info;
  card_info.fmtLen = fmtLen;
  card_info.fc = param_get32ex(Cmd, 1, 0, 10);
  card_info.cardnum = param_get64ex(Cmd, 2, 0, 10);
  card_info.parityValid = true;

  bool ret = pack_hid(&hi2, &hi, &lo, &card_info);
  if (ret) {
    if (hi2 != 0) {
      PrintAndLog("HID Prox TAG ID: %x%08x%08x (%d) - Format Len: %u bits - FC: %u - Card: %u",
        (unsigned int) hi2, (unsigned int) hi, (unsigned int) lo, (unsigned int) (lo>>1) & 0xFFFF,
        card_info.fmtLen, card_info.fc, card_info.cardnum);
    } else {
      PrintAndLog("HID Prox TAG ID: %x%08x (%d) - Format Len: %u bits - FC: %u - Card: %u",
        (unsigned int) hi, (unsigned int) lo, (unsigned int) (lo>>1) & 0xFFFF,
        card_info.fmtLen, card_info.fc, card_info.cardnum); 
    }
  } else {
    PrintAndLog("Invalid or unsupported tag length.");
  }
  return 0;
}


int CmdHIDUnpack(const char *Cmd)
{
  uint32_t hi2 = 0, hi = 0, lo = 0;
  if (strlen(Cmd)<1) {
    PrintAndLog("Usage:  lf hid unpack <ID>");
    PrintAndLog("        sample: lf hid unpack 2006f623ae");
    return 0;
  }

  hid_info card_info;
  bool ret = unpack_hid(&card_info, hi2, hi, lo);

  hexstring_to_int96(&hi2, &hi, &lo, Cmd);
  if (hi2 != 0) {
    PrintAndLog("HID Prox TAG ID: %x%08x%08x (%d) - Format Len: %u bits - FC: %u - Card: %u",
      (unsigned int) hi2, (unsigned int) hi, (unsigned int) lo, (unsigned int) (lo>>1) & 0xFFFF,
      card_info.fmtLen, card_info.fc, card_info.cardnum);
  } else {
    PrintAndLog("HID Prox TAG ID: %x%08x (%d) - Format Len: %u bits - FC: %u - Card: %u",
      (unsigned int) hi, (unsigned int) lo, (unsigned int) (lo>>1) & 0xFFFF,
      card_info.fmtLen, card_info.fc, card_info.cardnum);
  }
  if (card_info.fmtLen == 26 || card_info.fmtLen == 35 || card_info.fmtLen == 48) {
    PrintAndLog("Parity: %s", card_info.parityValid ? "valid" : "invalid");
  }
  if (!ret) {
    PrintAndLog("Invalid or unsupported tag length.");
  }
  return 0;
}


static command_t CommandTable[] = 
{
  {"help",      CmdHelp,        1, "This help"},
  {"demod",     CmdFSKdemodHID, 1, "Demodulate HID Prox from GraphBuffer"},
  {"read",      CmdHIDReadFSK,  0, "['1'] Realtime HID FSK Read from antenna (option '1' for one tag only)"},
  {"sim",       CmdHIDSim,      0, "<ID> -- HID tag simulator"},
  {"clone",     CmdHIDClone,    0, "<ID> -- Clone HID to T55x7 (tag must be in antenna)"},
  {"pack",      CmdHIDPack,     1, "<len> <fc> <num> -- packs an HID ID from its length, facility code and card number"},
  {"unpack",    CmdHIDUnpack,   1, "<ID> -- unpacks an HID ID to its length, facility code and card number"},
  {NULL, NULL, 0, NULL}
};

int CmdLFHID(const char *Cmd)
{
  CmdsParse(CommandTable, Cmd);
  return 0;
}

int CmdHelp(const char *Cmd)
{
  CmdsHelp(CommandTable);
  return 0;
}
