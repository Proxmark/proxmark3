// ISO15693 commons
// Adrian Dabrowski 2010 and others, GPLv2

#ifndef ISO15693TOOLS_H__
#define ISO15693TOOLS_H__

#include <stdint.h>

// ISO15693 CRC
#define ISO15693_CRC_CHECK   ((uint16_t)(~0xF0B8 & 0xFFFF))  // use this for checking of a correct crc
uint16_t Iso15693Crc(uint8_t *v, int n);
int Iso15693AddCrc(uint8_t *req, int n);
char* Iso15693sprintUID(char *target, uint8_t *uid);
unsigned short iclass_crc16(uint8_t *data_p, unsigned short length);

#endif
