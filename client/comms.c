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

#define _POSIX_C_SOURCE 199309L // need clock_gettime()

#include "comms.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>

#include "uart.h"
#include "ui.h"
#include "common.h"
#include "util_darwin.h"
#include "util_posix.h"


// Serial port that we are communicating with the PM3 on.
static serial_port sp = NULL;
static char *serial_port_name = NULL;

// If TRUE, then there is no active connection to the PM3, and we will drop commands sent.
static bool offline;

typedef struct {
	bool run; // If TRUE, continue running the uart_communication thread
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
#define CMD_BUFFER_SIZE 50
#define CMD_BUFFER_CHECK_TIME 10 // maximum time (in ms) to wait in getCommand()

static UsbCommand rxBuffer[CMD_BUFFER_SIZE];

// Points to the next empty position to write to
static int cmd_head = 0;

// Points to the position of the last unread command
static int cmd_tail = 0;

// to lock rxBuffer operations from different threads
static pthread_mutex_t rxBufferMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t rxBufferSig = PTHREAD_COND_INITIALIZER;

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
	printf("Sending %04" PRIx64 " cmd\n", c->cmd);
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
void clearCommandBuffer() {
	//This is a very simple operation
	pthread_mutex_lock(&rxBufferMutex);
	cmd_tail = cmd_head;
	pthread_mutex_unlock(&rxBufferMutex);
}

/**
 * @brief storeCommand stores a USB command in a circular buffer
 * @param UC
 */
static void storeCommand(UsbCommand *command) {
	pthread_mutex_lock(&rxBufferMutex);
	if ((cmd_head + 1) % CMD_BUFFER_SIZE == cmd_tail) {
		// If these two are equal, we're about to overwrite in the
		// circular buffer.
		PrintAndLog("WARNING: Command buffer about to overwrite command! This needs to be fixed!");
	}

	// Store the command at the 'head' location
	UsbCommand* destination = &rxBuffer[cmd_head];
	memcpy(destination, command, sizeof(UsbCommand));

	cmd_head = (cmd_head + 1) % CMD_BUFFER_SIZE; //increment head and wrap
	pthread_cond_signal(&rxBufferSig); // tell main thread that a new command can be retreived
	pthread_mutex_unlock(&rxBufferMutex);
}


/**
 * @brief getCommand gets a command from an internal circular buffer.
 * @param response location to write command
 * @return 1 if response was returned, 0 if nothing has been received in time
 */
static int getCommand(UsbCommand* response, uint32_t ms_timeout) {

	struct timespec end_time;
	clock_gettime(CLOCK_REALTIME, &end_time);
	end_time.tv_sec += ms_timeout / 1000;
	end_time.tv_nsec += (ms_timeout % 1000) * 1000000;
	if (end_time.tv_nsec > 1000000000) {
		end_time.tv_nsec -= 1000000000;
		end_time.tv_sec += 1;
	}
	pthread_mutex_lock(&rxBufferMutex);
	int res = 0;
	while (cmd_head == cmd_tail && !res) {
		res = pthread_cond_timedwait(&rxBufferSig, &rxBufferMutex, &end_time);
	}
	if (res) { // timeout
		pthread_mutex_unlock(&rxBufferMutex);
		return 0;
	}

	// Pick out the next unread command
	UsbCommand* last_unread = &rxBuffer[cmd_tail];
	memcpy(response, last_unread, sizeof(UsbCommand));
	// Increment tail - this is a circular buffer, so modulo buffer size
	cmd_tail = (cmd_tail + 1) % CMD_BUFFER_SIZE;

	pthread_mutex_unlock(&rxBufferMutex);
	return 1;
}


//----------------------------------------------------------------------------------
// Entry point into our code: called whenever we received a packet over USB.
// Handle debug commands directly, store all other commands in circular buffer.
//----------------------------------------------------------------------------------
static void UsbCommandReceived(UsbCommand *UC) {
	switch (UC->cmd) {
		// First check if we are handling a debug message
		case CMD_DEBUG_PRINT_STRING: {
			char s[USB_CMD_DATA_SIZE+1];
			memset(s, 0x00, sizeof(s));
			size_t len = MIN(UC->arg[0], USB_CMD_DATA_SIZE);
			memcpy(s, UC->d.asBytes,len);
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


static bool receive_from_serial(serial_port sp, uint8_t *rx_buf, size_t len, size_t *received_len) {
	size_t bytes_read = 0;
	*received_len = 0;
	// we eventually need to call uart_receive several times because it may timeout in the middle of a transfer
	while (uart_receive(sp, rx_buf + *received_len, len - *received_len, &bytes_read) && bytes_read && *received_len < len) {
		#ifdef COMMS_DEBUG
		if (bytes_read != len - *received_len) {
			printf("uart_receive() returned true but not enough bytes could be received. received: %zd, wanted to receive: %zd, already received before: %zd\n",
				bytes_read, len - *received_len, *received_len);
		}
		#endif
		*received_len += bytes_read;
		bytes_read = 0;
	}
	return (*received_len == len);
}


static void
#ifdef __has_attribute
#if __has_attribute(force_align_arg_pointer)
__attribute__((force_align_arg_pointer))
#endif
#endif
*uart_communication(void *targ) {
	communication_arg_t *conn = (communication_arg_t*)targ;
	uint8_t rx[sizeof(UsbCommand)];
	size_t rxlen = 0;
	uint8_t *prx = rx;
	UsbCommand *command = (UsbCommand*)rx;
	UsbResponse *response = (UsbResponse*)rx;

#if defined(__MACH__) && defined(__APPLE__)
	disableAppNap("Proxmark3 polling UART");
#endif

	while (conn->run) {
		bool ACK_received = false;
		prx = rx;
		size_t bytes_to_read = offsetof(UsbResponse, d);  // the fixed part of a new style UsbResponse. Otherwise this will be cmd and arg[0] (64 bit each)
		if (receive_from_serial(sp, prx, bytes_to_read, &rxlen)) {
			prx += rxlen;
			if (response->cmd & CMD_VARIABLE_SIZE_FLAG) { // new style response with variable size
#ifdef COMMS_DEBUG
				PrintAndLog("received new style response %04" PRIx16 ", datalen = %zd, arg[0] = %08" PRIx32 ", arg[1] = %08" PRIx32 ", arg[2] = %08" PRIx32,
					response->cmd, response->datalen, response->arg[0], response->arg[1], response->arg[2]);
#endif
				bytes_to_read = response->datalen;
				if (receive_from_serial(sp, prx, bytes_to_read, &rxlen)) {
					UsbCommand resp;
					resp.cmd = response->cmd & ~CMD_VARIABLE_SIZE_FLAG;  // remove the flag
					resp.arg[0] = response->arg[0];
					resp.arg[1] = response->arg[1];
					resp.arg[2] = response->arg[2];
					memcpy(&resp.d.asBytes, &response->d.asBytes, response->datalen);
					UsbCommandReceived(&resp);
					if (resp.cmd == CMD_ACK) {
						ACK_received = true;
					}
				}
			} else { // old style response uses same data structure as commands. Fixed size.
#ifdef COMMS_DEBUG
				PrintAndLog("received old style response %016" PRIx64 ", arg[0] = %016" PRIx64, command->cmd, command->arg[0]);
#endif
				bytes_to_read = sizeof(UsbCommand) - bytes_to_read;
				if (receive_from_serial(sp, prx, bytes_to_read, &rxlen)) {
					UsbCommandReceived(command);
					if (command->cmd == CMD_ACK) {
						ACK_received = true;
					}
				}
			}
		}

		pthread_mutex_lock(&txBufferMutex);
		// if we received an ACK the PM has done its job and waits for another command.
		// We therefore can wait here as well until a new command is to be transmitted.
		// The advantage is that the next command will be transmitted immediately without the need to wait for a receive timeout
		if (ACK_received) {
			while (!txBuffer_pending) {
				pthread_cond_wait(&txBufferSig, &txBufferMutex);
			}
		}
		if (txBuffer_pending) {
			if (!uart_send(sp, (uint8_t*) &txBuffer, sizeof(UsbCommand))) {
				PrintAndLog("Sending bytes to proxmark failed");
			}
			txBuffer_pending = false;
		}
		pthread_cond_signal(&txBufferSig); // tell main thread that txBuffer is empty
		pthread_mutex_unlock(&txBufferMutex);
	}

#if defined(__MACH__) && defined(__APPLE__)
	enableAppNap();
#endif

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
bool GetFromBigBuf(uint8_t *dest, int bytes, int start_index, UsbCommand *response, size_t ms_timeout, bool show_warning) {

	uint64_t start_time = msclock();

	UsbCommand c = {CMD_DOWNLOAD_RAW_ADC_SAMPLES_125K, {start_index, bytes, 0}};
	SendCommand(&c);

	UsbCommand resp;
	if (response == NULL) {
		response = &resp;
	}

	int bytes_completed = 0;
	while (true) {
		if (msclock() - start_time > ms_timeout) {
			break; // timeout
		}
		if (msclock() - start_time > 2000 && show_warning) {
			// 2 seconds elapsed (but this doesn't mean the timeout was exceeded)
			PrintAndLog("Waiting for a response from the proxmark...");
			PrintAndLog("You can cancel this operation by pressing the pm3 button");
			show_warning = false;
		}
		if (getCommand(response, CMD_BUFFER_CHECK_TIME)) {
			if (response->cmd == CMD_DOWNLOADED_RAW_ADC_SAMPLES_125K) {
				int copy_bytes = MIN(bytes - bytes_completed, response->arg[1]);
				memcpy(dest + response->arg[0], response->d.asBytes, copy_bytes);
				bytes_completed += copy_bytes;
			} else if (response->cmd == CMD_ACK) {
				return true;
			}
		}
	}

	return false;
}


bool GetFromFpgaRAM(uint8_t *dest, int bytes) {

	uint64_t start_time = msclock();

	UsbCommand c = {CMD_HF_PLOT, {0, 0, 0}};
	SendCommand(&c);

	UsbCommand response;

	int bytes_completed = 0;
	bool show_warning = true;
	while (true) {
		if (msclock() - start_time > 2000 && show_warning) {
			PrintAndLog("Waiting for a response from the proxmark...");
			PrintAndLog("You can cancel this operation by pressing the pm3 button");
			show_warning = false;
		}
		if (getCommand(&response, CMD_BUFFER_CHECK_TIME)) {
			if (response.cmd == CMD_DOWNLOADED_RAW_ADC_SAMPLES_125K) {
				int copy_bytes = MIN(bytes - bytes_completed, response.arg[1]);
				memcpy(dest + response.arg[0], response.d.asBytes, copy_bytes);
				bytes_completed += copy_bytes;
			} else if (response.cmd == CMD_ACK) {
				return true;
			}
		}
	}

	return false;
}


bool OpenProxmark(void *port, bool wait_for_port, int timeout) {
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
		} while (++openCount < timeout && (sp == INVALID_SERIAL_PORT || sp == CLAIMED_SERIAL_PORT));
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
		pthread_create(&USB_communication_thread, NULL, &uart_communication, &conn);
		return true;
	}
}


void CloseProxmark(void) {
	conn.run = false;

#ifdef __BIONIC__
	// In Android O and later, if an invalid pthread_t is passed to pthread_join, it calls fatal().
	// https://github.com/aosp-mirror/platform_bionic/blob/ed16b344e75f422fb36fbfd91fb30de339475880/libc/bionic/pthread_internal.cpp#L116-L128
	//
	// In Bionic libc, pthread_t is an integer.

	if (USB_communication_thread != 0) {
		pthread_join(USB_communication_thread, NULL);
	}
#else
	// pthread_t is a struct on other libc, treat as an opaque memory reference
	pthread_join(USB_communication_thread, NULL);
#endif

	if (sp) {
		uart_close(sp);
	}

	// Clean up our state
	sp = NULL;
	serial_port_name = NULL;
#ifdef __BIONIC__
	memset(&USB_communication_thread, 0, sizeof(pthread_t));
#endif
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

	uint64_t start_time = msclock();

	if (response == NULL) {
		response = &resp;
	}

	// Wait until the command is received
	while (true) {
		if (ms_timeout != -1 && msclock() > start_time + ms_timeout) {
			break; // timeout
		}
		if (msclock() - start_time > 2000 && show_warning) {
			// 2 seconds elapsed (but this doesn't mean the timeout was exceeded)
			PrintAndLog("Waiting for a response from the proxmark...");
			PrintAndLog("You can cancel this operation by pressing the pm3 button");
			show_warning = false;
		}
		if (getCommand(response, CMD_BUFFER_CHECK_TIME)) {
			if (cmd == CMD_UNKNOWN || response->cmd == cmd) {
				return true;
			}
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

