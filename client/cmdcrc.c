//-----------------------------------------------------------------------------
// Copyright (C) 2015 iceman <iceman at iuse.se>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// CRC Calculations from the software reveng commands
//-----------------------------------------------------------------------------

#include <stdlib.h>
#ifdef _WIN32
#  include <io.h>
#  include <fcntl.h>
#  ifndef STDIN_FILENO
#    define STDIN_FILENO 0
#  endif /* STDIN_FILENO */
#endif /* _WIN32 */

#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
//#include <ctype.h>
#include "cmdmain.h"
#include "cmdcrc.h"
#include "reveng/reveng.h"
#include "ui.h"
#include "util.h"

#define MAX_ARGS 20

int split(char *str, char *arr[MAX_ARGS]){
	int beginIndex = 0;
	int endIndex;
	int maxWords = MAX_ARGS;
	int wordCnt = 0;

	while(1){
		while(isspace(str[beginIndex])){
			++beginIndex;
		}
		if(str[beginIndex] == '\0')
			break;
			endIndex = beginIndex;
		while (str[endIndex] && !isspace(str[endIndex])){
			++endIndex;
		}
		int len = endIndex - beginIndex;
		char *tmp = calloc(len + 1, sizeof(char));
		memcpy(tmp, &str[beginIndex], len);
		arr[wordCnt++] = tmp;
		//PrintAndLog("cnt: %d, %s",wordCnt-1, arr[wordCnt-1]);
		beginIndex = endIndex;
		if (wordCnt == maxWords)
			break;
	}
	return wordCnt;
}

int CmdCrc(const char *Cmd)
{
	char name[] = {"reveng "};
	char Cmd2[50 + 7];
	memcpy(Cmd2, name, 7);
	memcpy(Cmd2 + 7, Cmd, 50);
	char *argv[MAX_ARGS];
	int argc = split(Cmd2, argv);
	//PrintAndLog("argc: %d, %s %s Cmd: %s",argc, argv[0], Cmd2, Cmd);
	reveng_main(argc, argv);
	for(int i = 0; i < argc; ++i){
		//puts(arr[i]);
		free(argv[i]);
	}

	return 0;
}

int GetModels(char *Models[], int *count, uint32_t *width){
	/* default values */
	static model_t model = {
		PZERO,		/* no CRC polynomial, user must specify */
		PZERO,		/* Init = 0 */
		P_BE,		/* RefIn = false, RefOut = false, plus P_RTJUST setting in reveng.h */
		PZERO,		/* XorOut = 0 */
		PZERO,		/* check value unused */
		NULL		/* no model name */
	};
	//int ibperhx = 8, obperhx = 8;
	//int rflags = 0, uflags = 0; /* search and UI flags */

	//unsigned long width = 0UL;
	//int c, mode = 0, args, psets, pass;
	//poly_t apoly, crc, qpoly = PZERO, *apolys, *pptr = NULL, *qptr = NULL;
	//model_t pset = model, *candmods, *mptr;
	//char *string;

	//myname = argv[0];

	/* stdin must be binary */
	#ifdef _WIN32
		_setmode(STDIN_FILENO, _O_BINARY);
	#endif /* _WIN32 */

	SETBMP();
	
	//pos=0;
	//optind=1;

	if (*width == 0) { //reveng -D
		*count = mcount();
		//PrintAndLog("Count: %d",*count);
		if(!*count){
			PrintAndLog("no preset models available");
			return 0;
		}
		for(int mode = 0; mode < *count; ++mode) {
			mbynum(&model, mode);
			mcanon(&model);
			size_t size = (model.name && *model.name) ? strlen(model.name) : 6;
			//PrintAndLog("Size: %d, %s",size,model.name);
			char *tmp = calloc(size+1, sizeof(char));
			if (tmp==NULL){
				PrintAndLog("out of memory?");
				return 0;
			}
			memcpy(tmp, model.name, size);
			Models[mode] = tmp;
			//ufound(&model);
		}
	} else { //reveng -s

	}
	//PrintAndLog("DONE");
	return 1;
}

//test call to GetModels
int CmdrevengTest(const char *Cmd){
	char *Models[80];
	int count = 0;
	uint32_t width = 0;
	int ans = GetModels(Models, &count, &width);
	if (!ans) return 0;
	
	PrintAndLog("Count: %d",count);
	for (int i = 0; i < count; i++){
		PrintAndLog("Model %d: %s",i,Models[i]);
		free(Models[i]);
	}
	return 1;
}
