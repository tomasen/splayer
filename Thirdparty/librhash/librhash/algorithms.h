/* algorithms.h - rhash library algorithms */
#ifndef RHASH_ALGORITHMS_H
#define RHASH_ALGORITHMS_H

#include <stddef.h> /* for ptrdiff_t */
#include "rhash.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RHASH_API
/* modifier for RHash library functions */
# define RHASH_API
#endif

typedef void (*pinit_t)(void*);
typedef void (*pupdate_t)(void *ctx, const void* msg, size_t size);
typedef void (*pfinal_t)(void*, unsigned char*);
typedef void (*pcleanup_t)(void*);

typedef struct rhash_hash_info
{
  rhash_info info;
  size_t context_size;
  ptrdiff_t  digest_diff;
  pinit_t    init;
  pupdate_t  update;
  pfinal_t   final;
  pcleanup_t cleanup;
} rhash_hash_info;

extern rhash_hash_info rhash_hash_info_default[RHASH_HASH_COUNT];
extern rhash_hash_info* rhash_info_table;
extern int rhash_info_size;

/* flags */
#define F_BS32 1   /* default output in base32 */
#define F_SWAP32 2 /* Big endian flag */
#define F_SWAP64 4
#define F_ME32 0

/* define endianness flags */
#ifdef CPU_LITTLE_ENDIAN
#define F_LE32 0
#define F_LE64 0
#define F_BE32 F_SWAP32
#define F_BE64 F_SWAP64
#else
#define F_LE32 F_SWAP32
#define F_LE64 F_SWAP64
#define F_BE32 0
#define F_BE64 0
#endif

void rhash_init_algorithms(void);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* RHASH_ALGORITHMS_H */
