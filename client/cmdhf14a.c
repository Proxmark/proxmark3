//-----------------------------------------------------------------------------
// 2011, 2017 Merlok
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>, Hagen Fritsch
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency ISO14443A commands
//-----------------------------------------------------------------------------

#include "cmdhf14a.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "util.h"
#include "util_posix.h"
#include "iso14443crc.h"
#include "comms.h"
#include "ui.h"
#include "cmdparser.h"
#include "common.h"
#include "cmdmain.h"
#include "mifare.h"
#include "cmdhfmfu.h"
#include "mifarehost.h"
#include "cliparser/cliparser.h"
#include "emv/apduinfo.h"
#include "emv/emvcore.h"

static int CmdHelp(const char *Cmd);
static int waitCmd(uint8_t iLen);

// structure and database for uid -> tagtype lookups 
typedef struct { 
	uint8_t uid;
	char* desc;
} manufactureName; 

static const manufactureName manufactureMapping[] = {
	// ID,  "Vendor Country"
	{ 0x01, "Motorola UK" },
	{ 0x02, "ST Microelectronics SA France" },
	{ 0x03, "Hitachi, Ltd Japan" }, 
	{ 0x04, "NXP Semiconductors Germany" }, 
	{ 0x05, "Infineon Technologies AG Germany" }, 
	{ 0x06, "Cylink USA" }, 
	{ 0x07, "Texas Instrument France" },
	{ 0x08, "Fujitsu Limited Japan" }, 
	{ 0x09, "Matsushita Electronics Corporation, Semiconductor Company Japan" }, 
	{ 0x0A, "NEC Japan" }, 
	{ 0x0B, "Oki Electric Industry Co. Ltd Japan" },
	{ 0x0C, "Toshiba Corp. Japan" },
	{ 0x0D, "Mitsubishi Electric Corp. Japan" },
	{ 0x0E, "Samsung Electronics Co. Ltd Korea" },
	{ 0x0F, "Hynix / Hyundai, Korea" },
	{ 0x10, "LG-Semiconductors Co. Ltd Korea" },
	{ 0x11, "Emosyn-EM Microelectronics USA" },
	{ 0x12, "INSIDE Technology France" },
	{ 0x13, "ORGA Kartensysteme GmbH Germany" },
	{ 0x14, "SHARP Corporation Japan" },
	{ 0x15, "ATMEL France" },
	{ 0x16, "EM Microelectronic-Marin SA Switzerland" },
	{ 0x17, "KSW Microtec GmbH Germany" },
	{ 0x18, "ZMD AG Germany" },
	{ 0x19, "XICOR, Inc. USA" },
	{ 0x1A, "Sony Corporation Japan Identifier Company Country" },
	{ 0x1B, "Malaysia Microelectronic Solutions Sdn. Bhd Malaysia" },
	{ 0x1C, "Emosyn USA" },
	{ 0x1D, "Shanghai Fudan Microelectronics Co. Ltd. P.R. China" },
	{ 0x1E, "Magellan Technology Pty Limited Australia" },
	{ 0x1F, "Melexis NV BO Switzerland" },
	{ 0x20, "Renesas Technology Corp. Japan" },
	{ 0x21, "TAGSYS France" },
	{ 0x22, "Transcore USA" },
	{ 0x23, "Shanghai belling corp., ltd. China" },
	{ 0x24, "Masktech Germany Gmbh Germany" },
	{ 0x25, "Innovision Research and Technology Plc UK" },
	{ 0x26, "Hitachi ULSI Systems Co., Ltd. Japan" },
	{ 0x27, "Cypak AB Sweden" },
	{ 0x28, "Ricoh Japan" },
	{ 0x29, "ASK France" },
	{ 0x2A, "Unicore Microsystems, LLC Russian Federation" },
	{ 0x2B, "Dallas Semiconductor/Maxim USA" },
	{ 0x2C, "Impinj, Inc. USA" },
	{ 0x2D, "RightPlug Alliance USA" },
	{ 0x2E, "Broadcom Corporation USA" },
	{ 0x2F, "MStar Semiconductor, Inc Taiwan, ROC" },
	{ 0x30, "BeeDar Technology Inc. USA" },
	{ 0x31, "RFIDsec Denmark" },
	{ 0x32, "Schweizer Electronic AG Germany" },
	{ 0x33, "AMIC Technology Corp Taiwan" }, 
	{ 0x34, "Mikron JSC Russia" },
	{ 0x35, "Fraunhofer Institute for Photonic Microsystems Germany" },
	{ 0x36, "IDS Microchip AG Switzerland" },
	{ 0x37, "Kovio USA" },
	{ 0x38, "HMT Microelectronic Ltd Switzerland Identifier Company Country" },
	{ 0x39, "Silicon Craft Technology Thailand" },
	{ 0x3A, "Advanced Film Device Inc. Japan" },
	{ 0x3B, "Nitecrest Ltd UK" },
	{ 0x3C, "Verayo Inc. USA" },
	{ 0x3D, "HID Global USA" },
	{ 0x3E, "Productivity Engineering Gmbh Germany" },
	{ 0x3F, "Austriamicrosystems AG (reserved) Austria" }, 
	{ 0x40, "Gemalto SA France" },
	{ 0x41, "Renesas Electronics Corporation Japan" },
	{ 0x42, "3Alogics Inc Korea" },
	{ 0x43, "Top TroniQ Asia Limited Hong Kong" },
	{ 0x44, "Gentag Inc (USA) USA" },
	{ 0x00, "no tag-info available" } // must be the last entry
};

