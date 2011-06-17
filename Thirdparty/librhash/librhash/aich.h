/* aich.h */
#ifndef AICH_H
#define AICH_H
#include "sha1.h"

#ifdef __cplusplus
extern "C" {
#endif

/* algorithm context */
typedef struct aich_ctx
{
  sha1_ctx sha1_context; /* context used to hash tree leaves */
#ifdef USE_OPENSSL
  unsigned long reserved; /* need more space for openssl sha1 context */
  void *sha_init, *sha_update, *sha_final;
  unsigned long sha1_length;
#endif
  unsigned index;        /* algoritm position in the current ed2k chunk */
  unsigned char (*block_hashes)[sha1_hash_size];

  void** chunk_table;    /* table of chunk hashes */
  size_t allocated;      /* allocated size of the chunk_table */
  size_t chunks_number;  /* number of ed2k chunks hashed */
  int error;             /* non-zero if a memory error occured, 0 otherwise */
} aich_ctx;

/* hash functions */

void aich_init(aich_ctx *ctx);
void aich_update(aich_ctx *ctx, const unsigned char* msg, size_t size);
void aich_final(aich_ctx *ctx, unsigned char result[20]);

/* clean up context by freeing allocated memory. 
  The function is called automatically by aich_final. 
  Should be called only when aborting hash calculations. */
void aich_cleanup(aich_ctx* ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* AICH_H */
