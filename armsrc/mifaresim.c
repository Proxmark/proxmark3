//-----------------------------------------------------------------------------
// Merlok - June 2011, 2012
// Gerhard de Koning Gans - May 2008
// Hagen Fritsch - June 2010
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Mifare Classic Card Simulation
//-----------------------------------------------------------------------------

#include "mifaresim.h"
#include "iso14443a.h"
#include "iso14443crc.h"
#include "crapto1/crapto1.h"
#include "BigBuf.h"
#include "string.h"
#include "mifareutil.h"
#include "fpgaloader.h"
#include "proxmark3.h"
#include "usb_cdc.h"
#include "cmd.h"
#include "protocols.h"
#include "apps.h"

//mifare emulator states
#define MFEMUL_NOFIELD           0
#define MFEMUL_IDLE              1
#define MFEMUL_SELECT1           2
#define MFEMUL_SELECT2           3
#define MFEMUL_SELECT3           4
#define MFEMUL_AUTH1             5
#define MFEMUL_AUTH2             6
#define MFEMUL_WORK              7
#define MFEMUL_WRITEBL2          8
#define MFEMUL_INTREG_INC        9
#define MFEMUL_INTREG_DEC       10
#define MFEMUL_INTREG_REST      11
#define MFEMUL_HALTED           12

#define AC_DATA_READ             0
#define AC_DATA_WRITE            1
#define AC_DATA_INC              2
#define AC_DATA_DEC_TRANS_REST   3
#define AC_KEYA_READ             0
#define AC_KEYA_WRITE            1
#define AC_KEYB_READ             2
#define AC_KEYB_WRITE            3
#define AC_AC_READ               4
#define AC_AC_WRITE              5

#define AUTHKEYA                 0
#define AUTHKEYB                 1
#define AUTHKEYNONE              0xff


static int ParamCardSizeBlocks(const char c) {
	int numBlocks = 16 * 4;
	switch (c) {
		case '0' : numBlocks = 5 * 4; break;
		case '2' : numBlocks = 32 * 4; break;
		case '4' : numBlocks = 32 * 4 + 8 * 16; break;
		default:   numBlocks = 16 * 4;
	}
	return numBlocks;
}

static uint8_t BlockToSector(int block_num) {
	if (block_num < 32 * 4) {    // 4 blocks per sector
		return (block_num / 4);
	} else {                     // 16 blocks per sector
		return 32 + (block_num - 32 * 4) / 16;
	}
}

static bool IsTrailerAccessAllowed(uint8_t blockNo, uint8_t keytype, uint8_t action) {
	uint8_t sector_trailer[16];
	emlGetMem(sector_trailer, blockNo, 1);
	uint8_t AC = ((sector_trailer[7] >> 5) & 0x04)
			   | ((sector_trailer[8] >> 2) & 0x02)
			   | ((sector_trailer[8] >> 7) & 0x01);
	switch (action) {
		case AC_KEYA_READ: {
			return false;
			break;
		}
		case AC_KEYA_WRITE: {
			return ((keytype == AUTHKEYA && (AC == 0x00 || AC == 0x01))
				 || (keytype == AUTHKEYB && (AC == 0x04 || AC == 0x03)));
			break;
		}
		case AC_KEYB_READ: {
			return (keytype == AUTHKEYA && (AC == 0x00 || AC == 0x02 || AC == 0x01));
			break;
		}
		case AC_KEYB_WRITE: {
			return ((keytype == AUTHKEYA && (AC == 0x00 || AC == 0x01))
				 || (keytype == AUTHKEYB && (AC == 0x04 || AC == 0x03)));
			break;
		}
		case AC_AC_READ: {
			return ((keytype == AUTHKEYA)
				 || (keytype == AUTHKEYB && !(AC == 0x00 || AC == 0x02 || AC == 0x01)));
			break;
		}
		case AC_AC_WRITE: {
			return ((keytype == AUTHKEYA && (AC == 0x01))
				 || (keytype == AUTHKEYB && (AC == 0x03 || AC == 0x05)));
			break;
		}
		default: return false;
	}
}


