//-----------------------------------------------------------------------------
// Copyright (C) 2018 iceman
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Proxmark3 RDV40 Smartcard module commands
//-----------------------------------------------------------------------------
#include "cmdsmartcard.h"

#include <ctype.h>
#include <string.h>

#include "ui.h"
#include "cmdparser.h"
#include "proxmark3.h"
#include "util.h"
#include "smartcard.h"
#include "comms.h"
#include "protocols.h"
#include "cmdhw.h"
#include "cmdhflist.h"
#include "emv/apduinfo.h"       // APDUcode description
#include "emv/emvcore.h"        // decodeTVL
#include "crypto/libpcrypto.h"	// sha512hash
#include "emv/dump.h"			// dump_buffer
#include "pcsc.h"

#define SC_UPGRADE_FILES_DIRECTORY          "sc_upgrade_firmware/"

static bool UseAlternativeSmartcardReader = false;	// default: use PM3 RDV40 Smartcard Slot (if available)

static int CmdHelp(const char *Cmd);

static int usage_sm_raw(void) {
	PrintAndLogEx(NORMAL, "Usage: sc raw [h|r|c] d <0A 0B 0C ... hex>");
	PrintAndLogEx(NORMAL, "       h          :  this help");
	PrintAndLogEx(NORMAL, "       r          :  do not read response");
	PrintAndLogEx(NORMAL, "       a          :  active smartcard without select (reset sc module)");
	PrintAndLogEx(NORMAL, "       s          :  active smartcard with select (get ATR)");
	PrintAndLogEx(NORMAL, "       t          :  executes TLV decoder if it possible");
	PrintAndLogEx(NORMAL, "       0          :  use protocol T=0");
	PrintAndLogEx(NORMAL, "       d <bytes>  :  bytes to send");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        sc raw s 0 d 00a404000e315041592e5359532e4444463031  - `1PAY.SYS.DDF01` PSE directory with get ATR");
	return 0;
}

static int usage_sm_select(void) {
	PrintAndLogEx(NORMAL, "Usage: sc select [h|<reader name>] ");
	PrintAndLogEx(NORMAL, "       h             :  this help");
	PrintAndLogEx(NORMAL, "       <reader name> :  a card reader's name, wildcards allowed, leave empty to pick from available readers");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        sc select          : list available card readers and pick");
	PrintAndLogEx(NORMAL, "        sc select Gemalto* : select a connected Gemalto card reader" );
	return 0;
}

static int usage_sm_reader(void) {
	PrintAndLogEx(NORMAL, "Usage: sc reader [h|s]");
	PrintAndLogEx(NORMAL, "       h          :  this help");
	PrintAndLogEx(NORMAL, "       s          :  silent (no messages)");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        sc reader");
	return 0;
}

static int usage_sm_info(void) {
	PrintAndLogEx(NORMAL, "Usage: s info [h|s]");
	PrintAndLogEx(NORMAL, "       h          :  this help");
	PrintAndLogEx(NORMAL, "       s          :  silent (no messages)");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        sc info");
	return 0;
}

static int usage_sm_upgrade(void) {
	PrintAndLogEx(NORMAL, "Upgrade RDV4.0 Smartcard Socket Firmware");
	PrintAndLogEx(NORMAL, "Usage:  sc upgrade f <file name>");
	PrintAndLogEx(NORMAL, "       h               :  this help");
	PrintAndLogEx(NORMAL, "       f <filename>    :  firmware file name");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        sc upgrade f SIM010.BIN");
	return 0;
}

static int usage_sm_setclock(void) {
	PrintAndLogEx(NORMAL, "Usage: sc setclock [h] c <clockspeed>");
	PrintAndLogEx(NORMAL, "       h          :  this help");
	PrintAndLogEx(NORMAL, "       c <>       :  clockspeed (0 = 16mhz, 1=8mhz, 2=4mhz) ");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        sc setclock c 2");
	return 0;
}

static int usage_sm_brute(void) {
	PrintAndLogEx(NORMAL, "Tries to bruteforce SFI, ");
	PrintAndLogEx(NORMAL, "Usage: sc brute [h]");
	PrintAndLogEx(NORMAL, "       h          :  this help");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        sc brute");
	return 0;
}

uint8_t GetATRTA1(uint8_t *atr, size_t atrlen) {
	if (atrlen > 2) {
		uint8_t T0 = atr[1];
		if (T0 & 0x10)
			return atr[2];
	}

	return 0x11; // default value is 0x11, corresponding to fmax=5 MHz, Fi=372, Di=1.
}

