//-----------------------------------------------------------------------------
// Merlok - June 2011
// Gerhard de Koning Gans - May 2008
// Hagen Fritsch - June 2010
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support ISO 14443 type A.
//-----------------------------------------------------------------------------

#ifndef MIFARECMD_H__
#define MIFARECMD_H__

#include <stdint.h>

extern void MifareReadBlock(uint8_t arg0, uint8_t arg1, uint8_t arg2, uint8_t *data);
extern void MifareUReadBlock(uint8_t arg0, uint8_t arg1, uint8_t *datain);
extern void MifareUC_Auth(uint8_t arg0, uint8_t *datain);
extern void MifareUReadCard(uint8_t arg0, uint16_t arg1, uint8_t arg2, uint8_t *datain);
extern void MifareReadSector(uint8_t arg0, uint8_t arg1, uint8_t arg2, uint8_t *datain);
extern void MifareWriteBlock(uint8_t arg0, uint8_t arg1, uint8_t arg2, uint8_t *datain);
//extern void MifareUWriteBlockCompat(uint8_t arg0,uint8_t *datain);
extern void MifareUWriteBlock(uint8_t arg0, uint8_t arg1, uint8_t *datain);
extern void MifareNested(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void MifareAcquireEncryptedNonces(uint32_t arg0, uint32_t arg1, uint32_t flags, uint8_t *datain);
extern void MifareChkKeys(uint16_t arg0, uint16_t arg1, uint8_t arg2, uint8_t *datain);
extern void MifareSetDbgLvl(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void MifareEMemClr(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void MifareEMemSet(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void MifareEMemGet(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void MifareECardLoad(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void MifareCWipe(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);       // Work with "magic Chinese" card
extern void MifareCSetBlock(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void MifareCGetBlock(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void MifareCIdent();  // is "magic chinese" card?
extern void MifareUSetPwd(uint8_t arg0, uint8_t *datain);
extern void MifarePersonalizeUID(uint8_t keyType, uint8_t perso_option, uint8_t *datain);

//desfire
extern void Mifare_DES_Auth1(uint8_t arg0,uint8_t *datain);
extern void Mifare_DES_Auth2(uint32_t arg0, uint8_t *datain);

#endif