// get a product description based on the UID
//		uid[8] 	tag uid
// returns description of the best match	
char* getTagInfo(uint8_t uid) {

	int i;
	int len = sizeof(manufactureMapping) / sizeof(manufactureName);
	
	for ( i = 0; i < len; ++i ) 
		if ( uid == manufactureMapping[i].uid) 
			return manufactureMapping[i].desc;

	//No match, return default
	return manufactureMapping[len-1].desc; 
}

int CmdHF14AList(const char *Cmd)
{
	PrintAndLog("Deprecated command, use 'hf list 14a' instead");
	return 0;
}

int CmdHF14AReader(const char *Cmd) {
	uint32_t cm = ISO14A_CONNECT;
	bool leaveSignalON = false;
	
	CLIParserInit("hf 14a reader", "Executes ISO1443A anticollision-select group of commands.", NULL);
	void* argtable[] = {
		arg_param_begin,
		arg_lit0("kK",  "keep",    "keep the field active after command executed"),
		arg_lit0("xX",  "drop",    "just drop the signal field"),
		arg_lit0("3",   NULL,      "ISO14443-3 select only (skip RATS)"),
		arg_param_end
	};
	if (CLIParserParseString(Cmd, argtable, arg_getsize(argtable), true)){
		CLIParserFree();
		return 0;
	}
	
	leaveSignalON = arg_get_lit(1);
	if (arg_get_lit(2)) {
		cm = cm - ISO14A_CONNECT;
	}
	if (arg_get_lit(3)) {
		cm |= ISO14A_NO_RATS;
	}
	
	CLIParserFree();
	
	if (leaveSignalON)
		cm |= ISO14A_NO_DISCONNECT; 
	
	UsbCommand c = {CMD_READER_ISO_14443a, {cm, 0, 0}};
	SendCommand(&c);

	if (ISO14A_CONNECT & cm) {
		UsbCommand resp;
		WaitForResponse(CMD_ACK,&resp);
		
		iso14a_card_select_t card;
		memcpy(&card, (iso14a_card_select_t *)resp.d.asBytes, sizeof(iso14a_card_select_t));

		uint64_t select_status = resp.arg[0];		// 0: couldn't read, 1: OK, with ATS, 2: OK, no ATS, 3: proprietary Anticollision
		
		if(select_status == 0) {
			PrintAndLog("iso14443a card select failed");
			return 1;
		}

		if(select_status == 3) {
			PrintAndLog("Card doesn't support standard iso14443-3 anticollision");
			PrintAndLog("ATQA : %02x %02x", card.atqa[1], card.atqa[0]);
			return 1;
		}

		PrintAndLog(" UID : %s", sprint_hex(card.uid, card.uidlen));
		PrintAndLog("ATQA : %02x %02x", card.atqa[1], card.atqa[0]);
		PrintAndLog(" SAK : %02x [%" PRIu64 "]", card.sak, resp.arg[0]);
		if(card.ats_len >= 3) {			// a valid ATS consists of at least the length byte (TL) and 2 CRC bytes
			PrintAndLog(" ATS : %s", sprint_hex(card.ats, card.ats_len));
		}
		if (leaveSignalON) {
			PrintAndLog("Card is selected. You can now start sending commands");
		}
	}

	if (!leaveSignalON) {
		PrintAndLog("Field dropped.");
	}
	
	return 0;
}