int DiArray[] = {
	0,  // b0000 RFU
	1,  // b0001
	2,
	4,
	8,
	16,
	32,  // b0110
	64,  // b0111. This was RFU in ISO/IEC 7816-3:1997 and former. Some card readers or drivers may erroneously reject cards using this value
	12,
	20,
	0,   // b1010 RFU
	0,
	0,   // ...
	0,
	0,
	0    // b1111 RFU
};

int FiArray[] = {
	372,    // b0000 Historical note: in ISO/IEC 7816-3:1989, this was assigned to cards with internal clock
	372,    // b0001
	558,    // b0010
	744,    // b0011
	1116,   // b0100
	1488,   // b0101
	1860,   // b0110
	0,      // b0111 RFU
	0,      // b1000 RFU
	512,    // b1001
	768,    // b1010
	1024,   // b1011
	1536,   // b1100
	2048,   // b1101
	0,      // b1110 RFU
	0       // b1111 RFU
};

float FArray[] = {
	4,    // b0000 Historical note: in ISO/IEC 7816-3:1989, this was assigned to cards with internal clock
	5,    // b0001
	6,    // b0010
	8,    // b0011
	12,   // b0100
	16,   // b0101
	20,   // b0110
	0,    // b0111 RFU
	0,    // b1000 RFU
	5,    // b1001
	7.5,  // b1010
	10,   // b1011
	15,   // b1100
	20,   // b1101
	0,    // b1110 RFU
	0     // b1111 RFU
};

static int GetATRDi(uint8_t *atr, size_t atrlen) {
	uint8_t TA1 = GetATRTA1(atr, atrlen);

	return DiArray[TA1 & 0x0f];  // The 4 low-order bits of TA1 (4th MSbit to 1st LSbit) encode Di
}

static int GetATRFi(uint8_t *atr, size_t atrlen) {
	uint8_t TA1 = GetATRTA1(atr, atrlen);

	return FiArray[TA1 >> 4];  // The 4 high-order bits of TA1 (8th MSbit to 5th LSbit) encode fmax and Fi
}

static float GetATRF(uint8_t *atr, size_t atrlen) {
	uint8_t TA1 = GetATRTA1(atr, atrlen);

	return FArray[TA1 >> 4];  // The 4 high-order bits of TA1 (8th MSbit to 5th LSbit) encode fmax and Fi
}

