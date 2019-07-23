//-----------------------------------------------------------------------------
// Ultralight Code (c) 2013,2014 Midnitesnake & Andy Davies of Pentura
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency MIFARE ULTRALIGHT (C) commands
//-----------------------------------------------------------------------------

#include "cmdhfmfu.h"

#include <stdint.h>
#include <stdio.h>
#include "comms.h"
#include "usb_cmd.h"
#include "cmdmain.h"
#include "ui.h"
#include "mbedtls/des.h"
#include "cmdhfmf.h"
#include "cmdhf14a.h" // DropField()
#include "mifare.h"
#include "util.h"
#include "util_posix.h"
#include "protocols.h"
#include "taginfo.h"
#include "crypto/libpcrypto.h"

typedef enum TAGTYPE_UL {
	UNKNOWN       = 0x000000,
	UL            = 0x000001,
	UL_C          = 0x000002,
	UL_EV1_48     = 0x000004,
	UL_EV1_128    = 0x000008,
	NTAG          = 0x000010,
	NTAG_203      = 0x000020,
	NTAG_210      = 0x000040,
	NTAG_212      = 0x000080,
	NTAG_213      = 0x000100,
	NTAG_215      = 0x000200,
	NTAG_216      = 0x000400,
	MY_D          = 0x000800,
	MY_D_NFC      = 0x001000,
	MY_D_MOVE     = 0x002000,
	MY_D_MOVE_LEAN= 0x004000,
	NTAG_I2C_1K   = 0x008000,
	NTAG_I2C_2K   = 0x010000,
	FUDAN_UL      = 0x020000,
	MAGIC         = 0x040000,
	UL_MAGIC      = UL | MAGIC,
	UL_C_MAGIC    = UL_C | MAGIC,
	UL_ERROR      = 0xFFFFFF,
} TagTypeUL_t;

#define MAX_UL_BLOCKS      0x0f
#define MAX_ULC_BLOCKS     0x2b
#define MAX_ULEV1a_BLOCKS  0x13
#define MAX_ULEV1b_BLOCKS  0x28
#define MAX_NTAG_203       0x29
#define MAX_NTAG_210       0x13
#define MAX_NTAG_212       0x28
#define MAX_NTAG_213       0x2c
#define MAX_NTAG_215       0x86
#define MAX_NTAG_216       0xe6
#define MAX_MY_D_NFC       0xff
#define MAX_MY_D_MOVE      0x25
#define MAX_MY_D_MOVE_LEAN 0x0f

#define KEYS_3DES_COUNT 7
static uint8_t default_3des_keys[KEYS_3DES_COUNT][16] = {
	{ 0x42,0x52,0x45,0x41,0x4b,0x4d,0x45,0x49,0x46,0x59,0x4f,0x55,0x43,0x41,0x4e,0x21 },// 3des std key
	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },// all zeroes
	{ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f },// 0x00-0x0F
	{ 0x49,0x45,0x4D,0x4B,0x41,0x45,0x52,0x42,0x21,0x4E,0x41,0x43,0x55,0x4F,0x59,0x46 },// NFC-key
	{ 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01 },// all ones
	{ 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF },// all FF
	{ 0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF } // 11 22 33
};

#define KEYS_PWD_COUNT 6
static uint8_t default_pwd_pack[KEYS_PWD_COUNT][4] = {
	{0xFF,0xFF,0xFF,0xFF}, // PACK 0x00,0x00 -- factory default
	{0x4A,0xF8,0x4B,0x19}, // PACK 0xE5,0xBE -- italian bus (sniffed)
	{0x33,0x6B,0xA1,0x19}, // PACK 0x9c,0x2d -- italian bus (sniffed)
	{0xFF,0x90,0x6C,0xB2}, // PACK 0x12,0x9e -- italian bus (sniffed)
	{0x46,0x1c,0xA3,0x19}, // PACK 0xE9,0x5A -- italian bus (sniffed)
	{0x35,0x1C,0xD0,0x19}, // PACK 0x9A,0x5a -- italian bus (sniffed)
};

// known public keys for the originality check (source: https://github.com/alexbatalov/node-nxp-originality-verifier)
uint8_t public_keys[2][33] = {{0x04,0x49,0x4e,0x1a,0x38,0x6d,0x3d,0x3c,0xfe,0x3d,0xc1,0x0e,0x5d,0xe6,0x8a,0x49,0x9b,  // UL and NDEF
									0x1c,0x20,0x2d,0xb5,0xb1,0x32,0x39,0x3e,0x89,0xed,0x19,0xfe,0x5b,0xe8,0xbc,0x61},
							  {0x04,0x90,0x93,0x3b,0xdc,0xd6,0xe9,0x9b,0x4e,0x25,0x5e,0x3d,0xa5,0x53,0x89,0xa8,0x27,  // UL EV1
									0x56,0x4e,0x11,0x71,0x8e,0x01,0x72,0x92,0xfa,0xf2,0x32,0x26,0xa9,0x66,0x14,0xb8}
};

#define MAX_UL_TYPES 17
static uint32_t UL_TYPES_ARRAY[MAX_UL_TYPES] = {UNKNOWN, UL, UL_C, UL_EV1_48, UL_EV1_128, NTAG, NTAG_203,
		NTAG_210, NTAG_212, NTAG_213, NTAG_215, NTAG_216, MY_D, MY_D_NFC, MY_D_MOVE, MY_D_MOVE_LEAN, FUDAN_UL};

static uint8_t UL_MEMORY_ARRAY[MAX_UL_TYPES] = {MAX_UL_BLOCKS, MAX_UL_BLOCKS, MAX_ULC_BLOCKS, MAX_ULEV1a_BLOCKS,
		MAX_ULEV1b_BLOCKS, MAX_NTAG_203, MAX_NTAG_203, MAX_NTAG_210, MAX_NTAG_212, MAX_NTAG_213,
		MAX_NTAG_215, MAX_NTAG_216, MAX_UL_BLOCKS, MAX_MY_D_NFC, MAX_MY_D_MOVE, MAX_MY_D_MOVE_LEAN, MAX_UL_BLOCKS};

// get version nxp product type
static char *getProductTypeStr( uint8_t id){

	static char buf[20];
	char *retStr = buf;

	switch(id) {
		case 3: sprintf(retStr, "%02X, Ultralight", id); break;
		case 4: sprintf(retStr, "%02X, NTAG", id); break;
		default: sprintf(retStr, "%02X, unknown", id); break;
	}
	return buf;
}

/*
  The 7 MSBits (=n) code the storage size itself based on 2^n,
  the LSBit is set to '0' if the size is exactly 2^n
  and set to '1' if the storage size is between 2^n and 2^(n+1).
*/
static char *getUlev1CardSizeStr( uint8_t fsize ){

	static char buf[40];
	char *retStr = buf;
	memset(buf, 0, sizeof(buf));

	uint16_t usize = 1 << ((fsize >>1) + 1);
	uint16_t lsize = 1 << (fsize >>1);

	// is  LSB set?
	if (  fsize & 1 )
		sprintf(retStr, "%02X, (%u <-> %u bytes)",fsize, usize, lsize);
	else
		sprintf(retStr, "%02X, (%u bytes)", fsize, lsize);
	return buf;
}


static int ul_send_cmd_raw(uint8_t *cmd, uint8_t cmdlen, uint8_t *response, uint16_t responseLength) {
	UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_RAW | ISO14A_NO_DISCONNECT | ISO14A_APPEND_CRC, cmdlen, 0}};
	memcpy(c.d.asBytes, cmd, cmdlen);
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 1500)) return -1;
	if (!resp.arg[0] && responseLength) return -1;

	uint16_t resplen = (resp.arg[0] < responseLength) ? resp.arg[0] : responseLength;
	memcpy(response, resp.d.asBytes, resplen);
	return resplen;
}


static int ul_select(iso14a_card_select_t *card, bool clear_trace) {

	UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_CONNECT | ISO14A_NO_DISCONNECT | ISO14A_NO_RATS | (clear_trace?ISO14A_CLEAR_TRACE:0), 0, 0}};
	clearCommandBuffer();
	SendCommand(&c);

	UsbCommand resp;
	bool ans = false;
	ans = WaitForResponseTimeout(CMD_ACK, &resp, 1500);
	if (ans == 0 || resp.arg[0] == 0) {
		PrintAndLogEx(WARNING, "iso14443a card select failed");
		return 0;
	}

	memcpy(card, resp.d.asBytes, sizeof(iso14a_card_select_t));
	return 1;
}


// This read command will return 16 bytes.
static int ul_read(uint8_t page, uint8_t *response, uint16_t responseLength) {
	uint8_t cmd[] = {MIFARE_CMD_READBLOCK, page};
	int len = ul_send_cmd_raw(cmd, sizeof(cmd), response, responseLength);
	return len;
}


static int ul_halt(void) {
	uint8_t cmd[] = {ISO14443A_CMD_HALT, 0x00};
	uint8_t response;
	int len = ul_send_cmd_raw(cmd, sizeof(cmd), &response, sizeof(response));
	return len;
}


static int ul_comp_write_ex(uint8_t page, uint8_t *data, uint8_t datalen, bool first_part_only) {

	uint8_t cmd[18] = {0x00};
	datalen = ( datalen > 16) ? 16 : datalen;

	cmd[0] = MIFARE_CMD_WRITEBLOCK;
	cmd[1] = page;

	uint8_t response = {0xff};
	ul_send_cmd_raw(cmd, 2, &response, sizeof(response));
	if (response != CARD_ACK)
		return -1;
	if (first_part_only)
		return 0;

	memcpy(cmd, data, datalen);
	ul_send_cmd_raw(cmd, 16, &response, sizeof(response));
	if (response != CARD_ACK)
		return -1;

	return 0;
}


