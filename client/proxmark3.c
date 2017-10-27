//-----------------------------------------------------------------------------
// Copyright (C) 2009 Michael Gernoth <michael at gernoth.net>
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Main binary
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "proxmark3.h"
#include "util_posix.h"
#include "proxgui.h"
#include "cmdmain.h"
#include "uart.h"
#include "ui.h"
#include "util.h"
#include "cmdparser.h"
#include "cmdhw.h"
#include "whereami.h"

#ifdef _WIN32
#define SERIAL_PORT_H	"com3"
#else
#define SERIAL_PORT_H	"/dev/ttyACM0"
#endif

// a global mutex to prevent interlaced printing from different threads
pthread_mutex_t print_lock;

static serial_port sp;
static UsbCommand txcmd;
volatile static bool txcmd_pending = false;

void SendCommand(UsbCommand *c) {
	#if 0
		printf("Sending %d bytes\n", sizeof(UsbCommand));
	#endif

	if (offline) {
      PrintAndLog("Sending bytes to proxmark failed - offline");
      return;
    }
  /**
	The while-loop below causes hangups at times, when the pm3 unit is unresponsive
	or disconnected. The main console thread is alive, but comm thread just spins here.
	Not good.../holiman
	**/
	while(txcmd_pending);
	txcmd = *c;
	txcmd_pending = true;
}

struct receiver_arg {
	int run;
};

byte_t rx[sizeof(UsbCommand)];
byte_t* prx = rx;

static void *uart_receiver(void *targ) {
	struct receiver_arg *arg = (struct receiver_arg*)targ;
	size_t rxlen;

	while (arg->run) {
		rxlen = 0;
		if (uart_receive(sp, prx, sizeof(UsbCommand) - (prx-rx), &rxlen) && rxlen) {
			prx += rxlen;
			if (prx-rx < sizeof(UsbCommand)) {
				continue;
			}
			UsbCommandReceived((UsbCommand*)rx);
		}
		prx = rx;

		if(txcmd_pending) {
			if (!uart_send(sp, (byte_t*) &txcmd, sizeof(UsbCommand))) {
				PrintAndLog("Sending bytes to proxmark failed");
			}
			txcmd_pending = false;
		}
	}

	pthread_exit(NULL);
	return NULL;
}


void main_loop(char *script_cmds_file, char *script_cmd, bool usb_present) {
	struct receiver_arg rarg;
	char *cmd = NULL;
	pthread_t reader_thread;
	bool execCommand = (script_cmd != NULL);
	bool stdinOnPipe = !isatty(STDIN_FILENO);
	
	if (usb_present) {
		rarg.run = 1;
		pthread_create(&reader_thread, NULL, &uart_receiver, &rarg);
		// cache Version information now:
		CmdVersion(NULL);
	}

	// file with script
	FILE *script_file = NULL;
	char script_cmd_buf[256] = {0};  // iceman, needs lua script the same file_path_buffer as the rest

	if (script_cmds_file) {
		script_file = fopen(script_cmds_file, "r");
		if (script_file) {
			printf("executing commands from file: %s\n", script_cmds_file);
		}
	}
	
	read_history(".history");

	while(1)  {
		// If there is a script file
		if (script_file)
		{
			memset(script_cmd_buf, 0, sizeof(script_cmd_buf));
			if (!fgets(script_cmd_buf, sizeof(script_cmd_buf), script_file)) {
				fclose(script_file);
				script_file = NULL;
			} else {
				strcleanrn(script_cmd_buf, sizeof(script_cmd_buf));

				if ((cmd = strmcopy(script_cmd_buf)) != NULL) {
					printf(PROXPROMPT"%s\n", cmd);
				}
			}
		} else {
			// If there is a script command
			if (execCommand){
				if ((cmd = strmcopy(script_cmd)) != NULL) {
					printf(PROXPROMPT"%s\n", cmd);
				}

				execCommand = false;
			} else {
				// exit after exec command
				if (script_cmd)
					break;

				// if there is a pipe from stdin
				if (stdinOnPipe) {
					memset(script_cmd_buf, 0, sizeof(script_cmd_buf));
					if (!fgets(script_cmd_buf, sizeof(script_cmd_buf), stdin)) {
						printf("\nStdin end. Exit...\n");
						break;
					}
					strcleanrn(script_cmd_buf, sizeof(script_cmd_buf));

					if ((cmd = strmcopy(script_cmd_buf)) != NULL) {
						printf(PROXPROMPT"%s\n", cmd);
					}
					
				} else {		
					// read command from command prompt
					cmd = readline(PROXPROMPT);
				}
			}
		}
		
		// execute command
		if (cmd) {

			while(cmd[strlen(cmd) - 1] == ' ')
				cmd[strlen(cmd) - 1] = 0x00;
			
			if (cmd[0] != 0x00) {
				int ret = CommandReceived(cmd);
				add_history(cmd);
				if (ret == 99) {  // exit or quit
					break;
				}
			}
			free(cmd);
			cmd = NULL;
		} else {
			printf("\n");
			break;
		}
	}

	write_history(".history");
  
	if (usb_present) {
		rarg.run = 0;
		pthread_join(reader_thread, NULL);
	}
	
	if (script_file) {
		fclose(script_file);
		script_file = NULL;
	}
}