int CmdHF14AInfo(const char *Cmd)
{
	UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_CONNECT | ISO14A_NO_DISCONNECT, 0, 0}};
	SendCommand(&c);

	UsbCommand resp;
	WaitForResponse(CMD_ACK,&resp);
	
	iso14a_card_select_t card;
	memcpy(&card, (iso14a_card_select_t *)resp.d.asBytes, sizeof(iso14a_card_select_t));

	uint64_t select_status = resp.arg[0];		// 0: couldn't read, 1: OK, with ATS, 2: OK, no ATS, 3: proprietary Anticollision
	
	if(select_status == 0) {
		if (Cmd[0] != 's') PrintAndLog("iso14443a card select failed");
		// disconnect
		c.arg[0] = 0;
		c.arg[1] = 0;
		c.arg[2] = 0;
		SendCommand(&c);
		return 0;
	}

	if(select_status == 3) {
		PrintAndLog("Card doesn't support standard iso14443-3 anticollision");
		PrintAndLog("ATQA : %02x %02x", card.atqa[1], card.atqa[0]);
		// disconnect
		c.arg[0] = 0;
		c.arg[1] = 0;
		c.arg[2] = 0;
		SendCommand(&c);
		return 0;
	}

	PrintAndLog(" UID : %s", sprint_hex(card.uid, card.uidlen));
	PrintAndLog("ATQA : %02x %02x", card.atqa[1], card.atqa[0]);
	PrintAndLog(" SAK : %02x [%" PRIu64 "]", card.sak, resp.arg[0]);

	bool isMifareClassic = true;
	switch (card.sak) {
		case 0x00: 
			isMifareClassic = false;

			//***************************************test****************
			// disconnect
			c.arg[0] = 0;
			c.arg[1] = 0;
			c.arg[2] = 0;
			SendCommand(&c);
			
			uint32_t tagT = GetHF14AMfU_Type();
			ul_print_type(tagT, 0);

			//reconnect for further tests
			c.arg[0] = ISO14A_CONNECT | ISO14A_NO_DISCONNECT;
			c.arg[1] = 0;
			c.arg[2] = 0;

			SendCommand(&c);

			UsbCommand resp;
			WaitForResponse(CMD_ACK,&resp);
			
			memcpy(&card, (iso14a_card_select_t *)resp.d.asBytes, sizeof(iso14a_card_select_t));

			select_status = resp.arg[0];		// 0: couldn't read, 1: OK, with ATS, 2: OK, no ATS
			
			if(select_status == 0) {
				//PrintAndLog("iso14443a card select failed");
				// disconnect
				c.arg[0] = 0;
				c.arg[1] = 0;
				c.arg[2] = 0;
				SendCommand(&c);
				return 0;
			}

			/*  orig
			// check if the tag answers to GETVERSION (0x60)
			c.arg[0] = ISO14A_RAW | ISO14A_APPEND_CRC | ISO14A_NO_DISCONNECT;
			c.arg[1] = 1;
			c.arg[2] = 0;
			c.d.asBytes[0] = 0x60;
			SendCommand(&c);
			WaitForResponse(CMD_ACK,&resp);

			uint8_t version[10] = {0};
			memcpy(version, resp.d.asBytes, resp.arg[0] < sizeof(version) ? resp.arg[0] : sizeof(version));
			uint8_t len = resp.arg[0] & 0xff;
			switch ( len ){
				// todo, identify "Magic UL-C tags". // they usually have a static nonce response to 0x1A command.
				// UL-EV1, size, check version[6] == 0x0b (smaller)  0x0b * 4 == 48
				case 0x0A:PrintAndLog("TYPE : NXP MIFARE Ultralight EV1 %d bytes", (version[6] == 0xB) ? 48 : 128);break;
				case 0x01:PrintAndLog("TYPE : NXP MIFARE Ultralight C");break;
				case 0x00:PrintAndLog("TYPE : NXP MIFARE Ultralight");break;	
			}
			*/
			break;
		case 0x01: PrintAndLog("TYPE : NXP TNP3xxx Activision Game Appliance"); break;
		case 0x04: PrintAndLog("TYPE : NXP MIFARE (various !DESFire !DESFire EV1)"); break;
		case 0x08: PrintAndLog("TYPE : NXP MIFARE CLASSIC 1k | Plus 2k SL1"); break;
		case 0x09: PrintAndLog("TYPE : NXP MIFARE Mini 0.3k"); break;
		case 0x10: PrintAndLog("TYPE : NXP MIFARE Plus 2k SL2"); break;
		case 0x11: PrintAndLog("TYPE : NXP MIFARE Plus 4k SL2"); break;
		case 0x18: PrintAndLog("TYPE : NXP MIFARE Classic 4k | Plus 4k SL1"); break;
		case 0x20: PrintAndLog("TYPE : NXP MIFARE DESFire 4k | DESFire EV1 2k/4k/8k | Plus 2k/4k SL3 | JCOP 31/41"); break;
		case 0x24: PrintAndLog("TYPE : NXP MIFARE DESFire | DESFire EV1"); break;
		case 0x28: PrintAndLog("TYPE : JCOP31 or JCOP41 v2.3.1"); break;
		case 0x38: PrintAndLog("TYPE : Nokia 6212 or 6131 MIFARE CLASSIC 4K"); break;
		case 0x88: PrintAndLog("TYPE : Infineon MIFARE CLASSIC 1K"); break;
		case 0x98: PrintAndLog("TYPE : Gemplus MPCOS"); break;
		default: ;
	}

	// Double & triple sized UID, can be mapped to a manufacturer.
	// HACK: does this apply for Ultralight cards?
	if ( card.uidlen > 4 ) {
		PrintAndLog("MANUFACTURER : %s", getTagInfo(card.uid[0]));
	}

	// try to request ATS even if tag claims not to support it
	if (select_status == 2) {
		uint8_t rats[] = { 0xE0, 0x80 }; // FSDI=8 (FSD=256), CID=0
		c.arg[0] = ISO14A_RAW | ISO14A_APPEND_CRC | ISO14A_NO_DISCONNECT;
		c.arg[1] = 2;
		c.arg[2] = 0;
		memcpy(c.d.asBytes, rats, 2);
		SendCommand(&c);
		WaitForResponse(CMD_ACK,&resp);
		
	    memcpy(card.ats, resp.d.asBytes, resp.arg[0]);
		card.ats_len = resp.arg[0];				// note: ats_len includes CRC Bytes
	} 

	if(card.ats_len >= 3) {			// a valid ATS consists of at least the length byte (TL) and 2 CRC bytes
		bool ta1 = 0, tb1 = 0, tc1 = 0;
		int pos;

		if (select_status == 2) {
			PrintAndLog("SAK incorrectly claims that card doesn't support RATS");
		}
		PrintAndLog(" ATS : %s", sprint_hex(card.ats, card.ats_len));
		PrintAndLog("       -  TL : length is %d bytes", card.ats[0]);
		if (card.ats[0] != card.ats_len - 2) {
			PrintAndLog("ATS may be corrupted. Length of ATS (%d bytes incl. 2 Bytes CRC) doesn't match TL", card.ats_len);
		}
		
		if (card.ats[0] > 1) {		// there is a format byte (T0)
			ta1 = (card.ats[1] & 0x10) == 0x10;
			tb1 = (card.ats[1] & 0x20) == 0x20;
			tc1 = (card.ats[1] & 0x40) == 0x40;
			int16_t fsci = card.ats[1] & 0x0f;
			PrintAndLog("       -  T0 : TA1 is%s present, TB1 is%s present, "
					"TC1 is%s present, FSCI is %d (FSC = %ld)",
				(ta1 ? "" : " NOT"), (tb1 ? "" : " NOT"), (tc1 ? "" : " NOT"),
				fsci,
				fsci < 5 ? (fsci - 2) * 8 : 
					fsci < 8 ? (fsci - 3) * 32 :
					fsci == 8 ? 256 :
					-1
				);
		}
		pos = 2;
		if (ta1) {
			char dr[16], ds[16];
			dr[0] = ds[0] = '\0';
			if (card.ats[pos] & 0x10) strcat(ds, "2, ");
			if (card.ats[pos] & 0x20) strcat(ds, "4, ");
			if (card.ats[pos] & 0x40) strcat(ds, "8, ");
			if (card.ats[pos] & 0x01) strcat(dr, "2, ");
			if (card.ats[pos] & 0x02) strcat(dr, "4, ");
			if (card.ats[pos] & 0x04) strcat(dr, "8, ");
			if (strlen(ds) != 0) ds[strlen(ds) - 2] = '\0';
			if (strlen(dr) != 0) dr[strlen(dr) - 2] = '\0';
			PrintAndLog("       - TA1 : different divisors are%s supported, "
					"DR: [%s], DS: [%s]",
					(card.ats[pos] & 0x80 ? " NOT" : ""), dr, ds);
			pos++;
		}
		if (tb1) {
			uint32_t sfgi = card.ats[pos] & 0x0F;
			uint32_t fwi = card.ats[pos] >> 4;
			PrintAndLog("       - TB1 : SFGI = %d (SFGT = %s%ld/fc), FWI = %d (FWT = %ld/fc)",
					(sfgi),
					sfgi ? "" : "(not needed) ",
					sfgi ? (1 << 12) << sfgi : 0,
					fwi,
					(1 << 12) << fwi
					);
			pos++;
		}
		if (tc1) {
			PrintAndLog("       - TC1 : NAD is%s supported, CID is%s supported",
					(card.ats[pos] & 0x01) ? "" : " NOT",
					(card.ats[pos] & 0x02) ? "" : " NOT");
			pos++;
		}
		if (card.ats[0] > pos) {
			char *tip = "";
			if (card.ats[0] - pos >= 7) {
				if (memcmp(card.ats + pos, "\xC1\x05\x2F\x2F\x01\xBC\xD6", 7) == 0) {
					tip = "-> MIFARE Plus X 2K or 4K";
				} else if (memcmp(card.ats + pos, "\xC1\x05\x2F\x2F\x00\x35\xC7", 7) == 0) {
					tip = "-> MIFARE Plus S 2K or 4K";
				}
			} 
			PrintAndLog("       -  HB : %s%s", sprint_hex(card.ats + pos, card.ats[0] - pos), tip);
			if (card.ats[pos] == 0xC1) {
				PrintAndLog("               c1 -> Mifare or (multiple) virtual cards of various type");
				PrintAndLog("                  %02x -> Length is %d bytes",
						card.ats[pos + 1], card.ats[pos + 1]);
				switch (card.ats[pos + 2] & 0xf0) {
					case 0x10:
						PrintAndLog("                     1x -> MIFARE DESFire");
						break;
					case 0x20:
						PrintAndLog("                     2x -> MIFARE Plus");
						break;
				}
				switch (card.ats[pos + 2] & 0x0f) {
					case 0x00:
						PrintAndLog("                     x0 -> <1 kByte");
						break;
					case 0x01:
						PrintAndLog("                     x1 -> 1 kByte");
						break;
					case 0x02:
						PrintAndLog("                     x2 -> 2 kByte");
						break;
					case 0x03:
						PrintAndLog("                     x3 -> 4 kByte");
						break;
					case 0x04:
						PrintAndLog("                     x4 -> 8 kByte");
						break;
				}
				switch (card.ats[pos + 3] & 0xf0) {
					case 0x00:
						PrintAndLog("                        0x -> Engineering sample");
						break;
					case 0x20:
						PrintAndLog("                        2x -> Released");
						break;
				}
				switch (card.ats[pos + 3] & 0x0f) {
					case 0x00:
						PrintAndLog("                        x0 -> Generation 1");
						break;
					case 0x01:
						PrintAndLog("                        x1 -> Generation 2");
						break;
					case 0x02:
						PrintAndLog("                        x2 -> Generation 3");
						break;
				}
				switch (card.ats[pos + 4] & 0x0f) {
					case 0x00:
						PrintAndLog("                           x0 -> Only VCSL supported");
						break;
					case 0x01:
						PrintAndLog("                           x1 -> VCS, VCSL, and SVC supported");
						break;
					case 0x0E:
						PrintAndLog("                           xE -> no VCS command supported");
						break;
				}
			}
		}
	} else {
		PrintAndLog("proprietary non iso14443-4 card found, RATS not supported");
	}

	
	// try to see if card responses to "chinese magic backdoor" commands.
	(void)mfCIdentify();
	
	if (isMifareClassic) {		
		switch(DetectClassicPrng()) {
		case 0:
			PrintAndLog("Prng detection: HARDENED (hardnested)");		
			break;
		case 1:
			PrintAndLog("Prng detection: WEAK");
			break;
		default:
			PrintAndLog("Prng detection error.");		
		}
	}
	
	return select_status;
}

