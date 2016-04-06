#include "proxmark3.h"
#include "apps.h"
#include "util.h"
#include "string.h"

#define EM4x50_PASSWORD					0
#define EM4x50_PROTECTION				1
#define EM4x50_CONTROL					2
#define EM4x50_SERIAL					32
#define EM4x50_DEVICE_ID				33

#define EM4x50_FIRST_WORD_READ_BYTE					0
#define EM4x50_LAST_WORD_READ_BYTE					1
#define EM4x50_FIRST_WORD_READ_PROTECTED_BYTE		0
#define EM4x50_LAST_WORD_READ_PROTECTED_BYTE		1
#define EM4x50_FIRST_WORD_WRITE_INHIBITED_BYTE		2
#define EM4x50_LAST_WORD_WRITE_INHIBITED_BYTE		3
#define EM4X50_PASSWORD_CHECK_BIT					16
#define EM4X50_READ_AFTER_WRITE						17

// Sam7s has several timers, we will use the source TIMER_CLOCK1 (aka AT91C_TC_CLKS_TIMER_DIV1_CLOCK)
// TIMER_CLOCK1 = MCK/2, MCK is running at 48 MHz, Timer is running at 48/2 = 24 MHz
// Hitag units (T0) have duration of 8 microseconds (us), which is 1/125000 per second (carrier)
// T0 = TIMER_CLOCK1 / 125000 = 192
#define T0 192

#define EM4x50_T_HALF_PERIOD				32
#define EM4x50_T_FULL_PERIOD				64

#define EM4x50_COMMAND_NONE					0xFF
#define EM4x50_COMMAND_BREAK				0x00
#define EM4x50_COMMAND_LOGIN				0x01
#define EM4x50_COMMAND_WRITE_PASSWORD		0x11
#define EM4x50_COMMAND_WRITE				0x12
#define EM4x50_COMMAND_SELECTIVE_READ		0x0A
#define EM4x50_COMMAND_RESET				0x80

#define EM4X50_T_PROCESSING_PAUSE	64
#define EM4X50_T_INIT				2112
#define EM4X50_T_WRITE_ACCESS		64
#define EM4X50_T_EEPROM_WRITE		3200

#define EM4X50_WAITING_FOR_DATA				0
#define EM4X50_WAITING_FOR_BYTE_PARITY		1
#define EM4X50_WAITING_FOR_CULOMN_PARITY	2
#define EM4X50_WAITING_FOR_0				3

#define EM4X50_STATE_READ				0
#define EM4X50_STATE_SELECTIVE_READ		1

// 1. Login command arguments are
//		a. current password (32 bit password + 4 bit byte parity + 8 bit culommn parity + 0)
// 2. Write password arguments are
//		a. new password (32 bit password + 4 bit byte parity + 8 bit culommn parity + 0)
//		b. current password (32 bit password + 4 bit byte parity + 8 bit culommn parity + 0)
// 3. Write arguments are
//		a. address (2 0s + 6 bit address + 1 bit parity)
//		b. data (32 bit data + 4 bit byte parity + 8 bit culommn parity + 0)
// 4. Selective read arguments are
//		a. First address (2 0s + 6 bit address + 1 bit parity)
//		b. Last address (2 0s + 6 bit address + 1 bit parity)
// 5. Reset command has not arguments

struct em4x50_tag
{
	int state;
	int first_word;
	int last_word;
	byte_t data[34][4];
};

