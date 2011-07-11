/* algorithms.c - the algorithms supported by the rhash library
 * written by Alexei Kravchenko.
 *
 * Copyleft:
 * I, the author, hereby place this code into the public domain.
 * This applies worldwide. I grant any entity the right to use this work for
 * ANY PURPOSE, without any conditions, unless such conditions are required
 * by law.
 */

#include <stdio.h>
#include <assert.h>

#include "byte_order.h"
#include "rhash.h"
#include "algorithms.h"

/* header files of all supported hash sums */
#include "aich.h"
#include "crc32.h"
#include "ed2k.h"
#include "edonr.h"
#include "gost.h"
#include "has160.h"
#include "md4.h"
#include "md5.h"
#include "ripemd-160.h"
#include "snefru.h"
#include "sha1.h"
#include "sha256.h"
#include "sha512.h"
#include "tiger.h"
#include "torrent.h"
#include "tth.h"
#include "whirlpool.h"

rhash_hash_info* rhash_info_table = rhash_hash_info_default;
int rhash_info_size = RHASH_HASH_COUNT;

static void fcrc32_init(uint32_t* crc32);
static void fcrc32_update(uint32_t* crc32, const unsigned char* msg, size_t size);
static void fcrc32_final(uint32_t* crc32, unsigned char* result);

/* some helper macroses
 * like shift in bytes of a message digest in a hash sum context */
#define dgshft(name) (((char*)&((name##_ctx*)0)->hash) - (char*)0)
#define dgshft2(name, field) (((char*)&((name##_ctx*)0)->field) - (char*)0)
#define ini(name) ((pinit_t)(name##_init))
#define upd(name) ((pupdate_t)(name##_update))
#define fin(name) ((pfinal_t)(name##_final))
#define iuf(name) ini(name), upd(name), fin(name)
#define diuf(name) dgshft(name), ini(name), upd(name), fin(name)