// not used yet
// static int ul_comp_write(uint8_t page, uint8_t *data, uint8_t datalen) {
	// return ul_comp_write_ex(page, data, datalen, false);
// }


static int ulc_requestAuthentication(uint8_t *nonce, uint16_t nonceLength) {

	uint8_t cmd[] = {MIFARE_ULC_AUTH_1, 0x00};
	int len = ul_send_cmd_raw(cmd, sizeof(cmd), nonce, nonceLength);
	return len;
}


static int ulc_authentication(uint8_t *key, bool switch_off_field) {

	UsbCommand c = {CMD_MIFAREUC_AUTH, {switch_off_field}};
	memcpy(c.d.asBytes, key, 16);
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if ( !WaitForResponseTimeout(CMD_ACK, &resp, 1500) ) return 0;
	if ( resp.arg[0] == 1 ) return 1;

	return 0;
}


static int ulev1_requestAuthentication(uint8_t *pwd, uint8_t *pack, uint16_t packLength) {

	uint8_t cmd[] = {MIFARE_ULEV1_AUTH, pwd[0], pwd[1], pwd[2], pwd[3]};
	int len = ul_send_cmd_raw(cmd, sizeof(cmd), pack, packLength);
	return len;
}


static int ul_auth_select(iso14a_card_select_t *card, TagTypeUL_t tagtype, bool hasAuthKey, uint8_t *authenticationkey, uint8_t *pack, uint8_t packSize) {

	if (hasAuthKey && (tagtype & UL_C)) {
		//will select card automatically and close connection on error
		if (!ulc_authentication(authenticationkey, false)) {
			PrintAndLogEx(ERR, "Authentication Failed UL-C");
			return 0;
		}
	} else {
		if (!ul_select(card, false)) return 0;

		if (hasAuthKey) {
			if (ulev1_requestAuthentication(authenticationkey, pack, packSize) < 1) {
				PrintAndLogEx(ERR, "Authentication Failed UL-EV1/NTAG");
				return 0;
			}
		}
	}
	return 1;
}

static int ulev1_getVersion(uint8_t *response, uint16_t responseLength) {

	uint8_t cmd[] = {MIFARE_ULEV1_VERSION};
	int len = ul_send_cmd_raw(cmd, sizeof(cmd), response, responseLength);
	return len;
}

// static int ulev1_fastRead( uint8_t startblock, uint8_t endblock, uint8_t *response ){

	// uint8_t cmd[] = {MIFARE_ULEV1_FASTREAD, startblock, endblock};

	// if ( !ul_send_cmd_raw(cmd, sizeof(cmd), response)){
		// return -1;
	// }
	// return 0;
// }


static int ulev1_readCounter(uint8_t counter, uint8_t *response, uint16_t responseLength) {

	uint8_t cmd[] = {MIFARE_ULEV1_READ_CNT, counter};
	int len = ul_send_cmd_raw(cmd, sizeof(cmd), response, responseLength);
	return len;
}


static int ulev1_readTearing(uint8_t counter, uint8_t *response, uint16_t responseLength) {

	uint8_t cmd[] = {MIFARE_ULEV1_CHECKTEAR, counter};
	int len = ul_send_cmd_raw(cmd, sizeof(cmd), response, responseLength);
	return len;
}


static int ulev1_readSignature(uint8_t *response, uint16_t responseLength) {

	uint8_t cmd[] = {MIFARE_ULEV1_READSIG, 0x00};
	int len = ul_send_cmd_raw(cmd, sizeof(cmd), response, responseLength);
	return len;
}


// Fudan check checks for which error is given for a command with incorrect crc
// NXP UL chip responds with 01, fudan 00.
// other possible checks:
//  send a0 + crc
//  UL responds with 00, fudan doesn't respond
//  or
//  send a200 + crc
//  UL doesn't respond, fudan responds with 00
//  or
//  send 300000 + crc (read with extra byte(s))
//  UL responds with read of page 0, fudan doesn't respond.
//
// make sure field is off before calling this function
static int ul_fudan_check(void) {
	iso14a_card_select_t card;
	if (!ul_select(&card, false))
		return UL_ERROR;

	UsbCommand c = {CMD_READER_ISO_14443a, {ISO14A_RAW | ISO14A_NO_DISCONNECT, 4, 0}};

	uint8_t cmd[4] = {0x30,0x00,0x02,0xa7}; //wrong crc on purpose  should be 0xa8
	memcpy(c.d.asBytes, cmd, 4);
	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 1500)) return UL_ERROR;
	if (resp.arg[0] != 1) return UL_ERROR;

	return (!resp.d.asBytes[0]) ? FUDAN_UL : UL; //if response == 0x00 then Fudan, else Genuine NXP
}


static int ul_print_default(uint8_t *data) {

	uint8_t uid[7];
	memcpy(uid, data, 3);
	memcpy(uid+3, data+4, 4);

	PrintAndLogEx(NORMAL,"       UID : %s", sprint_hex(uid, 7));
	PrintAndLogEx(NORMAL,"    UID[0] : %02X, %s", uid[0], getManufacturerName(uid[0]));

	if (uid[0] == 0x05 && ((uid[1] & 0xf0) >> 4) == 2 ) { // is infineon and 66RxxP
		uint8_t chip = (data[8] & 0xC7); // 11000111  mask, bit 3,4,5 RFU
		switch (chip){
			case 0xc2: PrintAndLogEx(NORMAL, "   IC type : SLE 66R04P 770 Bytes"); break; //77 pages
			case 0xc4: PrintAndLogEx(NORMAL, "   IC type : SLE 66R16P 2560 Bytes"); break; //256 pages
			case 0xc6: PrintAndLogEx(NORMAL, "   IC type : SLE 66R32P 5120 Bytes"); break; //512 pages /2 sectors
		}
	}
	// CT (cascade tag byte) 0x88 xor SN0 xor SN1 xor SN2
	int crc0 = 0x88 ^ uid[0] ^ uid[1] ^ uid[2];
	if ( data[3] == crc0 )
		PrintAndLogEx(NORMAL, "      BCC0 : %02X, Ok", data[3]);
	else
		PrintAndLogEx(NORMAL, "      BCC0 : %02X, crc should be %02X", data[3], crc0);

	int crc1 = uid[3] ^ uid[4] ^ uid[5] ^ uid[6];
	if ( data[8] == crc1 )
		PrintAndLogEx(NORMAL, "      BCC1 : %02X, Ok", data[8]);
	else
		PrintAndLogEx(NORMAL, "      BCC1 : %02X, crc should be %02X", data[8], crc1 );

	PrintAndLogEx(NORMAL, "  Internal : %02X, %sdefault", data[9], (data[9]==0x48)?"":"not " );

	PrintAndLogEx(NORMAL, "      Lock : %s       (binary %s %s)",
				sprint_hex(data+10, 2),
				printBits(1, data+10),
				printBits(1, data+11)
		);

	PrintAndLogEx(NORMAL, "OneTimePad : %s (binary %s %s %s %s)\n",
				sprint_hex(data+12, 4),
				printBits(1, data+12),
				printBits(1, data+13),
				printBits(1, data+14),
				printBits(1, data+15)
		);

	return 0;
}


static int ndef_print_CC(uint8_t *data) {
	// no NDEF message
	if(data[0] != 0xe1)
		return -1;

	PrintAndLogEx(NORMAL, "--- NDEF Message");
	PrintAndLogEx(NORMAL, "Capability Container: %s", sprint_hex(data,4) );
	PrintAndLogEx(NORMAL, "  %02X : NDEF Magic Number", data[0]);
	PrintAndLogEx(NORMAL, "  %02X : version %d.%d supported by tag", data[1], (data[1] & 0xF0) >> 4, data[1] & 0x0f);
	PrintAndLogEx(NORMAL, "  %02X : Physical Memory Size: %d bytes", data[2], (data[2] + 1) * 8);
	if ( data[2] == 0x12 )
		PrintAndLogEx(NORMAL, "  %02X : NDEF Memory Size: %d bytes", data[2], 144);
	else if ( data[2] == 0x3e )
		PrintAndLogEx(NORMAL, "  %02X : NDEF Memory Size: %d bytes", data[2], 496);
	else if ( data[2] == 0x6d )
		PrintAndLogEx(NORMAL, "  %02X : NDEF Memory Size: %d bytes", data[2], 872);

	PrintAndLogEx(NORMAL, "  %02X : %s / %s", data[3],
				(data[3] & 0xF0) ? "(RFU)" : "Read access granted without any security",
				(data[3] & 0x0F)==0 ? "Write access granted without any security" : (data[3] & 0x0F)==0x0F ? "No write access granted at all" : "(RFU)");
	return 0;
}


