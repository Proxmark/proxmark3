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

#ifndef MFKEY_H
#define MFKEY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
			  uint32_t cuid;
			  uint8_t  sector;
			  uint8_t  keytype;
			  uint32_t nonce;
			  uint32_t ar;
			  uint32_t nr;
			  uint32_t at;
			  uint32_t nonce2;
			  uint32_t ar2;
			  uint32_t nr2;
			} nonces_t;

extern bool mfkey32(nonces_t data, uint64_t *outputkey);
extern bool mfkey32_moebius(nonces_t data, uint64_t *outputkey);
extern int mfkey64(nonces_t data, uint64_t *outputkey);

#endif
