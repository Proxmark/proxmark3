//-----------------------------------------------------------------------------
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>, Hagen Fritsch
// Copyright (C) 2011 Gerhard de Koning Gans
// Copyright (C) 2014 Midnitesnake & Andy Davies & Martin Holst Swende
// Copyright (C) 2019 piwi
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// High frequency iClass commands
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "iso14443crc.h" // Can also be used for iClass, using 0xE012 as CRC-type
#include "comms.h"
#include "ui.h"
#include "cliparser/cliparser.h"
#include "cmdparser.h"
#include "cmdhficlass.h"
#include "common.h"
#include "util.h"
#include "cmdmain.h"
#include "mbedtls/des.h"
#include "loclass/cipherutils.h"
#include "loclass/cipher.h"
#include "loclass/ikeys.h"
#include "loclass/elite_crack.h"
#include "loclass/fileutils.h"
#include "protocols.h"
#include "usb_cmd.h"
#include "cmdhfmfu.h"
#include "util_posix.h"
#include "cmdhf14a.h" // DropField()


#define ICLASS_KEYS_MAX 8
static uint8_t iClass_Key_Table[ICLASS_KEYS_MAX][8] = {
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
		{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }
};


// iclass / picopass chip config structures and shared routines
typedef struct {
	uint8_t app_limit;      //[8]
	uint8_t otp[2];         //[9-10]
	uint8_t block_writelock;//[11]
	uint8_t chip_config;    //[12]
	uint8_t mem_config;     //[13]
	uint8_t eas;            //[14]
	uint8_t fuses;          //[15]
} picopass_conf_block;

typedef struct {
	uint8_t csn[8];
	picopass_conf_block conf;
	uint8_t epurse[8];
	uint8_t key_d[8];
	uint8_t key_c[8];
	uint8_t app_issuer_area[8];
} picopass_hdr;


static void fuse_config(const picopass_hdr *hdr) {
	uint8_t fuses = hdr->conf.fuses;

	if (fuses & FUSE_FPERS)
		PrintAndLog("  Mode: Personalization [Programmable]");
	else
		PrintAndLog("  Mode: Application [Locked]");

	if (fuses & FUSE_CODING1)
		PrintAndLog("Coding: RFU");
	else {
		if (fuses & FUSE_CODING0)
			PrintAndLog("Coding: ISO 14443-2 B/ISO 15693");
		else
			PrintAndLog("Coding: ISO 14443B only");
	}
	if ((fuses & FUSE_CRYPT1) && (fuses & FUSE_CRYPT0)) PrintAndLog(" Crypt: Secured page, keys not locked");
	if ((fuses & FUSE_CRYPT1) && !(fuses & FUSE_CRYPT0)) PrintAndLog(" Crypt: Secured page, keys locked");
	if (!(fuses & FUSE_CRYPT1) && (fuses & FUSE_CRYPT0)) PrintAndLog(" Crypt: Non secured page");
	if (!(fuses & FUSE_CRYPT1) && !(fuses & FUSE_CRYPT0)) PrintAndLog(" Crypt: No auth possible. Read only if RA is enabled");

	if (fuses & FUSE_RA)
		PrintAndLog("    RA: Read access enabled");
	else
		PrintAndLog("    RA: Read access not enabled");
}


static void getMemConfig(uint8_t mem_cfg, uint8_t chip_cfg, uint8_t *max_blk, uint8_t *app_areas, uint8_t *kb) {
	// mem-bit 5, mem-bit 7, chip-bit 4: defines chip type
	if((chip_cfg & 0x10) && !(mem_cfg & 0x80) && !(mem_cfg & 0x20)) {
		*kb = 2;
		*app_areas = 2;
		*max_blk = 31;
	} else if((chip_cfg & 0x10) && (mem_cfg & 0x80) && !(mem_cfg & 0x20)) {
		*kb = 16;
		*app_areas = 2;
		*max_blk = 255; //16kb
	} else if(!(chip_cfg & 0x10) && !(mem_cfg & 0x80) && !(mem_cfg & 0x20)) {
		*kb = 16;
		*app_areas = 16;
		*max_blk = 255; //16kb
	} else if((chip_cfg & 0x10) && (mem_cfg & 0x80) && (mem_cfg & 0x20)) {
		*kb = 32;
		*app_areas = 3;
		*max_blk = 255; //16kb
	} else if(!(chip_cfg & 0x10) && !(mem_cfg & 0x80) && (mem_cfg & 0x20)) {
		*kb = 32;
		*app_areas = 17;
		*max_blk = 255; //16kb
	} else {
		*kb = 32;
		*app_areas = 2;
		*max_blk = 255;
	}
}


static void mem_app_config(const picopass_hdr *hdr) {
	uint8_t mem = hdr->conf.mem_config;
	uint8_t chip = hdr->conf.chip_config;
	uint8_t applimit = hdr->conf.app_limit;
	if (applimit < 6) applimit = 26;
	uint8_t kb = 2;
	uint8_t app_areas = 2;
	uint8_t max_blk = 31;
	getMemConfig(mem, chip, &max_blk, &app_areas, &kb);
	PrintAndLog("   Mem: %u KBits/%u App Areas (%u * 8 bytes) [%02X]", kb, app_areas, max_blk+1, mem);
	PrintAndLog("   AA1: blocks 06-%02X", applimit);
	PrintAndLog("   AA2: blocks %02X-%02X", applimit+1, max_blk);
}


static void printIclassDumpInfo(uint8_t* iclass_dump) {
	fuse_config((picopass_hdr*)iclass_dump);
	mem_app_config((picopass_hdr*)iclass_dump);
}


static void usage_hf_iclass_chk(void) {
	PrintAndLog("Checkkeys loads a dictionary text file with 8byte hex keys to test authenticating against a iClass tag");
	PrintAndLog("Usage: hf iclass chk [h|e|r] <f  (*.dic)>");
	PrintAndLog("Options:");
	PrintAndLog("h             Show this help");
	PrintAndLog("f <filename>  Dictionary file with default iclass keys");
	PrintAndLog("      e             target Elite / High security key scheme");
	PrintAndLog("      r             interpret dictionary file as raw (diversified keys)");
	PrintAndLog("Samples:");
	PrintAndLog("        hf iclass chk f default_iclass_keys.dic");
	PrintAndLog("        hf iclass chk f default_iclass_keys.dic e");
}


static int CmdHFiClassList(const char *Cmd) {
	PrintAndLog("Deprecated command, use 'hf list iclass' instead");
	return 0;
}


static int CmdHFiClassSnoop(const char *Cmd) {

	CLIParserInit("hf iclass snoop", "\nSnoop a communication between an iClass Reader and an iClass Tag.", NULL);
	void* argtable[] = {
		arg_param_begin,
		arg_lit0("j",  "jam",    "Jam (prevent) e-purse Updates"),
		arg_param_end
	};
	if (CLIParserParseString(Cmd, argtable, arg_getsize(argtable), true)){
		CLIParserFree();
		return 0;
	}

	bool jam_epurse_update = arg_get_lit(1);

	const uint8_t update_epurse_sequence[2] = {0x87, 0x02};

	UsbCommand c = {CMD_SNOOP_ICLASS, {0}};
	if (jam_epurse_update) {
		c.arg[0] = sizeof(update_epurse_sequence);
		memcpy(c.d.asBytes, update_epurse_sequence, sizeof(update_epurse_sequence));
	}
	SendCommand(&c);

	return 0;
}


static void usage_hf_iclass_sim(void) {
	PrintAndLog("Usage:  hf iclass sim <option> [CSN]");
	PrintAndLog("        options");
	PrintAndLog("                0 <CSN> simulate the given CSN");
	PrintAndLog("                1       simulate default CSN");
	PrintAndLog("                2       Reader-attack, gather reader responses to extract elite key");
	PrintAndLog("                3       Full simulation using emulator memory (see 'hf iclass eload')");
	PrintAndLog("        example: hf iclass sim 0 031FEC8AF7FF12E0");
	PrintAndLog("        example: hf iclass sim 2");
	PrintAndLog("        example: hf iclass eload 'tagdump.bin'");
	PrintAndLog("                 hf iclass sim 3");
}


// the original malicious IDs from Flavio D. Garcia, Gerhard de Koning Gans, Roel Verdult,
// and Milosch Meriac. Dismantling iClass and iClass Elite.
#define NUM_CSNS 15
static uint8_t csns[8 * NUM_CSNS] = {
	0x00, 0x0B, 0x0F, 0xFF, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x04, 0x0E, 0x08, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x09, 0x0D, 0x05, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x0A, 0x0C, 0x06, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x0F, 0x0B, 0x03, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x08, 0x0A, 0x0C, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x0D, 0x09, 0x09, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x0E, 0x08, 0x0A, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x03, 0x07, 0x17, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x3C, 0x06, 0xE0, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x01, 0x05, 0x1D, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x02, 0x04, 0x1E, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x07, 0x03, 0x1B, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x00, 0x02, 0x24, 0xF7, 0xFF, 0x12, 0xE0,
	0x00, 0x05, 0x01, 0x21, 0xF7, 0xFF, 0x12, 0xE0 };


// pre-defined 9 CSNs by iceman.
// only one csn depend on several others.
// six depends only on the first csn,  (0,1, 0x45)

