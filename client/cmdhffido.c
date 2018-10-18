//-----------------------------------------------------------------------------
// Copyright (C) 2018 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency MIFARE  Plus commands
//-----------------------------------------------------------------------------
//
//  Documentation here:
//
// FIDO Alliance specifications
// https://fidoalliance.org/download/
// FIDO NFC Protocol Specification v1.0
// https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/fido-u2f-nfc-protocol-v1.2-ps-20170411.html
// FIDO U2F Raw Message Formats
// https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/fido-u2f-raw-message-formats-v1.2-ps-20170411.html
//-----------------------------------------------------------------------------


#include "cmdhffido.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "comms.h"
#include "cmdmain.h"
#include "util.h"
#include "ui.h"
#include "cmdhf14a.h"
#include "mifare.h"
#include "emv/emvcore.h"
#include "emv/dump.h"

static int CmdHelp(const char *Cmd);

int FIDOSelect(bool ActivateField, bool LeaveFieldON, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw) {
	uint8_t data[] = {0xA0, 0x00, 0x00, 0x06, 0x47, 0x2F, 0x00, 0x01};
	
	return EMVSelect(ActivateField, LeaveFieldON, data, sizeof(data), Result, MaxResultLen, ResultLen, sw, NULL);
}

int FIDOExchange(sAPDU apdu, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw) {
	int res = EMVExchange(true, apdu, Result, MaxResultLen, ResultLen, sw, NULL);
	if (res == 5) // apdu result (sw) not a 0x9000
		res = 0;
	// software chaining
	while (!res && (*sw >> 8) == 0x61) {
		size_t oldlen = *ResultLen;
		res = EMVExchange(true, (sAPDU){0x00, 0xC0, 0x00, 0x00, 0x00, NULL}, &Result[oldlen], MaxResultLen - oldlen, ResultLen, sw, NULL);
		if (res == 5) // apdu result (sw) not a 0x9000
			res = 0;
		
		*ResultLen += oldlen;
		if (*ResultLen > MaxResultLen) 
			return 100;
	}
	return res;
}

int FIDORegister(uint8_t *params, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw) {
	return FIDOExchange((sAPDU){0x00, 0x01, 0x03, 0x00, 64, params}, Result, MaxResultLen, ResultLen, sw);
}

int FIDOAuthentication(uint8_t *params, uint8_t paramslen, uint8_t controlb, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw) {
	return FIDOExchange((sAPDU){0x00, 0x02, controlb, 0x00, paramslen, params}, Result, MaxResultLen, ResultLen, sw);
}

