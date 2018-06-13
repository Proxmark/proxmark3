//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
// Merlok - 2017
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency commands
//-----------------------------------------------------------------------------

#include "cmdhf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "comms.h"
#include "util.h"
#include "ui.h"
#include "iso14443crc.h"
#include "parity.h"
#include "cmdmain.h"
#include "cmdparser.h"
#include "cmdhf14a.h"
#include "cmdhf14b.h"
#include "cmdhf15.h"
#include "cmdhfepa.h"
#include "cmdhflegic.h"
#include "cmdhficlass.h"
#include "cmdhfmf.h"
#include "cmdhfmfu.h"
#include "cmdhftopaz.h"
#include "protocols.h"
#include "emv/cmdemv.h"
#include "cmdhflist.h"

static int CmdHelp(const char *Cmd);

int CmdHFTune(const char *Cmd)
{
  UsbCommand c={CMD_MEASURE_ANTENNA_TUNING_HF};
  SendCommand(&c);
  return 0;
}

/**
 * @brief iso14443B_CRC_check Checks CRC in command or response
 * @param isResponse
 * @param data
 * @param len
 * @return  0 : CRC-command, CRC not ok
 *          1 : CRC-command, CRC ok
 *          2 : Not crc-command
 */

uint8_t iso14443B_CRC_check(bool isResponse, uint8_t* data, uint8_t len)
{
	uint8_t b1,b2;

	if(len <= 2) return 2;

	ComputeCrc14443(CRC_14443_B, data, len-2, &b1, &b2);
	if(b1 != data[len-2] || b2 != data[len-1]) {
		return 0;
	} else {
		return 1;
	}
}

/**
 * @brief iclass_CRC_Ok Checks CRC in command or response
 * @param isResponse
 * @param data
 * @param len
 * @return  0 : CRC-command, CRC not ok
 *	        1 : CRC-command, CRC ok
 *          2 : Not crc-command
 */
uint8_t iclass_CRC_check(bool isResponse, uint8_t* data, uint8_t len)
{
	if(len < 4) return 2;//CRC commands (and responses) are all at least 4 bytes

	uint8_t b1, b2;

	if(!isResponse)//Commands to tag
	{
		/**
		  These commands should have CRC. Total length leftmost
		  4	READ
		  4 READ4
		  12 UPDATE - unsecured, ends with CRC16
		  14 UPDATE - secured, ends with signature instead
		  4 PAGESEL
		  **/
		if(len == 4 || len == 12)//Covers three of them
		{
			//Don't include the command byte
			ComputeCrc14443(CRC_ICLASS, (data+1), len-3, &b1, &b2);
			return b1 == data[len -2] && b2 == data[len-1];
		}
		return 2;
	}else{
		/**
		These tag responses should have CRC. Total length leftmost

		10  READ		data[8] crc[2]
		34  READ4		data[32]crc[2]
		10  UPDATE	data[8] crc[2]
		10 SELECT	csn[8] crc[2]
		10  IDENTIFY  asnb[8] crc[2]
		10  PAGESEL   block1[8] crc[2]
		10  DETECT    csn[8] crc[2]

		These should not

		4  CHECK		chip_response[4]
		8  READCHECK data[8]
		1  ACTALL    sof[1]
		1  ACT	     sof[1]

		In conclusion, without looking at the command; any response
		of length 10 or 34 should have CRC
		  **/
		if(len != 10 && len != 34) return true;

		ComputeCrc14443(CRC_ICLASS, data, len-2, &b1, &b2);
		return b1 == data[len -2] && b2 == data[len-1];
	}
}


bool is_last_record(uint16_t tracepos, uint8_t *trace, uint16_t traceLen)
{
	return(tracepos + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) >= traceLen);
}


bool next_record_is_response(uint16_t tracepos, uint8_t *trace)
{
	uint16_t next_records_datalen = *((uint16_t *)(trace + tracepos + sizeof(uint32_t) + sizeof(uint16_t)));
	
	return(next_records_datalen & 0x8000);
}


