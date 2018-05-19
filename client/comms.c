//-----------------------------------------------------------------------------
// Copyright (C) 2009 Michael Gernoth <michael at gernoth.net>
// Copyright (C) 2010 iZsh <izsh at fail0verflow.com>
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Code for communicating with the proxmark3 hardware.
//-----------------------------------------------------------------------------

#include <pthread.h>

#include "comms.h"
#include "uart.h"
#include "ui.h"
#include "common.h"
#include "util_posix.h"

// Declare globals.

// Serial port that we are communicating with the PM3 on.
static serial_port sp;

// If TRUE, then there is no active connection to the PM3, and we will drop commands sent.
static bool offline;

// Transmit buffer.
// TODO: Use locks and execute this on the main thread, rather than the receiver
// thread.  Running on the main thread means we need to be careful in the
// flasher, as it means SendCommand is no longer async, and can't be used as a
// buffer for a pending command when the connection is re-established.
static UsbCommand txcmd;
volatile static bool txcmd_pending = false;

// Used by UsbReceiveCommand as a ring buffer for messages that are yet to be
// processed by a command handler (WaitForResponse{,Timeout})
static UsbCommand cmdBuffer[CMD_BUFFER_SIZE];

// Points to the next empty position to write to
static int cmd_head = 0;

// Points to the position of the last unread command
static int cmd_tail = 0;

// to lock cmdBuffer operations from different threads
static pthread_mutex_t cmdBufferMutex = PTHREAD_MUTEX_INITIALIZER;

// These wrappers are required because it is not possible to access a static
// global variable outside of the context of a single file.

void SetOffline(bool new_offline) {
	offline = new_offline;
}

bool IsOffline() {
	return offline;
}

bool OpenProxmark(char *portname, bool waitCOMPort, int timeout) {
	if (!waitCOMPort) {
		sp = uart_open(portname);
	} else {
		printf("Waiting for Proxmark to appear on %s ", portname);
		fflush(stdout);
		int openCount = 0;
		do {
			sp = uart_open(portname);
			msleep(1000);
			printf(".");
			fflush(stdout);
		} while(++openCount < timeout && (sp == INVALID_SERIAL_PORT || sp == CLAIMED_SERIAL_PORT));
		printf("\n");
	}

	// check result of uart opening
	if (sp == INVALID_SERIAL_PORT) {
		printf("ERROR: invalid serial port\n");
		return false;
	} else if (sp == CLAIMED_SERIAL_PORT) {
		printf("ERROR: serial port is claimed by another process\n");
		return false;
	} else {
		return true;
	}
}

void CloseProxmark(void) {
	uart_close(sp);
}

void SendCommand(UsbCommand *c) {
	#ifdef COMMS_DEBUG
	printf("Sending %04x cmd\n", c->cmd);
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


/**
 * @brief This method should be called when sending a new command to the pm3. In case any old
 *  responses from previous commands are stored in the buffer, a call to this method should clear them.
 *  A better method could have been to have explicit command-ACKS, so we can know which ACK goes to which
 *  operation. Right now we'll just have to live with this.
 */
void clearCommandBuffer()
{
	//This is a very simple operation
	pthread_mutex_lock(&cmdBufferMutex);
	cmd_tail = cmd_head;
	pthread_mutex_unlock(&cmdBufferMutex);
}

/**
 * @brief storeCommand stores a USB command in a circular buffer
 * @param UC
 */
void storeCommand(UsbCommand *command)
{
	pthread_mutex_lock(&cmdBufferMutex);
	if( (cmd_head+1) % CMD_BUFFER_SIZE == cmd_tail)
	{
		// If these two are equal, we're about to overwrite in the
		// circular buffer.
		PrintAndLog("WARNING: Command buffer about to overwrite command! This needs to be fixed!");
	}

	// Store the command at the 'head' location
	UsbCommand* destination = &cmdBuffer[cmd_head];
	memcpy(destination, command, sizeof(UsbCommand));

	cmd_head = (cmd_head +1) % CMD_BUFFER_SIZE; //increment head and wrap
	pthread_mutex_unlock(&cmdBufferMutex);
}


/**
 * @brief getCommand gets a command from an internal circular buffer.
 * @param response location to write command
 * @return 1 if response was returned, 0 if nothing has been received
 */
int getCommand(UsbCommand* response)
{
	pthread_mutex_lock(&cmdBufferMutex);
	//If head == tail, there's nothing to read, or if we just got initialized
	if (cmd_head == cmd_tail){
		pthread_mutex_unlock(&cmdBufferMutex);
		return 0;
	}

	//Pick out the next unread command
	UsbCommand* last_unread = &cmdBuffer[cmd_tail];
	memcpy(response, last_unread, sizeof(UsbCommand));
	//Increment tail - this is a circular buffer, so modulo buffer size
	cmd_tail = (cmd_tail + 1) % CMD_BUFFER_SIZE;

	pthread_mutex_unlock(&cmdBufferMutex);
	return 1;
}


//----------------------------------------------------------------------------------
// Entry point into our code: called whenever we received a packet over USB.
// Handle debug commands directly, store all other commands in circular buffer.
//----------------------------------------------------------------------------------
static void UsbCommandReceived(UsbCommand *UC)
{
	switch(UC->cmd) {
		// First check if we are handling a debug message
		case CMD_DEBUG_PRINT_STRING: {
			char s[USB_CMD_DATA_SIZE+1];
			memset(s, 0x00, sizeof(s));
			size_t len = MIN(UC->arg[0],USB_CMD_DATA_SIZE);
			memcpy(s,UC->d.asBytes,len);
			PrintAndLog("#db# %s", s);
			return;
		} break;

		case CMD_DEBUG_PRINT_INTEGERS: {
			PrintAndLog("#db# %08x, %08x, %08x       \r\n", UC->arg[0], UC->arg[1], UC->arg[2]);
			return;
		} break;

		default:
 			storeCommand(UC);
			break;
	}

}


void
#ifdef __has_attribute
#if __has_attribute(force_align_arg_pointer)
__attribute__((force_align_arg_pointer)) 
#endif
#endif
*uart_receiver(void *targ) {
	receiver_arg *conn = (receiver_arg*)targ;
	size_t rxlen;
	uint8_t rx[sizeof(UsbCommand)];
	uint8_t *prx = rx;

	while (conn->run) {
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
			if (!uart_send(sp, (uint8_t*) &txcmd, sizeof(UsbCommand))) {
				PrintAndLog("Sending bytes to proxmark failed");
			}
			txcmd_pending = false;
		}
	}

	pthread_exit(NULL);
	return NULL;
}