/* information about all hashes */
rhash_hash_info rhash_hash_info_default[RHASH_HASH_COUNT] =
{
  { { RHASH_CRC32,     F_BE32,  4, "CRC32" }, sizeof(uint32_t), 0, iuf(fcrc32), 0}, /* 32 bit */
  { { RHASH_MD4,       F_LE32, 16, "MD4" }, sizeof(md4_ctx), diuf(md4), 0 }, /* 128 bit */
  { { RHASH_MD5,       F_LE32, 16,  "MD5" }, sizeof(md5_ctx), diuf(md5), 0 }, /* 128 bit */
  { { RHASH_SHA1,      F_BE32, 20, "SHA1" }, sizeof(sha1_ctx), diuf(sha1), 0 }, /* 160 bit */
  { { RHASH_TIGER,     F_LE64, 24, "TIGER" }, sizeof(tiger_ctx), diuf(tiger), 0 }, /* 192 bit */
  { { RHASH_TTH,       F_BS32, 24, "TTH" }, sizeof(tth_ctx), dgshft2(tth, tiger.hash), iuf(tth), 0 }, /* 192 bit */
  { { RHASH_BTIH,      0, 20, "BTIH" }, sizeof(torrent_ctx), dgshft2(torrent, btih), iuf(torrent), (pcleanup_t)torrent_cleanup }, /* 160 bit */
  { { RHASH_ED2K,      F_LE32, 16, "ED2K" }, sizeof(ed2k_ctx), dgshft2(ed2k, md4_context_inner.hash), iuf(ed2k), 0 }, /* 128 bit */
  { { RHASH_AICH,      F_BS32, 20, "AICH" }, sizeof(aich_ctx), dgshft2(aich, sha1_context.hash), iuf(aich), (pcleanup_t)aich_cleanup }, /* 160 bit */
  { { RHASH_WHIRLPOOL, F_BE64, 64, "WHIRLPOOL" }, sizeof(whirlpool_ctx), diuf(whirlpool), 0 }, /* 512 bit */
  { { RHASH_RIPEMD160, F_LE32, 20, "RIPEMD-160" }, sizeof(ripemd160_ctx), diuf(ripemd160), 0 }, /* 160 bit */
  { { RHASH_GOST,      F_LE32, 32, "GOST" }, sizeof(gost_ctx), diuf(gost), 0 }, /* 256 bit */
  { { RHASH_GOST_CRYPTOPRO, F_LE32, 32, "GOST-CRYPTOPRO" }, sizeof(gost_ctx), dgshft(gost), ini(gost_cryptopro), upd(gost), fin(gost), 0 }, /* 256 bit */
  { { RHASH_HAS160,    F_LE32, 20, "HAS-160" }, sizeof(has160_ctx), diuf(has160), 0 }, /* 160 bit */
  { { RHASH_SNEFRU128, F_BE32, 16, "SNEFRU-128" }, sizeof(snefru_ctx), dgshft(snefru), ini(snefru128), upd(snefru), fin(snefru), 0 }, /* 128 bit */
  { { RHASH_SNEFRU256, F_BE32, 32, "SNEFRU-256" }, sizeof(snefru_ctx), dgshft(snefru), ini(snefru256), upd(snefru), fin(snefru), 0 }, /* 256 bit */
  { { RHASH_SHA224,    F_BE32, 28, "SHA-224" }, sizeof(sha256_ctx), dgshft(sha256), ini(sha224), upd(sha256), fin(sha256), 0 }, /* 224 bit */
  { { RHASH_SHA256,    F_BE32, 32, "SHA-256" }, sizeof(sha256_ctx), diuf(sha256), 0 },  /* 256 bit */
  { { RHASH_SHA384,    F_BE64, 48, "SHA-384" }, sizeof(sha512_ctx), dgshft(sha512), ini(sha384), upd(sha512), fin(sha512), 0 }, /* 384 bit */
  { { RHASH_SHA512,    F_BE64, 64, "SHA-512" }, sizeof(sha512_ctx), diuf(sha512), 0 },  /* 512 bit */
  { { RHASH_EDONR256,  F_LE32, 32,  "EDON-R256" }, sizeof(edonr_ctx), dgshft2(edonr, u.data256.hash) + 32, ini(edonr256), upd(edonr256), fin(edonr256), 0 },  /* 256 bit */
  { { RHASH_EDONR512,  F_LE64, 64,  "EDON-R512" }, sizeof(edonr_ctx), dgshft2(edonr, u.data512.hash) + 64, ini(edonr512), upd(edonr512), fin(edonr512), 0 },  /* 512 bit */
};

void rhash_init_algorithms(void)
{
  /* verify that RHASH_HASH_COUNT is the index of the major bit of RHASH_ALL_HASHES */
  assert(1 == (RHASH_ALL_HASHES >> (RHASH_HASH_COUNT - 1)));

#ifdef GENERATE_CRC32_TABLE
  crc32_init_table();
#endif
#ifdef GENERATE_GOST_LOOKUP_TABLE
  gost_init_table();
#endif
}

/* CRC32 helper functions */

/**
 * Initialize crc32 hash.
 *
 * @param crc32 pointer to the hash to initalize
 */
static void fcrc32_init(uint32_t* crc32)
{
  *crc32 = 0; /* note: context size is sizeof(uint32_t) */
}

/**
 * Calculate message CRC32 hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param crc32 pointer to the hash
 * @param msg message chunk
 * @param size length of the message chunk
 */
static void fcrc32_update(uint32_t* crc32, const unsigned char* msg, size_t size)
{
  *crc32 = get_crc32(*crc32, msg, size);
}

/**
 * Store calculated hash into the given array.
 *
 * @param crc32 pointer to the current hash value
 * @param result calculated hash in binary form
 */
static void fcrc32_final(uint32_t* crc32, unsigned char* result)
{
#if defined(CPU_IA32) || defined(CPU_X64)
   /* intel CPUs support assigment with non 32-bit aligned pointers */
  *(unsigned*)result = be2me_32(*crc32);
#else
  /* correct saving BigEndian integer on all archs */
  result[0] = (unsigned char)(*crc32 >> 24), result[1] = (unsigned char)(*crc32 >> 16);
  result[2] = (unsigned char)(*crc32 >> 8), result[3] = (unsigned char)(*crc32);
#endif
}
