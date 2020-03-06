//-----------------------------------------------------------------------------
// Gerhard de Koning Gans - May 2008
// Hagen Fritsch - June 2010
// Gerhard de Koning Gans - May 2011
// Gerhard de Koning Gans - June 2012 - Added iClass card and reader emulation
// piwi - 2019
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Routines to support iClass.
//-----------------------------------------------------------------------------
// Contribution made during a security research at Radboud University Nijmegen
//
// Please feel free to contribute and extend iClass support!!
//-----------------------------------------------------------------------------

#include "iclass.h"

#include "proxmark3.h"
#include "apps.h"
#include "util.h"
#include "string.h"
#include "printf.h"
#include "common.h"
#include "usb_cdc.h"
#include "iso14443a.h"
#include "iso15693.h"
// Needed for CRC in emulation mode;
// same construction as in ISO 14443;
// different initial value (CRC_ICLASS)
#include "iso14443crc.h"
#include "iso15693tools.h"
#include "protocols.h"
#include "optimized_cipher.h"
#include "fpgaloader.h"

// iCLASS has a slightly different timing compared to ISO15693. According to the picopass data sheet the tag response is expected 330us after
// the reader command. This is measured from end of reader EOF to first modulation of the tag's SOF which starts with a 56,64us unmodulated period.
// 330us = 140 ssp_clk cycles @ 423,75kHz when simulating.
// 56,64us = 24 ssp_clk_cycles
#define DELAY_ICLASS_VCD_TO_VICC_SIM     (140 - 24)
// times in ssp_clk_cycles @ 3,3625MHz when acting as reader
#define DELAY_ICLASS_VICC_TO_VCD_READER  DELAY_ISO15693_VICC_TO_VCD_READER
// times in samples @ 212kHz when acting as reader
#define ICLASS_READER_TIMEOUT_ACTALL     330 // 1558us, nominal 330us + 7slots*160us = 1450us
#define ICLASS_READER_TIMEOUT_UPDATE    3390 // 16000us, nominal 4-15ms
#define ICLASS_READER_TIMEOUT_OTHERS      80 // 380us, nominal 330us

#define ICLASS_BUFFER_SIZE 34                // we expect max 34 bytes as tag answer (response to READ4)


//=============================================================================
// A `sniffer' for iClass communication
// Both sides of communication!
//=============================================================================
void SnoopIClass(uint8_t jam_search_len, uint8_t *jam_search_string) {
	SnoopIso15693(jam_search_len, jam_search_string);
}


void rotateCSN(uint8_t* originalCSN, uint8_t* rotatedCSN) {
	int i;
	for (i = 0; i < 8; i++) {
		rotatedCSN[i] = (originalCSN[i] >> 3) | (originalCSN[(i+1)%8] << 5);
	}
}


// Encode SOF only
static void CodeIClassTagSOF() {
	ToSendReset();
	ToSend[++ToSendMax] = 0x1D;
	ToSendMax++;
}


static void AppendCrc(uint8_t *data, int len) {
	ComputeCrc14443(CRC_ICLASS, data, len, data+len, data+len+1);
}


/**
 * @brief Does the actual simulation
 */
