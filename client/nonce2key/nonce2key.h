//-----------------------------------------------------------------------------
// Merlok - June 2011
// Roel - Dec 2009
// Unknown author
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// MIFARE Darkside hack
//-----------------------------------------------------------------------------

#ifndef __NONCE2KEY_H
#define __NONCE2KEY_H

#include <stdio.h>
#include <stdlib.h>
#include "crapto1.h"
#include "common.h"
//#include <stdbool.h> //for bool

typedef struct {
			  uint32_t cuid;
			  uint8_t  sector;
			  uint8_t  keytype;
			  uint32_t nonce;
			  uint32_t ar;
			  uint32_t nr;
			  uint32_t nonce2;
			  uint32_t ar2;
			  uint32_t nr2;
			} nonces_t;

int nonce2key(uint32_t uid, uint32_t nt, uint32_t nr, uint64_t par_info, uint64_t ks_info, uint64_t * key); 
bool mfkey32(nonces_t data, uint64_t *outputkey);
bool tryMfk32_moebius(nonces_t data, uint64_t *outputkey);
int tryMfk64_ex(uint8_t *data, uint64_t *outputkey);
int tryMfk64(uint32_t uid, uint32_t nt, uint32_t nr_enc, uint32_t ar_enc, uint32_t at_enc, uint64_t *outputkey);

//uint64_t mfkey32(uint32_t uid, uint32_t nt, uint32_t nr0_enc, uint32_t ar0_enc, uint32_t nr1_enc, uint32_t ar1_enc);

#endif
