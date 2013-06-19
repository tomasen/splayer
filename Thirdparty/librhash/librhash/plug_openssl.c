/* plug_openssl.c - plug-in openssl algorithms
 * written by Alexei Kravchenko.
 *
 * Copyleft:
 * I, the author, hereby place this code into the public domain.
 * This applies worldwide. I grant any entity the right to use this work for
 * ANY PURPOSE, without any conditions, unless such conditions are required
 * by law.
 */
#ifdef USE_OPENSSL

#include <string.h>
#include <assert.h>
#include <openssl/opensslv.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#if (OPENSSL_VERSION_NUMBER >= 0x10000000L)
#include <openssl/whrlpool.h>
#define USE_OPENSSL_WHIRLPOOL
#endif

#include "algorithms.h"
#include "byte_order.h"
#include "plug_openssl.h"

/* the mask of ids of hashing algorithms to use from the OpenSLL library */
unsigned openssl_hash_mask = OPENSSL_DEFAULT_HASH_MASK;

#define WRAP_UPDATE(rhash_init_func, os_func, type) \
  static void rhash_init_func(void *ctx, const void* msg, size_t size) { \
    os_func((type*)ctx, msg, (unsigned long)size); \
  }
#define WRAP_FINAL(rhash_upd_func, os_func, type) \
  static void rhash_upd_func(void* ctx, unsigned char* result) { \
    os_func(result, (type*)ctx); \
  }

/* openssl final functions have different signatures, so we wrap them */
WRAP_FINAL(os_md4_final, MD4_Final, MD4_CTX)
WRAP_FINAL(os_md5_final, MD5_Final, MD5_CTX)
WRAP_FINAL(os_ripemd160_final, RIPEMD160_Final, RIPEMD160_CTX)
WRAP_FINAL(os_sha1_final, SHA1_Final, SHA_CTX)
WRAP_FINAL(os_sha224_final, SHA224_Final, SHA256_CTX)
WRAP_FINAL(os_sha256_final, SHA256_Final, SHA256_CTX)
WRAP_FINAL(os_sha384_final, SHA384_Final, SHA512_CTX)
WRAP_FINAL(os_sha512_final, SHA512_Final, SHA512_CTX)

#ifdef USE_OPENSSL_WHIRLPOOL
/* os_whirlpool_final requires special attention */
static void os_whirlpool_final(void* ctx, unsigned char* result) {
	/* pass NULL, otherwise ctx will be zeroed */
	WHIRLPOOL_Final(NULL, (WHIRLPOOL_CTX*)ctx);
	memcpy(result, ((WHIRLPOOL_CTX*)ctx)->H.c, 64);
}
#endif

rhash_hash_info openssl_methods[] = {
  { { RHASH_MD4,       F_BE32, 16, "MD4"  }, sizeof(MD4_CTX), offsetof(MD4_CTX, A), (pinit_t)MD4_Init, (pupdate_t)MD4_Update, os_md4_final, 0 }, /* 128 bit */
  { { RHASH_MD5,       F_BE32, 16, "MD5"  }, sizeof(MD5_CTX), offsetof(MD5_CTX, A), (pinit_t)MD5_Init, (pupdate_t)MD5_Update, os_md5_final, 0 }, /* 128 bit */
  { { RHASH_SHA1,      F_LE32, 20, "SHA1" }, sizeof(SHA_CTX), offsetof(SHA_CTX, h0),  (pinit_t)SHA1_Init, (pupdate_t)SHA1_Update, os_sha1_final, 0 }, /* 160 bit */
  { { RHASH_RIPEMD160, F_BE32, 20, "RIPEMD-160" }, sizeof(RIPEMD160_CTX), offsetof(RIPEMD160_CTX, A), (pinit_t)RIPEMD160_Init, (pupdate_t)RIPEMD160_Update, os_ripemd160_final, 0 }, /* 160 bit */
  { { RHASH_SHA224,    F_LE32, 28, "SHA-224" }, sizeof(SHA256_CTX), offsetof(SHA256_CTX, h), (pinit_t)SHA224_Init, (pupdate_t)SHA224_Update, os_sha224_final, 0 }, /* 224 bit */
  { { RHASH_SHA256,    F_LE32, 32, "SHA-256" }, sizeof(SHA256_CTX), offsetof(SHA256_CTX, h), (pinit_t)SHA256_Init, (pupdate_t)SHA256_Update, os_sha256_final, 0 }, /* 256 bit */
  { { RHASH_SHA384,    F_LE64, 48, "SHA-384" }, sizeof(SHA512_CTX), offsetof(SHA512_CTX, h), (pinit_t)SHA384_Init, (pupdate_t)SHA384_Update, os_sha384_final, 0 }, /* 384 bit */
  { { RHASH_SHA512,    F_LE64, 64, "SHA-512" }, sizeof(SHA512_CTX), offsetof(SHA512_CTX, h), (pinit_t)SHA512_Init, (pupdate_t)SHA512_Update, os_sha512_final, 0 }, /* 512 bit */
#ifdef USE_OPENSSL_WHIRLPOOL
  { { RHASH_WHIRLPOOL, 0, 64, "WHIRLPOOL" }, sizeof(WHIRLPOOL_CTX), offsetof(WHIRLPOOL_CTX, H.c), (pinit_t)WHIRLPOOL_Init, (pupdate_t)WHIRLPOOL_Update, os_whirlpool_final, 0 }, /* 512 bit */
#endif
};

/* The openssl_hash_info static array initialized by plug_openssl() replaces
 * rhash internal algorithms table. It is kept in an unitialized-data segment 
 * taking no space in the executable. */
rhash_hash_info openssl_hash_info[RHASH_HASH_COUNT];

/**
 * Replace several RHash internal algorithms with the openssl's.
 * It replaces MD4/MD5, SHA1/SHA2, RIPEMD, WHIRLPOOL.
 *
 * @return 1 on success, 0 on fail;
 */
int plug_openssl(void)
{
	int i, bit_index;

	/* required to use openssl _Update function without wrapping */
	assert(sizeof(size_t) == sizeof(unsigned long));
	assert(rhash_info_size <= RHASH_HASH_COUNT); /* buffer-overflow protection */

	memcpy(openssl_hash_info, rhash_info_table, sizeof(openssl_hash_info));

	/* replace internal rhash methods with the openssl ones */
	for(i = 0; i < (int)(sizeof(openssl_methods) / sizeof(rhash_hash_info)); i++) {
		if((openssl_hash_mask & openssl_methods[i].info.hash_id) == 0) continue;
		bit_index = get_ctz(openssl_methods[i].info.hash_id);
		assert(openssl_methods[i].info.hash_id == openssl_hash_info[bit_index].info.hash_id);
		memcpy(&openssl_hash_info[bit_index],
			&openssl_methods[i], sizeof(rhash_hash_info));
	}

	rhash_info_table = openssl_hash_info;
	return 1;
}
#endif /* USE_OPENSSL */
