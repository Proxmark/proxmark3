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

#include "ui.h"
#include "cmdparser.h"
#include "proxmark3.h"
#include "util.h"
#include "smartcard.h"
#include "comms.h"
#include "protocols.h"
#include "cmdhf.h"              // CmdHFlist
#include "emv/apduinfo.h"       // APDUcode description
#include "emv/emvcore.h"        // decodeTVL
#include "crypto/libpcrypto.h"			// sha512hash

#define SC_UPGRADE_FILES_DIRECTORY          "sc_upgrade_firmware/"

static int CmdHelp(const char *Cmd);

static int usage_sm_raw(void) {
	PrintAndLogEx(NORMAL, "Usage: sc raw [h|r|c] d <0A 0B 0C ... hex>");
	PrintAndLogEx(NORMAL, "       h          :  this help");
	PrintAndLogEx(NORMAL, "       r          :  do not read response");
	PrintAndLogEx(NORMAL, "       a          :  active smartcard without select");
	PrintAndLogEx(NORMAL, "       s          :  active smartcard with select");
	PrintAndLogEx(NORMAL, "       t          :  executes TLV decoder if it possible");
	PrintAndLogEx(NORMAL, "       d <bytes>  :  bytes to send");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        sc raw d 00a404000e315041592e5359532e444446303100    - `1PAY.SYS.DDF01` PPSE directory");
	PrintAndLogEx(NORMAL, "        sc raw d 00a404000e325041592e5359532e444446303100    - `2PAY.SYS.DDF01` PPSE directory");
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

static bool smart_select(bool silent) {
	UsbCommand c = {CMD_SMART_ATR, {0, 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if ( !WaitForResponseTimeout(CMD_ACK, &resp, 2500) ) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return false;
	}

	uint8_t isok = resp.arg[0] & 0xFF;
	if (!isok) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return false;
	}

	if (!silent) {
		smart_card_atr_t card;
		memcpy(&card, (smart_card_atr_t *)resp.d.asBytes, sizeof(smart_card_atr_t));

		PrintAndLogEx(INFO, "ISO7816-3 ATR : %s", sprint_hex(card.atr, card.atr_len));
	}

	return true;
}

static int smart_wait(uint8_t *data) {
	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 2500)) {
		PrintAndLogEx(WARNING, "smart card response failed");
		return -1;
	}

	uint32_t len = resp.arg[0];
	if ( !len ) {
		PrintAndLogEx(WARNING, "smart card response failed");
		return -2;
	}
	memcpy(data, resp.d.asBytes, len);
	PrintAndLogEx(SUCCESS, " %d | %s", len, sprint_hex_inrow_ex(data,  len, 32));

	if (len >= 2) {
		PrintAndLogEx(SUCCESS, "%02X%02X | %s", data[len - 2], data[len - 1], GetAPDUCodeDescription(data[len - 2], data[len - 1]));
	}
	return len;
}

static int smart_response(uint8_t *data) {

	int len = -1;
	int datalen = smart_wait(data);

	if ( data[datalen - 2] == 0x61 || data[datalen - 2] == 0x9F ) {
		len = data[datalen - 1];
	}

	if (len == -1 ) {
		goto out;
	}

	PrintAndLogEx(INFO, "Requesting response. len=0x%x", len);
	uint8_t getstatus[] = {ISO7816_GETSTATUS, 0x00, 0x00, len};
	UsbCommand cStatus = {CMD_SMART_RAW, {SC_RAW, sizeof(getstatus), 0}};
	memcpy(cStatus.d.asBytes, getstatus, sizeof(getstatus) );
	clearCommandBuffer();
	SendCommand(&cStatus);

	datalen = smart_wait(data);
out:

	return datalen;
}

int CmdSmartRaw(const char *Cmd) {

	int hexlen = 0;
	bool active = false;
	bool active_select = false;
	uint8_t cmdp = 0;
	bool errors = false, reply = true, decodeTLV = false, breakloop = false;
	uint8_t data[USB_CMD_DATA_SIZE] = {0x00};

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

	// arg0 = RFU flags
	// arg1 = length
	UsbCommand c = {CMD_SMART_RAW, {0, hexlen, 0}};

	if (active || active_select) {
		c.arg[0] |= SC_CONNECT;
		if (active_select)
			c.arg[0] |= SC_SELECT;
	}

	if (hexlen > 0) {
		c.arg[0] |= SC_RAW;
	}

	memcpy(c.d.asBytes, data, hexlen );
	clearCommandBuffer();
	SendCommand(&c);

	// reading response from smart card
	if ( reply ) {

		uint8_t* buf = calloc(USB_CMD_DATA_SIZE, sizeof(uint8_t));
		if ( !buf )
			return 1;

		int len = smart_response(buf);
		if ( len < 0 ) {
			free(buf);
			return 2;
		}

		if ( buf[0] == 0x6C ) {
			data[4] = buf[1];

			memcpy(c.d.asBytes, data, sizeof(data) );
			clearCommandBuffer();
			SendCommand(&c);
			len = smart_response(buf);

			data[4] = 0;
		}

		if (decodeTLV && len > 4)
			TLVPrintFromBuffer(buf+1, len-3);

		free(buf);
	}
	return 0;
}