static struct em4x50_tag tag = {
	.state = EM4X50_STATE_READ,
	.first_word = 0x20,
	.last_word = 0x21,
	.data = {
		[0] = { 0x00, 0x00, 0x00, 0x00 }, // PASSWORD
		[1] = { 0x00, 0x1F, 0x00, 0x00 }, // PROTECTION WORD
		[2] = { 0x20, 0x20, 0x00, 0x00 }, // CONTROL WORD
		[3] = { 0x01, 0x02, 0x03, 0x04 }, // User
		[4] = { 0x05, 0x06, 0x07, 0x08 }, // User
		[5] = { 0x09, 0x0A, 0x0B, 0x0C }, // User
		[6] = { 0x0D, 0x0E, 0x0F, 0x00 }, // User
		[7] = { 0x11, 0x12, 0x13, 0x14 }, // User
		[8] = { 0x15, 0x16, 0x17, 0x18 }, // User
		[9] = { 0x19, 0x1A, 0x1B, 0x1C }, // User
		[10] = { 0x1D, 0x1E, 0x1F, 0x10 }, // User
		[11] = { 0x21, 0x22, 0x23, 0x24 }, // User
		[12] = { 0x25, 0x26, 0x27, 0x28 }, // User
		[13] = { 0x29, 0x2A, 0x2B, 0x2C }, // User
		[14] = { 0x2D, 0x2E, 0x2F, 0x20 }, // User
		[15] = { 0x31, 0x32, 0x33, 0x34 }, // User
		[16] = { 0x35, 0x36, 0x37, 0x38 }, // User
		[17] = { 0x39, 0x3A, 0x3B, 0x3C }, // User
		[18] = { 0x3D, 0x3E, 0x3F, 0x30 }, // User
		[19] = { 0x41, 0x42, 0x43, 0x44 }, // User
		[20] = { 0x45, 0x46, 0x47, 0x48 }, // User
		[21] = { 0x49, 0x4A, 0x4B, 0x4C }, // User
		[22] = { 0x4D, 0x4E, 0x4F, 0x40 }, // User
		[23] = { 0x51, 0x52, 0x53, 0x54 }, // User
		[24] = { 0x55, 0x56, 0x57, 0x58 }, // User
		[25] = { 0x59, 0x5A, 0x5B, 0x5C }, // User
		[26] = { 0x5D, 0x5E, 0x5F, 0x50 }, // User
		[27] = { 0x61, 0x62, 0x63, 0x64 }, // User
		[28] = { 0x65, 0x66, 0x67, 0x68 }, // User
		[29] = { 0x69, 0x6A, 0x6B, 0x6C }, // User
		[30] = { 0x6D, 0x6E, 0x6F, 0x60 }, // User
		[31] = { 0x71, 0x72, 0x73, 0x74 }, // User
		[32] = { 0x11, 0x22, 0x33, 0x44 }, // Serial number
		[33] = { 0x55, 0x66, 0x77, 0x88 } // Device id
	}
};

static void em4x50_init()
{
	tag.state = EM4X50_STATE_READ;
	tag.first_word = tag.data[EM4x50_CONTROL][EM4x50_FIRST_WORD_READ_BYTE];
	tag.last_word = tag.data[EM4x50_CONTROL][EM4x50_LAST_WORD_READ_BYTE];
}

static int em4x50_receive_bit()
{
	return 1;
}

static void em4x50_send_bit(int bit)
{
	// Reset clock for the next bit 
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;

	// Fixed modulation
	if (bit == 0)
	{
		// Manchester: Unloaded, then loaded |__--|
		LOW(GPIO_SSC_DOUT);
		while (AT91C_BASE_TC0->TC_CV < T0*EM4x50_T_HALF_PERIOD);
		HIGH(GPIO_SSC_DOUT);
		while (AT91C_BASE_TC0->TC_CV < T0*EM4x50_T_FULL_PERIOD);
	}
	else
	{
		// Manchester: Loaded, then unloaded |--__|
		HIGH(GPIO_SSC_DOUT);
		while (AT91C_BASE_TC0->TC_CV < T0*EM4x50_T_HALF_PERIOD);
		LOW(GPIO_SSC_DOUT);
		while (AT91C_BASE_TC0->TC_CV < T0*EM4x50_T_FULL_PERIOD);
	}
}

static void em4x50_send_byte(byte_t byte)
{
	int mask = 0x80;

	for (int i = 0; i < 8; i++)
	{
		if ((byte & mask) == 0)
		{
			em4x50_send_bit(0);
		}
		else
		{
			em4x50_send_bit(1);
		}

		mask >>= 1;
	}
}

static void em4x50_send_byte_with_parity(byte_t byte)
{
	int parity = 0;
	int mask = 0x80;

	for (int i = 0; i < 8; i++)
	{
		if ((byte & mask) == 0)
		{
			em4x50_send_bit(0);
			parity ^= 0;
		}
		else
		{
			em4x50_send_bit(1);
			parity ^= 1;
		}

		mask >>= 1;
	}

	em4x50_send_bit(parity);
}

static void em4x50_send_word(int index)
{
	int parity = 0;

	for (int i = 0; i < 4; i++)
	{
		em4x50_send_byte_with_parity(tag.data[index][i]);

		parity ^= tag.data[index][i];
	}

	em4x50_send_byte(parity);
	em4x50_send_bit(0);
}

