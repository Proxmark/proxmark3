#ifndef __MFKEY32_H
#define __MFKEY32_H

#include <inttypes.h>
#include <stdbool.h>

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

bool mfkey32(nonces_t data, uint64_t *outputkey);
bool tryMfk32_moebius(nonces_t data, uint64_t *outputkey);

#endif