bool merge_topaz_reader_frames(uint32_t timestamp, uint32_t *duration, uint16_t *tracepos, uint16_t traceLen, uint8_t *trace, uint8_t *frame, uint8_t *topaz_reader_command, uint16_t *data_len)
{

#define MAX_TOPAZ_READER_CMD_LEN	16

	uint32_t last_timestamp = timestamp + *duration;

	if ((*data_len != 1) || (frame[0] == TOPAZ_WUPA) || (frame[0] == TOPAZ_REQA)) return false;

	memcpy(topaz_reader_command, frame, *data_len);

	while (!is_last_record(*tracepos, trace, traceLen) && !next_record_is_response(*tracepos, trace)) {
		uint32_t next_timestamp = *((uint32_t *)(trace + *tracepos));
		*tracepos += sizeof(uint32_t);
		uint16_t next_duration = *((uint16_t *)(trace + *tracepos));
		*tracepos += sizeof(uint16_t);
		uint16_t next_data_len = *((uint16_t *)(trace + *tracepos)) & 0x7FFF;
		*tracepos += sizeof(uint16_t);
		uint8_t *next_frame = (trace + *tracepos);
		*tracepos += next_data_len;
		if ((next_data_len == 1) && (*data_len + next_data_len <= MAX_TOPAZ_READER_CMD_LEN)) {
			memcpy(topaz_reader_command + *data_len, next_frame, next_data_len);
			*data_len += next_data_len;
			last_timestamp = next_timestamp + next_duration;
		} else {
			// rewind and exit
			*tracepos = *tracepos - next_data_len - sizeof(uint16_t) - sizeof(uint16_t) - sizeof(uint32_t);
			break;
		}
		uint16_t next_parity_len = (next_data_len-1)/8 + 1;
		*tracepos += next_parity_len;
	}

	*duration = last_timestamp - timestamp;
	
	return true;
}