static int PrintATR(uint8_t *atr, size_t atrlen) {

	uint8_t T0 = atr[1];
	uint8_t K = T0 & 0x0F;
	uint8_t TD1 = 0, T1len = 0, TD1len = 0, TDilen = 0;
	bool protocol_T0_present = true;
	bool protocol_T15_present = false;

	if (T0 & 0x10) {
		PrintAndLog("\t- TA1 (Maximum clock frequency, proposed bit duration) [ 0x%02x ]", atr[2 + T1len]);
		T1len++;
	}
	
	if (T0 & 0x20) {
		PrintAndLog("\t- TB1 (Deprecated: VPP requirements) [ 0x%02x ]", atr[2 + T1len]);
		T1len++;
	}
	
	if (T0 & 0x40) {
		PrintAndLog("\t- TC1 (Extra delay between bytes required by card) [ 0x%02x ]", atr[2 + T1len]);
		T1len++;
	}
	
	if (T0 & 0x80) {
		TD1 = atr[2 + T1len];
		PrintAndLog("\t- TD1 (First offered transmission protocol, presence of TA2..TD2) [ 0x%02x ] Protocol T%d", TD1, TD1 & 0x0f);
		protocol_T0_present = false;
		if ((TD1 & 0x0f) == 0) {
			protocol_T0_present = true;
		}
		if ((TD1 & 0x0f) == 15) {
			protocol_T15_present = true;
		}
		
		T1len++;

		if (TD1 & 0x10) {
			PrintAndLog("\t- TA2 (Specific protocol and parameters to be used after the ATR) [ 0x%02x ]", atr[2 + T1len + TD1len]);
			TD1len++;
		}
		if (TD1 & 0x20) {
			PrintAndLog("\t- TB2 (Deprecated: VPP precise voltage requirement) [ 0x%02x ]", atr[2 + T1len + TD1len]);
			TD1len++;
		}
		if (TD1 & 0x40) {
			PrintAndLog("\t- TC2 (Maximum waiting time for protocol T=0) [ 0x%02x ]", atr[2 + T1len + TD1len]);
			TD1len++;
		}
		if (TD1 & 0x80) {
			uint8_t TDi = atr[2 + T1len + TD1len];
			PrintAndLog("\t- TD2 (A supported protocol or more global parameters, presence of TA3..TD3) [ 0x%02x ] Protocol T%d", TDi, TDi & 0x0f);
			if ((TDi & 0x0f) == 0) {
				protocol_T0_present = true;
			}
			if ((TDi & 0x0f) == 15) {
				protocol_T15_present = true;
			}
			TD1len++;

			bool nextCycle = true;
			uint8_t vi = 3;
			while (nextCycle) {
				nextCycle = false;
				if (TDi & 0x10) {
					PrintAndLog("\t- TA%d: 0x%02x", vi, atr[2 + T1len + TD1len + TDilen]);
					TDilen++;
				}
				if (TDi & 0x20) {
					PrintAndLog("\t- TB%d: 0x%02x", vi, atr[2 + T1len + TD1len + TDilen]);
					TDilen++;
				}
				if (TDi & 0x40) {
					PrintAndLog("\t- TC%d: 0x%02x", vi, atr[2 + T1len + TD1len + TDilen]);
					TDilen++;
				}
				if (TDi & 0x80) {
					TDi = atr[2 + T1len + TD1len + TDilen];
					PrintAndLog("\t- TD%d [ 0x%02x ] Protocol T%d", vi, TDi, TDi & 0x0f);
					TDilen++;

					nextCycle = true;
					vi++;
				}
			}
		}
	}

	if (!protocol_T0_present || protocol_T15_present) { // there is CRC Check Byte TCK
		uint8_t vxor = 0;
		for (int i = 1; i < atrlen; i++)
			vxor ^= atr[i];
		
		if (vxor)
			PrintAndLogEx(WARNING, "Check sum error. Must be 0 got 0x%02X", vxor);
		else
			PrintAndLogEx(INFO, "Check sum OK.");
	}
	
	if (atr[0] != 0x3b)
		PrintAndLogEx(WARNING, "Not a direct convention [ 0x%02x ]", atr[0]);

	uint8_t calen = 2 + T1len + TD1len + TDilen + K;

	if (atrlen != calen && atrlen != calen + 1)  // may be CRC
		PrintAndLogEx(ERR, "ATR length error. len: %d, T1len: %d, TD1len: %d, TDilen: %d, K: %d", atrlen, T1len, TD1len, TDilen, K);

	if (K > 0)
		PrintAndLogEx(INFO, "\nHistorical bytes | len %02d | format %02x", K, atr[2 + T1len + TD1len + TDilen]);
	
	if (K > 1) {
		PrintAndLogEx(INFO, "\tHistorical bytes");
		dump_buffer(&atr[2 + T1len + TD1len + TDilen], K, NULL, 1);
	}

	return 0;
}

bool smart_getATR(smart_card_atr_t *card)
{
	if (UseAlternativeSmartcardReader) {
		return pcscGetATR(card);
	} else {
		UsbCommand c = {CMD_SMART_ATR, {0, 0, 0}};
		SendCommand(&c);

		UsbCommand resp;
		if ( !WaitForResponseTimeout(CMD_ACK, &resp, 2500) ) {
			return false;
		}

		if (resp.arg[0] & 0xff) {
			return resp.arg[0] & 0xFF;
		}

		memcpy(card, (smart_card_atr_t *)resp.d.asBytes, sizeof(smart_card_atr_t));

		return true;
	}
}

static bool smart_select(bool silent) {

	smart_card_atr_t card;
	if (!smart_getATR(&card)) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return false;
	}

	if (!silent) {
		PrintAndLogEx(INFO, "ISO7816-3 ATR : %s", sprint_hex(card.atr, card.atr_len));
	}

	return true;
}


