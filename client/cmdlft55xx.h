//-----------------------------------------------------------------------------
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Low frequency T55xx commands
//-----------------------------------------------------------------------------

#ifndef CMDLFT55XX_H__
#define CMDLFT55XX_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint32_t bl1;
	uint32_t bl2; 
	uint32_t acl; 
	uint32_t mfc; 
	uint32_t cid; 
	uint32_t year; 
	uint32_t quarter; 
	uint32_t icr;
	uint32_t lotid; 
	uint32_t wafer; 
	uint32_t dw;
} t55x7_tracedata_t;

typedef struct {
	uint32_t bl1;
	uint32_t bl2;
	uint32_t icr;
	char lotidc;
	uint32_t lotid;
	uint32_t wafer;
	uint32_t dw;
} t5555_tracedata_t;

typedef struct {
	enum {
		DEMOD_NRZ  = 0x00,
		DEMOD_PSK1 = 0x01,
		DEMOD_PSK2 = 0x02,
		DEMOD_PSK3 = 0x03,
		DEMOD_FSK1  = 0x04,
		DEMOD_FSK1a = 0x05,
		DEMOD_FSK2  = 0x06,
		DEMOD_FSK2a = 0x07, 
		DEMOD_FSK   = 0xF0, //generic FSK (auto detect FCs)    
		DEMOD_ASK  = 0x08,
		DEMOD_BI   = 0x10,
		DEMOD_BIa  = 0x18
	}  modulation;
	bool inverted;
	uint8_t offset;
	uint32_t block0;
	enum {
		RF_8 = 0x00,
		RF_16 = 0x01,
		RF_32 = 0x02,
		RF_40 = 0x03,
		RF_50 = 0x04,
		RF_64 = 0x05,
		RF_100 = 0x06,
		RF_128 = 0x07
	} bitrate;
	bool Q5;
	bool ST;
} t55xx_conf_block_t;

t55xx_conf_block_t Get_t55xx_Config(void);
void Set_t55xx_Config(t55xx_conf_block_t conf);

extern int CmdLFT55XX(const char *Cmd);
extern int CmdT55xxBruteForce(const char *Cmd);
extern int CmdT55xxSetConfig(const char *Cmd);
extern int CmdT55xxReadBlock(const char *Cmd);
extern int CmdT55xxWriteBlock(const char *Cmd);
extern int CmdT55xxReadTrace(const char *Cmd);
extern int CmdT55xxInfo(const char *Cmd);
extern int CmdT55xxDetect(const char *Cmd);
extern int CmdResetRead(const char *Cmd);
extern int CmdT55xxWipe(const char *Cmd);

char * GetBitRateStr(uint32_t id, bool xmode);
char * GetSaferStr(uint32_t id);
char * GetModulationStr( uint32_t id);
char * GetModelStrFromCID(uint32_t cid);
char * GetSelectedModulationStr( uint8_t id);
uint32_t PackBits(uint8_t start, uint8_t len, uint8_t *bitstream);
void printT5xxHeader(uint8_t page);
void printT55xxBlock(const char *demodStr);
int printConfiguration( t55xx_conf_block_t b);

bool DecodeT55xxBlock(void);
bool tryDetectModulation(void);
extern bool tryDetectP1(bool getData);
bool test(uint8_t mode, uint8_t *offset, int *fndBitRate, uint8_t clk, bool *Q5);
int special(const char *Cmd);
int AquireData( uint8_t page, uint8_t block, bool pwdmode, uint32_t password );

void printT55x7Trace( t55x7_tracedata_t data, uint8_t repeat );
void printT5555Trace( t5555_tracedata_t data, uint8_t repeat );

#endif
