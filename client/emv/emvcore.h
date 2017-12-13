//-----------------------------------------------------------------------------
// Copyright (C) 2017 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// EMV core functionality
//-----------------------------------------------------------------------------

#ifndef EMVCORE_H__
#define EMVCORE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "util.h"
#include "common.h"
#include "ui.h"
#include "cmdhf14a.h"
#include "emv/apduinfo.h"
#include "emv/tlv.h"
#include "emv/dol.h"
#include "emv/dump.h"
#include "emv/emv_tags.h"

#define APDU_RES_LEN 260
#define APDU_AID_LEN 50

enum TransactionType {
	TT_MSD,
	TT_VSDC,        // not standart for contactless!!!!
	TT_QVSDCMCHIP,
	TT_CDA,
};

typedef struct {
	uint8_t CLA;
	uint8_t INS;
	uint8_t P1;
	uint8_t P2;
	uint8_t Lc;
	uint8_t *data;
} sAPDU;

enum CardPSVendor {
	CV_NA,
	CV_VISA,
	CV_MASTERCARD,
	CV_AMERICANEXPRESS,
	CV_JCB,
	CV_CB,
	CV_OTHER,
};
enum CardPSVendor GetCardPSVendor(uint8_t * AID, size_t AIDlen);

void TLVPrintFromBuffer(uint8_t *data, int datalen);
void TLVPrintFromTLV(struct tlvdb *tlv);
void TLVPrintFromTLVLev(struct tlvdb *tlv, int level);
void TLVPrintAIDlistFromSelectTLV(struct tlvdb *tlv);

struct tlvdb *GetPANFromTrack2(const struct tlv *track2);
struct tlvdb *GetdCVVRawFromTrack2(const struct tlv *track2);

void SetAPDULogging(bool logging);

// search application
int EMVSearchPSE(bool ActivateField, bool LeaveFieldON, bool decodeTLV, struct tlvdb *tlv);
int EMVSearch(bool ActivateField, bool LeaveFieldON, bool decodeTLV, struct tlvdb *tlv);
int EMVSelectPSE(bool ActivateField, bool LeaveFieldON, uint8_t PSENum, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw);
int EMVSelect(bool ActivateField, bool LeaveFieldON, uint8_t *AID, size_t AIDLen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// select application
int EMVSelectApplication(struct tlvdb *tlv, uint8_t *AID, size_t *AIDlen);
// Get Processing Options
int EMVGPO(bool LeaveFieldON, uint8_t *PDOL, size_t PDOLLen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
int EMVReadRecord(bool LeaveFieldON, uint8_t SFI, uint8_t SFIrec, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// AC
int EMVGenerateChallenge(bool LeaveFieldON, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
int EMVAC(bool LeaveFieldON, uint8_t RefControl, uint8_t *CDOL, size_t CDOLLen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// Mastercard
int MSCComputeCryptoChecksum(bool LeaveFieldON, uint8_t *UDOL, uint8_t UDOLlen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// Auth
int trSDA(uint8_t *AID, size_t AIDlen, struct tlvdb *tlv);

#endif




