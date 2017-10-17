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

void SendCommand(pm3_connection *conn, UsbCommand *c) {
	#ifdef COMMS_DEBUG
	printf("Sending %04x cmd\n", c->cmd);
	#endif

	if (conn->offline) {
      PrintAndLog("Sending bytes to proxmark failed - offline");
      return;
    }
  /**
	The while-loop below causes hangups at times, when the pm3 unit is unresponsive
	or disconnected. The main console thread is alive, but comm thread just spins here.
	Not good.../holiman
	**/
	while(conn->txcmd_pending);
	conn->txcmd = *c;
	conn->txcmd_pending = true;
}

/**
 * @brief This method should be called when sending a new command to the pm3. In case any old
 *  responses from previous commands are stored in the buffer, a call to this method should clear them.
 *  A better method could have been to have explicit command-ACKS, so we can know which ACK goes to which
 *  operation. Right now we'll just have to live with this.
 */
void clearCommandBuffer(pm3_connection* conn)
{
	if (!conn) return;
	//This is a very simple operation
    conn->cmd_tail = conn->cmd_head;
}

/**
 * @brief storeCommand stores a USB command in a circular buffer
 * @param UC
 */
void storeCommand(pm3_connection *conn, UsbCommand *command)
{
    if (!conn) return;
    if( (conn->cmd_head+1) % CMD_BUFFER_SIZE == conn->cmd_tail)
    {
        //If these two are equal, we're about to overwrite in the
        // circular buffer.
        PrintAndLog("WARNING: Command buffer about to overwrite command! This needs to be fixed!");
    }
    //Store the command at the 'head' location
    UsbCommand* destination = &conn->cmdBuffer[conn->cmd_head];
    memcpy(destination, command, sizeof(UsbCommand));

    conn->cmd_head = (conn->cmd_head +1) % CMD_BUFFER_SIZE; //increment head and wrap
}


/**
 * @brief getCommand gets a command from an internal circular buffer.
 * @param conn reference to Proxmark3 connection
 * @param response location to write command
 * @return 1 if response was returned, 0 if nothing has been received
 */
int getCommand(pm3_connection *conn, UsbCommand* response)
{
    //If head == tail, there's nothing to read, or if we just got initialized
    if (!conn || conn->cmd_head == conn->cmd_tail){
        return 0;
    }
    //Pick out the next unread command
    UsbCommand* last_unread = &conn->cmdBuffer[conn->cmd_tail];
    memcpy(response, last_unread, sizeof(UsbCommand));
    //Increment tail - this is a circular buffer, so modulo buffer size
    conn->cmd_tail = (conn->cmd_tail +1 ) % CMD_BUFFER_SIZE;

    return 1;
}

//-----------------------------------------------------------------------------
// Entry point into our code: called whenever we received a packet over USB
// that we weren't necessarily expecting, for example a debug print.
//-----------------------------------------------------------------------------
void UsbCommandReceived(pm3_connection* conn, UsbCommand *UC)
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
	}

	case CMD_DEBUG_PRINT_INTEGERS: {
		PrintAndLog("#db# %08x, %08x, %08x       \r\n", UC->arg[0], UC->arg[1], UC->arg[2]);
		return;
	}

	case CMD_DOWNLOADED_RAW_ADC_SAMPLES_125K: {
		// FIXME: This does unsanitised copies into memory when we don't know
		// the size of the buffer.
		if (conn->sample_buf) {
			memcpy(conn->sample_buf+(UC->arg[0]),UC->d.asBytes,UC->arg[1]);
		}
		return;
	}
	
	default: {
		storeCommand(conn, UC);
		return;
	}
	}
}

// Gets a single command from a proxmark3 device. This should never be used
// with the full client.
//
// @param conn A pm3_connection structure.
// @param command A buffer to store the received command.
bool ReceiveCommand(pm3_connection* conn, UsbCommand* command) {
	// Local recieve buffer
	size_t rxlen;
	byte_t rx[sizeof(UsbCommand)];
	byte_t* prx = rx;

	while (conn->run) {
		rxlen = 0;
		if (uart_receive(conn->port, prx, sizeof(UsbCommand) - (prx-rx), &rxlen)) {
			prx += rxlen;
			if (prx-rx < sizeof(UsbCommand)) {
				continue;
			}
			
			memcpy(command, rx, sizeof(UsbCommand));
			return true;
		}
		
		if (prx == rx) {
			// We got no complete command while waiting, give up control
			return false;
		}
	}

	// did not get a complete command before being cancelled.
	return false;
}

// Worker thread for processing incoming events from the PM3
void *uart_receiver(void *targ) {
	pm3_connection *conn = (pm3_connection*)targ;
	UsbCommand rx;

	while (conn->run) {
		// Lock up receives, in case they try to take it away from us.
		pthread_mutex_lock(&conn->recv_lock);

		if (conn->port == NULL) {
			// Our port disappeared, stall. This code path matters for the flasher,
			// where it is fiddling with the serial port under us.
			pthread_mutex_unlock(&conn->recv_lock);
			msleep(10);
			continue;
		}
		
		bool got_command = ReceiveCommand(conn, &rx);
		pthread_mutex_unlock(&conn->recv_lock);
		
		if (got_command) {
			UsbCommandReceived(conn, &rx);
		}

		// We aren't normally trying to transmit in the flasher when the port would
		// be reset, so we can just keep going at this point.
		if (conn->txcmd_pending) {
			if (!uart_send(conn->port, (byte_t*) &(conn->txcmd), sizeof(UsbCommand))) {
				PrintAndLog("Sending bytes to proxmark failed");
			}
			conn->txcmd_pending = false;
		}
	}

	pthread_exit(NULL);
	return NULL;
}

/**
 * Waits for a certain response type. This method waits for a maximum of
 * ms_timeout milliseconds for a specified response command.
 *@brief WaitForResponseTimeout
 * @param cmd command to wait for, or CMD_ANY to take any command.
 * @param response struct to copy received command into.
 * @param ms_timeout
 * @param show_warning
 * @return true if command was returned, otherwise false
 */
bool WaitForResponseTimeoutW(pm3_connection *conn, uint64_t cmd, UsbCommand* response, size_t ms_timeout, bool show_warning) {
	UsbCommand resp;

	#ifdef COMMS_DEBUG
	printf("Waiting for %04x cmd\n", cmd);
	#endif
	
	if (response == NULL) {
		response = &resp;
	}

	// Wait until the command is received
	for(size_t dm_seconds=0; dm_seconds < ms_timeout/10; dm_seconds++) {
		while(getCommand(conn, response)) {
			if (cmd == CMD_ANY || response->cmd == cmd) {
				return true;
			}
		}
		msleep(10); // XXX ugh
		if (dm_seconds == 200 && show_warning) { // Two seconds elapsed
			PrintAndLog("Waiting for a response from the proxmark...");
			PrintAndLog("Don't forget to cancel its operation first by pressing on the button");
		}
	}
	return false;
}

bool WaitForResponseTimeout(pm3_connection* conn, uint64_t cmd, UsbCommand* response, size_t ms_timeout) {
	return WaitForResponseTimeoutW(conn, cmd, response, ms_timeout, true);
}

bool WaitForResponse(pm3_connection* conn, uint64_t cmd, UsbCommand* response) {
	return WaitForResponseTimeout(conn, cmd, response, -1);
}

