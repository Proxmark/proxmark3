//-----------------------------------------------------------------------------
// Jonathan Westhues, Aug 2005
// Gerhard de Koning Gans, April 2008, May 2011
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Definitions internal to the app source.
//-----------------------------------------------------------------------------

#ifndef __APPS_H
#define __APPS_H

#include <stdint.h>
#include <stddef.h>
#include "common.h"
#include "usb_cmd.h"
#include "hitagS.h"
#include "mifare.h"
#include "../common/crc32.h"
#include "BigBuf.h"

extern const uint8_t OddByteParity[256];
extern int rsamples;   // = 0;
extern uint8_t trigger;

/// appmain.h
void ReadMem(int addr);
void __attribute__((noreturn)) AppMain(void);
void SamyRun(void);
//void DbpIntegers(int a, int b, int c);
void DbpString(char *str);
void Dbprintf(const char *fmt, ...);
void Dbhexdump(int len, uint8_t *d, bool bAsci);

// ADC Vref = 3300mV, and an (10M+1M):1M voltage divider on the HF input can measure voltages up to 36300 mV
#define MAX_ADC_HF_VOLTAGE_LOW   36300
// ADC Vref = 3300mV, and an (10000k+240k):240k voltage divider on the LF input can measure voltages up to 140800 mV
#define MAX_ADC_HF_VOLTAGE_HIGH 140800
#define MAX_ADC_LF_VOLTAGE      140800
int AvgAdc(int ch);

void ToSendStuffBit(int b);
void ToSendReset(void);
void ListenReaderField(int limit);
extern int ToSendMax;
extern uint8_t ToSend[];


/// lfops.h
extern uint8_t decimation;
extern uint8_t bits_per_sample ;
extern bool averaging;

void AcquireRawAdcSamples125k(int divisor);
void ModThenAcquireRawAdcSamples125k(uint32_t delay_off, uint32_t period_0, uint32_t period_1, uint8_t *command);
void ReadTItag(void);
void WriteTItag(uint32_t idhi, uint32_t idlo, uint16_t crc);

void AcquireTiType(void);
void AcquireRawBitsTI(void);
void SimulateTagLowFrequency(int period, int gap, int ledcontrol);
void SimulateTagLowFrequencyBidir(int divisor, int max_bitlen);
void CmdHIDsimTAG(int hi2, int hi, int lo, int ledcontrol);
void CmdFSKsimTAG(uint16_t arg1, uint16_t arg2, size_t size, uint8_t *BitStream);
void CmdASKsimTag(uint16_t arg1, uint16_t arg2, size_t size, uint8_t *BitStream);
void CmdPSKsimTag(uint16_t arg1, uint16_t arg2, size_t size, uint8_t *BitStream);
void CmdHIDdemodFSK(int findone, int *high2, int *high, int *low, int ledcontrol);
void CmdAWIDdemodFSK(int findone, int *high, int *low, int ledcontrol); // Realtime demodulation mode for AWID26
void CmdEM410xdemod(int findone, int *high, int *low, int ledcontrol);
void CmdIOdemodFSK(int findone, int *high, int *low, int ledcontrol);
void CopyIOtoT55x7(uint32_t hi, uint32_t lo); // Clone an ioProx card to T5557/T5567
void CopyHIDtoT55x7(uint32_t hi2, uint32_t hi, uint32_t lo, uint8_t longFMT, uint8_t preamble); // Clone an HID-like card to T5557/T5567
void CopyVikingtoT55xx(uint32_t block1, uint32_t block2, uint8_t Q5);
void WriteEM410x(uint32_t card, uint32_t id_hi, uint32_t id_lo);
void CopyIndala64toT55x7(uint32_t hi, uint32_t lo); // Clone Indala 64-bit tag by UID to T55x7
void CopyIndala224toT55x7(uint32_t uid1, uint32_t uid2, uint32_t uid3, uint32_t uid4, uint32_t uid5, uint32_t uid6, uint32_t uid7); // Clone Indala 224-bit tag by UID to T55x7
void T55xxResetRead(void);
void T55xxWriteBlock(uint32_t Data, uint32_t Block, uint32_t Pwd, uint8_t PwdMode);
void T55xxReadBlock(uint16_t arg0, uint8_t Block, uint32_t Pwd);
void T55xxWakeUp(uint32_t Pwd);
void TurnReadLFOn();
//void T55xxReadTrace(void);
void EM4xReadWord(uint8_t Address, uint32_t Pwd, uint8_t PwdMode);
void EM4xWriteWord(uint32_t flag, uint32_t Data, uint32_t Pwd);
void EM4xProtect(uint32_t flag, uint32_t Data, uint32_t Pwd);
void Cotag(uint32_t arg0);

/// iso14443.h
void SimulateIso14443bTag(void);
void AcquireRawAdcSamplesIso14443b(uint32_t parameter);
void ReadSTMemoryIso14443b(uint32_t);
void RAMFUNC SnoopIso14443b(void);
void SendRawCommand14443B(uint32_t, uint32_t, uint8_t, uint8_t[]);

// Also used in iclass.c
void GetParity(const uint8_t *pbtCmd, uint16_t len, uint8_t *parity);

void RAMFUNC SniffMifare(uint8_t param);

/// epa.h
void EPA_PACE_Collect_Nonce(UsbCommand * c);
void EPA_PACE_Replay(UsbCommand *c);

// mifaredesfire.h
bool    InitDesfireCard();
void    MifareSendCommand(uint8_t arg0,uint8_t arg1, uint8_t *datain);
void    MifareDesfireGetInformation();
void    MifareDES_Auth1(uint8_t arg0,uint8_t arg1,uint8_t arg2, uint8_t *datain);
void    ReaderMifareDES(uint32_t param, uint32_t param2, uint8_t * datain);
int     DesfireAPDU(uint8_t *cmd, size_t cmd_len, uint8_t *dataout);
size_t  CreateAPDU( uint8_t *datain, size_t len, uint8_t *dataout);

// cmd.h
bool cmd_receive(UsbCommand* cmd);
bool cmd_send(uint32_t cmd, uint32_t arg0, uint32_t arg1, uint32_t arg2, void* data, size_t len);

#endif
