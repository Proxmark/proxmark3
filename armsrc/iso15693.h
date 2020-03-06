//-----------------------------------------------------------------------------
// Piwi - October 2018
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support ISO 15693.
//-----------------------------------------------------------------------------

#ifndef ISO15693_H__
#define ISO15693_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Delays in SSP_CLK ticks.
// SSP_CLK runs at 13,56MHz / 32 = 423.75kHz when simulating a tag
#define DELAY_ISO15693_VCD_TO_VICC_SIM     132  // 132/423.75kHz = 311.5us from end of command EOF to start of tag response
//SSP_CLK runs at 13.56MHz / 4 = 3,39MHz when acting as reader. All values should be multiples of 16
#define DELAY_ISO15693_VCD_TO_VICC_READER 1056 // 1056/3,39MHz = 311.5us from end of command EOF to start of tag response
#define DELAY_ISO15693_VICC_TO_VCD_READER 1024 // 1024/3.39MHz = 302.1us between end of tag response and next reader command

extern void Iso15693InitReader(void);
extern void Iso15693InitTag(void);
extern void CodeIso15693AsReader(uint8_t *cmd, int n);
extern void CodeIso15693AsTag(uint8_t *cmd, size_t len);
extern void TransmitTo15693Reader(const uint8_t *cmd, size_t len, uint32_t *start_time, uint32_t slot_time, bool slow);
extern int GetIso15693CommandFromReader(uint8_t *received, size_t max_len, uint32_t *eof_time);
extern void TransmitTo15693Tag(const uint8_t *cmd, int len, uint32_t *start_time);
extern int GetIso15693AnswerFromTag(uint8_t* response, uint16_t max_len, uint16_t timeout, uint32_t *eof_time);
extern void SnoopIso15693(uint8_t jam_search_len, uint8_t *jam_search_string);
extern void AcquireRawAdcSamplesIso15693(void);
extern void ReaderIso15693(uint32_t parameter);
extern void SimTagIso15693(uint32_t parameter, uint8_t *uid);
extern void BruteforceIso15693Afi(uint32_t speed);
extern void DirectTag15693Command(uint32_t datalen, uint32_t speed, uint32_t recv, uint8_t data[]);
extern void SetTag15693Uid(uint8_t *uid);
extern void SetDebugIso15693(uint32_t flag);
extern bool LogTrace_ISO15693(const uint8_t *btBytes, uint16_t iLen, uint32_t timestamp_start, uint32_t timestamp_end, uint8_t *parity, bool readerToTag);

#endif
