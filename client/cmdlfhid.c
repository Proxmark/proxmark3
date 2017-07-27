//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency HID commands (known)
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "cmdlfhid.h"
#include "proxmark3.h"
#include "ui.h"
#include "graph.h"
#include "cmdparser.h"
#include "cmddata.h"  //for g_debugMode, demodbuff cmds
#include "lfdemod.h" // for HIDdemodFSK

static int CmdHelp(const char *Cmd);

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
  if (hi2 != 0){ //extra large HID tags
    PrintAndLog("HID Prox TAG ID: %x%08x%08x (%d)",
       (unsigned int) hi2, (unsigned int) hi, (unsigned int) lo, (unsigned int) (lo>>1) & 0xFFFF);
  }
  else {  //standard HID tags <38 bits
    uint8_t fmtLen = 0;
    uint32_t fc = 0;
    uint32_t cardnum = 0;
    if (((hi>>5)&1)==1){//if bit 38 is set then < 37 bit format is used
      uint32_t lo2=0;
      lo2=(((hi & 31) << 12) | (lo>>20)); //get bits 21-37 to check for format len bit
      uint8_t idx3 = 1;
      while(lo2>1){ //find last bit set to 1 (format len bit)
        lo2=lo2>>1;
        idx3++;
      }
      fmtLen =idx3+19;
      fc =0;
      cardnum=0;
      if(fmtLen==26){
        cardnum = (lo>>1)&0xFFFF;
        fc = (lo>>17)&0xFF;
      }
      if(fmtLen==34){
        cardnum = (lo>>1)&0xFFFF;
        fc= ((hi&1)<<15)|(lo>>17);
      }
      if(fmtLen==35){
        cardnum = (lo>>1)&0xFFFFF;
        fc = ((hi&1)<<11)|(lo>>21);
      }
    }
    else { //if bit 38 is not set then 37 bit format is used
      fmtLen = 37;
      fc = 0;
      cardnum = 0;
      if(fmtLen == 37){
        cardnum = (lo>>1)&0x7FFFF;
        fc = ((hi&0xF)<<12)|(lo>>20);
      }
    }
    PrintAndLog("HID Prox TAG ID: %x%08x (%d) - Format Len: %dbit - FC: %d - Card: %d",
      (unsigned int) hi, (unsigned int) lo, (unsigned int) (lo>>1) & 0xFFFF,
      (unsigned int) fmtLen, (unsigned int) fc, (unsigned int) cardnum);
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
	uint32_t hi = 0, lo = 0;
	int n = 0, i = 0;

	while (sscanf(&Cmd[i++], "%1x", &n ) == 1) {
		hi = (hi << 4) | (lo >> 28);
		lo = (lo << 4) | (n & 0xf);
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
  int n = 0, i = 0;
  UsbCommand c;

  if (strchr(Cmd,'l') != 0) {
  	while (sscanf(&Cmd[i++], "%1x", &n ) == 1) {
      hi2 = (hi2 << 4) | (hi >> 28);
      hi = (hi << 4) | (lo >> 28);
      lo = (lo << 4) | (n & 0xf);
    }

    PrintAndLog("Cloning tag with long ID %x%08x%08x", hi2, hi, lo);

    c.d.asBytes[0] = 1;
  }
  else {
  	while (sscanf(&Cmd[i++], "%1x", &n ) == 1) {
      hi = (hi << 4) | (lo >> 28);
      lo = (lo << 4) | (n & 0xf);
    }

    PrintAndLog("Cloning tag with ID %x%08x", hi, lo);

    hi2 = 0;
    c.d.asBytes[0] = 0;
  }

  c.cmd = CMD_HID_CLONE_TAG;
  c.arg[0] = hi2;
  c.arg[1] = hi;
  c.arg[2] = lo;

  SendCommand(&c);
  return 0;
}

static command_t CommandTable[] = 
{
  {"help",      CmdHelp,        1, "This help"},
  {"demod",     CmdFSKdemodHID, 1, "Demodulate HID Prox from GraphBuffer"},
  {"read",      CmdHIDReadFSK,  0, "['1'] Realtime HID FSK Read from antenna (option '1' for one tag only)"},
  {"sim",       CmdHIDSim,      0, "<ID> -- HID tag simulator"},
  {"clone",     CmdHIDClone,    0, "<ID> ['l'] -- Clone HID to T55x7 (tag must be in antenna)(option 'l' for 84bit ID)"},
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
