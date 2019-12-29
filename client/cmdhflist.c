//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
// Copyright (C) Merlok - 2017
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Command: hf list. It shows data from arm buffer.
//-----------------------------------------------------------------------------

#include "cmdhflist.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "ui.h"
#include "cliparser/cliparser.h"
#include "comms.h"
#include "iso14443crc.h"
#include "iso15693tools.h"
#include "parity.h"
#include "protocols.h"
#include "crapto1/crapto1.h"
#include "mifare/mifarehost.h"
#include "mifare/mifaredefault.h"
#include "usb_cmd.h"
#include "pcsc.h"

typedef struct {
	uint32_t uid;       // UID
	uint32_t nt;        // tag challenge
	uint32_t nt_enc;    // encrypted tag challenge
	uint8_t nt_enc_par; // encrypted tag challenge parity
	uint32_t nr_enc;    // encrypted reader challenge
	uint32_t ar_enc;    // encrypted reader response
	uint8_t ar_enc_par; // encrypted reader response parity
	uint32_t at_enc;    // encrypted tag response
	uint8_t at_enc_par; // encrypted tag response parity
	bool first_auth;    // is first authentication
	uint32_t ks2;       // ar ^ ar_enc
	uint32_t ks3;       // at ^ at_enc
} TAuthData;

enum MifareAuthSeq {
	masNone,
	masNt,
	masNrAr,
	masAt,
	masAuthComplete,
	masFirstData,
	masData,
	masError,
};

static enum MifareAuthSeq MifareAuthState;
static TAuthData AuthData;

static void ClearAuthData() {
	AuthData.uid = 0;
	AuthData.nt = 0;
	AuthData.first_auth = true;
	AuthData.ks2 = 0;
	AuthData.ks3 = 0;
}

/**
 * @brief iso14443A_CRC_check Checks CRC in command or response
 * @param isResponse
 * @param data
 * @param len
 * @return  0 : CRC-command, CRC not ok
 *          1 : CRC-command, CRC ok
 *          2 : Not crc-command
 */
static uint8_t iso14443A_CRC_check(bool isResponse, uint8_t* data, uint8_t len)
{
	uint8_t b1,b2;

	if(len <= 2) return 2;

	if(isResponse & (len < 6)) return 2;

	ComputeCrc14443(CRC_14443_A, data, len-2, &b1, &b2);
	if (b1 != data[len-2] || b2 != data[len-1]) {
		return 0;
	} else {
		return 1;
	}
}


static uint8_t iso14443_4_CRC_check(uint8_t* data, uint8_t len)
{
	uint8_t b1,b2;

	if(len <= 2) return 2;

	ComputeCrc14443(CRC_14443_A, data, len-2, &b1, &b2);
	if (b1 != data[len-2] || b2 != data[len-1]) {
		return 0;
	} else {
		return 1;
	}
}


static uint8_t mifare_CRC_check(bool isResponse, uint8_t* data, uint8_t len)
{
	switch(MifareAuthState) {
		case masNone:
		case masError:
			return iso14443A_CRC_check(isResponse, data, len);
		default:
			return 2;
	}
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
static uint8_t iso14443B_CRC_check(bool isResponse, uint8_t* data, uint8_t len)
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


static uint8_t iso15693_CRC_check(uint8_t* d, uint16_t n)
{
	if (n <= 2) return 2;

	return (Iso15693Crc(d, n) == ISO15693_CRC_CHECK ? 1 : 0);
}


/**
 * @brief iclass_CRC_Ok Checks CRC in command or response
 * @param isResponse
 * @param data
 * @param len
 * @return  0 : CRC-command, CRC not ok
 *          1 : CRC-command, CRC ok
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
		  4 READ
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

		10  READ        data[8] crc[2]
		34  READ4       data[32]crc[2]
		10  UPDATE  data[8] crc[2]
		10 SELECT   csn[8] crc[2]
		10  IDENTIFY  asnb[8] crc[2]
		10  PAGESEL   block1[8] crc[2]
		10  DETECT    csn[8] crc[2]

		These should not

		4  CHECK        chip_response[4]
		8  READCHECK data[8]
		1  ACTALL    sof[1]
		1  ACT       sof[1]

		In conclusion, without looking at the command; any response
		of length 10 or 34 should have CRC
		  **/
		if(len != 10 && len != 34) return true;

		ComputeCrc14443(CRC_ICLASS, data, len-2, &b1, &b2);
		return b1 == data[len -2] && b2 == data[len-1];
	}
}


void annotateIclass(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize) {
	switch(cmd[0])
	{
	case ICLASS_CMD_ACTALL:      snprintf(exp, size, "ACTALL"); break;
	case ICLASS_CMD_READ_OR_IDENTIFY: {
		if (cmdsize > 1){
			snprintf(exp,size,"READ(%d)",cmd[1]);
		} else {
			snprintf(exp,size,"IDENTIFY");
		}
		break;
	}
	case ICLASS_CMD_SELECT:       snprintf(exp,size, "SELECT"); break;
	case ICLASS_CMD_PAGESEL:      snprintf(exp,size, "PAGESEL(%d)", cmd[1]); break;
	case ICLASS_CMD_READCHECK_KC: snprintf(exp,size, "READCHECK[Kc](%d)", cmd[1]); break;
	case ICLASS_CMD_READCHECK_KD: snprintf(exp,size, "READCHECK[Kd](%d)", cmd[1]); break;
	case ICLASS_CMD_CHECK_KC:
	case ICLASS_CMD_CHECK_KD:     snprintf(exp,size, "CHECK"); break;
	case ICLASS_CMD_DETECT:       snprintf(exp,size, "DETECT"); break;
	case ICLASS_CMD_HALT:         snprintf(exp,size, "HALT"); break;
	case ICLASS_CMD_UPDATE:       snprintf(exp,size, "UPDATE(%d)",cmd[1]); break;
	case ICLASS_CMD_ACT:          snprintf(exp,size, "ACT"); break;
	case ICLASS_CMD_READ4:        snprintf(exp,size, "READ4(%d)",cmd[1]); break;
	default:                      snprintf(exp,size, "?"); break;
	}
	return;
}