static void dumpAllHelp(int markdown)
{
  printf("\n%sProxmark3 command dump%s\n\n",markdown?"# ":"",markdown?"":"\n======================");
  printf("Some commands are available only if a Proxmark is actually connected.%s\n",markdown?"  ":"");
  printf("Check column \"offline\" for their availability.\n");
  printf("\n");
  command_t *cmds = getTopLevelCommandTable();
  dumpCommandsRecursive(cmds, markdown);
}

static char *my_executable_path = NULL;
static char *my_executable_directory = NULL;

const char *get_my_executable_path(void)
{
	return my_executable_path;
}

const char *get_my_executable_directory(void)
{
	return my_executable_directory;
}

static void set_my_executable_path(void)
{
	int path_length = wai_getExecutablePath(NULL, 0, NULL);
	if (path_length != -1) {
		my_executable_path = (char*)malloc(path_length + 1);
		int dirname_length = 0;
		if (wai_getExecutablePath(my_executable_path, path_length, &dirname_length) != -1) {
			my_executable_path[path_length] = '\0';
			my_executable_directory = (char *)malloc(dirname_length + 2);
			strncpy(my_executable_directory, my_executable_path, dirname_length+1);
			my_executable_directory[dirname_length+1] = '\0';
		}
	}
}

static void show_help(bool showFullHelp, char *command_line){
	printf("syntax: %s <port> [-h|-help|-m|-f|-flush|-w|-wait|-c|-command|-l|-lua] [cmd_script_file_name] [command][lua_script_name]\n", command_line);
	printf("\tLinux example:'%s /dev/ttyACM0'\n", command_line);
	printf("\tWindows example:'%s com3'\n\n", command_line);
	
	if (showFullHelp){
		printf("help: <-h|-help> Dump all interactive command's help at once.\n");
		printf("\t%s  -h\n\n", command_line);
		printf("markdown: <-m> Dump all interactive help at once in markdown syntax\n");
		printf("\t%s -m\n\n", command_line);
		printf("flush: <-f|-flush> Output will be flushed after every print.\n");
		printf("\t%s -f\n\n", command_line);
		printf("wait: <-w|-wait> 20sec waiting the serial port to appear in the OS\n");
		printf("\t%s "SERIAL_PORT_H" -w\n\n", command_line);
		printf("script: A script file with one proxmark3 command per line.\n\n");
		printf("command: <-c|-command> Execute one proxmark3 command.\n");
		printf("\t%s "SERIAL_PORT_H" -c \"hf mf chk 1* ?\"\n", command_line);
		printf("\t%s "SERIAL_PORT_H" -command \"hf mf nested 1 *\"\n\n", command_line);
		printf("lua: <-l|-lua> Execute lua script.\n");
		printf("\t%s "SERIAL_PORT_H" -l hf_read\n\n", command_line);
	}
}