int ul_print_type(uint32_t tagtype, uint8_t spaces){
	char spc[11] = "          ";
	spc[10]=0x00;
	char *spacer = spc + (10-spaces);

	if (tagtype & UL )
		PrintAndLogEx(NORMAL, "%sTYPE : MIFARE Ultralight (MF0ICU1) %s", spacer, (tagtype & MAGIC) ? "<magic>" : "" );
	else if (tagtype & UL_C)
		PrintAndLogEx(NORMAL, "%sTYPE : MIFARE Ultralight C (MF0ULC) %s", spacer, (tagtype & MAGIC) ? "<magic>" : "" );
	else if (tagtype & UL_EV1_48)
		PrintAndLogEx(NORMAL, "%sTYPE : MIFARE Ultralight EV1 48bytes (MF0UL1101)", spacer);
	else if (tagtype & UL_EV1_128)
		PrintAndLogEx(NORMAL, "%sTYPE : MIFARE Ultralight EV1 128bytes (MF0UL2101)", spacer);
	else if (tagtype & NTAG)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG UNKNOWN", spacer);
	else if (tagtype & NTAG_203)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG 203 144bytes (NT2H0301F0DT)", spacer);
	else if (tagtype & NTAG_210)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG 210 48bytes (NT2L1011G0DU)", spacer);
	else if (tagtype & NTAG_212)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG 212 128bytes (NT2L1211G0DU)", spacer);
	else if (tagtype & NTAG_213)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG 213 144bytes (NT2H1311G0DU)", spacer);
	else if (tagtype & NTAG_215)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG 215 504bytes (NT2H1511G0DU)", spacer);
	else if (tagtype & NTAG_216)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG 216 888bytes (NT2H1611G0DU)", spacer);
	else if (tagtype & NTAG_I2C_1K)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG I%sC 888bytes (NT3H1101FHK)", spacer, "\xFD");
	else if (tagtype & NTAG_I2C_2K)
		PrintAndLogEx(NORMAL, "%sTYPE : NTAG I%sC 1904bytes (NT3H1201FHK)", spacer, "\xFD");
	else if (tagtype & MY_D)
		PrintAndLogEx(NORMAL, "%sTYPE : INFINEON my-d\x99 (SLE 66RxxS)", spacer);
	else if (tagtype & MY_D_NFC)
		PrintAndLogEx(NORMAL, "%sTYPE : INFINEON my-d\x99 NFC (SLE 66RxxP)", spacer);
	else if (tagtype & MY_D_MOVE)
		PrintAndLogEx(NORMAL, "%sTYPE : INFINEON my-d\x99 move | my-d\x99move NFC (SLE 66R01P)", spacer);
	else if (tagtype & MY_D_MOVE_LEAN)
		PrintAndLogEx(NORMAL, "%sTYPE : INFINEON my-d\x99 move lean (SLE 66R01L)", spacer);
	else if (tagtype & FUDAN_UL)
		PrintAndLogEx(NORMAL, "%sTYPE : FUDAN Ultralight Compatible (or other compatible) %s", spacer, (tagtype & MAGIC) ? "<magic>" : "" );
	else
		PrintAndLogEx(NORMAL, "%sTYPE : Unknown %06x", spacer, tagtype);
	return 0;
}


static int ulc_print_3deskey(uint8_t *data) {
	PrintAndLogEx(NORMAL, "         deskey1 [44/0x2C] : %s [%.4s]", sprint_hex(data   ,4),data);
	PrintAndLogEx(NORMAL, "         deskey1 [45/0x2D] : %s [%.4s]", sprint_hex(data+4 ,4),data+4);
	PrintAndLogEx(NORMAL, "         deskey2 [46/0x2E] : %s [%.4s]", sprint_hex(data+8 ,4),data+8);
	PrintAndLogEx(NORMAL, "         deskey2 [47/0x2F] : %s [%.4s]", sprint_hex(data+12,4),data+12);
	PrintAndLogEx(NORMAL, "\n 3des key : %s", sprint_hex(SwapEndian64(data, 16, 8), 16));
	return 0;
}


static int ulc_print_configuration(uint8_t *data) {

	PrintAndLogEx(NORMAL, "--- UL-C Configuration");
	PrintAndLogEx(NORMAL, " Higher Lockbits [40/0x28] : %s       (binary %s %s)",
		sprint_hex(data, 2),
		printBits(1, data),
		printBits(1, data+1)
		);
	PrintAndLogEx(NORMAL, "         Counter [41/0x29] : %s", sprint_hex(data+4, 2));

	bool validAuth = (data[8] >= 0x03 && data[8] <= 0x30);
	if (validAuth)
		PrintAndLogEx(NORMAL, "           Auth0 [42/0x2A] : %s page %d/0x%02X and above need authentication", sprint_hex(data+8, 4), data[8], data[8] );
	else{
		if (data[8] == 0) {
			PrintAndLogEx(NORMAL, "           Auth0 [42/0x2A] : %s default", sprint_hex(data+8, 4) );
		} else {
			PrintAndLogEx(NORMAL, "           Auth0 [42/0x2A] : %s auth byte is out-of-range", sprint_hex(data+8, 4) );
		}
	}
	PrintAndLogEx(NORMAL, "           Auth1 [43/0x2B] : %s %s",
			sprint_hex(data+12, 4),
			(data[12] & 1) ? "write access restricted": "read and write access restricted"
			);
	return 0;
}


static int ulev1_print_configuration(uint8_t *data, uint8_t startPage) {

	PrintAndLogEx(NORMAL, "\n--- Tag Configuration");

	bool strg_mod_en = (data[0] & 2);
	uint8_t authlim = (data[4] & 0x07);
	bool cfglck = (data[4] & 0x40);
	bool prot = (data[4] & 0x80);
	uint8_t vctid = data[5];

	PrintAndLogEx(NORMAL, "  cfg0 [%u/0x%02X] : %s", startPage, startPage, sprint_hex(data, 4));
	if (data[3] < 0xff)
		PrintAndLogEx(NORMAL, "                    - page %d and above need authentication", data[3]);
	else
		PrintAndLogEx(NORMAL, "                    - pages don't need authentication");
	PrintAndLogEx(NORMAL, "                    - strong modulation mode %s", (strg_mod_en) ? "enabled" : "disabled");
	PrintAndLogEx(NORMAL, "  cfg1 [%u/0x%02X] : %s", startPage + 1, startPage + 1,  sprint_hex(data+4, 4) );
	if (authlim == 0)
		PrintAndLogEx(NORMAL, "                    - Unlimited password attempts");
	else
		PrintAndLogEx(NORMAL, "                    - Max number of password attempts is %d", authlim);
	PrintAndLogEx(NORMAL, "                    - user configuration %s", cfglck ? "permanently locked":"writeable");
	PrintAndLogEx(NORMAL, "                    - %s access is protected with password", prot ? "read and write":"write");
	PrintAndLogEx(NORMAL, "                    - %02X, Virtual Card Type Identifier is %s default", vctid, (vctid==0x05)? "":"not");
	PrintAndLogEx(NORMAL, "  PWD  [%u/0x%02X] : %s- (cannot be read)", startPage + 2, startPage + 2, sprint_hex(data+8, 4));
	PrintAndLogEx(NORMAL, "  PACK [%u/0x%02X] : %s      - (cannot be read)", startPage + 3, startPage + 3, sprint_hex(data+12, 2));
	PrintAndLogEx(NORMAL, "  RFU  [%u/0x%02X] :       %s- (cannot be read)", startPage + 3, startPage + 3, sprint_hex(data+14, 2));
	return 0;
}


static int ulev1_print_counters(void) {
	PrintAndLogEx(NORMAL, "--- Tag Counters");
	uint8_t tear[1] = {0};
	uint8_t counter[3] = {0,0,0};
	uint16_t len = 0;
	for ( uint8_t i = 0; i<3; ++i) {
		ulev1_readTearing(i, tear, sizeof(tear));
		len = ulev1_readCounter(i, counter, sizeof(counter) );
		if (len == 3) {
			PrintAndLogEx(NORMAL, "       [%0d] : %s", i, sprint_hex(counter,3));
			PrintAndLogEx(NORMAL, "                    - %02X tearing %s", tear[0], ( tear[0]==0xBD)?"Ok":"failure");
		}
	}
	return len;
}


static int ulev1_print_signature(TagTypeUL_t tagtype, uint8_t *uid, uint8_t *signature, size_t signature_len){
	uint8_t public_key = 0;
	if (tagtype == UL_EV1_48 || tagtype == UL_EV1_128) {
		public_key = 1;
	}
	int res = ecdsa_signature_r_s_verify(MBEDTLS_ECP_DP_SECP128R1, public_keys[public_key], uid, 7, signature, signature_len, false);
	bool signature_valid = (res == 0);
	
	PrintAndLogEx(NORMAL, "\n--- Tag Originality Signature");
	//PrintAndLogEx(NORMAL, "IC signature public key name  : NXP NTAG21x 2013"); // don't know if there is other NXP public keys.. :(
	PrintAndLogEx(NORMAL, "         Signature public key : %s", sprint_hex(public_keys[public_key]+1, sizeof(public_keys[public_key])-1));
	PrintAndLogEx(NORMAL, "    Elliptic curve parameters : secp128r1");
	PrintAndLogEx(NORMAL, "            Tag ECC Signature : %s", sprint_hex(signature, signature_len));
	PrintAndLogEx(NORMAL, "  Originality signature check : signature is %svalid", signature_valid?"":"NOT ");
	return 0;
}


static int ulev1_print_version(uint8_t *data){
	PrintAndLogEx(NORMAL, "\n--- Tag Version");
	PrintAndLogEx(NORMAL, "       Raw bytes : %s", sprint_hex(data, 8) );
	PrintAndLogEx(NORMAL, "       Vendor ID : %02X, %s", data[1], getManufacturerName(data[1]));
	PrintAndLogEx(NORMAL, "    Product type : %s", getProductTypeStr(data[2]));
	PrintAndLogEx(NORMAL, " Product subtype : %02X, %s", data[3], (data[3]==1) ?"17 pF":"50pF");
	PrintAndLogEx(NORMAL, "   Major version : %02X", data[4]);
	PrintAndLogEx(NORMAL, "   Minor version : %02X", data[5]);
	PrintAndLogEx(NORMAL, "            Size : %s", getUlev1CardSizeStr(data[6]));
	PrintAndLogEx(NORMAL, "   Protocol type : %02X", data[7]);
	return 0;
}


