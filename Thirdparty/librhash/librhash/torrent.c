/* torrent.c - create BitTorrent files and calculate BitTorrent  InfoHash (BTIH).
 * Implementation written by Alexei Kravchenko.
 *
 * Copyleft:
 * I hereby release this code into the public domain. This applies worldwide.
 * I grant any entity the right to use this work for ANY PURPOSE,
 * without any conditions, unless such conditions are required by law.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>  /* time() */

#include "byte_order.h"
#include "algorithms.h"
#include "torrent.h"

#ifdef USE_OPENSSL
#define SHA1_INIT(ctx) ((pinit_t)ctx->sha_init)(&ctx->sha1_context)
#define SHA1_UPDATE(ctx, msg, size) ((pupdate_t)ctx->sha_update)(&ctx->sha1_context, (msg), (size))
#define SHA1_FINAL(ctx, result) ((pfinal_t)ctx->sha_final)(&ctx->sha1_context, (result))
#else
#define SHA1_INIT(ctx) sha1_init(&ctx->sha1_context)
#define SHA1_UPDATE(ctx, msg, size) sha1_update(&ctx->sha1_context, (msg), (size))
#define SHA1_FINAL(ctx, result) sha1_final(&ctx->sha1_context, (result))
#endif

/**
 * Initialize torrent context before calculaing hash.
 *
 * @param ctx context to initalize
 */
void torrent_init(torrent_ctx* ctx)
{
  memset(ctx, 0, sizeof(torrent_ctx));
  ctx->blocks_hashes.blocks.destructor = free;
  ctx->files.destructor = free;
  ctx->piece_length = 65536;

#ifdef USE_OPENSSL
  {
    /* get the methods of the selected SHA1 algorithm */
    rhash_hash_info *sha1_info = &rhash_info_table[3];
    assert(sha1_info->info.hash_id == RHASH_SHA1);
    assert(sha1_info->context_size <= (sizeof(sha1_ctx) + sizeof(unsigned long)));
    ctx->sha_init = sha1_info->init;
    ctx->sha_update = sha1_info->update;
    ctx->sha_final = sha1_info->final;
  }
#endif

  SHA1_INIT(ctx);
}

/**
 * Clean up torrent context by freeing all dynamically 
 * allocated memory.
 */
void torrent_cleanup(torrent_ctx *ctx)
{
  assert(ctx);
  vector_destroy(&ctx->blocks_hashes.blocks);
  vector_destroy(&ctx->files);
  free(ctx->program_name);
  free(ctx->announce);
  ctx->announce = ctx->program_name = 0;
  str_free(ctx->torrent);
}

static void make_torrent(torrent_ctx *ctx);

/* macros to access array of SHA1 hashes of 1Kb blocks */
#define BT_HASH_SIZE 20
#define BT_BLOCK_SIZE 256
#define BT_GET(blocks_hashes, index) blocks_vector_get_ptr(blocks_hashes, index, BT_BLOCK_SIZE, BT_HASH_SIZE)
#define BT_ADD_HASH(ctx, hash) blocks_vector_add((&ctx->blocks_hashes), hash, BT_BLOCK_SIZE, BT_HASH_SIZE)

/**
 * A filepath and filesize information.
 */
typedef struct file_n_size_info
{
  uint64_t size;
  char path[1];
} file_n_size_info;

/**
 * Add a file info into the batch of files of given torrent.
 *
 * @param ctx torrent algorithm context
 * @param path file path
 * @param size file size
 */
void torrent_add_file(torrent_ctx *ctx, const char* path, uint64_t filesize)
{
  size_t len = strlen(path);
  file_n_size_info* info = (file_n_size_info*)rsh_malloc(sizeof(uint64_t) + len + 1);
  info->size = filesize;
  memcpy(info->path, path, len + 1);
  vector_add_ptr(&ctx->files, info);

  /* recalculate piece length (but only if hashing not started yet) */
  if(ctx->blocks_hashes.size == 0 && ctx->index == 0) {
    /* note: in case of batch of files should use a total batch size */
    ctx->piece_length = torrent_default_piece_length(filesize);
  }
}

/**
 * Calculate message hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param msg message chunk
 * @param size length of the message chunk
 */
