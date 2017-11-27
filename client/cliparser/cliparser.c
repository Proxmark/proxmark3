//-----------------------------------------------------------------------------
// Copyright (C) 2017 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Command line parser core commands
//-----------------------------------------------------------------------------

#include "cliparser.h"
#include <stdio.h>
#include <string.h>

void **argtable = NULL;
size_t argtableLen = 0;
char *programName = NULL;
char *programHint = NULL;
char buf[500] = {0};

int CLIParserInit(char *vprogramName, char *vprogramHint) {
	argtable = NULL;
	argtableLen = 0;
	programName = vprogramName;
	programHint = vprogramHint;
	memset(buf, 0x00, 500);
	
	return 0;
}

int CLIParserParseArg(int argc, char **argv, void* vargtable[], size_t vargtableLen, bool allowEmptyExec) { 
	int nerrors;
	
	argtable = vargtable;
	argtableLen = vargtableLen;
	
	/* verify the argtable[] entries were allocated sucessfully */
	if (arg_nullcheck(argtable) != 0) {
		/* NULL entries were detected, some allocations must have failed */
		printf("ERROR: Insufficient memory\n");
		return 2;
	}
	/* Parse the command line as defined by argtable[] */
	nerrors = arg_parse(argc, argv, argtable);
	
	/* special case: '--help' takes precedence over error reporting */
	if ((argc < 2 && !allowEmptyExec) ||((struct arg_lit *)argtable[0])->count > 0) { // help must be the first record  
		printf("Usage: %s", programName);
		arg_print_syntaxv(stdout, argtable, "\n");
		if (programHint)
			printf("%s\n\n", programHint);
		arg_print_glossary(stdout, argtable, "    %-19s %s\n");
		printf("\n");
		
		return 1;
	}

	/* If the parser returned any errors then display them and exit */
	if (nerrors > 0) {
		/* Display the error details contained in the arg_end struct.*/
		arg_print_errors(stdout, ((struct arg_end *)argtable[vargtableLen - 1]), programName);
		printf("Try '%s --help' for more information.\n", programName);
		
		return 3;
	}	
	
	return 0;
}

enum ParserState {
	PS_FIRST,
	PS_ARGUMENT,
	PS_OPTION,
};

#define isSpace(c)(c == ' ' || c == '\t')

int CLIParserParseString(const char* str, void* vargtable[], size_t vargtableLen, bool allowEmptyExec) {
	int argc = 0;
	char *argv[200] = {NULL};
	
	int len = strlen(str);
	char *bufptr = buf;
	char *spaceptr = NULL;
	enum ParserState state = PS_FIRST;
	
	argv[argc++] = bufptr;	
	// param0 = program name
	memcpy(buf, programName, strlen(programName) + 1); // with 0x00
	bufptr += strlen(programName) + 1;
	if (len)
		argv[argc++] = bufptr;	
	
	// parse params
	for (int i = 0; i < len; i++) {
		switch(state){
			case PS_FIRST: // first char
				if (str[i] == '-'){ // first char before space is '-' - next element - option
					state = PS_OPTION;

					if (spaceptr) {
						bufptr = spaceptr;
						*bufptr = 0x00;
						bufptr++;
						argv[argc++] = bufptr;	
					}
				}
				spaceptr = NULL;
			case PS_ARGUMENT:
				if (state == PS_FIRST)
					state = PS_ARGUMENT;
				if (isSpace(str[i])) {
					spaceptr = bufptr;
					state = PS_FIRST;
				}
				*bufptr = str[i];
				bufptr++;
				break;
			case PS_OPTION:
				if (isSpace(str[i])){
					state = PS_FIRST;

					*bufptr = 0x00;
					bufptr++;
					argv[argc++] = bufptr;	
					break;
				}
				
				*bufptr = str[i];
				bufptr++;
				break;
		}
	}

	return CLIParserParseArg(argc, argv, vargtable, vargtableLen, allowEmptyExec);
}

void CLIParserFree() {
	arg_freetable(argtable, argtableLen);
	argtable = NULL;
	
	return;
}

// convertors
int CLIParamHexToBuf(struct arg_str *argstr, uint8_t *data, int maxdatalen, int *datalen) {
	switch(param_gethex_to_eol(argstr->sval[0], 0, data, maxdatalen, datalen)) {
	case 1:
		printf("Parameter error: Invalid HEX value.\n");
		return 1;
	case 2:
		printf("Parameter error: parameter too large.\n");
		return 2;
	case 3:
		printf("Parameter error: Hex string must have even number of digits.\n");
		return 3;
	}
	
	return 0;
}



