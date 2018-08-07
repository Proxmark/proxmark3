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
#include "smartcard.h"
#include "comms.h"
#include "protocols.h"


static int CmdHelp(const char *Cmd);

int usage_sm_raw(void) {
	PrintAndLog("Usage: sc raw [h|r|c] d <0A 0B 0C ... hex>");
	PrintAndLog("       h          :  this help");
	PrintAndLog("       r          :  do not read response");
	PrintAndLog("       a          :  active signal field ON without select");
	PrintAndLog("       s          :  active signal field ON with select");
	PrintAndLog("       t          :  executes TLV decoder if it is possible");
	PrintAndLog("       d <bytes>  :  bytes to send");
	PrintAndLog("");
	PrintAndLog("Examples:");
	PrintAndLog("        sc raw d 11223344");	
	return 0;
}
int usage_sm_reader(void) {
	PrintAndLog("Usage: sc reader [h|s]");
	PrintAndLog("       h          :  this help");
	PrintAndLog("       s          :  silent (no messages)");
	PrintAndLog("");
	PrintAndLog("Examples:");
	PrintAndLog("        sc reader");	
	return 0;
}
int usage_sm_info(void) {
	PrintAndLog("Usage: sc info [h|s]");
	PrintAndLog("       h          :  this help");
	PrintAndLog("       s          :  silent (no messages)");
	PrintAndLog("");
	PrintAndLog("Examples:");
	PrintAndLog("        sc info");
	return 0;
}
int usage_sm_upgrade(void) {
	PrintAndLog("Upgrade firmware");
	PrintAndLog("Usage: sc upgrade f <file name>");
	PrintAndLog("       h               :  this help");
	PrintAndLog("       f <filename>    :  firmware file name");
	PrintAndLog("");
	PrintAndLog("Examples:");
	PrintAndLog("        sc upgrade f myfile");
  PrintAndLog("");
	PrintAndLog("WARNING - Dangerous command, do wrong and you will brick the smart card socket");
	return 0;
}
int usage_sm_setclock(void) {
	PrintAndLog("Usage: sc setclock [h] c <clockspeed>");
	PrintAndLog("       h          :  this help");
	PrintAndLog("       c <>       :  clockspeed (0 = 16mhz, 1=8mhz, 2=4mhz) ");
	PrintAndLog("");
	PrintAndLog("Examples:");
	PrintAndLog("        sc setclock c 2");
	return 0;
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
				PrintAndLog("Invalid HEX value.");
				return 1;
			case 2:
				PrintAndLog("Too many bytes.  Max %d bytes", sizeof(data));
				return 1;
			case 3:
				PrintAndLog("Hex must have an even number of digits.");
				return 1;
			}
			cmdp++;
			breakloop = true;
			break;
		}
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
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
		if (active)
			c.arg[0] |= SC_NO_SELECT;
		}

	if (hexlen > 0) {
		c.arg[0] |= SC_RAW;
	}

	memcpy(c.d.asBytes, data, hexlen );
	clearCommandBuffer();
	SendCommand(&c);

	// reading response from smart card
	if ( reply ) {
		UsbCommand resp;
		if (!WaitForResponseTimeout(CMD_ACK, &resp, 2500)) {
			PrintAndLog("smart card response failed");
			return 1;
		}
		uint32_t datalen = resp.arg[0];

		if ( !datalen ) {
			PrintAndLog("smart card response failed");
			return 1;
		}

		PrintAndLog("received %i bytes", datalen);

		if (!datalen)
			return 1;

		uint8_t *data = resp.d.asBytes;

		// TLV decoder
		if (decodeTLV ) {

			if (datalen >= 2) {
				PrintAndLog("%02x %02x | %s", data[datalen - 2], data[datalen - 1], GetAPDUCodeDescription(data[datalen - 2], data[datalen - 1])); 
			}
			if (datalen > 4) {
				TLVPrintFromBuffer(data, datalen - 2);
			}
		} else {
			PrintAndLog("%s", sprint_hex(data,  datalen)); 
		}
	}
	return 0;
}

