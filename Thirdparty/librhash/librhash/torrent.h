/* torrent.h */
#ifndef TORRENT_H
#define TORRENT_H
#include "stdint.h"
#include "util.h"
#include "sha1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define btih_hash_size  20

typedef struct torrent_ctx
{
  unsigned char btih[20]; /* resulting BTIH hash sum */
  unsigned flags;
  sha1_ctx sha1_context;
#ifdef USE_OPENSSL
  unsigned long reserved; /* need more space for openssl sha1 context */
  void *sha_init, *sha_update, *sha_final;
#endif
  blocks_vector_t blocks_hashes; /* the list of sha1 hashes of pieces */
  size_t index;           /* byte index in the current piece */
  size_t piece_length;    /* length of a torrent file piece */
  vector_t files;         /* names of files in the torrent batch */
  strbuf_t* torrent;      /* content of generated torrent file */
  char* program_name;     /* the name of the program */
  char* announce;         /* announce url */
} torrent_ctx;

void torrent_init(torrent_ctx *ctx);
void torrent_update(torrent_ctx *ctx, const void* msg, size_t size);
void torrent_final(torrent_ctx *ctx, unsigned char result[20]);
void torrent_cleanup(torrent_ctx *ctx);

size_t torrent_get_text(torrent_ctx *ctx, char** pstr);
unsigned char* torrent_get_btih(torrent_ctx *ctx);

/* possible options */
#define BT_OPT_PRIVATE 1
#define BT_OPT_INFOHASH_ONLY 2

void torrent_set_options(torrent_ctx *ctx, unsigned flags);
void torrent_add_file(torrent_ctx *ctx, const char* path, uint64_t filesize);
void torrent_set_announce(torrent_ctx *ctx, const char* announce_url);
void torrent_set_program_name(torrent_ctx *ctx, const char* name);
void torrent_set_piece_length(torrent_ctx *ctx, size_t piece_length);
size_t torrent_default_piece_length(uint64_t total_size);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* TORRENT_H */
