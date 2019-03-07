//-----------------------------------------------------------------------------
// Merlok - June 2011
// Gerhard de Koning Gans - May 2008
// Hagen Fritsch - June 2010
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support ISO 14443 type B.
//-----------------------------------------------------------------------------

#ifndef ISO14443B_H__
#define ISO14443B_H__

#include <stdint.h>
#include <stddef.h>

extern int iso14443b_apdu(uint8_t const *message, size_t message_length, uint8_t *response);
extern void iso14443b_setup();
extern int iso14443b_select_card();
extern void SimulateIso14443bTag(void);
extern void ReadSTMemoryIso14443b(uint32_t);
extern void SnoopIso14443b(void);
extern void SendRawCommand14443B(uint32_t, uint32_t, uint8_t, uint8_t[]);

#endif /* __ISO14443B_H */
