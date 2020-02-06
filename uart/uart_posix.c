/*
 * Generic uart / rs232/ serial port library
 *
 * Copyright (c) 2013, Roel Verdult
 * Copyright (c) 2018 Google
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @file uart_posix.c
 *
 * This version of the library has functionality removed which was not used by
 * proxmark3 project.
 */

// Test if we are dealing with posix operating systems
#ifndef _WIN32
#define _DEFAULT_SOURCE

#include "uart.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>

// Fix missing definition on OS X.
// Taken from https://github.com/unbit/uwsgi/commit/b608eb1772641d525bfde268fe9d6d8d0d5efde7
#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif

typedef struct termios term_info;
typedef struct {
	int fd;           // Serial port file descriptor
	term_info tiOld;  // Terminal info before using the port
	term_info tiNew;  // Terminal info during the transaction
} serial_port_unix;

// Set time-out on 30 miliseconds
static struct timeval timeout = {
	.tv_sec  =     0, // 0 second
	.tv_usec = 30000  // 30000 micro seconds
};


void uart_close(const serial_port sp) {
	serial_port_unix* spu = (serial_port_unix*)sp;
	tcflush(spu->fd, TCIOFLUSH);
	tcsetattr(spu->fd, TCSANOW, &(spu->tiOld));
	struct flock fl;
	fl.l_type   = F_UNLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start  = 0;
	fl.l_len    = 0;
	fl.l_pid    = getpid();
	fcntl(spu->fd, F_SETLK, &fl);
	close(spu->fd);
	free(sp);
}


serial_port uart_open(const char* pcPortName) {

	serial_port_unix* sp = malloc(sizeof(serial_port_unix));
	if (sp == 0) return INVALID_SERIAL_PORT;

	if (memcmp(pcPortName, "tcp:", 4) == 0) {
		struct addrinfo *addr = NULL, *rp;
		char *addrstr = strdup(pcPortName + 4);
		if (addrstr == NULL) {
			printf("Error: strdup\n");
			return INVALID_SERIAL_PORT;
		}
		char *colon = strrchr(addrstr, ':');
		char *portstr;

		// Set time-out to 300 milliseconds only for TCP port
		timeout.tv_usec = 300000;

		if (colon) {
			portstr = colon + 1;
			*colon = '\0';
		} else {
			portstr = "7901";
		}

		struct addrinfo info;

		memset(&info, 0, sizeof(info));

		info.ai_socktype = SOCK_STREAM;

		int s = getaddrinfo(addrstr, portstr, &info, &addr);
		if (s != 0) {
			printf("Error: getaddrinfo: %s\n", gai_strerror(s));
			return INVALID_SERIAL_PORT;
		}

		int sfd;
		for (rp = addr; rp != NULL; rp = rp->ai_next) {
			sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (sfd == -1)
				continue;
			if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
				break;
			close(sfd);
		}

		if (rp == NULL) {               /* No address succeeded */
			printf("Error: Could not connect\n");
			return INVALID_SERIAL_PORT;
		}

		freeaddrinfo(addr);
		free(addrstr);

		sp->fd = sfd;

		int one = 1;
		setsockopt(sp->fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));
		return sp;
	}

	sp->fd = open(pcPortName, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	if (sp->fd == -1) {
		uart_close(sp);
		return INVALID_SERIAL_PORT;
	}

	// Finally figured out a way to claim a serial port interface under unix
	// We just try to set a (advisory) lock on the file descriptor
	struct flock fl;
	fl.l_type   = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start  = 0;
	fl.l_len    = 0;
	fl.l_pid    = getpid();

	// Does the system allows us to place a lock on this file descriptor
	if (fcntl(sp->fd, F_SETLK, &fl) == -1) {
		// A conflicting lock is held by another process
		free(sp);
		return CLAIMED_SERIAL_PORT;
	}

	// Try to retrieve the old (current) terminal info struct
	if (tcgetattr(sp->fd,&sp->tiOld) == -1) {
		uart_close(sp);
		return INVALID_SERIAL_PORT;
	}

	// Duplicate the (old) terminal info struct
	sp->tiNew = sp->tiOld;

	// Configure the serial port
	sp->tiNew.c_cflag &= ~(CSIZE | PARENB);
	sp->tiNew.c_cflag |= (CS8 | CLOCAL | CREAD);
	sp->tiNew.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	sp->tiNew.c_iflag |= IGNPAR;
	sp->tiNew.c_oflag &= ~OPOST;
	sp->tiNew.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);


	// Try to set the new terminal info struct
	if (tcsetattr(sp->fd, TCSANOW, &sp->tiNew) == -1) {
		uart_close(sp);
		return INVALID_SERIAL_PORT;
	}

	// Flush all lingering data that may exist
	tcflush(sp->fd, TCIOFLUSH);

	return sp;
}


bool uart_receive(const serial_port sp, uint8_t* pbtRx, size_t szMaxRxLen, size_t* pszRxLen) {

	*pszRxLen = 0;

	if (szMaxRxLen == 0) return true;

	struct timeval t_current;
	gettimeofday(&t_current, NULL);
	struct timeval t_end;
	timeradd(&t_current, &timeout, &t_end);

	while (true) {
		int res = read(((serial_port_unix*)sp)->fd, pbtRx, szMaxRxLen - *pszRxLen);
		if (res < 0 && errno != EAGAIN && errno != EWOULDBLOCK) return false;
		if (res > 0) {
			*pszRxLen += res;
			pbtRx += res;
		}
		if (*pszRxLen == szMaxRxLen) return true; // we could read all requested bytes in time
		gettimeofday(&t_current, NULL);
		if (timercmp(&t_current, &t_end, >)) return true; // timeout
		// set next select timeout
		struct timeval t_remains;
		timersub(&t_end, &t_current, &t_remains);
		// Set the file descriptor set
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(((serial_port_unix*)sp)->fd, &rfds);
		// wait for more bytes available
		res = select(((serial_port_unix*)sp)->fd+1, &rfds, NULL, NULL, &t_remains);
		if (res < 0) return false;
		if (res == 0) return true; // timeout
	}
	return true; // should never come here
}