int CmdSmartUpgrade(const char *Cmd) {

	PrintAndLog("WARNING - Smartcard socket firmware upgrade.");
	PrintAndLog("Dangerous command, do wrong and you will brick the smart card socket");

	FILE *f;
	char filename[FILE_PATH_SIZE] = {0};
	uint8_t cmdp = 0;
	bool errors = false;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (tolower(param_getchar(Cmd, cmdp))) {
		case 'f':
			//File handling and reading
			if ( param_getstr(Cmd, cmdp+1, filename, FILE_PATH_SIZE) >= FILE_PATH_SIZE ) {
				PrintAndLog("Filename too long");
				errors = true;
				break;
			}
			cmdp += 2;
			break;
		case 'h':
			return usage_sm_upgrade();
		default:
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
	}

	//Validations
	if (errors || cmdp == 0 ) return usage_sm_upgrade();

	// load file
	f = fopen(filename, "rb");
	if ( !f ) {
		PrintAndLog("File: %s: not found or locked.", filename);
		return 1;
	}

	// get filesize in order to malloc memory
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (fsize < 0) {
		PrintAndLog("error, when getting filesize");
		fclose(f);
		return 1;
	}
		
	uint8_t *dump = calloc(fsize, sizeof(uint8_t));
	if (!dump) {
		PrintAndLog("error, cannot allocate memory ");
		fclose(f);
		return 1;
	}

	size_t bytes_read = fread(dump, 1, fsize, f);
	if (f)
		fclose(f);

	PrintAndLog("Smartcard socket firmware uploading to PM3");
	//Send to device
	uint32_t index = 0;
	uint32_t bytes_sent = 0;
	uint32_t bytes_remaining = bytes_read;

	while (bytes_remaining > 0){
		uint32_t bytes_in_packet = MIN(USB_CMD_DATA_SIZE, bytes_remaining);
		UsbCommand c = {CMD_SMART_UPLOAD, {index + bytes_sent, bytes_in_packet, 0}};

		// Fill usb bytes with 0xFF
		memset(c.d.asBytes, 0xFF, USB_CMD_DATA_SIZE);
		memcpy(c.d.asBytes, dump + bytes_sent, bytes_in_packet);
		clearCommandBuffer();
		SendCommand(&c);
		if ( !WaitForResponseTimeout(CMD_ACK, NULL, 2000) ) {
			PrintAndLog("timeout while waiting for reply.");
			free(dump);
			return 1;
		}

		bytes_remaining -= bytes_in_packet;
		bytes_sent += bytes_in_packet;
		printf("."); fflush(stdout);
	}
	free(dump);
	printf("\n");
	PrintAndLog("Smartcard socket firmware updating,  don\'t turn off your PM3!");

	// trigger the firmware upgrade
	UsbCommand c = {CMD_SMART_UPGRADE, {bytes_read, 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if ( !WaitForResponseTimeout(CMD_ACK, &resp, 2500) ) {
		PrintAndLog("timeout while waiting for reply.");
		return 1;
	}
	if ( (resp.arg[0] && 0xFF ) )
		PrintAndLog("Smartcard socket firmware upgraded successful");
	else
		PrintAndLog("Smartcard socket firmware updating failed");
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
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
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
		if (!silent) PrintAndLog("smart card select failed");
		return 1;
	}

	uint8_t isok = resp.arg[0] & 0xFF;
	if (!isok) {
		if (!silent) PrintAndLog("smart card select failed");
		return 1;
	}

	smart_card_atr_t card;
	memcpy(&card, (smart_card_atr_t *)resp.d.asBytes, sizeof(smart_card_atr_t));

	// print header
	PrintAndLog("\n--- Smartcard Information ---------");
	PrintAndLog("-------------------------------------------------------------");
	PrintAndLog("ISO76183 ATR : %s", sprint_hex(card.atr, card.atr_len));
	PrintAndLog("look up ATR");
	PrintAndLog("http://smartcard-atr.appspot.com/parse?ATR=%s", sprint_hex_inrow(card.atr, card.atr_len) );
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
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
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
		if (!silent) PrintAndLog("smart card select failed");
		return 1;
	}

	uint8_t isok = resp.arg[0] & 0xFF;
	if (!isok) {
		if (!silent) PrintAndLog("smart card select failed");
		return 1;
	}
	smart_card_atr_t card;
	memcpy(&card, (smart_card_atr_t *)resp.d.asBytes, sizeof(smart_card_atr_t));
	PrintAndLog("ISO7816-3 ATR : %s", sprint_hex(card.atr, card.atr_len));	
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
			PrintAndLog("Unknown parameter '%c'", param_getchar(Cmd, cmdp));
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
		PrintAndLog("smart card select failed");
		return 1;
	}

	uint8_t isok = resp.arg[0] & 0xFF;
	if (!isok) {
		PrintAndLog("smart card set clock failed");
		return 1;
	}

	switch (clock) {
		case 0:
			PrintAndLog("Clock changed to 16mhz giving 10800 baudrate");
			break;
		case 1:
			PrintAndLog("Clock changed to 8mhz giving 21600 baudrate");
			break;
		case 2:
			PrintAndLog("Clock changed to 4mhz giving 86400 baudrate");
			break;
		default:
			break;
	}
	return 0;
}