static int em4x50_listen_one_bit(int *time, int *timestamp)
{
	int bit = 1;

	// 32 RF periods - high
	*time += T0*EM4x50_T_HALF_PERIOD;
	HIGH(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < *time);

	// 32 RF periods - low
	*time += T0*EM4x50_T_HALF_PERIOD;
	LOW(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < *time);

	return bit;
}

static int em4x50_listen()
{
	int period = 0;
	int first_bit = 1;
	int second_bit = 1;
	int data_bit;
	int parity_bit;
	int command = EM4x50_COMMAND_BREAK;
	bool command_is_valid = false;
	int parity = 0;
	int zero_falling_edge_time[2] = { -1, -1 };
	int data_falling_edge_time[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };

	// Reset clocks
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;

	// The listen window is composed of 
	// 32 RF periods - high
	// 32 RF periods - low
	// 128 RF periods - high
	// 64 RF periods - low
	// 64 RF periods - high

	first_bit = em4x50_listen_one_bit(&period, &zero_falling_edge_time[0]);
	if (first_bit == 0)
	{
		second_bit = em4x50_listen_one_bit(&period, &zero_falling_edge_time[1]);
		if (second_bit == 0)
		{
			for (int i = 0; i < 8; i++)
			{
				data_bit = em4x50_listen_one_bit(&period, &data_falling_edge_time[i]);
				command |= (data_bit << (7 - i));
				parity ^= data_bit;
			}

			parity_bit = em4x50_listen_one_bit(&period, &data_falling_edge_time[8]);
			if (parity_bit == parity)
			{
				command_is_valid = true;
			}
		}
	}
	else
	{
		// 128 RF periods - high
		period += T0 * 2 * EM4x50_T_FULL_PERIOD;
		HIGH(GPIO_SSC_DOUT);
		while (AT91C_BASE_TC0->TC_CV < period);

		// 64 RF periods - low
		period += T0 * EM4x50_T_FULL_PERIOD;
		LOW(GPIO_SSC_DOUT);
		while (AT91C_BASE_TC0->TC_CV < period);

		// 64 RF periods - high
		period += T0 * EM4x50_T_FULL_PERIOD;
		HIGH(GPIO_SSC_DOUT);
		while (AT91C_BASE_TC0->TC_CV < period);

		command = EM4x50_COMMAND_NONE;
	}

	if (second_bit == 0)
	{
		if ((command_is_valid == false) || (command == EM4x50_COMMAND_NONE))
		{
			command = EM4x50_COMMAND_BREAK;
		}
	}

	return command;
}