static int ul_magic_test(void) {
	// try a compatibility write to page0, and see if tag answers with ACK/NACK to the first part of the command
	iso14a_card_select_t card;
	if (!ul_select(&card, false))
		return UL_ERROR;
	int status = ul_comp_write_ex(0, NULL, 0, true);
	if (status == 0) {
		return MAGIC;
	}
	return 0;
}


uint32_t GetHF14AMfU_Type(void){

	TagTypeUL_t tagtype = UNKNOWN;
	iso14a_card_select_t card;
	uint8_t version[10] = {0x00};
	int len;

	if (!ul_select(&card, true)) {
		DropField();
		msleep(200);
		return UL_ERROR;
	}

	// Check for Ultralight Family
	if (card.uidlen != 7 || (card.sak & 0x38) != 0x00) {
		DropField();
		PrintAndLogEx(NORMAL, "Tag is not Ultralight | NTAG | MY-D  [ATQA: %02X %02X SAK: %02X]\n", card.atqa[1], card.atqa[0], card.sak);
		return UL_ERROR;
	}

	if (card.uid[0] != 0x05) {
		len  = ulev1_getVersion(version, sizeof(version));
		if (len == 10) {
			if (version[2] == 0x03 && version[6] == 0x0B)
				tagtype = UL_EV1_48;
			else if (version[2] == 0x03 && version[6] != 0x0B)
				tagtype = UL_EV1_128;
			else if (version[2] == 0x04 && version[3] == 0x01 && version[6] == 0x0B)
				tagtype = NTAG_210;
			else if (version[2] == 0x04 && version[3] == 0x01 && version[6] == 0x0E)
				tagtype = NTAG_212;
			else if (version[2] == 0x04 && version[3] == 0x02 && version[6] == 0x0F)
				tagtype = NTAG_213;
			else if (version[2] == 0x04 && version[3] == 0x02 && version[6] == 0x11)
				tagtype = NTAG_215;
			else if (version[2] == 0x04 && version[3] == 0x02 && version[6] == 0x13)
				tagtype = NTAG_216;
			else if (version[2] == 0x04 && version[3] == 0x05 && version[6] == 0x13)
				tagtype = NTAG_I2C_1K;
			else if (version[2] == 0x04 && version[3] == 0x05 && version[6] == 0x15)
				tagtype = NTAG_I2C_2K;
			else if (version[2] == 0x04)
				tagtype = NTAG;
		}

		// UL vs UL-C vs ntag203 test
		if (tagtype == UNKNOWN) {
			ul_halt();
			if (!ul_select(&card, false)) {
				DropField();
				msleep(200);
				return UL_ERROR;
			}

			// do UL_C check first...
			uint8_t nonce[11] = {0x00};
			len = ulc_requestAuthentication(nonce, sizeof(nonce));
			ul_halt();
			if (len == 11) {
				tagtype = UL_C;
			} else {
				// need to re-select after authentication error
				if (!ul_select(&card, false)) {
					DropField();
					msleep(200);
					return UL_ERROR;
				}

				uint8_t data[16] = {0x00};
				// read page 0x29 (last valid ntag203 page)
				len = ul_read(0x29, data, sizeof(data));
				if (len <= 1) {
					tagtype = UL;
				} else {
					// read page 0x30 (should error if it is a ntag203)
					len = ul_read(0x30, data, sizeof(data));
					if (len <= 1) {
						ul_halt();
						tagtype = NTAG_203;
					}
				}
			}
		}
		if (tagtype & UL) {
			tagtype = ul_fudan_check();
			ul_halt();
		}

	} else {  // manufacturer Infineon. Check for my-d variants

		uint8_t nib = (card.uid[1] & 0xf0) >> 4;
		switch (nib) {
			case 1: tagtype = MY_D; break;                      //or SLE 66RxxS ... up to 512 pages of 8 user bytes...
			case 2: tagtype = MY_D_NFC; break;                  //or SLE 66RxxP ... up to 512 pages of 8 user bytes... (or in nfc mode FF pages of 4 bytes)
			case 3: tagtype = MY_D_MOVE; break;                 //or SLE 66R01P  // 38 pages of 4 bytes
			case 7: tagtype = MY_D_MOVE_LEAN; break;            //or SLE 66R01L  // 16 pages of 4 bytes
		}
	}

	tagtype |= ul_magic_test();
	
	if (tagtype == (UNKNOWN | MAGIC)) tagtype = (UL_MAGIC);

	DropField();
	msleep(200);
	
	printf("Tagtype: %08x\n", tagtype);
	return tagtype;
}


static int usage_hf_mfu_info(void) {
	PrintAndLogEx(NORMAL, "It gathers information about the tag and tries to detect what kind it is.");
	PrintAndLogEx(NORMAL, "Sometimes the tags are locked down, and you may need a key to be able to read the information");
	PrintAndLogEx(NORMAL, "The following tags can be identified:\n");
	PrintAndLogEx(NORMAL, "Ultralight, Ultralight-C, Ultralight EV1, NTAG 203, NTAG 210,");
	PrintAndLogEx(NORMAL, "NTAG 212, NTAG 213, NTAG 215, NTAG 216, NTAG I2C 1K & 2K");
	PrintAndLogEx(NORMAL, "my-d, my-d NFC, my-d move, my-d move NFC\n");
	PrintAndLogEx(NORMAL, "Usage:  hf mfu info k <key> l");
	PrintAndLogEx(NORMAL, "  Options : ");
	PrintAndLogEx(NORMAL, "  k <key> : (optional) key for authentication [UL-C 16bytes, EV1/NTAG 4bytes]");
	PrintAndLogEx(NORMAL, "  l       : (optional) swap entered key's endianness");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "   sample : hf mfu info");
	PrintAndLogEx(NORMAL, "          : hf mfu info k 00112233445566778899AABBCCDDEEFF");
	PrintAndLogEx(NORMAL, "          : hf mfu info k AABBCCDDD");
	return 0;
}