int main(int argc, char* argv[]) {
	srand(time(0));
  
	bool usb_present = false;
	bool waitCOMPort = false;
	bool executeCommand = false;
	bool addLuaExec = false;
	char *script_cmds_file = NULL;
	char *script_cmd = NULL;
  
	if (argc < 2) {
		show_help(true, argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i],"-help") == 0) {
			show_help(false, argv[0]);
			dumpAllHelp(0);
			return 0;
		}
		
		if (strcmp(argv[i], "-m") == 0) {
			dumpAllHelp(1);
			return 0;
		}
		
		if(strcmp(argv[i],"-f") == 0 || strcmp(argv[i],"-flush") == 0){
			printf("Output will be flushed after every print.\n");
			flushAfterWrite = 1;
		}
		
		if(strcmp(argv[i],"-w") == 0 || strcmp(argv[i],"-wait") == 0){
			waitCOMPort = true;
		}

		if(strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"-command") == 0){
			executeCommand = true;
		}

		if(strcmp(argv[i],"-l") == 0 || strcmp(argv[i],"-lua") == 0){
			executeCommand = true;
			addLuaExec = true;
		}
	}

	// If the user passed the filename of the 'script' to execute, get it from last parameter
	if (argc > 2 && argv[argc - 1] && argv[argc - 1][0] != '-') {
		if (executeCommand){
			script_cmd = argv[argc - 1];
			
			while(script_cmd[strlen(script_cmd) - 1] == ' ')
				script_cmd[strlen(script_cmd) - 1] = 0x00;
			
			if (strlen(script_cmd) == 0) {
				script_cmd = NULL;
			} else {
				if (addLuaExec){
					// add "script run " to command
					char *ctmp = NULL;
					int len = strlen(script_cmd) + 11 + 1;
					if ((ctmp = (char*) malloc(len)) != NULL) {
						memset(ctmp, 0, len);
						strcpy(ctmp, "script run ");
						strcpy(&ctmp[11], script_cmd);
						script_cmd = ctmp;
					}
				}
				
				printf("Execute command from commandline: %s\n", script_cmd);
			}
		} else {
			script_cmds_file = argv[argc - 1];
		}
	}

	// check command
	if (executeCommand && (!script_cmd || strlen(script_cmd) == 0)){
		printf("ERROR: execute command: command not found.\n");
		return 2;
	}
	
	// set global variables
	set_my_executable_path();
	
	// open uart
	if (!waitCOMPort) {
		sp = uart_open(argv[1]);
	} else {
		printf("Waiting for Proxmark to appear on %s ", argv[1]);
		fflush(stdout);
		int openCount = 0;
		do {
			sp = uart_open(argv[1]);
			msleep(1000);
			printf(".");
			fflush(stdout);
		} while(++openCount < 20 && (sp == INVALID_SERIAL_PORT || sp == CLAIMED_SERIAL_PORT));
		printf("\n");
	}

	// check result of uart opening
	if (sp == INVALID_SERIAL_PORT) {
		printf("ERROR: invalid serial port\n");
		usb_present = false;
		offline = 1;
	} else if (sp == CLAIMED_SERIAL_PORT) {
		printf("ERROR: serial port is claimed by another process\n");
		usb_present = false;
		offline = 1;
	} else {
		usb_present = true;
		offline = 0;
	}
	
	// create a mutex to avoid interlacing print commands from our different threads
	pthread_mutex_init(&print_lock, NULL);

#ifdef HAVE_GUI
#ifdef _WIN32
	InitGraphics(argc, argv, script_cmds_file, script_cmd, usb_present);
	MainGraphics();
#else
	char* display = getenv("DISPLAY");

	if (display && strlen(display) > 1)
	{
		InitGraphics(argc, argv, script_cmds_file, script_cmd, usb_present);
		MainGraphics();
	}
	else
	{
		main_loop(script_cmds_file, script_cmd, usb_present);
	}
#endif
#else
	main_loop(script_cmds_file, script_cmd, usb_present);
#endif	

	// Clean up the port
	if (usb_present) {
		uart_close(sp);
	}

	// clean up mutex
	pthread_mutex_destroy(&print_lock);

	exit(0);
}