uint16_t printTraceLine(uint16_t tracepos, uint16_t traceLen, uint8_t *trace, uint8_t protocol, bool showWaitCycles, bool markCRCBytes)
{
	bool isResponse;
	uint16_t data_len, parity_len;
	uint32_t duration;
	uint8_t topaz_reader_command[9];
	uint32_t timestamp, first_timestamp, EndOfTransmissionTimestamp;
	char explanation[30] = {0};
	uint8_t mfData[32] = {0};
	size_t mfDataLen = 0;

	if (tracepos + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) > traceLen) return traceLen;
	
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
	uint8_t *parityBytes = trace + tracepos;
	tracepos += parity_len;

	if (protocol == TOPAZ && !isResponse) {
		// topaz reader commands come in 1 or 9 separate frames with 7 or 8 Bits each.
		// merge them:
		if (merge_topaz_reader_frames(timestamp, &duration, &tracepos, traceLen, trace, frame, topaz_reader_command, &data_len)) {
			frame = topaz_reader_command;
		}
	}
	
	//Check the CRC status
	uint8_t crcStatus = 2;

	if (data_len > 2) {
		switch (protocol) {
			case ICLASS:
				crcStatus = iclass_CRC_check(isResponse, frame, data_len);
				break;
			case ISO_14443B:
			case TOPAZ:
				crcStatus = iso14443B_CRC_check(isResponse, frame, data_len); 
				break;
			case PROTO_MIFARE:
				crcStatus = mifare_CRC_check(isResponse, frame, data_len);
				break;
			case ISO_14443A:
				crcStatus = iso14443A_CRC_check(isResponse, frame, data_len);
				break;
			default: 
				break;
		}
	}
	//0 CRC-command, CRC not ok
	//1 CRC-command, CRC ok
	//2 Not crc-command

	//--- Draw the data column
	//char line[16][110];
	char line[16][110];

	for (int j = 0; j < data_len && j/16 < 16; j++) {

		uint8_t parityBits = parityBytes[j>>3];
		if (protocol != ISO_14443B && (isResponse || protocol == ISO_14443A) && (oddparity8(frame[j]) != ((parityBits >> (7-(j&0x0007))) & 0x01))) {
			snprintf(line[j/16]+(( j % 16) * 4),110, "%02x! ", frame[j]);
		} else {
			snprintf(line[j/16]+(( j % 16) * 4), 110, " %02x ", frame[j]);
		}

	}

	if (markCRCBytes) {
		if(crcStatus == 0 || crcStatus == 1)
		{//CRC-command
			char *pos1 = line[(data_len-2)/16]+(((data_len-2) % 16) * 4);
			(*pos1) = '[';
			char *pos2 = line[(data_len)/16]+(((data_len) % 16) * 4);
			sprintf(pos2, "%c", ']');
		}
	}

	if(data_len == 0)
	{
		if(data_len == 0){
			sprintf(line[0],"<empty trace - possible error>");
		}
	}
	//--- Draw the CRC column

	char *crc = (crcStatus == 0 ? "!crc" : (crcStatus == 1 ? " ok " : "    "));

	EndOfTransmissionTimestamp = timestamp + duration;

	if (protocol == PROTO_MIFARE)
		annotateMifare(explanation, sizeof(explanation), frame, data_len, parityBytes, parity_len, isResponse);
	
	if(!isResponse)
	{
		switch(protocol) {
			case ICLASS:		annotateIclass(explanation,sizeof(explanation),frame,data_len); break;
			case ISO_14443A:	annotateIso14443a(explanation,sizeof(explanation),frame,data_len); break;
			case ISO_14443B:	annotateIso14443b(explanation,sizeof(explanation),frame,data_len); break;
			case TOPAZ:			annotateTopaz(explanation,sizeof(explanation),frame,data_len); break;
			default:			break;
		}
	}

	int num_lines = MIN((data_len - 1)/16 + 1, 16);
	for (int j = 0; j < num_lines ; j++) {
		if (j == 0) {
			PrintAndLog(" %10d | %10d | %s |%-64s | %s| %s",
				(timestamp - first_timestamp),
				(EndOfTransmissionTimestamp - first_timestamp),
				(isResponse ? "Tag" : "Rdr"),
				line[j],
				(j == num_lines-1) ? crc : "    ",
				(j == num_lines-1) ? explanation : "");
		} else {
			PrintAndLog("            |            |     |%-64s | %s| %s",
				line[j],
				(j == num_lines-1) ? crc : "    ",
				(j == num_lines-1) ? explanation : "");
		}
	}
	
	if (DecodeMifareData(frame, data_len, parityBytes, isResponse, mfData, &mfDataLen)) {
		memset(explanation, 0x00, sizeof(explanation));
		if (!isResponse) {
			explanation[0] = '>';
			annotateIso14443a(&explanation[1], sizeof(explanation) - 1, mfData, mfDataLen);
		}
		uint8_t crcc = iso14443A_CRC_check(isResponse, mfData, mfDataLen);
		PrintAndLog("            |          * | dec |%-64s | %-4s| %s",
			sprint_hex(mfData, mfDataLen),
			(crcc == 0 ? "!crc" : (crcc == 1 ? " ok " : "    ")),
			(true) ? explanation : "");
	};

	if (is_last_record(tracepos, trace, traceLen)) return traceLen;
	
	if (showWaitCycles && !isResponse && next_record_is_response(tracepos, trace)) {
		uint32_t next_timestamp = *((uint32_t *)(trace + tracepos));
		PrintAndLog(" %10d | %10d | %s | fdt (Frame Delay Time): %d",
			(EndOfTransmissionTimestamp - first_timestamp),
			(next_timestamp - first_timestamp),
			"   ",
			(next_timestamp - EndOfTransmissionTimestamp));
	}

	return tracepos;
}


