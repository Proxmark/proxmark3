//-----------------------------------------------------------------------------
// Copyright (C) 2018 Merlok
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// crypto commands
//-----------------------------------------------------------------------------

#include "polarssl/libpcrypto.h"
#include <string.h>
#include <mbedtls/aes.h>
#include <mbedtls/cmac.h>

// NIST Special Publication 800-38A — Recommendation for block cipher modes of operation: methods and techniques, 2001.
int aes_encode(uint8_t *iv, uint8_t *key, uint8_t *input, uint8_t *output, int length){
	uint8_t iiv[16] = {0};
	if (iv)
		memcpy(iiv, iv, 16);
	
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	if (mbedtls_aes_setkey_enc(&aes, key, 128))
		return 1;
	if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, length, iiv, input, output))
		return 2;
	mbedtls_aes_free(&aes);

	return 0;
}

int aes_decode(uint8_t *iv, uint8_t *key, uint8_t *input, uint8_t *output, int length){
	uint8_t iiv[16] = {0};
	if (iv)
		memcpy(iiv, iv, 16);
	
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	if (mbedtls_aes_setkey_dec(&aes, key, 128))
		return 1;
	if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, length, iiv, input, output))
		return 2;
	mbedtls_aes_free(&aes);

	return 0;
}

//  NIST Special Publication 800-38B — Recommendation for block cipher modes of operation: The CMAC mode for authentication.
// https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/AES_CMAC.pdf
int aes_cmac(uint8_t *iv, uint8_t *key, uint8_t *input, uint8_t *mac, int length) {
	memset(mac, 0x00, 16);
	uint8_t iiv[16] = {0};
	if (iv)
		memcpy(iiv, iv, 16);
	
	// padding:  ISO/IEC 9797-1 Message Authentication Codes (MACs) - Part 1: Mechanisms using a block cipher
	uint8_t data[2049] = {0}; // length + 16
	memcpy(data, input, length);
	data[length] = 0x80;
	int datalen = (length & 0xfffffff0) + 0x10;
	
	//  NIST 800-38B 
	mbedtls_cipher_context_t ctx;
	mbedtls_cipher_init(&ctx);
	mbedtls_cipher_cmac_starts(&ctx, key, 128);
	mbedtls_cipher_cmac_update(&ctx, data, datalen);
	mbedtls_cipher_cmac_finish(&ctx, mac);

	return 0;
}

int aes_cmac8(uint8_t *iv, uint8_t *key, uint8_t *input, uint8_t *mac, int length) {
	uint8_t cmac[16] = {0};
	memset(mac, 0x00, 8);
	
	int res = aes_cmac(iv, key, input, cmac, length);
	if (res)
		return res;
	
	for(int i = 0; i < 8; i++) 
		mac[i] = cmac[i * 2 + 1];

	return 0;
}