static bool IsDataAccessAllowed(uint8_t blockNo, uint8_t keytype, uint8_t action)
{
	uint8_t sector_trailer[16];
	emlGetMem(sector_trailer, SectorTrailer(blockNo), 1);

	uint8_t sector_block;
	if (blockNo < 32*4) {
		sector_block = blockNo & 0x03;
	} else {
		sector_block = (blockNo & 0x0f) / 5;
	}

	uint8_t AC;
	switch (sector_block) {
		case 0x00: {
			AC = ((sector_trailer[7] >> 2) & 0x04)
			   | ((sector_trailer[8] << 1) & 0x02)
			   | ((sector_trailer[8] >> 4) & 0x01);
			break;
		}
		case 0x01: {
			AC = ((sector_trailer[7] >> 3) & 0x04)
			   | ((sector_trailer[8] >> 0) & 0x02)
			   | ((sector_trailer[8] >> 5) & 0x01);
			break;
		}
		case 0x02: {
			AC = ((sector_trailer[7] >> 4) & 0x04)
			   | ((sector_trailer[8] >> 1) & 0x02)
			   | ((sector_trailer[8] >> 6) & 0x01);
			break;
		}
		default:
			return false;
	}

	switch (action) {
		case AC_DATA_READ: {
			return ((keytype == AUTHKEYA && !(AC == 0x03 || AC == 0x05 || AC == 0x07))
				 || (keytype == AUTHKEYB && !(AC == 0x07)));
			break;
		}
		case AC_DATA_WRITE: {
			return ((keytype == AUTHKEYA && (AC == 0x00))
				 || (keytype == AUTHKEYB && (AC == 0x00 || AC == 0x04 || AC == 0x06 || AC == 0x03)));
			break;
		}
		case AC_DATA_INC: {
			return ((keytype == AUTHKEYA && (AC == 0x00))
				 || (keytype == AUTHKEYB && (AC == 0x00 || AC == 0x06)));
			break;
		}
		case AC_DATA_DEC_TRANS_REST: {
			return ((keytype == AUTHKEYA && (AC == 0x00 || AC == 0x06 || AC == 0x01))
				 || (keytype == AUTHKEYB && (AC == 0x00 || AC == 0x06 || AC == 0x01)));
			break;
		}
	}

	return false;
}


static bool IsAccessAllowed(uint8_t blockNo, uint8_t keytype, uint8_t action) {
	if (IsSectorTrailer(blockNo)) {
		return IsTrailerAccessAllowed(blockNo, keytype, action);
	} else {
		return IsDataAccessAllowed(blockNo, keytype, action);
	}
}


static void MifareSimInit(uint8_t flags, uint8_t *datain, tag_response_info_t **responses, uint32_t *cuid, uint8_t *uid_len, uint8_t cardsize) {

	#define TAG_RESPONSE_COUNT 5                                // number of precompiled responses
	static uint8_t rATQA[]    = {0x00, 0x00};
	static uint8_t rUIDBCC1[] = {0x00, 0x00, 0x00, 0x00, 0x00}; // UID 1st cascade level
	static uint8_t rUIDBCC2[] = {0x00, 0x00, 0x00, 0x00, 0x00}; // UID 2nd cascade level
	static uint8_t rSAKfinal[]= {0x00, 0x00, 0x00};             // SAK after UID complete
	static uint8_t rSAK1[]    = {0x00, 0x00, 0x00};             // indicate UID not finished

	*uid_len = 4;
	// UID can be set from emulator memory or incoming data and can be 4 or 7 bytes long
	if (flags & FLAG_4B_UID_IN_DATA) {  // get UID from datain
		memcpy(rUIDBCC1, datain, 4);
	} else if (flags & FLAG_7B_UID_IN_DATA) {
		rUIDBCC1[0] = 0x88;
		memcpy(rUIDBCC1+1, datain, 3);
		memcpy(rUIDBCC2, datain+3, 4);
		*uid_len = 7;
	} else {
		uint8_t probable_atqa;
		emlGetMemBt(&probable_atqa, 7, 1);  // get UID from emul memory - weak guess at length
		if (probable_atqa == 0x00) {        // ---------- 4BUID
			emlGetMemBt(rUIDBCC1, 0, 4);
		} else {                            // ---------- 7BUID
			rUIDBCC1[0] = 0x88;
			emlGetMemBt(rUIDBCC1+1, 0, 3);
			emlGetMemBt(rUIDBCC2, 3, 4);
			*uid_len = 7;
		}
	}

	switch (*uid_len) {
		case 4:
			*cuid = bytes_to_num(rUIDBCC1, 4);
			rUIDBCC1[4] = rUIDBCC1[0] ^ rUIDBCC1[1] ^ rUIDBCC1[2] ^ rUIDBCC1[3];
			if (MF_DBGLEVEL >= MF_DBG_INFO)   {
				Dbprintf("4B UID: %02x%02x%02x%02x",
					rUIDBCC1[0], rUIDBCC1[1], rUIDBCC1[2], rUIDBCC1[3]  );
			}
			break;
		case 7:
			*cuid = bytes_to_num(rUIDBCC2, 4);
			rUIDBCC1[4] = rUIDBCC1[0] ^ rUIDBCC1[1] ^ rUIDBCC1[2] ^ rUIDBCC1[3];
			rUIDBCC2[4] = rUIDBCC2[0] ^ rUIDBCC2[1] ^ rUIDBCC2[2] ^ rUIDBCC2[3];
			if (MF_DBGLEVEL >= MF_DBG_INFO)   {
				Dbprintf("7B UID: %02x %02x %02x %02x %02x %02x %02x",
					rUIDBCC1[1], rUIDBCC1[2], rUIDBCC1[3], rUIDBCC2[0], rUIDBCC2[1], rUIDBCC2[2], rUIDBCC2[3]  );
			}
			break;
		default:
			break;
	}

	// set SAK based on cardsize
	switch (cardsize) {
		case '0': rSAKfinal[0] = 0x09; break; // Mifare Mini
		case '2': rSAKfinal[0] = 0x10; break; // Mifare 2K
		case '4': rSAKfinal[0] = 0x18; break; // Mifare 4K
		default: rSAKfinal[0] = 0x08;         // Mifare 1K
	}
	ComputeCrc14443(CRC_14443_A, rSAKfinal, 1, rSAKfinal + 1, rSAKfinal + 2);
	if (MF_DBGLEVEL >= MF_DBG_INFO)   {
		Dbprintf("SAK:    %02x", rSAKfinal[0]);
	}

	// set SAK for incomplete UID
	rSAK1[0] = 0x04;                          // Bit 3 indicates incomplete UID
	ComputeCrc14443(CRC_14443_A, rSAK1, 1, rSAK1 + 1, rSAK1 + 2);

	// set ATQA based on cardsize and UIDlen
	if (cardsize == '4') {
		rATQA[0] = 0x02;
	} else {
		rATQA[0] = 0x04;
	}
	if (*uid_len == 7) {
		rATQA[0] |= 0x40;
	}
	if (MF_DBGLEVEL >= MF_DBG_INFO)   {
		Dbprintf("ATQA:   %02x %02x", rATQA[1], rATQA[0]);
	}

	static tag_response_info_t responses_init[TAG_RESPONSE_COUNT] = {
		{ .response = rATQA,     .response_n = sizeof(rATQA)  },        // Answer to request - respond with card type
		{ .response = rUIDBCC1,  .response_n = sizeof(rUIDBCC1) },      // Anticollision cascade1 - respond with first part of uid
		{ .response = rUIDBCC2,  .response_n = sizeof(rUIDBCC2) },      // Anticollision cascade2 - respond with 2nd part of uid
		{ .response = rSAKfinal, .response_n = sizeof(rSAKfinal)  },    // Acknowledge select - last cascade
		{ .response = rSAK1,     .response_n = sizeof(rSAK1) }          // Acknowledge select - previous cascades
	};

	// Prepare ("precompile") the responses of the anticollision phase. There will be not enough time to do this at the moment the reader sends its REQA or SELECT
	// There are 5 predefined responses with a total of 18 bytes data to transmit. Coded responses need one byte per bit to transfer (data, parity, start, stop, correction)
	// 18 * 8 data bits, 18 * 1 parity bits, 5 start bits, 5 stop bits, 5 correction bits  ->   need 177 bytes buffer
	#define ALLOCATED_TAG_MODULATION_BUFFER_SIZE 177    // number of bytes required for precompiled responses

	uint8_t *free_buffer_pointer = BigBuf_malloc(ALLOCATED_TAG_MODULATION_BUFFER_SIZE);
	size_t free_buffer_size = ALLOCATED_TAG_MODULATION_BUFFER_SIZE;
	for (size_t i = 0; i < TAG_RESPONSE_COUNT; i++) {
		prepare_allocated_tag_modulation(&responses_init[i], &free_buffer_pointer, &free_buffer_size);
	}

	*responses = responses_init;

	// indices into responses array:
	#define ATQA     0
	#define UIDBCC1  1
	#define UIDBCC2  2
	#define SAKfinal 3
	#define SAK1     4

}