// #define NUM_CSNS 9
// static uint8_t csns[8 * NUM_CSNS] = {
	// 0x01, 0x0A, 0x0F, 0xFF, 0xF7, 0xFF, 0x12, 0xE0,
	// 0x0C, 0x06, 0x0C, 0xFE, 0xF7, 0xFF, 0x12, 0xE0,
	// 0x10, 0x97, 0x83, 0x7B, 0xF7, 0xFF, 0x12, 0xE0,
	// 0x13, 0x97, 0x82, 0x7A, 0xF7, 0xFF, 0x12, 0xE0,
	// 0x07, 0x0E, 0x0D, 0xF9, 0xF7, 0xFF, 0x12, 0xE0,
	// 0x14, 0x96, 0x84, 0x76, 0xF7, 0xFF, 0x12, 0xE0,
	// 0x17, 0x96, 0x85, 0x71, 0xF7, 0xFF, 0x12, 0xE0,
	// 0xCE, 0xC5, 0x0F, 0x77, 0xF7, 0xFF, 0x12, 0xE0,
	// 0xD2, 0x5A, 0x82, 0xF8, 0xF7, 0xFF, 0x12, 0xE0
	// //0x04, 0x08, 0x9F, 0x78, 0x6E, 0xFF, 0x12, 0xE0
// };


static int CmdHFiClassSim(const char *Cmd) {
	uint8_t simType = 0;
	uint8_t CSN[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	if (strlen(Cmd) < 1) {
		usage_hf_iclass_sim();
		return 0;
	}
	simType = param_get8ex(Cmd, 0, 0, 10);

	if (simType == ICLASS_SIM_MODE_CSN) {
		if (param_gethex(Cmd, 1, CSN, 16)) {
			PrintAndLog("A CSN should consist of 16 HEX symbols");
			usage_hf_iclass_sim();
			return 0;
		}
		PrintAndLog("--simtype:%02x csn:%s", simType, sprint_hex(CSN, 8));
	}

	if (simType == ICLASS_SIM_MODE_READER_ATTACK) {
		UsbCommand c = {CMD_SIMULATE_TAG_ICLASS, {simType, NUM_CSNS}};
		UsbCommand resp = {0};

		memcpy(c.d.asBytes, csns, 8 * NUM_CSNS);

		SendCommand(&c);
		if (!WaitForResponseTimeout(CMD_ACK, &resp, -1)) {
			PrintAndLog("Command timed out");
			return 0;
		}

		uint8_t num_mac_responses  = resp.arg[1];
		PrintAndLog("Mac responses: %d MACs obtained (should be %d)", num_mac_responses, NUM_CSNS);

		size_t datalen = NUM_CSNS * 24;
		/*
		 * Now, time to dump to file. We'll use this format:
		 * <8-byte CSN><8-byte CC><4 byte NR><4 byte MAC>....
		 * So, it should wind up as
		 * 8 * 24 bytes.
		 *
		 * The returndata from the pm3 is on the following format
		 * <8 byte CC><4 byte NR><4 byte MAC>
		 * CSN is the same as was sent in
		 **/
		void* dump = malloc(datalen);
		for(int i = 0; i < NUM_CSNS; i++) {
			memcpy(dump + i*24, csns+i*8, 8); //CSN
			//copy CC from response
			memcpy(dump + i*24 + 8, resp.d.asBytes + i*16, 8);
			//Then comes NR_MAC (eight bytes from the response)
			memcpy(dump + i*24 + 16, resp.d.asBytes + i*16 + 8, 8);
		}
		/** Now, save to dumpfile **/
		saveFile("iclass_mac_attack", "bin", dump,datalen);
		free(dump);

	} else if (simType == ICLASS_SIM_MODE_CSN || simType == ICLASS_SIM_MODE_CSN_DEFAULT || simType == ICLASS_SIM_MODE_FULL) {
		UsbCommand c = {CMD_SIMULATE_TAG_ICLASS, {simType, 0}};
		memcpy(c.d.asBytes, CSN, 8);
		SendCommand(&c);

	} else {
		PrintAndLog("Undefined simtype %d", simType);
		usage_hf_iclass_sim();
		return 0;
	}

	return 0;
}


int HFiClassReader(bool loop, bool verbose) {

	bool tagFound = false;
	UsbCommand c = {CMD_READER_ICLASS, {FLAG_ICLASS_READER_INIT | FLAG_ICLASS_READER_CLEARTRACE | FLAG_ICLASS_READER_CSN | FLAG_ICLASS_READER_CONF | FLAG_ICLASS_READER_CC | FLAG_ICLASS_READER_AA} };
	UsbCommand resp;

	while (!ukbhit()) {
		SendCommand(&c);
		if (WaitForResponseTimeout(CMD_ACK, &resp, 1000)) {
			uint8_t readStatus = resp.arg[0] & 0xff;
			uint8_t *data = resp.d.asBytes;

			// no tag found
			if (readStatus == 0 && !loop) {
				// abort
				if (verbose) PrintAndLog("Quitting...");
				DropField();
				return 0;
			}

			if (readStatus & FLAG_ICLASS_READER_CSN) {
				PrintAndLog("   CSN: %s",sprint_hex(data,8));
				tagFound = true;
			}
			if (readStatus & FLAG_ICLASS_READER_CC) {
				PrintAndLog("    CC: %s",sprint_hex(data+16,8));
			}
			if (readStatus & FLAG_ICLASS_READER_CONF) {
				printIclassDumpInfo(data);
			}
			if (readStatus & FLAG_ICLASS_READER_AA) {
				bool legacy = true;
				PrintAndLog(" AppIA: %s",sprint_hex(data+8*5,8));
				for (int i = 0; i<8; i++) {
					if (data[8*5+i] != 0xFF) {
						legacy = false;
					}
				}
				PrintAndLog("      : Possible iClass %s",(legacy) ? "(legacy tag)" : "(NOT legacy tag)");
			}

			if (tagFound && !loop) return 1;
		} else {
			if (verbose) PrintAndLog("Error: No response from Proxmark.");
			break;
		}
		if (!loop) break;
	}

	DropField();
	return 0;
}


static void usage_hf_iclass_reader(void) {
	PrintAndLogEx(NORMAL, "Act as a Iclass reader.  Look for iClass tags until Enter or the pm3 button is pressed\n");
	PrintAndLogEx(NORMAL, "Usage:  hf iclass reader [h] [1]\n");
	PrintAndLogEx(NORMAL, "Options:");
	PrintAndLogEx(NORMAL, "    h   This help text");
	PrintAndLogEx(NORMAL, "    1   read only 1 tag");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "        hf iclass reader 1");
}


static int CmdHFiClassReader(const char *Cmd) {
	char cmdp = tolower(param_getchar(Cmd, 0));
	if (cmdp == 'h') {
		usage_hf_iclass_reader();
		return 0;
	}
	bool findone = (cmdp == '1') ? false : true;
	return HFiClassReader(findone, true);
}


static void usage_hf_iclass_eload(void) {
	PrintAndLog("Loads iclass tag-dump into emulator memory on device");
	PrintAndLog("Usage:  hf iclass eload f <filename>");
	PrintAndLog("");
	PrintAndLog("Example: hf iclass eload f iclass_tagdump-aa162d30f8ff12f1.bin");
}


