//-----------------------------------------------------------------------------
// Copyright (C) 2017 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Command line parser core commands
//-----------------------------------------------------------------------------

#include "argtable3.h"
#include "util.h"
#include <stdbool.h>

#define arg_param_begin arg_lit0("hH",  "help",    "print this help and exit")
#define arg_param_end arg_end(20)

#define arg_get_lit(n)(((struct arg_lit*)argtable[n]))
#define arg_get_str(n)(((struct arg_str*)argtable[n]))
#define arg_get_int(n)(((struct arg_int*)argtable[n]))

extern int CLIParserInit(char *vprogramName, char *vprogramHint);
extern int CLIParserParseString(const char* str, void* argtable[], size_t vargtableLen, bool allowEmptyExec);
extern int CLIParserParseArg(int argc, char **argv, void* argtable[], size_t vargtableLen, bool allowEmptyExec);
extern void CLIParserFree();

extern int CLIParamHexToBuf(struct arg_str *argstr, uint8_t *data, int maxdatalen, int *datalen);
