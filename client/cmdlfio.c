//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency ioProx commands
// FSK2a, rf/64, 64 bits (complete)
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include "cmdlfio.h"
#include "proxmark3.h"
#include "data.h"
#include "graph.h"
#include "ui.h"
#include "cmdparser.h"
#include "cmdmain.h"
#include "cmddata.h"
#include "cmdlf.h"
#include "lfdemod.h"  //for IOdemodFSK + bytebits_to_byte
#include "util.h"     //for sprint_bin_break

static int CmdHelp(const char *Cmd);

int CmdIOReadFSK(const char *Cmd)
{
  int findone=0;
  if(Cmd[0]=='1') findone=1;
	
  UsbCommand c={CMD_IO_DEMOD_FSK};
  c.arg[0]=findone;
  SendCommand(&c);
  return 0;
}

//by marshmellow
//IO-Prox demod - FSK RF/64 with preamble of 000000001
//print ioprox ID and some format details
int CmdFSKdemodIO(const char *Cmd)
{
  int idx=0;
  //something in graphbuffer?
  if (GraphTraceLen < 65) {
    if (g_debugMode)PrintAndLog("DEBUG: not enough samples in GraphBuffer");
    return 0;
  }
  uint8_t BitStream[MAX_GRAPH_TRACE_LEN]={0};
  size_t BitLen = getFromGraphBuf(BitStream);
  if (BitLen==0) return 0;

  int waveIdx = 0;
  //get binary from fsk wave
  idx = IOdemodFSK(BitStream,BitLen, &waveIdx);
  if (idx<0){
    if (g_debugMode){
      if (idx==-1){
        PrintAndLog("DEBUG: Just Noise Detected");     
      } else if (idx == -2) {
        PrintAndLog("DEBUG: not enough samples");
      } else if (idx == -3) {
        PrintAndLog("DEBUG: error during fskdemod");        
      } else if (idx == -4) {
        PrintAndLog("DEBUG: Preamble not found");
      } else if (idx == -5) {
        PrintAndLog("DEBUG: Separator bits not found");
      } else {
        PrintAndLog("DEBUG: Error demoding fsk %d", idx);
      }
    }
    return 0;
  }
  if (idx==0){
    if (g_debugMode){
      PrintAndLog("DEBUG: IO Prox Data not found - FSK Bits: %d",BitLen);
      if (BitLen > 92) PrintAndLog("%s", sprint_bin_break(BitStream,92,16));
    } 
    return 0;
  }
    //Index map
    //0           10          20          30          40          50          60
    //|           |           |           |           |           |           |
    //01234567 8 90123456 7 89012345 6 78901234 5 67890123 4 56789012 3 45678901 23
    //-----------------------------------------------------------------------------
    //00000000 0 11110000 1 facility 1 version* 1 code*one 1 code*two 1 ???????? 11
    //
    //XSF(version)facility:codeone+codetwo (raw)
    //Handle the data
  if (idx+64>BitLen) {
    if (g_debugMode) PrintAndLog("not enough bits found - bitlen: %d",BitLen);
    return 0;
  }
  PrintAndLog("%d%d%d%d%d%d%d%d %d",BitStream[idx],    BitStream[idx+1],  BitStream[idx+2], BitStream[idx+3], BitStream[idx+4], BitStream[idx+5], BitStream[idx+6], BitStream[idx+7], BitStream[idx+8]);
  PrintAndLog("%d%d%d%d%d%d%d%d %d",BitStream[idx+9],  BitStream[idx+10], BitStream[idx+11],BitStream[idx+12],BitStream[idx+13],BitStream[idx+14],BitStream[idx+15],BitStream[idx+16],BitStream[idx+17]);
  PrintAndLog("%d%d%d%d%d%d%d%d %d facility",BitStream[idx+18], BitStream[idx+19], BitStream[idx+20],BitStream[idx+21],BitStream[idx+22],BitStream[idx+23],BitStream[idx+24],BitStream[idx+25],BitStream[idx+26]);
  PrintAndLog("%d%d%d%d%d%d%d%d %d version",BitStream[idx+27], BitStream[idx+28], BitStream[idx+29],BitStream[idx+30],BitStream[idx+31],BitStream[idx+32],BitStream[idx+33],BitStream[idx+34],BitStream[idx+35]);
  PrintAndLog("%d%d%d%d%d%d%d%d %d code1",BitStream[idx+36], BitStream[idx+37], BitStream[idx+38],BitStream[idx+39],BitStream[idx+40],BitStream[idx+41],BitStream[idx+42],BitStream[idx+43],BitStream[idx+44]);
  PrintAndLog("%d%d%d%d%d%d%d%d %d code2",BitStream[idx+45], BitStream[idx+46], BitStream[idx+47],BitStream[idx+48],BitStream[idx+49],BitStream[idx+50],BitStream[idx+51],BitStream[idx+52],BitStream[idx+53]);
  PrintAndLog("%d%d%d%d%d%d%d%d %d%d checksum",BitStream[idx+54],BitStream[idx+55],BitStream[idx+56],BitStream[idx+57],BitStream[idx+58],BitStream[idx+59],BitStream[idx+60],BitStream[idx+61],BitStream[idx+62],BitStream[idx+63]);

  uint32_t code = bytebits_to_byte(BitStream+idx,32);
  uint32_t code2 = bytebits_to_byte(BitStream+idx+32,32);
  uint8_t version = bytebits_to_byte(BitStream+idx+27,8); //14,4
  uint8_t facilitycode = bytebits_to_byte(BitStream+idx+18,8) ;
  uint16_t number = (bytebits_to_byte(BitStream+idx+36,8)<<8)|(bytebits_to_byte(BitStream+idx+45,8)); //36,9
  uint8_t crc = bytebits_to_byte(BitStream+idx+54,8);
  uint16_t calccrc = 0;

  for (uint8_t i=1; i<6; ++i){
    calccrc += bytebits_to_byte(BitStream+idx+9*i,8);
  }
  calccrc &= 0xff;
  calccrc = 0xff - calccrc;

  char *crcStr = (crc == calccrc) ? "crc ok": "!crc";

  PrintAndLog("IO Prox XSF(%02d)%02x:%05d (%08x%08x) [%02x %s]",version,facilitycode,number,code,code2, crc, crcStr);
  setDemodBuf(BitStream,64,idx);
  setClockGrid(64, waveIdx + (idx*64));

  if (g_debugMode){
    PrintAndLog("DEBUG: idx: %d, Len: %d, Printing demod buffer:",idx,64);
    printDemodBuff();
  }
  return 1;
}