/**
 * Data transfer from Proxmark to client. This method times out after
 * ms_timeout milliseconds.
 * @brief GetFromBigBuf
 * @param dest Destination address for transfer
 * @param bytes number of bytes to be transferred
 * @param start_index offset into Proxmark3 BigBuf[]
 * @param response struct to copy last command (CMD_ACK) into
 * @param ms_timeout timeout in milliseconds
 * @param show_warning display message after 2 seconds
 * @return true if command was returned, otherwise false
 */
bool GetFromBigBuf(uint8_t *dest, int bytes, int start_index, UsbCommand *response, size_t ms_timeout, bool show_warning)
{
	UsbCommand c = {CMD_DOWNLOAD_RAW_ADC_SAMPLES_125K, {start_index, bytes, 0}};
	SendCommand(&c);

	uint64_t start_time = msclock();

	UsbCommand resp;
  	if (response == NULL) {
		response = &resp;
	}

	int bytes_completed = 0;
	while(true) {
		if (getCommand(response)) {
			if (response->cmd == CMD_DOWNLOADED_RAW_ADC_SAMPLES_125K) {
				int copy_bytes = MIN(bytes - bytes_completed, response->arg[1]);
				memcpy(dest + response->arg[0], response->d.asBytes, copy_bytes);
				bytes_completed += copy_bytes;
			} else if (response->cmd == CMD_ACK) {
				return true;
			}
		}

		if (msclock() - start_time > ms_timeout) {
			break;
		}

		if (msclock() - start_time > 2000 && show_warning) {
			PrintAndLog("Waiting for a response from the proxmark...");
			PrintAndLog("You can cancel this operation by pressing the pm3 button");
			show_warning = false;
		}
	}

	return false;
}


/**
 * Waits for a certain response type. This method waits for a maximum of
 * ms_timeout milliseconds for a specified response command.
 *@brief WaitForResponseTimeout
 * @param cmd command to wait for, or CMD_UNKNOWN to take any command.
 * @param response struct to copy received command into.
 * @param ms_timeout
 * @param show_warning display message after 2 seconds
 * @return true if command was returned, otherwise false
 */
bool WaitForResponseTimeoutW(uint32_t cmd, UsbCommand* response, size_t ms_timeout, bool show_warning) {

	UsbCommand resp;

	#ifdef COMMS_DEBUG
	printf("Waiting for %04x cmd\n", cmd);
	#endif

	if (response == NULL) {
		response = &resp;
	}

	uint64_t start_time = msclock();

	// Wait until the command is received
	while (true) {
		while(getCommand(response)) {
			if (cmd == CMD_UNKNOWN || response->cmd == cmd) {
				return true;
			}
		}

		if (msclock() - start_time > ms_timeout) {
			break;
		}

		if (msclock() - start_time > 2000 && show_warning) {
			// 2 seconds elapsed (but this doesn't mean the timeout was exceeded)
			PrintAndLog("Waiting for a response from the proxmark...");
			PrintAndLog("You can cancel this operation by pressing the pm3 button");
			show_warning = false;
		}
	}
	return false;
}


bool WaitForResponseTimeout(uint32_t cmd, UsbCommand* response, size_t ms_timeout) {
	return WaitForResponseTimeoutW(cmd, response, ms_timeout, true);
}

bool WaitForResponse(uint32_t cmd, UsbCommand* response) {
	return WaitForResponseTimeoutW(cmd, response, -1, true);
}