static int CmdHF14AMfUInfo(const char *Cmd) {

	uint8_t authlim = 0xff;
	iso14a_card_select_t card;
	uint8_t uid[7];
	bool errors = false;
	uint8_t keybytes[16] = {0x00};
	uint8_t *authenticationkey = keybytes;
	int keyLen = 0;
	bool hasAuthKey = false;
	bool locked = false;
	bool swapEndian = false;
	uint8_t cmdp = 0;
	uint8_t pack[4] = {0,0,0,0};
	int len = 0;

	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
		case 'H':
			return usage_hf_mfu_info();
		case 'k':
		case 'K':
			keyLen = 32;
			errors = param_gethex_ex(Cmd, cmdp+1, authenticationkey, &keyLen);
			if (errors || (keyLen != 32 && keyLen != 8)) { //ul-c or ev1/ntag key length
				PrintAndLogEx(ERR, "Key has incorrect length.\n");
				errors = true;
			}
			cmdp += 2;
			keyLen /= 2;
			hasAuthKey = true;
			break;
		case 'l':
		case 'L':
			swapEndian = true;
			cmdp++;
			break;
		default:
			PrintAndLogEx(WARNING, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if(errors) break;
	}

	//Validations
	if (errors)
		return usage_hf_mfu_info();

	TagTypeUL_t tagtype = GetHF14AMfU_Type();
	if (tagtype == UL_ERROR) {
		return -1;
	}

	PrintAndLogEx(NORMAL, "\n--- Tag Information ---------");
	PrintAndLogEx(NORMAL, "-------------------------------------------------------------");
	ul_print_type(tagtype, 6);

	// Swap endianness
	if (swapEndian && hasAuthKey) 
		authenticationkey = SwapEndian64(authenticationkey, keyLen, (keyLen == 16) ? 8 : 4 );

	if (!ul_auth_select(&card, tagtype, hasAuthKey, authenticationkey, pack, sizeof(pack))) {
		DropField();
		return -1;
	}

	// read pages 0,1,2,3 (should read 4pages)
	uint8_t data[16];
	len = ul_read(0, data, sizeof(data));
	if (len == -1) {
		DropField();
		PrintAndLogEx(WARNING, "Error: tag didn't answer to READ");
		return -1;
	} else if (len == 16) {
		memcpy(uid, data, 3);
		memcpy(uid+3, data+4, 4);
		ul_print_default(data);
		ndef_print_CC(data+12);
	} else {
		locked = true;
	}

	// UL_C Specific
	if ((tagtype & UL_C)) {

		// read pages 0x28, 0x29, 0x2A, 0x2B
		uint8_t ulc_conf[16] = {0x00};
		len = ul_read(0x28, ulc_conf, sizeof(ulc_conf));
		if (len == -1) {
			DropField();
			PrintAndLogEx(WARNING, "Error: tag didn't answer to READ UL-C");
			return -1;
		}
		if (len == 16) {
			ulc_print_configuration(ulc_conf);
		} else {
			locked = true;
		}

		if ((tagtype & MAGIC)) {
			//just read key
			uint8_t ulc_deskey[16] = {0x00};
			len = ul_read(0x2C, ulc_deskey, sizeof(ulc_deskey));
			if (len == -1) {
				DropField();
				PrintAndLogEx(WARNING, "Error: tag didn't answer to READ magic");
				return -1;
			}
			if (len == 16) ulc_print_3deskey(ulc_deskey);
		} else {
			// if we called info with key, just return
			if (hasAuthKey) {
				DropField();
				return 1;
			}

			// also try to diversify default keys..  look into CmdHF14AMfuGenDiverseKeys
			PrintAndLogEx(INFO, "Trying some default 3des keys");
			for (uint8_t i = 0; i < KEYS_3DES_COUNT; ++i ) {
				uint8_t *key = default_3des_keys[i];
				if (ulc_authentication(key, true)) {
					DropField();
					PrintAndLogEx(SUCCESS, "Found default 3des key: ");
					uint8_t keySwap[16];
					memcpy(keySwap, SwapEndian64(key,16,8), 16);
					ulc_print_3deskey(keySwap);
					return 1;
				}
			}
			DropField();
			return 1;
		}
	}

	// do counters and signature first (don't neet auth)

	// ul counters are different than ntag counters
	if ((tagtype & (UL_EV1_48 | UL_EV1_128))) {
		if (ulev1_print_counters() != 3) {
			// failed - re-select
			if (!ul_auth_select( &card, tagtype, hasAuthKey, authenticationkey, pack, sizeof(pack))) {
				DropField();
				return -1;
			}
		}
	}

	if ((tagtype & (UL_EV1_48 | UL_EV1_128 | NTAG_213 | NTAG_215 | NTAG_216 | NTAG_I2C_1K | NTAG_I2C_2K ))) {
		uint8_t ulev1_signature[32] = {0x00};
		len = ulev1_readSignature(ulev1_signature, sizeof(ulev1_signature));
		if (len == -1) {
			DropField();
			PrintAndLogEx(WARNING, "Error: tag didn't answer to READ SIGNATURE");
			return -1;
		}
		if (len == 32) {
			ulev1_print_signature(tagtype, uid, ulev1_signature, sizeof(ulev1_signature));
		} else {
			// re-select
			if (!ul_auth_select( &card, tagtype, hasAuthKey, authenticationkey, pack, sizeof(pack))) {
				DropField();
				return -1;
			}
		}
	}

	if ((tagtype & (UL_EV1_48 | UL_EV1_128 | NTAG_210 | NTAG_212 | NTAG_213 | NTAG_215 | NTAG_216 | NTAG_I2C_1K | NTAG_I2C_2K))) {
		uint8_t version[10] = {0x00};
		len = ulev1_getVersion(version, sizeof(version));
		if (len == -1) {
			DropField();
			PrintAndLogEx(WARNING, "Error: tag didn't answer to GETVERSION");
			return -1;
		} else if (len == 10) {
			ulev1_print_version(version);
		} else {
			locked = true;
			if (!ul_auth_select( &card, tagtype, hasAuthKey, authenticationkey, pack, sizeof(pack))) {
				DropField();
				return -1;
			}
		}

		uint8_t startconfigblock = 0;
		uint8_t ulev1_conf[16] = {0x00};
		// config blocks always are last 4 pages
		for (uint8_t idx = 0; idx < MAX_UL_TYPES; idx++) {
			if (tagtype & UL_TYPES_ARRAY[idx]) {
				startconfigblock = UL_MEMORY_ARRAY[idx]-3;
				break;
			}
		}

		if (startconfigblock) { // if we know where the config block is...
			len = ul_read(startconfigblock, ulev1_conf, sizeof(ulev1_conf));
			if (len == -1) {
				DropField();
				PrintAndLogEx(WARNING, "Error: tag didn't answer to READ EV1");
				return -1;
			} else if (len == 16) {
				// save AUTHENTICATION LIMITS for later:
				authlim = (ulev1_conf[4] & 0x07);
				ulev1_print_configuration(ulev1_conf, startconfigblock);
			}
		}

		// AUTHLIMIT, (number of failed authentications)
		// 0 = limitless.
		// 1-7 = limit. No automatic tries then.
		// hasAuthKey,  if we was called with key, skip test.
		if (!authlim && !hasAuthKey) {
			PrintAndLogEx(NORMAL, "\n--- Known EV1/NTAG passwords.");
			len = 0;
			for (uint8_t i = 0; i < KEYS_PWD_COUNT; ++i ) {
				uint8_t *key = default_pwd_pack[i];
				len = ulev1_requestAuthentication(key, pack, sizeof(pack));
				if (len >= 1) {
					PrintAndLogEx(SUCCESS, "Found a default password: %s || Pack: %02X %02X", sprint_hex(key, 4), pack[0], pack[1]);
					break;
				} else {
					if (!ul_auth_select( &card, tagtype, hasAuthKey, authenticationkey, pack, sizeof(pack))) {
						DropField();
						return -1;
					}
				}
			}
			if (len < 1) PrintAndLogEx(WARNING, "password not known");
		}
	}

	DropField();

	if (locked) 
		PrintAndLogEx(FAILED, "\nTag appears to be locked, try using the key to get more info");
	PrintAndLogEx(NORMAL, "");

	return 1;
}

//
//  Write Single Block
//
static int usage_hf_mfu_wrbl(void) {
	PrintAndLogEx(NORMAL, "Write a block. It autodetects card type.\n");
	PrintAndLogEx(NORMAL, "Usage:  hf mfu wrbl b <block number> d <data> k <key> l\n");
	PrintAndLogEx(NORMAL, "  Options:");
	PrintAndLogEx(NORMAL, "  b <no>   : block to write");
	PrintAndLogEx(NORMAL, "  d <data> : block data - (8 hex symbols)");
	PrintAndLogEx(NORMAL, "  k <key>  : (optional) key for authentication [UL-C 16bytes, EV1/NTAG 4bytes]");
	PrintAndLogEx(NORMAL, "  l        : (optional) swap entered key's endianness");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "    sample : hf mfu wrbl b 0 d 01234567");
	PrintAndLogEx(NORMAL, "           : hf mfu wrbl b 0 d 01234567 k AABBCCDDD\n");
	return 0;
}


static int CmdHF14AMfUWrBl(const char *Cmd){

	int blockNo = -1;
	bool errors = false;
	uint8_t keybytes[16] = {0x00};
	uint8_t *authenticationkey = keybytes;
	int keyLen = 0;
	bool hasAuthKey = false;
	bool swapEndian = false;
	uint8_t cmdp = 0;
	uint8_t blockdata[20] = {0x00};

	while(param_getchar(Cmd, cmdp) != 0x00) {
		switch(param_getchar(Cmd, cmdp)) {
			case 'h':
			case 'H':
				return usage_hf_mfu_wrbl();
			case 'k':
			case 'K':
				keyLen = 32;
				errors = param_gethex_ex(Cmd, cmdp+1, authenticationkey, &keyLen);
				if (errors || (keyLen != 32 && keyLen != 8)) { //ul-c or ev1/ntag key length
					PrintAndLogEx(ERR, "Key has incorrect length.\n");
					errors = true;
				}
				cmdp += 2;
				keyLen /= 2;
				hasAuthKey = true;
				break;
			case 'b':
			case 'B':
				blockNo = param_get8(Cmd, cmdp+1);
				if (blockNo < 0) {
					PrintAndLogEx(ERR, "Wrong block number");
					errors = true;
				}
				cmdp += 2;
				break;
			case 'l':
			case 'L':
				swapEndian = true;
				cmdp++;
				break;
			case 'd':
			case 'D':
				if ( param_gethex(Cmd, cmdp+1, blockdata, 8) ) {
					PrintAndLogEx(ERR, "Block data must include 8 HEX symbols");
					errors = true;
					break;
				}
				cmdp += 2;
				break;
			default:
				PrintAndLogEx(ERR, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
				errors = true;
				break;
		}
		//Validations
		if(errors) return usage_hf_mfu_wrbl();
	}

	if (blockNo == -1) return usage_hf_mfu_wrbl();
	// starting with getting tagtype
	TagTypeUL_t tagtype = GetHF14AMfU_Type();
	if (tagtype == UL_ERROR) {
		return -1;
	}

	uint8_t maxblockno = 0;
	for (uint8_t idx = 0; idx < MAX_UL_TYPES; idx++) {
		if (tagtype & UL_TYPES_ARRAY[idx]) {
			maxblockno = UL_MEMORY_ARRAY[idx];
			break;
		}
	}
	if (blockNo > maxblockno){
		DropField();
		PrintAndLogEx(WARNING, "block number too large. Max block is %u/0x%02X \n", maxblockno,maxblockno);
		return usage_hf_mfu_wrbl();
	}

	// Swap endianness
	if (swapEndian && hasAuthKey) authenticationkey = SwapEndian64(authenticationkey, keyLen, (keyLen == 16) ? 8 : 4);

	if ( blockNo <= 3)
		PrintAndLogEx(NORMAL, "Special Block: %0d (0x%02X) [ %s]", blockNo, blockNo, sprint_hex(blockdata, 4));
	else
		PrintAndLogEx(NORMAL, "Block: %0d (0x%02X) [ %s]", blockNo, blockNo, sprint_hex(blockdata, 4));

	//Send write Block
	UsbCommand c = {CMD_MIFAREU_WRITEBL, {blockNo}};
	memcpy(c.d.asBytes, blockdata, 4);

	if (hasAuthKey) {
		c.arg[1] = (keyLen == 16) ? 1 : 2;
		memcpy(c.d.asBytes+4, authenticationkey, keyLen);
	}

	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (WaitForResponseTimeout(CMD_ACK, &resp, 1500)) {
		uint8_t isOK  = resp.arg[0] & 0xff;
		PrintAndLogEx(SUCCESS, "isOk:%02x", isOK);
	} else {
		PrintAndLogEx(ERR, "Command execute timeout");
	}

	DropField();
	return 0;
}


//
//  Read Single Block
//
static int usage_hf_mfu_rdbl(void) {
	PrintAndLogEx(NORMAL, "Read a block and print. It autodetects card type.\n");
	PrintAndLogEx(NORMAL, "Usage:  hf mfu rdbl b <block number> k <key> l\n");
	PrintAndLogEx(NORMAL, "  Options:");
	PrintAndLogEx(NORMAL, "  b <no>  : block to read");
	PrintAndLogEx(NORMAL, "  k <key> : (optional) key for authentication [UL-C 16bytes, EV1/NTAG 4bytes]");
	PrintAndLogEx(NORMAL, "  l       : (optional) swap entered key's endianness");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "   sample : hf mfu rdbl b 0");
	PrintAndLogEx(NORMAL, "          : hf mfu rdbl b 0 k 00112233445566778899AABBCCDDEEFF");
	PrintAndLogEx(NORMAL, "          : hf mfu rdbl b 0 k AABBCCDDD\n");
	return 0;
}