void torrent_update(torrent_ctx *ctx, const void* msg, size_t size)
{
  unsigned char hash[20];
  const unsigned char* pmsg = (const unsigned char*)msg;
  size_t rest = (size_t)(ctx->piece_length - ctx->index);
  assert(ctx->index < ctx->piece_length);

  while(size > 0) {
    size_t left = (size < rest ? size : rest);
    SHA1_UPDATE(ctx, pmsg, left);
    if(size < rest) {
      ctx->index += left;
      break;
    }
    SHA1_FINAL(ctx, hash);
    BT_ADD_HASH(ctx, hash); /* store a piece hash */
    SHA1_INIT(ctx);
    ctx->index = 0;

    pmsg += rest;
    size -= rest;
    rest = ctx->piece_length;
  }
}

/**
 * Finalize hashing and optionally store calculated hash into the given array.
 * If the result parameter is NULL, the hash is not stored, but it is
 * accessible by torrent_get_btih().
 *
 * @param ctx the algorithm context containing current hashing state
 * @param result pointer to the array store message hash into
 */
void torrent_final(torrent_ctx *ctx, unsigned char result[20])
{
  unsigned char hash[20];
  if(ctx->index > 0) {
    SHA1_FINAL(ctx, hash);
    BT_ADD_HASH(ctx, hash);
  }

  make_torrent(ctx);
  if(result) memcpy(result, ctx->btih, btih_hash_size);
}

/* bittorrent functions */

/**
 * Print 64-bit number with trailing '\0' to string buffer.
 *
 * @param dst output buffer
 * @param number the number to print
 * @return length of the printed number (without trailing '\0')
 */
static int sprintI64(char *dst, uint64_t number)
{
  /* The biggest number has 20 digits: 2^64 = 18 446 744 073 709 551 616 */
  char buf[24];
  size_t len;
  char *p = buf + 23;
  *p = '\0'; /* last symbol should be '\0' */
  if(number == 0) {
    *(--p) = '0';
  } else {
    for(; p >= buf && number != 0; number /= 10) {
      *(--p) = '0' + (char)(number % 10);
    }
  }
  len = buf + 23 - p;
  memcpy(dst, p, len + 1);
  return (int)len;
}

/**
 * B-encode given integer.
 *
 * @param out the string buffer to output encoded integer to
 * @param number the integer to output
 */
static void bt_bencode_int(strbuf_t *out, uint64_t number)
{
  char* p;
  /* add up to 20 digits and 2 letters */
  str_ensure_size(out, out->len + 22);
  p = out->str + out->len;
  *(p++) = 'i';
  p += sprintI64(p, number);
  *(p++) = 'e';
  *p = '\0'; /* terminate string with \0 */

  out->len = (p - out->str);
}

/**
 * B-encode a string.
 *
 * @param out the string buffer to put encoded string into
 * @param str the string to encode
 */
static void bt_bencode_str(strbuf_t *out, const char* str)
{
  size_t len = strlen(str);
  int num_len;
  char* p;
  str_ensure_size(out, out->len + len + 21);

  p = out->str + out->len;
  p += (num_len = sprintI64(p, len));
  out->len += len + num_len + 1;

  *(p++) = ':';
  memcpy(p, str, len + 1); /* copy with trailing '\0' */
}

/**
 * B-encode array of SHA1 hashes of file pieces.
 *
 * @param out the string buffer to put encoded array into
 * @param ctx pointer to the torrent structure containing hashes
 */
static void bt_bencode_pieces(strbuf_t *out, torrent_ctx* ctx)
{
  int pieces_length = ctx->blocks_hashes.size * BT_HASH_SIZE;
  int num_len;
  int size, i;
  char* p;

  str_ensure_size(out, out->len + pieces_length + 21);
  p = out->str + out->len;
  p += (num_len = sprintI64(p, pieces_length));
  out->len += pieces_length + num_len + 1;

  *(p++) = ':';
  p[pieces_length] = '\0'; /* terminate with \0 just in case */

  for(size = ctx->blocks_hashes.size, i = 0; size > 0; size -= BT_BLOCK_SIZE, i++) {
    memcpy(p, ctx->blocks_hashes.blocks.array[i], (size < BT_BLOCK_SIZE ? size : BT_BLOCK_SIZE) * BT_HASH_SIZE);
    p += BT_BLOCK_SIZE * BT_HASH_SIZE;
  }
}

/**
 * Calculate default torrent piece length, using uTorrent algorithm.
 * Algorithm: 
 *  length = 64K for total_size < 64M,
 *  length = 4M for total_size >= 2G,
 *  length = top_bit(total_size) / 512 otherwise.
 *
 * @param total_size total hashed batch size of torrent file
 * @return piece length used by torrent file
 */
size_t torrent_default_piece_length(uint64_t total_size)
{
  uint64_t hi_bit;
  if(total_size < 67108864) return 65536;
  if(total_size >= I64(2147483648) ) return 4194304;
  for(hi_bit = 67108864 << 1; hi_bit <= total_size; hi_bit <<= 1);
  return (size_t)(hi_bit >> 10);
}

