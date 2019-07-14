//-----------------------------------------------------------------------------
// Piwi - October 2018
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support ISO 15693.
//-----------------------------------------------------------------------------

#ifndef __ISO15693_H
#define __ISO15693_H

#include <stdint.h>

void SnoopIso15693(void);
void AcquireRawAdcSamplesIso15693(void);
void ReaderIso15693(uint32_t parameter);
void SimTagIso15693(uint32_t parameter, uint8_t *uid);
void BruteforceIso15693Afi(uint32_t speed);
void DirectTag15693Command(uint32_t datalen,uint32_t speed, uint32_t recv, uint8_t data[]); 
void SetTag15693Uid(uint8_t *uid);
void SetDebugIso15693(uint32_t flag);

#endif
