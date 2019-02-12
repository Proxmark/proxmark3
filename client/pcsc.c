//-----------------------------------------------------------------------------
// Copyright (C) 2019 piwi
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// PCSC functions to use alternative Smartcard Readers
//-----------------------------------------------------------------------------

#include "pcsc.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined (__APPLE__)
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#define SCARD_ATTR_VALUE(Class, Tag) ((((ULONG)(Class)) << 16) | ((ULONG)(Tag)))
#define SCARD_CLASS_ICC_STATE       9
#define SCARD_ATTR_ATR_STRING SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0303)
#elif defined (_WIN32)
#include <winscard.h>
#else
#include <winscard.h>
#include <reader.h>
#endif

#include "ui.h"
#include "util.h"
#include "cmdhw.h"

#define PM3_SMARTCARD_DEFAULT_NAME "PM3 RDV40 Smartcard Slot"

static SCARDCONTEXT SC_Context;
static SCARDHANDLE SC_Card;
static DWORD SC_Protocol;
static char* AlternativeSmartcardReader = NULL;

#define PCSC_MAX_TRACELEN 60000
static uint8_t pcsc_trace_buf[PCSC_MAX_TRACELEN];
static bool tracing = false;
static uint32_t traceLen = 0;


uint8_t *pcsc_get_trace_addr(void)
{
	return pcsc_trace_buf;
}


uint32_t pcsc_get_traceLen(void)
{
	return traceLen;
}


static void pcsc_clear_trace(void)
{
	traceLen = 0;
}


static void pcsc_set_tracing(bool enable) {
	tracing = enable;
}


static bool pcsc_LogTrace(const uint8_t *btBytes, uint16_t iLen, uint32_t timestamp_start, uint32_t timestamp_end, bool readerToTag)
{
	if (!tracing) return false;

	uint8_t *trace = pcsc_trace_buf;

	uint32_t num_paritybytes = (iLen-1)/8 + 1;	// number of paritybytes
	uint32_t duration = timestamp_end - timestamp_start;

	// Return when trace is full
	if (traceLen + sizeof(iLen) + sizeof(timestamp_start) + sizeof(duration) + num_paritybytes + iLen >= PCSC_MAX_TRACELEN) {
		tracing = false;	// don't trace any more
		return false;
	}
	// Traceformat:
	// 32 bits timestamp (little endian)
	// 16 bits duration (little endian)
	// 16 bits data length (little endian, Highest Bit used as readerToTag flag)
	// y Bytes data
	// x Bytes parity (one byte per 8 bytes data)

	// timestamp (start)
	trace[traceLen++] = ((timestamp_start >> 0) & 0xff);
	trace[traceLen++] = ((timestamp_start >> 8) & 0xff);
	trace[traceLen++] = ((timestamp_start >> 16) & 0xff);
	trace[traceLen++] = ((timestamp_start >> 24) & 0xff);

	// duration
	trace[traceLen++] = ((duration >> 0) & 0xff);
	trace[traceLen++] = ((duration >> 8) & 0xff);

	// data length
	trace[traceLen++] = ((iLen >> 0) & 0xff);
	trace[traceLen++] = ((iLen >> 8) & 0xff);

	// readerToTag flag
	if (!readerToTag) {
		trace[traceLen - 1] |= 0x80;
	}

	// data bytes
	if (btBytes != NULL && iLen != 0) {
		for (int i = 0; i < iLen; i++) {
			trace[traceLen++] = *btBytes++;
		}
	}

	// dummy parity bytes
	if (num_paritybytes != 0) {
		for (int i = 0; i < num_paritybytes; i++) {
			trace[traceLen++] = 0x00;
		}
	}

	return true;
}


char *getAlternativeSmartcardReader(void)
{
	return AlternativeSmartcardReader ? AlternativeSmartcardReader : PM3_SMARTCARD_DEFAULT_NAME;
}


bool pcscCheckForCardReaders(void)
{
	LONG res = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &SC_Context);
	if (res != SCARD_S_SUCCESS) {
		return false;
	}

	DWORD pcchReaders;
	res = SCardListReaders(SC_Context, NULL, NULL, &pcchReaders);
	if (res != SCARD_S_SUCCESS) {
		SCardReleaseContext(SC_Context);
		return false;
	}

	if (res == SCARD_E_NO_READERS_AVAILABLE || res == SCARD_E_NO_SERVICE) {
		SCardReleaseContext(SC_Context);
		return false;
	}

	return true;
}


static char *pickReader(LPTSTR readerlist)
{
	PrintAndLogEx(NORMAL, "Please select one of these:");
	PrintAndLogEx(NORMAL, "  [0] %s %s", PM3_SMARTCARD_DEFAULT_NAME, PM3hasSmartcardSlot() ? "(default)" : "(default, not available)");

	int num = 1;
	for (LPTSTR p = readerlist; *p != '\0'; ) {
		PrintAndLogEx(NORMAL, "  [%1d] %s", num++, p);
		while (*p++ != '\0') ; // advance to next entry
	}

	num--;

	if (num == 1) {
		printf("Your choice (0 or 1)?");
	} else {
		printf("Your choice (0...%d)? ", num);
	}
	int selection = getch() - '0';
	printf("\n");

	if (selection == 0) {
		PrintAndLogEx(INFO, "Selected %s", PM3_SMARTCARD_DEFAULT_NAME);
		return NULL;
	}

	if (selection >= 1 && selection <= num) {
		LPTSTR p = readerlist;
		for (int i = 1; i < selection; i++) {
			while (*p++ != '\0') ; // advance to next entry
		}
		PrintAndLogEx(INFO, "Selected %s", p);
		return p;
	}

	PrintAndLogEx(INFO, "Invalid selection. Using %s", PM3_SMARTCARD_DEFAULT_NAME);
	return NULL;

}