static int CmdHFiClassELoad(const char *Cmd) {

	char opt = param_getchar(Cmd, 0);
	if (strlen(Cmd)<1 || opt == 'h') {
		usage_hf_iclass_eload();
		return 0;
	}

	//File handling and reading
	FILE *f;
	char filename[FILE_PATH_SIZE];
	if (opt == 'f' && param_getstr(Cmd, 1, filename, sizeof(filename)) > 0) {
		f = fopen(filename, "rb");
	} else {
		usage_hf_iclass_eload();
		return 0;
	}

	if (!f) {
		PrintAndLog("Failed to read from file '%s'", filename);
		return 1;
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (fsize < 0) {
		PrintAndLog("Error, when getting filesize");
		fclose(f);
		return 1;
	}

	uint8_t *dump = malloc(fsize);

	size_t bytes_read = fread(dump, 1, fsize, f);
	fclose(f);

	printIclassDumpInfo(dump);
	//Validate

	if (bytes_read < fsize) {
		prnlog("Error, could only read %d bytes (should be %d)",bytes_read, fsize );
		free(dump);
		return 1;
	}
	//Send to device
	uint32_t bytes_sent = 0;
	uint32_t bytes_remaining  = bytes_read;

	while (bytes_remaining > 0) {
		uint32_t bytes_in_packet = MIN(USB_CMD_DATA_SIZE, bytes_remaining);
		UsbCommand c = {CMD_ICLASS_EML_MEMSET, {bytes_sent,bytes_in_packet,0}};
		memcpy(c.d.asBytes, dump+bytes_sent, bytes_in_packet);
		SendCommand(&c);
		bytes_remaining -= bytes_in_packet;
		bytes_sent += bytes_in_packet;
	}
	free(dump);
	PrintAndLog("Sent %d bytes of data to device emulator memory", bytes_sent);
	return 0;
}


static int readKeyfile(const char *filename, size_t len, uint8_t* buffer) {
	FILE *f = fopen(filename, "rb");
	if(!f) {
		PrintAndLog("Failed to read from file '%s'", filename);
		return 1;
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	size_t bytes_read = fread(buffer, 1, len, f);
	fclose(f);
	if(fsize != len)
	{
		PrintAndLog("Warning, file size is %d, expected %d", fsize, len);
		return 1;
	}
	if(bytes_read != len)
	{
		PrintAndLog("Warning, could only read %d bytes, expected %d" ,bytes_read, len);
		return 1;
	}
	return 0;
}


static void usage_hf_iclass_decrypt(void) {
	PrintAndLog("Usage: hf iclass decrypt f <tagdump>");
	PrintAndLog("");
	PrintAndLog("OBS! In order to use this function, the file 'iclass_decryptionkey.bin' must reside");
	PrintAndLog("in the working directory. The file should be 16 bytes binary data");
	PrintAndLog("");
	PrintAndLog("example: hf iclass decrypt f tagdump_12312342343.bin");
	PrintAndLog("");
	PrintAndLog("OBS! This is pretty stupid implementation, it tries to decrypt every block after block 6. ");
	PrintAndLog("Correct behaviour would be to decrypt only the application areas where the key is valid,");
	PrintAndLog("which is defined by the configuration block.");
}


static int CmdHFiClassDecrypt(const char *Cmd) {
	uint8_t key[16] = { 0 };
	if(readKeyfile("iclass_decryptionkey.bin", 16, key))
	{
		usage_hf_iclass_decrypt();
		return 1;
	}
	PrintAndLog("Decryption file found... ");
	char opt = param_getchar(Cmd, 0);
	if (strlen(Cmd)<1 || opt == 'h') {
		usage_hf_iclass_decrypt();
		return 0;
	}

	//Open the tagdump-file
	FILE *f;
	char filename[FILE_PATH_SIZE];
	if(opt == 'f' && param_getstr(Cmd, 1, filename, sizeof(filename)) > 0) {
		f = fopen(filename, "rb");
		if ( f == NULL ) {
			PrintAndLog("Could not find file %s", filename);
			return 1;
		}
	} else {
		usage_hf_iclass_decrypt();
		return 0;
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t enc_dump[8] = {0};
	uint8_t *decrypted = malloc(fsize);
	mbedtls_des3_context ctx = { {0} };
	mbedtls_des3_set2key_dec( &ctx, key);
	size_t bytes_read = fread(enc_dump, 1, 8, f);

	//Use the first block (CSN) for filename
	char outfilename[FILE_PATH_SIZE] = { 0 };
	snprintf(outfilename,FILE_PATH_SIZE,"iclass_tagdump-%02x%02x%02x%02x%02x%02x%02x%02x-decrypted",
			 enc_dump[0],enc_dump[1],enc_dump[2],enc_dump[3],
			 enc_dump[4],enc_dump[5],enc_dump[6],enc_dump[7]);

	size_t blocknum =0;
	while(bytes_read == 8)
	{
		if(blocknum < 7)
		{
			memcpy(decrypted+(blocknum*8), enc_dump, 8);
		}else{
			mbedtls_des3_crypt_ecb(&ctx, enc_dump,decrypted +(blocknum*8) );
		}
		printvar("decrypted block", decrypted +(blocknum*8), 8);
		bytes_read = fread(enc_dump, 1, 8, f);
		blocknum++;
	}
	fclose(f);

	saveFile(outfilename,"bin", decrypted, blocknum*8);
	free(decrypted);
	return 0;
}


static void usage_hf_iclass_encrypt(void) {
	PrintAndLog("Usage: hf iclass encrypt <BlockData>");
	PrintAndLog("");
	PrintAndLog("OBS! In order to use this function, the file 'iclass_decryptionkey.bin' must reside");
	PrintAndLog("in the working directory. The file should be 16 bytes binary data");
	PrintAndLog("");
	PrintAndLog("example: hf iclass encrypt 0102030405060708");
	PrintAndLog("");
}


static int iClassEncryptBlkData(uint8_t *blkData) {
	uint8_t key[16] = { 0 };
	if(readKeyfile("iclass_decryptionkey.bin", 16, key))
	{
		usage_hf_iclass_encrypt();
		return 1;
	}
	PrintAndLog("Decryption file found... ");

	uint8_t encryptedData[16];
	uint8_t *encrypted = encryptedData;
	mbedtls_des3_context ctx = { {0} };
	mbedtls_des3_set2key_enc( &ctx, key);

	mbedtls_des3_crypt_ecb(&ctx, blkData,encrypted);
	//printvar("decrypted block", decrypted, 8);
	memcpy(blkData,encrypted,8);

	return 1;
}


static int CmdHFiClassEncryptBlk(const char *Cmd) {
	uint8_t blkData[8] = {0};
	char opt = param_getchar(Cmd, 0);
	if (strlen(Cmd)<1 || opt == 'h') {
		usage_hf_iclass_encrypt();
		return 0;
	}
	//get the bytes to encrypt
	if (param_gethex(Cmd, 0, blkData, 16)) {
		PrintAndLog("BlockData must include 16 HEX symbols");
		return 0;
	}
	if (!iClassEncryptBlkData(blkData)) return 0;
	printvar("encrypted block", blkData, 8);
	return 1;
}


static void Calc_wb_mac(uint8_t blockno, uint8_t *data, uint8_t *div_key, uint8_t MAC[4]) {
	uint8_t WB[9];
	WB[0] = blockno;
	memcpy(WB+1, data, 8);
	doMAC_N(WB, sizeof(WB), div_key, MAC);
	//printf("Cal wb mac block [%02x][%02x%02x%02x%02x%02x%02x%02x%02x] : MAC [%02x%02x%02x%02x]",WB[0],WB[1],WB[2],WB[3],WB[4],WB[5],WB[6],WB[7],WB[8],MAC[0],MAC[1],MAC[2],MAC[3]);
}


static bool iClass_select(uint8_t *CSN, bool verbose, bool cleartrace, bool init) {

	UsbCommand c = {CMD_READER_ICLASS, {FLAG_ICLASS_READER_CSN}};
	if (init) c.arg[0] |= FLAG_ICLASS_READER_INIT;
	if (cleartrace) c.arg[0] |= FLAG_ICLASS_READER_CLEARTRACE;

	UsbCommand resp;
	clearCommandBuffer();
	SendCommand(&c);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
		PrintAndLog("Command execute timeout");
		return false;
	}

	uint8_t isOK = resp.arg[0] & 0xff;
	uint8_t *data = resp.d.asBytes;

	if (isOK & FLAG_ICLASS_READER_CSN) {
		memcpy(CSN, data, 8);
		if (verbose) PrintAndLog("CSN: %s", sprint_hex(CSN, 8));
	} else {
		PrintAndLog("Failed to select card! Aborting");
		return false;
	}
	return true;
}


static void HFiClassCalcDivKey(uint8_t *CSN, uint8_t *KEY, uint8_t *div_key, bool elite){
	uint8_t keytable[128] = {0};
	uint8_t key_index[8] = {0};
	if (elite) {
		uint8_t key_sel[8] = { 0 };
		uint8_t key_sel_p[8] = { 0 };
		hash2(KEY, keytable);
		hash1(CSN, key_index);
		for(uint8_t i = 0; i < 8 ; i++)
			key_sel[i] = keytable[key_index[i]] & 0xFF;

		//Permute from iclass format to standard format
		permutekey_rev(key_sel, key_sel_p);
		diversifyKey(CSN, key_sel_p, div_key);
	} else {
		diversifyKey(CSN, KEY, div_key);
	}
}


static bool iClass_authenticate(uint8_t *CSN, uint8_t *KEY, uint8_t *MAC, uint8_t *div_key, bool use_credit_key, bool elite, bool rawkey, bool replay, bool verbose) {

	//get div_key
	if (rawkey || replay)
		memcpy(div_key, KEY, 8);
	else
		HFiClassCalcDivKey(CSN, KEY, div_key, elite);

	char keytypetext[23] = "legacy diversified key";
	if (rawkey) {
		strcpy(keytypetext, "raw key");
	} else if (replay) {
		strcpy(keytypetext, "replayed NR/MAC");
	} else if (elite) {
		strcpy(keytypetext, "Elite diversified key");
	}

	if (verbose) PrintAndLog("Authenticating with %s: %s", keytypetext, sprint_hex(div_key, 8));

	UsbCommand resp;
	UsbCommand d = {CMD_ICLASS_READCHECK, {2, use_credit_key, 0}};

	clearCommandBuffer();
	SendCommand(&d);

	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
		if (verbose) PrintAndLog("Auth Command (READCHECK[2]) execute timeout");
		return false;
	}
	bool isOK = resp.arg[0];
	if (!isOK) {
		if (verbose) PrintAndLog("Couldn't get Card Challenge");
		return false;
	}

	if (replay) {
		memcpy(MAC, KEY+4, 4);
	} else {
		uint8_t CCNR[12];
		memcpy(CCNR, resp.d.asBytes, 8);
		memset(CCNR+8, 0x00, 4); // default NR = {0, 0, 0, 0}
		doMAC(CCNR, div_key, MAC);
	}

	d.cmd = CMD_ICLASS_CHECK;
	if (replay) {
		memcpy(d.d.asBytes, KEY, 8);
	} else {
		memset(d.d.asBytes, 0x00, 4); // default NR = {0, 0, 0, 0}
		memcpy(d.d.asBytes+4, MAC, 4);
	}
	clearCommandBuffer();
	SendCommand(&d);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
		if (verbose) PrintAndLog("Auth Command (CHECK) execute timeout");
		return false;
	}
	isOK = resp.arg[0];
	if (!isOK) {
		if (verbose) PrintAndLog("Authentication error");
		return false;
	}
	return true;
}


static void usage_hf_iclass_dump(void) {
	PrintAndLog("Usage:  hf iclass dump f <fileName> k <Key> c <CreditKey> e|r|n\n");
	PrintAndLog("Options:");
	PrintAndLog("  f <filename> : specify a filename to save dump to");
	PrintAndLog("  k <Key>      : *Debit Key (AA1) as 16 hex symbols (8 bytes) or 1 hex to select key from memory");
	PrintAndLog("  c <CreditKey>: Credit Key (AA2) as 16 hex symbols (8 bytes) or 1 hex to select key from memory");
	PrintAndLog("  e            : If 'e' is specified, the keys are interpreted as Elite");
	PrintAndLog("                 Custom Keys (KCus), which can be obtained via reader-attack");
	PrintAndLog("                 See 'hf iclass sim 2'. This key should be on iclass-format");
	PrintAndLog("  r            : If 'r' is specified, keys are interpreted as raw blocks 3/4");
	PrintAndLog("  n            : If 'n' is specified, keys are interpreted as NR/MAC pairs which can be obtained by 'hf iclass snoop'");
	PrintAndLog("  NOTE: * = required");
	PrintAndLog("Samples:");
	PrintAndLog("  hf iclass dump k 001122334455667B");
	PrintAndLog("  hf iclass dump k AAAAAAAAAAAAAAAA c 001122334455667B");
	PrintAndLog("  hf iclass dump k AAAAAAAAAAAAAAAA e");
}