static void em4x50_send_ack()
{
	int period;

	// Reset clock
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;

	period = T0*EM4x50_T_HALF_PERIOD;
	HIGH(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);
	
	period += T0*EM4x50_T_HALF_PERIOD;
	LOW(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0 * 3 * EM4x50_T_HALF_PERIOD;
	HIGH(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0 * EM4x50_T_HALF_PERIOD;
	LOW(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0 * 3 * EM4x50_T_HALF_PERIOD;
	HIGH(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0 * EM4x50_T_HALF_PERIOD;
	LOW(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);
}

static void em4x50_send_nack()
{
	int period;

	// Reset clock
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;

	period = T0*EM4x50_T_HALF_PERIOD;
	HIGH(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0*EM4x50_T_HALF_PERIOD;
	LOW(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0 * 3 * EM4x50_T_HALF_PERIOD;
	HIGH(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0 * EM4x50_T_HALF_PERIOD;
	LOW(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0 * 3 * EM4x50_T_HALF_PERIOD;
	HIGH(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);

	period += T0 * EM4x50_T_HALF_PERIOD;
	LOW(GPIO_SSC_DOUT);
	while (AT91C_BASE_TC0->TC_CV < period);
}

static void em4x50_handle_login_command()
{
	bool done = false;
	bool success = true;
	int state = EM4X50_WAITING_FOR_DATA;
	int in_state_bits = 0;
	int data_bits = 0;
	int password = 0;
	int byte_parity = 0;
	byte_t password_culomn_parity = 0;
	byte_t culomn_parity = 0;

	while ((success == true) && (done == false))
	{
		int bit = em4x50_receive_bit();

		switch (state)
		{
			case EM4X50_WAITING_FOR_DATA:
			{
				password <<= 1;
				password |= bit;
				byte_parity ^= bit;
				in_state_bits++;
				if (in_state_bits == 8)
				{
					password_culomn_parity ^= (byte_t)password;

					data_bits += in_state_bits;
					in_state_bits = 0;

					state = EM4X50_WAITING_FOR_BYTE_PARITY;
				}
			}
			break;
			case EM4X50_WAITING_FOR_BYTE_PARITY:
			{
				if (byte_parity == bit)
				{
					byte_parity = 0;

					if (data_bits == 32)
					{
						state = EM4X50_WAITING_FOR_CULOMN_PARITY;
					}
					else
					{
						state = EM4X50_WAITING_FOR_DATA;
					}
				}
				else
				{
					success = false;
				}
			}
			break;
			case EM4X50_WAITING_FOR_CULOMN_PARITY:
			{
				culomn_parity <<= 1;
				culomn_parity |= bit;
				in_state_bits++;
				if (in_state_bits == 8)
				{
					if (culomn_parity == password_culomn_parity)
					{
						state = EM4X50_WAITING_FOR_0;
					}
					else
					{
						success = false;
					}
				}
			}
			break;
			case EM4X50_WAITING_FOR_0:
			{
				if (bit == 0)
				{
					done = true;
				}
				{
					success = false;
				}
			}
			break;
		};
	}

	if (success == false)
	{
		em4x50_send_nack();
	}
	else
	{
		em4x50_send_ack();
	}

	// Wait for processing pause time
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;
	while (AT91C_BASE_TC0->TC_CV < EM4X50_T_PROCESSING_PAUSE);
}

static void em4x50_handle_write_password_command()
{
}

static void em4x50_handle_write_command()
{
}

static void em4x50_handle_selective_read_command()
{
	bool done = false;
	bool success = true;
	int state = EM4X50_WAITING_FOR_DATA;
	int in_state_bits = 0;
	byte_t first_address = 0;
	byte_t last_address = 0;
	byte_t *p_address = &first_address;
	int byte_parity = 0;

	while ((success == true) && (done == false))
	{
		int bit = em4x50_receive_bit();

		switch (state)
		{
			case EM4X50_WAITING_FOR_DATA:
			{
				(*p_address) <<= 1;
				(*p_address) |= bit;
				byte_parity ^= bit;
				in_state_bits++;
				if (in_state_bits == 8)
				{
					in_state_bits = 0;

					state = EM4X50_WAITING_FOR_BYTE_PARITY;
				}
			}
			break;
			case EM4X50_WAITING_FOR_BYTE_PARITY:
			{
				if (byte_parity == bit)
				{
					if (p_address == &first_address)
					{
						p_address = &last_address;
						byte_parity = 0;

						state = EM4X50_WAITING_FOR_DATA;
					}
					else
					{
						done = true;
					}
				}
				else
				{
					success = false;
				}
			}
			break;
		};
	}

	if (success == false)
	{
		em4x50_send_nack();
	}
	else
	{
		em4x50_send_ack();
	}

	// Wait for processing pause time
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;
	while (AT91C_BASE_TC0->TC_CV < EM4X50_T_PROCESSING_PAUSE);
}

static void em4x50_handle_reset_command()
{
	// Wait for processing pause time
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;
	while (AT91C_BASE_TC0->TC_CV < EM4X50_T_PROCESSING_PAUSE);
	
	em4x50_init();

	em4x50_send_ack();

	// Wait for init time
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;
	while (AT91C_BASE_TC0->TC_CV < EM4X50_T_INIT);
}

void SimulateEM4x50Tag(bool tag_mem_supplied, byte_t* data)
{
	int command;

	FpgaDownloadAndGo(FPGA_BITSTREAM_LF);

	// Clean up trace and prepare it for storing frames
	set_tracing(TRUE);
	clear_trace();

	DbpString("Starting EM4x50 simulation");
	LED_D_ON();

	if (tag_mem_supplied)
	{
		DbpString("Loading EM4x50 memory...");
		memcpy((byte_t*)tag.data, data, sizeof(tag.data));
		em4x50_init();
	}

	Dbprintf("Read mode %d -> %d", tag.first_word, tag.last_word);

	// Set up simulator mode, frequency divisor which will drive the FPGA
	// and analog mux selection.
	FpgaWriteConfWord(FPGA_MAJOR_MODE_LF_EDGE_DETECT | FPGA_LF_EDGE_DETECT_READER_FIELD);
	FpgaSendCommand(FPGA_CMD_SET_DIVISOR, 95); //125Khz
	SetAdcMuxFor(GPIO_MUXSEL_LOPKD);
	RELAY_OFF();

	// Configure output pin that is connected to the FPGA (for modulating)
	AT91C_BASE_PIOA->PIO_OER = GPIO_SSC_DOUT;
	AT91C_BASE_PIOA->PIO_PER = GPIO_SSC_DOUT;

	// Disable modulation at default, which means release resistance
	LOW(GPIO_SSC_DOUT);

	// Enable Peripheral Clock for TIMER_CLOCK0, used to measure exact timing before answering
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);

	// Enable Peripheral Clock for TIMER_CLOCK1, used to capture edges of the reader frames
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC1);
	AT91C_BASE_PIOA->PIO_BSR = GPIO_SSC_FRAME;

	// Enable timer 
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;

	// Disable timer during configuration	
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
	// Capture mode, default timer source = MCK/2 (TIMER_CLOCK1), TIOA is external trigger,
	// external trigger falling edge, load RA on falling edge of TIOA.
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK | AT91C_TC_ETRGEDG_FALLING | AT91C_TC_ABETRG | AT91C_TC_LDRA_FALLING;

	while (!BUTTON_PRESS())
	{
		// Watchdog hit
		WDT_HIT();

		// The structure for each frame is
		// LISTEN_WINDOW LISTEN_WINDOW FIRST_WORD LISTEN_WINDOW SECOND_WORD ...... LISTEN_WINDOW LAST_WORD
		// i.e. it starts with a listen windows and then pairs of (listen window, data word).
		// In read mode the range of transmitteed words is defined by the control register.
		// During the listen window the transceiver can send commands to the em4x50 . The em4x50 will stop the frame transmission
		// and will perform the command.

		// Start a frame with a listen window.
		command = em4x50_listen();

		// If no command was received during the listen window, continue with the frame transmission.
		if (command == EM4x50_COMMAND_NONE)
		{
			// Loop thought the [first_word..last_word] and transmit pairs of (listen window, data word).
			for (int i = tag.first_word; i <= tag.last_word; i++)
			{
				// // Start a pair with a listen window.
				command = em4x50_listen();

				// If no command was received during the listen window, continue with the data word transmission.
				if (command == EM4x50_COMMAND_NONE)
				{
					em4x50_send_word(i);
				}
				// If a command was received during the listen window, break the frame transmission.
				else
				{
					break;
				}
			}
		}

		// The transceiver can send a selective mode command and provide the first and last word to be transmitted. The em4x50 will send
		// those words in the follwoing frame once and will return to read mode.
		// If the current frame was transmitted in selective read state then the em4x50 should return to read mode.
		if (tag.state == EM4X50_STATE_SELECTIVE_READ)
		{
			// Set the state to read mode.
			tag.state = EM4X50_STATE_READ;

			// Set the transmitted words range according to the control register.
			tag.first_word = tag.data[EM4x50_CONTROL][EM4x50_FIRST_WORD_READ_BYTE];
			tag.last_word = tag.data[EM4x50_CONTROL][EM4x50_LAST_WORD_READ_BYTE];
		}

		// If a command was received during the frame 
		if (command != EM4x50_COMMAND_NONE)
		{
			switch (command)
			{
				case EM4x50_COMMAND_BREAK:
				{
				}
				break;
				case EM4x50_COMMAND_LOGIN:
				{
					// Handle login command.
					em4x50_handle_login_command();
				}
				break;
				case EM4x50_COMMAND_WRITE_PASSWORD:
				{
					// Handle write password command.
					em4x50_handle_write_password_command();
				}
				break;
				case EM4x50_COMMAND_WRITE:
				{
					// Handle write command.
					em4x50_handle_write_command();
				}
				break;
				case EM4x50_COMMAND_SELECTIVE_READ:
				{
					// Handle selective command.
					em4x50_handle_selective_read_command();
				}
				break;
				case EM4x50_COMMAND_RESET:
				{
					// Handle reset command.
					em4x50_handle_reset_command();
				}
				break;
			};
		}
	}

	LED_D_OFF();
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);

	DbpString("Sim Stopped");
}