int CmdHFFidoInfo(const char *cmd) {
	
	if (cmd && strlen(cmd) > 0)
		PrintAndLog("WARNING: command don't have any parameters.\n");
	
	// info about 14a part
	CmdHF14AInfo("");

	// FIDO info
	PrintAndLog("--------------------------------------------"); 
	SetAPDULogging(false);
	
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = FIDOSelect(true, false, buf, sizeof(buf), &len, &sw);

	if (res)
		return res;
	
	if (sw != 0x9000) {
		if (sw)
			PrintAndLog("Not a FIDO card! APDU response: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 
		else
			PrintAndLog("APDU exchange error. Card returns 0x0000."); 
		
		return 0;
	}
	
	if (!strncmp((char *)buf, "U2F_V2", 7)) {
		PrintAndLog("FIDO authenricator detected."); 
		PrintAndLog("WARNING: strange version:"); 
		dump_buffer((const unsigned char *)buf, len, NULL, 0);
		return 0;
	}
	
	PrintAndLog("FIDO authenricator detected. Version: %.*s", len, buf); 
	
	DropField();
	
	return 0;
}

// test only!!!
static uint8_t GkeyHandle[512] = {0};

int CmdHFFidoRegister(const char *cmd) {
	
	// here will be command extraction
	// challenge parameter [32 bytes] - The challenge parameter is the SHA-256 hash of the Client Data, a stringified JSON data structure that the FIDO Client prepares
	// application parameter [32 bytes] - The application parameter is the SHA-256 hash of the UTF-8 encoding of the application identity
	
	uint8_t data[64] = {0};
	
	SetAPDULogging(true);
	DropField();
	
	uint8_t buf[2048] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = FIDOSelect(true, true, buf, sizeof(buf), &len, &sw);

	if (res) {
		PrintAndLog("Can't select authenticator. res=%x. Exit...", res);
		return res;
	}
	
	if (sw != 0x9000) {
		PrintAndLog("Can't select FIDO application. APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 
		return 2;
	}

	res = FIDORegister(data, buf,  sizeof(buf), &len, &sw);
	if (res) {
		PrintAndLog("Can't execute register command. res=%x. Exit...", res);
		return res;
	}
	
	if (sw != 0x9000) {
		PrintAndLog("ERROR execute register command. APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 
		return 3;
	}
	
	PrintAndLog("---------------------------------------------------------------");
	PrintAndLog("data len: %d", len);
	dump_buffer((const unsigned char *)buf, len, NULL, 0);

	if (buf[0] != 0x05) {
		PrintAndLog("ERROR: First byte must be 0x05, but it %2x", buf[0]);
		return 5;
	}
	PrintAndLog("User public key: %s", sprint_hex(&buf[1], 65));
	
	uint8_t keyHandleLen = buf[66];
	PrintAndLog("Key handle[%d]: %s", keyHandleLen, sprint_hex(&buf[67], keyHandleLen));
	memmove(GkeyHandle, &buf[67], keyHandleLen);
	
	int derp = 67 + keyHandleLen;
	int derLen = (buf[derp + 2] << 8) + buf[derp + 3] + 4;
	// needs to decode DER certificate
	PrintAndLog("DER certificate[%d]: %s...", derLen, sprint_hex(&buf[derp], 20));
	dump_buffer_simple((const unsigned char *)&buf[67 + keyHandleLen], derLen, NULL);
	PrintAndLog("---------------------------------------------------------------");
	
	
	int hashp = 1 + 65 + 1 + keyHandleLen + derLen;
	PrintAndLog("Hash[%d]: %s", len - hashp, sprint_hex(&buf[hashp], len - hashp));
	
	// check ANSI X9.62 format ECDSA signature (on P-256)

	DropField();
	return 0;
};

int CmdHFFidoAuthenticate(const char *cmd) {

	// here will be command extraction
	// (in parameter) conrtol byte 0x07 - check only, 0x03 - user presense + cign. 0x08 - sign only
 	// challenge parameter [32 bytes]
	// application parameter [32 bytes]
	// key handle length [1b] = N
	// key handle [N]

	uint8_t keyHandleLen = 64;
	uint8_t data[512] = {0};
	uint8_t datalen = 32 + 32 + 1 + keyHandleLen;
	uint8_t controlByte = 0x08;
	data[64] = keyHandleLen;
	memmove(&data[65], GkeyHandle, keyHandleLen);
	
	SetAPDULogging(true);
	DropField();
	
	uint8_t buf[2048] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = FIDOSelect(true, true, buf, sizeof(buf), &len, &sw);

	if (res) {
		PrintAndLog("Can't select authenticator. res=%x. Exit...", res);
		return res;
	}
	
	if (sw != 0x9000) {
		PrintAndLog("Can't select FIDO application. APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 
		return 2;
	}

	res = FIDOAuthentication(data, datalen, controlByte,  buf,  sizeof(buf), &len, &sw);
	if (res) {
		PrintAndLog("Can't execute authentication command. res=%x. Exit...", res);
		return res;
	}
	
	if (sw != 0x9000) {
		PrintAndLog("ERROR execute authentication command. APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 
		return 3;
	}
	
	PrintAndLog("---------------------------------------------------------------");
	PrintAndLog("User presence: %s", (buf[0]?"verified":"not verified"));
	uint32_t cntr =  (uint32_t)bytes_to_num(&buf[1], 4);
	PrintAndLog("Counter: %d", cntr);
	PrintAndLog("Hash[%d]: %s", len - 5, sprint_hex(&buf[5], len - 5));

	
	DropField();
	return 0;
};

static command_t CommandTable[] =
{
  {"help",             CmdHelp,					1, "This help."},
  {"info",  	       CmdHFFidoInfo,			0, "Info about FIDO tag."},
  {"reg",  	  	 	   CmdHFFidoRegister,		0, "FIDO U2F Registration Message."},
  {"auth",  	       CmdHFFidoAuthenticate,	0, "FIDO U2F Authentication Message."},
  {NULL,               NULL,					0, NULL}
};

int CmdHFFido(const char *Cmd) {
	(void)WaitForResponseTimeout(CMD_ACK,NULL,100);
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd) {
  CmdsHelp(CommandTable);
  return 0;
}