static void printIclassDumpContents(uint8_t *iclass_dump, uint8_t startblock, uint8_t endblock, size_t filesize) {
	uint8_t mem_config;
	memcpy(&mem_config, iclass_dump + 13,1);
	uint8_t maxmemcount;
	uint8_t filemaxblock = filesize / 8;
	if (mem_config & 0x80)
		maxmemcount = 255;
	else
		maxmemcount = 31;
	//PrintAndLog   ("endblock: %d, filesize: %d, maxmemcount: %d, filemaxblock: %d", endblock,filesize, maxmemcount, filemaxblock);

	if (startblock == 0)
		startblock = 6;
	if ((endblock > maxmemcount) || (endblock == 0))
		endblock = maxmemcount;

	// remember endblock need to relate to zero-index arrays.
	if (endblock > filemaxblock-1)
		endblock = filemaxblock;

	int i = startblock;
	printf("------+--+-------------------------+\n");
	while (i <= endblock) {
		uint8_t *blk = iclass_dump + (i * 8);
		printf("Block |%02X| %s|\n", i, sprint_hex(blk, 8) );
		i++;
	}
	printf("------+--+-------------------------+\n");
}


static int CmdHFiClassReader_Dump(const char *Cmd) {

	uint8_t MAC[4] = {0x00,0x00,0x00,0x00};
	uint8_t div_key[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t c_div_key[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t blockno = 0;
	uint8_t AA1_maxBlk = 0;
	uint8_t maxBlk = 31;
	uint8_t app_areas = 1;
	uint8_t kb = 2;
	uint8_t KEY[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t CreditKEY[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t keyNbr = 0;
	uint8_t dataLen = 0;
	uint8_t fileNameLen = 0;
	char filename[FILE_PATH_SIZE]={0};
	char tempStr[50] = {0};
	bool have_debit_key = false;
	bool have_credit_key = false;
	bool use_credit_key = false;
	bool elite = false;
	bool rawkey = false;
	bool NRMAC_replay = false;
	bool errors = false;
	bool verbose = false;
	uint8_t cmdp = 0;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch(param_getchar(Cmd, cmdp)) {
			case 'h':
			case 'H':
				usage_hf_iclass_dump();
				return 0;
			case 'c':
			case 'C':
				have_credit_key = true;
				dataLen = param_getstr(Cmd, cmdp+1, tempStr, sizeof(tempStr));
				if (dataLen == 16) {
					errors = param_gethex(tempStr, 0, CreditKEY, dataLen);
				} else if (dataLen == 1) {
					keyNbr = param_get8(Cmd, cmdp+1);
					if (keyNbr < ICLASS_KEYS_MAX) {
						memcpy(CreditKEY, iClass_Key_Table[keyNbr], 8);
					} else {
						PrintAndLog("\nERROR: Credit KeyNbr is invalid\n");
						errors = true;
					}
				} else {
					PrintAndLog("\nERROR: Credit Key is incorrect length\n");
					errors = true;
				}
				cmdp += 2;
				break;
			case 'e':
			case 'E':
				elite = true;
				cmdp++;
				break;
			case 'f':
			case 'F':
				fileNameLen = param_getstr(Cmd, cmdp+1, filename, sizeof(filename));
				if (fileNameLen < 1) {
					PrintAndLog("No filename found after f");
					errors = true;
				}
				cmdp += 2;
				break;
			case 'k':
			case 'K':
				have_debit_key = true;
				dataLen = param_getstr(Cmd, cmdp+1, tempStr, sizeof(tempStr));
				if (dataLen == 16) {
					errors = param_gethex(tempStr, 0, KEY, dataLen);
				} else if (dataLen == 1) {
					keyNbr = param_get8(Cmd, cmdp+1);
					if (keyNbr < ICLASS_KEYS_MAX) {
						memcpy(KEY, iClass_Key_Table[keyNbr], 8);
					} else {
						PrintAndLog("\nERROR: Debit KeyNbr is invalid\n");
						errors = true;
					}
				} else {
					PrintAndLog("\nERROR: Debit Key is incorrect length\n");
					errors = true;
				}
				cmdp += 2;
				break;
			case 'r':
			case 'R':
				rawkey = true;
				cmdp++;
				break;
			case 'n':
			case 'N':
				NRMAC_replay = true;
				cmdp++;
				break;
			case 'v':
			case 'V':
				verbose = true;
				cmdp++;
				break;
			default:
				PrintAndLog("Unknown parameter '%c'\n", param_getchar(Cmd, cmdp));
				errors = true;
				break;
		}
	}

	if (elite + rawkey + NRMAC_replay > 1) {
		PrintAndLog("You cannot combine the 'e', 'r', and 'n' options\n");
		errors = true;
	}

	if (errors || cmdp < 2) {
		usage_hf_iclass_dump();
		return 0;
	}

	// if only credit key is given: try for AA1 as well (not for iclass but for some picopass this will work)
	if (!have_debit_key && have_credit_key) {
		memcpy(KEY, CreditKEY, 8);
	}

	// clear trace and get first 3 blocks
	UsbCommand c = {CMD_READER_ICLASS, {FLAG_ICLASS_READER_INIT | FLAG_ICLASS_READER_CLEARTRACE | FLAG_ICLASS_READER_CSN | FLAG_ICLASS_READER_CONF | FLAG_ICLASS_READER_CC}};
	UsbCommand resp;
	uint8_t tag_data[256*8];

	clearCommandBuffer();
	SendCommand(&c);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
		PrintAndLog("Command execute timeout");
		DropField();
		return 0;
	}

	uint8_t readStatus = resp.arg[0] & 0xff;
	uint8_t *data = resp.d.asBytes;
	uint8_t status_mask = FLAG_ICLASS_READER_CSN | FLAG_ICLASS_READER_CONF | FLAG_ICLASS_READER_CC;

	if (readStatus != status_mask) {
		PrintAndLog("No tag found ...");
		return 0;
	} else {
		memcpy(tag_data, data, 8*3);
		if (verbose) PrintAndLog("CSN: %s", sprint_hex(tag_data, 8));
		AA1_maxBlk = data[8];
		getMemConfig(data[13], data[12], &maxBlk, &app_areas, &kb);
		// large memory - not able to dump pages currently
		if (AA1_maxBlk > maxBlk) AA1_maxBlk = maxBlk;
	}

	// authenticate with debit key (or credit key if we have no debit key) and get div_key - later store in dump block 3
	if (!iClass_authenticate(tag_data, KEY, MAC, div_key, false, elite, rawkey, NRMAC_replay, verbose)) {
		DropField();
		return 0;
	}

	// read AA1
	UsbCommand w = {CMD_ICLASS_DUMP};
	uint32_t blocksRead = 0;
	for (blockno = 3; blockno <= AA1_maxBlk; blockno += blocksRead) {
		w.arg[0] = blockno;
		w.arg[1] = AA1_maxBlk - blockno + 1;
		clearCommandBuffer();
		SendCommand(&w);
		if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
			PrintAndLog("Command execute time-out 1");
			DropField();
			return 1;
		}
		blocksRead = resp.arg[1];
		bool isOK = resp.arg[0];
		if (!isOK) {
			PrintAndLog("Reading AA1 block failed");
			DropField();
			return 0;
		}
		memcpy(tag_data + blockno*8, resp.d.asBytes, blocksRead*8);
	}

	// do we still need to read more blocks (AA2 enabled)?
	if (have_credit_key && maxBlk > AA1_maxBlk) {
		if (!use_credit_key) {
			//turn off hf field before authenticating with different key
			DropField();
			// AA2 authenticate credit key and git c_div_key - later store in dump block 4
			uint8_t CSN[8];
			if (!iClass_select(CSN, verbose, false, true) || !iClass_authenticate(CSN, CreditKEY, MAC, c_div_key, true, false, false, NRMAC_replay, verbose)){
				DropField();
				return 0;
			}
		}
		for ( ; blockno <= maxBlk; blockno += blocksRead) {
			w.arg[0] = blockno;
			w.arg[1] = maxBlk - blockno + 1;
			clearCommandBuffer();
			SendCommand(&w);
			if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
				PrintAndLog("Command execute time-out 1");
				DropField();
				return 1;
			}
			blocksRead = resp.arg[1];
			bool isOK = resp.arg[0];
			if (!isOK) {
				PrintAndLog("Reading AA2 block failed");
				DropField();
				return 0;
			}
			memcpy(tag_data + blockno*8, resp.d.asBytes, blocksRead*8);
		}
	}

	DropField();

	// add diversified keys to dump
	if (have_debit_key) {
		memcpy(tag_data + 3*8, div_key, 8);
	} else {
		memset(tag_data + 3*8, 0xff, 8);
	}
	if (have_credit_key) {
		memcpy(tag_data + 4*8, c_div_key, 8);
	} else {
		memset(tag_data + 4*8, 0xff, 8);
	}

	// print the dump
	printf("------+--+-------------------------+\n");
	printf("CSN   |00| %s|\n",sprint_hex(tag_data, 8));
	printIclassDumpContents(tag_data, 1, blockno-1, blockno*8);

	if (filename[0] == 0) {
		snprintf(filename, FILE_PATH_SIZE,"iclass_tagdump-%02x%02x%02x%02x%02x%02x%02x%02x",
			tag_data[0],tag_data[1],tag_data[2],tag_data[3],
			tag_data[4],tag_data[5],tag_data[6],tag_data[7]);
	}

	// save the dump to .bin file
	PrintAndLog("Saving dump file - %d blocks read", blockno);
	saveFile(filename, "bin", tag_data, blockno*8);
	return 1;
}


