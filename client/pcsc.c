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
#include "util.h"
#include "cmdhw.h"

#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#endif

#include "ui.h"

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

/*
int main(void)
{
 LONG rv;

 SCARDCONTEXT SC_Context;
 LPTSTR mszReaders;
 SCARDHANDLE SC_Card;
 DWORD dwReaders, dwActiveProtocol, dwRecvLength;

 SCARD_IO_REQUEST pioSendPci;
 BYTE pbRecvBuffer[258];
 BYTE cmd1[] = { 0x00, 0xA4, 0x04, 0x00, 0x0A, 0xA0,
  0x00, 0x00, 0x00, 0x62, 0x03, 0x01, 0x0C, 0x06, 0x01 };
 BYTE cmd2[] = { 0x00, 0x00, 0x00, 0x00 };

 unsigned int i;

 rv = SCardEstablisSC_Context(SCARD_SCOPE_SYSTEM, NULL, NULL, &SC_Context);
 CHECK("SCardEstablisSC_Context", rv)

#ifdef SCARD_AUTOALLOCATE
 dwReaders = SCARD_AUTOALLOCATE;

 rv = SCardListReaders(SC_Context, NULL, (LPTSTR)&mszReaders, &dwReaders);
 CHECK("SCardListReaders", rv)
#else
 CHECK("SCardListReaders", rv)

 mszReaders = calloc(dwReaders, sizeof(char));
 rv = SCardListReaders(SC_Context, NULL, mszReaders, &dwReaders);
 CHECK("SCardListReaders", rv)
#endif
 printf("reader name: %s\n", mszReaders);

 rv = SCardConnect(SC_Context, mszReaders, SCARD_SHARE_SHARED,
  SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &SC_Card, &dwActiveProtocol);
 CHECK("SCardConnect", rv)

 switch(dwActiveProtocol)
 {
  case SCARD_PROTOCOL_T0:
   pioSendPci = *SCARD_PCI_T0;
   break;

  case SCARD_PROTOCOL_T1:
   pioSendPci = *SCARD_PCI_T1;
   break;
 }
 dwRecvLength = sizeof(pbRecvBuffer);
 rv = SCardTransmit(SC_Card, &pioSendPci, cmd1, sizeof(cmd1),
  NULL, pbRecvBuffer, &dwRecvLength);
 CHECK("SCardTransmit", rv)

 printf("response: ");
 for(i=0; i<dwRecvLength; i++)
  printf("%02X ", pbRecvBuffer[i]);
 printf("\n");

 dwRecvLength = sizeof(pbRecvBuffer);
 rv = SCardTransmit(SC_Card, &pioSendPci, cmd2, sizeof(cmd2),
  NULL, pbRecvBuffer, &dwRecvLength);
 CHECK("SCardTransmit", rv)

 printf("response: ");
 for(i=0; i<dwRecvLength; i++)
  printf("%02X ", pbRecvBuffer[i]);
 printf("\n");

 rv = SCardDisconnect(SC_Card, SCARD_LEAVE_CARD);
 CHECK("SCardDisconnect", rv)

#ifdef SCARD_AUTOALLOCATE
 rv = SCardFreeMemory(SC_Context, mszReaders);
 CHECK("SCardFreeMemory", rv)

#else
 free(mszReaders);
#endif

 rv = SCardReleaseContext(SC_Context);

 CHECK("SCardReleaseContext", rv)

 return 0;
}
*/