void annotateIso15693(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize) {
	if (cmdsize >= 2) {
		switch (cmd[1]) {
			// Mandatory Commands, all Tags must support them:
			case ISO15693_INVENTORY             :snprintf(exp, size, "INVENTORY");return;
			case ISO15693_STAYQUIET             :snprintf(exp, size, "STAY_QUIET");return;
			// Optional Commands, Tags may support them:
			case ISO15693_READBLOCK             :snprintf(exp, size, "READBLOCK");return;
			case ISO15693_WRITEBLOCK            :snprintf(exp, size, "WRITEBLOCK");return;
			case ISO15693_LOCKBLOCK             :snprintf(exp, size, "LOCKBLOCK");return;
			case ISO15693_READ_MULTI_BLOCK      :snprintf(exp, size, "READ_MULTI_BLOCK");return;
			case ISO15693_WRITE_MULTI_BLOCK     :snprintf(exp, size, "WRITE_MULTI_BLOCK");return;
			case ISO15693_SELECT                :snprintf(exp, size, "SELECT");return;
			case ISO15693_RESET_TO_READY        :snprintf(exp, size, "RESET_TO_READY");return;
			case ISO15693_WRITE_AFI             :snprintf(exp, size, "WRITE_AFI");return;
			case ISO15693_LOCK_AFI              :snprintf(exp, size, "LOCK_AFI");return;
			case ISO15693_WRITE_DSFID           :snprintf(exp, size, "WRITE_DSFID");return;
			case ISO15693_LOCK_DSFID            :snprintf(exp, size, "LOCK_DSFID");return;
			case ISO15693_GET_SYSTEM_INFO       :snprintf(exp, size, "GET_SYSTEM_INFO");return;
			case ISO15693_READ_MULTI_SECSTATUS  :snprintf(exp, size, "READ_MULTI_SECSTATUS");return;
			default: break;
		}

		if (cmd[1] > ISO15693_STAYQUIET && cmd[1] < ISO15693_READBLOCK) snprintf(exp, size, "Mandatory RFU");
		else if (cmd[1] > ISO15693_READ_MULTI_SECSTATUS && cmd[1] <= 0x9F) snprintf(exp, size, "Optional RFU");
		else if ( cmd[1] >= 0xA0 && cmd[1] <= 0xDF ) snprintf(exp, size, "Custom command");
		else if ( cmd[1] >= 0xE0 && cmd[1] <= 0xFF ) snprintf(exp, size, "Proprietary command");
	}
}


void annotateTopaz(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize)
{
	switch(cmd[0]) {
		case TOPAZ_REQA                     :snprintf(exp, size, "REQA");break;
		case TOPAZ_WUPA                     :snprintf(exp, size, "WUPA");break;
		case TOPAZ_RID                      :snprintf(exp, size, "RID");break;
		case TOPAZ_RALL                     :snprintf(exp, size, "RALL");break;
		case TOPAZ_READ                     :snprintf(exp, size, "READ");break;
		case TOPAZ_WRITE_E                  :snprintf(exp, size, "WRITE-E");break;
		case TOPAZ_WRITE_NE                 :snprintf(exp, size, "WRITE-NE");break;
		case TOPAZ_RSEG                     :snprintf(exp, size, "RSEG");break;
		case TOPAZ_READ8                    :snprintf(exp, size, "READ8");break;
		case TOPAZ_WRITE_E8                 :snprintf(exp, size, "WRITE-E8");break;
		case TOPAZ_WRITE_NE8                :snprintf(exp, size, "WRITE-NE8");break;
		default:                            snprintf(exp,size,"?"); break;
	}
}


void annotateIso7816(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize)
{
	switch ( cmd[1] ){
		case ISO7816_READ_BINARY                :snprintf(exp, size, "READ BINARY");break;
		case ISO7816_WRITE_BINARY               :snprintf(exp, size, "WRITE BINARY");break;
		case ISO7816_UPDATE_BINARY              :snprintf(exp, size, "UPDATE BINARY");break;
		case ISO7816_ERASE_BINARY               :snprintf(exp, size, "ERASE BINARY");break;
		case ISO7816_READ_RECORDS               :snprintf(exp, size, "READ RECORD(S)");break;
		case ISO7816_WRITE_RECORD               :snprintf(exp, size, "WRITE RECORD");break;
		case ISO7816_APPEND_RECORD              :snprintf(exp, size, "APPEND RECORD");break;
		case ISO7816_UPDATE_DATA                :snprintf(exp, size, "UPDATE DATA");break;
		case ISO7816_GET_DATA                   :snprintf(exp, size, "GET DATA");break;
		case ISO7816_PUT_DATA                   :snprintf(exp, size, "PUT DATA");break;
		case ISO7816_SELECT_FILE                :snprintf(exp, size, "SELECT FILE");break;
		case ISO7816_VERIFY                     :snprintf(exp, size, "VERIFY");break;
		case ISO7816_INTERNAL_AUTHENTICATE      :snprintf(exp, size, "INTERNAL AUTHENTICATE");break;
		case ISO7816_EXTERNAL_AUTHENTICATE      :snprintf(exp, size, "EXTERNAL AUTHENTICATE");break;
		case ISO7816_GET_CHALLENGE              :snprintf(exp, size, "GET CHALLENGE");break;
		case ISO7816_MANAGE_CHANNEL             :snprintf(exp, size, "MANAGE CHANNEL");break;
		case ISO7816_GET_RESPONSE               :snprintf(exp, size, "GET RESPONSE");break;
		case ISO7816_ENVELOPE                   :snprintf(exp, size, "ENVELOPE");break;
		case ISO7816_GET_PROCESSING_OPTIONS     :snprintf(exp, size, "GET PROCESSING OPTIONS");break;
		default                                 :snprintf(exp,size,"?"); break;
	}
}