// Collect ISO14443 Type A UIDs
int CmdHF14ACUIDs(const char *Cmd)
{
	// requested number of UIDs
	int n = atoi(Cmd);
	// collect at least 1 (e.g. if no parameter was given)
	n = n > 0 ? n : 1;

	PrintAndLog("Collecting %d UIDs", n);
	PrintAndLog("Start: %" PRIu64, msclock()/1000);
	// repeat n times
	for (int i = 0; i < n; i++) {
		// execute anticollision procedure
		UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_CONNECT | ISO14A_NO_RATS, 0, 0}};
		SendCommand(&c);
    
		UsbCommand resp;
		WaitForResponse(CMD_ACK,&resp);

		iso14a_card_select_t *card = (iso14a_card_select_t *) resp.d.asBytes;

		// check if command failed
		if (resp.arg[0] == 0) {
			PrintAndLog("Card select failed.");
		} else {
			char uid_string[20];
			for (uint16_t i = 0; i < card->uidlen; i++) {
				sprintf(&uid_string[2*i], "%02X", card->uid[i]);
			}
			PrintAndLog("%s", uid_string);
		}
	}
	PrintAndLog("End: %" PRIu64, msclock()/1000);

	return 1;
}

// ## simulate iso14443a tag
// ## greg - added ability to specify tag UID
int CmdHF14ASim(const char *Cmd)
{
	UsbCommand c = {CMD_SIMULATE_TAG_ISO_14443a,{0,0,0}};
	
	// Retrieve the tag type
	uint8_t tagtype = param_get8ex(Cmd,0,0,10);
	
	// When no argument was given, just print help message
	if (tagtype == 0) {
		PrintAndLog("");
		PrintAndLog(" Emulating ISO/IEC 14443 type A tag with 4 or 7 byte UID");
		PrintAndLog("");
		PrintAndLog("   syntax: hf 14a sim <type> <uid>");
		PrintAndLog("    types: 1 = MIFARE Classic");
		PrintAndLog("           2 = MIFARE Ultralight");
		PrintAndLog("           3 = MIFARE Desfire");
		PrintAndLog("           4 = ISO/IEC 14443-4");
		PrintAndLog("           5 = MIFARE Tnp3xxx");		
		PrintAndLog("");
		return 1;
	}
	
	// Store the tag type
	c.arg[0] = tagtype;
	
	// Retrieve the full 4 or 7 byte long uid 
	uint64_t long_uid = param_get64ex(Cmd,1,0,16);

	// Are we handling the (optional) second part uid?
	if (long_uid > 0xffffffff) {
		PrintAndLog("Emulating ISO/IEC 14443 type A tag with 7 byte UID (%014" PRIx64 ")",long_uid);
		// Store the second part
		c.arg[2] = (long_uid & 0xffffffff);
		long_uid >>= 32;
		// Store the first part, ignore the first byte, it is replaced by cascade byte (0x88)
		c.arg[1] = (long_uid & 0xffffff);
	} else {
		PrintAndLog("Emulating ISO/IEC 14443 type A tag with 4 byte UID (%08x)",long_uid);
		// Only store the first part
		c.arg[1] = long_uid & 0xffffffff;
	}
/*
		// At lease save the mandatory first part of the UID
		c.arg[0] = long_uid & 0xffffffff;

	if (c.arg[1] == 0) {
		PrintAndLog("Emulating ISO/IEC 14443 type A tag with UID %01d %08x %08x",c.arg[0],c.arg[1],c.arg[2]);
	}
	
	switch (c.arg[0]) {
		case 1: {
			PrintAndLog("Emulating ISO/IEC 14443-3 type A tag with 4 byte UID");
			UsbCommand c = {CMD_SIMULATE_TAG_ISO_14443a,param_get32ex(Cmd,0,0,10),param_get32ex(Cmd,1,0,16),param_get32ex(Cmd,2,0,16)};
		} break;
		case 2: {
			PrintAndLog("Emulating ISO/IEC 14443-4 type A tag with 7 byte UID");
		} break;
		default: {
			PrintAndLog("Error: unkown tag type (%d)",c.arg[0]);
			PrintAndLog("syntax: hf 14a sim <uid>",c.arg[0]);
			PrintAndLog(" type1: 4 ",c.arg[0]);

			return 1;
		} break;
	}	
*/
/*
  unsigned int hi = 0, lo = 0;
  int n = 0, i = 0;
  while (sscanf(&Cmd[i++], "%1x", &n ) == 1) {
    hi= (hi << 4) | (lo >> 28);
    lo= (lo << 4) | (n & 0xf);
  }
*/
//	UsbCommand c = {CMD_SIMULATE_TAG_ISO_14443a,param_get32ex(Cmd,0,0,10),param_get32ex(Cmd,1,0,16),param_get32ex(Cmd,2,0,16)};
//  PrintAndLog("Emulating ISO/IEC 14443 type A tag with UID %01d %08x %08x",c.arg[0],c.arg[1],c.arg[2]);
  SendCommand(&c);
  return 0;
}

