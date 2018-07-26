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

#include "comms.h"

#include <pthread.h>
#if defined(__linux__) && !defined(NO_UNLINK)
#include <unistd.h>		// for unlink()
#endif
#include "uart.h"
#include "ui.h"
#include "common.h"
#include "util_posix.h"


// Serial port that we are communicating with the PM3 on.
static serial_port sp = NULL;
static char *serial_port_name = NULL;

// If TRUE, then there is no active connection to the PM3, and we will drop commands sent.
static bool offline;

typedef struct {
	bool run; // If TRUE, continue running the uart_communication thread
	bool block_after_ACK; // if true, block after receiving an ACK package
} communication_arg_t;

static communication_arg_t conn;
static pthread_t USB_communication_thread;

// Transmit buffer.
static UsbCommand txBuffer;
static bool txBuffer_pending = false;
static pthread_mutex_t txBufferMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t txBufferSig = PTHREAD_COND_INITIALIZER;

// Used by UsbReceiveCommand as a ring buffer for messages that are yet to be
// processed by a command handler (WaitForResponse{,Timeout})
static UsbCommand rxBuffer[CMD_BUFFER_SIZE];

// Points to the next empty position to write to
static int cmd_head = 0;

// Points to the position of the last unread command
static int cmd_tail = 0;

// to lock rxBuffer operations from different threads
static pthread_mutex_t rxBufferMutex = PTHREAD_MUTEX_INITIALIZER;

// These wrappers are required because it is not possible to access a static
// global variable outside of the context of a single file.

void SetOffline(bool new_offline) {
	offline = new_offline;
}

bool IsOffline() {
	return offline;
}

void SendCommand(UsbCommand *c) {
	#ifdef COMMS_DEBUG
	printf("Sending %04x cmd\n", c->cmd);
	#endif

	if (offline) {
		PrintAndLog("Sending bytes to proxmark failed - offline");
		return;
    }

	pthread_mutex_lock(&txBufferMutex);
	/**
	This causes hangups at times, when the pm3 unit is unresponsive or disconnected. The main console thread is alive, 
	but comm thread just spins here. Not good.../holiman
	**/
	while (txBuffer_pending) {
		pthread_cond_wait(&txBufferSig, &txBufferMutex); // wait for communication thread to complete sending a previous commmand
	}

	txBuffer = *c;
	txBuffer_pending = true;
	pthread_cond_signal(&txBufferSig); // tell communication thread that a new command can be send

	pthread_mutex_unlock(&txBufferMutex);

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
	pthread_mutex_lock(&rxBufferMutex);
	cmd_tail = cmd_head;
	pthread_mutex_unlock(&rxBufferMutex);
}

/**
 * @brief storeCommand stores a USB command in a circular buffer
 * @param UC
 */
static void storeCommand(UsbCommand *command)
{
	pthread_mutex_lock(&rxBufferMutex);
	if( (cmd_head+1) % CMD_BUFFER_SIZE == cmd_tail)
	{
		// If these two are equal, we're about to overwrite in the
		// circular buffer.
		PrintAndLog("WARNING: Command buffer about to overwrite command! This needs to be fixed!");
	}

	// Store the command at the 'head' location
	UsbCommand* destination = &rxBuffer[cmd_head];
	memcpy(destination, command, sizeof(UsbCommand));

	cmd_head = (cmd_head +1) % CMD_BUFFER_SIZE; //increment head and wrap
	pthread_mutex_unlock(&rxBufferMutex);
}


/**
 * @brief getCommand gets a command from an internal circular buffer.
 * @param response location to write command
 * @return 1 if response was returned, 0 if nothing has been received
 */
static int getCommand(UsbCommand* response)
{
	pthread_mutex_lock(&rxBufferMutex);
	//If head == tail, there's nothing to read, or if we just got initialized
	if (cmd_head == cmd_tail){
		pthread_mutex_unlock(&rxBufferMutex);
		return 0;
	}

	//Pick out the next unread command
	UsbCommand* last_unread = &rxBuffer[cmd_tail];
	memcpy(response, last_unread, sizeof(UsbCommand));
	//Increment tail - this is a circular buffer, so modulo buffer size
	cmd_tail = (cmd_tail + 1) % CMD_BUFFER_SIZE;

	pthread_mutex_unlock(&rxBufferMutex);
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


static void
#ifdef __has_attribute
#if __has_attribute(force_align_arg_pointer)
__attribute__((force_align_arg_pointer)) 
#endif
#endif
*uart_communication(void *targ) {
	communication_arg_t *conn = (communication_arg_t*)targ;
	size_t rxlen;
	UsbCommand rx;
	UsbCommand *prx = &rx;

	while (conn->run) {
		rxlen = 0;
		bool ACK_received = false;
		if (uart_receive(sp, (uint8_t *)prx, sizeof(UsbCommand) - (prx-&rx), &rxlen) && rxlen) {
			prx += rxlen;
			if (prx-&rx < sizeof(UsbCommand)) {
				continue;
			}
			UsbCommandReceived(&rx);
			if (rx.cmd == CMD_ACK) {
				ACK_received = true;
			}
		}
		prx = &rx;

		
		pthread_mutex_lock(&txBufferMutex);

		if (conn->block_after_ACK) {
			// if we just received an ACK, wait here until a new command is to be transmitted
			if (ACK_received) {
				while (!txBuffer_pending) {
					pthread_cond_wait(&txBufferSig, &txBufferMutex);
				}
			}
		}
				
		if(txBuffer_pending) {
			if (!uart_send(sp, (uint8_t*) &txBuffer, sizeof(UsbCommand))) {
				PrintAndLog("Sending bytes to proxmark failed");
			}
			txBuffer_pending = false;
			pthread_cond_signal(&txBufferSig); // tell main thread that txBuffer is empty
		}

		pthread_mutex_unlock(&txBufferMutex);
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

	
bool OpenProxmark(void *port, bool wait_for_port, int timeout, bool flash_mode) {
	char *portname = (char *)port;
	if (!wait_for_port) {
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
		sp = NULL;
		serial_port_name = NULL;
		return false;
	} else if (sp == CLAIMED_SERIAL_PORT) {
		printf("ERROR: serial port is claimed by another process\n");
		sp = NULL;
		serial_port_name = NULL;
		return false;
	} else {
		// start the USB communication thread
		serial_port_name = portname;
		conn.run = true;
		conn.block_after_ACK = flash_mode;
		pthread_create(&USB_communication_thread, NULL, &uart_communication, &conn);
		return true;
	}
}


void CloseProxmark(void) {
	conn.run = false;
	pthread_join(USB_communication_thread, NULL);

	if (sp) {
		uart_close(sp);
	}

#if defined(__linux__) && !defined(NO_UNLINK)
	// Fix for linux, it seems that it is extremely slow to release the serial port file descriptor /dev/*
	//
	// This may be disabled at compile-time with -DNO_UNLINK (used for a JNI-based serial port on Android).
	if (serial_port_name) {
		unlink(serial_port_name);
	}
#endif

	// Clean up our state
	sp = NULL;
	serial_port_name = NULL;
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

