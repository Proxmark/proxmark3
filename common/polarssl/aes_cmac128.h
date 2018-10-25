#include <stdint.h>
#include <stddef.h>
#include "aes.h"

typedef struct aes_cmac_128_context {
    aes_context aes_key;

    uint8_t K1[16];
    uint8_t K2[16];

    uint8_t X[16];

    uint8_t last[16];
    size_t last_len;
}
aes_cmac128_context;

/*
 * \brief AES-CMAC-128 context setup
 *
 * \param ctx      context to be initialized
 * \param key      secret key for AES-128
 */
void aes_cmac128_starts(aes_cmac128_context *ctx, const uint8_t K[16]);

/*
 * \brief AES-CMAC-128 process message
 *
 * \param ctx      context to be initialized
 * \param _msg     the given message
 * \param _msg_len the length of message
 */
void aes_cmac128_update(aes_cmac128_context *ctx, const uint8_t *_msg, size_t _msg_len);

/*
 * \brief AES-CMAC-128 compute T
 *
 * \param ctx      context to be initialized
 * \param T        the generated MAC which is used to validate the message
 */
void aes_cmac128_final(aes_cmac128_context *ctx, uint8_t T[16]);

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int aes_cmac_self_test( int verbose );

