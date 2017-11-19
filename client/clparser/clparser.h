//-----------------------------------------------------------------------------
// Copyright (C) 2017 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// EMV commands
//-----------------------------------------------------------------------------

#include "argtable3.h"
#include "util.h"

extern int CLParserInit(char *vprogramName, char *vprogramHint);
extern int CLParserParseString(const char* str, void* argtable[], size_t vargtableLen);
extern int CLParserParseArg(int argc, char **argv, void* argtable[], size_t vargtableLen);
extern void CLParserFree();

extern int CLParamHexToBuf(struct arg_str *argstr, uint8_t *data, int maxdatalen, int *datalen);
