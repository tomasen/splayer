/* tth.c - calculate TTH (Tiger Tree Hash) function.
 * Implementation written by Alexei Kravchenko.
 *
 * Copyleft:
 * I hereby release this code into the public domain. This applies worldwide.
 * I grant any entity the right to use this work for ANY PURPOSE,
 * without any conditions, unless such conditions are required by law.
 */

#include <string.h>
#include "byte_order.h"
#include "tth.h"

/**
 * Initialize context before calculaing hash.
 *
 * @param ctx context to initalize
 */
void tth_init(tth_ctx *ctx)
{
  tiger_init(&ctx->tiger);
  ctx->tiger.message[ ctx->tiger.length++ ] = 0x00;
  ctx->block_count = 0;
}

/**
 * The core transformation.
 *
 * @param ctx algorithm state
 */
static void tth_process_block_hash(tth_ctx *ctx)
{
  uint64_t it;
  unsigned pos = 0;
  unsigned char msg[24];

  for(it=1; it & ctx->block_count; it <<= 1) {
    tiger_final(&ctx->tiger, msg);
    tiger_init(&ctx->tiger);
    ctx->tiger.message[ ctx->tiger.length++ ] = 0x01;
    tiger_update(&ctx->tiger, (unsigned char*)(ctx->stack + pos), 24);
    /* note: we can cut this step, if the previous tiger_final saves directly to ctx->tiger.message+25; */
    tiger_update(&ctx->tiger, msg, 24);
    pos += 3;
  }
  tiger_final(&ctx->tiger, (unsigned char*)(ctx->stack + pos));
  ctx->block_count++;
}

/**
 * Calculate message hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param msg message chunk
 * @param size length of the message chunk
 */
void tth_update(tth_ctx *ctx, const unsigned char* msg, size_t size)
{
  size_t rest = 1025 - (size_t)ctx->tiger.length;
  for(;;) {
    if(size<rest) rest = size;
    tiger_update(&ctx->tiger, msg, rest);
    msg += rest;
    size -= rest;
    if(ctx->tiger.length < 1025) {
      return;
    }
    
    /* process block hash */
    tth_process_block_hash(ctx);
    
    /* init block hash */
    tiger_init(&ctx->tiger);
    ctx->tiger.message[ ctx->tiger.length++ ] = 0x00;
    rest = 1024;
  }
}

/**
 * Store calculated hash into the given array.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param result calculated hash in binary form
 */
void tth_final(tth_ctx *ctx, unsigned char result[24])
{
  uint64_t it = 1;
  unsigned pos = 0;
  unsigned char msg[24];
  const unsigned char* last_message;

  /* process the bytes left in the context buffer */
  if(ctx->tiger.length>1 || ctx->block_count==0) {
    tth_process_block_hash(ctx);
  }
  
  for(; it < ctx->block_count && (it & ctx->block_count)==0; it <<= 1) pos += 3;
  last_message = (unsigned char*)(ctx->stack + pos);

  for(it <<= 1; it <= ctx->block_count; it <<= 1) {
    /* merge tth sums in the tree */
    pos += 3;
    if(it & ctx->block_count) {
      tiger_init(&ctx->tiger);
      ctx->tiger.message[ ctx->tiger.length++ ] = 0x01;
      tiger_update(&ctx->tiger, (unsigned char*)(ctx->stack + pos), 24);
      tiger_update(&ctx->tiger, last_message, 24);

      tiger_final(&ctx->tiger, msg);
      last_message = msg;
    }
  }
  
  /* save result hash */
  memcpy(ctx->tiger.hash, last_message, tiger_hash_length);
  if(result) memcpy(result, last_message, tiger_hash_length);
}
