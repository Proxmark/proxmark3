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
#include "comms.h"
#include "usb_cmd.h"


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
	fprintf(stderr, "\nExample:\n\n\t %s "SERIAL_PORT_H" armsrc/obj/fullimage.elf\n", argv0);
#ifdef __linux__
	fprintf(stderr, "\nNote (Linux): if the flasher gets stuck at 'Waiting for Proxmark to reappear',\n");
	fprintf(stderr, "       you may need to blacklist proxmark for modem-manager. v1.4.14 and later\n");
	fprintf(stderr, "       include this configuration patch already. The change can be found at:\n");
	fprintf(stderr, "       https://cgit.freedesktop.org/ModemManager/ModemManager/commit/?id=6e7ff47\n\n");
#endif
}

#define MAX_FILES 4

int main(int argc, char **argv)
{
	int can_write_bl = false;
	int num_files = 0;
	int res;
	flash_file_t files[MAX_FILES];

	memset(files, 0, sizeof(files));

	if (argc < 3) {
		usage(argv[0]);
		return -1;
	}

	for (int i = 2; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (!strcmp(argv[i], "-b")) {
				can_write_bl = true;
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

	char* serial_port_name = argv[1];

	if (!OpenProxmark(serial_port_name, true, 120, true)) {   // wait for 2 minutes
		fprintf(stderr, "Could not find Proxmark on %s.\n\n", serial_port_name);
		return -1;
	} else {
		fprintf(stderr," Found.\n");
	}

	res = flash_start_flashing(can_write_bl, serial_port_name);
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
	CloseProxmark();

	fprintf(stderr, "All done.\n\n");
	fprintf(stderr, "Have a nice day!\n");

	return 0;
}
