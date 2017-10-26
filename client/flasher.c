//-----------------------------------------------------------------------------
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Flasher frontend tool
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <pthread.h>

#include "proxmark3.h"
#include "util.h"
#include "util_posix.h"
#include "flash.h"
#include "uart.h"
#include "usb_cmd.h"
#include "comms.h"

#ifdef _WIN32
# define unlink(x)
#else
# include <unistd.h>
#endif

void cmd_debug(UsbCommand* UC) {
  //  Debug
  printf("UsbCommand length[len=%zd]\n",sizeof(UsbCommand));
  printf("  cmd[len=%zd]: %016" PRIx64 "\n",sizeof(UC->cmd),UC->cmd);
  printf(" arg0[len=%zd]: %016" PRIx64 "\n",sizeof(UC->arg[0]),UC->arg[0]);
  printf(" arg1[len=%zd]: %016" PRIx64 "\n",sizeof(UC->arg[1]),UC->arg[1]);
  printf(" arg2[len=%zd]: %016" PRIx64 "\n",sizeof(UC->arg[2]),UC->arg[2]);
  printf(" data[len=%zd]: ",sizeof(UC->d.asBytes));
  for (size_t i=0; i<16; i++) {
    printf("%02x",UC->d.asBytes[i]);
  }
  printf("...\n");
}

static void usage(char *argv0)
{
	fprintf(stderr, "Usage:   %s <port> [-b] image.elf [image.elf...]\n\n", argv0);
	fprintf(stderr, "\t-b\tEnable flashing of bootloader area (DANGEROUS)\n\n");
	//Is the example below really true? /Martin
	fprintf(stderr, "Example:\n\n\t %s path/to/osimage.elf path/to/fpgaimage.elf\n", argv0);
	fprintf(stderr, "\nExample (Linux):\n\n\t %s  /dev/ttyACM0 armsrc/obj/fullimage.elf\n", argv0);
	fprintf(stderr, "\nNote (Linux): if the flasher gets stuck at 'Waiting for Proxmark to reappear',\n");
	fprintf(stderr, "       you may need to blacklist proxmark for modem-manager. v1.4.14 and later\n");
	fprintf(stderr, "       include this configuration patch already. The change can be found at:\n");
	fprintf(stderr, "       https://cgit.freedesktop.org/ModemManager/ModemManager/commit/?id=6e7ff47\n\n");
}

#define MAX_FILES 4

int main(int argc, char **argv)
{
	int can_write_bl = 0;
	int num_files = 0;
	int res;
	flash_file_t files[MAX_FILES];
	receiver_arg conn;
	pthread_t reader_thread;

	memset(&conn, 0, sizeof(receiver_arg));
	memset(files, 0, sizeof(files));

	if (argc < 3) {
		usage(argv[0]);
		return -1;
	}

	for (int i = 2; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (!strcmp(argv[i], "-b")) {
				can_write_bl = 1;
			} else {
				usage(argv[0]);
				return -1;
			}
		} else {
			res = flash_load(&files[num_files], argv[i], can_write_bl);
			if (res < 0) {
				fprintf(stderr, "Error while loading %s\n", argv[i]);
				return -1;
			}
			fprintf(stderr, "\n");
			num_files++;
		}
	}

	pthread_mutex_init(&conn.recv_lock, NULL);

	char* serial_port_name = argv[1];

	fprintf(stderr,"Waiting for Proxmark to appear on %s", serial_port_name);
	do {
		sleep(1);
		fprintf(stderr, ".");
	} while (!OpenProxmark(serial_port_name));
	fprintf(stderr," Found.\n");

	// Lets start up the communications thread
	conn.run = true;
	pthread_create(&reader_thread, NULL, &uart_receiver, &conn);

	res = flash_start_flashing(&conn, can_write_bl, serial_port_name);
	if (res < 0)
		return -1;

	fprintf(stderr, "\nFlashing...\n");

	for (int i = 0; i < num_files; i++) {
		res = flash_write(&files[i]);
		if (res < 0)
			return -1;
		flash_free(&files[i]);
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "Resetting hardware...\n");

	res = flash_stop_flashing();
	if (res < 0)
		return -1;

	// Stop the command thread.
	conn.run = false;
	pthread_join(reader_thread, NULL);
	CloseProxmark(&conn, serial_port_name);
	pthread_mutex_destroy(&conn.recv_lock);

	fprintf(stderr, "All done.\n\n");
	fprintf(stderr, "Have a nice day!\n");

	return 0;
}