int CmdHFList(const char *Cmd)
{
	bool showWaitCycles = false;
	bool markCRCBytes = false;
	bool loadFromFile = false;
	bool saveToFile = false;
	char param1 = '\0';
	char param2 = '\0';
	char param3 = '\0';
	char type[40] = {0};
	char filename[FILE_PATH_SIZE] = {0};
	uint8_t protocol = 0;
	
	// parse command line
	int tlen = param_getstr(Cmd, 0, type, sizeof(type));
	if (param_getlength(Cmd, 1) == 1) {
		param1 = param_getchar(Cmd, 1);
	} else {
		param_getstr(Cmd, 1, filename, sizeof(filename));
	}
	if (param_getlength(Cmd, 2) == 1) {
		param2 = param_getchar(Cmd, 2);
	} else if (strlen(filename) == 0) {
		param_getstr(Cmd, 2, filename, sizeof(filename));
	}
	if (param_getlength(Cmd, 3) == 1) {
		param3 = param_getchar(Cmd, 3);
	} else if (strlen(filename) == 0) {
		param_getstr(Cmd, 3, filename, sizeof(filename));
	}

	// Validate param1
	bool errors = false;

	if(tlen == 0) {
		errors = true;
	}

	if(param1 == 'h'
			|| (param1 != 0 && param1 != 'f' && param1 != 'c' && param1 != 'l')
			|| (param2 != 0 && param2 != 'f' && param2 != 'c' && param2 != 'l')
			|| (param3 != 0 && param3 != 'f' && param3 != 'c' && param3 != 'l')) {
		errors = true;
	}

	if(!errors) {
		if(strcmp(type, "iclass") == 0)	{
			protocol = ICLASS;
		} else if(strcmp(type, "mf") == 0) {
			protocol = PROTO_MIFARE;
		} else if(strcmp(type, "14a") == 0) {
			protocol = ISO_14443A;
		} else if(strcmp(type, "14b") == 0)	{
			protocol = ISO_14443B;
		} else if(strcmp(type,"topaz") == 0) {
			protocol = TOPAZ;
		} else if(strcmp(type,"raw") == 0) {
			protocol = -1; //No crc, no annotations
		} else if (strcmp(type, "save") == 0) {
			saveToFile = true;
		} else {
			errors = true;
		}
	}
	
	if (param1 == 'f' || param2 == 'f' || param3 == 'f') {
		showWaitCycles = true;
	}

	if (param1 == 'c' || param2 == 'c' || param3 == 'c') {
		markCRCBytes = true;
	}

	if (param1 == 'l' || param2 == 'l' || param3 == 'l') {
		loadFromFile = true;
	}

	if ((loadFromFile || saveToFile) && strlen(filename) == 0) {
		errors = true;
	}

	if (loadFromFile && saveToFile) {
		errors = true;
	}
	
	if (errors) {
		PrintAndLog("List or save protocol data.");
		PrintAndLog("Usage:  hf list <protocol> [f] [c] [l <filename>]");
		PrintAndLog("        hf list save <filename>");
		PrintAndLog("    f      - show frame delay times as well");
		PrintAndLog("    c      - mark CRC bytes");
		PrintAndLog("    l      - load data from file instead of trace buffer");
		PrintAndLog("    save   - save data to file");
		PrintAndLog("Supported <protocol> values:");
		PrintAndLog("    raw    - just show raw data without annotations");
		PrintAndLog("    14a    - interpret data as iso14443a communications");
		PrintAndLog("    mf     - interpret data as iso14443a communications and decrypt crypto1 stream");
		PrintAndLog("    14b    - interpret data as iso14443b communications");
		PrintAndLog("    iclass - interpret data as iclass communications");
		PrintAndLog("    topaz  - interpret data as topaz communications");
		PrintAndLog("");
		PrintAndLog("example: hf list 14a f");
		PrintAndLog("example: hf list iclass");
		PrintAndLog("example: hf list save myCardTrace.trc");
		PrintAndLog("example: hf list 14a l myCardTrace.trc");
		return 0;
	}


	uint8_t *trace;
	uint32_t tracepos = 0;
	uint32_t traceLen = 0;
	
	if (loadFromFile) {
		#define TRACE_CHUNK_SIZE (1<<16)		// 64K to start with. Will be enough for BigBuf and some room for future extensions
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
		PrintAndLog("iso14443a - All times are in carrier periods (1/13.56Mhz)");
		PrintAndLog("iClass    - Timings are not as accurate");
		PrintAndLog("");
		PrintAndLog("      Start |        End | Src | Data (! denotes parity error)                                   | CRC | Annotation         |");
		PrintAndLog("------------|------------|-----|-----------------------------------------------------------------|-----|--------------------|");

		ClearAuthData();
		while(tracepos < traceLen)
		{
			tracepos = printTraceLine(tracepos, traceLen, trace, protocol, showWaitCycles, markCRCBytes);
		}
	}

	free(trace);
	return 0;
}

