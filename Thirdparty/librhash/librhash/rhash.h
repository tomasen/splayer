/* rhash.h */
#ifndef RHASH_H
#define RHASH_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RHASH_API
/* modifier for RHash library functions */
# define RHASH_API
#endif

/* bit flags */
enum rhash_ids
{
  RHASH_CRC32 = 0x01,
  RHASH_MD4   = 0x02,
  RHASH_MD5   = 0x04,
  RHASH_SHA1  = 0x08,
  RHASH_TIGER = 0x10,
  RHASH_TTH   = 0x20,
  RHASH_BTIH  = 0x40,
  RHASH_ED2K  = 0x80,
  RHASH_AICH  = 0x100,
  RHASH_WHIRLPOOL = 0x200,
  RHASH_RIPEMD160 = 0x400,
  RHASH_GOST      = 0x800,
  RHASH_GOST_CRYPTOPRO = 0x1000,
  RHASH_HAS160    = 0x2000,
  RHASH_SNEFRU128 = 0x4000,
  RHASH_SNEFRU256 = 0x8000,
  RHASH_SHA224    = 0x10000,
  RHASH_SHA256    = 0x20000,
  RHASH_SHA384    = 0x40000,
  RHASH_SHA512    = 0x80000,
  RHASH_EDONR256    = 0x100000,
  RHASH_EDONR512    = 0x200000,
  RHASH_ALL_HASHES = RHASH_CRC32 | RHASH_MD4 | RHASH_MD5 | RHASH_ED2K | RHASH_SHA1 |
      RHASH_TIGER | RHASH_TTH | RHASH_GOST | RHASH_GOST_CRYPTOPRO |
      RHASH_BTIH | RHASH_AICH | RHASH_WHIRLPOOL | RHASH_RIPEMD160 |
      RHASH_HAS160 | RHASH_SNEFRU128 | RHASH_SNEFRU256|
      RHASH_SHA224 | RHASH_SHA256 | RHASH_SHA384 | RHASH_SHA512 |
      RHASH_EDONR256 | RHASH_EDONR512,
  RHASH_HASH_COUNT = 22
};

/* hash function information and its context */
typedef struct rhash_vector_item {
  struct rhash_hash_info* hash_info;
  void *context;
} rhash_vector_item;

/* rhash context containing contexts for several hash functions */
typedef struct rhash_context
{
  unsigned long long msg_size;
  unsigned hash_id;
  unsigned hash_vector_size; /* number of calculated hashes sum */
  void *callback,*callback_data;
  rhash_vector_item vector[1]; /* number of calculated hashes sum */
} rhash_context;

typedef struct rhash_context* rhash;
typedef void (*rhash_callback_t)(void* data, unsigned long long offset);

RHASH_API void rhash_library_init(void); /* initialize static data */

/* hi-level hashing functions */
RHASH_API int rhash_msg(unsigned hash_id, const void* message, size_t length, unsigned char* result);
RHASH_API int rhash_file(unsigned hash_id, const char* filepath, unsigned char* result);
RHASH_API int rhash_file_update(rhash ctx, FILE* fd);

#ifdef _WIN32 /* windows only function */
RHASH_API int rhash_wfile(unsigned hash_id, const wchar_t* filepath, unsigned char* result);
#endif

/* lo-level interface */
RHASH_API rhash rhash_init(unsigned hash_id);
RHASH_API rhash rhash_init_by_ids(unsigned hash_ids[], unsigned count);
RHASH_API void rhash_update(rhash ctx, const void* message, size_t length);
RHASH_API void rhash_final(rhash ctx, unsigned hash_id, unsigned char* first_result);
RHASH_API void rhash_reset(rhash ctx); /* reinitialize the context */
RHASH_API void rhash_free(rhash ctx);

/* additional lo-level functions */
RHASH_API void* rhash_get_context_ptr(rhash ctx, unsigned hash_id);
RHASH_API void  rhash_set_callback(rhash ctx, rhash_callback_t callback, void* callback_data);

#define RHASH_INFO_BASE32 1
typedef struct rhash_info
{
  unsigned hash_id;
  unsigned flags;
  size_t digest_size;
  const char* name;
} rhash_info;

/* information functions */
RHASH_API int  rhash_count(void); /* number of supported hashes */
RHASH_API int  rhash_get_digest_size(unsigned hash_id); /* size of binary digest size   */
RHASH_API int  rhash_get_hash_length(unsigned hash_id); /* length of digest hash string */
RHASH_API int  rhash_is_base32(unsigned hash_id); /* default digest output format */
RHASH_API const char* rhash_get_name(unsigned hash_id); /* get hash function name */

rhash_info* rhash_info_by_id(unsigned hash_id); /* get hash sum info by hash id */

/* manual hash sum output */
enum rhash_print_sum_flags
{
  RHPR_DEFAULT   = 0x0,
  RHPR_RAW       = 0x1,
  RHPR_HEX       = 0x2,
  RHPR_BASE32    = 0x3,
  RHPR_BASE64    = 0x4,
  RHPR_UPPERCASE = 0x8,
  RHPR_REVERSE   = 0x10, /* reverse bytes of hex representation, can be used for GOST hash */
};

/* output hash into given buffer */
size_t rhash_print_bytes(char* output, unsigned char* bytes, size_t size, int flags);
RHASH_API size_t rhash_print(char* output, rhash ctx, unsigned hash_id, int flags);

/* rhash API to set/get data via messages */
RHASH_API unsigned long rhash_transmit(unsigned msg_id, void*dst, unsigned long ldata, unsigned long rdata);

#define RMSG_SET_OPENSSL_MASK 1
#define RMSG_GET_OPENSSL_MASK 2

/* set the mask of algorithms to be used from openssl library */
#define rhash_set_openssl_mask(mask) rhash_transmit(RMSG_SET_OPENSSL_MASK, NULL, mask, 0);
#define rhash_get_openssl_mask() rhash_transmit(RMSG_GET_OPENSSL_MASK, NULL, 0, 0);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* RHASH_H */
