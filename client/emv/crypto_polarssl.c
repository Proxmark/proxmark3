/*
 * libopenemv - a library to work with EMV family of smart cards
 * Copyright (C) 2015 Dmitry Eremin-Solenikov
 * Copyright (C) 2017 Merlok
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "crypto.h"
#include "crypto_backend.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "rsa.h"
#include "sha1.h"

struct crypto_hash_polarssl {
	struct crypto_hash ch;
	sha1_context ctx;
};

static void crypto_hash_polarssl_close(struct crypto_hash *_ch)
{
	struct crypto_hash_polarssl *ch = (struct crypto_hash_polarssl *)_ch;

	free(ch);
}

static void crypto_hash_polarssl_write(struct crypto_hash *_ch, const unsigned char *buf, size_t len)
{
	struct crypto_hash_polarssl *ch = (struct crypto_hash_polarssl *)_ch;

	sha1_update(&(ch->ctx), buf, len);
}

static unsigned char *crypto_hash_polarssl_read(struct crypto_hash *_ch)
{
	struct crypto_hash_polarssl *ch = (struct crypto_hash_polarssl *)_ch;

	static unsigned char sha1sum[20];
	sha1_finish(&(ch->ctx), sha1sum);
	return sha1sum;
}

static size_t crypto_hash_polarssl_get_size(const struct crypto_hash *ch)
{
	if (ch->algo == HASH_SHA_1)
		return 20;
	else
		return 0;
}

static struct crypto_hash *crypto_hash_polarssl_open(enum crypto_algo_hash hash)
{
	struct crypto_hash_polarssl *ch = malloc(sizeof(*ch));

	if (hash != HASH_SHA_1)
		return NULL;
	
	sha1_starts(&(ch->ctx));

	ch->ch.write = crypto_hash_polarssl_write;
	ch->ch.read = crypto_hash_polarssl_read;
	ch->ch.close = crypto_hash_polarssl_close;
	ch->ch.get_size = crypto_hash_polarssl_get_size;

	return &ch->ch;
}

struct crypto_pk_polarssl {
	struct crypto_pk cp;
	rsa_context ctx;
};

static struct crypto_pk *crypto_pk_polarssl_open_rsa(va_list vl)
{
	struct crypto_pk_polarssl *cp = malloc(sizeof(*cp));

	char *mod = va_arg(vl, char *);	 // N
	int modlen = va_arg(vl, size_t);
	char *exp = va_arg(vl, char *);	 // E
	int explen = va_arg(vl, size_t);

	rsa_init(&cp->ctx, RSA_PKCS_V15, 0);
	
	cp->ctx.len = modlen * 2; // size(N) in chars
	mpi_read_binary(&cp->ctx.N, (const unsigned char *)mod, modlen);
	mpi_read_binary(&cp->ctx.E, (const unsigned char *)exp, explen);
	
	if(rsa_check_pubkey(&cp->ctx) != 0) {
		fprintf(stderr, "PolarSSL key error exp=%d mod=%d.\n", explen, modlen);

		return NULL;
	}

	return &cp->cp;
}

static struct crypto_pk *crypto_pk_polarssl_open_priv_rsa(va_list vl)
{
	struct crypto_pk_polarssl *cp = malloc(sizeof(*cp));
	//gcry_error_t err;
/*	char *mod = va_arg(vl, char *);
	int modlen = va_arg(vl, size_t);
	char *exp = va_arg(vl, char *);
	int explen = va_arg(vl, size_t);
	char *d = va_arg(vl, char *);
	int dlen = va_arg(vl, size_t);
	char *p = va_arg(vl, char *);
	int plen = va_arg(vl, size_t);
//	gcry_mpi_t pmpi;
	char *q = va_arg(vl, char *);
	int qlen = va_arg(vl, size_t);
//	gcry_mpi_t qmpi;
	(void) va_arg(vl, char *);
	(void) va_arg(vl, size_t);
	(void) va_arg(vl, char *);
	(void) va_arg(vl, size_t);
	char *inv = va_arg(vl, char *);
	int invlen = va_arg(vl, size_t);*/
	/*gcry_mpi_t invmpi;

	err = gcry_mpi_scan(&pmpi, GCRYMPI_FMT_USG, p, plen, NULL);
	if (err)
		goto err_p;

	err = gcry_mpi_scan(&qmpi, GCRYMPI_FMT_USG, q, qlen, NULL);
	if (err)
		goto err_q;

	err = gcry_mpi_scan(&invmpi, GCRYMPI_FMT_USG, inv, invlen, NULL);
	if (err)
		goto err_inv;

	if (gcry_mpi_cmp (pmpi, qmpi) > 0) {
		gcry_mpi_swap (pmpi, qmpi);
		gcry_mpi_invm (invmpi, pmpi, qmpi);
	}

	err = gcry_sexp_build(&cp->pk, NULL, "(private-key (rsa (n %b) (e %b) (d %b) (p %M) (q %M) (u %M)))",
			modlen, mod, explen, exp, dlen, d,
			pmpi, qmpi, invmpi);
	if (err)
		goto err_sexp;

	err = gcry_pk_testkey(cp->pk);
	if (err)
		goto err_test;

	gcry_mpi_release(invmpi);
	gcry_mpi_release(qmpi);
	gcry_mpi_release(pmpi);

	return &cp->cp;

err_test:
	gcry_sexp_release(cp->pk);
err_sexp:
	gcry_mpi_release(invmpi);
err_inv:
	gcry_mpi_release(qmpi);
err_q:
	gcry_mpi_release(pmpi);
err_p:
	free(cp);

	fprintf(stderr, "LibGCrypt error %s/%s\n",
			gcry_strsource (err),
			gcry_strerror (err));*/
	return NULL;
}