static int CmdHF14AMfURdBl(const char *Cmd){

	int blockNo = -1;
	bool errors = false;
	uint8_t keybytes[16] = {0x00};
	uint8_t *authenticationkey = keybytes;
	int keyLen = 0;
	bool hasAuthKey = false;
	bool swapEndian = false;
	uint8_t cmdp = 0;

	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
			case 'h':
			case 'H':
				return usage_hf_mfu_rdbl();
			case 'k':
			case 'K':
				keyLen = 32;
				errors = param_gethex_ex(Cmd, cmdp+1, authenticationkey, &keyLen);
				if (errors || (keyLen != 32 && keyLen != 8)) { //ul-c or ev1/ntag key length
					PrintAndLogEx(ERR, "Key has incorrect length.\n");
					errors = true;
				}
				cmdp += 2;
				keyLen /= 2;
				hasAuthKey = true;
				break;
			case 'b':
			case 'B':
				blockNo = param_get8(Cmd, cmdp+1);
				if (blockNo < 0) {
					PrintAndLogEx(ERR, "Wrong block number");
					errors = true;
				}
				cmdp += 2;
				break;
			case 'l':
			case 'L':
				swapEndian = true;
				cmdp++;
				break;
			default:
				PrintAndLogEx(WARNING, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
				errors = true;
				break;
		}
		//Validations
		if (errors) return usage_hf_mfu_rdbl();
	}

	if (blockNo == -1) return usage_hf_mfu_rdbl();
	// start with getting tagtype
	TagTypeUL_t tagtype = GetHF14AMfU_Type();
	if (tagtype == UL_ERROR) {
		return -1;
	}

	uint8_t maxblockno = 0;
	for (uint8_t idx = 0; idx < MAX_UL_TYPES; idx++) {
		if (tagtype & UL_TYPES_ARRAY[idx]) {
			maxblockno = UL_MEMORY_ARRAY[idx];
			break;
		}
	}
	if (blockNo > maxblockno){
		DropField();
		PrintAndLogEx(WARNING, "block number to large. Max block is %u/0x%02X \n", maxblockno,maxblockno);
		return usage_hf_mfu_rdbl();
	}

	// Swap endianness
	if (swapEndian) authenticationkey = SwapEndian64(authenticationkey, keyLen, (keyLen == 16) ? 8 : 4);

	//Read Block
	UsbCommand c = {CMD_MIFAREU_READBL, {blockNo}};
	if (hasAuthKey) {
		c.arg[1] = (keyLen == 16) ? 1 : 2;
		memcpy(c.d.asBytes, authenticationkey, keyLen);
	}

	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (WaitForResponseTimeout(CMD_ACK, &resp, 1500)) {
		uint8_t isOK = resp.arg[0] & 0xff;
		if (isOK) {
			uint8_t *data = resp.d.asBytes;
			PrintAndLogEx(NORMAL, "\n Block#  | Data        | Ascii");
			PrintAndLogEx(NORMAL, "---------+-------------+------");
			PrintAndLogEx(NORMAL, " %02d/0x%02X | %s| %s\n", blockNo, blockNo, sprint_hex(data, 4), sprint_ascii(data, 4));
		} else {
			PrintAndLogEx(ERR, "Failed reading block: (%02x)", isOK);
		}
	} else {
		PrintAndLogEx(ERR, "Command execute time-out");
	}
	DropField();
	return 0;
}


//
//  Mifare Ultralight / Ultralight-C / Ultralight-EV1
//  Read and Dump Card Contents,  using auto detection of tag size.

typedef struct {
	uint8_t version[8];
	uint8_t tbo[2];
	uint8_t tbo1[1];
	uint8_t pages;                  // max page number in dump
	uint8_t signature[32];
	uint8_t counter_tearing[3][4];  // 3 bytes counter, 1 byte tearing flag
	uint8_t data[1024];
} mfu_dump_t;


static void printMFUdumpEx(mfu_dump_t *card, uint16_t pages, uint8_t startpage, TagTypeUL_t tagtype) {

	bool tmplockbit = false;
	bool bit[16]  = {false};
	bool bit2[16] = {false};

	// standard lock bits
	for(int i = 0; i < 16; i++){
		bit[i] = card->data[10+i/8] & (1 << (7-i%8));
	}

	// dynamic lock bits
	// TODO -- FIGURE OUT LOCK BYTES FOR EV1 and/or NTAG
	if (tagtype & UL_C) {
		for (int i = 0; i < 16; i++) {
			bit2[i] = card->data[40*4+i/8] & (1 << (7-i%8));
		}
	}

	PrintAndLogEx(NORMAL, "\n  Block# | Data        |lck| Ascii");
	PrintAndLogEx(NORMAL, "---------+-------------+---+------");

	for (int i = startpage; i < startpage + pages; i++) {
		if (i < 3) {
			PrintAndLogEx(NORMAL, "%3d/0x%02X | %s|   | ", i, i, sprint_hex(card->data + i * 4, 4));
			continue;
		}
		switch(i){
			case 3: tmplockbit = bit[4]; break;
			case 4: tmplockbit = bit[3]; break;
			case 5: tmplockbit = bit[2]; break;
			case 6: tmplockbit = bit[1]; break;
			case 7: tmplockbit = bit[0]; break;
			case 8: tmplockbit = bit[15]; break;
			case 9: tmplockbit = bit[14]; break;
			case 10: tmplockbit = bit[13]; break;
			case 11: tmplockbit = bit[12]; break;
			case 12: tmplockbit = bit[11]; break;
			case 13: tmplockbit = bit[10]; break;
			case 14: tmplockbit = bit[9]; break;
			case 15: tmplockbit = bit[8]; break;
			case 16:
			case 17:
			case 18:
			case 19: tmplockbit = bit2[6]; break;
			case 20:
			case 21:
			case 22:
			case 23: tmplockbit = bit2[5]; break;
			case 24:
			case 25:
			case 26:
			case 27: tmplockbit = bit2[4]; break;
			case 28:
			case 29:
			case 30:
			case 31: tmplockbit = bit2[2]; break;
			case 32:
			case 33:
			case 34:
			case 35: tmplockbit = bit2[1]; break;
			case 36:
			case 37:
			case 38:
			case 39: tmplockbit = bit2[0]; break;
			case 40: tmplockbit = bit2[12]; break;
			case 41: tmplockbit = bit2[11]; break;
			case 42: tmplockbit = bit2[10]; break; //auth0
			case 43: tmplockbit = bit2[9]; break;  //auth1
			default: break;
		}

		PrintAndLogEx(NORMAL, "%3d/0x%02X | %s| %d | %.4s", i, i, sprint_hex(card->data + i * 4, 4), tmplockbit, sprint_ascii(card->data + i * 4, 4));
	}
	PrintAndLogEx(NORMAL, "---------------------------------");
}


static int usage_hf_mfu_dump(void) {
	PrintAndLogEx(NORMAL, "Reads all pages from Ultralight, Ultralight-C, Ultralight EV1");
	PrintAndLogEx(NORMAL, "NTAG 203, NTAG 210, NTAG 212, NTAG 213, NTAG 215, NTAG 216");
	PrintAndLogEx(NORMAL, "and saves binary dump into the file `filename.bin` or `cardUID.bin`");
	PrintAndLogEx(NORMAL, "It autodetects card type.\n");
	PrintAndLogEx(NORMAL, "Usage:  hf mfu dump k <key> l n <filename w/o .bin>");
	PrintAndLogEx(NORMAL, "  Options : ");
	PrintAndLogEx(NORMAL, "  k <key> : (optional) key for authentication [UL-C 16bytes, EV1/NTAG 4bytes]");
	PrintAndLogEx(NORMAL, "  l       : (optional) swap entered key's endianness");
	PrintAndLogEx(NORMAL, "  f <FN > : filename w/o .bin to save the dump as");
	PrintAndLogEx(NORMAL, "  p <Pg > : starting Page number to manually set a page to start the dump at");
	PrintAndLogEx(NORMAL, "  q <qty> : number of Pages to manually set how many pages to dump");

	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "   sample : hf mfu dump");
	PrintAndLogEx(NORMAL, "          : hf mfu dump n myfile");
	PrintAndLogEx(NORMAL, "          : hf mfu dump k 00112233445566778899AABBCCDDEEFF");
	PrintAndLogEx(NORMAL, "          : hf mfu dump k AABBCCDDD\n");
	return 0;
}