static void smart_transmit(uint8_t *data, uint32_t data_len, uint32_t flags, uint8_t *response, int *response_len, uint32_t max_response_len)
{
	// PrintAndLogEx(SUCCESS, "C-TPDU>>>> %s", sprint_hex(data, data_len));
	if (UseAlternativeSmartcardReader) {
		*response_len = max_response_len;
		pcscTransmit(data, data_len, flags, response, response_len);
	} else {
		UsbCommand c = {CMD_SMART_RAW, {flags, data_len, 0}};
		memcpy(c.d.asBytes, data, data_len);
		SendCommand(&c);

		if (!WaitForResponseTimeout(CMD_ACK, &c, 2500)) {
			PrintAndLogEx(WARNING, "smart card response timeout");
			*response_len = -1;
			return;
		}

		*response_len = c.arg[0];
		if (*response_len > 0) {
			memcpy(response, c.d.asBytes, *response_len);
		}
	}

	if (*response_len <= 0) {
		PrintAndLogEx(WARNING, "smart card response failed");
		*response_len = -2;
		return;
	}

	if (*response_len < 2) {
		// PrintAndLogEx(SUCCESS, "R-TPDU  %02X | ", response[0]);
		return;
	}

	// PrintAndLogEx(SUCCESS, "R-TPDU<<<< %s", sprint_hex(response, *response_len));
	// PrintAndLogEx(SUCCESS, "R-TPDU SW %02X%02X | %s", response[*response_len-2], response[*response_len-1], GetAPDUCodeDescription(response[*response_len-2], response[*response_len-1]));
}


static int CmdSmartSelect(const char *Cmd)
{
	const char *readername;
	
	if (tolower(param_getchar(Cmd, 0)) == 'h') {
		return usage_sm_select();
	}
	
	if (!PM3hasSmartcardSlot() && !pcscCheckForCardReaders()) {
		PrintAndLogEx(WARNING, "No Smartcard Readers available");
		UseAlternativeSmartcardReader = false;
		return 1;
	}
	
	int bg, en;
	if (param_getptr(Cmd, &bg, &en, 0)) {
		UseAlternativeSmartcardReader = pcscSelectAlternativeCardReader(NULL);
	} else {
		readername = Cmd + bg;
		UseAlternativeSmartcardReader = pcscSelectAlternativeCardReader(readername);
	}

	return 0;
}


static int CmdSmartRaw(const char *Cmd) {

	int hexlen = 0;
	bool active = false;
	bool active_select = false;
    bool useT0 = false;	
	uint8_t cmdp = 0;
	bool errors = false, reply = true, decodeTLV = false, breakloop = false;
	uint8_t data[ISO7816_MAX_FRAME_SIZE] = {0x00};

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (tolower(param_getchar(Cmd, cmdp))) {
		case 'h': return usage_sm_raw();
		case 'r':
			reply = false;
			cmdp++;
			break;
		case 'a':
			active = true;
			cmdp++;
			break;
		case 's':
			active_select = true;
			cmdp++;
			break;
		case 't':
			decodeTLV = true;
			cmdp++;
			break;
		case '0':
			useT0 = true;
			cmdp++;
			break;			
		case 'd': {
			switch (param_gethex_to_eol(Cmd, cmdp+1, data, sizeof(data), &hexlen)) {
			case 1:
				PrintAndLogEx(WARNING, "Invalid HEX value.");
				return 1;
			case 2:
				PrintAndLogEx(WARNING, "Too many bytes.  Max %d bytes", sizeof(data));
				return 1;
			case 3:
				PrintAndLogEx(WARNING, "Hex must have even number of digits.");
				return 1;
			}
			cmdp++;
			breakloop = true;
			break;
		}
		default:
			PrintAndLogEx(WARNING, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}

		if ( breakloop )
			break;
	}

	//Validations
	if (errors || cmdp == 0 ) return usage_sm_raw();

	uint32_t flags = 0;
	uint32_t protocol = 0;
	if (active || active_select) {
		flags |= SC_CONNECT;
		if (active_select)
			flags |= SC_SELECT;
	}
	if (hexlen > 0) {
		if (useT0)
			protocol = SC_RAW_T0;
		else
			protocol = SC_RAW;
	}
	
	int response_len = 0;
	uint8_t *response = NULL;
	if (reply) {
		response = calloc(ISO7816_MAX_FRAME_SIZE, sizeof(uint8_t));
		if ( !response )
			return 1;
	}
	
	smart_transmit(data, hexlen, flags|protocol, response, &response_len, ISO7816_MAX_FRAME_SIZE);

	// reading response from smart card
	if ( reply ) {
		if ( response_len < 0 ) {
			free(response);
			return 2;
		}

		if ( response[0] == 0x6C ) {
			data[4] = response[1];
			smart_transmit(data, hexlen, protocol, response, &response_len, ISO7816_MAX_FRAME_SIZE);
			data[4] = 0;
		}

		if (decodeTLV && response_len > 4)
			TLVPrintFromBuffer(response, response_len-2);

		free(response);
	}
	return 0;
}