void annotateIso14443_4(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize) {
	// S-block
	if ((cmd[0] & 0xc3) == 0xc2) {
		switch (cmd[0] & 0x30) {
			case 0x00   : snprintf(exp, size, "S-block DESELECT"); break;
			case 0x30   : snprintf(exp, size, "S-block WTX"); break;
			default     : snprintf(exp, size, "S-block (RFU)"); break;
		}
	}
	// R-block (ack)
	else if ((cmd[0] & 0xe0) == 0xa0) {
		if ((cmd[0] & 0x10) == 0)
			snprintf(exp, size, "R-block ACK");
		else
			snprintf(exp, size, "R-block NACK");
	}
	// I-block
	else {
		int pos = 1;
		switch (cmd[0] & 0x0c) {
			case 0x08: // CID following
			case 0x04: // NAD following
				pos = 2;
				break;
			case 0x0c: // CID and NAD following
				pos = 3;
				break;
			default:
				pos = 1; // no CID, no NAD
				break;
		}
		annotateIso7816(exp, size, &cmd[pos], cmdsize-pos);
	}
}


/**
06 00 = INITIATE
0E xx = SELECT ID (xx = Chip-ID)
0B = Get UID
08 yy = Read Block (yy = block number)
09 yy dd dd dd dd = Write Block (yy = block number; dd dd dd dd = data to be written)
0C = Reset to Inventory
0F = Completion
0A 11 22 33 44 55 66 = Authenticate (11 22 33 44 55 66 = data to authenticate)
**/

void annotateIso14443b(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize) {
	switch(cmd[0]){
	case ISO14443B_REQB   : snprintf(exp,size,"REQB");break;
	case ISO14443B_ATTRIB : snprintf(exp,size,"ATTRIB");break;
	case ISO14443B_HALT   : snprintf(exp,size,"HALT");break;
	case ISO14443B_INITIATE     : snprintf(exp,size,"INITIATE");break;
	case ISO14443B_SELECT       : snprintf(exp,size,"SELECT(%d)",cmd[1]);break;
	case ISO14443B_GET_UID      : snprintf(exp,size,"GET UID");break;
	case ISO14443B_READ_BLK     : snprintf(exp,size,"READ_BLK(%d)", cmd[1]);break;
	case ISO14443B_WRITE_BLK    : snprintf(exp,size,"WRITE_BLK(%d)",cmd[1]);break;
	case ISO14443B_RESET        : snprintf(exp,size,"RESET");break;
	case ISO14443B_COMPLETION   : snprintf(exp,size,"COMPLETION");break;
	case ISO14443B_AUTHENTICATE : snprintf(exp,size,"AUTHENTICATE");break;
	default                     : snprintf(exp,size ,"?");break;
	}

}

void annotateIso14443a(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize) {
	switch(cmd[0])
	{
	case ISO14443A_CMD_WUPA:
		snprintf(exp,size,"WUPA");
		break;
	case ISO14443A_CMD_ANTICOLL_OR_SELECT:{
		// 93 20 = Anticollision (usage: 9320 - answer: 4bytes UID+1byte UID-bytes-xor)
		// 93 70 = Select (usage: 9370+5bytes 9320 answer - answer: 1byte SAK)
		if(cmd[1] == 0x70)
		{
			snprintf(exp,size,"SELECT_UID"); break;
		}else
		{
			snprintf(exp,size,"ANTICOLL"); break;
		}
	}
	case ISO14443A_CMD_ANTICOLL_OR_SELECT_2:{
		//95 20 = Anticollision of cascade level2
		//95 70 = Select of cascade level2
		if(cmd[2] == 0x70)
		{
			snprintf(exp,size,"SELECT_UID-2"); break;
		}else
		{
			snprintf(exp,size,"ANTICOLL-2"); break;
		}
	}
	case ISO14443A_CMD_REQA:
		snprintf(exp,size,"REQA");
		break;
	case MIFARE_CMD_READBLOCK:   snprintf(exp,size,"READBLOCK(%d)",cmd[1]); break;
	case MIFARE_CMD_WRITEBLOCK:  snprintf(exp,size,"WRITEBLOCK(%d)",cmd[1]); break;
	case ISO14443A_CMD_HALT:
		snprintf(exp,size,"HALT");
		MifareAuthState = masNone;
		break;
	case ISO14443A_CMD_RATS:        snprintf(exp,size,"RATS"); break;
	case MIFARE_CMD_INC:            snprintf(exp,size,"INC(%d)",cmd[1]); break;
	case MIFARE_CMD_DEC:            snprintf(exp,size,"DEC(%d)",cmd[1]); break;
	case MIFARE_CMD_RESTORE:        snprintf(exp,size,"RESTORE(%d)",cmd[1]); break;
	case MIFARE_CMD_TRANSFER:       snprintf(exp,size,"TRANSFER(%d)",cmd[1]); break;
	case MIFARE_AUTH_KEYA:
		if ( cmdsize > 3) {
			snprintf(exp,size,"AUTH-A(%d)",cmd[1]);
			MifareAuthState = masNt;
		} else {
			//  case MIFARE_ULEV1_VERSION :  both 0x60.
			snprintf(exp,size,"EV1 VERSION");
		}
		break;
	case MIFARE_AUTH_KEYB:
		MifareAuthState = masNt;
		snprintf(exp,size,"AUTH-B(%d)",cmd[1]);
		break;
	case MIFARE_MAGICWUPC1:         snprintf(exp,size,"MAGIC WUPC1"); break;
	case MIFARE_MAGICWUPC2:         snprintf(exp,size,"MAGIC WUPC2"); break;
	case MIFARE_MAGICWIPEC:         snprintf(exp,size,"MAGIC WIPEC"); break;
	case MIFARE_ULC_AUTH_1:     snprintf(exp,size,"AUTH "); break;
	case MIFARE_ULC_AUTH_2:     snprintf(exp,size,"AUTH_ANSW"); break;
	case MIFARE_ULEV1_AUTH:
		if ( cmdsize == 7 )
			snprintf(exp,size,"PWD-AUTH KEY: 0x%02x%02x%02x%02x", cmd[1], cmd[2], cmd[3], cmd[4] );
		else
			snprintf(exp,size,"PWD-AUTH");
		break;
	case MIFARE_ULEV1_FASTREAD:{
		if ( cmdsize >=3 && cmd[2] <= 0xE6)
			snprintf(exp,size,"READ RANGE (%d-%d)",cmd[1],cmd[2]);
		else
			snprintf(exp,size,"?");
		break;
	}
	case MIFARE_ULC_WRITE:{
		if ( cmd[1] < 0x21 )
			snprintf(exp,size,"WRITEBLOCK(%d)",cmd[1]);
		else
			snprintf(exp,size,"?");
		break;
	}
	case MIFARE_ULEV1_READ_CNT:{
		if ( cmd[1] < 5 )
			snprintf(exp,size,"READ CNT(%d)",cmd[1]);
		else
			snprintf(exp,size,"?");
		break;
	}
	case MIFARE_ULEV1_INCR_CNT:{
		if ( cmd[1] < 5 )
			snprintf(exp,size,"INCR(%d)",cmd[1]);
		else
			snprintf(exp,size,"?");
		break;
	}
	case MIFARE_ULEV1_READSIG:      snprintf(exp,size,"READ_SIG"); break;
	case MIFARE_ULEV1_CHECKTEAR:    snprintf(exp,size,"CHK_TEARING(%d)",cmd[1]); break;
	case MIFARE_ULEV1_VCSL:     snprintf(exp,size,"VCSL"); break;
	default:                        snprintf(exp,size,"?"); break;
	}
	return;
}