bool uart_send(const serial_port sp, const uint8_t* pbtTx, const size_t szTxLen) {

	if (szTxLen == 0) return true;

	size_t bytes_written = 0;

	struct timeval t_current;
	gettimeofday(&t_current, NULL);
	struct timeval t_end;
	timeradd(&t_current, &timeout, &t_end);

	while (true) {
		int res = write(((serial_port_unix*)sp)->fd, pbtTx, szTxLen - bytes_written);
		if (res < 0 && res != EAGAIN && res != EWOULDBLOCK) return false;
		if (res > 0) {
			pbtTx += res;
			bytes_written += res;
		}
		if (bytes_written == szTxLen) return true; // we could write all bytes
		gettimeofday(&t_current, NULL);
		if (timercmp(&t_current, &t_end, >)) return false; // timeout
		// set next select timeout
		struct timeval t_remains;
		timersub(&t_end, &t_current, &t_remains);
		// Set the file descriptor set
		fd_set wfds;
		FD_ZERO(&wfds);
		FD_SET(((serial_port_unix*)sp)->fd, &wfds);
		// wait until more bytes can be written
		res = select(((serial_port_unix*)sp)->fd+1, NULL, &wfds, NULL, &t_remains);
		if (res < 0) return false;  // error
		if (res == 0) return false; // timeout
	}
	return true;
}


bool uart_set_speed(serial_port sp, const uint32_t uiPortSpeed) {
	const serial_port_unix* spu = (serial_port_unix*)sp;
	speed_t stPortSpeed;
	switch (uiPortSpeed) {
		case 0: stPortSpeed = B0; break;
		case 50: stPortSpeed = B50; break;
		case 75: stPortSpeed = B75; break;
		case 110: stPortSpeed = B110; break;
		case 134: stPortSpeed = B134; break;
		case 150: stPortSpeed = B150; break;
		case 300: stPortSpeed = B300; break;
		case 600: stPortSpeed = B600; break;
		case 1200: stPortSpeed = B1200; break;
		case 1800: stPortSpeed = B1800; break;
		case 2400: stPortSpeed = B2400; break;
		case 4800: stPortSpeed = B4800; break;
		case 9600: stPortSpeed = B9600; break;
		case 19200: stPortSpeed = B19200; break;
		case 38400: stPortSpeed = B38400; break;
#  ifdef B57600
		case 57600: stPortSpeed = B57600; break;
#  endif
#  ifdef B115200
		case 115200: stPortSpeed = B115200; break;
#  endif
#  ifdef B230400
		case 230400: stPortSpeed = B230400; break;
#  endif
#  ifdef B460800
		case 460800: stPortSpeed = B460800; break;
#  endif
#  ifdef B921600
		case 921600: stPortSpeed = B921600; break;
#  endif
		default: return false;
	};
	struct termios ti;
	if (tcgetattr(spu->fd, &ti) == -1) return false;
	// Set port speed (Input and Output)
	cfsetispeed(&ti,stPortSpeed);
	cfsetospeed(&ti,stPortSpeed);
	return (tcsetattr(spu->fd, TCSANOW, &ti) != -1);
}

uint32_t uart_get_speed(const serial_port sp) {
	struct termios ti;
	uint32_t uiPortSpeed;
	const serial_port_unix* spu = (serial_port_unix*)sp;
	if (tcgetattr(spu->fd, &ti) == -1) return 0;
	// Set port speed (Input)
	speed_t stPortSpeed = cfgetispeed(&ti);
	switch (stPortSpeed) {
		case B0: uiPortSpeed = 0; break;
		case B50: uiPortSpeed = 50; break;
		case B75: uiPortSpeed = 75; break;
		case B110: uiPortSpeed = 110; break;
		case B134: uiPortSpeed = 134; break;
		case B150: uiPortSpeed = 150; break;
		case B300: uiPortSpeed = 300; break;
		case B600: uiPortSpeed = 600; break;
		case B1200: uiPortSpeed = 1200; break;
		case B1800: uiPortSpeed = 1800; break;
		case B2400: uiPortSpeed = 2400; break;
		case B4800: uiPortSpeed = 4800; break;
		case B9600: uiPortSpeed = 9600; break;
		case B19200: uiPortSpeed = 19200; break;
		case B38400: uiPortSpeed = 38400; break;
#  ifdef B57600
		case B57600: uiPortSpeed = 57600; break;
#  endif
#  ifdef B115200
		case B115200: uiPortSpeed = 115200; break;
#  endif
#  ifdef B230400
		case B230400: uiPortSpeed = 230400; break;
#  endif
#  ifdef B460800
		case B460800: uiPortSpeed = 460800; break;
#  endif
#  ifdef B921600
		case B921600: uiPortSpeed = 921600; break;
#  endif
		default: return 0;
	};
	return uiPortSpeed;
}

#endif
