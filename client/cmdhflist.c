//-----------------------------------------------------------------------------
// Copyright (C) Merlok - 2017
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Command: hf mf list. It shows data from arm buffer.
//-----------------------------------------------------------------------------

#include "cmdhflist.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "ui.h"
#include "iso14443crc.h"
#include "parity.h"
#include "protocols.h"
#include "crapto1/crapto1.h"
#include "mifarehost.h"
#include "mifaredefault.h"


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

void ClearAuthData() {
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
uint8_t iso14443A_CRC_check(bool isResponse, uint8_t* data, uint8_t len)
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

uint8_t mifare_CRC_check(bool isResponse, uint8_t* data, uint8_t len)
{
	switch(MifareAuthState) {
		case masNone:
		case masError:
			return iso14443A_CRC_check(isResponse, data, len);
		default:
			return 2;
	}
}

void annotateIclass(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize)
{
	switch(cmd[0])
	{
	case ICLASS_CMD_ACTALL:      snprintf(exp,size,"ACTALL"); break;
	case ICLASS_CMD_READ_OR_IDENTIFY:{
		if(cmdsize > 1){
			snprintf(exp,size,"READ(%d)",cmd[1]);
		}else{
			snprintf(exp,size,"IDENTIFY");
		}
		break;
	}
	case ICLASS_CMD_SELECT:      snprintf(exp,size,"SELECT"); break;
	case ICLASS_CMD_PAGESEL:     snprintf(exp,size,"PAGESEL(%d)", cmd[1]); break;
	case ICLASS_CMD_READCHECK_KC:snprintf(exp,size,"READCHECK[Kc](%d)", cmd[1]); break;
	case ICLASS_CMD_READCHECK_KD:snprintf(exp,size,"READCHECK[Kd](%d)", cmd[1]); break;
	case ICLASS_CMD_CHECK:       snprintf(exp,size,"CHECK"); break;
	case ICLASS_CMD_DETECT:      snprintf(exp,size,"DETECT"); break;
	case ICLASS_CMD_HALT:        snprintf(exp,size,"HALT"); break;
	case ICLASS_CMD_UPDATE:      snprintf(exp,size,"UPDATE(%d)",cmd[1]); break;
	case ICLASS_CMD_ACT:         snprintf(exp,size,"ACT"); break;
	case ICLASS_CMD_READ4:       snprintf(exp,size,"READ4(%d)",cmd[1]); break;
	default:                     snprintf(exp,size,"?"); break;
	}
	return;
}

void annotateIso15693(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize)
{

	if(cmd[0] == 0x26)
	{
		switch(cmd[1]){
		case ISO15693_INVENTORY           :snprintf(exp, size, "INVENTORY");break;
		case ISO15693_STAYQUIET           :snprintf(exp, size, "STAY_QUIET");break;
		default:                     snprintf(exp,size,"?"); break;

		}
	}else if(cmd[0] == 0x02)
	{
		switch(cmd[1])
		{
		case ISO15693_READBLOCK            :snprintf(exp, size, "READBLOCK");break;
		case ISO15693_WRITEBLOCK           :snprintf(exp, size, "WRITEBLOCK");break;
		case ISO15693_LOCKBLOCK            :snprintf(exp, size, "LOCKBLOCK");break;
		case ISO15693_READ_MULTI_BLOCK     :snprintf(exp, size, "READ_MULTI_BLOCK");break;
		case ISO15693_SELECT               :snprintf(exp, size, "SELECT");break;
		case ISO15693_RESET_TO_READY       :snprintf(exp, size, "RESET_TO_READY");break;
		case ISO15693_WRITE_AFI            :snprintf(exp, size, "WRITE_AFI");break;
		case ISO15693_LOCK_AFI             :snprintf(exp, size, "LOCK_AFI");break;
		case ISO15693_WRITE_DSFID          :snprintf(exp, size, "WRITE_DSFID");break;
		case ISO15693_LOCK_DSFID           :snprintf(exp, size, "LOCK_DSFID");break;
		case ISO15693_GET_SYSTEM_INFO      :snprintf(exp, size, "GET_SYSTEM_INFO");break;
		case ISO15693_READ_MULTI_SECSTATUS :snprintf(exp, size, "READ_MULTI_SECSTATUS");break;
		default:                            snprintf(exp,size,"?"); break;
		}
	}
}


void annotateTopaz(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize)
{
	switch(cmd[0]) {
		case TOPAZ_REQA						:snprintf(exp, size, "REQA");break;
		case TOPAZ_WUPA						:snprintf(exp, size, "WUPA");break;
		case TOPAZ_RID						:snprintf(exp, size, "RID");break;
		case TOPAZ_RALL						:snprintf(exp, size, "RALL");break;
		case TOPAZ_READ						:snprintf(exp, size, "READ");break;
		case TOPAZ_WRITE_E					:snprintf(exp, size, "WRITE-E");break;
		case TOPAZ_WRITE_NE					:snprintf(exp, size, "WRITE-NE");break;
		case TOPAZ_RSEG						:snprintf(exp, size, "RSEG");break;
		case TOPAZ_READ8					:snprintf(exp, size, "READ8");break;
		case TOPAZ_WRITE_E8					:snprintf(exp, size, "WRITE-E8");break;
		case TOPAZ_WRITE_NE8				:snprintf(exp, size, "WRITE-NE8");break;
		default:                            snprintf(exp,size,"?"); break;
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

void annotateIso14443b(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize)
{
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

void annotateIso14443a(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize)
{
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
	case ISO14443A_CMD_READBLOCK:	snprintf(exp,size,"READBLOCK(%d)",cmd[1]); break;
	case ISO14443A_CMD_WRITEBLOCK:	snprintf(exp,size,"WRITEBLOCK(%d)",cmd[1]); break;
	case ISO14443A_CMD_HALT:		
		snprintf(exp,size,"HALT"); 
		MifareAuthState = masNone;
		break;
	case ISO14443A_CMD_RATS:		snprintf(exp,size,"RATS"); break;
	case MIFARE_CMD_INC:			snprintf(exp,size,"INC(%d)",cmd[1]); break;
	case MIFARE_CMD_DEC:			snprintf(exp,size,"DEC(%d)",cmd[1]); break;
	case MIFARE_CMD_RESTORE:		snprintf(exp,size,"RESTORE(%d)",cmd[1]); break;
	case MIFARE_CMD_TRANSFER:		snprintf(exp,size,"TRANSFER(%d)",cmd[1]); break;
	case MIFARE_AUTH_KEYA:
		if ( cmdsize > 3) {
			snprintf(exp,size,"AUTH-A(%d)",cmd[1]); 
			MifareAuthState = masNt;
		} else {
			//	case MIFARE_ULEV1_VERSION :  both 0x60.
			snprintf(exp,size,"EV1 VERSION");
		}
		break;
	case MIFARE_AUTH_KEYB:
		MifareAuthState = masNt;
		snprintf(exp,size,"AUTH-B(%d)",cmd[1]); 
		break;
	case MIFARE_MAGICWUPC1:			snprintf(exp,size,"MAGIC WUPC1"); break;
	case MIFARE_MAGICWUPC2:			snprintf(exp,size,"MAGIC WUPC2"); break;
	case MIFARE_MAGICWIPEC:			snprintf(exp,size,"MAGIC WIPEC"); break;
	case MIFARE_ULC_AUTH_1:		snprintf(exp,size,"AUTH "); break;
	case MIFARE_ULC_AUTH_2:		snprintf(exp,size,"AUTH_ANSW"); break;
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
	case MIFARE_ULEV1_READSIG:		snprintf(exp,size,"READ_SIG"); break;
	case MIFARE_ULEV1_CHECKTEAR:	snprintf(exp,size,"CHK_TEARING(%d)",cmd[1]); break;
	case MIFARE_ULEV1_VCSL:		snprintf(exp,size,"VCSL"); break;
	default:						snprintf(exp,size,"?"); break;
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

bool DecodeMifareData(uint8_t *cmd, uint8_t cmdsize, uint8_t *parity, bool isResponse, uint8_t *mfData, size_t *mfDataLen) {
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

bool NTParityChk(TAuthData *ad, uint32_t ntx) {
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

bool NestedCheckKey(uint64_t key, TAuthData *ad, uint8_t *cmd, uint8_t cmdsize, uint8_t *parity) {
	uint8_t buf[32] = {0};
	struct Crypto1State *pcs;
	
	AuthData.ks2 = 0;
	AuthData.ks3 = 0;

	pcs = crypto1_create(key);
	uint32_t nt1 = crypto1_word(pcs, ad->nt_enc ^ ad->uid, 1) ^ ad->nt_enc;
	uint32_t ar = prng_successor(nt1, 64);
	uint32_t at = prng_successor(nt1, 96);

	crypto1_word(pcs, ad->nr_enc, 1);
//	uint32_t nr1 = crypto1_word(pcs, ad->nr_enc, 1) ^ ad->nr_enc;  // if needs deciphered nr
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

bool CheckCrypto1Parity(uint8_t *cmd_enc, uint8_t cmdsize, uint8_t *cmd, uint8_t *parity_enc) {
	for (int i = 0; i < cmdsize - 1; i++) {
		if (oddparity8(cmd[i]) ^ (cmd[i + 1] & 0x01) ^ ((parity_enc[i / 8] >> (7 - i % 8)) & 0x01) ^ (cmd_enc[i + 1] & 0x01))
			return false;
	}
	
	return true;
}

uint64_t GetCrypto1ProbableKey(TAuthData *ad) {
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
