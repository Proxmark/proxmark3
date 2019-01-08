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

#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#endif

#include "ui.h"

static SCARDCONTEXT hContext;
static char* AlternativeSmartcardReader = NULL;


char *getAlternativeSmartcardReader(void)
{
	return AlternativeSmartcardReader;
}


bool pcscCheckForCardReaders(void)
{
	int res = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	if (res != SCARD_S_SUCCESS) {
		return false;
	}

	DWORD pcchReaders;
	res = SCardListReaders(hContext, NULL, NULL, &pcchReaders);
	if (res != SCARD_S_SUCCESS) {
		SCardReleaseContext(hContext);
		return false;
	}

	if (res == SCARD_E_NO_READERS_AVAILABLE || res == SCARD_E_NO_SERVICE) {
		SCardReleaseContext(hContext);
		return false;
	}
	
	return true;
}


static char *pickReader(LPTSTR readerlist)
{
	PrintAndLogEx(NORMAL, "Please select one of these:");
	PrintAndLogEx(NORMAL, "  [0] PM3 RDV40 Smartcard Slot");

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
	int res = SCardListReaders(hContext, NULL, NULL, &readerlist_len);
	if (res != SCARD_S_SUCCESS) {
		return false;
	}

	LPTSTR readerlist = calloc(readerlist_len, sizeof(char));
	res = SCardListReaders(hContext, NULL, readerlist, &readerlist_len);
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

/*
int main(void)
{
 LONG rv;

 SCARDCONTEXT hContext;
 LPTSTR mszReaders;
 SCARDHANDLE hCard;
 DWORD dwReaders, dwActiveProtocol, dwRecvLength;

 SCARD_IO_REQUEST pioSendPci;
 BYTE pbRecvBuffer[258];
 BYTE cmd1[] = { 0x00, 0xA4, 0x04, 0x00, 0x0A, 0xA0,
  0x00, 0x00, 0x00, 0x62, 0x03, 0x01, 0x0C, 0x06, 0x01 };
 BYTE cmd2[] = { 0x00, 0x00, 0x00, 0x00 };

 unsigned int i;

 rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
 CHECK("SCardEstablishContext", rv)

#ifdef SCARD_AUTOALLOCATE
 dwReaders = SCARD_AUTOALLOCATE;

 rv = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
 CHECK("SCardListReaders", rv)
#else
 CHECK("SCardListReaders", rv)

 mszReaders = calloc(dwReaders, sizeof(char));
 rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
 CHECK("SCardListReaders", rv)
#endif
 printf("reader name: %s\n", mszReaders);

 rv = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED,
  SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
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
 rv = SCardTransmit(hCard, &pioSendPci, cmd1, sizeof(cmd1),
  NULL, pbRecvBuffer, &dwRecvLength);
 CHECK("SCardTransmit", rv)

 printf("response: ");
 for(i=0; i<dwRecvLength; i++)
  printf("%02X ", pbRecvBuffer[i]);
 printf("\n");

 dwRecvLength = sizeof(pbRecvBuffer);
 rv = SCardTransmit(hCard, &pioSendPci, cmd2, sizeof(cmd2),
  NULL, pbRecvBuffer, &dwRecvLength);
 CHECK("SCardTransmit", rv)

 printf("response: ");
 for(i=0; i<dwRecvLength; i++)
  printf("%02X ", pbRecvBuffer[i]);
 printf("\n");

 rv = SCardDisconnect(hCard, SCARD_LEAVE_CARD);
 CHECK("SCardDisconnect", rv)

#ifdef SCARD_AUTOALLOCATE
 rv = SCardFreeMemory(hContext, mszReaders);
 CHECK("SCardFreeMemory", rv)

#else
 free(mszReaders);
#endif

 rv = SCardReleaseContext(hContext);

 CHECK("SCardReleaseContext", rv)

 return 0;
}
*/