static bool matchString(char *string, const char *search)
{
	if (search[0] == '*' && search[1] == '\0') {  // the wildcard only string "*" matches everything
		return true;
	}

	if (search[0] == '\0' && string[0] != '\0') {  // string is longer than pattern. No match.
		return false;
	}

	if (search[0] == '?' || search[0] == string[0]) { // wildcard '?' matches any character
		return matchString(string + 1, search + 1);
	}

	if (search[0] == '*') { // wildcard '*' matches any sequence of characters
		for (size_t i = 0; i < strlen(string); i++) {
			if (matchString(string + i, search + 1)) {
				return true;
			}
		}
	}

	return false;
}


static char *matchReader(LPTSTR readerlist, const char *readername)
{
	if (matchString(PM3_SMARTCARD_DEFAULT_NAME, readername)) {
		PrintAndLogEx(INFO, "Selected %s", PM3_SMARTCARD_DEFAULT_NAME);
		return NULL;
	}

	for (LPTSTR p = readerlist; *p != '\0'; ) {
		if (matchString(p, readername)) {
			PrintAndLogEx(INFO, "Selected %s", p);
			return p;
		}
		while (*p++ != '\0') ; // advance to next entry
	}

	PrintAndLogEx(INFO, "No match. Using %s", PM3_SMARTCARD_DEFAULT_NAME);
	return NULL;
}


bool pcscSelectAlternativeCardReader(const char *readername)
{
	DWORD readerlist_len;
	LONG res = SCardListReaders(SC_Context, NULL, NULL, &readerlist_len);
	if (res != SCARD_S_SUCCESS) {
		return false;
	}

	LPTSTR readerlist = calloc(readerlist_len, sizeof(char));
	res = SCardListReaders(SC_Context, NULL, readerlist, &readerlist_len);
	if (res != SCARD_S_SUCCESS) {
		free(readerlist);
		return false;
	}

	char *selected_readername = NULL;
	if (readername) {
		selected_readername = matchReader(readerlist, readername);
	} else {
		selected_readername = pickReader(readerlist);
	}

	if (selected_readername == NULL) {
		free(readerlist);
		return false;
	}

	free(AlternativeSmartcardReader);
	AlternativeSmartcardReader = malloc((strlen(selected_readername) + 1) * sizeof(char));
	strcpy(AlternativeSmartcardReader, selected_readername);

	free(readerlist);
	return true;
}


bool pcscGetATR(smart_card_atr_t *card)
{
	pcsc_clear_trace();
	pcsc_set_tracing(true);
	
	if (!card) {
		return false;
	}

	card->atr_len = 0;
	memset(card->atr, 0, sizeof(card->atr));

	LONG res = SCardConnect(SC_Context, AlternativeSmartcardReader, SCARD_SHARE_SHARED,
	                        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &SC_Card, &SC_Protocol);
	if (res != SCARD_S_SUCCESS) {
		return false;
	}

	DWORD atr_len = sizeof(card->atr);
	res = SCardGetAttrib(SC_Card, SCARD_ATTR_ATR_STRING, card->atr, &atr_len);
	if (res != SCARD_S_SUCCESS) {
		return false;
	}
	card->atr_len = atr_len;

	pcsc_LogTrace(card->atr, card->atr_len, 0, 0, false);
	
	pcsc_set_tracing(false);
	
	return true;
}


void pcscTransmit(uint8_t *data, uint32_t data_len, uint32_t flags, uint8_t *response, int *response_len)
{
	LPCSCARD_IO_REQUEST protocol;
	if (flags & SC_RAW_T0) {
		protocol = SCARD_PCI_T0;
	} else {
		protocol = SCARD_PCI_RAW;
	}

	if ((flags & SC_CONNECT))
		pcsc_clear_trace();

	pcsc_set_tracing(true);

	if ((flags & SC_CONNECT || flags & SC_SELECT)) {
		LONG res = SCardConnect(SC_Context, AlternativeSmartcardReader, SCARD_SHARE_SHARED,
		                        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &SC_Card, &SC_Protocol);
		if (res != SCARD_S_SUCCESS) {
			*response_len = -1;
			return;
		}
	}

	if ((flags & SC_RAW) || (flags & SC_RAW_T0)) {
		pcsc_LogTrace(data, data_len, 0, 0, true);
		DWORD len = *response_len;
		LONG res = SCardTransmit(SC_Card, protocol, data, data_len, NULL, response, &len);
		if (res != SCARD_S_SUCCESS) {
			*response_len = -1;
		} else {
			pcsc_LogTrace(response, len, 0, 0, false);
			*response_len = len;
		}
	}
	pcsc_set_tracing(false);
}