static int CmdHF14AMfUDump(const char *Cmd){

	char filename[FILE_PATH_SIZE] = {'\0'};
	size_t fileNameLen = 0;
	uint8_t keybytes[16] = {0x00};
	uint8_t *authenticationkey = keybytes;
	int keyLen = 0;
	bool hasAuthKey = false;
	uint8_t cmdp = 0;
	bool errors = false;
	bool swapEndian = false;
	bool manualPages = false;
	uint8_t startPage = 0;
	int Pages = 16;
	iso14a_card_select_t card_select;
	mfu_dump_t card;

	while(param_getchar(Cmd, cmdp) != 0x00)
	{
		switch(param_getchar(Cmd, cmdp))
		{
		case 'h':
		case 'H':
			return usage_hf_mfu_dump();
		case 'k':
		case 'K':
			keyLen = 32;
			errors = param_gethex_ex(Cmd, cmdp+1, authenticationkey, &keyLen);
			if (errors || (keyLen != 32 && keyLen != 8)) { //ul-c or ev1/ntag key length
				PrintAndLogEx(ERR, "Key has incorrect length.\n");
				errors = true;
			}
			cmdp += 2;
			keyLen /= 2;
			hasAuthKey = true;
			break;
		case 'l':
		case 'L':
			swapEndian = true;
			cmdp++;
			break;
		case 'f':
		case 'F':
			fileNameLen = param_getstr(Cmd, cmdp+1, filename, sizeof(filename));
			if (fileNameLen == 0) errors = true;
			if (fileNameLen > FILE_PATH_SIZE-5) fileNameLen = FILE_PATH_SIZE-5;
			cmdp += 2;
			break;
		case 'p':
		case 'P':
			startPage = param_get8(Cmd, cmdp+1);
			manualPages = true;
			cmdp += 2;
			break;
		case 'q':
		case 'Q':
			Pages = param_get8(Cmd, cmdp+1);
			cmdp += 2;
			manualPages = true;
			break;
		default:
			PrintAndLogEx(ERR, "Unknown parameter '%c'", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if (errors) break;
	}

	//Validations
	if (errors) return usage_hf_mfu_dump();

	if (swapEndian && hasAuthKey)
		authenticationkey = SwapEndian64(authenticationkey, keyLen, (keyLen == 16) ? 8 : 4);

	TagTypeUL_t tagtype = GetHF14AMfU_Type();

	if (tagtype == UL_ERROR) {
		return -1;
	}

	uint8_t maxPages = 0;
	for (uint8_t idx = 0; idx < MAX_UL_TYPES; idx++) {
		if (tagtype & UL_TYPES_ARRAY[idx]) {
			maxPages = UL_MEMORY_ARRAY[idx]+1; //add one as maxblks starts at 0
			break;
		}
	}

	if (!manualPages) {
		Pages = maxPages;
	} else {
		if (startPage + Pages - 1 > maxPages - 1) {
			PrintAndLogEx(ERR, "Invalid page range. Card has only %d readable pages.", maxPages);
			DropField();
			return 1;
		}
	}

	ul_print_type(tagtype, 0);

	PrintAndLogEx(NORMAL, "Reading tag memory...");
	memset(&card, 0x00, sizeof(card));
	UsbCommand c = {CMD_MIFAREU_READCARD, {startPage, Pages}};
	if (hasAuthKey) {
		if (tagtype & UL_C)
			c.arg[2] = 1; //UL_C auth
		else
			c.arg[2] = 2; //UL_EV1/NTAG auth
		memcpy(c.d.asBytes, authenticationkey, keyLen);
	}

	clearCommandBuffer();
	SendCommand(&c);
	UsbCommand resp;
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 1500)) {
		PrintAndLogEx(ERR, "Command execution timeout");
		DropField();
		return 1;
	}
	if (resp.arg[0] != 1) {
		PrintAndLogEx(ERR, "Failed reading card");
		DropField();
		return 1;
	}

	uint32_t startindex = resp.arg[2];
	uint32_t bufferSize = resp.arg[1];
	if (bufferSize > sizeof(card.data)) {
		PrintAndLogEx(FAILED, "Data exceeded Buffer size!");
		bufferSize = sizeof(card.data);
	}

	if (!GetFromBigBuf(card.data + startPage*4, bufferSize, startindex, NULL, -1, false)) {
		PrintAndLogEx(ERR, "Command execution timeout");
		DropField();
		return 1;
	}

	// not ul_c and not std ul then attempt to collect
	//  VERSION, SIGNATURE, COUNTERS, TEARING, PACK
	if (!(tagtype & UL_C || tagtype & UL)) {
		//attempt to read pack
		if (!ul_auth_select(&card_select, tagtype, true, authenticationkey, card.data + maxPages*4 - 4, 2)) {
			//reset pack
			card.data[maxPages*4 - 4] = 0;
			card.data[maxPages*4 - 3] = 0;
		}

		if (hasAuthKey) {
			uint8_t dummy_pack[2];
			ul_auth_select(&card_select, tagtype, hasAuthKey, authenticationkey, dummy_pack, sizeof(dummy_pack));
		} else {
			ul_select(&card_select, false);
		}
		ulev1_getVersion(card.version, sizeof(card.version));
		for (uint8_t n = 0; n < 3; ++n) {
			ulev1_readTearing(n, &card.counter_tearing[n][3], 1);
			ulev1_readCounter(n, &card.counter_tearing[n][0], 3);
		}

		ulev1_readSignature(card.signature, sizeof(card.signature));
	}

	DropField();
	
	// add key to dump data
	if (hasAuthKey) {
		authenticationkey = SwapEndian64(authenticationkey, keyLen, (keyLen == 16) ? 8 : 4);
		if (tagtype & UL_C){ // additional 4 pages
			memcpy(card.data + maxPages*4, authenticationkey, keyLen);
			maxPages += 4;
		} else { // 2nd page from end
			memcpy(card.data + (maxPages*4) - 8, authenticationkey, 4);
		}
	}

	printMFUdumpEx(&card, Pages, startPage, tagtype);

	if (!manualPages) {
		// user supplied filename?
		if (fileNameLen < 1) {
			char *fptr = filename;
			fptr += sprintf(fptr, "hf-mfu-");
			uint8_t UID[] = {card.data[0], card.data[1], card.data[2], card.data[4], card.data[5], card.data[6], card.data[7]};
			FillFileNameByUID(fptr, UID, "-dump.bin", 7);
		} else {
			sprintf(filename + fileNameLen, ".bin");
		}

#define MFU_DUMP_PREFIX_LENGTH (sizeof(card) - sizeof(card.data))

		FILE *fout;
		if ((fout = fopen(filename, "wb")) == NULL) {
			PrintAndLogEx(ERR, "Could not create file name %s", filename);
			return 1;
		}
		fwrite(&card, 1, MFU_DUMP_PREFIX_LENGTH + maxPages*4, fout);
		fclose(fout);

		PrintAndLogEx(SUCCESS, "Dumped %d pages, wrote %d bytes to %s", maxPages, MFU_DUMP_PREFIX_LENGTH + maxPages*4, filename);
	}

	return 0;
}


//-------------------------------------------------------------------------------
// Ultralight C Methods
//-------------------------------------------------------------------------------

//
// Ultralight C Authentication Demo {currently uses hard-coded key}
//
static int CmdHF14AMfucAuth(const char *Cmd){

	uint8_t keyNo = 3;
	bool errors = false;

	char cmdp = param_getchar(Cmd, 0);

	//Change key to user defined one
	if (cmdp == 'k' || cmdp == 'K'){
		keyNo = param_get8(Cmd, 1);
		if(keyNo > KEYS_3DES_COUNT-1)
			errors = true;
	}

	if (cmdp == 'h' || cmdp == 'H')
		errors = true;

	if (errors) {
		PrintAndLogEx(NORMAL, "Usage:  hf mfu cauth k <key number>");
		PrintAndLogEx(NORMAL, "      0 (default): 3DES standard key");
		PrintAndLogEx(NORMAL, "      1 : all 0x00 key");
		PrintAndLogEx(NORMAL, "      2 : 0x00-0x0F key");
		PrintAndLogEx(NORMAL, "      3 : nfc key");
		PrintAndLogEx(NORMAL, "      4 : all 0x01 key");
		PrintAndLogEx(NORMAL, "      5 : all 0xff key");
		PrintAndLogEx(NORMAL, "      6 : 0x00-0xFF key");
		PrintAndLogEx(NORMAL, "\n      sample : hf mfu cauth k");
		PrintAndLogEx(NORMAL, "               : hf mfu cauth k 3");
		return 0;
	}

	uint8_t *key = default_3des_keys[keyNo];
	if (ulc_authentication(key, true)) {
		DropField();
		PrintAndLogEx(SUCCESS, "Authentication successful. 3des key: %s",sprint_hex(key, 16));
	} else {
		DropField();
		PrintAndLogEx(WARNING, "Authentication failed");
	}
	return 0;
}


//
// Mifare Ultralight C - Set password
//
static int CmdHF14AMfucSetPwd(const char *Cmd){

	uint8_t pwd[16] = {0x00};

	char cmdp = param_getchar(Cmd, 0);

	if (strlen(Cmd) == 0  || cmdp == 'h' || cmdp == 'H') {
		PrintAndLogEx(NORMAL, "Usage:  hf mfu setpwd <password (32 hex symbols)>");
		PrintAndLogEx(NORMAL, "       [password] - (32 hex symbols)");
		PrintAndLogEx(NORMAL, "");
		PrintAndLogEx(NORMAL, "sample: hf mfu setpwd 000102030405060708090a0b0c0d0e0f");
		PrintAndLogEx(NORMAL, "");
		return 0;
	}

	if (param_gethex(Cmd, 0, pwd, 32)) {
		PrintAndLogEx(WARNING, "Password must include 32 HEX symbols");
		return 1;
	}

	UsbCommand c = {CMD_MIFAREUC_SETPWD};
	memcpy( c.d.asBytes, pwd, 16);
	clearCommandBuffer();
	SendCommand(&c);

	UsbCommand resp;

	if (WaitForResponseTimeout(CMD_ACK, &resp, 1500) ) {
		DropField();
		if ((resp.arg[0] & 0xff) == 1) {
			PrintAndLogEx(INFO, "Ultralight-C new password: %s", sprint_hex(pwd,16));
			return 0;
		} else {
			PrintAndLogEx(ERR, "Failed writing at block %d", resp.arg[1] & 0xff);
			return 1;
		}
	} else {
		DropField();
		PrintAndLogEx(ERR, "command execution timeout");
		return 1;
	}

	return 0;
}