int doIClassSimulation(int simulationMode, uint8_t *reader_mac_buf) {

	// free eventually allocated BigBuf memory
	BigBuf_free_keep_EM();

	uint16_t page_size = 32 * 8;
	uint8_t current_page = 0;

	// maintain cipher states for both credit and debit key for each page
	State cipher_state_KC[8];
	State cipher_state_KD[8];
	State *cipher_state = &cipher_state_KD[0];

	uint8_t *emulator = BigBuf_get_EM_addr();
	uint8_t *csn = emulator;

	// CSN followed by two CRC bytes
	uint8_t anticoll_data[10];
	uint8_t csn_data[10];
	memcpy(csn_data, csn, sizeof(csn_data));
	Dbprintf("Simulating CSN %02x%02x%02x%02x%02x%02x%02x%02x", csn[0], csn[1], csn[2], csn[3], csn[4], csn[5], csn[6], csn[7]);

	// Construct anticollision-CSN
	rotateCSN(csn_data, anticoll_data);

	// Compute CRC on both CSNs
	AppendCrc(anticoll_data, 8);
	AppendCrc(csn_data, 8);

	uint8_t diversified_key_d[8] = { 0x00 };
	uint8_t diversified_key_c[8] = { 0x00 };
	uint8_t *diversified_key = diversified_key_d;

	// configuration block
	uint8_t conf_block[10] = {0x12, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F, 0xFF, 0x3C, 0x00, 0x00};

	// e-Purse
	uint8_t card_challenge_data[8] = { 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	if (simulationMode == ICLASS_SIM_MODE_FULL) {
		// initialize from page 0
		memcpy(conf_block, emulator + 8 * 1, 8);
		memcpy(card_challenge_data, emulator + 8 * 2, 8); // e-purse
		memcpy(diversified_key_d, emulator + 8 * 3, 8);   // Kd
		memcpy(diversified_key_c, emulator + 8 * 4, 8);   // Kc
	}

	AppendCrc(conf_block, 8);

	// save card challenge for sim2,4 attack
	if (reader_mac_buf != NULL) {
		memcpy(reader_mac_buf, card_challenge_data, 8);
	}

	if (conf_block[5] & 0x80) {
		page_size = 256 * 8;
	}

	// From PicoPass DS:
	// When the page is in personalization mode this bit is equal to 1.
	// Once the application issuer has personalized and coded its dedicated areas, this bit must be set to 0:
	// the page is then "in application mode".
	bool personalization_mode = conf_block[7] & 0x80;

	// chip memory may be divided in 8 pages
	uint8_t max_page = conf_block[4] & 0x10 ? 0 : 7;

	// Precalculate the cipher states, feeding it the CC
	cipher_state_KD[0] = opt_doTagMAC_1(card_challenge_data, diversified_key_d);
	cipher_state_KC[0] = opt_doTagMAC_1(card_challenge_data, diversified_key_c);
	if (simulationMode == ICLASS_SIM_MODE_FULL) {
		for (int i = 1; i < max_page; i++) {
			uint8_t *epurse = emulator + i*page_size + 8*2;
			uint8_t *Kd = emulator + i*page_size + 8*3;
			uint8_t *Kc = emulator + i*page_size + 8*4;
			cipher_state_KD[i] = opt_doTagMAC_1(epurse, Kd);
			cipher_state_KC[i] = opt_doTagMAC_1(epurse, Kc);
		}
	}

	int exitLoop = 0;
	// Reader 0a
	// Tag    0f
	// Reader 0c
	// Tag    anticoll. CSN
	// Reader 81 anticoll. CSN
	// Tag    CSN

	uint8_t *modulated_response;
	int modulated_response_size = 0;
	uint8_t *trace_data = NULL;
	int trace_data_size = 0;

	// Respond SOF -- takes 1 bytes
	uint8_t *resp_sof = BigBuf_malloc(1);
	int resp_sof_Len;

	// Anticollision CSN (rotated CSN)
	// 22: Takes 2 bytes for SOF/EOF and 10 * 2 = 20 bytes (2 bytes/byte)
	uint8_t *resp_anticoll = BigBuf_malloc(22);
	int resp_anticoll_len;

	// CSN (block 0)
	// 22: Takes 2 bytes for SOF/EOF and 10 * 2 = 20 bytes (2 bytes/byte)
	uint8_t *resp_csn = BigBuf_malloc(22);
	int resp_csn_len;

	// configuration (block 1) picopass 2ks
	uint8_t *resp_conf = BigBuf_malloc(22);
	int resp_conf_len;

	// e-Purse (block 2)
	// 18: Takes 2 bytes for SOF/EOF and 8 * 2 = 16 bytes (2 bytes/bit)
	uint8_t *resp_cc = BigBuf_malloc(18);
	int resp_cc_len;

	// Kd, Kc (blocks 3 and 4). Cannot be read. Always respond with 0xff bytes only
	uint8_t *resp_ff = BigBuf_malloc(22);
	int resp_ff_len;
	uint8_t ff_data[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};
	AppendCrc(ff_data, 8);

	// Application Issuer Area (block 5)
	uint8_t *resp_aia = BigBuf_malloc(22);
	int resp_aia_len;
	uint8_t aia_data[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};
	AppendCrc(aia_data, 8);

	uint8_t *receivedCmd = BigBuf_malloc(MAX_FRAME_SIZE);
	int len;

	// Prepare card messages

	// First card answer: SOF only
	CodeIClassTagSOF();
	memcpy(resp_sof, ToSend, ToSendMax);
	resp_sof_Len = ToSendMax;

	// Anticollision CSN
	CodeIso15693AsTag(anticoll_data, sizeof(anticoll_data));
	memcpy(resp_anticoll, ToSend, ToSendMax);
	resp_anticoll_len = ToSendMax;

	// CSN (block 0)
	CodeIso15693AsTag(csn_data, sizeof(csn_data));
	memcpy(resp_csn, ToSend, ToSendMax);
	resp_csn_len = ToSendMax;

	// Configuration (block 1)
	CodeIso15693AsTag(conf_block, sizeof(conf_block));
	memcpy(resp_conf, ToSend, ToSendMax);
	resp_conf_len = ToSendMax;

	// e-Purse (block 2)
	CodeIso15693AsTag(card_challenge_data, sizeof(card_challenge_data));
	memcpy(resp_cc, ToSend, ToSendMax);
	resp_cc_len = ToSendMax;

	// Kd, Kc (blocks 3 and 4)
	CodeIso15693AsTag(ff_data, sizeof(ff_data));
	memcpy(resp_ff, ToSend, ToSendMax);
	resp_ff_len = ToSendMax;

	// Application Issuer Area (block 5)
	CodeIso15693AsTag(aia_data, sizeof(aia_data));
	memcpy(resp_aia, ToSend, ToSendMax);
	resp_aia_len = ToSendMax;

	//This is used for responding to READ-block commands or other data which is dynamically generated
	uint8_t *data_generic_trace = BigBuf_malloc(32 + 2); // 32 bytes data + 2byte CRC is max tag answer
	uint8_t *data_response = BigBuf_malloc( (32 + 2) * 2 + 2);

	bool buttonPressed = false;
	enum { IDLE, ACTIVATED, SELECTED, HALTED } chip_state = IDLE;

	while (!exitLoop) {
		WDT_HIT();

		uint32_t reader_eof_time = 0;
		len = GetIso15693CommandFromReader(receivedCmd, MAX_FRAME_SIZE, &reader_eof_time);
		if (len < 0) {
			buttonPressed = true;
			break;
		}

		// Now look at the reader command and provide appropriate responses
		// default is no response:
		modulated_response = NULL;
		modulated_response_size = 0;
		trace_data = NULL;
		trace_data_size = 0;

		if (receivedCmd[0] == ICLASS_CMD_ACTALL && len == 1) {
			// Reader in anticollision phase
			if (chip_state != HALTED) {
				modulated_response = resp_sof;
				modulated_response_size = resp_sof_Len;
				chip_state = ACTIVATED;
			}

		} else if (receivedCmd[0] == ICLASS_CMD_READ_OR_IDENTIFY && len == 1) { // identify
			// Reader asks for anticollision CSN
			if (chip_state == SELECTED || chip_state == ACTIVATED) {
				modulated_response = resp_anticoll;
				modulated_response_size = resp_anticoll_len;
				trace_data = anticoll_data;
				trace_data_size = sizeof(anticoll_data);
			}

		} else if (receivedCmd[0] == ICLASS_CMD_SELECT && len == 9) {
			// Reader selects anticollision CSN.
			// Tag sends the corresponding real CSN
			if (chip_state == ACTIVATED || chip_state == SELECTED) {
				if (!memcmp(receivedCmd+1, anticoll_data, 8)) {
					modulated_response = resp_csn;
					modulated_response_size = resp_csn_len;
					trace_data = csn_data;
					trace_data_size = sizeof(csn_data);
					chip_state = SELECTED;
				} else {
					chip_state = IDLE;
				}
			} else if (chip_state == HALTED) {
				// RESELECT with CSN
				if (!memcmp(receivedCmd+1, csn_data, 8)) {
					modulated_response = resp_csn;
					modulated_response_size = resp_csn_len;
					trace_data = csn_data;
					trace_data_size = sizeof(csn_data);
					chip_state = SELECTED;
				}
			}

		} else if (receivedCmd[0] == ICLASS_CMD_READ_OR_IDENTIFY && len == 4) { // read block
			uint16_t blockNo = receivedCmd[1];
			if (chip_state == SELECTED) {
				if (simulationMode == ICLASS_SIM_MODE_EXIT_AFTER_MAC) {
					// provide defaults for blocks 0 ... 5
					switch (blockNo) {
						case 0: // csn (block 00)
							modulated_response = resp_csn;
							modulated_response_size = resp_csn_len;
							trace_data = csn_data;
							trace_data_size = sizeof(csn_data);
							break;
						case 1: // configuration (block 01)
							modulated_response = resp_conf;
							modulated_response_size = resp_conf_len;
							trace_data = conf_block;
							trace_data_size = sizeof(conf_block);
							break;
						case 2: // e-purse (block 02)
							modulated_response = resp_cc;
							modulated_response_size = resp_cc_len;
							trace_data = card_challenge_data;
							trace_data_size = sizeof(card_challenge_data);
							// set epurse of sim2,4 attack
							if (reader_mac_buf != NULL) {
								memcpy(reader_mac_buf, card_challenge_data, 8);
							}
							break;
						case 3:
						case 4: // Kd, Kc, always respond with 0xff bytes
							modulated_response = resp_ff;
							modulated_response_size = resp_ff_len;
							trace_data = ff_data;
							trace_data_size = sizeof(ff_data);
							break;
						case 5: // Application Issuer Area (block 05)
							modulated_response = resp_aia;
							modulated_response_size = resp_aia_len;
							trace_data = aia_data;
							trace_data_size = sizeof(aia_data);
							break;
						// default: don't respond
					}
				} else if (simulationMode == ICLASS_SIM_MODE_FULL) {
					if (blockNo == 3 || blockNo == 4) { // Kd, Kc, always respond with 0xff bytes
						modulated_response = resp_ff;
						modulated_response_size = resp_ff_len;
						trace_data = ff_data;
						trace_data_size = sizeof(ff_data);
					} else { // use data from emulator memory
						memcpy(data_generic_trace, emulator + current_page*page_size + 8*blockNo, 8);
						AppendCrc(data_generic_trace, 8);
						trace_data = data_generic_trace;
						trace_data_size = 10;
						CodeIso15693AsTag(trace_data, trace_data_size);
						memcpy(data_response, ToSend, ToSendMax);
						modulated_response = data_response;
						modulated_response_size = ToSendMax;
					}
				}
			}

		} else if ((receivedCmd[0] == ICLASS_CMD_READCHECK_KD
					|| receivedCmd[0] == ICLASS_CMD_READCHECK_KC) && receivedCmd[1] == 0x02 && len == 2) {
			// Read e-purse (88 02 || 18 02)
			if (chip_state == SELECTED) {
				if(receivedCmd[0] == ICLASS_CMD_READCHECK_KD){
					cipher_state = &cipher_state_KD[current_page];
					diversified_key = diversified_key_d;
				} else {
					cipher_state = &cipher_state_KC[current_page];
					diversified_key = diversified_key_c;
				}
				modulated_response = resp_cc;
				modulated_response_size = resp_cc_len;
				trace_data = card_challenge_data;
				trace_data_size = sizeof(card_challenge_data);
			}

		} else if ((receivedCmd[0] == ICLASS_CMD_CHECK_KC
					|| receivedCmd[0] == ICLASS_CMD_CHECK_KD) && len == 9) {
			// Reader random and reader MAC!!!
			if (chip_state == SELECTED) {
				if (simulationMode == ICLASS_SIM_MODE_FULL) {
					//NR, from reader, is in receivedCmd+1
					opt_doTagMAC_2(*cipher_state, receivedCmd+1, data_generic_trace, diversified_key);
					trace_data = data_generic_trace;
					trace_data_size = 4;
					CodeIso15693AsTag(trace_data, trace_data_size);
					memcpy(data_response, ToSend, ToSendMax);
					modulated_response = data_response;
					modulated_response_size = ToSendMax;
					//exitLoop = true;
				} else { // Not fullsim, we don't respond
					// We do not know what to answer, so lets keep quiet
					if (simulationMode == ICLASS_SIM_MODE_EXIT_AFTER_MAC) {
						if (reader_mac_buf != NULL) {
							// save NR and MAC for sim 2,4
							memcpy(reader_mac_buf + 8, receivedCmd + 1, 8);
						}
						exitLoop = true;
					}
				}
			}

		} else if (receivedCmd[0] == ICLASS_CMD_HALT && len == 1) {
			if (chip_state == SELECTED) {
				// Reader ends the session
				modulated_response = resp_sof;
				modulated_response_size = resp_sof_Len;
				chip_state = HALTED;
			}

		} else if (simulationMode == ICLASS_SIM_MODE_FULL && receivedCmd[0] == ICLASS_CMD_READ4 && len == 4) {  // 0x06
			//Read 4 blocks
			if (chip_state == SELECTED) {
				uint8_t blockNo = receivedCmd[1];
				memcpy(data_generic_trace, emulator + current_page*page_size + blockNo*8, 8 * 4);
				AppendCrc(data_generic_trace, 8 * 4);
				trace_data = data_generic_trace;
				trace_data_size = 8 * 4 + 2;
				CodeIso15693AsTag(trace_data, trace_data_size);
				memcpy(data_response, ToSend, ToSendMax);
				modulated_response = data_response;
				modulated_response_size = ToSendMax;
			}

		} else if (receivedCmd[0] == ICLASS_CMD_UPDATE && (len == 12 || len == 14)) {
			// We're expected to respond with the data+crc, exactly what's already in the receivedCmd
			// receivedCmd is now UPDATE 1b | ADDRESS 1b | DATA 8b | Signature 4b or CRC 2b
			if (chip_state == SELECTED) {
				uint8_t blockNo = receivedCmd[1];
				if (blockNo == 2) { // update e-purse
					memcpy(card_challenge_data, receivedCmd+2, 8);
					CodeIso15693AsTag(card_challenge_data, sizeof(card_challenge_data));
					memcpy(resp_cc, ToSend, ToSendMax);
					resp_cc_len = ToSendMax;
					cipher_state_KD[current_page] = opt_doTagMAC_1(card_challenge_data, diversified_key_d);
					cipher_state_KC[current_page] = opt_doTagMAC_1(card_challenge_data, diversified_key_c);
					if (simulationMode == ICLASS_SIM_MODE_FULL) {
						memcpy(emulator + current_page*page_size + 8*2, card_challenge_data, 8);
					}
				} else if (blockNo == 3) { // update Kd
					for (int i = 0; i < 8; i++) {
						if (personalization_mode) {
							diversified_key_d[i] = receivedCmd[2 + i];
						} else {
							diversified_key_d[i] ^= receivedCmd[2 + i];
						}
					}
					cipher_state_KD[current_page] = opt_doTagMAC_1(card_challenge_data, diversified_key_d);
					if (simulationMode == ICLASS_SIM_MODE_FULL) {
						memcpy(emulator + current_page*page_size + 8*3, diversified_key_d, 8);
					}
				} else if (blockNo == 4) { // update Kc
					for (int i = 0; i < 8; i++) {
						if (personalization_mode) {
							diversified_key_c[i] = receivedCmd[2 + i];
						} else {
							diversified_key_c[i] ^= receivedCmd[2 + i];
						}
					}
					cipher_state_KC[current_page] = opt_doTagMAC_1(card_challenge_data, diversified_key_c);
					if (simulationMode == ICLASS_SIM_MODE_FULL) {
						memcpy(emulator + current_page*page_size + 8*4, diversified_key_c, 8);
					}
				} else if (simulationMode == ICLASS_SIM_MODE_FULL) { // update any other data block
						memcpy(emulator + current_page*page_size + 8*blockNo, receivedCmd+2, 8);
				}
				memcpy(data_generic_trace, receivedCmd + 2, 8);
				AppendCrc(data_generic_trace, 8);
				trace_data = data_generic_trace;
				trace_data_size = 10;
				CodeIso15693AsTag(trace_data, trace_data_size);
				memcpy(data_response, ToSend, ToSendMax);
				modulated_response = data_response;
				modulated_response_size = ToSendMax;
			}

		} else if (receivedCmd[0] == ICLASS_CMD_PAGESEL && len == 4) {
			// Pagesel
			// Chips with a single page will not answer to this command
			// Otherwise, we should answer 8bytes (conf block 1) + 2bytes CRC
			if (chip_state == SELECTED) {
				if (simulationMode == ICLASS_SIM_MODE_FULL && max_page > 0) {
					current_page = receivedCmd[1];
					memcpy(data_generic_trace, emulator + current_page*page_size + 8*1, 8);
					memcpy(diversified_key_d, emulator + current_page*page_size + 8*3, 8);
					memcpy(diversified_key_c, emulator + current_page*page_size + 8*4, 8);
					cipher_state = &cipher_state_KD[current_page];
					personalization_mode = data_generic_trace[7] & 0x80;
					AppendCrc(data_generic_trace, 8);
					trace_data = data_generic_trace;
					trace_data_size = 10;
					CodeIso15693AsTag(trace_data, trace_data_size);
					memcpy(data_response, ToSend, ToSendMax);
					modulated_response = data_response;
					modulated_response_size = ToSendMax;
				}
			}

		} else if (receivedCmd[0] == 0x26 && len == 5) {
			// standard ISO15693 INVENTORY command. Ignore.

		} else {
			// don't know how to handle this command
			char debug_message[250]; // should be enough
			sprintf(debug_message, "Unhandled command (len = %d) received from reader:", len);
			for (int i = 0; i < len && strlen(debug_message) < sizeof(debug_message) - 3 - 1; i++) {
				sprintf(debug_message + strlen(debug_message), " %02x", receivedCmd[i]);
			}
			Dbprintf("%s", debug_message);
			// Do not respond
		}

		/**
		A legit tag has about 273,4us delay between reader EOT and tag SOF.
		**/
		if (modulated_response_size > 0) {
			uint32_t response_time = reader_eof_time + DELAY_ICLASS_VCD_TO_VICC_SIM;
			TransmitTo15693Reader(modulated_response, modulated_response_size, &response_time, 0, false);
			LogTrace_ISO15693(trace_data, trace_data_size, response_time*32, response_time*32 + modulated_response_size*32*64, NULL, false);
		}

	}

	if (buttonPressed)
	{
		DbpString("Button pressed");
	}
	return buttonPressed;
}

/**
 * @brief SimulateIClass simulates an iClass card.
 * @param arg0 type of simulation
 *          - 0 uses the first 8 bytes in usb data as CSN
 *          - 2 "dismantling iclass"-attack. This mode iterates through all CSN's specified
 *          in the usb data. This mode collects MAC from the reader, in order to do an offline
 *          attack on the keys. For more info, see "dismantling iclass" and proxclone.com.
 *          - Other : Uses the default CSN (031fec8af7ff12e0)
 * @param arg1 - number of CSN's contained in datain (applicable for mode 2 only)
 * @param arg2
 * @param datain
 */
void SimulateIClass(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint8_t *datain) {

	LED_A_ON();

	Iso15693InitTag();
	
	uint32_t simType = arg0;
	uint32_t numberOfCSNS = arg1;

	// Enable and clear the trace
	set_tracing(true);
	clear_trace();
	//Use the emulator memory for SIM
	uint8_t *emulator = BigBuf_get_EM_addr();

	if (simType == ICLASS_SIM_MODE_CSN) {
		// Use the CSN from commandline
		memcpy(emulator, datain, 8);
		doIClassSimulation(ICLASS_SIM_MODE_CSN, NULL);
	} else if (simType == ICLASS_SIM_MODE_CSN_DEFAULT) {
		//Default CSN
		uint8_t csn[] = {0x03, 0x1f, 0xec, 0x8a, 0xf7, 0xff, 0x12, 0xe0};
		memcpy(emulator, csn, 8);
		doIClassSimulation(ICLASS_SIM_MODE_CSN, NULL);
	} else if (simType == ICLASS_SIM_MODE_READER_ATTACK) {
		uint8_t mac_responses[USB_CMD_DATA_SIZE] = { 0 };
		Dbprintf("Going into attack mode, %d CSNS sent", numberOfCSNS);
		// In this mode, a number of csns are within datain. We'll simulate each one, one at a time
		// in order to collect MAC's from the reader. This can later be used in an offline-attack
		// in order to obtain the keys, as in the "dismantling iclass"-paper.
		int i;
		for (i = 0; i < numberOfCSNS && i*16+16 <= USB_CMD_DATA_SIZE; i++) {
			// The usb data is 512 bytes, fitting 32 responses (8 byte CC + 4 Byte NR + 4 Byte MAC = 16 Byte response).
			memcpy(emulator, datain+(i*8), 8);
			if (doIClassSimulation(ICLASS_SIM_MODE_EXIT_AFTER_MAC, mac_responses+i*16)) {
				 // Button pressed
				 break;
			}
			Dbprintf("CSN: %02x %02x %02x %02x %02x %02x %02x %02x",
					datain[i*8+0], datain[i*8+1], datain[i*8+2], datain[i*8+3],
					datain[i*8+4], datain[i*8+5], datain[i*8+6], datain[i*8+7]);
			Dbprintf("NR,MAC: %02x %02x %02x %02x %02x %02x %02x %02x",
					mac_responses[i*16+ 8], mac_responses[i*16+ 9], mac_responses[i*16+10], mac_responses[i*16+11],
					mac_responses[i*16+12], mac_responses[i*16+13], mac_responses[i*16+14], mac_responses[i*16+15]);
			SpinDelay(100); // give the reader some time to prepare for next CSN
		}
		cmd_send(CMD_ACK, CMD_SIMULATE_TAG_ICLASS, i, 0, mac_responses, i*16);
	} else if (simType == ICLASS_SIM_MODE_FULL) {
		//This is 'full sim' mode, where we use the emulator storage for data.
		doIClassSimulation(ICLASS_SIM_MODE_FULL, NULL);
	} else {
		// We may want a mode here where we hardcode the csns to use (from proxclone).
		// That will speed things up a little, but not required just yet.
		Dbprintf("The mode is not implemented, reserved for future use");
	}

	Dbprintf("Done...");

	LED_A_OFF();
}


/// THE READER CODE

static void ReaderTransmitIClass(uint8_t *frame, int len, uint32_t *start_time) {

	CodeIso15693AsReader(frame, len);
	TransmitTo15693Tag(ToSend, ToSendMax, start_time);
	uint32_t end_time = *start_time + 32*(8*ToSendMax-4); // substract the 4 padding bits after EOF
	LogTrace_ISO15693(frame, len, *start_time*4, end_time*4, NULL, true);
}


static bool sendCmdGetResponseWithRetries(uint8_t* command, size_t cmdsize, uint8_t* resp, size_t max_resp_size,
										  uint8_t expected_size, uint8_t tries, uint32_t start_time, uint32_t timeout, uint32_t *eof_time) {
	while (tries-- > 0) {
		ReaderTransmitIClass(command, cmdsize, &start_time);
		if (expected_size == GetIso15693AnswerFromTag(resp, max_resp_size, timeout, eof_time)) {
			return true;
		}
	}
	return false;//Error
}


/**
 * @brief Selects an iclass tag
 * @param card_data where the CSN is stored for return
 * @return false = fail
 *         true = success
 */
static bool selectIclassTag(uint8_t *card_data, uint32_t *eof_time) {
	uint8_t act_all[]      = { 0x0a };
	uint8_t identify[]     = { 0x0c };
	uint8_t select[]       = { 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	uint8_t resp[ICLASS_BUFFER_SIZE];

	uint32_t start_time = GetCountSspClk();

	// Send act_all
	ReaderTransmitIClass(act_all, 1, &start_time);
	// Card present?
	if (GetIso15693AnswerFromTag(resp, sizeof(resp), ICLASS_READER_TIMEOUT_ACTALL, eof_time) < 0) return false; //Fail

	//Send Identify
	start_time = *eof_time + DELAY_ICLASS_VICC_TO_VCD_READER;
	ReaderTransmitIClass(identify, 1, &start_time);
	//We expect a 10-byte response here, 8 byte anticollision-CSN and 2 byte CRC
	uint8_t len = GetIso15693AnswerFromTag(resp, sizeof(resp), ICLASS_READER_TIMEOUT_OTHERS, eof_time);
	if (len != 10) return false; //Fail

	//Copy the Anti-collision CSN to our select-packet
	memcpy(&select[1], resp, 8);
	//Select the card
	start_time = *eof_time + DELAY_ICLASS_VICC_TO_VCD_READER;
	ReaderTransmitIClass(select, sizeof(select), &start_time);
	//We expect a 10-byte response here, 8 byte CSN and 2 byte CRC
	len = GetIso15693AnswerFromTag(resp, sizeof(resp), ICLASS_READER_TIMEOUT_OTHERS, eof_time);
	if (len != 10) return false; //Fail

	//Success - we got CSN
	//Save CSN in response data
	memcpy(card_data, resp, 8);

	return true;
}


// Select an iClass tag and read all blocks which are always readable without authentication
void ReaderIClass(uint8_t flags) {

	LED_A_ON();

	uint8_t card_data[6 * 8] = {0};
	memset(card_data, 0xFF, sizeof(card_data));
	uint8_t resp[ICLASS_BUFFER_SIZE];
	//Read conf block CRC(0x01) => 0xfa 0x22
	uint8_t readConf[] = {ICLASS_CMD_READ_OR_IDENTIFY, 0x01, 0xfa, 0x22};
	//Read e-purse block CRC(0x02) => 0x61 0x10
	uint8_t readEpurse[] = {ICLASS_CMD_READ_OR_IDENTIFY, 0x02, 0x61, 0x10};
	//Read App Issuer Area block CRC(0x05) => 0xde  0x64
	uint8_t readAA[] = {ICLASS_CMD_READ_OR_IDENTIFY, 0x05, 0xde, 0x64};

	uint8_t result_status = 0;

	if (flags & FLAG_ICLASS_READER_INIT) {
		Iso15693InitReader();
	}

	if (flags & FLAG_ICLASS_READER_CLEARTRACE) {
		set_tracing(true);
		clear_trace();
		StartCountSspClk();
	}

	uint32_t start_time = 0;
	uint32_t eof_time = 0;

	if (selectIclassTag(resp, &eof_time)) {
		result_status = FLAG_ICLASS_READER_CSN;
		memcpy(card_data, resp, 8);

		start_time = eof_time + DELAY_ICLASS_VICC_TO_VCD_READER;

		//Read block 1, config
		if (flags & FLAG_ICLASS_READER_CONF) {
			if (sendCmdGetResponseWithRetries(readConf, sizeof(readConf), resp, sizeof(resp), 10, 10, start_time, ICLASS_READER_TIMEOUT_OTHERS, &eof_time)) {
				result_status |= FLAG_ICLASS_READER_CONF;
				memcpy(card_data+8, resp, 8);
			} else {
				Dbprintf("Failed to read config block");
			}
			start_time = eof_time + DELAY_ICLASS_VICC_TO_VCD_READER;
		}

		//Read block 2, e-purse
		if (flags & FLAG_ICLASS_READER_CC) {
			if (sendCmdGetResponseWithRetries(readEpurse, sizeof(readEpurse), resp, sizeof(resp), 10, 10, start_time, ICLASS_READER_TIMEOUT_OTHERS, &eof_time)) {
				result_status |= FLAG_ICLASS_READER_CC;
				memcpy(card_data + (8*2), resp, 8);
			} else {
				Dbprintf("Failed to read e-purse");
			}
			start_time = eof_time + DELAY_ICLASS_VICC_TO_VCD_READER;
		}

		//Read block 5, AA
		if (flags & FLAG_ICLASS_READER_AA) {
			if (sendCmdGetResponseWithRetries(readAA, sizeof(readAA), resp, sizeof(resp), 10, 10, start_time, ICLASS_READER_TIMEOUT_OTHERS, &eof_time)) {
				result_status |= FLAG_ICLASS_READER_AA;
				memcpy(card_data + (8*5), resp, 8);
			} else {
				Dbprintf("Failed to read AA block");
			}
		}
	}
	
	cmd_send(CMD_ACK, result_status, 0, 0, card_data, sizeof(card_data));

	LED_A_OFF();
}


void iClass_Check(uint8_t *NRMAC) {
	uint8_t check[9] = {ICLASS_CMD_CHECK_KD, 0x00};
	uint8_t resp[4];
	memcpy(check+1, NRMAC, 8);
	uint32_t eof_time;
	bool isOK = sendCmdGetResponseWithRetries(check, sizeof(check), resp, sizeof(resp), 4, 3, 0, ICLASS_READER_TIMEOUT_OTHERS, &eof_time);
	cmd_send(CMD_ACK, isOK, 0, 0, resp, sizeof(resp));
}


void iClass_Readcheck(uint8_t block, bool use_credit_key) {
	uint8_t readcheck[2] = {ICLASS_CMD_READCHECK_KD, block};
	if (use_credit_key) {
		readcheck[0] = ICLASS_CMD_READCHECK_KC;
	}
	uint8_t resp[8];
	uint32_t eof_time;
	bool isOK = sendCmdGetResponseWithRetries(readcheck, sizeof(readcheck), resp, sizeof(resp), 8, 3, 0, ICLASS_READER_TIMEOUT_OTHERS, &eof_time);
	cmd_send(CMD_ACK, isOK, 0, 0, resp, sizeof(resp));
}


static bool iClass_ReadBlock(uint8_t blockNo, uint8_t *readdata) {
	uint8_t readcmd[] = {ICLASS_CMD_READ_OR_IDENTIFY, blockNo, 0x00, 0x00}; //0x88, 0x00 // can i use 0C?
	uint8_t bl = blockNo;
	uint16_t rdCrc = iclass_crc16(&bl, 1);
	readcmd[2] = rdCrc >> 8;
	readcmd[3] = rdCrc & 0xff;
	uint8_t resp[10];
	uint32_t eof_time;

	bool isOK = sendCmdGetResponseWithRetries(readcmd, sizeof(readcmd), resp, sizeof(resp), 10, 10, 0, ICLASS_READER_TIMEOUT_OTHERS, &eof_time);
	memcpy(readdata, resp, sizeof(resp));

	return isOK;
}


void iClass_ReadBlk(uint8_t blockno) {

	LED_A_ON();

	uint8_t readblockdata[10];
	bool isOK = iClass_ReadBlock(blockno, readblockdata);
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();
	cmd_send(CMD_ACK, isOK, 0, 0, readblockdata, 8);

	LED_A_OFF();
}


void iClass_Dump(uint8_t startblock, uint8_t numblks) {

	LED_A_ON();

	uint8_t readblockdata[USB_CMD_DATA_SIZE+2] = {0};
	bool isOK = false;
	uint16_t blkCnt = 0;

	if (numblks > USB_CMD_DATA_SIZE / 8) {
		numblks = USB_CMD_DATA_SIZE / 8;
	}

	for (blkCnt = 0; blkCnt < numblks; blkCnt++) {
		isOK = iClass_ReadBlock(startblock+blkCnt, readblockdata+8*blkCnt);
		if (!isOK) {
			Dbprintf("Block %02X failed to read", startblock+blkCnt);
			break;
		}
	}

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();

	cmd_send(CMD_ACK, isOK, blkCnt, 0, readblockdata, blkCnt*8);

	LED_A_OFF();
}


static bool iClass_WriteBlock_ext(uint8_t blockNo, uint8_t *data) {

	uint8_t write[16] = {ICLASS_CMD_UPDATE, blockNo};
	memcpy(write+2, data, 12); // data + mac
	AppendCrc(write+1, 13);
	uint8_t resp[10];
	bool isOK = false;
	uint32_t eof_time = 0;

	isOK = sendCmdGetResponseWithRetries(write, sizeof(write), resp, sizeof(resp), 10, 3, 0, ICLASS_READER_TIMEOUT_UPDATE, &eof_time);
	if (!isOK) {
		return false;
	}
	
	uint8_t all_ff[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	if (blockNo == 2) {
		if (memcmp(data+4, resp, 4) || memcmp(data, resp+4, 4)) { // check response. e-purse update swaps first and second half
			return false;
		}
	} else if (blockNo == 3 || blockNo == 4) {
		if (memcmp(all_ff, resp, 8)) { // check response. Key updates always return 0xffffffffffffffff
			return false;
		}
	} else {
		if (memcmp(data, resp, 8)) { // check response. All other updates return unchanged data
			return false;
		}
	}

	return true;
}


void iClass_WriteBlock(uint8_t blockNo, uint8_t *data) {

	LED_A_ON();

	bool isOK = iClass_WriteBlock_ext(blockNo, data);
	if (isOK) {
		Dbprintf("Write block [%02x] successful", blockNo);
	} else {
		Dbprintf("Write block [%02x] failed", blockNo);
	}
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();

	cmd_send(CMD_ACK, isOK, 0, 0, 0, 0);
	LED_A_OFF();
}


void iClass_Clone(uint8_t startblock, uint8_t endblock, uint8_t *data) {

	LED_A_ON();

	int written = 0;
	int total_blocks = (endblock - startblock) + 1;

	for (uint8_t block = startblock; block <= endblock; block++) {
		// block number
		if (iClass_WriteBlock_ext(block, data + (block-startblock)*12)) {
			Dbprintf("Write block [%02x] successful", block);
			written++;
		} else {
			Dbprintf("Write block [%02x] failed", block);
		}
	}

	if (written == total_blocks)
		Dbprintf("Clone complete");
	else
		Dbprintf("Clone incomplete");

	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
	LED_D_OFF();

	cmd_send(CMD_ACK, 1, 0, 0, 0, 0);
	LED_A_OFF();
}