// iso 7816-3 
void annotateIso7816(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize){
	// S-block
	if ( (cmd[0] & 0xC0) && (cmdsize == 3) ) {
		switch ( (cmd[0] & 0x3f)  ) {
			case 0x00 : snprintf(exp, size, "S-block RESYNCH req"); break;
			case 0x20 : snprintf(exp, size, "S-block RESYNCH resp"); break;
			case 0x01 : snprintf(exp, size, "S-block IFS req"); break;
			case 0x21 : snprintf(exp, size, "S-block IFS resp"); break;
			case 0x02 : snprintf(exp, size, "S-block ABORT req"); break;
			case 0x22 : snprintf(exp, size, "S-block ABORT resp"); break;
			case 0x03 : snprintf(exp, size, "S-block WTX reqt"); break;
			case 0x23 : snprintf(exp, size, "S-block WTX resp"); break;
			default   : snprintf(exp, size, "S-block"); break;
		}
	}
	// R-block (ack)
	else if ( ((cmd[0] & 0xD0) == 0x80) && ( cmdsize > 2) ) {
		if ( (cmd[0] & 0x10) == 0 ) 
			snprintf(exp, size, "R-block ACK");
		else
			snprintf(exp, size, "R-block NACK");
	}
	// I-block
	else {

		int pos = (cmd[0] == 2 ||  cmd[0] == 3) ? 2 : 3;
		switch ( cmd[pos] ) {
			case ISO7816_READ_BINARY             :snprintf(exp, size, "READ BIN");break;
			case ISO7816_WRITE_BINARY            :snprintf(exp, size, "WRITE BIN");break;
			case ISO7816_UPDATE_BINARY           :snprintf(exp, size, "UPDATE BIN");break;
			case ISO7816_ERASE_BINARY            :snprintf(exp, size, "ERASE BIN");break;
			case ISO7816_READ_RECORDS            :snprintf(exp, size, "READ RECORDS");break;
			case ISO7816_WRITE_RECORDS           :snprintf(exp, size, "WRITE RECORDS");break;
			case ISO7816_APPEND_RECORD           :snprintf(exp, size, "APPEND RECORD");break;
			case ISO7816_UPDATE_RECORD           :snprintf(exp, size, "UPDATE RECORD");break;
			case ISO7816_GET_DATA                :snprintf(exp, size, "GET DATA");break;
			case ISO7816_PUT_DATA                :snprintf(exp, size, "PUT DATA");break;
			case ISO7816_SELECT_FILE             :snprintf(exp, size, "SELECT FILE");break;
			case ISO7816_VERIFY                  :snprintf(exp, size, "VERIFY");break;
			case ISO7816_INTERNAL_AUTHENTICATION :snprintf(exp, size, "INTERNAL AUTH");break;
			case ISO7816_EXTERNAL_AUTHENTICATION :snprintf(exp, size, "EXTERNAL AUTH");break;
			case ISO7816_GET_CHALLENGE           :snprintf(exp, size, "GET CHALLENGE");break;
			case ISO7816_MANAGE_CHANNEL          :snprintf(exp, size, "MANAGE CHANNEL");break;
			default                              :snprintf(exp, size, "?"); break;
		}
	}
}


