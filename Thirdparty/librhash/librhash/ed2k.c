/* ed2k.c - an implementation of EDonkey 2000 Hash Algorithm.
 * Written by Alexei Kravchenko.
 *
 * Copyleft:
 * I hereby release this code into the public domain. This applies worldwide.
 * I grant any entity the right to use this work for ANY PURPOSE,
 * without any conditions, unless such conditions are required by law.
 *
 * This file implements eMule-compatible version of algorithm.
 * Note that eDonkey and eMule ed2k hashes are different for
 * files containing exactly multiple of 9728000 bytes.
 * 
 * The file data is divided into full chunks of 9500 KiB (9728000 bytes) plus
 * a remainder chunk, and a separate 128-bit MD4 hash is computed for each. 
 * If the file length is an exact multiple of 9500 KiB, the remainder zero 
 * size chunk is still used at the end of the hash list. The ed2k hash is 
 * computed by concatenating the chunks' MD4 hashes in order and hashing the 
 * result using MD4. Although, if the file is composed of a single non-full 
 * chunk, its MD4 hash is returned with no further modifications.
 *
 * See http://en.wikipedia.org/wiki/EDonkey_network for algorithm description.
 */

#include <string.h>
#include "ed2k.h"

/* each hashed file is divided into 9500 KiB sized chunks */
#define ED2K_CHUNK_SIZE 9728000
#define USE_EMULE_ALGORITHM

/**
 * Initialize context before calculaing hash.
 *
 * @param ctx context to initalize
 */
void ed2k_init(ed2k_ctx *ctx)
{
  md4_init(&ctx->md4_context);
  md4_init(&ctx->md4_context_inner);
  ctx->not_emule = 0;
}

/**
 * Calculate message hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param msg message chunk
 * @param size length of the message chunk
 */
void ed2k_update(ed2k_ctx *ctx, const unsigned char* msg, size_t size)
{
  unsigned char chunk_md4_hash[16];
  unsigned blockleft = ED2K_CHUNK_SIZE - (unsigned)ctx->md4_context_inner.length;

  /* note: eMule-compatible algorithm hashes by md4_inner
   * the messages which sizes are multiple of 9728000 
   * and then processes obtained hash by external md4 */

  while( size >= blockleft )
  {
    if(size == blockleft && ctx->not_emule) break;

    /* if internal ed2k chunk is full, then finalize it */
    md4_update(&ctx->md4_context_inner, msg, blockleft);
    msg += blockleft;
    size -= blockleft;
    blockleft = ED2K_CHUNK_SIZE;

    /* just finished an ed2k chunk, updating md4_external context */
    md4_final(&ctx->md4_context_inner, chunk_md4_hash);
    md4_update(&ctx->md4_context, chunk_md4_hash, 16);
    md4_init(&ctx->md4_context_inner);
  }

  if(size) {
    /* hash leftovers */
    md4_update(&ctx->md4_context_inner, msg, size);
  }
}

/**
 * Store calculated hash into the given array.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param result calculated hash in binary form
 */
void ed2k_final(ed2k_ctx *ctx, unsigned char result[16])
{
  /* check if hashed message size is greater or equal to ED2K_CHUNK_SIZE */
  if( ctx->md4_context.length ) {

    /* note: weird eMule algorithm always here flushes the md4_context_inner,
     * no matter if it contains data or is empty */

    /* if any data are left in the md4_context_inner */
    if( (size_t)ctx->md4_context_inner.length > 0 || !ctx->not_emule)
    {
      /* emule algorithm processes aditional block, even if it's empty */
      unsigned char md4_digest_inner[16];
      md4_final(&ctx->md4_context_inner, md4_digest_inner);
      md4_update(&ctx->md4_context, md4_digest_inner, 16);
    }
    /* first call final to finalize the hash value */
    md4_final(&ctx->md4_context, result);
    /* result shall be always stored in one place - md4_context_inner.hash */
    memcpy(&ctx->md4_context_inner.hash, &ctx->md4_context.hash, md4_hash_size);
  } else {
    /* return just the message MD4 hash */
    if(result) md4_final(&ctx->md4_context_inner, result);
  }
}
