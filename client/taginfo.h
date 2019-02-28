//-----------------------------------------------------------------------------
// ISO/IEC 7816-6 manufacturer byte decoding
//-----------------------------------------------------------------------------

#ifndef MANUFACTURERS_H__
#define MANUFACTURERS_H__

#include <stdint.h>

extern char *getManufacturerName(uint8_t vendorID);
extern char *getChipInfo(uint8_t vendorID, uint8_t chipID);

#endif
