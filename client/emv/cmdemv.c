//-----------------------------------------------------------------------------
// Copyright (C) 2017, 2018 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// EMV commands
//-----------------------------------------------------------------------------

#include <ctype.h>
#include "cmdemv.h"
#include "test/cryptotest.h"
#include "cliparser/cliparser.h"
#include <jansson.h>

bool HexToBuffer(const char *errormsg, const char *hexvalue, uint8_t * buffer, size_t maxbufferlen, size_t *bufferlen) {
	int buflen = 0;	
	
	switch(param_gethex_to_eol(hexvalue, 0, buffer, maxbufferlen, &buflen)) {
	case 1:
		PrintAndLog("%s Invalid HEX value.", errormsg);
		return false;
	case 2:
		PrintAndLog("%s Hex value too large.", errormsg);
		return false;
	case 3:
		PrintAndLog("%s Hex value must have even number of digits.", errormsg);
		return false;
	}
	
	if (buflen > maxbufferlen) {
		PrintAndLog("%s HEX length (%d) more than %d", errormsg, *bufferlen, maxbufferlen);
		return false;
	}
	
	*bufferlen = buflen;
	
	return true;
}

#define TLV_ADD(tag, value)( tlvdb_change_or_add_node(tlvRoot, tag, sizeof(value) - 1, (const unsigned char *)value) )
void ParamLoadDefaults(struct tlvdb *tlvRoot) {
	//9F02:(Amount, authorized (Numeric)) len:6
	TLV_ADD(0x9F02, "\x00\x00\x00\x00\x01\x00");
	//9F1A:(Terminal Country Code) len:2
	TLV_ADD(0x9F1A, "ru");
	//5F2A:(Transaction Currency Code) len:2
	// USD 840, EUR 978, RUR 810, RUB 643, RUR 810(old), UAH 980, AZN 031, n/a 999
	TLV_ADD(0x5F2A, "\x09\x80");
	//9A:(Transaction Date) len:3
	TLV_ADD(0x9A,   "\x00\x00\x00");
	//9C:(Transaction Type) len:1   |  00 => Goods and service #01 => Cash
	TLV_ADD(0x9C,   "\x00");
	// 9F37 Unpredictable Number len:4
	TLV_ADD(0x9F37, "\x01\x02\x03\x04");
	// 9F6A Unpredictable Number (MSD for UDOL) len:4
	TLV_ADD(0x9F6A, "\x01\x02\x03\x04");
	//9F66:(Terminal Transaction Qualifiers (TTQ)) len:4
	TLV_ADD(0x9F66, "\x26\x00\x00\x00"); // qVSDC
}

bool ParamLoadFromJson(struct tlvdb *tlv) {
	json_t *root;
	json_error_t error;

	if (!tlv) {
		PrintAndLog("ERROR load params: tlv tree is NULL.");
		return false; 
	}

	// current path + file name
	const char *relfname = "emv/defparams.json"; 
	char fname[strlen(get_my_executable_directory()) + strlen(relfname) + 1];
	strcpy(fname, get_my_executable_directory());
	strcat(fname, relfname);

	root = json_load_file(fname, 0, &error);
	if (!root) {
		PrintAndLog("Load params: json error on line %d: %s", error.line, error.text);
		return false; 
	}
	
	if (!json_is_array(root)) {
		PrintAndLog("Load params: Invalid json format. root must be array.");
		return false; 
	}
	
	PrintAndLog("Load params: json OK");
	
	for(int i = 0; i < json_array_size(root); i++) {
		json_t *data, *jtype, *jlength, *jvalue;

		data = json_array_get(root, i);
		if(!json_is_object(data))
		{
			PrintAndLog("Load params: data [%d] is not an object", i + 1);
			json_decref(root);
			return false;
		}
		
		jtype = json_object_get(data, "type");
		if(!json_is_string(jtype))
		{
			PrintAndLog("Load params: data [%d] type is not a string", i + 1);
			json_decref(root);
			return false;
		}
		const char *tlvType = json_string_value(jtype);

		jvalue = json_object_get(data, "value");
		if(!json_is_string(jvalue))
		{
			PrintAndLog("Load params: data [%d] value is not a string", i + 1);
			json_decref(root);
			return false;
		}
		const char *tlvValue = json_string_value(jvalue);

		jlength = json_object_get(data, "length");
		if(!json_is_number(jlength))
		{
			PrintAndLog("Load params: data [%d] length is not a number", i + 1);
			json_decref(root);
			return false;
		}
		
		int tlvLength = json_integer_value(jlength);
		if (tlvLength > 250) {
			PrintAndLog("Load params: data [%d] length more than 250", i + 1);
			json_decref(root);
			return false;
		}
		
		PrintAndLog("TLV param: %s[%d]=%s", tlvType, tlvLength, tlvValue);
		uint8_t buf[251] = {0};
		size_t buflen = 0;
		
		// here max length must be 4, but now tlv_tag_t is 2-byte var. so let it be 2 by now...  TODO: needs refactoring tlv_tag_t...
		if (!HexToBuffer("TLV Error type:", tlvType, buf, 2, &buflen)) { 
			json_decref(root);
			return false;
		}
		tlv_tag_t tag = 0;
		for (int i = 0; i < buflen; i++) {
			tag = (tag << 8) + buf[i];
		}	
		
		if (!HexToBuffer("TLV Error value:", tlvValue, buf, sizeof(buf) - 1, &buflen)) {
			json_decref(root);
			return false;
		}
		
		if (buflen != tlvLength) {
			PrintAndLog("Load params: data [%d] length of HEX must(%d) be identical to length in TLV param(%d)", i + 1, buflen, tlvLength);
			json_decref(root);
			return false;
		}
		
		tlvdb_change_or_add_node(tlv, tag, tlvLength, (const unsigned char *)buf);		
	}

	json_decref(root);

	return true;
}

