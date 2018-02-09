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
#include "data.h"
#include "ui.h"
#include "iso14443crc.h"
#include "parity.h"
#include "protocols.h"


enum MifareAuthSeq {
	masNone,
	masNt,
	masNrAr,
	masAt,
	masData,
	masDataNested,
	masError,
};
static enum MifareAuthSeq MifareAuthState;

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
		case masData:
		case masDataNested:
		case masError:
			return iso14443A_CRC_check(isResponse, data, len);
		default:
			return 2;
	}

}

void annotateIso14443a(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize)
{
	switch(cmd[0])
	{
	case ISO14443A_CMD_WUPA:        snprintf(exp,size,"WUPA"); break;
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
	case ISO14443A_CMD_REQA:		snprintf(exp,size,"REQA"); break;
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

void annotateMifare(char *exp, size_t size, uint8_t* cmd, uint8_t cmdsize, bool isResponse) {
	switch(MifareAuthState) {
		case masNt:
			if (cmdsize == 4) {
				snprintf(exp,size,"AUTH: nt");
				MifareAuthState = masNrAr;
				printf("--ntok\n");
				return;
			} else {
				MifareAuthState = masError;
				printf("--err %d\n", cmdsize);
			}
			break;
		case masNrAr:
			if (cmdsize == 8) {
				snprintf(exp,size,"AUTH: nr ar");
				MifareAuthState = masAt;
				return;
			} else {
				MifareAuthState = masError;
			}
			break;
		case masAt:
			if (cmdsize == 4) {
				snprintf(exp,size,"AUTH: at");
				MifareAuthState = masData;
				return;
			} else {
				MifareAuthState = masError;
			}
			break;
		default:
			break;
	}
	
	if (!isResponse)
		annotateIso14443a(exp, size, cmd, cmdsize);
	
}