int ExchangeAPDUSC(uint8_t *APDU, int APDUlen, bool activateCard, bool leaveSignalON, uint8_t *response, int maxresponselen, int *responselen) 
{
	uint8_t TPDU[ISO7816_MAX_FRAME_SIZE];
	
	*responselen = 0;

	if (activateCard)
		smart_select(false);

	uint32_t flags = SC_RAW_T0;
	if (activateCard) {
		flags |= SC_SELECT | SC_CONNECT;
	}
	
	if (APDUlen == 4) {	// Case 1
		memcpy(TPDU, APDU, 4);
		TPDU[4] = 0x00;
		smart_transmit(TPDU, 5, flags, response, responselen, maxresponselen);
	} else if (APDUlen == 5) { // Case 2 Short
		smart_transmit(APDU, 5, flags, response, responselen, maxresponselen);
		if (response[0] == 0x6C) { // wrong Le
			uint16_t Le = APDU[4] ? APDU[4] : 256;
			uint8_t La = response[1];
			memcpy(TPDU, APDU, 5);
			TPDU[4] = La;
			smart_transmit(TPDU, 5, SC_RAW_T0, response, responselen, maxresponselen);
			if (Le < La && *responselen >= 0) {
				response[Le] = response[*responselen-2];
				response[Le+1] = response[*responselen-1];
				*responselen = Le + 2;
			}
		}
	} else if (APDU[4] != 0 && APDUlen == 5 + APDU[4]) { // Case 3 Short
		smart_transmit(APDU, APDUlen, flags, response, responselen, maxresponselen);
	} else if (APDU[4] != 0 && APDUlen == 5 + APDU[4] + 1) { // Case 4 Short
		smart_transmit(APDU, APDUlen-1, flags, response, responselen, maxresponselen);
		if (response[0] == 0x90 && response[1] == 0x00) {
			uint8_t Le = APDU[APDUlen-1];
			uint8_t get_response[5] = {0x00, ISO7816_GET_RESPONSE, 0x00, 0x00, Le};
			return ExchangeAPDUSC(get_response, 5, false, leaveSignalON, response, maxresponselen, responselen);
		}
	} else { // Long Cases not yet implemented
		PrintAndLogEx(ERR, "Long APDUs not yet implemented");
		*responselen = -3;
	}

	if (*responselen < 0 ) {
		return 1;
	} else {
		return 0;
	}
}


