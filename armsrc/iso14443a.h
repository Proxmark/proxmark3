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

#ifndef __ISO14443A_H
#define __ISO14443A_H

#include <stdint.h>
#include <stdbool.h>
#include "mifare.h"

extern void GetParity(const uint8_t *pbtCmd, uint16_t len, uint8_t *par);
extern void AppendCrc14443a(uint8_t *data, int len);

extern void ReaderTransmit(uint8_t *frame, uint16_t len, uint32_t *timing);
extern void ReaderTransmitBitsPar(uint8_t *frame, uint16_t bits, uint8_t *par, uint32_t *timing);
extern void ReaderTransmitPar(uint8_t *frame, uint16_t len, uint8_t *par, uint32_t *timing);
extern int ReaderReceive(uint8_t *receivedAnswer, uint8_t *par);

extern void iso14443a_setup(uint8_t fpga_minor_mode);
extern int iso14_apdu(uint8_t *cmd, uint16_t cmd_len, void *data);
extern int iso14443a_select_card(uint8_t *uid_ptr, iso14a_card_select_t *resp_data, uint32_t *cuid_ptr, bool anticollision, uint8_t num_cascades);
extern void iso14a_set_trigger(bool enable);
#endif /* __ISO14443A_H */