int CmdHFSearch(const char *Cmd){
	int ans = 0;
	PrintAndLog("");
	ans = CmdHF14AInfo("s");
	if (ans > 0) {
		PrintAndLog("\nValid ISO14443A Tag Found - Quiting Search\n");
		return ans;
	}
	ans = HFiClassReader("", false, false);
	if (ans) {
		PrintAndLog("\nValid iClass Tag (or PicoPass Tag) Found - Quiting Search\n");
		return ans;
	}
	ans = HF15Reader("", false);
	if (ans) {
		PrintAndLog("\nValid ISO15693 Tag Found - Quiting Search\n");
		return ans;
	}
	//14b is longest test currently (and rarest chip type) ... put last
	ans = HF14BInfo(false);
	if (ans) {
		PrintAndLog("\nValid ISO14443B Tag Found - Quiting Search\n");
		return ans;
	}
	PrintAndLog("\nno known/supported 13.56 MHz tags found\n");
	return 0;
}

int CmdHFSnoop(const char *Cmd)
{
	char * pEnd;
	UsbCommand c = {CMD_HF_SNIFFER, {strtol(Cmd, &pEnd,0),strtol(pEnd, &pEnd,0),0}};
	SendCommand(&c);
	return 0;
}

static command_t CommandTable[] = 
{
	{"help",	CmdHelp,		1, "This help"},
	{"14a",		CmdHF14A,		1, "{ ISO14443A RFIDs... }"},
	{"14b",		CmdHF14B,		1, "{ ISO14443B RFIDs... }"},
	{"15",		CmdHF15,		1, "{ ISO15693 RFIDs... }"},
	{"epa",		CmdHFEPA,		1, "{ German Identification Card... }"},
	{"emv",		CmdHFEMV,		1, "{ EMV cards... }"},
	{"legic",	CmdHFLegic,		0, "{ LEGIC RFIDs... }"},
	{"iclass",	CmdHFiClass,	1, "{ ICLASS RFIDs... }"},
	{"mf",		CmdHFMF,		1, "{ MIFARE RFIDs... }"},
	{"mfu",		CmdHFMFUltra,	1, "{ MIFARE Ultralight RFIDs... }"},
	{"topaz",	CmdHFTopaz,		1, "{ TOPAZ (NFC Type 1) RFIDs... }"},
	{"tune",	CmdHFTune,		0, "Continuously measure HF antenna tuning"},
	{"list",	CmdHFList,		1, "List protocol data in trace buffer"},
	{"search",	CmdHFSearch,	1, "Search for known HF tags [preliminary]"},
	{"snoop",   CmdHFSnoop,     0, "<samples to skip (10000)> <triggers to skip (1)> Generic HF Snoop"},
	{NULL,		NULL,			0, NULL}
};

int CmdHF(const char *Cmd)
{
  CmdsParse(CommandTable, Cmd);
  return 0; 
}

int CmdHelp(const char *Cmd)
{
  CmdsHelp(CommandTable);
  return 0;
}
