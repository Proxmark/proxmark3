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
#include "hidcardformats.h"
#include "hidcardformatutils.h"
#include "util.h" // for param_get8,32,64


/**
 * Converts a hex string to component "hi2", "hi" and "lo" 32-bit integers, one nibble
 * at a time.
 *
 * Returns the number of nibbles (4 bits) entered.
 */
int hid_hexstring_to_int96(/* out */ uint32_t* hi2,/* out */ uint32_t* hi, /* out */ uint32_t* lo, const char* str) {
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

void usage_encode(){
  PrintAndLog("Usage:  lf hid encode <format> [<field> <value (decimal)>] {...}");
  PrintAndLog("   Fields:    c: Card number");
  PrintAndLog("              f: Facility code");
  PrintAndLog("              i: Issue Level");
  PrintAndLog("              o: OEM code");
  PrintAndLog("   example: lf hid encode H10301 f 123 c 4567");
}
void PrintProxTagId(hidproxmessage_t *packed){
  if (packed->top != 0) {
      PrintAndLog("HID Prox TAG ID: %x%08x%08x",
        (uint32_t)packed->top, (uint32_t)packed->mid, (uint32_t)packed->bot);
    } else {
      PrintAndLog("HID Prox TAG ID: %x%08x",
        (uint32_t)packed->mid, (uint32_t)packed->bot); 
    }
}
bool Encode(/* in */ const char *Cmd, /* out */ hidproxmessage_t *packed){
  int formatIndex = -1;
  char format[16];
  memset(format, 0, sizeof(format));
  if (!strcmp(Cmd, "help") || !strcmp(Cmd, "h") || !strcmp(Cmd, "list") || !strcmp(Cmd, "?")){
    usage_encode();
    return false;
  } else {
    param_getstr(Cmd, 0, format, sizeof(format));
    formatIndex = HIDFindCardFormat(format);
    if (formatIndex == -1) {
      printf("Unknown format: %s\r\n", format);
      return false;
    }
  }
  hidproxcard_t data;
  memset(&data, 0, sizeof(hidproxcard_t));
  uint8_t cmdp = 1;
	while(param_getchar(Cmd, cmdp) != 0x00) {
		switch(param_getchar(Cmd, cmdp)) {
      case 'I':
      case 'i':
        data.IssueLevel = param_get32ex(Cmd, cmdp+1, 0, 10);
        cmdp += 2;
        break;
      case 'F':
      case 'f':
        data.FacilityCode = param_get32ex(Cmd, cmdp+1, 0, 10);
        cmdp += 2;
        break;
      case 'C':
      case 'c':
        data.CardNumber = param_get64ex(Cmd, cmdp+1, 0, 10);
        cmdp += 2;
			  break;
      case 'o':
      case 'O':
        data.OEM = param_get32ex(Cmd, cmdp+1, 0, 10);
        cmdp += 2;
			break;
      default:
        PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
        return false;
		}
	}
	memset(packed, 0, sizeof(hidproxmessage_t));
  if (!HIDPack(formatIndex, &data, packed))
  {
    PrintAndLog("The card data could not be encoded in the selected format.");
    return false;
  } else {
    return true;
  }

}
void Write(hidproxmessage_t *packed){
  UsbCommand c;
  c.d.asBytes[0] = (packed->top != 0 && ((packed->mid & 0xFFFFFFC0) != 0))
    ? 1 : 0; // Writing long format?
  c.cmd = CMD_HID_CLONE_TAG;
  c.arg[0] = (packed->top & 0x000FFFFF);
  c.arg[1] = packed->mid;
  c.arg[2] = packed->bot;
  SendCommand(&c);
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
  
  hidproxmessage_t packed = initialize_proxmessage_object(hi2, hi, lo);
  PrintProxTagId(&packed);

  bool ret = HIDTryUnpack(&packed, false);
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
  hid_hexstring_to_int96(&hi2, &hi, &lo, Cmd);
  if (hi2 != 0) {
    PrintAndLog("Emulating tag with ID %x%08x%08x", hi2, hi, lo);
  } else {
    PrintAndLog("Emulating tag with ID %x%08x", hi, lo);
  }

  PrintAndLog("Press pm3-button to abort simulation");

  UsbCommand c = {CMD_HID_SIM_TAG, {hi2, hi, lo}};
  SendCommand(&c);
  return 0;
}

int CmdHIDClone(const char *Cmd)
{
  unsigned int top = 0, mid = 0, bot = 0;
  hid_hexstring_to_int96(&top, &mid, &bot, Cmd);
  hidproxmessage_t packed = initialize_proxmessage_object(top, mid, bot);
  Write(&packed);
  return 0;
}

int CmdHIDDecode(const char *Cmd){
  if (strlen(Cmd)<3) {
    PrintAndLog("Usage:  lf hid decode <id> {p}");
    PrintAndLog("        (optional) p: Ignore invalid parity");
    PrintAndLog("        sample: lf hid decode 2006f623ae");
    return 0;
  }

  uint32_t top = 0, mid = 0, bot = 0;
  bool ignoreParity = false;
  hid_hexstring_to_int96(&top, &mid, &bot, Cmd);
  hidproxmessage_t packed = initialize_proxmessage_object(top, mid, bot);

  char opt = param_getchar(Cmd, 1);
  if (opt == 'p') ignoreParity = true;

  HIDTryUnpack(&packed, ignoreParity);
  return 0;
}
int CmdHIDEncode(const char *Cmd) {
  if (strlen(Cmd) == 0) {
    usage_encode();
    return 0;
  }

  hidproxmessage_t packed;
  memset(&packed, 0, sizeof(hidproxmessage_t));
  if (Encode(Cmd, &packed))
    PrintProxTagId(&packed);
  return 0;
}

int CmdHIDWrite(const char *Cmd) {
  if (strlen(Cmd) == 0) {
    usage_encode();
    return 0;
  }
  hidproxmessage_t packed;
  memset(&packed, 0, sizeof(hidproxmessage_t));
  if (Encode(Cmd, &packed)){
    PrintProxTagId(&packed);
    Write(&packed);
  }
  return 0;
}

int CmdHIDFormats(){
  HIDListFormats();
  return 0;
}
static int CmdHelp(const char *Cmd); // define this now so the below won't error out.
static command_t CommandTable[] = 
{
  {"help",      CmdHelp,        1, "This help"},
  {"demod",     CmdFSKdemodHID, 1, "Demodulate HID Prox from GraphBuffer"},
  {"read",      CmdHIDReadFSK,  0, "['1'] Realtime HID FSK Read from antenna (option '1' for one tag only)"},
  {"sim",       CmdHIDSim,      0, "<ID> -- HID tag simulator"},
  {"clone",     CmdHIDClone,    0, "<ID> -- Clone HID to T55x7 (tag must be in antenna)"},
  {"decode",    CmdHIDDecode,   1, "<ID> -- Try to decode an HID tag and show its contents"},
  {"encode",    CmdHIDEncode,   1, "<format> <fields> -- Encode an HID ID with the specified format and fields"},
  {"formats",   CmdHIDFormats,  1, "List supported card formats"},
  {"write",     CmdHIDWrite,    0, "<format> <fields> -- Encode and write to a T55x7 tag (tag must be in antenna)"},
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