static int WriteBlock(uint8_t blockno, uint8_t *bldata, uint8_t *KEY, bool use_credit_key, bool elite, bool rawkey, bool NRMAC_replay, bool verbose) {

	uint8_t MAC[4] = {0x00,0x00,0x00,0x00};
	uint8_t div_key[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t CSN[8];

	if (!iClass_select(CSN, verbose, true, true) || !iClass_authenticate(CSN, KEY, MAC, div_key, use_credit_key, elite, rawkey, NRMAC_replay, verbose)) {
		DropField();
		return 0;
	}

	UsbCommand resp;

	Calc_wb_mac(blockno, bldata, div_key, MAC);

	UsbCommand w = {CMD_ICLASS_WRITEBLOCK, {blockno}};
	memcpy(w.d.asBytes, bldata, 8);
	memcpy(w.d.asBytes + 8, MAC, 4);

	clearCommandBuffer();
	SendCommand(&w);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
		PrintAndLog("Write Command execute timeout");
		DropField();
		return 0;
	}
	bool isOK = resp.arg[0];
	if (!isOK) {
		PrintAndLog("Write Block Failed");
		DropField();
		return 0;
	}

	PrintAndLog("Write Block Successful");
	return 1;
}


static void usage_hf_iclass_writeblock(void) {
	PrintAndLog("Options:");
	PrintAndLog("  b <Block> : The block number as 2 hex symbols");
	PrintAndLog("  d <data>  : Set the Data to write as 16 hex symbols");
	PrintAndLog("  k <Key>   : Access Key as 16 hex symbols or 1 hex to select key from memory");
	PrintAndLog("  c         : If 'c' is specified, the key set is assumed to be the credit key\n");
	PrintAndLog("  e         : If 'e' is specified, elite computations applied to key");
	PrintAndLog("  r         : If 'r' is specified, no computations applied to key");
	PrintAndLog("  o         : override protection and allow modification of blocks 0...4");
	PrintAndLog("Samples:");
	PrintAndLog("  hf iclass writeblk b 0A d AAAAAAAAAAAAAAAA k 001122334455667B");
	PrintAndLog("  hf iclass writeblk b 1B d AAAAAAAAAAAAAAAA k 001122334455667B c");
	PrintAndLog("  hf iclass writeblk b 03 d AAAAAAAAAAAAAAAA k 001122334455667B c o");
}