static struct crypto_pk *crypto_pk_polarssl_genkey_rsa(va_list vl)
{
	struct crypto_pk_polarssl *cp = malloc(sizeof(*cp));
//	gcry_error_t err;
//	gcry_sexp_t params;
/*	int transient = va_arg(vl, int);
	unsigned int nbits = va_arg(vl, unsigned int);
	unsigned int exp = va_arg(vl, unsigned int);
*/
/*	err = gcry_sexp_build(&params, NULL,
			transient ?
			"(genkey (rsa (nbits %u) (rsa-use-e %u) (flags transient-key)))":
			"(genkey (rsa (nbits %u) (rsa-use-e %u)))",
			nbits, exp);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		free(cp);
		return NULL;
	}

	err = gcry_pk_genkey(&cp->pk, params);
	gcry_sexp_release(params);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		free(cp);
		return NULL;
	}*/

	return &cp->cp;
}

static void crypto_pk_polarssl_close(struct crypto_pk *_cp)
{
	struct crypto_pk_polarssl *cp = (struct crypto_pk_polarssl *)_cp;

	rsa_free(&cp->ctx);
	free(cp);
}

static int myrand(void *rng_state, unsigned char *output, size_t len) {
	size_t i;

	if(rng_state != NULL)
		rng_state = NULL;

	for(i = 0; i < len; ++i)
		output[i] = rand();
	
	return 0;
}

static unsigned char *crypto_pk_polarssl_encrypt(const struct crypto_pk *_cp, const unsigned char *buf, size_t len, size_t *clen)
{
	struct crypto_pk_polarssl *cp = (struct crypto_pk_polarssl *)_cp;
	int res;
	unsigned char *result;
	
	size_t keylen = mpi_size(&cp->ctx.N);

	result = malloc(keylen);
	if (!result) {
		printf("RSA encrypt failed. Can't allocate result memory.\n");
		return NULL;
	}
printf("## RSA len %d\n", keylen);
	res = rsa_pkcs1_encrypt(&cp->ctx, &myrand, NULL, RSA_PUBLIC, len, buf, result);
	if(res) {
		printf("RSA encrypt failed. Error: %x\n", res * -1);

		return NULL;
	}
	
	*clen = keylen;
	
	return result;
}

static unsigned char *crypto_pk_polarssl_decrypt(const struct crypto_pk *_cp, const unsigned char *buf, size_t len, size_t *clen)
{
//	struct crypto_pk_polarssl *ch = (struct crypto_pk_polarssl *)_ch;
	/*struct crypto_pk_polarssl *cp = container_of(_cp, struct crypto_pk_libgcrypt, cp);
	gcry_error_t err;
	int blen = len;
	gcry_sexp_t esexp, dsexp;
	gcry_mpi_t tmpi;
	size_t templen;
	size_t keysize;
	unsigned char *result;

	 XXX: RSA-only! 
	err = gcry_sexp_build(&esexp, NULL, "(enc-val (flags) (rsa (a %b)))",
			blen, buf);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		return NULL;
	}

	err = gcry_pk_decrypt(&dsexp, esexp, cp->pk);
	gcry_sexp_release(esexp);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		return NULL;
	}

	tmpi = gcry_sexp_nth_mpi(dsexp, 1, GCRYMPI_FMT_USG);
	gcry_sexp_release(dsexp);
	if (!tmpi)
		return NULL;

	keysize = (gcry_pk_get_nbits(cp->pk) + 7) / 8;
	result = malloc(keysize);
	if (!result) {
		gcry_mpi_release(tmpi);
		return NULL;
	}

	err = gcry_mpi_print(GCRYMPI_FMT_USG, NULL, keysize, &templen, tmpi);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		gcry_mpi_release(tmpi);
		free(result);
		return NULL;
	}

	err = gcry_mpi_print(GCRYMPI_FMT_USG, result + keysize - templen, templen, &templen, tmpi);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		gcry_mpi_release(tmpi);
		free(result);
		return NULL;
	}
	memset(result, 0, keysize - templen);

	*clen = keysize;
	gcry_mpi_release(tmpi);

	return result;*/
	return NULL;
}