int CmdHF14ASnoop(const char *Cmd) {
	int param = 0;
	
	uint8_t ctmp = param_getchar(Cmd, 0) ;
	if (ctmp == 'h' || ctmp == 'H') {
		PrintAndLog("It get data from the field and saves it into command buffer.");
		PrintAndLog("Buffer accessible from command hf list 14a.");
		PrintAndLog("Usage:  hf 14a snoop [c][r]");
		PrintAndLog("c - triggered by first data from card");
		PrintAndLog("r - triggered by first 7-bit request from reader (REQ,WUP,...)");
		PrintAndLog("sample: hf 14a snoop c r");
		return 0;
	}	
	
	for (int i = 0; i < 2; i++) {
		ctmp = param_getchar(Cmd, i);
		if (ctmp == 'c' || ctmp == 'C') param |= 0x01;
		if (ctmp == 'r' || ctmp == 'R') param |= 0x02;
	}

	UsbCommand c = {CMD_SNOOP_ISO_14443a, {param, 0, 0}};
	SendCommand(&c);
	return 0;
}

void DropField() {
	UsbCommand c = {CMD_READER_ISO_14443a, {0, 0, 0}}; 
	SendCommand(&c);
}

int ExchangeAPDU14a(uint8_t *datain, int datainlen, bool activateField, bool leaveSignalON, uint8_t *dataout, int maxdataoutlen, int *dataoutlen) {
	uint16_t cmdc = 0;
	
	if (activateField) {
		cmdc |= ISO14A_CONNECT | ISO14A_CLEAR_TRACE;
	}
	if (leaveSignalON)
		cmdc |= ISO14A_NO_DISCONNECT;

	// "Command APDU" length should be 5+255+1, but javacard's APDU buffer might be smaller - 133 bytes
	// https://stackoverflow.com/questions/32994936/safe-max-java-card-apdu-data-command-and-respond-size
	// here length USB_CMD_DATA_SIZE=512
	// timeout must be authomatically set by "get ATS"
	UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_APDU | cmdc, (datainlen & 0xFFFF), 0}}; 
	memcpy(c.d.asBytes, datain, datainlen);
	SendCommand(&c);
	
    uint8_t *recv;
    UsbCommand resp;

	if (activateField) {
		if (!WaitForResponseTimeout(CMD_ACK, &resp, 1500)) {
			PrintAndLog("APDU ERROR: Proxmark connection timeout.");
			return 1;
		}
		if (resp.arg[0] != 1) {
			PrintAndLog("APDU ERROR: Proxmark error %d.", resp.arg[0]);
			return 1;
		}
	}

    if (WaitForResponseTimeout(CMD_ACK, &resp, 1500)) {
        recv = resp.d.asBytes;
        int iLen = resp.arg[0];
		
		*dataoutlen = iLen - 2;
		if (*dataoutlen < 0)
			*dataoutlen = 0;
		
		if (maxdataoutlen && *dataoutlen > maxdataoutlen) {
			PrintAndLog("APDU ERROR: Buffer too small(%d). Needs %d bytes", *dataoutlen, maxdataoutlen);
			return 2;
		}
		
		memcpy(dataout, recv, *dataoutlen);
		
        if(!iLen) {
			PrintAndLog("APDU ERROR: No APDU response.");
            return 1;
		}

		// check block TODO
		if (iLen == -2) {
			PrintAndLog("APDU ERROR: Block type mismatch.");
			return 2;
		}
		
		// CRC Check
		if (iLen == -1) {
			PrintAndLog("APDU ERROR: ISO 14443A CRC error.");
			return 3;
		}

		// check apdu length
		if (iLen < 4) {
			PrintAndLog("APDU ERROR: Small APDU response. Len=%d", iLen);
			return 2;
		}
		
    } else {
        PrintAndLog("APDU ERROR: Reply timeout.");
		return 4;
    }
	
	return 0;
}

