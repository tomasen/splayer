/* ed2k.h */
#ifndef ED2K_H
#define ED2K_H
#include "md4.h"

#ifdef __cplusplus
extern "C" {
#endif

/* algorithm context */
typedef struct ed2k_ctx
{
  md4_ctx md4_context;       /* context to hash block hashes */
  md4_ctx md4_context_inner; /* context to hash file blocks */
  int not_emule;
} ed2k_ctx;

/* hash functions */

void ed2k_init(ed2k_ctx *ctx);
void ed2k_update(ed2k_ctx *ctx, const unsigned char* msg, size_t size);
void ed2k_final(ed2k_ctx *ctx, unsigned char result[16]);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* ED2K_H */