static size_t crypto_pk_polarssl_get_nbits(const struct crypto_pk *_cp)
{
//	struct crypto_pk_polarssl *ch = (struct crypto_pk_polarssl *)_ch;

//	return gcry_pk_get_nbits(cp->pk);
return 0;
}

static unsigned char *crypto_pk_polarssl_get_parameter(const struct crypto_pk *_cp, unsigned param, size_t *plen)
{
	/*struct crypto_pk_polarssl *cp = container_of(_cp, struct crypto_pk_libgcrypt, cp);
	gcry_error_t err;
	gcry_sexp_t psexp;
	gcry_mpi_t tmpi;
	size_t parameter_size;
	unsigned char *result;
	const char *name;

	 XXX: RSA-only! 
	if (param == 0)
		name = "n";
	else if (param == 1)
		name = "e";
	else
		return NULL;

	psexp = gcry_sexp_find_token(cp->pk, name, 1);
	if (!psexp)
		return NULL;

	tmpi = gcry_sexp_nth_mpi(psexp, 1, GCRYMPI_FMT_USG);
	gcry_sexp_release(psexp);
	if (!tmpi)
		return NULL;

	parameter_size = (gcry_mpi_get_nbits(tmpi) + 7) / 8;
	result = malloc(parameter_size);
	if (!result) {
		gcry_mpi_release(tmpi);
		return NULL;
	}

	err = gcry_mpi_print(GCRYMPI_FMT_USG, result, parameter_size, NULL, tmpi);
	if (err) {
		fprintf(stderr, "LibGCrypt error %s/%s\n",
				gcry_strsource (err),
				gcry_strerror (err));
		gcry_mpi_release(tmpi);
		free(result);
		return NULL;
	}

	*plen = parameter_size;
	gcry_mpi_release(tmpi);

	return result;*/
	return NULL;
}

static struct crypto_pk *crypto_pk_polarssl_open(enum crypto_algo_pk pk, va_list vl)
{
	struct crypto_pk *cp;

	if (pk == PK_RSA)
		cp = crypto_pk_polarssl_open_rsa(vl);
	else
		return NULL;

	cp->close = crypto_pk_polarssl_close;
	cp->encrypt = crypto_pk_polarssl_encrypt;
	cp->get_parameter = crypto_pk_polarssl_get_parameter;
	cp->get_nbits = crypto_pk_polarssl_get_nbits;

	return cp;
}

static struct crypto_pk *crypto_pk_polarssl_open_priv(enum crypto_algo_pk pk, va_list vl)
{
	struct crypto_pk *cp;

	if (pk == PK_RSA)
		cp = crypto_pk_polarssl_open_priv_rsa(vl);
	else
		return NULL;

	cp->close = crypto_pk_polarssl_close;
	cp->encrypt = crypto_pk_polarssl_encrypt;
	cp->decrypt = crypto_pk_polarssl_decrypt;
	cp->get_parameter = crypto_pk_polarssl_get_parameter;
	cp->get_nbits = crypto_pk_polarssl_get_nbits;

	return cp;
}

static struct crypto_pk *crypto_pk_polarssl_genkey(enum crypto_algo_pk pk, va_list vl)
{
	struct crypto_pk *cp;

	if (pk == PK_RSA)
		cp = crypto_pk_polarssl_genkey_rsa(vl);
	else
		return NULL;

	cp->close = crypto_pk_polarssl_close;
	cp->encrypt = crypto_pk_polarssl_encrypt;
	cp->decrypt = crypto_pk_polarssl_decrypt;
	cp->get_parameter = crypto_pk_polarssl_get_parameter;
	cp->get_nbits = crypto_pk_polarssl_get_nbits;

	return cp;
}

static struct crypto_backend crypto_polarssl_backend = {
	.hash_open = crypto_hash_polarssl_open,
	.pk_open = crypto_pk_polarssl_open,
	.pk_open_priv = crypto_pk_polarssl_open_priv,
	.pk_genkey = crypto_pk_polarssl_genkey,
};

struct crypto_backend *crypto_polarssl_init(void)
{
	return &crypto_polarssl_backend;
}
