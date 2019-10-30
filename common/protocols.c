#include "protocols.h"
#include <stdint.h>

// ATA55xx shared presets & routines
uint32_t GetT55xxClockBit(uint32_t clock) {
	switch (clock) {
		case 128:
			return T55x7_BITRATE_RF_128;
		case 100:
			return T55x7_BITRATE_RF_100;
		case 64:
			return T55x7_BITRATE_RF_64;
		case 50:
			return T55x7_BITRATE_RF_50;
		case 40:
			return T55x7_BITRATE_RF_40;
		case 32:
			return T55x7_BITRATE_RF_32;
		case 16:
			return T55x7_BITRATE_RF_16;
		case 8:
			return T55x7_BITRATE_RF_8;
		default:
			return 0;
	}
}
