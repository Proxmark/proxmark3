// -*- mode: c; indent-tabs-mode: nil; tab-width: 3 -*-
//-----------------------------------------------------------------------------
// Copyright (C) 2019 micolous+git@gmail.com
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Commands for KS X 6924 transit cards (T-Money, Snapper+)
//-----------------------------------------------------------------------------
// This is used in T-Money (South Korea) and Snapper plus (Wellington, New
// Zealand).
//
// References:
// - https://github.com/micolous/metrodroid/wiki/T-Money (in English)
// - https://github.com/micolous/metrodroid/wiki/Snapper (in English)
// - https://kssn.net/StdKS/ks_detail.asp?k1=X&k2=6924-1&k3=4
//   (KS X 6924, only available in Korean)
// - http://www.tta.or.kr/include/Download.jsp?filename=stnfile/TTAK.KO-12.0240_%5B2%5D.pdf
//   (TTAK.KO 12.0240, only available in Korean)
//-----------------------------------------------------------------------------


#include "cmdhfksx6924.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "comms.h"
#include "cmdmain.h"
#include "util.h"
#include "ui.h"
#include "proxmark3.h"
#include "cliparser/cliparser.h"
#include "ksx6924/ksx6924core.h"
#include "emv/tlv.h"
#include "emv/apduinfo.h"
#include "cmdhf14a.h"

static int CmdHelp(const char *Cmd);

void getAndPrintBalance() {
   uint32_t balance;
   bool ret = KSX6924GetBalance(&balance);
   if (!ret) {
      PrintAndLog("Error getting balance");
      return;
   }
   
   PrintAndLog("Current balance: %ld won/cents", balance);
}

int CmdHFKSX6924Balance(const char* cmd) {
   CLIParserInit("hf ksx6924 balance",
      "Gets the current purse balance.\n",
      "Usage:\n\thf ksx6924 balance\n");

   void* argtable[] = {
      arg_param_begin,
      arg_lit0("kK",  "keep",    "keep field ON for next command"),
      arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
      arg_param_end
   };
   CLIExecWithReturn(cmd, argtable, true);

   bool leaveSignalON = arg_get_lit(1);
   bool APDULogging = arg_get_lit(2);

   CLIParserFree();
   SetAPDULogging(APDULogging);

   bool ret = KSX6924TrySelect();
   if (!ret) {
      goto end;
   }

   getAndPrintBalance();
   
end:
   if (!leaveSignalON) {
      DropField();
   }
   return 0;
}

int CmdHFKSX6924Info(const char *cmd) {
   CLIParserInit("hf ksx6924 info",
      "Get info about a KS X 6924 transit card.\nThis application is used by T-Money (South Korea) and Snapper+ (Wellington, New Zealand).\n",
      "Usage:\n\thf ksx6924 info\n");

   void* argtable[] = {
      arg_param_begin,
      arg_lit0("kK",  "keep",    "keep field ON for next command"),
      arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
      arg_param_end
   };
   CLIExecWithReturn(cmd, argtable, true);

   bool leaveSignalON = arg_get_lit(1);
   bool APDULogging = arg_get_lit(2);

   CLIParserFree();
   SetAPDULogging(APDULogging);

   // KSX6924 info   
   uint8_t buf[APDU_RESPONSE_LEN] = {0};
   size_t len = 0;
   uint16_t sw = 0;
   int res = KSX6924Select(true, true, buf, sizeof(buf), &len, &sw);

   if (res) {
      if (!leaveSignalON) {
         DropField();
      }
      return res;
   }

   if (sw != 0x9000) {
      if (sw) {
         PrintAndLog("Not a KS X 6924 card! APDU response: %04x - %s",
                     sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff));
      } else {
         PrintAndLog("APDU exchange error. Card returns 0x0000."); 
      }
      goto end;
   }


   // PrintAndLog("APDU response: %s", sprint_hex(buf, len)); 

   // FCI Response is a BER-TLV, we are interested in tag 6F,B0 only.
   const uint8_t* p = buf;
   struct tlv fci_tag;
   
   while (len > 0) {
      memset(&fci_tag, 0, sizeof(fci_tag));
      bool ret = tlv_parse_tl(&p, &len, &fci_tag);
      
      if (!ret) {
         PrintAndLog("Error parsing FCI!");
         goto end;
      }
      
      // PrintAndLog("tag %02x, len %d, value %s",
      //             fci_tag.tag, fci_tag.len,
      //             sprint_hex(p, fci_tag.len));
      
      if (fci_tag.tag == 0x6f) { /* FCI template */
         break;
      } else {
         p += fci_tag.len;
         continue;
      }
   } 
   
   if (fci_tag.tag != 0x6f) {
      PrintAndLog("Couldn't find tag 6F (FCI) in SELECT response");
      goto end;
   }
   
   // We now are at Tag 6F (FCI template), get Tag B0 inside of it
   while (len > 0) {
      memset(&fci_tag, 0, sizeof(fci_tag));
      bool ret = tlv_parse_tl(&p, &len, &fci_tag);
      
      if (!ret) {
         PrintAndLog("Error parsing FCI!");
         goto end;
      }
      
      // PrintAndLog("tag %02x, len %d, value %s",
      //             fci_tag.tag, fci_tag.len,
      //             sprint_hex(p, fci_tag.len));
      
      if (fci_tag.tag == 0xb0) { /* KS X 6924 purse info */
         break;
      } else {
         p += fci_tag.len;
         continue;
      }
   }

   if (fci_tag.tag != 0xb0) {
      PrintAndLog("Couldn't find tag B0 (KS X 6924 purse info) in FCI");
      goto end;
   }
   
   struct ksx6924_purse_info purseInfo;
   bool ret = KSX6924ParsePurseInfo(p, fci_tag.len, &purseInfo);
   
   if (!ret) {
      PrintAndLog("Error parsing KS X 6924 purse info");
      goto end;
   }

   KSX6924PrintPurseInfo(&purseInfo);

   getAndPrintBalance();

