//-----------------------------------------------------------------------------
// Gerhard de Koning Gans - May 2008
// Hagen Fritsch - June 2010
// Gerhard de Koning Gans - May 2011
// Gerhard de Koning Gans - June 2012 - Added iClass card and reader emulation
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support iClass.
//-----------------------------------------------------------------------------

#ifndef ICLASS_H__
#define ICLASS_H__

#include <stdint.h>
#include "common.h" // for RAMFUNC

extern void RAMFUNC SnoopIClass(void);
extern void SimulateIClass(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain);
extern void ReaderIClass(uint8_t arg0);
extern void ReaderIClass_Replay(uint8_t arg0, uint8_t *MAC);
extern void IClass_iso14443A_GetPublic(uint8_t arg0);
extern void iClass_Authentication(uint8_t *MAC);
extern void iClass_WriteBlock(uint8_t blockNo, uint8_t *data);
extern void iClass_ReadBlk(uint8_t blockNo);
extern void iClass_Dump(uint8_t blockno, uint8_t numblks);
extern void iClass_Clone(uint8_t startblock, uint8_t endblock, uint8_t *data);

#endif
