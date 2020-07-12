//-----------------------------------------------------------------------------
// Copyright (C) 2009 Michael Gernoth <michael at gernoth.net>
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// UI utilities
//-----------------------------------------------------------------------------

#include <stdbool.h>
#ifndef EXTERNAL_PRINTANDLOG
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <readline/readline.h>
#include <pthread.h>
#include "util.h"
#endif

#include "ui.h"

double CursorScaleFactor = 1;
int PlotGridX=0, PlotGridY=0, PlotGridXdefault= 64, PlotGridYdefault= 64, CursorCPos= 0, CursorDPos= 0;
bool flushAfterWrite = false;  //buzzy
int GridOffset = 0;
bool GridLocked = false;
bool showDemod = true;

static char *logfilename = "proxmark3.log";

#ifndef EXTERNAL_PRINTANDLOG
static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void PrintAndLogEx(logLevel_t level, char *fmt, ...) {

	// skip debug messages if client debugging is turned off i.e. 'DATA SETDEBUG 0' 
//	if (g_debugMode	== 0 && level == DEBUG)
//		return;
	
	char buffer[MAX_PRINT_BUFFER] = {0};
	char buffer2[MAX_PRINT_BUFFER] = {0};
	char prefix[20] = {0};
	char *token = NULL;
	int size = 0;
						//   {NORMAL, SUCCESS, INFO, FAILED, WARNING, ERR, DEBUG}
	static char *prefixes[7] = { "", "", "INFO: ", "FAILED: ", "WARNING: ", "ERROR: ", "#: "};
	
	switch( level ) {
		case FAILED:
			strncpy(prefix,_RED_(FAILED: ), sizeof(prefix)-1);
			break;
		case DEBUG:
			strncpy(prefix,_BLUE_(#: ), sizeof(prefix)-1);			
			break;
		case SUCCESS: 
			strncpy(prefix,_GREEN_( ), sizeof(prefix)-1);
			break;
		case WARNING:
			strncpy(prefix,_CYAN_(WARNING: ), sizeof(prefix)-1);
			break;		
		default:
			strncpy(prefix, prefixes[level], sizeof(prefix)-1);
			break;
	}
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	// no prefixes for normal
	if ( level == NORMAL ) {
		PrintAndLog(buffer);
		return;
	}
	
	if (strchr(buffer, '\n')) {

		const char delim[2] = "\n";
			
		// line starts with newline
		if (buffer[0] == '\n') 
			PrintAndLog("");
		
		token = strtok(buffer, delim);
		
		while (token != NULL) {
			
			size = strlen(buffer2);
		
			if (strlen(token))
				snprintf(buffer2+size, sizeof(buffer2)-size, "%s%s\n", prefix, token);
			else
				snprintf(buffer2+size, sizeof(buffer2)-size, "\n");
			
			token = strtok(NULL, delim);
		}
		PrintAndLog(buffer2);
	} else {
		snprintf(buffer2, sizeof(buffer2), "%s%.*s", prefix, MAX_PRINT_BUFFER - 20, buffer);
		PrintAndLog(buffer2);
	}
}

void PrintAndLog(char *fmt, ...)
{
	char *saved_line;
	int saved_point;
	va_list argptr, argptr2;
	static FILE *logfile = NULL;
	static int logging=1;

	// lock this section to avoid interlacing prints from different threads
	pthread_mutex_lock(&print_lock);
  
	if (logging && !logfile) {
		logfile=fopen(logfilename, "a");
		if (!logfile) {
			fprintf(stderr, "Can't open logfile, logging disabled!\n");
			logging=0;
		}
	}

	// If there is an incoming message from the hardware (eg: lf hid read) in
	// the background (while the prompt is displayed and accepting user input),
	// stash the prompt and bring it back later.
#ifdef RL_STATE_READCMD
	// We are using GNU readline. libedit (OSX) doesn't support this flag.
	int need_hack = (rl_readline_state & RL_STATE_READCMD) > 0;

	if (need_hack) {
		saved_point = rl_point;
		saved_line = rl_copy_text(0, rl_end);
		rl_save_prompt();
		rl_replace_line("", 0);
		rl_redisplay();
	}
#endif
	
	va_start(argptr, fmt);
	va_copy(argptr2, argptr);
	vprintf(fmt, argptr);
	printf("          "); // cleaning prompt
	va_end(argptr);
	printf("\n");

#ifdef RL_STATE_READCMD
	// We are using GNU readline. libedit (OSX) doesn't support this flag.
	if (need_hack) {
		rl_restore_prompt();
		rl_replace_line(saved_line, 0);
		rl_point = saved_point;
		rl_redisplay();
		free(saved_line);
	}
#endif
	
	if (logging && logfile) {
		vfprintf(logfile, fmt, argptr2);
		fprintf(logfile,"\n");
		fflush(logfile);
	}
	va_end(argptr2);

	if (flushAfterWrite)  //buzzy
	{
		fflush(NULL);
	}
	//release lock
	pthread_mutex_unlock(&print_lock);  
}
#endif

void SetLogFilename(char *fn)
{
  logfilename = fn;
}

void SetFlushAfterWrite(bool flush_after_write) {
	flushAfterWrite = flush_after_write;
}