int CmdIOClone(const char *Cmd)
{
  unsigned int hi = 0, lo = 0;
  int n = 0, i = 0;
  char ch;
  UsbCommand c;

  while (sscanf(&Cmd[i++], "%1x", &n ) == 1) {
      hi = (hi << 4) | (lo >> 28);
      lo = (lo << 4) | (n & 0xf);
  }

  if (sscanf(&Cmd[--i], "%c", &ch) == 1) {
	  PrintAndLog("Usage:    lf io clone <tag-ID>");
	  return 0;
  }
  
  PrintAndLog("Cloning ioProx tag with ID %08x %08x", hi, lo);

  c.cmd = CMD_IO_CLONE_TAG;
  c.arg[0] = hi;
  c.arg[1] = lo;

  SendCommand(&c);
  return 0;
}

static command_t CommandTable[] = 
{
  {"help",        CmdHelp,            1, "This help"},
  {"demod",       CmdFSKdemodIO,      1, "Demodulate IO Prox tag from the GraphBuffer"},
  {"read",        CmdIOReadFSK,       0, "['1'] Realtime IO FSK demodulate from antenna (option '1' for one tag only)"},
  {"clone",       CmdIOClone,         0, "Clone ioProx Tag"},
  {NULL, NULL, 0, NULL}
};

int CmdLFIO(const char *Cmd)
{
  CmdsParse(CommandTable, Cmd);
  return 0; 
}

int CmdHelp(const char *Cmd)
{
  CmdsHelp(CommandTable);
  return 0;
}