int CmdHFEMVSelect(const char *cmd) {
	uint8_t data[APDU_AID_LEN] = {0};
	int datalen = 0;

	CLIParserInit("hf emv select", 
		"Executes select applet command", 
		"Usage:\n\thf emv select -s a00000000101 -> select card, select applet\n\thf emv select -st a00000000101 -> select card, select applet, show result in TLV\n");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("sS",  "select",  "activate field and select card"),
		arg_lit0("kK",  "keep",    "keep field for next command"),
		arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
		arg_lit0("tT",  "tlv",     "TLV decode results"),
		arg_strx0(NULL,  NULL,     "<HEX applet AID>", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool activateField = arg_get_lit(1);
	bool leaveSignalON = arg_get_lit(2);
	bool APDULogging = arg_get_lit(3);
	bool decodeTLV = arg_get_lit(4);
	CLIGetStrWithReturn(5, data, &datalen);
	CLIParserFree();
	
	SetAPDULogging(APDULogging);
	
	// exec
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = EMVSelect(activateField, leaveSignalON, data, datalen, buf, sizeof(buf), &len, &sw, NULL);

	if (sw)
		PrintAndLog("APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 
	
	if (res)
		return res;
	
	if (decodeTLV)
		TLVPrintFromBuffer(buf, len);

	return 0;
}

int CmdHFEMVSearch(const char *cmd) {

	CLIParserInit("hf emv search", 
		"Tries to select all applets from applet list:\n", 
		"Usage:\n\thf emv search -s -> select card and search\n\thf emv search -st -> select card, search and show result in TLV\n");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("sS",  "select",  "activate field and select card"),
		arg_lit0("kK",  "keep",    "keep field ON for next command"),
		arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
		arg_lit0("tT",  "tlv",     "TLV decode results of selected applets"),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool activateField = arg_get_lit(1);
	bool leaveSignalON = arg_get_lit(2);
	bool APDULogging = arg_get_lit(3);
	bool decodeTLV = arg_get_lit(4);
	CLIParserFree();
	
	SetAPDULogging(APDULogging);
	
	struct tlvdb *t = NULL;
	const char *al = "Applets list";
	t = tlvdb_fixed(1, strlen(al), (const unsigned char *)al);

	if (EMVSearch(activateField, leaveSignalON, decodeTLV, t)) {
		tlvdb_free(t);
		return 2;
	}
	
	PrintAndLog("Search completed.");

	// print list here
	if (!decodeTLV) {  
		TLVPrintAIDlistFromSelectTLV(t);
	}
	
	tlvdb_free(t);
	
	return 0;
}

int CmdHFEMVPPSE(const char *cmd) {
	
	CLIParserInit("hf emv pse", 
		"Executes PSE/PPSE select command. It returns list of applet on the card:\n", 
		"Usage:\n\thf emv pse -s1 -> select, get pse\n\thf emv pse -st2 -> select, get ppse, show result in TLV\n");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("sS",  "select",  "activate field and select card"),
		arg_lit0("kK",  "keep",    "keep field ON for next command"),
		arg_lit0("1",   "pse",     "pse (1PAY.SYS.DDF01) mode"),
		arg_lit0("2",   "ppse",    "ppse (2PAY.SYS.DDF01) mode (default mode)"),
		arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
		arg_lit0("tT",  "tlv",     "TLV decode results of selected applets"),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool activateField = arg_get_lit(1);
	bool leaveSignalON = arg_get_lit(2);
	uint8_t PSENum = 2;
	if (arg_get_lit(3))
		PSENum = 1;
	if (arg_get_lit(4))
		PSENum = 2;
	bool APDULogging = arg_get_lit(5);
	bool decodeTLV = arg_get_lit(6);
	CLIParserFree();	
	
	SetAPDULogging(APDULogging);
	
	// exec
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = EMVSelectPSE(activateField, leaveSignalON, PSENum, buf, sizeof(buf), &len, &sw);
	
	if (sw)
		PrintAndLog("APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 

	if (res)
		return res;
	
	
	if (decodeTLV)
		TLVPrintFromBuffer(buf, len);

	return 0;
}

int CmdHFEMVGPO(const char *cmd) {
	uint8_t data[APDU_RES_LEN] = {0};
	int datalen = 0;

	CLIParserInit("hf emv gpo", 
		"Executes Get Processing Options command. It returns data in TLV format (0x77 - format2) or plain format (0x80 - format1).\nNeeds a EMV applet to be selected.", 
		"Usage:\n\thf emv gpo -k -> execute GPO\n"
			"\thf emv gpo -t 01020304 -> execute GPO with 4-byte PDOL data, show result in TLV\n"
			"\thf emv gpo -pmt 9F 37 04 -> load params from file, make PDOL data from PDOL, execute GPO with PDOL, show result in TLV\n"); 

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("kK",  "keep",    "keep field ON for next command"),
		arg_lit0("pP",  "params",  "load parameters from `emv/defparams.json` file for PDOLdata making from PDOL and parameters"),
		arg_lit0("mM",  "make",    "make PDOLdata from PDOL (tag 9F38) and parameters (by default uses default parameters)"),
		arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
		arg_lit0("tT",  "tlv",     "TLV decode results of selected applets"),
		arg_strx0(NULL,  NULL,     "<HEX PDOLdata/PDOL>", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool leaveSignalON = arg_get_lit(1);
	bool paramsLoadFromFile = arg_get_lit(2);
	bool dataMakeFromPDOL = arg_get_lit(3);
	bool APDULogging = arg_get_lit(4);
	bool decodeTLV = arg_get_lit(5);
	CLIGetStrWithReturn(6, data, &datalen);
	CLIParserFree();	
	
	SetAPDULogging(APDULogging);
	
	// Init TLV tree
	const char *alr = "Root terminal TLV tree";
	struct tlvdb *tlvRoot = tlvdb_fixed(1, strlen(alr), (const unsigned char *)alr);
	
	// calc PDOL
	struct tlv *pdol_data_tlv = NULL;
	struct tlv data_tlv = {
		.tag = 0x83,
		.len = datalen,
		.value = (uint8_t *)data,
	};
	if (dataMakeFromPDOL) {
		ParamLoadDefaults(tlvRoot);

		if (paramsLoadFromFile) {
			PrintAndLog("Params loading from file...");
			ParamLoadFromJson(tlvRoot);
		};
		
		pdol_data_tlv = dol_process((const struct tlv *)tlvdb_external(0x9f38, datalen, data), tlvRoot, 0x83);
		if (!pdol_data_tlv){
			PrintAndLog("ERROR: can't create PDOL TLV.");
			tlvdb_free(tlvRoot);
			return 4;
		}
	} else {
		if (paramsLoadFromFile) {
			PrintAndLog("WARNING: don't need to load parameters. Sending plain PDOL data...");
		}
		pdol_data_tlv = &data_tlv;
	}

	size_t pdol_data_tlv_data_len = 0;
	unsigned char *pdol_data_tlv_data = tlv_encode(pdol_data_tlv, &pdol_data_tlv_data_len);
	if (!pdol_data_tlv_data) {
		PrintAndLog("ERROR: can't create PDOL data.");
		tlvdb_free(tlvRoot);
		return 4;
	}
	PrintAndLog("PDOL data[%d]: %s", pdol_data_tlv_data_len, sprint_hex(pdol_data_tlv_data, pdol_data_tlv_data_len));
	
	// exec
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = EMVGPO(leaveSignalON, pdol_data_tlv_data, pdol_data_tlv_data_len, buf, sizeof(buf), &len, &sw, tlvRoot);
	
	if (pdol_data_tlv != &data_tlv)
		free(pdol_data_tlv);
	tlvdb_free(tlvRoot);
	
	if (sw)
		PrintAndLog("APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 

	if (res)
		return res;
	
	if (decodeTLV)
		TLVPrintFromBuffer(buf, len);

	return 0;
}

int CmdHFEMVReadRecord(const char *cmd) {
	uint8_t data[APDU_RES_LEN] = {0};
	int datalen = 0;

	CLIParserInit("hf emv readrec", 
		"Executes Read Record command. It returns data in TLV format.\nNeeds a bank applet to be selected and sometimes needs GPO to be executed.", 
		"Usage:\n\thf emv readrec -k 0101 -> read file SFI=01, SFIrec=01\n\thf emv readrec -kt 0201-> read file 0201 and show result in TLV\n");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("kK",  "keep",    "keep field ON for next command"),
		arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
		arg_lit0("tT",  "tlv",     "TLV decode results of selected applets"),
		arg_strx1(NULL,  NULL,     "<SFI 1byte HEX><SFIrec 1byte HEX>", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool leaveSignalON = arg_get_lit(1);
	bool APDULogging = arg_get_lit(2);
	bool decodeTLV = arg_get_lit(3);
	CLIGetStrWithReturn(4, data, &datalen);
	CLIParserFree();
	
	if (datalen != 2) {
		PrintAndLog("ERROR: Command needs to have 2 bytes of data");
		return 1;
	}
	
	SetAPDULogging(APDULogging);
		
	// exec
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = EMVReadRecord(leaveSignalON, data[0], data[1], buf, sizeof(buf), &len, &sw, NULL);
	
	if (sw)
		PrintAndLog("APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 

	if (res)
		return res;
	
	
	if (decodeTLV)
		TLVPrintFromBuffer(buf, len);

	return 0;
}

int CmdHFEMVAC(const char *cmd) {
	uint8_t data[APDU_RES_LEN] = {0};
	int datalen = 0;

	CLIParserInit("hf emv genac", 
		"Generate Application Cryptogram command. It returns data in TLV format .\nNeeds a EMV applet to be selected and GPO to be executed.", 
		"Usage:\n\thf emv genac -k 0102 -> generate AC with 2-byte CDOLdata and keep field ON after command\n"
			"\thf emv genac -t 01020304 -> generate AC with 4-byte CDOL data, show result in TLV\n"
			"\thf emv genac -Daac 01020304 -> generate AC with 4-byte CDOL data and terminal decision 'declined'\n"
			"\thf emv genac -pmt 9F 37 04 -> load params from file, make CDOL data from CDOL, generate AC with CDOL, show result in TLV"); 

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("kK",  "keep",     "keep field ON for next command"),
		arg_lit0("cC",  "cda",      "executes CDA transaction. Needs to get SDAD in results."),
		arg_str0("dD",  "decision", "<aac|tc|arqc>", "Terminal decision. aac - declined, tc - approved, arqc - online authorisation requested"),
		arg_lit0("pP",  "params",   "load parameters from `emv/defparams.json` file for CDOLdata making from CDOL and parameters"),
		arg_lit0("mM",  "make",     "make CDOLdata from CDOL (tag 8C and 8D) and parameters (by default uses default parameters)"),
		arg_lit0("aA",  "apdu",     "show APDU reqests and responses"),
		arg_lit0("tT",  "tlv",      "TLV decode results of selected applets"),
		arg_strx1(NULL,  NULL,      "<HEX CDOLdata/CDOL>", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, false);
	
	bool leaveSignalON = arg_get_lit(1);
	bool trTypeCDA = arg_get_lit(2);
	uint8_t termDecision = 0xff;
	if (arg_get_str_len(3)) {
		if (!strncmp(arg_get_str(3)->sval[0], "aac", 4))
			termDecision = EMVAC_AAC;
		if (!strncmp(arg_get_str(3)->sval[0], "tc", 4))
			termDecision = EMVAC_TC;
		if (!strncmp(arg_get_str(3)->sval[0], "arqc", 4))
			termDecision = EMVAC_ARQC;

		if (termDecision == 0xff) {
			PrintAndLog("ERROR: can't find terminal decision '%s'", arg_get_str(3)->sval[0]);
			return 1;
		}
	} else {
		termDecision = EMVAC_TC;
	}
	if (trTypeCDA)
		termDecision = termDecision | EMVAC_CDAREQ;
	bool paramsLoadFromFile = arg_get_lit(4);
	bool dataMakeFromCDOL = arg_get_lit(5);
	bool APDULogging = arg_get_lit(6);
	bool decodeTLV = arg_get_lit(7);
	CLIGetStrWithReturn(8, data, &datalen);
	CLIParserFree();	
	
	SetAPDULogging(APDULogging);
	
	// Init TLV tree
	const char *alr = "Root terminal TLV tree";
	struct tlvdb *tlvRoot = tlvdb_fixed(1, strlen(alr), (const unsigned char *)alr);
	
	// calc CDOL
	struct tlv *cdol_data_tlv = NULL;
	struct tlv data_tlv = {
		.tag = 0x01,
		.len = datalen,
		.value = (uint8_t *)data,
	};
	
	if (dataMakeFromCDOL) {
		ParamLoadDefaults(tlvRoot);

		if (paramsLoadFromFile) {
			PrintAndLog("Params loading from file...");
			ParamLoadFromJson(tlvRoot);
		};
		
		cdol_data_tlv = dol_process((const struct tlv *)tlvdb_external(0x8c, datalen, data), tlvRoot, 0x01); // 0x01 - dummy tag
		if (!cdol_data_tlv){
			PrintAndLog("ERROR: can't create CDOL TLV.");
			tlvdb_free(tlvRoot);
			return 4;
		}
	} else {
		if (paramsLoadFromFile) {
			PrintAndLog("WARNING: don't need to load parameters. Sending plain CDOL data...");
		}
		cdol_data_tlv = &data_tlv;
	}
	
	PrintAndLog("CDOL data[%d]: %s", cdol_data_tlv->len, sprint_hex(cdol_data_tlv->value, cdol_data_tlv->len));

	// exec
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = EMVAC(leaveSignalON, termDecision, (uint8_t *)cdol_data_tlv->value, cdol_data_tlv->len, buf, sizeof(buf), &len, &sw, tlvRoot);
	
	if (cdol_data_tlv != &data_tlv)
		free(cdol_data_tlv);
	tlvdb_free(tlvRoot);
	
	if (sw)
		PrintAndLog("APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 

	if (res)
		return res;
	
	if (decodeTLV)
		TLVPrintFromBuffer(buf, len);

	return 0;	
}

int CmdHFEMVGenerateChallenge(const char *cmd) {

	CLIParserInit("hf emv challenge", 
		"Executes Generate Challenge command. It returns 4 or 8-byte random number from card.\nNeeds a EMV applet to be selected and GPO to be executed.", 
		"Usage:\n\thf emv challenge -> get challenge\n\thf emv challenge -k -> get challenge, keep fileld ON\n");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("kK",  "keep",    "keep field ON for next command"),
		arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool leaveSignalON = arg_get_lit(1);
	bool APDULogging = arg_get_lit(2);
	CLIParserFree();	
	
	SetAPDULogging(APDULogging);
	
	// exec
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = EMVGenerateChallenge(leaveSignalON, buf, sizeof(buf), &len, &sw, NULL);
	
	if (sw)
		PrintAndLog("APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 

	if (res)
		return res;

	PrintAndLog("Challenge: %s", sprint_hex(buf, len));
	
	if (len != 4 && len != 8)
		PrintAndLog("WARNING: length of challenge must be 4 or 8, but it %d", len);
	
	return 0;
}

int CmdHFEMVInternalAuthenticate(const char *cmd) {
	uint8_t data[APDU_RES_LEN] = {0};
	int datalen = 0;

	CLIParserInit("hf emv intauth", 
		"Generate Internal Authenticate command. Usually needs 4-byte random number. It returns data in TLV format .\nNeeds a EMV applet to be selected and GPO to be executed.", 
		"Usage:\n\thf emv intauth -k 01020304 -> execute Internal Authenticate with 4-byte DDOLdata and keep field ON after command\n"
			"\thf emv intauth -t 01020304 -> execute Internal Authenticate with 4-byte DDOL data, show result in TLV\n"
			"\thf emv intauth -pmt 9F 37 04 -> load params from file, make DDOL data from DDOL, Internal Authenticate with DDOL, show result in TLV"); 

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("kK",  "keep",    "keep field ON for next command"),
		arg_lit0("pP",  "params",  "load parameters from `emv/defparams.json` file for DDOLdata making from DDOL and parameters"),
		arg_lit0("mM",  "make",    "make DDOLdata from DDOL (tag 9F49) and parameters (by default uses default parameters)"),
		arg_lit0("aA",  "apdu",    "show APDU reqests and responses"),
		arg_lit0("tT",  "tlv",     "TLV decode results of selected applets"),
		arg_strx1(NULL,  NULL,     "<HEX DDOLdata/DDOL>", NULL),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, false);
	
	bool leaveSignalON = arg_get_lit(1);
	bool paramsLoadFromFile = arg_get_lit(2);
	bool dataMakeFromDDOL = arg_get_lit(3);
	bool APDULogging = arg_get_lit(4);
	bool decodeTLV = arg_get_lit(5);
	CLIGetStrWithReturn(6, data, &datalen);
	CLIParserFree();	
	
	SetAPDULogging(APDULogging);

	// Init TLV tree
	const char *alr = "Root terminal TLV tree";
	struct tlvdb *tlvRoot = tlvdb_fixed(1, strlen(alr), (const unsigned char *)alr);
	
	// calc DDOL
	struct tlv *ddol_data_tlv = NULL;
	struct tlv data_tlv = {
		.tag = 0x01,
		.len = datalen,
		.value = (uint8_t *)data,
	};
	
	if (dataMakeFromDDOL) {
		ParamLoadDefaults(tlvRoot);

		if (paramsLoadFromFile) {
			PrintAndLog("Params loading from file...");
			ParamLoadFromJson(tlvRoot);
		};
		
		ddol_data_tlv = dol_process((const struct tlv *)tlvdb_external(0x9f49, datalen, data), tlvRoot, 0x01); // 0x01 - dummy tag
		if (!ddol_data_tlv){
			PrintAndLog("ERROR: can't create DDOL TLV.");
			tlvdb_free(tlvRoot);
			return 4;
		}
	} else {
		if (paramsLoadFromFile) {
			PrintAndLog("WARNING: don't need to load parameters. Sending plain DDOL data...");
		}
		ddol_data_tlv = &data_tlv;
	}
	
	PrintAndLog("DDOL data[%d]: %s", ddol_data_tlv->len, sprint_hex(ddol_data_tlv->value, ddol_data_tlv->len));
	
	// exec
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	int res = EMVInternalAuthenticate(leaveSignalON, data, datalen, buf, sizeof(buf), &len, &sw, NULL);
	
	if (ddol_data_tlv != &data_tlv)
		free(ddol_data_tlv);
	tlvdb_free(tlvRoot);	
	
	if (sw)
		PrintAndLog("APDU response status: %04x - %s", sw, GetAPDUCodeDescription(sw >> 8, sw & 0xff)); 

	if (res)
		return res;
	
	if (decodeTLV)
		TLVPrintFromBuffer(buf, len);

	return 0;	
}

#define dreturn(n) {free(pdol_data_tlv);tlvdb_free(tlvSelect);tlvdb_free(tlvRoot);DropField();return n;}

int CmdHFEMVExec(const char *cmd) {
	uint8_t buf[APDU_RES_LEN] = {0};
	size_t len = 0;
	uint16_t sw = 0;
	uint8_t AID[APDU_AID_LEN] = {0};
	size_t AIDlen = 0;
	uint8_t ODAiList[4096];
	size_t ODAiListLen = 0;
	
	int res;
	
	struct tlvdb *tlvSelect = NULL;
	struct tlvdb *tlvRoot = NULL;
	struct tlv *pdol_data_tlv = NULL;

	CLIParserInit("hf emv exec", 
		"Executes EMV contactless transaction", 
		"Usage:\n\thf emv exec -sat -> select card, execute MSD transaction, show APDU and TLV\n"
			"\thf emv exec -satc -> select card, execute CDA transaction, show APDU and TLV\n");

	void* argtable[] = {
		arg_param_begin,
		arg_lit0("sS",  "select",   "activate field and select card."),
		arg_lit0("aA",  "apdu",     "show APDU reqests and responses."),
		arg_lit0("tT",  "tlv",      "TLV decode results."),
		arg_lit0("jJ",  "jload",    "Load transaction parameters from `emv/defparams.json` file."),
		arg_lit0("fF",  "forceaid", "Force search AID. Search AID instead of execute PPSE."),
		arg_rem("By default:",      "Transaction type - MSD"),
		arg_lit0("vV",  "qvsdc",    "Transaction type - qVSDC or M/Chip."),
		arg_lit0("cC",  "qvsdccda", "Transaction type - qVSDC or M/Chip plus CDA (SDAD generation)."),
		arg_lit0("xX",  "vsdc",     "Transaction type - VSDC. For test only. Not a standart behavior."),
		arg_lit0("gG",  "acgpo",    "VISA. generate AC from GPO."),
		arg_param_end
	};
	CLIExecWithReturn(cmd, argtable, true);
	
	bool activateField = arg_get_lit(1);
	bool showAPDU = arg_get_lit(2);
	bool decodeTLV = arg_get_lit(3);
	bool paramLoadJSON = arg_get_lit(4);
	bool forceSearch = arg_get_lit(5);

	enum TransactionType TrType = TT_MSD;
	if (arg_get_lit(6))
		TrType = TT_QVSDCMCHIP;
	if (arg_get_lit(7))
		TrType = TT_CDA;
	if (arg_get_lit(8))
		TrType = TT_VSDC;

	bool GenACGPO = arg_get_lit(9);
	CLIParserFree();
	
	SetAPDULogging(showAPDU);
	
	// init applets list tree
	const char *al = "Applets list";
	tlvSelect = tlvdb_fixed(1, strlen(al), (const unsigned char *)al);

	// Application Selection
	// https://www.openscdp.org/scripts/tutorial/emv/applicationselection.html
	if (!forceSearch) {
		// PPSE
		PrintAndLog("\n* PPSE.");
		SetAPDULogging(showAPDU);
		res = EMVSearchPSE(activateField, true, decodeTLV, tlvSelect);

		// check PPSE and select application id
		if (!res) {	
			TLVPrintAIDlistFromSelectTLV(tlvSelect);
			EMVSelectApplication(tlvSelect, AID, &AIDlen);
		}
	}
	
	// Search
	if (!AIDlen) {
		PrintAndLog("\n* Search AID in list.");
		SetAPDULogging(false);
		if (EMVSearch(activateField, true, decodeTLV, tlvSelect)) {
			dreturn(2);
		}

		// check search and select application id
		TLVPrintAIDlistFromSelectTLV(tlvSelect);
		EMVSelectApplication(tlvSelect, AID, &AIDlen);
	}
	
	// Init TLV tree
	const char *alr = "Root terminal TLV tree";
	tlvRoot = tlvdb_fixed(1, strlen(alr), (const unsigned char *)alr);
	
	// check if we found EMV application on card
	if (!AIDlen) {
		PrintAndLog("Can't select AID. EMV AID not found");
		dreturn(2);
	}
	
	// Select
	PrintAndLog("\n* Selecting AID:%s", sprint_hex_inrow(AID, AIDlen));
	SetAPDULogging(showAPDU);
	res = EMVSelect(false, true, AID, AIDlen, buf, sizeof(buf), &len, &sw, tlvRoot);
	
	if (res) {	
		PrintAndLog("Can't select AID (%d). Exit...", res);
		dreturn(3);
	}
	
	if (decodeTLV)
		TLVPrintFromBuffer(buf, len);
	PrintAndLog("* Selected.");
	
	PrintAndLog("\n* Init transaction parameters.");

	ParamLoadDefaults(tlvRoot);

	if (paramLoadJSON) {
		PrintAndLog("* * Transaction parameters loading from JSON...");
		ParamLoadFromJson(tlvRoot);
	}
	
	//9F66:(Terminal Transaction Qualifiers (TTQ)) len:4
	char *qVSDC = "\x26\x00\x00\x00";
	if (GenACGPO) {
		qVSDC = "\x26\x80\x00\x00";
	}
	switch(TrType) {
		case TT_MSD:
			TLV_ADD(0x9F66, "\x86\x00\x00\x00"); // MSD
			break;
		// not standard for contactless. just for test.
		case TT_VSDC:  
			TLV_ADD(0x9F66, "\x46\x00\x00\x00"); // VSDC
			break;
		case TT_QVSDCMCHIP:
			TLV_ADD(0x9F66, qVSDC); // qVSDC
			break;
		case TT_CDA:
			TLV_ADD(0x9F66, qVSDC); // qVSDC (VISA CDA not enabled)
			break;
		default:
			break;
	}
	
	TLVPrintFromTLV(tlvRoot); // TODO delete!!!
	
	PrintAndLog("\n* Calc PDOL.");
	pdol_data_tlv = dol_process(tlvdb_get(tlvRoot, 0x9f38, NULL), tlvRoot, 0x83);
	if (!pdol_data_tlv){
		PrintAndLog("ERROR: can't create PDOL TLV.");
		dreturn(4);
	}
	
	size_t pdol_data_tlv_data_len;
	unsigned char *pdol_data_tlv_data = tlv_encode(pdol_data_tlv, &pdol_data_tlv_data_len);
	if (!pdol_data_tlv_data) {
		PrintAndLog("ERROR: can't create PDOL data.");
		dreturn(4);
	}
	PrintAndLog("PDOL data[%d]: %s", pdol_data_tlv_data_len, sprint_hex(pdol_data_tlv_data, pdol_data_tlv_data_len));

	PrintAndLog("\n* GPO.");
	res = EMVGPO(true, pdol_data_tlv_data, pdol_data_tlv_data_len, buf, sizeof(buf), &len, &sw, tlvRoot);
	
	free(pdol_data_tlv_data);
	//free(pdol_data_tlv); --- free on exit.
	
	if (res) {	
		PrintAndLog("GPO error(%d): %4x. Exit...", res, sw);
		dreturn(5);
	}

	// process response template format 1 [id:80  2b AIP + x4b AFL] and format 2 [id:77 TLV]
	if (buf[0] == 0x80) {
		if (decodeTLV){
			PrintAndLog("GPO response format1:");
			TLVPrintFromBuffer(buf, len);
		}
		
		if (len < 4 || (len - 4) % 4) {
			PrintAndLog("ERROR: GPO response format1 parsing error. length=%d", len);
		} else {
			// AIP
			struct tlvdb * f1AIP = tlvdb_fixed(0x82, 2, buf + 2);
			tlvdb_add(tlvRoot, f1AIP);
			if (decodeTLV){
				PrintAndLog("\n* * Decode response format 1 (0x80) AIP and AFL:");
				TLVPrintFromTLV(f1AIP);
			}

			// AFL
			struct tlvdb * f1AFL = tlvdb_fixed(0x94, len - 4, buf + 2 + 2);
			tlvdb_add(tlvRoot, f1AFL);
			if (decodeTLV)
				TLVPrintFromTLV(f1AFL);
		}		
	} else {
		if (decodeTLV)
			TLVPrintFromBuffer(buf, len);
	}
	
	// extract PAN from track2
	{
		const struct tlv *track2 = tlvdb_get(tlvRoot, 0x57, NULL);
		if (!tlvdb_get(tlvRoot, 0x5a, NULL) && track2 && track2->len >= 8) {
			struct tlvdb *pan = GetPANFromTrack2(track2);
			if (pan) {
				tlvdb_add(tlvRoot, pan); 
				
				const struct tlv *pantlv = tlvdb_get(tlvRoot, 0x5a, NULL);	
				PrintAndLog("\n* * Extracted PAN from track2: %s", sprint_hex(pantlv->value, pantlv->len));
			} else {
				PrintAndLog("\n* * WARNING: Can't extract PAN from track2.");
			}
		}
	}
	
	PrintAndLog("\n* Read records from AFL.");
	const struct tlv *AFL = tlvdb_get(tlvRoot, 0x94, NULL);
	if (!AFL || !AFL->len) {
		PrintAndLog("WARNING: AFL not found.");
	}
	
	while(AFL && AFL->len) {
		if (AFL->len % 4) {
			PrintAndLog("ERROR: Wrong AFL length: %d", AFL->len);
			break;
		}

		for (int i = 0; i < AFL->len / 4; i++) {
			uint8_t SFI = AFL->value[i * 4 + 0] >> 3;
			uint8_t SFIstart = AFL->value[i * 4 + 1];
			uint8_t SFIend = AFL->value[i * 4 + 2];
			uint8_t SFIoffline = AFL->value[i * 4 + 3];
			
			PrintAndLog("* * SFI[%02x] start:%02x end:%02x offline:%02x", SFI, SFIstart, SFIend, SFIoffline);
			if (SFI == 0 || SFI == 31 || SFIstart == 0 || SFIstart > SFIend) {
				PrintAndLog("SFI ERROR! Skipped...");
				continue;
			}
			
			for(int n = SFIstart; n <= SFIend; n++) {
				PrintAndLog("* * * SFI[%02x] %d", SFI, n);
				
				res = EMVReadRecord(true, SFI, n, buf, sizeof(buf), &len, &sw, tlvRoot);
				if (res) {
					PrintAndLog("ERROR SFI[%02x]. APDU error %4x", SFI, sw);
					continue;
				}
				
				if (decodeTLV) {
					TLVPrintFromBuffer(buf, len);
					PrintAndLog("");
				}
				
				// Build Input list for Offline Data Authentication
				// EMV 4.3 book3 10.3, page 96
				if (SFIoffline) {
					if (SFI < 11) {
						const unsigned char *abuf = buf;
						size_t elmlen = len;
						struct tlv e;
						if (tlv_parse_tl(&abuf, &elmlen, &e)) {
							memcpy(&ODAiList[ODAiListLen], &buf[len - elmlen], elmlen);
							ODAiListLen += elmlen;
						} else {
							PrintAndLog("ERROR SFI[%02x]. Creating input list for Offline Data Authentication error.", SFI);
						}
					} else {
						memcpy(&ODAiList[ODAiListLen], buf, len);
						ODAiListLen += len;
					}
				}
			}
		}
		
		break;
	}	
	
	// copy Input list for Offline Data Authentication
	if (ODAiListLen) {
		struct tlvdb *oda = tlvdb_fixed(0x21, ODAiListLen, ODAiList); // not a standard tag
		tlvdb_add(tlvRoot, oda); 
		PrintAndLog("* Input list for Offline Data Authentication added to TLV. len=%d \n", ODAiListLen);
	}
	
	// get AIP
	const struct tlv *AIPtlv = tlvdb_get(tlvRoot, 0x82, NULL);	
	uint16_t AIP = AIPtlv->value[0] + AIPtlv->value[1] * 0x100;
	PrintAndLog("* * AIP=%04x", AIP);

	// SDA
	if (AIP & 0x0040) {
		PrintAndLog("\n* SDA");
		trSDA(tlvRoot);
	}

	// DDA
	if (AIP & 0x0020) {
		PrintAndLog("\n* DDA");
		trDDA(decodeTLV, tlvRoot);
	}	
	
	// transaction check
	
	// qVSDC
	if (TrType == TT_QVSDCMCHIP|| TrType == TT_CDA){
		// 9F26: Application Cryptogram
		const struct tlv *AC = tlvdb_get(tlvRoot, 0x9F26, NULL);
		if (AC) {
			PrintAndLog("\n--> qVSDC transaction.");
			PrintAndLog("* AC path");
			
			// 9F36: Application Transaction Counter (ATC)
			const struct tlv *ATC = tlvdb_get(tlvRoot, 0x9F36, NULL);
			if (ATC) {
			
				// 9F10: Issuer Application Data - optional
				const struct tlv *IAD = tlvdb_get(tlvRoot, 0x9F10, NULL);

				// print AC data
				PrintAndLog("ATC: %s", sprint_hex(ATC->value, ATC->len));
				PrintAndLog("AC: %s", sprint_hex(AC->value, AC->len));
				if (IAD){
					PrintAndLog("IAD: %s", sprint_hex(IAD->value, IAD->len));
					
					if (IAD->len >= IAD->value[0] + 1) {
						PrintAndLog("\tKey index:  0x%02x", IAD->value[1]);
						PrintAndLog("\tCrypto ver: 0x%02x(%03d)", IAD->value[2], IAD->value[2]);
						PrintAndLog("\tCVR:", sprint_hex(&IAD->value[3], IAD->value[0] - 2));
						struct tlvdb * cvr = tlvdb_fixed(0x20, IAD->value[0] - 2, &IAD->value[3]);
						TLVPrintFromTLVLev(cvr, 1);
					}
				} else {
					PrintAndLog("WARNING: IAD not found.");
				}
				
			} else {
				PrintAndLog("ERROR AC: Application Transaction Counter (ATC) not found.");
			}
		}
	}
	
	// Mastercard M/CHIP
	if (GetCardPSVendor(AID, AIDlen) == CV_MASTERCARD && (TrType == TT_QVSDCMCHIP || TrType == TT_CDA)){
		const struct tlv *CDOL1 = tlvdb_get(tlvRoot, 0x8c, NULL);
		if (CDOL1 && GetCardPSVendor(AID, AIDlen) == CV_MASTERCARD) { // and m/chip transaction flag
			PrintAndLog("\n--> Mastercard M/Chip transaction.");

			PrintAndLog("* * Generate challenge");
			res = EMVGenerateChallenge(true, buf, sizeof(buf), &len, &sw, tlvRoot);
			if (res) {
				PrintAndLog("ERROR GetChallenge. APDU error %4x", sw);
				dreturn(6);
			}
			if (len < 4) {
				PrintAndLog("ERROR GetChallenge. Wrong challenge length %d", len);
				dreturn(6);
			}
			
			// ICC Dynamic Number
			struct tlvdb * ICCDynN = tlvdb_fixed(0x9f4c, len, buf);
			tlvdb_add(tlvRoot, ICCDynN);
			if (decodeTLV){
				PrintAndLog("\n* * ICC Dynamic Number:");
				TLVPrintFromTLV(ICCDynN);
			}
			
			PrintAndLog("* * Calc CDOL1");
			struct tlv *cdol_data_tlv = dol_process(tlvdb_get(tlvRoot, 0x8c, NULL), tlvRoot, 0x01); // 0x01 - dummy tag
			if (!cdol_data_tlv){
				PrintAndLog("ERROR: can't create CDOL1 TLV.");
				dreturn(6);
			}
			PrintAndLog("CDOL1 data[%d]: %s", cdol_data_tlv->len, sprint_hex(cdol_data_tlv->value, cdol_data_tlv->len));
			
			PrintAndLog("* * AC1");
			// EMVAC_TC + EMVAC_CDAREQ --- to get SDAD
			res = EMVAC(true, (TrType == TT_CDA) ? EMVAC_TC + EMVAC_CDAREQ : EMVAC_TC, (uint8_t *)cdol_data_tlv->value, cdol_data_tlv->len, buf, sizeof(buf), &len, &sw, tlvRoot);
			
			if (res) {	
				PrintAndLog("AC1 error(%d): %4x. Exit...", res, sw);
				dreturn(7);
			}
			
			if (decodeTLV)
				TLVPrintFromBuffer(buf, len);
			
			// CDA
			PrintAndLog("\n* CDA:");
			struct tlvdb *ac_tlv = tlvdb_parse_multi(buf, len);
			res = trCDA(tlvRoot, ac_tlv, pdol_data_tlv, cdol_data_tlv);
			if (res) {	
				PrintAndLog("CDA error (%d)", res);
			}
			free(ac_tlv);
			free(cdol_data_tlv);
			
			PrintAndLog("\n* M/Chip transaction result:");
			// 9F27: Cryptogram Information Data (CID)
			const struct tlv *CID = tlvdb_get(tlvRoot, 0x9F27, NULL);
			if (CID) {
				emv_tag_dump(CID, stdout, 0);
				PrintAndLog("------------------------------");
				if (CID->len > 0) {
					switch(CID->value[0] & EMVAC_AC_MASK){
						case EMVAC_AAC:
							PrintAndLog("Transaction DECLINED.");
							break;
						case EMVAC_TC:
							PrintAndLog("Transaction approved OFFLINE.");
							break;
						case EMVAC_ARQC:
							PrintAndLog("Transaction approved ONLINE.");
							break;
						default:
							PrintAndLog("ERROR: CID transaction code error %2x", CID->value[0] & EMVAC_AC_MASK);
							break;
					}
				} else {
					PrintAndLog("ERROR: Wrong CID length %d", CID->len);
				}
			} else {
				PrintAndLog("ERROR: CID(9F27) not found.");
			}
		
		}
	}
		
	// MSD
	if (AIP & 0x8000 && TrType == TT_MSD) { 
		PrintAndLog("\n--> MSD transaction.");
		
		PrintAndLog("* MSD dCVV path. Check dCVV");

		const struct tlv *track2 = tlvdb_get(tlvRoot, 0x57, NULL);
		if (track2) {
			PrintAndLog("Track2: %s", sprint_hex(track2->value, track2->len));

			struct tlvdb *dCVV = GetdCVVRawFromTrack2(track2);
			PrintAndLog("dCVV raw data:");
			TLVPrintFromTLV(dCVV);
			
			if (GetCardPSVendor(AID, AIDlen) == CV_MASTERCARD) {
				PrintAndLog("\n* Mastercard calculate UDOL");

				// UDOL (9F69)
				const struct tlv *UDOL = tlvdb_get(tlvRoot, 0x9F69, NULL);
				// UDOL(9F69) default: 9F6A (Unpredictable number) 4 bytes
				const struct tlv defUDOL = {
					.tag = 0x01,
					.len = 3,
					.value = (uint8_t *)"\x9f\x6a\x04",
				};
				if (!UDOL)
					PrintAndLog("Use default UDOL.");

				struct tlv *udol_data_tlv = dol_process(UDOL ? UDOL : &defUDOL, tlvRoot, 0x01); // 0x01 - dummy tag
				if (!udol_data_tlv){
					PrintAndLog("ERROR: can't create UDOL TLV.");
					dreturn(8);
				}

				PrintAndLog("UDOL data[%d]: %s", udol_data_tlv->len, sprint_hex(udol_data_tlv->value, udol_data_tlv->len));
				
				PrintAndLog("\n* Mastercard compute cryptographic checksum(UDOL)");
				
				res = MSCComputeCryptoChecksum(true, (uint8_t *)udol_data_tlv->value, udol_data_tlv->len, buf, sizeof(buf), &len, &sw, tlvRoot);
				if (res) {
					PrintAndLog("ERROR Compute Crypto Checksum. APDU error %4x", sw);
					free(udol_data_tlv);
					dreturn(9);
				}
				
				if (decodeTLV) {
					TLVPrintFromBuffer(buf, len);
					PrintAndLog("");
				}
				free(udol_data_tlv);

			}
		} else {
			PrintAndLog("ERROR MSD: Track2 data not found.");
		}
	}

	// DropField
	DropField();
	
	// Destroy TLV's
	free(pdol_data_tlv);
	tlvdb_free(tlvSelect);
	tlvdb_free(tlvRoot);

	PrintAndLog("\n* Transaction completed.");
	
	return 0;
}

int UsageCmdHFEMVScan(void) {
	PrintAndLog("HELP :  Scan EMV card and save it contents to a file. \n");
	PrintAndLog("        It executes EMV contactless transaction and saves result to a file which can be used for emulation.\n");
	PrintAndLog("Usage:  hf emv scan [-a][-t][-v][-c][-x][-g] <file_name>\n");
	PrintAndLog("  Options:");
	PrintAndLog("  -a       : show APDU reqests and responses\n");
	PrintAndLog("  -t       : TLV decode results\n");
	PrintAndLog("  -v       : transaction type - qVSDC or M/Chip.\n");
	PrintAndLog("  -c       : transaction type - qVSDC or M/Chip plus CDA (SDAD generation).\n");
	PrintAndLog("  -x       : transaction type - VSDC. For test only. Not a standart behavior.\n");
	PrintAndLog("  -g       : VISA. generate AC from GPO\n");
	PrintAndLog("By default : transaction type - MSD.\n");
	PrintAndLog("Samples:");
	PrintAndLog(" hf emv scan -a -t -> scan MSD transaction mode");
	PrintAndLog(" hf emv scan -a -t -c -> scan CDA transaction mode");
	return 0;
}

int CmdHFEMVScan(const char *cmd) {
	UsageCmdHFEMVScan();
	
	return 0;
}

int CmdHFEMVTest(const char *cmd) {
	return ExecuteCryptoTests(true);
}

int CmdHelp(const char *Cmd);
static command_t CommandTable[] =  {
	{"help",		CmdHelp,						1,	"This help"},
	{"exec",		CmdHFEMVExec,					0,	"Executes EMV contactless transaction."},
	{"pse",			CmdHFEMVPPSE,					0,	"Execute PPSE. It selects 2PAY.SYS.DDF01 or 1PAY.SYS.DDF01 directory."},
	{"search",		CmdHFEMVSearch,					0,	"Try to select all applets from applets list and print installed applets."},
	{"select",		CmdHFEMVSelect,					0,	"Select applet."},
	{"gpo",			CmdHFEMVGPO,					0,	"Execute GetProcessingOptions."},
	{"readrec",		CmdHFEMVReadRecord,				0,	"Read files from card."},
	{"genac",		CmdHFEMVAC,						0,	"Generate ApplicationCryptogram."},
	{"challenge",	CmdHFEMVGenerateChallenge,		0,	"Generate challenge."},
	{"intauth",		CmdHFEMVInternalAuthenticate,	0,	"Internal authentication."},
//	{"scan",	CmdHFEMVScan,	0,	"Scan EMV card and save it contents to json file for emulator."},
	{"test",		CmdHFEMVTest,					0,	"Crypto logic test."},
	{NULL, NULL, 0, NULL}
};

int CmdHFEMV(const char *Cmd) {
	CmdsParse(CommandTable, Cmd);
	return 0;
}

int CmdHelp(const char *Cmd) {
	CmdsHelp(CommandTable);
	return 0;
}