static bool HasValidCRC(uint8_t *receivedCmd, uint16_t receivedCmd_len) {
	uint8_t CRC_byte_1, CRC_byte_2;
	ComputeCrc14443(CRC_14443_A, receivedCmd, receivedCmd_len-2, &CRC_byte_1, &CRC_byte_2);
	return (receivedCmd[receivedCmd_len-2] == CRC_byte_1 && receivedCmd[receivedCmd_len-1] == CRC_byte_2);
}


/**
  *MIFARE simulate.
  *
  *@param flags :
  * FLAG_INTERACTIVE - In interactive mode, we are expected to finish the operation with an ACK
  * FLAG_4B_UID_IN_DATA - means that there is a 4-byte UID in the data-section, we're expected to use that
  * FLAG_7B_UID_IN_DATA - means that there is a 7-byte UID in the data-section, we're expected to use that
  * FLAG_NR_AR_ATTACK  - means we should collect NR_AR responses for bruteforcing later
  * FLAG_RANDOM_NONCE - means we should generate some pseudo-random nonce data (only allows moebius attack)
  *@param exitAfterNReads, exit simulation after n blocks have been read, 0 is infinite ...
  * (unless reader attack mode enabled then it runs util it gets enough nonces to recover all keys attmpted)
  */
void MifareSim(uint8_t flags, uint8_t exitAfterNReads, uint8_t cardsize, uint8_t *datain)
{
	LED_A_ON();

	tag_response_info_t *responses;
	uint8_t uid_len = 4;
	uint32_t cuid = 0;
	uint8_t cardWRBL = 0;
	uint8_t cardAUTHSC = 0;
	uint8_t cardAUTHKEY = AUTHKEYNONE;  // no authentication
	uint32_t cardRr = 0;
	//uint32_t rn_enc = 0;
	uint32_t ans = 0;
	uint32_t cardINTREG = 0;
	uint8_t cardINTBLOCK = 0;
	struct Crypto1State mpcs = {0, 0};
	struct Crypto1State *pcs = &mpcs;
	uint32_t numReads = 0; //Counts numer of times reader reads a block
	uint8_t receivedCmd[MAX_MIFARE_FRAME_SIZE];
	uint8_t receivedCmd_dec[MAX_MIFARE_FRAME_SIZE];
	uint8_t receivedCmd_par[MAX_MIFARE_PARITY_SIZE];
	uint16_t receivedCmd_len;
	uint8_t response[MAX_MIFARE_FRAME_SIZE];
	uint8_t response_par[MAX_MIFARE_PARITY_SIZE];
	uint8_t fixed_nonce[] = {0x01, 0x02, 0x03, 0x04};

	int num_blocks = ParamCardSizeBlocks(cardsize);

	// Here we collect UID, sector, keytype, NT, AR, NR, NT2, AR2, NR2
	// This will be used in the reader-only attack.

	// allow collecting up to 7 sets of nonces to allow recovery of up to 7 keys
	#define ATTACK_KEY_COUNT 7 // keep same as define in cmdhfmf.c -> readerAttack() (Cannot be more than 7)
	nonces_t ar_nr_resp[ATTACK_KEY_COUNT*2]; // *2 for 2 separate attack types (nml, moebius) 36 * 7 * 2 bytes = 504 bytes
	memset(ar_nr_resp, 0x00, sizeof(ar_nr_resp));

	uint8_t ar_nr_collected[ATTACK_KEY_COUNT*2]; // *2 for 2nd attack type (moebius)
	memset(ar_nr_collected, 0x00, sizeof(ar_nr_collected));
	uint8_t nonce1_count = 0;
	uint8_t nonce2_count = 0;
	uint8_t moebius_n_count = 0;
	bool gettingMoebius = false;
	uint8_t mM = 0; // moebius_modifier for collection storage

	// Authenticate response - nonce
	uint32_t nonce;
	if (flags & FLAG_RANDOM_NONCE) {
		nonce = prand();
	} else {
		nonce = bytes_to_num(fixed_nonce, 4);
	}

	// free eventually allocated BigBuf memory but keep Emulator Memory
	BigBuf_free_keep_EM();

	MifareSimInit(flags, datain, &responses, &cuid, &uid_len, cardsize);

	// We need to listen to the high-frequency, peak-detected path.
	iso14443a_setup(FPGA_HF_ISO14443A_TAGSIM_LISTEN);

	// clear trace
	clear_trace();
	set_tracing(true);
	ResetSspClk();

	bool finished = false;
	bool button_pushed = BUTTON_PRESS();
	int cardSTATE = MFEMUL_NOFIELD;

	while (!button_pushed && !finished && !usb_poll_validate_length()) {
		WDT_HIT();

		if (cardSTATE == MFEMUL_NOFIELD) {
			// wait for reader HF field
			int vHf = (MAX_ADC_HF_VOLTAGE_LOW * AvgAdc(ADC_CHAN_HF_LOW)) >> 10;
			if (vHf > MF_MINFIELDV) {
				LED_D_ON();
				cardSTATE = MFEMUL_IDLE;
			}
			button_pushed = BUTTON_PRESS();
			continue;
		}

		//Now, get data
		FpgaEnableTracing();
		int res = EmGetCmd(receivedCmd, &receivedCmd_len, receivedCmd_par);

		if (res == 2) { //  Reader has dropped the HF field. Power off.
			FpgaDisableTracing();
			LED_D_OFF();
			cardSTATE = MFEMUL_NOFIELD;
			continue;
		} else if (res == 1) { // button pressed
			FpgaDisableTracing();
			button_pushed = true;
			break;
		}

		// WUPA in HALTED state or REQA or WUPA in any other state
		if (receivedCmd_len == 1 && ((receivedCmd[0] == ISO14443A_CMD_REQA && cardSTATE != MFEMUL_HALTED) || receivedCmd[0] == ISO14443A_CMD_WUPA)) {
			EmSendPrecompiledCmd(&responses[ATQA]);
			FpgaDisableTracing();

			// init crypto block
			crypto1_destroy(pcs);
			cardAUTHKEY = AUTHKEYNONE;
			if (flags & FLAG_RANDOM_NONCE) {
				nonce = prand();
			}
			cardSTATE = MFEMUL_SELECT1;
			continue;
		}

		switch (cardSTATE) {
			case MFEMUL_NOFIELD:
			case MFEMUL_HALTED:
			case MFEMUL_IDLE:{
				break;
			}

			case MFEMUL_SELECT1:{
				// select all - 0x93 0x20
				if (receivedCmd_len == 2 && (receivedCmd[0] == ISO14443A_CMD_ANTICOLL_OR_SELECT && receivedCmd[1] == 0x20)) {
					EmSendPrecompiledCmd(&responses[UIDBCC1]);
					FpgaDisableTracing();
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED)   Dbprintf("SELECT ALL CL1 received");
					break;
				}
				// select card - 0x93 0x70 ...
				if (receivedCmd_len == 9 &&
						(receivedCmd[0] == ISO14443A_CMD_ANTICOLL_OR_SELECT && receivedCmd[1] == 0x70 && memcmp(&receivedCmd[2], responses[UIDBCC1].response, 4) == 0)) {
					if (uid_len == 4) {
						EmSendPrecompiledCmd(&responses[SAKfinal]);
						cardSTATE = MFEMUL_WORK;
					} else if (uid_len == 7) {
						EmSendPrecompiledCmd(&responses[SAK1]);
						cardSTATE = MFEMUL_SELECT2;
					}
					FpgaDisableTracing();
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("SELECT CL1 %02x%02x%02x%02x received",receivedCmd[2],receivedCmd[3],receivedCmd[4],receivedCmd[5]);
					break;
				}
				cardSTATE = MFEMUL_IDLE;
				break;
			}

			case MFEMUL_SELECT2:{
				// select all cl2 - 0x95 0x20
				if (receivedCmd_len == 2 && (receivedCmd[0] == ISO14443A_CMD_ANTICOLL_OR_SELECT_2 && receivedCmd[1] == 0x20)) {
					EmSendPrecompiledCmd(&responses[UIDBCC2]);
					FpgaDisableTracing();
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED)   Dbprintf("SELECT ALL CL2 received");
					break;
				}
				// select cl2 card - 0x95 0x70 xxxxxxxxxxxx
				if (receivedCmd_len == 9 &&
						(receivedCmd[0] == ISO14443A_CMD_ANTICOLL_OR_SELECT_2 && receivedCmd[1] == 0x70 && memcmp(&receivedCmd[2], responses[UIDBCC2].response, 4) == 0)) {
					if (uid_len == 7) {
						EmSendPrecompiledCmd(&responses[SAKfinal]);
						FpgaDisableTracing();
						if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("SELECT CL2 %02x%02x%02x%02x received",receivedCmd[2],receivedCmd[3],receivedCmd[4],receivedCmd[5]);
						cardSTATE = MFEMUL_WORK;
						break;
					}
				}
				cardSTATE = MFEMUL_IDLE;
				break;
			}

			case MFEMUL_WORK:{
				if (receivedCmd_len != 4) { // all commands must have exactly 4 bytes
					break;
				}
				bool encrypted_data = (cardAUTHKEY != AUTHKEYNONE) ;
				if (encrypted_data) {
					// decrypt seqence
					mf_crypto1_decryptEx(pcs, receivedCmd, receivedCmd_len, receivedCmd_dec);
				} else {
					memcpy(receivedCmd_dec, receivedCmd, receivedCmd_len);
				}
				if (!HasValidCRC(receivedCmd_dec, receivedCmd_len)) { // all commands must have a valid CRC
					EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_TR));
					break;
				}

				if (receivedCmd_dec[0] == MIFARE_AUTH_KEYA || receivedCmd_dec[0] == MIFARE_AUTH_KEYB) {
					// if authenticating to a block that shouldn't exist - as long as we are not doing the reader attack
					if (receivedCmd_dec[1] >= num_blocks && !(flags & FLAG_NR_AR_ATTACK)) {
						//is this the correct response to an auth on a out of range block? marshmellow
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
						FpgaDisableTracing();
						if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("Reader tried to operate (0x%02x) on out of range block: %d (0x%02x), nacking", receivedCmd_dec[0], receivedCmd_dec[1], receivedCmd_dec[1]);
						break;
					}
					cardAUTHSC = BlockToSector(receivedCmd_dec[1]);  // received block num
					cardAUTHKEY = receivedCmd_dec[0] & 0x01;
					crypto1_destroy(pcs);//Added by martin
					crypto1_create(pcs, emlGetKey(cardAUTHSC, cardAUTHKEY));
					if (!encrypted_data) { // first authentication
						crypto1_word(pcs, cuid ^ nonce, 0); // Update crypto state
						num_to_bytes(nonce, 4, response);   // Send unencrypted nonce
						EmSendCmd(response, sizeof(nonce));
						FpgaDisableTracing();
						if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("Reader authenticating for block %d (0x%02x) with key %d", receivedCmd_dec[1], receivedCmd_dec[1], cardAUTHKEY);
					} else { // nested authentication
						num_to_bytes(nonce, sizeof(nonce), response);
						uint8_t pcs_in[4] = {0};
						num_to_bytes(cuid ^ nonce, sizeof(nonce), pcs_in);
						mf_crypto1_encryptEx(pcs, response, pcs_in, sizeof(nonce), response_par);
						EmSendCmdPar(response, sizeof(nonce), response_par); // send encrypted nonce
						FpgaDisableTracing();
						if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("Reader doing nested authentication for block %d (0x%02x) with key %d", receivedCmd_dec[1], receivedCmd_dec[1], cardAUTHKEY);
					}
					cardSTATE = MFEMUL_AUTH1;
					break;
				}

				// halt can be sent encrypted or in clear
				if (receivedCmd_dec[0] == ISO14443A_CMD_HALT && receivedCmd_dec[1] == 0x00) {
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED)   Dbprintf("--> HALTED.");
					cardSTATE = MFEMUL_HALTED;
					break;
				}

				if(receivedCmd_dec[0] == MIFARE_CMD_READBLOCK
					|| receivedCmd_dec[0] == MIFARE_CMD_WRITEBLOCK
					|| receivedCmd_dec[0] == MIFARE_CMD_INC
					|| receivedCmd_dec[0] == MIFARE_CMD_DEC
					|| receivedCmd_dec[0] == MIFARE_CMD_RESTORE
					|| receivedCmd_dec[0] == MIFARE_CMD_TRANSFER) {
					if (receivedCmd_dec[1] >= num_blocks) {
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
						FpgaDisableTracing();
						if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("Reader tried to operate (0x%02x) on out of range block: %d (0x%02x), nacking",receivedCmd_dec[0],receivedCmd_dec[1],receivedCmd_dec[1]);
						break;
					}
					if (BlockToSector(receivedCmd_dec[1]) != cardAUTHSC) {
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
						FpgaDisableTracing();
						if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("Reader tried to operate (0x%02x) on block (0x%02x) not authenticated for (0x%02x), nacking",receivedCmd_dec[0],receivedCmd_dec[1],cardAUTHSC);
						break;
					}
				}

				if (receivedCmd_dec[0] == MIFARE_CMD_READBLOCK) {
					uint8_t blockNo = receivedCmd_dec[1];
					emlGetMem(response, blockNo, 1);
					if (IsSectorTrailer(blockNo)) {
						memset(response, 0x00, 6);  // keyA can never be read
						if (!IsAccessAllowed(blockNo, cardAUTHKEY, AC_KEYB_READ)) {
							memset(response+10, 0x00, 6);   // keyB cannot be read
						}
						if (!IsAccessAllowed(blockNo, cardAUTHKEY, AC_AC_READ)) {
							memset(response+6, 0x00, 4);    // AC bits cannot be read
						}
					} else {
						if (!IsAccessAllowed(blockNo, cardAUTHKEY, AC_DATA_READ)) {
							memset(response, 0x00, 16);     // datablock cannot be read
						}
					}
					AppendCrc14443a(response, 16);
					mf_crypto1_encrypt(pcs, response, 18, response_par);
					EmSendCmdPar(response, 18, response_par);
					FpgaDisableTracing();
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED) {
						Dbprintf("Reader reading block %d (0x%02x)", blockNo, blockNo);
					}
					numReads++;
					if(exitAfterNReads > 0 && numReads == exitAfterNReads) {
						Dbprintf("%d reads done, exiting", numReads);
						finished = true;
					}
					break;
				}

				if (receivedCmd_dec[0] == MIFARE_CMD_WRITEBLOCK) {
					uint8_t blockNo = receivedCmd_dec[1];
					EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_ACK));
					FpgaDisableTracing();
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("RECV 0xA0 write block %d (%02x)", blockNo, blockNo);
					cardWRBL = blockNo;
					cardSTATE = MFEMUL_WRITEBL2;
					break;
				}

				if (receivedCmd_dec[0] == MIFARE_CMD_INC || receivedCmd_dec[0] == MIFARE_CMD_DEC || receivedCmd_dec[0] == MIFARE_CMD_RESTORE) {
					uint8_t blockNo = receivedCmd_dec[1];
					if (emlCheckValBl(blockNo)) {
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
						FpgaDisableTracing();
						if (MF_DBGLEVEL >= MF_DBG_EXTENDED) {
							Dbprintf("RECV 0x%02x inc(0xC1)/dec(0xC0)/restore(0xC2) block %d (%02x)",receivedCmd_dec[0], blockNo, blockNo);
						}
						if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("Reader tried to operate on block, but emlCheckValBl failed, nacking");
						break;
					}
					EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_ACK));
					FpgaDisableTracing();
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED) {
						Dbprintf("RECV 0x%02x inc(0xC1)/dec(0xC0)/restore(0xC2) block %d (%02x)",receivedCmd_dec[0], blockNo, blockNo);
					}
					cardWRBL = blockNo;
					if (receivedCmd_dec[0] == MIFARE_CMD_INC)
						cardSTATE = MFEMUL_INTREG_INC;
					if (receivedCmd_dec[0] == MIFARE_CMD_DEC)
						cardSTATE = MFEMUL_INTREG_DEC;
					if (receivedCmd_dec[0] == MIFARE_CMD_RESTORE)
						cardSTATE = MFEMUL_INTREG_REST;
					break;
				}

				if (receivedCmd_dec[0] == MIFARE_CMD_TRANSFER) {
					uint8_t blockNo = receivedCmd_dec[1];
					if (emlSetValBl(cardINTREG, cardINTBLOCK, receivedCmd_dec[1]))
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
					else
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_ACK));
					FpgaDisableTracing();
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("RECV 0x%02x transfer block %d (%02x)",receivedCmd_dec[0], blockNo, blockNo);
					break;
				}

				// command not allowed
				EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
				FpgaDisableTracing();
				if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("Received command not allowed, nacking");
				cardSTATE = MFEMUL_IDLE;
				break;
			}

			case MFEMUL_AUTH1:{
				if (receivedCmd_len != 8) {
					cardSTATE = MFEMUL_IDLE;
					break;
				}

				uint32_t nr = bytes_to_num(receivedCmd, 4);
				uint32_t ar = bytes_to_num(&receivedCmd[4], 4);

				// Collect AR/NR per keytype & sector
				if(flags & FLAG_NR_AR_ATTACK) {
					for (uint8_t i = 0; i < ATTACK_KEY_COUNT; i++) {
						if ( ar_nr_collected[i+mM]==0 || ((cardAUTHSC == ar_nr_resp[i+mM].sector) && (cardAUTHKEY == ar_nr_resp[i+mM].keytype) && (ar_nr_collected[i+mM] > 0)) ) {
							// if first auth for sector, or matches sector and keytype of previous auth
							if (ar_nr_collected[i+mM] < 2) {
								// if we haven't already collected 2 nonces for this sector
								if (ar_nr_resp[ar_nr_collected[i+mM]].ar != ar) {
									// Avoid duplicates... probably not necessary, ar should vary.
									if (ar_nr_collected[i+mM]==0) {
										// first nonce collect
										ar_nr_resp[i+mM].cuid = cuid;
										ar_nr_resp[i+mM].sector = cardAUTHSC;
										ar_nr_resp[i+mM].keytype = cardAUTHKEY;
										ar_nr_resp[i+mM].nonce = nonce;
										ar_nr_resp[i+mM].nr = nr;
										ar_nr_resp[i+mM].ar = ar;
										nonce1_count++;
										// add this nonce to first moebius nonce
										ar_nr_resp[i+ATTACK_KEY_COUNT].cuid = cuid;
										ar_nr_resp[i+ATTACK_KEY_COUNT].sector = cardAUTHSC;
										ar_nr_resp[i+ATTACK_KEY_COUNT].keytype = cardAUTHKEY;
										ar_nr_resp[i+ATTACK_KEY_COUNT].nonce = nonce;
										ar_nr_resp[i+ATTACK_KEY_COUNT].nr = nr;
										ar_nr_resp[i+ATTACK_KEY_COUNT].ar = ar;
										ar_nr_collected[i+ATTACK_KEY_COUNT]++;
									} else { // second nonce collect (std and moebius)
										ar_nr_resp[i+mM].nonce2 = nonce;
										ar_nr_resp[i+mM].nr2 = nr;
										ar_nr_resp[i+mM].ar2 = ar;
										if (!gettingMoebius) {
											nonce2_count++;
											// check if this was the last second nonce we need for std attack
											if ( nonce2_count == nonce1_count ) {
												// done collecting std test switch to moebius
												// first finish incrementing last sample
												ar_nr_collected[i+mM]++;
												// switch to moebius collection
												gettingMoebius = true;
												mM = ATTACK_KEY_COUNT;
												if (flags & FLAG_RANDOM_NONCE) {
													nonce = prand();
												} else {
													nonce = nonce*7;
												}
												break;
											}
										} else {
											moebius_n_count++;
											// if we've collected all the nonces we need - finish.
											if (nonce1_count == moebius_n_count) finished = true;
										}
									}
									ar_nr_collected[i+mM]++;
								}
							}
							// we found right spot for this nonce stop looking
							break;
						}
					}
				}

				// --- crypto
				crypto1_word(pcs, nr , 1);
				cardRr = ar ^ crypto1_word(pcs, 0, 0);

				// test if auth OK
				if (cardRr != prng_successor(nonce, 64)){
					FpgaDisableTracing();
					if (MF_DBGLEVEL >= MF_DBG_EXTENDED) Dbprintf("AUTH FAILED for sector %d with key %c. cardRr=%08x, succ=%08x",
							cardAUTHSC, cardAUTHKEY == AUTHKEYA ? 'A' : 'B',
							cardRr, prng_successor(nonce, 64));
					// Shouldn't we respond anything here?
					// Right now, we don't nack or anything, which causes the
					// reader to do a WUPA after a while. /Martin
					// -- which is the correct response. /piwi
					cardAUTHKEY = AUTHKEYNONE;  // not authenticated
					cardSTATE = MFEMUL_IDLE;
					break;
				}
				ans = prng_successor(nonce, 96);
				num_to_bytes(ans, 4, response);
				mf_crypto1_encrypt(pcs, response, 4, response_par);
				EmSendCmdPar(response, 4, response_par);
				FpgaDisableTracing();
				if (MF_DBGLEVEL >= MF_DBG_EXTENDED)   Dbprintf("AUTH COMPLETED for sector %d with key %c.", cardAUTHSC, cardAUTHKEY == AUTHKEYA ? 'A' : 'B');
				cardSTATE = MFEMUL_WORK;
				break;
			}

			case MFEMUL_WRITEBL2:{
				if (receivedCmd_len == 18) {
					mf_crypto1_decryptEx(pcs, receivedCmd, receivedCmd_len, receivedCmd_dec);
					if (HasValidCRC(receivedCmd_dec, receivedCmd_len)) {
						if (IsSectorTrailer(cardWRBL)) {
							emlGetMem(response, cardWRBL, 1);
							if (!IsAccessAllowed(cardWRBL, cardAUTHKEY, AC_KEYA_WRITE)) {
								memcpy(receivedCmd_dec, response, 6);   // don't change KeyA
							}
							if (!IsAccessAllowed(cardWRBL, cardAUTHKEY, AC_KEYB_WRITE)) {
								memcpy(receivedCmd_dec+10, response+10, 6); // don't change KeyA
							}
							if (!IsAccessAllowed(cardWRBL, cardAUTHKEY, AC_AC_WRITE)) {
								memcpy(receivedCmd_dec+6, response+6, 4);   // don't change AC bits
							}
						} else {
							if (!IsAccessAllowed(cardWRBL, cardAUTHKEY, AC_DATA_WRITE)) {
								memcpy(receivedCmd_dec, response, 16);  // don't change anything
							}
						}
						emlSetMem(receivedCmd_dec, cardWRBL, 1);
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_ACK));  // always ACK?
						cardSTATE = MFEMUL_WORK;
						break;
					}
				}
				cardSTATE = MFEMUL_IDLE;
				break;
			}

			case MFEMUL_INTREG_INC:{
				if (receivedCmd_len == 6) {
					mf_crypto1_decryptEx(pcs, receivedCmd, receivedCmd_len, (uint8_t*)&ans);
					if (emlGetValBl(&cardINTREG, &cardINTBLOCK, cardWRBL)) {
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
						cardSTATE = MFEMUL_IDLE;
						break;
					}
					cardINTREG = cardINTREG + ans;
					cardSTATE = MFEMUL_WORK;
				}
				break;
			}

			case MFEMUL_INTREG_DEC:{
				if (receivedCmd_len == 6) {
					mf_crypto1_decryptEx(pcs, receivedCmd, receivedCmd_len, (uint8_t*)&ans);
					if (emlGetValBl(&cardINTREG, &cardINTBLOCK, cardWRBL)) {
						EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
						cardSTATE = MFEMUL_IDLE;
						break;
					}
					cardINTREG = cardINTREG - ans;
					cardSTATE = MFEMUL_WORK;
				}
				break;
			}

			case MFEMUL_INTREG_REST:{
				mf_crypto1_decryptEx(pcs, receivedCmd, receivedCmd_len, (uint8_t*)&ans);
				if (emlGetValBl(&cardINTREG, &cardINTBLOCK, cardWRBL)) {
					EmSend4bit(mf_crypto1_encrypt4bit(pcs, CARD_NACK_NA));
					cardSTATE = MFEMUL_IDLE;
					break;
				}
				cardSTATE = MFEMUL_WORK;
				break;
			}

		} // end of switch

		FpgaDisableTracing();
		button_pushed = BUTTON_PRESS();

	} // end of while

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LEDsoff();

	if(flags & FLAG_NR_AR_ATTACK && MF_DBGLEVEL >= MF_DBG_INFO) {
		for ( uint8_t   i = 0; i < ATTACK_KEY_COUNT; i++) {
			if (ar_nr_collected[i] == 2) {
				Dbprintf("Collected two pairs of AR/NR which can be used to extract %s from reader for sector %d:", (i<ATTACK_KEY_COUNT/2) ? "keyA" : "keyB", ar_nr_resp[i].sector);
				Dbprintf("../tools/mfkey/mfkey32 %08x %08x %08x %08x %08x %08x",
						ar_nr_resp[i].cuid,  //UID
						ar_nr_resp[i].nonce, //NT
						ar_nr_resp[i].nr,    //NR1
						ar_nr_resp[i].ar,    //AR1
						ar_nr_resp[i].nr2,   //NR2
						ar_nr_resp[i].ar2    //AR2
						);
			}
		}
		for ( uint8_t   i = ATTACK_KEY_COUNT; i < ATTACK_KEY_COUNT*2; i++) {
			if (ar_nr_collected[i] == 2) {
				Dbprintf("Collected two pairs of AR/NR which can be used to extract %s from reader for sector %d:", (i<ATTACK_KEY_COUNT/2) ? "keyA" : "keyB", ar_nr_resp[i].sector);
				Dbprintf("../tools/mfkey/mfkey32 %08x %08x %08x %08x %08x %08x %08x",
						ar_nr_resp[i].cuid,  //UID
						ar_nr_resp[i].nonce, //NT
						ar_nr_resp[i].nr,    //NR1
						ar_nr_resp[i].ar,    //AR1
						ar_nr_resp[i].nonce2,//NT2
						ar_nr_resp[i].nr2,   //NR2
						ar_nr_resp[i].ar2    //AR2
						);
			}
		}
	}
	if (MF_DBGLEVEL >= MF_DBG_INFO) Dbprintf("Emulator stopped. Tracing: %d  trace length: %d ", get_tracing(), BigBuf_get_traceLen());

	if(flags & FLAG_INTERACTIVE) { // Interactive mode flag, means we need to send ACK
		//Send the collected ar_nr in the response
		cmd_send(CMD_ACK, CMD_SIMULATE_MIFARE_CARD, button_pushed, 0, &ar_nr_resp, sizeof(ar_nr_resp));
	}

	LED_A_OFF();
}
