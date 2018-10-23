// ISO15693 commons
// Adrian Dabrowski 2010 and others, GPLv2

#ifndef ISO15693_H__
#define ISO15693_H__

// ISO15693 CRC
#define ISO15693_CRC_PRESET  (uint16_t)0xFFFF
#define ISO15693_CRC_POLY    (uint16_t)0x8408
#define ISO15693_CRC_CHECK   ((uint16_t)(~0xF0B8 & 0xFFFF))  // use this for checking of a correct crc

uint16_t Iso15693Crc(uint8_t *v, int n);
int Iso15693AddCrc(uint8_t *req, int n);
char* Iso15693sprintUID(char *target,uint8_t *uid);
unsigned short iclass_crc16(char *data_p, unsigned short length);

//-----------------------------------------------------------------------------
// Map a sequence of octets (~layer 2 command) into the set of bits to feed
// to the FPGA, to transmit that command to the tag.
// Mode: highspeed && one subcarrier (ASK)
//-----------------------------------------------------------------------------

	// The sampling rate is 106.353 ksps/s, for T = 18.8 us

	// SOF defined as
	// 1) Unmodulated time of 56.64us
	// 2) 24 pulses of 423.75khz
	// 3) logic '1' (unmodulated for 18.88us followed by 8 pulses of 423.75khz)

	static const int Iso15693FrameSOF[] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		 1,  1,  1,  1,
		 1,  1,  1,  1
	};
	static const int Iso15693Logic0[] = {
		 1,  1,  1,  1,
		 1,  1,  1,  1,
		-1, -1, -1, -1,
		-1, -1, -1, -1
	};
	static const int Iso15693Logic1[] = {
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		 1,  1,  1,  1,
		 1,  1,  1,  1
	};

	// EOF defined as
	// 1) logic '0' (8 pulses of 423.75khz followed by unmodulated for 18.88us)
	// 2) 24 pulses of 423.75khz
	// 3) Unmodulated time of 56.64us

	static const int Iso15693FrameEOF[] = {
		 1,  1,  1,  1,
		 1,  1,  1,  1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};


#endif