// ISO14443-4. 7. Half-duplex block transmission protocol
int CmdHF14AAPDU(const char *cmd) {
	uint8_t data[USB_CMD_DATA_SIZE];
	int datalen = 0;
	bool activateField = false;
	bool leaveSignalON = false;
	bool decodeTLV = false;

	CLIParserInit("hf 14a apdu", 
		"Sends an ISO 7816-4 APDU via ISO 14443-4 block transmission protocol (T=CL)", 
		"Sample:\n\thf 14a apdu -st 00A404000E325041592E5359532E444446303100\n");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("sS",  "select",  "activate field and select card"),
		arg_lit0("kK",  "keep",    "leave the signal field ON after receive response"),
		arg_lit0("tT",  "tlv",     "executes TLV decoder if it possible"),
		arg_strx1(NULL, NULL,      "<APDU (hex)>", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, false);
	
	activateField = arg_get_lit(1);
	leaveSignalON = arg_get_lit(2);
	decodeTLV = arg_get_lit(3);
	// len = data + PCB(1b) + CRC(2b)
	CLIGetStrBLessWithReturn(4, data, &datalen, 1 + 2);


	CLIParserFree();
//	PrintAndLog("---str [%d] %s", arg_get_str(4)->count, arg_get_str(4)->sval[0]);
	PrintAndLog(">>>>[%s%s%s] %s", activateField ? "sel ": "", leaveSignalON ? "keep ": "", decodeTLV ? "TLV": "", sprint_hex(data, datalen));
	
	int res = ExchangeAPDU14a(data, datalen, activateField, leaveSignalON, data, USB_CMD_DATA_SIZE, &datalen);

	if (res)
		return res;

	PrintAndLog("<<<< %s", sprint_hex(data, datalen));
	
	PrintAndLog("APDU response: %02x %02x - %s", data[datalen - 2], data[datalen - 1], GetAPDUCodeDescription(data[datalen - 2], data[datalen - 1])); 

	// TLV decoder
	if (decodeTLV && datalen > 4) {
		TLVPrintFromBuffer(data, datalen - 2);
	}
	
	return 0;
}

