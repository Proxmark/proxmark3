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

#include <stdint.h>
#include <stdbool.h>
#include "tlv.h"
#include "jansson.h"

// maximum APDU lengths. Long APDUs not yet supported/needed
#define APDU_DATA_LEN      255
#define APDU_COMMAND_LEN   (4 + 1 + APDU_DATA_LEN + 1)
#define APDU_RESPONSE_LEN  (256 + 2)

typedef enum {
	ECC_CONTACTLESS,
	ECC_CONTACT
} EMVCommandChannel;

enum TransactionType {
	TT_MSD,
	TT_VSDC,        // not standart for contactless!!!!
	TT_QVSDCMCHIP,
	TT_CDA,
};
extern char *TransactionTypeStr[];

enum CardPSVendor {
	CV_NA,
	CV_VISA,
	CV_MASTERCARD,
	CV_AMERICANEXPRESS,
	CV_JCB,
	CV_CB,
	CV_SWITCH,
	CV_DINERS,
	CV_OTHER,
};
extern enum CardPSVendor GetCardPSVendor(uint8_t * AID, size_t AIDlen);

extern void DropFieldEx(EMVCommandChannel channel);

extern bool TLVPrintFromBuffer(uint8_t *data, int datalen);
extern void TLVPrintFromTLV(struct tlvdb *tlv);
extern void TLVPrintFromTLVLev(struct tlvdb *tlv, int level);
extern void TLVPrintAIDlistFromSelectTLV(struct tlvdb *tlv);

extern struct tlvdb *GetPANFromTrack2(const struct tlv *track2);
extern struct tlvdb *GetdCVVRawFromTrack2(const struct tlv *track2);

extern void SetAPDULogging(bool logging);

// exchange
extern int EMVExchangeEx(EMVCommandChannel channel, bool ActivateField, bool LeaveFieldON, uint8_t *apdu, int apdu_len, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);

// search application
extern int EMVSearchPSE(EMVCommandChannel channel, bool ActivateField, bool LeaveFieldON, uint8_t PSENum, bool decodeTLV, struct tlvdb *tlv);
extern int EMVSearch(EMVCommandChannel channel, bool ActivateField, bool LeaveFieldON, bool decodeTLV, struct tlvdb *tlv);
extern int EMVSelectPSE(EMVCommandChannel channel, bool ActivateField, bool LeaveFieldON, uint8_t PSENum, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw);
extern int EMVSelect(EMVCommandChannel channel, bool ActivateField, bool LeaveFieldON, uint8_t *AID, size_t AIDLen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// select application
extern int EMVSelectApplication(struct tlvdb *tlv, uint8_t *AID, size_t *AIDlen);
// Get Processing Options
extern int EMVGPO(EMVCommandChannel channel, bool LeaveFieldON, uint8_t *PDOL, size_t PDOLLen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
extern int EMVReadRecord(EMVCommandChannel channel, bool LeaveFieldON, uint8_t SFI, uint8_t SFIrec, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// AC
extern int EMVGenerateChallenge(EMVCommandChannel channel, bool LeaveFieldON, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
extern int EMVAC(EMVCommandChannel channel, bool LeaveFieldON, uint8_t RefControl, uint8_t *CDOL, size_t CDOLLen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// DDA
extern int EMVInternalAuthenticate(EMVCommandChannel channel, bool LeaveFieldON, uint8_t *DDOL, size_t DDOLLen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// Mastercard
int MSCComputeCryptoChecksum(EMVCommandChannel channel, bool LeaveFieldON, uint8_t *UDOL, uint8_t UDOLlen, uint8_t *Result, size_t MaxResultLen, size_t *ResultLen, uint16_t *sw, struct tlvdb *tlv);
// Auth
extern int trSDA(struct tlvdb *tlv);
extern int trDDA(EMVCommandChannel channel, bool decodeTLV, struct tlvdb *tlv);
extern int trCDA(struct tlvdb *tlv, struct tlvdb *ac_tlv, struct tlv *pdol_data_tlv, struct tlv *ac_data_tlv);

extern int RecoveryCertificates(struct tlvdb *tlvRoot, json_t *root);

extern struct emv_pk *get_ca_pk(struct tlvdb *db);
#endif




