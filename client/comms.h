#ifndef COMMS_H_
#define COMMS_H_

#include <stdbool.h>
#include <pthread.h>

#include "usb_cmd.h"
#include "uart.h"

#ifndef CMD_BUFFER_SIZE
#define CMD_BUFFER_SIZE 50
#endif

#ifndef MAX_DEMOD_BUF_LEN
#define MAX_DEMOD_BUF_LEN (1024*128)
#endif

#ifndef BIGBUF_SIZE
#define BIGBUF_SIZE 40000
#endif

typedef struct {
	// If TRUE, continue running the uart_receiver thread.
	bool run;

	// Lock around serial port receives
	pthread_mutex_t recv_lock;
} receiver_arg;


// Wrappers required as static variables can only be used in one file.
void SetSerialPort(serial_port* new_port);
serial_port* GetSerialPort();
void SetOffline(bool new_offline);
bool IsOffline();

void SendCommand(UsbCommand *c);
void *uart_receiver(void *targ);
void UsbCommandReceived(UsbCommand *UC);
void clearCommandBuffer();
bool WaitForResponseTimeoutW(uint64_t cmd, UsbCommand* response, size_t ms_timeout, bool show_warning);
bool WaitForResponseTimeout(uint64_t cmd, UsbCommand* response, size_t ms_timeout);
bool WaitForResponse(uint64_t cmd, UsbCommand* response);

#endif // COMMS_H_