int CmdHF14ACmdRaw(const char *cmd) {
	UsbCommand c = {CMD_READER_ISO_14443a, {0, 0, 0}};
	bool reply=1;
	bool crc = false;
	bool power = false;
	bool active = false;
	bool active_select = false;
	bool no_rats = false;
	uint16_t numbits = 0;
	bool bTimeout = false;
	uint32_t timeout = 0;
	bool topazmode = false;
	uint8_t data[USB_CMD_DATA_SIZE];
	int datalen = 0;

	// extract parameters
	CLIParserInit("hf 14a raw", "Send raw hex data to tag", 
		"Sample:\n"\
		"\thf 14a raw -pa -b7 -t1000 52  -- execute WUPA\n"\
		"\thf 14a raw -p 9320            -- anticollision\n"\
		"\thf 14a raw -psc 60 00         -- select and mifare AUTH\n");
	void* argtable[] = {
		arg_param_begin,
		arg_lit0("rR",  "nreply",  "do not read response"),
		arg_lit0("cC",  "crc",     "calculate and append CRC"),
		arg_lit0("pP",  "power",   "leave the signal field ON after receive"),
		arg_lit0("aA",  "active",  "active signal field ON without select"),
		arg_lit0("sS",  "actives", "active signal field ON with select"),
		arg_int0("bB",  "bits",    NULL, "number of bits to send. Useful for send partial byte"),
		arg_int0("t",   "timeout", NULL, "timeout in ms"),
		arg_lit0("T",   "topaz",   "use Topaz protocol to send command"),
		arg_lit0("3",   NULL,      "ISO14443-3 select only (skip RATS)"),
		arg_strx1(NULL, NULL,      "<data (hex)>", NULL),
		arg_param_end
	};
	// defaults
	arg_get_int(6) = 0;
	arg_get_int(7) = 0;
	
	if (CLIParserParseString(cmd, argtable, arg_getsize(argtable), false)){
		CLIParserFree();
		return 0;
	}
	
	reply = !arg_get_lit(1);
	crc = arg_get_lit(2);
	power = arg_get_lit(3);
	active = arg_get_lit(4);
	active_select = arg_get_lit(5);
	numbits = arg_get_int(6) & 0xFFFF;
	timeout = arg_get_int(7);
	bTimeout = (timeout > 0);	
	topazmode = arg_get_lit(8);
	no_rats = arg_get_lit(9);
	// len = data + CRC(2b)
	if (CLIParamHexToBuf(arg_get_str(10), data, sizeof(data) -2, &datalen)) {
		CLIParserFree();
		return 1;
	}
	
	CLIParserFree();	
	
	// logic 
	if(crc && datalen>0 && datalen<sizeof(data)-2)
	{
		uint8_t first, second;
		if (topazmode) {
			ComputeCrc14443(CRC_14443_B, data, datalen, &first, &second);
		} else {
			ComputeCrc14443(CRC_14443_A, data, datalen, &first, &second);
		}
		data[datalen++] = first;
		data[datalen++] = second;
	}

	if(active || active_select)
	{
		c.arg[0] |= ISO14A_CONNECT | ISO14A_CLEAR_TRACE;
		if(active)
			c.arg[0] |= ISO14A_NO_SELECT;
	}

	if(bTimeout){
		#define MAX_TIMEOUT 40542464 	// = (2^32-1) * (8*16) / 13560000Hz * 1000ms/s
		c.arg[0] |= ISO14A_SET_TIMEOUT;
		if(timeout > MAX_TIMEOUT) {
			timeout = MAX_TIMEOUT;
			PrintAndLog("Set timeout to 40542 seconds (11.26 hours). The max we can wait for response");
		}
		c.arg[2] = 13560000 / 1000 / (8*16) * timeout; // timeout in ETUs (time to transfer 1 bit, approx. 9.4 us)
	}

	if(power) {
		c.arg[0] |= ISO14A_NO_DISCONNECT;
	}

	if(datalen > 0) {
		c.arg[0] |= ISO14A_RAW;
	}

	if(topazmode) {
		c.arg[0] |= ISO14A_TOPAZMODE;
	}

	if(no_rats) {
		c.arg[0] |= ISO14A_NO_RATS;
	}

	// Max buffer is USB_CMD_DATA_SIZE (512)
	c.arg[1] = (datalen & 0xFFFF) | ((uint32_t)numbits << 16);
	memcpy(c.d.asBytes,data,datalen);

	SendCommand(&c);

	if (reply) {
		int res = 0;
		if (active_select)
			res = waitCmd(1);
		if (!res && datalen > 0)
			waitCmd(0);
	} // if reply
	return 0;
}