void annotateMifare(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize, uint8_t* parity, uint8_t paritysize, bool isResponse) {
	if (!isResponse && cmdsize == 1) {
		switch(cmd[0]) {
			case ISO14443A_CMD_WUPA:
			case ISO14443A_CMD_REQA:
				MifareAuthState = masNone;
				break;
			default:
				break;
		}
	}

	// get UID
	if (MifareAuthState == masNone) {
		if (cmdsize == 9 && cmd[0] == ISO14443A_CMD_ANTICOLL_OR_SELECT && cmd[1] == 0x70) {
			ClearAuthData();
			AuthData.uid = bytes_to_num(&cmd[2], 4);
		}
		if (cmdsize == 9 && cmd[0] == ISO14443A_CMD_ANTICOLL_OR_SELECT_2 && cmd[1] == 0x70) {
			ClearAuthData();
			AuthData.uid = bytes_to_num(&cmd[2], 4);
		}
	}

	switch(MifareAuthState) {
		case masNt:
			if (cmdsize == 4 && isResponse) {
				snprintf(exp,size,"AUTH: nt %s", (AuthData.first_auth) ? "" : "(enc)");
				MifareAuthState = masNrAr;
				if (AuthData.first_auth) {
					AuthData.nt = bytes_to_num(cmd, 4);
				} else {
					AuthData.nt_enc = bytes_to_num(cmd, 4);
					AuthData.nt_enc_par = parity[0];
				}
				return;
			} else {
				MifareAuthState = masError;
			}
			break;
		case masNrAr:
			if (cmdsize == 8 && !isResponse) {
				snprintf(exp,size,"AUTH: nr ar (enc)");
				MifareAuthState = masAt;
				AuthData.nr_enc = bytes_to_num(cmd, 4);
				AuthData.ar_enc = bytes_to_num(&cmd[4], 4);
				AuthData.ar_enc_par = parity[0] << 4;
				return;
			} else {
				MifareAuthState = masError;
			}
			break;
		case masAt:
			if (cmdsize == 4 && isResponse) {
				snprintf(exp,size,"AUTH: at (enc)");
				MifareAuthState = masAuthComplete;
				AuthData.at_enc = bytes_to_num(cmd, 4);
				AuthData.at_enc_par = parity[0];
				return;
			} else {
				MifareAuthState = masError;
			}
			break;
		default:
			break;
	}

	if (!isResponse && ((MifareAuthState == masNone) || (MifareAuthState == masError)))
		annotateIso14443a(exp, size, cmd, cmdsize);

}


static uint64_t GetCrypto1ProbableKey(TAuthData *ad) {
	struct Crypto1State *revstate = lfsr_recovery64(ad->ks2, ad->ks3);
	lfsr_rollback_word(revstate, 0, 0);
	lfsr_rollback_word(revstate, 0, 0);
	lfsr_rollback_word(revstate, ad->nr_enc, 1);
	lfsr_rollback_word(revstate, ad->uid ^ ad->nt, 0);

	uint64_t lfsr = 0;
	crypto1_get_lfsr(revstate, &lfsr);
	crypto1_destroy(revstate);

	return lfsr;
}