int ExchangeAPDUSC(uint8_t *datain, int datainlen, bool activateCard, bool leaveSignalON, uint8_t *dataout, int maxdataoutlen, int *dataoutlen) {
	*dataoutlen = 0;

	if (activateCard)
		smart_select(false);
	printf("* APDU SC\n");

	UsbCommand c = {CMD_SMART_RAW, {SC_RAW | SC_CONNECT, datainlen, 0}};
	if (activateCard) {
		c.arg[0] |= SC_SELECT;
	}
	memcpy(c.d.asBytes, datain, datainlen);
	clearCommandBuffer();
	SendCommand(&c);

	int len = smart_response(dataout);

	if ( len < 0 ) {
		return 2;
	}

	*dataoutlen = len;

	return 0;
}


int CmdSmartUpgrade(const char *Cmd) {

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

	char sha512filename[FILE_PATH_SIZE];
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
		strncpy(sha512filename, filename, strlen(filename) - strlen("bin"));
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

int CmdSmartInfo(const char *Cmd){
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

	UsbCommand c = {CMD_SMART_ATR, {0, 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if ( !WaitForResponseTimeout(CMD_ACK, &resp, 2500) ) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return 1;
	}

	uint8_t isok = resp.arg[0] & 0xFF;
	if (!isok) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return 1;
	}

	smart_card_atr_t card;
	memcpy(&card, (smart_card_atr_t *)resp.d.asBytes, sizeof(smart_card_atr_t));

	// print header
	PrintAndLogEx(INFO, "\n--- Smartcard Information ---------");
	PrintAndLogEx(INFO, "-------------------------------------------------------------");
	PrintAndLogEx(INFO, "ISO76183 ATR : %s", sprint_hex(card.atr, card.atr_len));
	PrintAndLogEx(INFO, "look up ATR");
	PrintAndLogEx(INFO, "http://smartcard-atr.appspot.com/parse?ATR=%s", sprint_hex_inrow(card.atr, card.atr_len) );
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

	UsbCommand c = {CMD_SMART_ATR, {0, 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if ( !WaitForResponseTimeout(CMD_ACK, &resp, 2500) ) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return 1;
	}

	uint8_t isok = resp.arg[0] & 0xFF;
	if (!isok) {
		if (!silent) PrintAndLogEx(WARNING, "smart card select failed");
		return 1;
	}
	smart_card_atr_t card;
	memcpy(&card, (smart_card_atr_t *)resp.d.asBytes, sizeof(smart_card_atr_t));

	PrintAndLogEx(INFO, "ISO7816-3 ATR : %s", sprint_hex(card.atr, card.atr_len));
	return 0;
}

int CmdSmartSetClock(const char *Cmd){
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

int CmdSmartList(const char *Cmd) {
	CmdHFList("7816");
	return 0;
}

int CmdSmartBruteforceSFI(const char *Cmd) {

	char ctmp = tolower(param_getchar(Cmd, 0));
	if (ctmp == 'h') return usage_sm_brute();

	uint8_t data[5] = {0x00, 0xB2, 0x00, 0x00, 0x00};

	PrintAndLogEx(INFO, "Selecting card");
	if ( !smart_select(false) ) {
		return 1;
	}

	PrintAndLogEx(INFO, "Selecting PPSE aid");
	CmdSmartRaw("d 00a404000e325041592e5359532e444446303100");
	CmdSmartRaw("d 00a4040007a000000004101000");

	PrintAndLogEx(INFO, "starting");

	UsbCommand c = {CMD_SMART_RAW, {SC_RAW, sizeof(data), 0}};
	uint8_t* buf = malloc(USB_CMD_DATA_SIZE);
	if ( !buf )
		return 1;

	for (uint8_t i=1; i < 4; i++) {
		for (int p1=1; p1 < 5; p1++) {

			data[2] = p1;
			data[3] = (i << 3) + 4;

			memcpy(c.d.asBytes, data, sizeof(data) );
			clearCommandBuffer();
			SendCommand(&c);

			smart_response(buf);

			// if 0x6C
			if ( buf[0] == 0x6C ) {
				data[4] = buf[1];

				memcpy(c.d.asBytes, data, sizeof(data) );
				clearCommandBuffer();
				SendCommand(&c);
				uint8_t len = smart_response(buf);

				// TLV decoder
				if (len > 4)
					TLVPrintFromBuffer(buf+1, len-3);

				data[4] = 0;
			}
			memset(buf, 0x00, USB_CMD_DATA_SIZE);
		}
	}
	free(buf);
	return 0;
}

static command_t CommandTable[] = {
	{"help",     CmdHelp,               1, "This help"},
	{"list",     CmdSmartList,          0, "List ISO 7816 history"},
	{"info",     CmdSmartInfo,          0, "Tag information"},
	{"reader",   CmdSmartReader,        0, "Act like an IS07816 reader"},
	{"raw",      CmdSmartRaw,           0, "Send raw hex data to tag"},
	{"upgrade",  CmdSmartUpgrade,       0, "Upgrade firmware"},
	{"setclock", CmdSmartSetClock,      0, "Set clock speed"},
	{"brute",    CmdSmartBruteforceSFI, 0, "Bruteforce SFI"},
	{NULL,       NULL,                  0, NULL}
};

int CmdSmartcard(const char *Cmd) {
	clearCommandBuffer();
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd) {
	CmdsHelp(CommandTable);
	return 0;
}