static int waitCmd(uint8_t iSelect) {
    uint8_t *recv;
    UsbCommand resp;
    char *hexout;

    if (WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
        recv = resp.d.asBytes;
        uint8_t iLen = resp.arg[0];
		if (iSelect){
			iLen = resp.arg[1];
			if (iLen){
				PrintAndLog("Card selected. UID[%i]:", iLen);
			} else {
				PrintAndLog("Can't select card.");
			}
		} else {
			PrintAndLog("received %i bytes:", iLen);
		}
        if(!iLen)
            return 1;
        hexout = (char *)malloc(iLen * 3 + 1);
        if (hexout != NULL) {
            for (int i = 0; i < iLen; i++) { // data in hex
                sprintf(&hexout[i * 3], "%02X ", recv[i]);
            }
            PrintAndLog("%s", hexout);
            free(hexout);
        } else {
            PrintAndLog("malloc failed your client has low memory?");
			return 2;
        }
    } else {
        PrintAndLog("timeout while waiting for reply.");
		return 3;
    }
	return 0;
}

static command_t CommandTable[] = 
{
  {"help",   CmdHelp,              1, "This help"},
  {"list",   CmdHF14AList,         0, "[Deprecated] List ISO 14443a history"},
  {"reader", CmdHF14AReader,       0, "Start acting like an ISO14443 Type A reader"},
  {"info",   CmdHF14AInfo,         0, "Reads card and shows information about it"},
  {"cuids",  CmdHF14ACUIDs,        0, "<n> Collect n>0 ISO14443 Type A UIDs in one go"},
  {"sim",    CmdHF14ASim,          0, "<UID> -- Simulate ISO 14443a tag"},
  {"snoop",  CmdHF14ASnoop,        0, "Eavesdrop ISO 14443 Type A"},
  {"apdu",   CmdHF14AAPDU,         0, "Send an ISO 7816-4 APDU via ISO 14443-4 block transmission protocol"},
  {"raw",    CmdHF14ACmdRaw,       0, "Send raw hex data to tag"},
  {NULL, NULL, 0, NULL}
};

int CmdHF14A(const char *Cmd) {
	(void)WaitForResponseTimeout(CMD_ACK,NULL,100);
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd)
{
  CmdsHelp(CommandTable);
  return 0;
}