static bool NTParityChk(TAuthData *ad, uint32_t ntx) {
	if (
		(oddparity8(ntx >> 8 & 0xff) ^ (ntx & 0x01) ^ ((ad->nt_enc_par >> 5) & 0x01) ^ (ad->nt_enc & 0x01)) ||
		(oddparity8(ntx >> 16 & 0xff) ^ (ntx >> 8 & 0x01) ^ ((ad->nt_enc_par >> 6) & 0x01) ^ (ad->nt_enc >> 8 & 0x01)) ||
		(oddparity8(ntx >> 24 & 0xff) ^ (ntx >> 16 & 0x01) ^ ((ad->nt_enc_par >> 7) & 0x01) ^ (ad->nt_enc >> 16 & 0x01))
		)
		return false;

	uint32_t ar = prng_successor(ntx, 64);
	if (
		(oddparity8(ar >> 8 & 0xff) ^ (ar & 0x01) ^ ((ad->ar_enc_par >> 5) & 0x01) ^ (ad->ar_enc & 0x01)) ||
		(oddparity8(ar >> 16 & 0xff) ^ (ar >> 8 & 0x01) ^ ((ad->ar_enc_par >> 6) & 0x01) ^ (ad->ar_enc >> 8 & 0x01)) ||
		(oddparity8(ar >> 24 & 0xff) ^ (ar >> 16 & 0x01) ^ ((ad->ar_enc_par >> 7) & 0x01) ^ (ad->ar_enc >> 16 & 0x01))
		)
		return false;

	uint32_t at = prng_successor(ntx, 96);
	if (
		(oddparity8(ar & 0xff) ^ (at >> 24 & 0x01) ^ ((ad->ar_enc_par >> 4) & 0x01) ^ (ad->at_enc >> 24 & 0x01)) ||
		(oddparity8(at >> 8 & 0xff) ^ (at & 0x01) ^ ((ad->at_enc_par >> 5) & 0x01) ^ (ad->at_enc & 0x01)) ||
		(oddparity8(at >> 16 & 0xff) ^ (at >> 8 & 0x01) ^ ((ad->at_enc_par >> 6) & 0x01) ^ (ad->at_enc >> 8 & 0x01)) ||
		(oddparity8(at >> 24 & 0xff) ^ (at >> 16 & 0x01) ^ ((ad->at_enc_par >> 7) & 0x01) ^ (ad->at_enc >> 16 & 0x01))
		)
		return false;

	return true;
}


static bool CheckCrypto1Parity(uint8_t *cmd_enc, uint8_t cmdsize, uint8_t *cmd, uint8_t *parity_enc) {
	for (int i = 0; i < cmdsize - 1; i++) {
		if (oddparity8(cmd[i]) ^ (cmd[i + 1] & 0x01) ^ ((parity_enc[i / 8] >> (7 - i % 8)) & 0x01) ^ (cmd_enc[i + 1] & 0x01))
			return false;
	}

	return true;
}


static bool NestedCheckKey(uint64_t key, TAuthData *ad, uint8_t *cmd, uint8_t cmdsize, uint8_t *parity) {
	uint8_t buf[32] = {0};
	struct Crypto1State *pcs;

	AuthData.ks2 = 0;
	AuthData.ks3 = 0;

	pcs = crypto1_create(key);
	uint32_t nt1 = crypto1_word(pcs, ad->nt_enc ^ ad->uid, 1) ^ ad->nt_enc;
	uint32_t ar = prng_successor(nt1, 64);
	uint32_t at = prng_successor(nt1, 96);

	crypto1_word(pcs, ad->nr_enc, 1);
//  uint32_t nr1 = crypto1_word(pcs, ad->nr_enc, 1) ^ ad->nr_enc;  // if needs deciphered nr
	uint32_t ar1 = crypto1_word(pcs, 0, 0) ^ ad->ar_enc;
	uint32_t at1 = crypto1_word(pcs, 0, 0) ^ ad->at_enc;

	if (!(ar == ar1 && at == at1 && NTParityChk(ad, nt1))) {
		crypto1_destroy(pcs);
		return false;
	}

	memcpy(buf, cmd, cmdsize);
	mf_crypto1_decrypt(pcs, buf, cmdsize, 0);

	crypto1_destroy(pcs);

	if (!CheckCrypto1Parity(cmd, cmdsize, buf, parity))
		return false;

	if(!CheckCrc14443(CRC_14443_A, buf, cmdsize))
		return false;

	AuthData.nt = nt1;
	AuthData.ks2 = AuthData.ar_enc ^ ar;
	AuthData.ks3 = AuthData.at_enc ^ at;

	return true;
}


