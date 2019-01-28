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

static SCARDCONTEXT SC_Context;
static SCARDHANDLE SC_Card;
static DWORD SC_Protocol;
static char* AlternativeSmartcardReader = NULL;


char *getAlternativeSmartcardReader(void)
{
	return AlternativeSmartcardReader;
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
	PrintAndLogEx(NORMAL, "  [0] PM3 RDV40 Smartcard Slot %s", PM3hasSmartcardSlot() ? "(default)" : "(default, not available)");

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
		PrintAndLogEx(INFO, "Selected RDV40 Smartcard Slot");
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

	PrintAndLogEx(INFO, "Invalid selection. Using RDV40 Smartcard Slot");
	return NULL;
	
}


char *matchString(LPTSTR readerlist, const char *readername)
{
	return pickReader(readerlist);
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
		selected_readername = matchString(readerlist, readername);
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
	
	return true;
}


bool pcscGetATR(smart_card_atr_t *card)
{
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
	
	// TODO: LogTrace without device
	
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

	// TODO: tracing
	// if ((flags & SC_CONNECT))
		// clear_trace();

	// set_tracing(true);

	if ((flags & SC_CONNECT || flags & SC_SELECT)) {	
		LONG res = SCardConnect(SC_Context, AlternativeSmartcardReader, SCARD_SHARE_SHARED,
		                        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &SC_Card, &SC_Protocol);
		if (res != SCARD_S_SUCCESS) {
			*response_len = -1;
			return;
		}
	}
	
	if ((flags & SC_RAW) || (flags & SC_RAW_T0)) {
		// TODO: tracing
		// LogTrace(data, arg1, 0, 0, NULL, true);
		DWORD len = *response_len;
		LONG res = SCardTransmit(SC_Card, protocol, data, data_len, NULL, response, &len);
		if (res != SCARD_S_SUCCESS) {
			*response_len = -1;
		} else {
			*response_len = len;
		}
	}
}
