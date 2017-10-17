#ifndef COMMS_H_
#define COMMS_H_

#include <stdbool.h>
#include <pthread.h>

#include "usb_cmd.h"
#include "uart.h"
#include "util_posix.h"

#ifndef CMD_BUFFER_SIZE
#define CMD_BUFFER_SIZE 50
#endif

#ifndef MAX_DEMOD_BUF_LEN
#define MAX_DEMOD_BUF_LEN (1024*128)
#endif

#ifndef BIGBUF_SIZE
#define BIGBUF_SIZE 40000
#endif

// Max graph trace len: 40000 (bigbuf) * 8 (at 1 bit per sample)
#define MAX_GRAPH_TRACE_LEN (40000 * 8)

/* pm3_connection: holds state related to a connection to a single PM3
 * All global state related to a single connection should make its way into this struct.
 */
typedef struct {
	// If TRUE, continue running the uart_reciever thread.
	bool run;
	
	// Serial port that we are communicating with the PM3 on.
	serial_port* port;
	
	// Lock around serial port receives
	pthread_mutex_t recv_lock;
	
	// If TRUE, then there is no active connection to the PM3, and we will drop commands sent.
	bool offline;
	
	// Transmit buffer.
	UsbCommand txcmd;
	bool txcmd_pending;
	
	// Used by UsbReceiveCommand as a ring buffer for messages that are yet to be
	// processed by a command handler (WaitForResponse{,Timeout})
	UsbCommand cmdBuffer[CMD_BUFFER_SIZE];

	// Points to the next empty position to write to
	int cmd_head;//Starts as 0
	
	// Points to the position of the last unread command
	int cmd_tail;//Starts as 0
	
	// Demodulation buffer, used by mainly low-frequency tags
	uint8_t DemodBuffer[MAX_DEMOD_BUF_LEN];
	size_t DemodBufferLen;
	int g_DemodStartIdx;
	int g_DemodClock;
	
	// Sample buffer, to be supplied in GetFromBigBuf and then written in by the device.
	uint8_t* sample_buf;
	
	// Graphing related globals.
	// FIXME: These should eventually make their way into particular UI implementations instead.
	int GraphBuffer[MAX_GRAPH_TRACE_LEN];
	int GraphTraceLen;

	int s_Buff[MAX_GRAPH_TRACE_LEN];
	double CursorScaleFactor;
	int PlotGridX;
	int PlotGridY;
	int PlotGridXdefault;
	int PlotGridYdefault;
	int CursorCPos;
	int CursorDPos;
	int flushAfterWrite;  //buzzy
	int GridOffset;
	bool GridLocked;
	bool showDemod;
	
} pm3_connection;

void SendCommand(pm3_connection *conn, UsbCommand *c);
bool ReceiveCommand(pm3_connection* conn, UsbCommand* command); // only for flasher
void *uart_receiver(void *targ);
void UsbCommandReceived(pm3_connection *conn, UsbCommand *UC);
void clearCommandBuffer(pm3_connection *conn);
bool WaitForResponseTimeoutW(pm3_connection* conn, uint64_t cmd, UsbCommand* response, size_t ms_timeout, bool show_warning);
bool WaitForResponseTimeout(pm3_connection* conn, uint64_t cmd, UsbCommand* response, size_t ms_timeout);
bool WaitForResponse(pm3_connection* conn, uint64_t cmd, UsbCommand* response);


#endif // COMMS_H_