uint16_t printScTraceLine(uint16_t tracepos, uint16_t traceLen, uint8_t *trace) {
		// sanity check
	if (tracepos + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) > traceLen) return traceLen;

	bool isResponse;
	uint16_t data_len, parity_len;
	uint32_t duration, timestamp, first_timestamp, EndOfTransmissionTimestamp;
	char explanation[30] = {0};

	first_timestamp = *((uint32_t *)(trace));
	timestamp = *((uint32_t *)(trace + tracepos));
	tracepos += 4;

	duration = *((uint16_t *)(trace + tracepos));
	tracepos += 2;

	data_len = *((uint16_t *)(trace + tracepos));
	tracepos += 2;

	if (data_len & 0x8000) {
		data_len &= 0x7fff;
		isResponse = true;
	} else {
		isResponse = false;
	}

	parity_len = (data_len-1)/8 + 1;
	if (tracepos + data_len + parity_len > traceLen) {
		return traceLen;
	}
	uint8_t *frame = trace + tracepos;
	tracepos += data_len;
	//uint8_t *parityBytes = trace + tracepos;
	tracepos += parity_len;

	//--- Draw the data column
	char line[18][110];

	if (data_len == 0 ) {
		sprintf(line[0],"<empty trace - possible error>");
		return tracepos;
	}

	for (int j = 0; j < data_len && j/18 < 18; j++) {
		snprintf(line[j/18]+(( j % 18) * 4),110, "%02x  ", frame[j]);
	}

	EndOfTransmissionTimestamp = timestamp + duration;

	annotateIso7816(explanation,sizeof(explanation),frame,data_len);

	int num_lines = MIN((data_len - 1)/18 + 1, 18);
	for (int j = 0; j < num_lines ; j++) {
		if (j == 0) {
			PrintAndLog(" %10u | %10u | %s |%-72s | %s| %s",
				(timestamp - first_timestamp),
				(EndOfTransmissionTimestamp - first_timestamp),
				(isResponse ? "Tag" : "Rdr"),
				line[j],
				"    ",
				(j == num_lines-1) ? explanation : "");
		} else {
			PrintAndLog("            |            |     |%-72s | %s| %s",
				line[j],
				"    ",
				(j == num_lines-1) ? explanation : "");
		}
	}

	// if is last record
	if (tracepos + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) >= traceLen) return traceLen;

	return tracepos;
}

