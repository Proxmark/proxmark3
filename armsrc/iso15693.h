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
#define DELAY_READER_TO_ARM_SIM           8
#define DELAY_ARM_TO_READER_SIM           0
#define DELAY_ISO15693_VCD_TO_VICC_SIM    132  // 132/423.75kHz = 311.5us from end of command EOF to start of tag response
//SSP_CLK runs at 13.56MHz / 4 = 3,39MHz when acting as reader
#define DELAY_ISO15693_VCD_TO_VICC_READER 1056 // 1056/3,39MHz = 311.5us from end of command EOF to start of tag response
#define DELAY_ISO15693_VICC_TO_VCD_READER 1017 // 1017/3.39MHz = 300us between end of tag response and next reader command

void CodeIso15693AsTag(uint8_t *cmd, size_t len);
int GetIso15693CommandFromReader(uint8_t *received, size_t max_len, uint32_t *eof_time);
void TransmitTo15693Reader(const uint8_t *cmd, size_t len, uint32_t *start_time, uint32_t slot_time, bool slow);
void SnoopIso15693(void);
void AcquireRawAdcSamplesIso15693(void);
void ReaderIso15693(uint32_t parameter);
void SimTagIso15693(uint32_t parameter, uint8_t *uid);
void BruteforceIso15693Afi(uint32_t speed);
void DirectTag15693Command(uint32_t datalen, uint32_t speed, uint32_t recv, uint8_t data[]);
void SetTag15693Uid(uint8_t *uid);
void SetDebugIso15693(uint32_t flag);

#endif