static int CmdHFiClass_WriteBlock(const char *Cmd) {
	uint8_t blockno = 0;
	uint8_t bldata[8] = {0};
	uint8_t KEY[8] = {0};
	uint8_t keyNbr = 0;
	uint8_t dataLen = 0;
	char tempStr[50] = {0};
	bool use_credit_key = false;
	bool elite = false;
	bool rawkey = false;
	bool override_protection = false;
	bool errors = false;
	uint8_t cmdp = 0;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch(param_getchar(Cmd, cmdp)) {
		case 'h':
		case 'H':
			usage_hf_iclass_writeblock();
			return 0;
		case 'b':
		case 'B':
			if (param_gethex(Cmd, cmdp+1, &blockno, 2)) {
				PrintAndLog("Block No must include 2 HEX symbols\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'c':
		case 'C':
			use_credit_key = true;
			cmdp++;
			break;
		case 'd':
		case 'D':
			if (param_gethex(Cmd, cmdp+1, bldata, 16)) {
				PrintAndLog("Data must include 16 HEX symbols\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'e':
		case 'E':
			elite = true;
			cmdp++;
			break;
		case 'k':
		case 'K':
			dataLen = param_getstr(Cmd, cmdp+1, tempStr, sizeof(tempStr));
			if (dataLen == 16) {
				errors = param_gethex(tempStr, 0, KEY, dataLen);
			} else if (dataLen == 1) {
				keyNbr = param_get8(Cmd, cmdp+1);
				if (keyNbr < ICLASS_KEYS_MAX) {
					memcpy(KEY, iClass_Key_Table[keyNbr], 8);
				} else {
					PrintAndLog("\nERROR: Credit KeyNbr is invalid\n");
					errors = true;
				}
			} else {
				PrintAndLog("\nERROR: Credit Key is incorrect length\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'r':
		case 'R':
			rawkey = true;
			cmdp++;
			break;
		case 'o':
		case 'O':
			override_protection = true;
			cmdp++;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'\n", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if (errors) {
			usage_hf_iclass_writeblock();
			return 0;
		}
	}

	if (elite && rawkey) {
		PrintAndLog("You cannot combine the 'e' and 'r' options\n");
		errors = true;
	}

	if (cmdp < 6) {
		usage_hf_iclass_writeblock();
		return 0;
	}

	if (blockno < 5) {
		if (override_protection) {
			PrintAndLog("Info: modifying keys, e-purse or configuration block.");
		} else {
			PrintAndLog("You are going to modify keys, e-purse or configuration block.");
			PrintAndLog("You must add the 'o' (override) option to confirm that you know what you are doing");
			return 0;
		}
	}

	int ans = WriteBlock(blockno, bldata, KEY, use_credit_key, elite, rawkey, false, true);

	DropField();
	return ans;
}


static void usage_hf_iclass_clone(void) {
	PrintAndLog("Usage:  hf iclass clone f <tagfile.bin> b <first block> l <last block> k <KEY> c e|r o");
	PrintAndLog("Options:");
	PrintAndLog("  f <filename>: specify a filename to clone from");
	PrintAndLog("  b <Block>   : The first block to clone as 2 hex symbols");
	PrintAndLog("  l <Last Blk>: The last block to clone as 2 hex symbols");
	PrintAndLog("  k <Key>     : Access Key as 16 hex symbols or 1 hex to select key from memory");
	PrintAndLog("  c           : If 'c' is specified, the key set is assumed to be the credit key\n");
	PrintAndLog("  e           : If 'e' is specified, elite computations applied to key");
	PrintAndLog("  r           : If 'r' is specified, no computations applied to key");
	PrintAndLog("  o           : override protection and allow modification of target blocks 0...4");
	PrintAndLog("Samples:");
	PrintAndLog("  hf iclass clone f iclass_tagdump-121345.bin b 06 l 1A k 1122334455667788 e");
	PrintAndLog("  hf iclass clone f iclass_tagdump-121345.bin b 05 l 19 k 0");
	PrintAndLog("  hf iclass clone f iclass_tagdump-121345.bin b 06 l 19 k 0 e");
	PrintAndLog("  hf iclass clone f iclass_tagdump-121345.bin b 06 l 19 k 0 e");
	PrintAndLog("  hf iclass clone f iclass_tagdump-121345.bin b 03 l 19 k 0 e o");
}


static int CmdHFiClassCloneTag(const char *Cmd) {
	char filename[FILE_PATH_SIZE] = {0};
	char tempStr[50]={0};
	uint8_t KEY[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t keyNbr = 0;
	uint8_t fileNameLen = 0;
	uint8_t startblock = 0;
	uint8_t endblock = 0;
	uint8_t dataLen = 0;
	bool use_credit_key = false;
	bool elite = false;
	bool rawkey = false;
	bool override_protection = false;
	bool errors = false;
	uint8_t cmdp = 0;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (param_getchar(Cmd, cmdp)) {
		case 'h':
		case 'H':
			usage_hf_iclass_clone();
			return 0;
		case 'b':
		case 'B':
			if (param_gethex(Cmd, cmdp+1, &startblock, 2)) {
				PrintAndLog("Start Block No must include 2 HEX symbols\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'c':
		case 'C':
			use_credit_key = true;
			cmdp++;
			break;
		case 'e':
		case 'E':
			elite = true;
			cmdp++;
			break;
		case 'f':
		case 'F':
			fileNameLen = param_getstr(Cmd, cmdp+1, filename, sizeof(filename));
			if (fileNameLen < 1) {
				PrintAndLog("No filename found after f");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'k':
		case 'K':
			dataLen = param_getstr(Cmd, cmdp+1, tempStr, sizeof(tempStr));
			if (dataLen == 16) {
				errors = param_gethex(tempStr, 0, KEY, dataLen);
			} else if (dataLen == 1) {
				keyNbr = param_get8(Cmd, cmdp+1);
				if (keyNbr < ICLASS_KEYS_MAX) {
					memcpy(KEY, iClass_Key_Table[keyNbr], 8);
				} else {
					PrintAndLog("\nERROR: Credit KeyNbr is invalid\n");
					errors = true;
				}
			} else {
				PrintAndLog("\nERROR: Credit Key is incorrect length\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'l':
		case 'L':
			if (param_gethex(Cmd, cmdp+1, &endblock, 2)) {
				PrintAndLog("Last Block No must include 2 HEX symbols\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'r':
		case 'R':
			rawkey = true;
			cmdp++;
			break;
		case 'o':
		case 'O':
			override_protection = true;
			cmdp++;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'\n", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if (errors) {
			usage_hf_iclass_clone();
			return 0;
		}
	}

	if (cmdp < 8) {
		usage_hf_iclass_clone();
		return 0;
	}

	if (startblock < 5) {
		if (override_protection) {
			PrintAndLog("Info: modifying keys, e-purse or configuration block.");
		} else {
			PrintAndLog("You are going to modify keys, e-purse or configuration block.");
			PrintAndLog("You must add the 'o' (override) option to confirm that you know what you are doing");
			return 0;
		}
	}
	
	if ((endblock - startblock + 1) * 12 > USB_CMD_DATA_SIZE) {
		PrintAndLog("Trying to write too many blocks at once.  Max: %d", USB_CMD_DATA_SIZE/12);
	}

	// file handling and reading
	FILE *f;
	f = fopen(filename,"rb");
	if (!f) {
		PrintAndLog("Failed to read from file '%s'", filename);
		return 1;
	}

	uint8_t tag_data[USB_CMD_DATA_SIZE/12][8];
	fseek(f, startblock*8, SEEK_SET);
	for (int i = 0; i < endblock - startblock + 1; i++) {
		if (fread(&tag_data[i], 1, 8, f) == 0 ) {
			PrintAndLog("File reading error.");
			fclose(f);
			return 2;
		}
	}

	uint8_t MAC[4] = {0x00, 0x00, 0x00, 0x00};
	uint8_t div_key[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t CSN[8];

	if (!iClass_select(CSN, true, true, true) || !iClass_authenticate(CSN, KEY, MAC, div_key, use_credit_key, elite, rawkey, false, true)) {
		DropField();
		return 0;
	}

	UsbCommand w = {CMD_ICLASS_CLONE, {startblock, endblock}};
	uint8_t *ptr;
	// calculate MAC for every block we will write
	for (int i = 0; i < endblock - startblock + 1; i++) {
		Calc_wb_mac(startblock + i, tag_data[i], div_key, MAC);
		ptr = w.d.asBytes + i * 12;
		memcpy(ptr, tag_data[i], 8);
		memcpy(ptr + 8, MAC, 4);
	}

	uint8_t p[12];
	PrintAndLog("Cloning");
	for (int i = 0; i < endblock - startblock + 1; i++){
		memcpy(p, w.d.asBytes + (i * 12), 12);
		PrintAndLog("Block |%02x| %02x%02x%02x%02x%02x%02x%02x%02x | MAC |%02x%02x%02x%02x|",
			i + startblock, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11]);
	}

	UsbCommand resp;
	SendCommand(&w);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
		PrintAndLog("Command execute timeout");
		DropField();
		return 0;
	}

	DropField();
	return 1;
}


static int ReadBlock(uint8_t *KEY, uint8_t blockno, uint8_t keyType, bool elite, bool rawkey, bool NRMAC_replay, bool verbose, bool auth) {

	uint8_t MAC[4]={0x00,0x00,0x00,0x00};
	uint8_t div_key[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t CSN[8];

	if (!iClass_select(CSN, verbose, true, true)) {
		DropField();
		return 0;
	}

	if (auth) {
		if (!iClass_authenticate(CSN, KEY, MAC, div_key, (keyType==0x18), elite, rawkey, NRMAC_replay, verbose)) {
			DropField();
			return 0;
		}
	}

	UsbCommand resp;
	UsbCommand w = {CMD_ICLASS_READBLOCK, {blockno}};
	clearCommandBuffer();
	SendCommand(&w);
	if (!WaitForResponseTimeout(CMD_ACK, &resp, 4500)) {
		PrintAndLog("Command execute timeout");
		DropField();
		return 0;
	}
	bool isOK = resp.arg[0];
	if (!isOK) {
		PrintAndLog("Read Block Failed");
		DropField();
		return 0;
	}
	//data read is stored in: resp.d.asBytes[0-15]
	if (verbose)
		PrintAndLog("Block %02X: %s\n",blockno, sprint_hex(resp.d.asBytes,8));

	return 1;
}


static void usage_hf_iclass_readblock(void) {
	PrintAndLog("Usage:  hf iclass readblk b <Block> k <Key> [c] [e|r|n]\n");
	PrintAndLog("Options:");
	PrintAndLog("  b <Block> : The block number as 2 hex symbols");
	PrintAndLog("  k <Key>   : Access Key as 16 hex symbols or 1 hex to select key from memory");
	PrintAndLog("  c         : If 'c' is specified, the key set is assumed to be the credit key\n");
	PrintAndLog("  e         : If 'e' is specified, elite computations applied to key");
	PrintAndLog("  r         : If 'r' is specified, no computations applied to key");
	PrintAndLog("  n         : If 'n' is specified, <Key> specifies a NR/MAC pair which can be obtained by 'hf iclass snoop'");
	PrintAndLog("Samples:");
	PrintAndLog("  hf iclass readblk b 06 k 0011223344556677");
	PrintAndLog("  hf iclass readblk b 1B k 0011223344556677 c");
	PrintAndLog("  hf iclass readblk b 0A k 0");
}


static int CmdHFiClass_ReadBlock(const char *Cmd) {
	uint8_t blockno=0;
	uint8_t keyType = 0x88; //debit key
	uint8_t KEY[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t keyNbr = 0;
	uint8_t dataLen = 0;
	char tempStr[50] = {0};
	bool elite = false;
	bool rawkey = false;
	bool NRMAC_replay = false;
	bool errors = false;
	bool auth = false;
	uint8_t cmdp = 0;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (param_getchar(Cmd, cmdp)) {
			case 'h':
			case 'H':
				usage_hf_iclass_readblock();
				return 0;
			case 'b':
			case 'B':
				if (param_gethex(Cmd, cmdp+1, &blockno, 2)) {
					PrintAndLog("Block No must include 2 HEX symbols\n");
					errors = true;
				}
				cmdp += 2;
				break;
			case 'c':
			case 'C':
				keyType = 0x18;
				cmdp++;
				break;
			case 'e':
			case 'E':
				elite = true;
				cmdp++;
				break;
			case 'k':
			case 'K':
				auth = true;
				dataLen = param_getstr(Cmd, cmdp+1, tempStr, sizeof(tempStr));
				if (dataLen == 16) {
					errors = param_gethex(tempStr, 0, KEY, dataLen);
				} else if (dataLen == 1) {
					keyNbr = param_get8(Cmd, cmdp+1);
					if (keyNbr < ICLASS_KEYS_MAX) {
						memcpy(KEY, iClass_Key_Table[keyNbr], 8);
					} else {
						PrintAndLog("\nERROR: KeyNbr is invalid\n");
						errors = true;
					}
				} else {
					PrintAndLog("\nERROR: Key is incorrect length\n");
					errors = true;
				}
				cmdp += 2;
				break;
			case 'r':
			case 'R':
				rawkey = true;
				cmdp++;
				break;
			case 'n':
			case 'N':
				NRMAC_replay = true;
				cmdp++;
				break;
			default:
				PrintAndLog("Unknown parameter '%c'\n", param_getchar(Cmd, cmdp));
				errors = true;
				break;
		}
	}

	if (elite + rawkey + NRMAC_replay > 1) {
		PrintAndLog("You cannot combine the 'e', 'r', and 'n' options\n");
		errors = true;
	}

	if (errors) {
		usage_hf_iclass_readblock();
		return 0;
	}

	if (cmdp < 2) {
		usage_hf_iclass_readblock();
		return 0;
	}
	if (!auth)
		PrintAndLog("warning: no authentication used with read, only a few specific blocks can be read accurately without authentication.");

	return ReadBlock(KEY, blockno, keyType, elite, rawkey, NRMAC_replay, true, auth);
}


static int CmdHFiClass_loclass(const char *Cmd) {
	char opt = param_getchar(Cmd, 0);

	if (strlen(Cmd)<1 || opt == 'h') {
		PrintAndLog("Usage: hf iclass loclass [options]");
		PrintAndLog("Options:");
		PrintAndLog("h             Show this help");
		PrintAndLog("t             Perform self-test");
		PrintAndLog("f <filename>  Bruteforce iclass dumpfile");
		PrintAndLog("                   An iclass dumpfile is assumed to consist of an arbitrary number of");
		PrintAndLog("                   malicious CSNs, and their protocol responses");
		PrintAndLog("                   The binary format of the file is expected to be as follows: ");
		PrintAndLog("                   <8 byte CSN><8 byte CC><4 byte NR><4 byte MAC>");
		PrintAndLog("                   <8 byte CSN><8 byte CC><4 byte NR><4 byte MAC>");
		PrintAndLog("                   <8 byte CSN><8 byte CC><4 byte NR><4 byte MAC>");
		PrintAndLog("                  ... totalling N*24 bytes");
		return 0;
	}
	char fileName[255] = {0};
	if(opt == 'f') {
		if(param_getstr(Cmd, 1, fileName, sizeof(fileName)) > 0) {
			return bruteforceFileNoKeys(fileName);
		} else {
			PrintAndLog("You must specify a filename");
		}
	} else if(opt == 't') {
		int errors = testCipherUtils();
		errors += testMAC();
		errors += doKeyTests(0);
		errors += testElite();
		if(errors) {
			prnlog("OBS! There were errors!!!");
		}
		return errors;
	}

	return 0;
}


static void usage_hf_iclass_readtagfile() {
	PrintAndLog("Usage: hf iclass readtagfile <filename> [startblock] [endblock]");
}


static int CmdHFiClassReadTagFile(const char *Cmd) {
	int startblock = 0;
	int endblock = 0;
	char tempnum[5];
	FILE *f;
	char filename[FILE_PATH_SIZE];
	if (param_getstr(Cmd, 0, filename, sizeof(filename)) < 1) {
		usage_hf_iclass_readtagfile();
		return 0;
	}
	if (param_getstr(Cmd, 1, tempnum, sizeof(tempnum)) < 1)
		startblock = 0;
	else
		sscanf(tempnum,"%d",&startblock);

	if (param_getstr(Cmd,2, tempnum, sizeof(tempnum)) < 1)
		endblock = 0;
	else
		sscanf(tempnum,"%d",&endblock);
	// file handling and reading
	f = fopen(filename,"rb");
	if(!f) {
		PrintAndLog("Failed to read from file '%s'", filename);
		return 1;
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if ( fsize < 0 ) {
		PrintAndLog("Error, when getting filesize");
		fclose(f);
		return 1;
	}

	uint8_t *dump = malloc(fsize);

	size_t bytes_read = fread(dump, 1, fsize, f);
	fclose(f);
	uint8_t *csn = dump;
	printf("------+--+-------------------------+\n");
	printf("CSN   |00| %s|\n", sprint_hex(csn, 8) );
	//    printIclassDumpInfo(dump);
	printIclassDumpContents(dump,startblock,endblock,bytes_read);
	free(dump);
	return 0;
}

/*
uint64_t xorcheck(uint64_t sdiv,uint64_t hdiv) {
	uint64_t new_div = 0x00;
	new_div ^= sdiv;
	new_div ^= hdiv;
	return new_div;
}

uint64_t hexarray_to_uint64(uint8_t *key) {
	char temp[17];
	uint64_t uint_key;
	for (int i = 0;i < 8;i++)
		sprintf(&temp[(i *2)],"%02X",key[i]);
	temp[16] = '\0';
	if (sscanf(temp,"%016" SCNx64,&uint_key) < 1)
		return 0;
	return uint_key;
}
*/


//when told CSN, oldkey, newkey, if new key is elite (elite), and if old key was elite (oldElite)
//calculate and return xor_div_key (ready for a key write command)
//print all div_keys if verbose
static void HFiClassCalcNewKey(uint8_t *CSN, uint8_t *OLDKEY, uint8_t *NEWKEY, uint8_t *xor_div_key, bool elite, bool oldElite, bool verbose){
	uint8_t old_div_key[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t new_div_key[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	//get old div key
	HFiClassCalcDivKey(CSN, OLDKEY, old_div_key, oldElite);
	//get new div key
	HFiClassCalcDivKey(CSN, NEWKEY, new_div_key, elite);

	for (uint8_t i = 0; i < sizeof(old_div_key); i++){
		xor_div_key[i] = old_div_key[i] ^ new_div_key[i];
	}
	if (verbose) {
		printf("Old Div Key : %s\n",sprint_hex(old_div_key,8));
		printf("New Div Key : %s\n",sprint_hex(new_div_key,8));
		printf("Xor Div Key : %s\n",sprint_hex(xor_div_key,8));
	}
}


static void usage_hf_iclass_calc_newkey(void) {
	PrintAndLog("HELP :  Manage iClass Keys in client memory:\n");
	PrintAndLog("Usage:  hf iclass calc_newkey o <Old key> n <New key> s [csn] e");
	PrintAndLog("  Options:");
	PrintAndLog("  o <oldkey> : *specify a key as 16 hex symbols or a key number as 1 symbol");
	PrintAndLog("  n <newkey> : *specify a key as 16 hex symbols or a key number as 1 symbol");
	PrintAndLog("  s <csn>    : specify a card Serial number to diversify the key (if omitted will attempt to read a csn)");
	PrintAndLog("  e          : specify new key as elite calc");
	PrintAndLog("  ee         : specify old and new key as elite calc");
	PrintAndLog("Samples:");
	PrintAndLog(" e key to e key given csn : hf iclass calcnewkey o 1122334455667788 n 2233445566778899 s deadbeafdeadbeaf ee");
	PrintAndLog(" std key to e key read csn: hf iclass calcnewkey o 1122334455667788 n 2233445566778899 e");
	PrintAndLog(" std to std read csn      : hf iclass calcnewkey o 1122334455667788 n 2233445566778899");
	PrintAndLog("NOTE: * = required\n");
}


static int CmdHFiClassCalcNewKey(const char *Cmd) {
	uint8_t OLDKEY[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t NEWKEY[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t xor_div_key[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t CSN[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t keyNbr = 0;
	uint8_t dataLen = 0;
	char tempStr[50] = {0};
	bool givenCSN = false;
	bool oldElite = false;
	bool elite = false;
	bool errors = false;
	uint8_t cmdp = 0;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch(param_getchar(Cmd, cmdp)) {
		case 'h':
		case 'H':
			usage_hf_iclass_calc_newkey();
			return 0;
		case 'e':
		case 'E':
			dataLen = param_getstr(Cmd, cmdp, tempStr, sizeof(tempStr));
			if (dataLen==2)
				oldElite = true;
			elite = true;
			cmdp++;
			break;
		case 'n':
		case 'N':
			dataLen = param_getstr(Cmd, cmdp+1, tempStr, sizeof(tempStr));
			if (dataLen == 16) {
				errors = param_gethex(tempStr, 0, NEWKEY, dataLen);
			} else if (dataLen == 1) {
				keyNbr = param_get8(Cmd, cmdp+1);
				if (keyNbr < ICLASS_KEYS_MAX) {
					memcpy(NEWKEY, iClass_Key_Table[keyNbr], 8);
				} else {
					PrintAndLog("\nERROR: NewKey Nbr is invalid\n");
					errors = true;
				}
			} else {
				PrintAndLog("\nERROR: NewKey is incorrect length\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'o':
		case 'O':
			dataLen = param_getstr(Cmd, cmdp+1, tempStr, sizeof(tempStr));
			if (dataLen == 16) {
				errors = param_gethex(tempStr, 0, OLDKEY, dataLen);
			} else if (dataLen == 1) {
				keyNbr = param_get8(Cmd, cmdp+1);
				if (keyNbr < ICLASS_KEYS_MAX) {
					memcpy(OLDKEY, iClass_Key_Table[keyNbr], 8);
				} else {
					PrintAndLog("\nERROR: Credit KeyNbr is invalid\n");
					errors = true;
				}
			} else {
				PrintAndLog("\nERROR: Credit Key is incorrect length\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 's':
		case 'S':
			givenCSN = true;
			if (param_gethex(Cmd, cmdp+1, CSN, 16)) {
				usage_hf_iclass_calc_newkey();
				return 0;
			}
			cmdp += 2;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'\n", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if (errors) {
			usage_hf_iclass_calc_newkey();
			return 0;
		}
	}

	if (cmdp < 4) {
		usage_hf_iclass_calc_newkey();
		return 0;
	}

	if (!givenCSN)
		if (!iClass_select(CSN, true, true, true)) {
			DropField();
			return 0;
		}
	DropField();

	HFiClassCalcNewKey(CSN, OLDKEY, NEWKEY, xor_div_key, elite, oldElite, true);
	return 0;
}


static int loadKeys(char *filename) {
	FILE *f;
	f = fopen(filename,"rb");
	if(!f) {
		PrintAndLog("Failed to read from file '%s'", filename);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if ( fsize < 0 ) {
		PrintAndLog("Error, when getting filesize");
		fclose(f);
		return 1;
	}

	uint8_t *dump = malloc(fsize);

	size_t bytes_read = fread(dump, 1, fsize, f);
	fclose(f);
	if (bytes_read > ICLASS_KEYS_MAX * 8){
		PrintAndLog("File is too long to load - bytes: %u", bytes_read);
		free(dump);
		return 0;
	}
	uint8_t i = 0;
	for (; i < bytes_read/8; i++){
		memcpy(iClass_Key_Table[i],dump+(i*8),8);
	}
	free(dump);
	PrintAndLog("%u keys loaded", i);
	return 1;
}


static int saveKeys(char *filename) {
	FILE *f;
	f = fopen(filename,"wb");
	if (f == NULL) {
		printf("error opening file %s\n",filename);
		return 0;
	}
	for (uint8_t i = 0; i < ICLASS_KEYS_MAX; i++){
		if (fwrite(iClass_Key_Table[i],8,1,f) != 1){
			PrintAndLog("save key failed to write to file: %s", filename);
			break;
		}
	}
	fclose(f);
	return 0;
}


static int printKeys(void) {
	PrintAndLog("");
	for (uint8_t i = 0; i < ICLASS_KEYS_MAX; i++){
		PrintAndLog("%u: %s",i,sprint_hex(iClass_Key_Table[i],8));
	}
	PrintAndLog("");
	return 0;
}


static void usage_hf_iclass_managekeys(void) {
	PrintAndLog("HELP :  Manage iClass Keys in client memory:\n");
	PrintAndLog("Usage:  hf iclass managekeys n [keynbr] k [key] f [filename] s l p\n");
	PrintAndLog("  Options:");
	PrintAndLog("  n <keynbr>  : specify the keyNbr to set in memory");
	PrintAndLog("  k <key>     : set a key in memory");
	PrintAndLog("  f <filename>: specify a filename to use with load or save operations");
	PrintAndLog("  s           : save keys in memory to file specified by filename");
	PrintAndLog("  l           : load keys to memory from file specified by filename");
	PrintAndLog("  p           : print keys loaded into memory\n");
	PrintAndLog("Samples:");
	PrintAndLog(" set key      : hf iclass managekeys n 0 k 1122334455667788");
	PrintAndLog(" save key file: hf iclass managekeys f mykeys.bin s");
	PrintAndLog(" load key file: hf iclass managekeys f mykeys.bin l");
	PrintAndLog(" print keys   : hf iclass managekeys p\n");
}


static int CmdHFiClassManageKeys(const char *Cmd) {
	uint8_t keyNbr = 0;
	uint8_t dataLen = 0;
	uint8_t KEY[8] = {0};
	char filename[FILE_PATH_SIZE];
	uint8_t fileNameLen = 0;
	bool errors = false;
	uint8_t operation = 0;
	char tempStr[20];
	uint8_t cmdp = 0;

	while(param_getchar(Cmd, cmdp) != 0x00) {
		switch(param_getchar(Cmd, cmdp)) {
		case 'h':
		case 'H':
			usage_hf_iclass_managekeys();
			return 0;
		case 'f':
		case 'F':
			fileNameLen = param_getstr(Cmd, cmdp+1, filename, sizeof(filename));
			if (fileNameLen < 1) {
				PrintAndLog("No filename found after f");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'n':
		case 'N':
			keyNbr = param_get8(Cmd, cmdp+1);
			if (keyNbr >= ICLASS_KEYS_MAX) {
				PrintAndLog("Invalid block number");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'k':
		case 'K':
			operation += 3; //set key
			dataLen = param_getstr(Cmd, cmdp+1, tempStr, sizeof(tempStr));
			if (dataLen == 16) { //ul-c or ev1/ntag key length
				errors = param_gethex(tempStr, 0, KEY, dataLen);
			} else {
				PrintAndLog("\nERROR: Key is incorrect length\n");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'p':
		case 'P':
			operation += 4; //print keys in memory
			cmdp++;
			break;
		case 'l':
		case 'L':
			operation += 5; //load keys from file
			cmdp++;
			break;
		case 's':
		case 'S':
			operation += 6; //save keys to file
			cmdp++;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'\n", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
		if (errors) {
			usage_hf_iclass_managekeys();
			return 0;
		}
	}

	if (operation == 0){
		PrintAndLog("no operation specified (load, save, or print)\n");
		usage_hf_iclass_managekeys();
		return 0;
	}

	if (operation > 6){
		PrintAndLog("Too many operations specified\n");
		usage_hf_iclass_managekeys();
		return 0;
	}
	if (operation > 4 && fileNameLen == 0){
		PrintAndLog("You must enter a filename when loading or saving\n");
		usage_hf_iclass_managekeys();
		return 0;
	}

	switch (operation){
		case 3: memcpy(iClass_Key_Table[keyNbr], KEY, 8); return 1;
		case 4: return printKeys();
		case 5: return loadKeys(filename);
		case 6: return saveKeys(filename);
		break;
	}
	return 0;
}


static int CmdHFiClassCheckKeys(const char *Cmd) {

	uint8_t mac[4] = {0x00,0x00,0x00,0x00};
	uint8_t key[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint8_t div_key[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	// elite key,  raw key, standard key
	bool use_elite = false;
	bool use_raw = false;
	bool found_debit = false;
	bool found_credit = false;
	bool errors = false;
	uint8_t cmdp = 0x00;
	FILE *f;
	char filename[FILE_PATH_SIZE] = {0};
	uint8_t fileNameLen = 0;
	char buf[17];
	uint8_t *keyBlock = NULL, *p;
	int keycnt = 0;

	while (param_getchar(Cmd, cmdp) != 0x00 && !errors) {
		switch (param_getchar(Cmd, cmdp)) {
		case 'h':
		case 'H':
			usage_hf_iclass_chk();
			return 0;
		case 'f':
		case 'F':
			fileNameLen = param_getstr(Cmd, cmdp+1, filename, sizeof(filename));
			if (fileNameLen < 1) {
				PrintAndLog("No filename found after f");
				errors = true;
			}
			cmdp += 2;
			break;
		case 'e':
		case 'E':
			use_elite = true;
			cmdp++;
			break;
		case 'r':
		case 'R':
			use_raw = true;
			cmdp++;
			break;
		default:
			PrintAndLog("Unknown parameter '%c'\n", param_getchar(Cmd, cmdp));
			errors = true;
			break;
		}
	}

	if (errors) {
		usage_hf_iclass_chk();
		return 0;
	}

	if (!(f = fopen(filename , "r"))) {
		PrintAndLog("File %s not found or locked.", filename);
		return 1;
	}

	while (fgets(buf, sizeof(buf), f)) {
		if (strlen(buf) < 16 || buf[15] == '\n')
			continue;

		while (fgetc(f) != '\n' && !feof(f)) ;  //goto next line

		if (buf[0] == '#') continue; //The line start with # is comment, skip

		if (!isxdigit(buf[0])){
			PrintAndLog("File content error. '%s' must include 16 HEX symbols", buf);
			continue;
		}

		buf[16] = 0;

		p = realloc(keyBlock, 8 * (keycnt + 1));
		if (!p) {
			PrintAndLog("Cannot allocate memory for default keys");
			free(keyBlock);
			fclose(f);
			return 2;
		}
		keyBlock = p;

		memset(keyBlock + 8 * keycnt, 0, 8);
		num_to_bytes(strtoull(buf, NULL, 16), 8, keyBlock + 8 * keycnt);

		keycnt++;
		memset(buf, 0, sizeof(buf));
	}
	fclose(f);
	PrintAndLog("Loaded %2d keys from %s", keycnt, filename);

	uint8_t CSN[8];
	if (!iClass_select(CSN, false, true, true)) {
		DropField();
		return 0;
	}

	for (uint32_t c = 0; c < keycnt; c++) {

		memcpy(key, keyBlock + 8 * c, 8);

		// debit key
		if (iClass_authenticate(CSN, key, mac, div_key, false, use_elite, use_raw, false, false)) {
			PrintAndLog("\n   Found AA1 debit key\t\t[%s]", sprint_hex(key, 8));
			found_debit = true;
		}

		// credit key
		if (iClass_authenticate(CSN, key, mac, div_key, true, use_elite, use_raw, false, false)) {
			PrintAndLog("\n   Found AA2 credit key\t\t[%s]", sprint_hex(key, 8));
			found_credit = true;
		}

		// both keys found.
		if (found_debit && found_credit)
			break;
	}

	DropField();
	free(keyBlock);
	PrintAndLog("");
	return 0;
}


static void usage_hf_iclass_permutekey(void) {
	PrintAndLogEx(NORMAL, "Convert keys from standard NIST to iClass format (and vice versa)");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Usage:  hf iclass permute [h] [r] <key>");
	PrintAndLogEx(NORMAL, "Options:");
	PrintAndLogEx(NORMAL, "           h          This help");
	PrintAndLogEx(NORMAL, "           r          reverse convert key from iClass to NIST format");
	PrintAndLogEx(NORMAL, "");
	PrintAndLogEx(NORMAL, "Examples:");
	PrintAndLogEx(NORMAL, "      hf iclass permute r 0123456789abcdef");
}


static int CmdHFiClassPermuteKey(const char *Cmd) {

	uint8_t key[8] = {0};
	uint8_t data[16] = {0};
	bool isReverse = false;
	int len = sizeof(data);
	char cmdp = tolower(param_getchar(Cmd, 0));
	if (strlen(Cmd) == 0 || cmdp == 'h') {
		usage_hf_iclass_permutekey();
		return 0;
	}

	if (cmdp == 'r') {
		isReverse = true;
		param_gethex_ex(Cmd, 1, data, &len);
	} else if (cmdp == 'f') {
		param_gethex_ex(Cmd, 1, data, &len);
	} else {
		param_gethex_ex(Cmd, 0, data, &len);
	}


	if (len % 2) {
		usage_hf_iclass_permutekey();
		return 0;
	}

	len >>= 1;

	memcpy(key, data, 8);

	if (isReverse) {
		// generate_rev(data, len);
		uint8_t key_std_format[8] = {0};
		permutekey_rev(key, key_std_format);
		PrintAndLogEx(SUCCESS, "key in standard NIST format:     %s \n", sprint_hex(key_std_format, 8));
		// if (mbedtls_des_key_check_key_parity(key_std_format
	} else {
		// generate(data, len);
		uint8_t key_iclass_format[8] = {0};
		permutekey(key, key_iclass_format);
		PrintAndLogEx(SUCCESS, "key in iClass (permuted) format: %s \n", sprint_hex(key_iclass_format, 8));
	}
	return 0;
}


static int CmdHelp(const char *Cmd);

static command_t CommandTable[] = {
	{"help",        CmdHelp,                        1,  "This help"},
	{"calcnewkey",  CmdHFiClassCalcNewKey,          1,  "[options..] Calc Diversified keys (blocks 3 & 4) to write new keys"},
	{"chk",         CmdHFiClassCheckKeys,           0,  "            Check keys"},
	{"clone",       CmdHFiClassCloneTag,            0,  "[options..] Authenticate and Clone from iClass bin file"},
	{"decrypt",     CmdHFiClassDecrypt,             1,  "[f <fname>] Decrypt tagdump" },
	{"dump",        CmdHFiClassReader_Dump,         0,  "[options..] Authenticate and Dump iClass tag's AA1 and/or AA2"},
	{"eload",       CmdHFiClassELoad,               0,  "[f <fname>] (experimental) Load data into iClass emulator memory"},
	{"encryptblk",  CmdHFiClassEncryptBlk,          1,  "<BlockData> Encrypt given block data"},
	{"list",        CmdHFiClassList,                0,  "            (Deprecated) List iClass history"},
	{"loclass",     CmdHFiClass_loclass,            1,  "[options..] Use loclass to perform bruteforce of reader attack dump"},
	{"managekeys",  CmdHFiClassManageKeys,          1,  "[options..] Manage the keys to use with iClass"},
	{"permutekey",  CmdHFiClassPermuteKey,          1,  "            iClass key permutation"},
	{"readblk",     CmdHFiClass_ReadBlock,          0,  "[options..] Authenticate and Read iClass block"},
	{"reader",      CmdHFiClassReader,              0,  "            Look for iClass tags until a key or the pm3 button is pressed"},
	{"readtagfile", CmdHFiClassReadTagFile,         1,  "[options..] Display Content from tagfile"},
	{"sim",         CmdHFiClassSim,                 0,  "[options..] Simulate iClass tag"},
	{"snoop",       CmdHFiClassSnoop,               0,  "            Eavesdrop iClass communication"},
	{"writeblk",    CmdHFiClass_WriteBlock,         0,  "[options..] Authenticate and Write iClass block"},
	{NULL, NULL, 0, NULL}
};


int CmdHFiClass(const char *Cmd) {
	clearCommandBuffer();
	CmdsParse(CommandTable, Cmd);
	return 0;
}


int CmdHelp(const char *Cmd) {
	CmdsHelp(CommandTable);
	return 0;
}