static bool DecodeMifareData(uint8_t *cmd, uint8_t cmdsize, uint8_t *parity, bool isResponse, uint8_t *mfData, size_t *mfDataLen) {
	static struct Crypto1State *traceCrypto1;
	static uint64_t mfLastKey;

	*mfDataLen = 0;

	if (MifareAuthState == masAuthComplete) {
		if (traceCrypto1) {
			crypto1_destroy(traceCrypto1);
			traceCrypto1 = NULL;
		}

		MifareAuthState = masFirstData;
		return false;
	}

	if (cmdsize > 32)
		return false;

	if (MifareAuthState == masFirstData) {
		if (AuthData.first_auth) {
			AuthData.ks2 = AuthData.ar_enc ^ prng_successor(AuthData.nt, 64);
			AuthData.ks3 = AuthData.at_enc ^ prng_successor(AuthData.nt, 96);

			mfLastKey = GetCrypto1ProbableKey(&AuthData);
			PrintAndLog("            |          * | key | probable key:%012"PRIx64" Prng:%s   ks2:%08x ks3:%08x |     |",
				mfLastKey,
				validate_prng_nonce(AuthData.nt) ? "WEAK": "HARD",
				AuthData.ks2,
				AuthData.ks3);

			AuthData.first_auth = false;

			traceCrypto1 = lfsr_recovery64(AuthData.ks2, AuthData.ks3);
		} else {
			if (traceCrypto1) {
				crypto1_destroy(traceCrypto1);
				traceCrypto1 = NULL;
			}

			// check last used key
			if (mfLastKey) {
				if (NestedCheckKey(mfLastKey, &AuthData, cmd, cmdsize, parity)) {
					PrintAndLog("            |          * | key | last used key:%012"PRIx64"            ks2:%08x ks3:%08x |     |",
						mfLastKey,
						AuthData.ks2,
						AuthData.ks3);

				traceCrypto1 = lfsr_recovery64(AuthData.ks2, AuthData.ks3);
				};
			}

			// check default keys
			if (!traceCrypto1) {
				for (int defaultKeyCounter = 0; defaultKeyCounter < MifareDefaultKeysSize; defaultKeyCounter++){
					if (NestedCheckKey(MifareDefaultKeys[defaultKeyCounter], &AuthData, cmd, cmdsize, parity)) {
						PrintAndLog("            |          * | key | default key:%012"PRIx64"              ks2:%08x ks3:%08x |     |",
							MifareDefaultKeys[defaultKeyCounter],
							AuthData.ks2,
							AuthData.ks3);

						mfLastKey = MifareDefaultKeys[defaultKeyCounter];
						traceCrypto1 = lfsr_recovery64(AuthData.ks2, AuthData.ks3);
						break;
					};
				}
			}

			// nested
			if (!traceCrypto1 && validate_prng_nonce(AuthData.nt)) {
				uint32_t ntx = prng_successor(AuthData.nt, 90);
				for (int i = 0; i < 16383; i++) {
					ntx = prng_successor(ntx, 1);
					if (NTParityChk(&AuthData, ntx)){

						uint32_t ks2 = AuthData.ar_enc ^ prng_successor(ntx, 64);
						uint32_t ks3 = AuthData.at_enc ^ prng_successor(ntx, 96);
						struct Crypto1State *pcs = lfsr_recovery64(ks2, ks3);
						memcpy(mfData, cmd, cmdsize);
						mf_crypto1_decrypt(pcs, mfData, cmdsize, 0);

						crypto1_destroy(pcs);
						if (CheckCrypto1Parity(cmd, cmdsize, mfData, parity) && CheckCrc14443(CRC_14443_A, mfData, cmdsize)) {
							AuthData.ks2 = ks2;
							AuthData.ks3 = ks3;

							AuthData.nt = ntx;
							mfLastKey = GetCrypto1ProbableKey(&AuthData);
							PrintAndLog("            |          * | key | nested probable key:%012"PRIx64"      ks2:%08x ks3:%08x |     |",
								mfLastKey,
								AuthData.ks2,
								AuthData.ks3);

							traceCrypto1 = lfsr_recovery64(AuthData.ks2, AuthData.ks3);
							break;
						}
					}
				}
			}

			//hardnested
			if (!traceCrypto1) {
				printf("hardnested not implemented. uid:%x nt:%x ar_enc:%x at_enc:%x\n", AuthData.uid, AuthData.nt, AuthData.ar_enc, AuthData.at_enc);
				MifareAuthState = masError;

				/* TOO SLOW( needs to have more strong filter. with this filter - aprox 4 mln tests
				uint32_t t = msclock();
				uint32_t t1 = t;
				int n = 0;
				for (uint32_t i = 0; i < 0xFFFFFFFF; i++) {
					if (NTParityChk(&AuthData, i)){

						uint32_t ks2 = AuthData.ar_enc ^ prng_successor(i, 64);
						uint32_t ks3 = AuthData.at_enc ^ prng_successor(i, 96);
						struct Crypto1State *pcs = lfsr_recovery64(ks2, ks3);




						n++;

						if (!(n % 100000)) {
							printf("delta=%d n=%d ks2=%x ks3=%x \n", msclock() - t1 , n, ks2, ks3);
							t1 = msclock();
						}

					}
				}
				printf("delta=%d n=%d\n", msclock() - t, n);
				*/
			}
		}



		MifareAuthState = masData;
	}

	if (MifareAuthState == masData && traceCrypto1) {
		memcpy(mfData, cmd, cmdsize);
		mf_crypto1_decrypt(traceCrypto1, mfData, cmdsize, 0);
		*mfDataLen = cmdsize;
	}

	return *mfDataLen > 0;
}


bool is_last_record(uint16_t tracepos, uint8_t *trace, uint16_t traceLen) {
	return(tracepos + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) >= traceLen);
}


bool next_record_is_response(uint16_t tracepos, uint8_t *trace) {
	uint16_t next_records_datalen = *((uint16_t *)(trace + tracepos + sizeof(uint32_t) + sizeof(uint16_t)));
	return(next_records_datalen & 0x8000);
}