int ScTraceList(const char *Cmd) {
	bool loadFromFile = false;
	bool saveToFile = false;
	char type[5] = {0};
	char filename[FILE_PATH_SIZE] = {0};

	// parse command line
	param_getstr(Cmd, 0, type, sizeof(type));
	param_getstr(Cmd, 1, filename, sizeof(filename));

	bool errors = false;
	if(type[0] == 'h') {
		errors = true;
	}

	if(!errors) {
		if (strcmp(type, "s") == 0) {
			saveToFile = true;
		} else if (strcmp(type,"l") == 0) {
			loadFromFile = true;
		}
	}

	if ((loadFromFile || saveToFile) && strlen(filename) == 0) {
		errors = true;
	}

	if (loadFromFile && saveToFile) {
		errors = true;
	}

	if (errors) {
		PrintAndLog("List or save protocol data.");
		PrintAndLog("Usage:  sc list [l <filename>]");
		PrintAndLog("        sc list [s <filename>]");
		PrintAndLog("    l      - load data from file instead of trace buffer");
		PrintAndLog("    s      - save data to file");
		PrintAndLog("");
		PrintAndLog("example: sc list");
		PrintAndLog("example: sc list save myCardTrace.trc");
		PrintAndLog("example: sc list l myCardTrace.trc");
		return 0;
	}

	uint8_t *trace;
	uint32_t tracepos = 0;
	uint32_t traceLen = 0;

	if (loadFromFile) {
		#define TRACE_CHUNK_SIZE (1<<16)    // 64K to start with. Will be enough for BigBuf and some room for future extensions
		FILE *tracefile = NULL;
		size_t bytes_read;
		trace = malloc(TRACE_CHUNK_SIZE);
		if (trace == NULL) {
			PrintAndLog("Cannot allocate memory for trace");
			return 2;
		}
		if ((tracefile = fopen(filename,"rb")) == NULL) { 
			PrintAndLog("Could not open file %s", filename);
			free(trace);
			return 0;
		}
		while (!feof(tracefile)) {
			bytes_read = fread(trace+traceLen, 1, TRACE_CHUNK_SIZE, tracefile);
			traceLen += bytes_read;
			if (!feof(tracefile)) {
				uint8_t *p = realloc(trace, traceLen + TRACE_CHUNK_SIZE);
				if (p == NULL) {
					PrintAndLog("Cannot allocate memory for trace");
					free(trace);
					fclose(tracefile);
					return 2;
				}
				trace = p;
			}
		}
		fclose(tracefile);
	} else {
		trace = malloc(USB_CMD_DATA_SIZE);
		// Query for the size of the trace
		UsbCommand response;
		GetFromBigBuf(trace, USB_CMD_DATA_SIZE, 0, &response, -1, false);
		traceLen = response.arg[2];
		if (traceLen > USB_CMD_DATA_SIZE) {
			uint8_t *p = realloc(trace, traceLen);
			if (p == NULL) {
				PrintAndLog("Cannot allocate memory for trace");
				free(trace);
				return 2;
			}
			trace = p;
			GetFromBigBuf(trace, traceLen, 0, NULL, -1, false);
		}
	}

	if (saveToFile) {
		FILE *tracefile = NULL;
		if ((tracefile = fopen(filename,"wb")) == NULL) { 
			PrintAndLog("Could not create file %s", filename);
			return 1;
		}
		fwrite(trace, 1, traceLen, tracefile);
		PrintAndLog("Recorded Activity (TraceLen = %d bytes) written to file %s", traceLen, filename);
		fclose(tracefile);
	} else {
		PrintAndLog("Recorded Activity (TraceLen = %d bytes)", traceLen);
		PrintAndLog("");
		PrintAndLog("Start = Start of Start Bit, End = End of last modulation. Src = Source of Transfer");
		PrintAndLog("");
		PrintAndLog("      Start |        End | Src | Data (! denotes parity error)                                           | CRC | Annotation         |");
		PrintAndLog("------------|------------|-----|-------------------------------------------------------------------------|-----|--------------------|");

		while(tracepos < traceLen)
		{
			tracepos = printScTraceLine(tracepos, traceLen, trace);
		}
	}

	free(trace);
	return 0;
}

int CmdSmartList(const char *Cmd) {
	ScTraceList(Cmd);
	return 0;
}

static command_t CommandTable[] = {
	{"help",    CmdHelp,          1, "This help"},
	{"list",    CmdSmartList,     0, "List ISO 7816 history"},
	{"info",    CmdSmartInfo,     1, "Tag information [rdv40]"},
	{"reader",  CmdSmartReader,   1, "Act like an IS07816 reader [rdv40]"},
	{"raw",     CmdSmartRaw,      1, "Send raw hex data to tag [rdv40]"},
	{"upgrade", CmdSmartUpgrade,  1, "Upgrade firmware [rdv40]"},
	{"setclock",CmdSmartSetClock, 1, "Set clock speed"},
	{NULL, NULL, 0, NULL}
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
