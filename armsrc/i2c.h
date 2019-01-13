//-----------------------------------------------------------------------------
// Willok, June 2018
// Edits by Iceman, July 2018
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// The main i2c code, for communications with smart card module
//-----------------------------------------------------------------------------
#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>
#include <stdbool.h>

#define I2C_DEVICE_ADDRESS_BOOT       0xB0
#define I2C_DEVICE_ADDRESS_MAIN       0xC0

#define I2C_DEVICE_CMD_GENERATE_ATR   0x01
#define I2C_DEVICE_CMD_SEND           0x02
#define I2C_DEVICE_CMD_READ           0x03
#define I2C_DEVICE_CMD_SETBAUD        0x04
#define I2C_DEVICE_CMD_SIM_CLC        0x05
#define I2C_DEVICE_CMD_GETVERSION     0x06
#define I2C_DEVICE_CMD_SEND_T0        0x07


bool I2C_is_available(void);

#ifdef WITH_SMARTCARD
void SmartCardAtr(void);
void SmartCardRaw(uint64_t arg0, uint64_t arg1, uint8_t *data);
void SmartCardUpgrade(uint64_t arg0);
void SmartCardSetClock(uint64_t arg0);
void I2C_print_status(void);
#endif

#endif // __I2C_H