bool merge_topaz_reader_frames(uint32_t timestamp, uint32_t *duration, uint16_t *tracepos, uint16_t traceLen, uint8_t *trace, uint8_t *frame, uint8_t *topaz_reader_command, uint16_t *data_len) {

#define MAX_TOPAZ_READER_CMD_LEN    16

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


uint16_t printTraceLine(uint16_t tracepos, uint16_t traceLen, uint8_t *trace, uint8_t protocol, bool showWaitCycles, bool markCRCBytes, uint32_t *prev_EOT, bool times_in_us) {
	bool isResponse;
	uint16_t data_len, parity_len;
	uint32_t duration;
	uint8_t topaz_reader_command[9];
	uint32_t timestamp, first_timestamp;
	uint32_t EndOfTransmissionTimestamp = 0;
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

	// adjust for different time scales
	if (protocol == ICLASS || protocol == ISO_15693) {
		duration *= 32;
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
			case ISO_14443_4:
				crcStatus = iso14443_4_CRC_check(frame, data_len);
				break;
			case ISO_15693:
				crcStatus = iso15693_CRC_check(frame, data_len);
				break;
			default:
				break;
		}
	}
	//0 CRC-command, CRC not ok
	//1 CRC-command, CRC ok
	//2 Not crc-command

	//--- Draw the data column
	char line[16][110];

	for (int j = 0; j < data_len && j/16 < 16; j++) {
		uint8_t parityBits = parityBytes[j>>3];
		if (protocol != ISO_14443B
			&& protocol != ISO_15693
			&& protocol != ICLASS
			&& protocol != ISO_7816_4
			&& (isResponse || protocol == ISO_14443A)
			&& (oddparity8(frame[j]) != ((parityBits >> (7-(j&0x0007))) & 0x01))) {
			snprintf(line[j/16]+(( j % 16) * 4), 110, " %02x!", frame[j]);
		} else {
			snprintf(line[j/16]+(( j % 16) * 4), 110, " %02x ", frame[j]);
		}
	}

	if (markCRCBytes) {
		if (crcStatus == 0 || crcStatus == 1) { //CRC-command
			char *pos1 = line[(data_len-2)/16]+(((data_len-2) % 16) * 4);
			(*pos1) = '[';
			char *pos2 = line[(data_len)/16]+(((data_len) % 16) * 4);
			sprintf(pos2, "%c", ']');
		}
	}

	// mark short bytes (less than 8 Bit + Parity)
	if (protocol == ISO_14443A || protocol == PROTO_MIFARE) {
		if (duration < 128 * (9 * data_len)) {
			line[(data_len-1)/16][((data_len-1)%16) * 4 + 3] = '\'';
		}
	}

	if (data_len == 0) {
		if (protocol == ICLASS && duration == 2048) {
			sprintf(line[0], " <SOF>");
		} else if (protocol == ISO_15693 && duration == 512) {
			sprintf(line[0], " <EOF>");
		} else {
			sprintf(line[0], " <empty trace - possible error>");
		}
	}

	//--- Draw the CRC column
	char *crc = (crcStatus == 0 ? "!crc" : (crcStatus == 1 ? " ok " : "    "));

	if (protocol == PROTO_MIFARE)
		annotateMifare(explanation, sizeof(explanation), frame, data_len, parityBytes, parity_len, isResponse);

	if (!isResponse) {
		switch(protocol) {
			case ICLASS:      annotateIclass(explanation,sizeof(explanation),frame,data_len); break;
			case ISO_14443A:  annotateIso14443a(explanation,sizeof(explanation),frame,data_len); break;
			case ISO_14443B:  annotateIso14443b(explanation,sizeof(explanation),frame,data_len); break;
			case TOPAZ:       annotateTopaz(explanation,sizeof(explanation),frame,data_len); break;
			case ISO_15693:   annotateIso15693(explanation,sizeof(explanation),frame,data_len); break;
			case ISO_7816_4:  annotateIso7816(explanation, sizeof(explanation), frame, data_len); break;
			case ISO_14443_4: annotateIso14443_4(explanation, sizeof(explanation), frame, data_len); break;
			default:          break;
		}
	}

	uint32_t previousEndOfTransmissionTimestamp = 0;
	if (prev_EOT) {
		if (*prev_EOT) {
			previousEndOfTransmissionTimestamp = *prev_EOT;
		} else {
			previousEndOfTransmissionTimestamp = timestamp;
		}
	}		
	EndOfTransmissionTimestamp = timestamp + duration;
	if (prev_EOT) *prev_EOT = EndOfTransmissionTimestamp;

	int num_lines = MIN((data_len - 1)/16 + 1, 16);
	for (int j = 0; j < num_lines ; j++) {
		if (j == 0) {
			uint32_t time1 = timestamp - first_timestamp;
			uint32_t time2 = EndOfTransmissionTimestamp - first_timestamp;
			if (prev_EOT) {
				time1 = timestamp - previousEndOfTransmissionTimestamp;
				time2 = duration;
			}
			if (times_in_us) {
				PrintAndLog(" %10.1f | %10.1f | %s |%-64s | %s| %s",
					(float)time1/13.56,
					(float)time2/13.56,
					isResponse ? "Tag" : "Rdr",
					line[j],
					(j == num_lines-1) ? crc : "    ",
					(j == num_lines-1) ? explanation : "");
			} else {
				PrintAndLog(" %10" PRIu32 " | %10" PRIu32 " | %s |%-64s | %s| %s",
					time1,
					time2,
					isResponse ? "Tag" : "Rdr",
					line[j],
					(j == num_lines-1) ? crc : "    ",
					(j == num_lines-1) ? explanation : "");
			}
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


int CmdHFList(const char *Cmd) {

	CLIParserInit("hf list", "\nList or save protocol data.",
		"examples: hf list 14a -f                    -- interpret as ISO14443A communication and display Frame Delay Times\n"\
		"          hf list iclass                    -- interpret as iClass trace\n"\
		"          hf list -s myCardTrace.trc        -- save trace for later use\n"\
		"          hf list 14a -l myCardTrace.trc    -- load trace and interpret as ISO14443A communication\n");
	void* argtable[] = {
		arg_param_begin,
		arg_lit0("f",  "fdt",      "display fdt (frame delay times)"),
		arg_lit0("r",  "relative", "show relative times (gap and duration)"),
		arg_lit0("c",  "crc"  ,    "mark CRC bytes"),
		arg_lit0("p",  "pcsc",     "show trace buffer from PCSC card reader instead of PM3"),
		arg_str0("l",  "load",     "<filename>", "load trace from file"),
		arg_str0("s",  "save",     "<filename>", "save trace to file"),
		arg_lit0("u",  "us",       "display times in microseconds instead of clock cycles"),
		arg_str0(NULL,  NULL,      "<protocol>", "protocol to interpret. Possible values:\n"\
			"\traw    - just show raw data without annotations (default)\n"\
            "\t14a    - interpret data as ISO14443A communications\n"\
            "\tmf     - interpret data as ISO14443A communications and decrypt Mifare Crypto1 stream\n"\
            "\t14b    - interpret data as ISO14443B communications\n"\
            "\t15     - interpret data as ISO15693 communications\n"\
            "\ticlass - interpret data as iClass communications\n"\
            "\ttopaz  - interpret data as Topaz communications\n"\
            "\t7816   - interpret data as 7816-4 APDU communications\n"\
            "\t14-4   - interpret data as ISO14443-4 communications"),
		arg_param_end
	};

	if (CLIParserParseString(Cmd, argtable, arg_getsize(argtable), true)){
		CLIParserFree();
		return 0;
	}

	bool showWaitCycles = arg_get_lit(1);
	bool relative_times = arg_get_lit(2);
	bool markCRCBytes   = arg_get_lit(3);
	bool PCSCtrace      = arg_get_lit(4);
	bool loadFromFile   = arg_get_str_len(5);
	bool saveToFile     = arg_get_str_len(6);
	bool times_in_us    = arg_get_lit(7);

	uint32_t previous_EOT = 0;
	uint32_t *prev_EOT = NULL;
	if (relative_times) {
		prev_EOT = &previous_EOT;
	}
	
	char load_filename[FILE_PATH_SIZE+1] = {0};
	if (loadFromFile) {
		strncpy(load_filename, arg_get_str(5)->sval[0], FILE_PATH_SIZE);
	}
	char save_filename[FILE_PATH_SIZE+1] = {0};
	if (saveToFile) {
		strncpy(save_filename, arg_get_str(6)->sval[0], FILE_PATH_SIZE);
	}
	
	uint8_t protocol = -1;
	if (arg_get_str_len(8)) {
		if     (strcmp(arg_get_str(8)->sval[0], "iclass") == 0) protocol = ICLASS;
		else if(strcmp(arg_get_str(8)->sval[0], "14a") == 0)    protocol = ISO_14443A;
		else if(strcmp(arg_get_str(8)->sval[0], "mf") == 0)     protocol = PROTO_MIFARE;
		else if(strcmp(arg_get_str(8)->sval[0], "14b") == 0)    protocol = ISO_14443B;
		else if(strcmp(arg_get_str(8)->sval[0], "topaz") == 0)  protocol = TOPAZ;
		else if(strcmp(arg_get_str(8)->sval[0], "7816") == 0)   protocol = ISO_7816_4;
		else if(strcmp(arg_get_str(8)->sval[0], "14-4") == 0)   protocol = ISO_14443_4;
		else if(strcmp(arg_get_str(8)->sval[0], "15") == 0)     protocol = ISO_15693;
		else if(strcmp(arg_get_str(8)->sval[0], "raw") == 0)    protocol = -1;//No crc, no annotations
		else {
			PrintAndLog("hf list: invalid argument \"%s\"\nTry 'hf list --help' for more information.", arg_get_str(8)->sval[0]);
			CLIParserFree();
			return 0;
		}
	}

	CLIParserFree();


	uint8_t *trace;
	uint32_t tracepos = 0;
	uint32_t traceLen = 0;
	
	if (loadFromFile) {
		#define TRACE_CHUNK_SIZE (1<<16)        // 64K to start with. Will be enough for BigBuf and some room for future extensions
		FILE *tracefile = NULL;
		size_t bytes_read;
		trace = malloc(TRACE_CHUNK_SIZE);
		if (trace == NULL) {
			PrintAndLog("Cannot allocate memory for trace");
			return 2;
		}
		if ((tracefile = fopen(load_filename,"rb")) == NULL) {
			PrintAndLog("Could not open file %s", load_filename);
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
	} else if (PCSCtrace) {
		trace = pcsc_get_trace_addr();
		traceLen = pcsc_get_traceLen();
	} else {
		trace = malloc(USB_CMD_DATA_SIZE);
		// Query for the size of the trace
		UsbCommand response;
		if (!(GetFromBigBuf(trace, USB_CMD_DATA_SIZE, 0, &response, 500, false))) {
			return 1;
		}
		traceLen = response.arg[2];
		if (traceLen > USB_CMD_DATA_SIZE) {
			uint8_t *p = realloc(trace, traceLen);
			if (p == NULL) {
				PrintAndLog("Cannot allocate memory for trace");
				free(trace);
				return 2;
			}
			trace = p;
			if (!(GetFromBigBuf(trace, traceLen, 0, NULL, 500, false))) {
				return 1;
			}
		}
	}

	if (saveToFile) {
		FILE *tracefile = NULL;
		if ((tracefile = fopen(save_filename,"wb")) == NULL) {
			PrintAndLog("Could not create file %s", save_filename);
			return 1;
		}
		fwrite(trace, 1, traceLen, tracefile);
		PrintAndLog("Recorded Activity (TraceLen = %d bytes) written to file %s", traceLen, save_filename);
		fclose(tracefile);
	} else {
		PrintAndLog("Recorded Activity (TraceLen = %d bytes)", traceLen);
		PrintAndLog("");
		if (relative_times) {
			PrintAndLog("Gap = time between transfers. Duration = duration of data transfer. Src = Source of transfer");
		} else {
			PrintAndLog("Start = Start of Frame, End = End of Frame. Src = Source of transfer");
		}
		if (times_in_us) {
			PrintAndLog("All times are in microseconds");
		} else {
			PrintAndLog("All times are in carrier periods (1/13.56Mhz)");
		}
		PrintAndLog("");
		if (relative_times) {
			PrintAndLog("        Gap |   Duration | Src | Data (! denotes parity error, ' denotes short bytes)            | CRC | Annotation         |");
		} else {
			PrintAndLog("      Start |        End | Src | Data (! denotes parity error, ' denotes short bytes)            | CRC | Annotation         |");
		}
		PrintAndLog("------------|------------|-----|-----------------------------------------------------------------|-----|--------------------|");

		ClearAuthData();
		while(tracepos < traceLen) {
			tracepos = printTraceLine(tracepos, traceLen, trace, protocol, showWaitCycles, markCRCBytes, prev_EOT, times_in_us);
		}
	}

	free(trace);
	return 0;
}