//
// Magic UL / UL-C tags  - Set UID
//
static int CmdHF14AMfucSetUid(const char *Cmd){

	UsbCommand c;
	UsbCommand resp;
	uint8_t uid[7] = {0x00};
	char cmdp = param_getchar(Cmd, 0);

	if (strlen(Cmd) == 0  || cmdp == 'h' || cmdp == 'H') {
		PrintAndLogEx(NORMAL, "Usage:  hf mfu setuid <uid (14 hex symbols)>");
		PrintAndLogEx(NORMAL, "       [uid] - (14 hex symbols)");
		PrintAndLogEx(NORMAL, "\nThis only works for Magic Ultralight tags.");
		PrintAndLogEx(NORMAL, "");
		PrintAndLogEx(NORMAL, "sample: hf mfu setuid 11223344556677");
		PrintAndLogEx(NORMAL, "");
		return 0;
	}

	if (param_gethex(Cmd, 0, uid, 14)) {
		PrintAndLogEx(WARNING, "UID must include 14 HEX symbols");
		return 1;
	}

	// read block2.
	c.cmd = CMD_MIFAREU_READBL;
	c.arg[0] = 2;
	clearCommandBuffer();
	SendCommand(&c);
	if (!WaitForResponseTimeout(CMD_ACK,&resp,1500)) {
		DropField();
		PrintAndLogEx(WARNING, "Command execute timeout");
		return 2;
	}

	// save old block2.
	uint8_t oldblock2[4] = {0x00};
	memcpy(resp.d.asBytes, oldblock2, 4);

	// block 0.
	c.cmd = CMD_MIFAREU_WRITEBL;
	c.arg[0] = 0;
	c.d.asBytes[0] = uid[0];
	c.d.asBytes[1] = uid[1];
	c.d.asBytes[2] = uid[2];
	c.d.asBytes[3] =  0x88 ^ uid[0] ^ uid[1] ^ uid[2];
	clearCommandBuffer();
	SendCommand(&c);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 1500)) {
		DropField();
		PrintAndLogEx(WARNING, "Command execute timeout");
		return 3;
	}

	// block 1.
	c.arg[0] = 1;
	c.d.asBytes[0] = uid[3];
	c.d.asBytes[1] = uid[4];
	c.d.asBytes[2] = uid[5];
	c.d.asBytes[3] = uid[6];
	clearCommandBuffer();
	SendCommand(&c);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 1500) ) {
		DropField();
		PrintAndLogEx(WARNING, "Command execute timeout");
		return 4;
	}

	// block 2.
	c.arg[0] = 2;
	c.d.asBytes[0] = uid[3] ^ uid[4] ^ uid[5] ^ uid[6];
	c.d.asBytes[1] = oldblock2[1];
	c.d.asBytes[2] = oldblock2[2];
	c.d.asBytes[3] = oldblock2[3];
	clearCommandBuffer();
	SendCommand(&c);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 1500) ) {
		DropField();
		PrintAndLogEx(WARNING, "Command execute timeout");
		return 5;
	}

	DropField();
	return 0;
}


static int CmdHF14AMfuGenDiverseKeys(const char *Cmd){

	uint8_t iv[8] = { 0x00 };
	uint8_t block = 0x07;

	// UL-EV1
	//04 57 b6 e2 05 3f 80 UID
	//4a f8 4b 19   PWD
	uint8_t uid[] = { 0xF4,0xEA, 0x54, 0x8E };
	uint8_t mifarekeyA[] = { 0xA0,0xA1,0xA2,0xA3,0xA4,0xA5 };
	uint8_t mifarekeyB[] = { 0xB0,0xB1,0xB2,0xB3,0xB4,0xB5 };
	uint8_t dkeyA[8] = { 0x00 };
	uint8_t dkeyB[8] = { 0x00 };

	uint8_t masterkey[] = { 0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff };

	uint8_t mix[8] = { 0x00 };
	uint8_t divkey[8] = { 0x00 };

	memcpy(mix, mifarekeyA, 4);

	mix[4] = mifarekeyA[4] ^ uid[0];
	mix[5] = mifarekeyA[5] ^ uid[1];
	mix[6] = block ^ uid[2];
	mix[7] = uid[3];

	mbedtls_des3_context ctx = { {0} };
	mbedtls_des3_set2key_enc(&ctx, masterkey);

	mbedtls_des3_crypt_cbc(&ctx  // des3_context
		, MBEDTLS_DES_ENCRYPT    // int mode
		, sizeof(mix)    // length
		, iv             // iv[8]
		, mix            // input
		, divkey         // output
		);

	PrintAndLogEx(NORMAL, "-- 3DES version");
	PrintAndLogEx(NORMAL, "Masterkey    :\t %s", sprint_hex(masterkey,sizeof(masterkey)));
	PrintAndLogEx(NORMAL, "UID          :\t %s", sprint_hex(uid, sizeof(uid)));
	PrintAndLogEx(NORMAL, "Block        :\t %0d", block);
	PrintAndLogEx(NORMAL, "Mifare key   :\t %s", sprint_hex(mifarekeyA, sizeof(mifarekeyA)));
	PrintAndLogEx(NORMAL, "Message      :\t %s", sprint_hex(mix, sizeof(mix)));
	PrintAndLogEx(NORMAL, "Diversified key: %s", sprint_hex(divkey+1, 6));

	for (int i=0; i < sizeof(mifarekeyA); ++i){
		dkeyA[i] = (mifarekeyA[i] << 1) & 0xff;
		dkeyA[6] |=  ((mifarekeyA[i] >> 7) & 1) << (i+1);
	}

	for (int i=0; i < sizeof(mifarekeyB); ++i){
		dkeyB[1] |=  ((mifarekeyB[i] >> 7) & 1) << (i+1);
		dkeyB[2+i] = (mifarekeyB[i] << 1) & 0xff;
	}

	uint8_t zeros[8] = {0x00};
	uint8_t newpwd[8] = {0x00};
	uint8_t dmkey[24] = {0x00};
	memcpy(dmkey, dkeyA, 8);
	memcpy(dmkey+8, dkeyB, 8);
	memcpy(dmkey+16, dkeyA, 8);
	memset(iv, 0x00, 8);

	mbedtls_des3_set3key_enc(&ctx, dmkey);

	mbedtls_des3_crypt_cbc(&ctx  // des3_context
		, MBEDTLS_DES_ENCRYPT    // int mode
		, sizeof(newpwd) // length
		, iv             // iv[8]
		, zeros         // input
		, newpwd         // output
		);

	PrintAndLogEx(NORMAL, "\n-- DES version");
	PrintAndLogEx(NORMAL, "Mifare dkeyA :\t %s", sprint_hex(dkeyA, sizeof(dkeyA)));
	PrintAndLogEx(NORMAL, "Mifare dkeyB :\t %s", sprint_hex(dkeyB, sizeof(dkeyB)));
	PrintAndLogEx(NORMAL, "Mifare ABA   :\t %s", sprint_hex(dmkey, sizeof(dmkey)));
	PrintAndLogEx(NORMAL, "Mifare Pwd   :\t %s", sprint_hex(newpwd, sizeof(newpwd)));

	return 0;
}

// static uint8_t * diversify_key(uint8_t * key){

 // for(int i=0; i<16; i++){
   // if(i<=6) key[i]^=cuid[i];
   // if(i>6) key[i]^=cuid[i%7];
 // }
 // return key;
// }

// static void GenerateUIDe( uint8_t *uid, uint8_t len){
	// for (int i=0; i<len; ++i){

	// }
	// return;
// }

//------------------------------------
// Menu Stuff
//------------------------------------
static int CmdHelp(const char *Cmd);

static command_t CommandTable[] =
{
	{"help",    CmdHelp,                   1, "This help"},
	{"dbg",     CmdHF14AMfDbg,             0, "Set default debug mode"},
	{"info",    CmdHF14AMfUInfo,           0, "Tag information"},
	{"dump",    CmdHF14AMfUDump,           0, "Dump Ultralight / Ultralight-C / NTAG tag to binary file"},
	// {"restore", CmdHF14AMfURestore,        0, "Restore a dump onto a MFU MAGIC tag"},
	{"rdbl",    CmdHF14AMfURdBl,           0, "Read block"},
	{"wrbl",    CmdHF14AMfUWrBl,           0, "Write block"},
	{"cauth",   CmdHF14AMfucAuth,          0, "Authentication    - Ultralight C"},
	{"setpwd",  CmdHF14AMfucSetPwd,        0, "Set 3des password - Ultralight-C"},
	{"setuid",  CmdHF14AMfucSetUid,        0, "Set UID - MAGIC tags only"},
	{"gen",     CmdHF14AMfuGenDiverseKeys, 1, "Generate 3des mifare diversified keys"},
	{NULL, NULL, 0, NULL}
};

int CmdHFMFUltra(const char *Cmd){
	(void)WaitForResponseTimeout(CMD_ACK,NULL,100);
	CmdsParse(CommandTable, Cmd);
	return 0;
}

static int CmdHelp(const char *Cmd){
	CmdsHelp(CommandTable);
	return 0;
}