end:
   if (!leaveSignalON) {
      DropField();
   }
   return 0;
}

int CmdHFKSX6924Select(const char *cmd) {
   CLIParserInit("hf ksx6924 select",
      "Selects KS X 6924 application, and leaves field up.\n",
      "Usage:\n\thf ksx6924 select\n");

   void* argtable[] = {
      arg_param_begin,
      arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
      arg_param_end
   };
   CLIExecWithReturn(cmd, argtable, true);

   bool APDULogging = arg_get_lit(1);
   CLIParserFree();
   SetAPDULogging(APDULogging);

   bool ret = KSX6924TrySelect();
   if (ret) {
      PrintAndLog("OK");
   } else {
      // Wrong app, drop field.
      DropField();
   }

   return 0;
}

int CmdHFKSX6924PRec(const char *cmd) {   
   CLIParserInit("hf ksx6924 prec",
      "Executes proprietary read record command.\nData format is unknown. Other records are available with 'emv getrec'.\n",
      "Usage:\n\thf ksx6924 prec 0b -> read proprietary record 0x0b\n");

   void* argtable[] = {
      arg_param_begin,
      arg_lit0("kK",  "keep",    "keep field ON for next command"),
      arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
      arg_strx1(NULL,  NULL,     "<record 1byte HEX>", NULL),
      arg_param_end
   };
   CLIExecWithReturn(cmd, argtable, true);

   bool leaveSignalON = arg_get_lit(1);
   bool APDULogging = arg_get_lit(2);
   uint8_t data[APDU_RESPONSE_LEN] = {0};
   int datalen = 0;
   CLIGetHexWithReturn(3, data, &datalen);
   CLIParserFree();
   SetAPDULogging(APDULogging);
   
   if (datalen != 1) {
      PrintAndLog("Record parameter must be 1 byte long (eg: 0f)");
      goto end;
   }
   
   bool ret = KSX6924TrySelect();
   if (!ret) {
      goto end;
   }

   PrintAndLog("Getting record %02x...", data[0]);
   uint8_t recordData[0x10];
   if (!KSX6924ProprietaryGetRecord(data[0], recordData, sizeof(recordData))) {
      PrintAndLog("Error getting record");
      goto end;
   }
   
   PrintAndLog("  %s", sprint_hex(recordData, sizeof(recordData)));

end:
   if (!leaveSignalON) {
      DropField();
   }
   return 0;
}

static command_t CommandTable[] =
{
   {"help",    CmdHelp,             1, "This help."},
   {"info",    CmdHFKSX6924Info,    0, "Get info about a KS X 6924 (T-Money, Snapper+) transit card"},
   {"select",  CmdHFKSX6924Select,  0, "Select application, and leave field up"},
   {"balance", CmdHFKSX6924Balance, 0, "Get current purse balance"},
   {"prec",    CmdHFKSX6924PRec,    0, "Send proprietary get record command (CLA=90, INS=4C)"},
   {NULL,      NULL,                0, NULL}
};

int CmdHFKSX6924(const char *Cmd) {
   (void)WaitForResponseTimeout(CMD_ACK,NULL,100);
   CmdsParse(CommandTable, Cmd);
   return 0;
}

int CmdHelp(const char *Cmd) {
   CmdsHelp(CommandTable);
   return 0;
}