/**
 * Generate torrent file content
 * @see http://wiki.theory.org/BitTorrentSpecification
 *
 * @param ctx the torrent algorithm context
 */
static void make_torrent(torrent_ctx *ctx)
{
  uint64_t total_size = 0;
  size_t info_start_pos;
  
  assert(ctx->torrent == NULL);
  assert(ctx->files.size <= 1);

  ctx->torrent = str_new();
  if(ctx->piece_length == 0) {
    if(ctx->files.size == 1) {
      total_size = ((file_n_size_info*)ctx->files.array[0])->size;
    }
    ctx->piece_length = torrent_default_piece_length(total_size);
  }

  /* write torrent header to the ctx->torrent string bufer */
  if((ctx->flags & BT_OPT_INFOHASH_ONLY) == 0) {
    str_append(ctx->torrent, "d");
    if(ctx->announce) {
      str_append(ctx->torrent, "8:announce");
      bt_bencode_str(ctx->torrent, ctx->announce);
    }

    if(ctx->program_name) {
      str_append(ctx->torrent, "10:created by");
      bt_bencode_str(ctx->torrent, ctx->program_name);
    }

    str_append(ctx->torrent, "13:creation date");
    bt_bencode_int(ctx->torrent, (uint64_t)time(NULL));
  }

  str_append(ctx->torrent, "8:encoding5:UTF-8");

  str_append(ctx->torrent, "4:infod"); /* start info dictionary */
  info_start_pos = ctx->torrent->len - 1;

  if(ctx->files.size == 1) {
    file_n_size_info* f = (file_n_size_info*)ctx->files.array[0];
    str_append(ctx->torrent, "6:length");
    bt_bencode_int(ctx->torrent, f->size);

    /* note: for one file f->path must be a basename */
    str_append(ctx->torrent, "4:name");
    bt_bencode_str(ctx->torrent, f->path);
  }
  str_append(ctx->torrent, "12:piece length");
  bt_bencode_int(ctx->torrent, ctx->piece_length);

  str_append(ctx->torrent, "6:pieces");
  bt_bencode_pieces(ctx->torrent, ctx);

  if(ctx->flags & BT_OPT_PRIVATE) {
    str_append(ctx->torrent, "7:privatei1e");
  }
  str_append(ctx->torrent, "ee");

  SHA1_INIT(ctx);
  SHA1_UPDATE(ctx, (unsigned char*)ctx->torrent->str + info_start_pos,
    ctx->torrent->len - info_start_pos - 1);
  SHA1_FINAL(ctx, ctx->btih);

}

/* Getters/Setters */

/**
 * Get BTIH (BitTorrent Info Hash) value.
 *
 * @param ctx the torrent algorithm context
 * @return the 20-bytes long BTIH value
 */
unsigned char* torrent_get_btih(torrent_ctx *ctx)
{
  return ctx->btih;
}

/**
 * Set the torrent algorithm options.
 *
 * @param ctx the torrent algorithm context
 * @param flags the options to set
 */
void torrent_set_options(torrent_ctx *ctx, unsigned flags)
{
  ctx->flags = flags;
}

/**
 * Set optional name of the program generating the torrent
 * for storing into torrent file.
 *
 * @param ctx the torrent algorithm context
 * @param name the program name
 */
void torrent_set_program_name(torrent_ctx *ctx, const char* name)
{
  ctx->program_name = rsh_strdup(name);
}

/**
 * Set length of a file piece.
 *
 * @param ctx the torrent algorithm context
 * @param piece_length the piece length in bytes
 */
void torrent_set_piece_length(torrent_ctx *ctx, size_t piece_length)
{
  ctx->piece_length = piece_length;
}

/**
 * Set torrent announcement-url for storing into torrent file.
 *
 * @param ctx the torrent algorithm context
 * @param announce_url the announcement-url
 */
void torrent_set_announce(torrent_ctx *ctx, const char* announce_url)
{
  free(ctx->announce);
  ctx->announce = rsh_strdup(announce_url);
}

/**
 * Get the content of generated torrent file.
 *
 * @param ctx the torrent algorithm context
 * @param pstr pointer to pointer recieving the buffer with file content
 * @return length of the torrent file content
 */
size_t torrent_get_text(torrent_ctx *ctx, char** pstr)
{
  assert(ctx->torrent);
  *pstr = ctx->torrent->str;
  return ctx->torrent->len;
}
