/* plug_openssl.h - plug-in openssl algorithms */
#ifndef RHASH_PLUG_OPENSSL_H
#define RHASH_PLUG_OPENSSL_H
#ifdef USE_OPENSSL

#ifdef __cplusplus
extern "C" {
#endif

int plug_openssl(void); /* load openssl algorithms */

#define OPENSSL_SUPPORTED_HASHES_MASK (RHASH_MD4 | RHASH_MD5 | RHASH_SHA1 | \
	RHASH_SHA224 | RHASH_SHA256 | RHASH_SHA384 | RHASH_SHA512 | \
	RHASH_RIPEMD160 | RHASH_WHIRLPOOL)

#define OPENSSL_DEFAULT_HASH_MASK (RHASH_MD5 | RHASH_SHA1 | \
	RHASH_SHA224 | RHASH_SHA256 | RHASH_SHA384 | RHASH_SHA512 | RHASH_WHIRLPOOL)

extern unsigned openssl_hash_mask; /* mask of hash sums to use */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* USE_OPENSSL */
#endif /* RHASH_PLUG_OPENSSL_H */
