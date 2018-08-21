//-----------------------------------------------------------------------------
// (c) 2018 Iceman, adapted by Marshmellow 
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// smart card type prototyping
//-----------------------------------------------------------------------------
#ifndef __SMARTCARD_H
#define __SMARTCARD_H

//-----------------------------------------------------------------------------
// ISO 7618  Smart Card 
//-----------------------------------------------------------------------------
typedef struct {
	uint8_t atr_len;
	uint8_t atr[30];
} __attribute__((__packed__)) smart_card_atr_t;

typedef enum SMARTCARD_COMMAND {
	SC_CONNECT =       (1 << 0),
	SC_NO_DISCONNECT = (1 << 1),
	SC_RAW =           (1 << 2),
	SC_NO_SELECT =     (1 << 3)
} smartcard_command_t;


#endif
