//-----------------------------------------------------------------------------
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// Generic CRC calculation code.
//-----------------------------------------------------------------------------
#include "crc.h"
#include <stdint.h>
#include <stddef.h>

#define BITMASK(X) (1 << (X))

uint32_t reflect(uint32_t v, int b) {
    uint32_t t = v;
    for (int i = 0; i < b; ++i) {
        if (t & 1)
            v |=  BITMASK((b - 1) - i);
        else
            v &= ~BITMASK((b - 1) - i);
        t >>= 1;
    }
    return v;
}

void crc_init(crc_t *crc, int order, uint32_t polynom, uint32_t initial_value, uint32_t final_xor)
{
	crc->order = order;
	crc->polynom = polynom;
	crc->initial_value = initial_value;
	crc->final_xor = final_xor;
	crc->mask = (1L<<order)-1;
	crc_clear(crc);
}

void crc_update(crc_t *crc, uint32_t data, int data_width)
{
	int i;
	for(i=0; i<data_width; i++) {
		int oldstate = crc->state;
		crc->state = crc->state >> 1;
		if( (oldstate^data) & 1 ) {
			crc->state ^= crc->polynom;
		}
		data >>= 1;
	}
}

void crc_clear(crc_t *crc)
{
	crc->state = crc->initial_value & crc->mask;
}

uint32_t crc_finish(crc_t *crc)
{
	return ( crc->state ^ crc->final_xor ) & crc->mask;
}

//credits to iceman
uint32_t CRC8Maxim(uint8_t *buff, size_t size) 
{
	crc_t crc;
	crc_init(&crc, 9, 0x8c, 0x00, 0x00);
	crc_clear(&crc);

	for (size_t i=0; i < size; ++i){
		crc_update(&crc, buff[i], 8);
	}
	return crc_finish(&crc);
}

// width=8 poly=0x1d, init=0xc7 (0xe3 - WRONG! but it mentioned in MAD datasheet) refin=false  refout=false  xorout=0x00 name="CRC-8/MIFARE-MAD"
uint32_t CRC8Mad(uint8_t *buff, size_t size) {
    crc_t crc;
    crc_init(&crc, 8, reflect(0x1d, 8), reflect(0xc7, 8), 0);
    for (int i = 0; i < size; ++i)
        crc_update(&crc, reflect(buff[i], 8), 8);

    return reflect(crc_finish(&crc), 8);
}
