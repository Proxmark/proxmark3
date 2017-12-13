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
#include "usb_cmd.h"
#include "mifare.h"

typedef struct {
  uint8_t* response;
  uint8_t* modulation;
  uint16_t response_n;
  uint16_t modulation_n;
  uint32_t ProxToAirDuration;
  uint8_t  par; // enough for precalculated parity of 8 Byte responses
} tag_response_info_t;

void GetParity(const uint8_t *pbtCmd, uint16_t len, uint8_t *par);
void AppendCrc14443a(uint8_t *data, int len);

void RAMFUNC SnoopIso14443a(uint8_t param);
void SimulateIso14443aTag(int tagType, int uid_1st, int uid_2nd, byte_t *data);
void ReaderIso14443a(UsbCommand *c);
void ReaderTransmit(uint8_t *frame, uint16_t len, uint32_t *timing);
void ReaderTransmitBitsPar(uint8_t *frame, uint16_t bits, uint8_t *par, uint32_t *timing);
void ReaderTransmitPar(uint8_t *frame, uint16_t len, uint8_t *par, uint32_t *timing);
int ReaderReceive(uint8_t *receivedAnswer, uint8_t *par);
void ReaderMifare(bool first_try);

int EmGetCmd(uint8_t *received, uint16_t *len, uint8_t *parity);
int EmSendCmd(uint8_t *resp, uint16_t respLen);
int EmSendCmdEx(uint8_t *resp, uint16_t respLen);
int EmSend4bit(uint8_t resp);
int EmSendCmdPar(uint8_t *resp, uint16_t respLen, uint8_t *par);
int EmSendPrecompiledCmd(tag_response_info_t *response_info);

bool prepare_allocated_tag_modulation(tag_response_info_t *response_info, uint8_t **buffer, size_t *buffer_size);

void iso14443a_setup(uint8_t fpga_minor_mode);
int iso14_apdu(uint8_t *cmd, uint16_t cmd_len, void *data);
int iso14443a_select_card(uint8_t *uid_ptr, iso14a_card_select_t *resp_data, uint32_t *cuid_ptr, bool anticollision, uint8_t num_cascades, bool no_rats);
void iso14a_set_trigger(bool enable);
void iso14a_set_timeout(uint32_t timeout);
#endif /* __ISO14443A_H */