static int CmdSmartUpgrade(const char *Cmd) {

	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(WARNING, "WARNING - RDV4.0 Smartcard Socket Firmware upgrade.");
	PrintAndLogEx(WARNING, "A dangerous command, do wrong and you will brick the smart card socket");
	PrintAndLogEx(NORMAL, "");

	FILE *f;
	char filename[FILE_PATH_SIZE] = {0};
	uint8_t cmdp = 0;
	bool errors = false;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (tolower(param_getchar(Cmd, cmdp))) {
		case 'f':
			//File handling and reading
			if ( param_getstr(Cmd, cmdp+1, filename, FILE_PATH_SIZE) >= FILE_PATH_SIZE ) {
				PrintAndLogEx(FAILED, "Filename too long");
				errors = true;
				break;
			}
			cmdp += 2;
			break;
		case 'h':
			return usage_sm_upgrade();
		default:
			PrintAndLogEx(WARNING, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
	}

	//Validations
	if (errors || cmdp == 0 ) return usage_sm_upgrade();

	if (strchr(filename, '\\') || strchr(filename, '/')) {
		PrintAndLogEx(FAILED, "Filename must not contain \\ or /. Firmware file will be found in client/sc_upgrade_firmware directory.");
		return 1;
	}
	
	char sc_upgrade_file_path[strlen(get_my_executable_directory()) + strlen(SC_UPGRADE_FILES_DIRECTORY) + strlen(filename) + 1];
	strcpy(sc_upgrade_file_path, get_my_executable_directory());
	strcat(sc_upgrade_file_path, SC_UPGRADE_FILES_DIRECTORY);
	strcat(sc_upgrade_file_path, filename);
	if (strlen(sc_upgrade_file_path) >= FILE_PATH_SIZE ) {
		PrintAndLogEx(FAILED, "Filename too long");
		return 1;
	}

	char sha512filename[FILE_PATH_SIZE] = {'\0'};
	char *bin_extension = filename;
	char *dot_position = NULL;
	while ((dot_position = strchr(bin_extension, '.')) != NULL) {
		bin_extension = dot_position + 1;
	}
	if (!strcmp(bin_extension, "BIN") 
#ifdef _WIN32
	    || !strcmp(bin_extension, "bin")
#endif
	    ) {
		memcpy(sha512filename, filename, strlen(filename) - strlen("bin"));
		strcat(sha512filename, "sha512.txt");
	} else {
		PrintAndLogEx(FAILED, "Filename extension of Firmware Upgrade File must be .BIN");
		return 1;
	}
	
	PrintAndLogEx(INFO, "Checking integrity using SHA512 File %s ...", sha512filename);
	char sc_upgrade_sha512file_path[strlen(get_my_executable_directory()) + strlen(SC_UPGRADE_FILES_DIRECTORY) + strlen(sha512filename) + 1];
	strcpy(sc_upgrade_sha512file_path, get_my_executable_directory());
	strcat(sc_upgrade_sha512file_path, SC_UPGRADE_FILES_DIRECTORY);
	strcat(sc_upgrade_sha512file_path, sha512filename);
	if (strlen(sc_upgrade_sha512file_path) >= FILE_PATH_SIZE ) {
		PrintAndLogEx(FAILED, "Filename too long");
		return 1;
	}
		
	// load firmware file
	f = fopen(sc_upgrade_file_path, "rb");
	if ( !f ){
		PrintAndLogEx(FAILED, "Firmware file not found or locked.");
		return 1;
	}

	// get filesize in order to malloc memory
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (fsize < 0)  {
		PrintAndLogEx(FAILED, "Could not determine size of firmware file");
		fclose(f);
		return 1;
	}

	uint8_t *dump = calloc(fsize, sizeof(uint8_t));
	if (!dump) {
		PrintAndLogEx(FAILED, "Could not allocate memory for firmware");
		fclose(f);
		return 1;
	}

	size_t firmware_size = fread(dump, 1, fsize, f);
	if (f)
		fclose(f);

	// load sha512 file
	f = fopen(sc_upgrade_sha512file_path, "rb");
	if ( !f ){
		PrintAndLogEx(FAILED, "SHA-512 file not found or locked.");
		return 1;
	}

	// get filesize in order to malloc memory
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (fsize < 0)  {
		PrintAndLogEx(FAILED, "Could not determine size of SHA-512 file");
		fclose(f);
		return 1;
	}
	
	if (fsize < 128) {
		PrintAndLogEx(FAILED, "SHA-512 file too short");
		fclose(f);
		return 1;
	}

	char hashstring[129];
	size_t bytes_read = fread(hashstring, 1, 128, f);
	hashstring[128] = '\0';

	if (f)
		fclose(f);

	uint8_t hash1[64];
	if (bytes_read != 128 || param_gethex(hashstring, 0, hash1, 128)) {
		PrintAndLogEx(FAILED, "Couldn't read SHA-512 file");
		return 1;
	}
	
	uint8_t hash2[64];
	if (sha512hash(dump, firmware_size, hash2)) {
		PrintAndLogEx(FAILED, "Couldn't calculate SHA-512 of Firmware");
		return 1;
	}

	if (memcmp(hash1, hash2, 64)) {
		PrintAndLogEx(FAILED, "Couldn't verify integrity of Firmware file (wrong SHA-512)");
		return 1;
	}
		
	PrintAndLogEx(SUCCESS, "RDV4.0 Smartcard Socket Firmware uploading to PM3");

	//Send to device
	uint32_t index = 0;
	uint32_t bytes_sent = 0;
	uint32_t bytes_remaining = firmware_size;

	while (bytes_remaining > 0){
		uint32_t bytes_in_packet = MIN(USB_CMD_DATA_SIZE, bytes_remaining);
		UsbCommand c = {CMD_SMART_UPLOAD, {index + bytes_sent, bytes_in_packet, 0}};

		// Fill usb bytes with 0xFF
		memset(c.d.asBytes, 0xFF, USB_CMD_DATA_SIZE);
		memcpy(c.d.asBytes, dump + bytes_sent, bytes_in_packet);
		clearCommandBuffer();
		SendCommand(&c);
		if ( !WaitForResponseTimeout(CMD_ACK, NULL, 2000) ) {
			PrintAndLogEx(WARNING, "timeout while waiting for reply.");
			free(dump);
			return 1;
		}

		bytes_remaining -= bytes_in_packet;
		bytes_sent += bytes_in_packet;
		printf("."); fflush(stdout);
	}
	free(dump);
	printf("\n");
	PrintAndLogEx(SUCCESS, "RDV4.0 Smartcard Socket Firmware updating,  don\'t turn off your PM3!");

	// trigger the firmware upgrade
	UsbCommand c = {CMD_SMART_UPGRADE, {firmware_size, 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if ( !WaitForResponseTimeout(CMD_ACK, &resp, 2500) ) {
		PrintAndLogEx(WARNING, "timeout while waiting for reply.");
		return 1;
	}
	if ( (resp.arg[0] & 0xFF ) )
		PrintAndLogEx(SUCCESS, "RDV4.0 Smartcard Socket Firmware upgraded successful");
	else
		PrintAndLogEx(FAILED, "RDV4.0 Smartcard Socket Firmware Upgrade failed");
	return 0;
}


static int CmdSmartInfo(const char *Cmd){
	uint8_t cmdp = 0;
	bool errors = false, silent = false;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (tolower(param_getchar(Cmd, cmdp))) {
		case 'h': return usage_sm_info();
		case 's':
			silent = true;
			break;
		default:
			PrintAndLogEx(WARNING, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		cmdp++;
	}

	//Validations
	if (errors ) return usage_sm_info();

	smart_card_atr_t card;
	if (!smart_getATR(&card)) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return 1;
	}
	
	if (!card.atr_len) {
		if (!silent) PrintAndLogEx(ERR, "can't get ATR from a smart card");
		return 1;
	}

	// print header
	PrintAndLogEx(INFO, "--- Smartcard Information ---------");
	PrintAndLogEx(INFO, "-------------------------------------------------------------");
	PrintAndLogEx(INFO, "ISO7618-3 ATR : %s", sprint_hex(card.atr, card.atr_len));
	PrintAndLogEx(INFO, "\nhttp://smartcard-atr.appspot.com/parse?ATR=%s", sprint_hex_inrow(card.atr, card.atr_len) );

	// print ATR
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(INFO, "ATR");
	PrintATR(card.atr, card.atr_len);

	// print D/F (brom byte TA1 or defaults)
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(INFO, "D/F (TA1)");
	int Di = GetATRDi(card.atr, card.atr_len);
	int Fi = GetATRFi(card.atr, card.atr_len);
	float F = GetATRF(card.atr, card.atr_len);
	if (GetATRTA1(card.atr, card.atr_len) == 0x11)
		PrintAndLogEx(INFO, "Using default values...");

	PrintAndLogEx(NORMAL, "\t- Di=%d", Di);
	PrintAndLogEx(NORMAL, "\t- Fi=%d", Fi);
	PrintAndLogEx(NORMAL, "\t- F=%.1f MHz", F);
  
	if (Di && Fi) {
		PrintAndLogEx(NORMAL, "\t- Cycles/ETU=%d", Fi/Di);
		PrintAndLogEx(NORMAL, "\t- %.1f bits/sec at 4MHz", (float)4000000 / (Fi/Di));
		PrintAndLogEx(NORMAL, "\t- %.1f bits/sec at Fmax=%.1fMHz", (F * 1000000) / (Fi/Di), F);
	} else {
		PrintAndLogEx(WARNING, "\t- Di or Fi is RFU.");
	};

	return 0;
}

int CmdSmartReader(const char *Cmd){
	uint8_t cmdp = 0;
	bool errors = false, silent = false;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (tolower(param_getchar(Cmd, cmdp))) {
		case 'h': return usage_sm_reader();
		case 's':
			silent = true;
			break;
		default:
			PrintAndLogEx(WARNING, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		cmdp++;
	}

	//Validations
	if (errors ) return usage_sm_reader();

	smart_card_atr_t card;
	if (!smart_getATR(&card)) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return 1;
	}

	PrintAndLogEx(INFO, "ISO7816-3 ATR : %s", sprint_hex(card.atr, card.atr_len));
	return 0;
}


static int CmdSmartSetClock(const char *Cmd){
	uint8_t cmdp = 0;
	bool errors = false;
	uint8_t clock = 0;
	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (tolower(param_getchar(Cmd, cmdp))) {
		case 'h': return usage_sm_setclock();
		case 'c':
			clock = param_get8ex(Cmd, cmdp+1, 2, 10);
			if ( clock > 2)
				errors = true;

			cmdp += 2;
			break;
		default:
			PrintAndLogEx(WARNING, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
	}

	//Validations
	if (errors || cmdp == 0) return usage_sm_setclock();

	UsbCommand c = {CMD_SMART_SETCLOCK, {clock, 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if ( !WaitForResponseTimeout(CMD_ACK, &resp, 2500) ) {
		PrintAndLogEx(WARNING, "smart card select failed");
		return 1;
	}

	uint8_t isok = resp.arg[0] & 0xFF;
	if (!isok) {
		PrintAndLogEx(WARNING, "smart card set clock failed");
		return 1;
	}

	switch (clock) {
		case 0:
			PrintAndLogEx(SUCCESS, "Clock changed to 16mhz giving 10800 baudrate");
			break;
		case 1:
			PrintAndLogEx(SUCCESS, "Clock changed to 8mhz giving 21600 baudrate");
			break;
		case 2:
			PrintAndLogEx(SUCCESS, "Clock changed to 4mhz giving 86400 baudrate");
			break;
		default:
			break;
	}
	return 0;
}


static int CmdSmartList(const char *Cmd) {
	if (UseAlternativeSmartcardReader) {
		CmdHFList("7816 p");
	} else {
		CmdHFList("7816");
	}
	return 0;
}


static int CmdSmartBruteforceSFI(const char *Cmd) {

	char ctmp = tolower(param_getchar(Cmd, 0));
	if (ctmp == 'h') return usage_sm_brute();

	uint8_t data[5] = {0x00, 0xB2, 0x00, 0x00, 0x00};

	PrintAndLogEx(INFO, "Selecting card");
	if ( !smart_select(false) ) {
		return 1;
	}

	PrintAndLogEx(INFO, "Selecting PSE aid");
	CmdSmartRaw("s 0 t d 00a404000e325041592e5359532e4444463031");
	CmdSmartRaw("0 t d 00a4040007a000000004101000");  // mastercard
//	CmdSmartRaw("0 t d 00a4040007a0000000031010"); // visa

	PrintAndLogEx(INFO, "starting");

	int response_len = 0;
	uint8_t* response = malloc(ISO7816_MAX_FRAME_SIZE);
	if (!response)
		return 1;

	for (uint8_t i=1; i < 4; i++) {
		for (int p1=1; p1 < 5; p1++) {

			data[2] = p1;
			data[3] = (i << 3) + 4;

			smart_transmit(data, sizeof(data), SC_RAW_T0, response, &response_len, ISO7816_MAX_FRAME_SIZE); 

			if ( response[0] == 0x6C ) {
				data[4] = response[1];
				smart_transmit(data, sizeof(data), SC_RAW_T0, response, &response_len, ISO7816_MAX_FRAME_SIZE); 

				// TLV decoder
				if (response_len > 4)
					TLVPrintFromBuffer(response+1, response_len-3);

				data[4] = 0;
			}
			memset(response, 0x00, ISO7816_MAX_FRAME_SIZE);
		}
	}
	free(response);
	return 0;
}

static command_t CommandTable[] = {
	{"help",     CmdHelp,               1, "This help"},
	{"select",   CmdSmartSelect,        1, "Select the Smartcard Reader to use"},
	{"list",     CmdSmartList,          1, "List ISO 7816 history"},
	{"info",     CmdSmartInfo,          1, "Tag information"},
	{"reader",   CmdSmartReader,        1, "Act like an IS07816 reader"},
	{"raw",      CmdSmartRaw,           1, "Send raw hex data to tag"},
	{"upgrade",  CmdSmartUpgrade,       0, "Upgrade firmware"},
	{"setclock", CmdSmartSetClock,      1, "Set clock speed"},
	{"brute",    CmdSmartBruteforceSFI, 1, "Bruteforce SFI"},
	{NULL,       NULL,                  0, NULL}
};


int CmdSmartcard(const char *Cmd) {
	clearCommandBuffer();
	CmdsParse(CommandTable, Cmd);
	return 0;
}


static int CmdHelp(const char *Cmd) {
	CmdsHelp(CommandTable);
	return 0;
}
