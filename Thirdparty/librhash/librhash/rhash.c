/* rhash.c - implementation of rhash library calls
 * written by Alexei Kravchenko.
 *
 * Copyleft:
 * I, the author, hereby place this code into the public domain.
 * This applies worldwide. I grant any entity the right to use this work for
 * ANY PURPOSE, without any conditions, unless such conditions are required
 * by law.
 */
#include <string.h> /* memset() */
#include <stdlib.h> /* free() */
#include <stddef.h> /* ptrdiff_t */
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "byte_order.h"
#include "algorithms.h"
#include "plug_openssl.h"
#include "util.h"
#include "hex.h"

/* modifier for Windows dll */
#if defined(_WIN32) && defined(RHASH_EXPORTS)
# define RHASH_API __declspec(dllexport)
#endif

#include "rhash.h" /* RHash library interface */

/**
 * Initialize static data of rhash algorithms
 */
void rhash_library_init(void)
{
  rhash_init_algorithms();
#ifdef USE_OPENSSL
  plug_openssl();
#endif
}

/**
 * Returns the number of supported hash algorithms.
 *
 * @return number  of supported hash functions
 */
int RHASH_API rhash_count(void)
{
  return rhash_info_size;
}

/* basic (lo-level) rhash library functions */

/**
 * Allocate and initialize RHASH library context.
 * After Initializing one should call rhash_update/rhash_final functions.
 * The context must be later freed by calling rhash_free.
 *
 * @param hash_id union of bit flags, containing ids of hashes to calculate.
 * @return initialized rhash context
 */
RHASH_API rhash rhash_init(unsigned hash_id)
{
  unsigned tail_bit_index; /* index of hash_id trailing bit */
  unsigned num = 0;      /* number of hashes to compute */
  rhash_context *rctx = NULL; /* allocated rhash context */
  size_t hash_size_sum = 0;   /* size of hash contexes to store in rctx */

  unsigned i, bit_index, id;
  struct rhash_hash_info* info;
  size_t aligned_size;
  char* phash_ctx;

  hash_id &= RHASH_ALL_HASHES;
  if(hash_id == 0) return NULL;

  tail_bit_index = get_ctz(hash_id); /* get trailing bit index */
  assert(tail_bit_index < RHASH_HASH_COUNT);

  id = 1 << tail_bit_index;

  if(hash_id == id) {
    /* handle the most common case of only one hash */
    num = 1;
    info = &rhash_info_table[tail_bit_index];
    hash_size_sum = info->context_size;
  } else {
    /* another case: hash_id contains several hashes */
    for(bit_index = tail_bit_index; id <= hash_id; bit_index++, id = id << 1) {
      assert(id != 0);
      assert(bit_index < RHASH_HASH_COUNT);
      info = &rhash_info_table[bit_index];
      if(hash_id & id) {
        /* align sizes by 8 bytes */
        aligned_size = (info->context_size + 7) & ~7;
        hash_size_sum += aligned_size;
        num++;
      }
    }
    assert(num > 1);
  }

  /* align the size of the rhash context common part */
  aligned_size = (offsetof(rhash_context, vector[num]) + 7) & ~7;
  assert(aligned_size >= sizeof(rhash_context));

  /* allocate rhash context with enough memory to store contexes of all used hashes */
  rctx = (rhash_context*)rsh_malloc(aligned_size + hash_size_sum);

  /* initialize common fields of the rhash context */
  rctx->msg_size = 0;
  rctx->hash_id = hash_id;
  rctx->hash_vector_size = num;
  rctx->callback = rctx->callback_data = NULL;

  /* alligned hash contextes follows rctx->vector[num] in the same memory block */
  phash_ctx = (char*)rctx + aligned_size;
  assert(phash_ctx >= (char*)&rctx->vector[num]);

  /* initialize context for every hash in a loop */
  for(bit_index = tail_bit_index, id = 1 << tail_bit_index, i = 0; 
      id <= hash_id; bit_index++, id = id << 1)
  {
    /* check if a hash function with given id shall be included into rctx */
    if((hash_id & id) != 0) {
      info = &rhash_info_table[bit_index];
      assert(info->context_size > 0);
      assert(((phash_ctx - (char*)0) & 7) == 0); /* hash context is aligned */
      assert(info->init != NULL);

      rctx->vector[i].hash_info = info;
      rctx->vector[i].context = phash_ctx;
      phash_ctx += (info->context_size + 7) & ~7;

      /* initialize the i-th hash context */
      info->init(rctx->vector[i].context);
      i++;
    }
  }

  return rctx; /* return allocated and initialized rhash context */
}

/**
 * Free RHash contex memory.
 *
 * @param ctx the context to free.
 */
void rhash_free(rhash ctx)
{
  unsigned i;
  if(!ctx) return;
  assert(ctx->hash_vector_size <= RHASH_HASH_COUNT);

  for(i = 0; i < ctx->hash_vector_size; i++) {
    struct rhash_hash_info* info = ctx->vector[i].hash_info;
    if(info->cleanup != 0) {
      info->cleanup(ctx->vector[i].context);
    }
  }

  free(ctx);
}

/**
 * Re-inititialize RHash context to reuse it.
 * Useful to speed up processing of many small messages.
 *
 * @param ctx context to reinitialize
 */
RHASH_API void rhash_reset(rhash ctx)
{
  unsigned i;
  assert(ctx->hash_vector_size > 0 && ctx->hash_vector_size < RHASH_HASH_COUNT);

  /* re-initialize every hash in a loop */
  for(i = 0; i < ctx->hash_vector_size; i++) {
    struct rhash_hash_info* info = ctx->vector[i].hash_info;
    if(info->cleanup != 0) {
      info->cleanup(ctx->vector[i].context);
    }

    assert(info->init != NULL);
    info->init(ctx->vector[i].context);
  }
}

/**
 * Calculate hashes of message.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param pointer to the rhash context
 * @param msg message chunk
 * @param size length of the message chunk
 */
RHASH_API void rhash_update(rhash ctx, const void* message, size_t length)
{
  unsigned i;
  assert(ctx->hash_vector_size <= RHASH_HASH_COUNT);
  ctx->msg_size += length;

  /* call update method for every algorithm */
  for(i = 0; i < ctx->hash_vector_size; i++) {
    struct rhash_hash_info* info = ctx->vector[i].hash_info;
    assert(info->update != 0);
    info->update(ctx->vector[i].context, message, length);
  }
}

/**
 * Finalize hash calculation and optionally store the first hash.
 *
 * @param ctx rhash context
 * @param first_result optional buffer to store a calculated hash with the lowest available id
 */
RHASH_API void rhash_final(rhash ctx, unsigned hash_id, unsigned char* first_result)
{
  unsigned i = 0;
  unsigned char buffer[130];
  unsigned char* out = (first_result ? first_result : buffer);
  assert(ctx->hash_vector_size <= RHASH_HASH_COUNT);

  /* call final method for every algorithm */
  for (i = 0; i < ctx->hash_vector_size; ++i)
  {
    struct rhash_hash_info *info = ctx->vector[i].hash_info;
    if (info->info.hash_id == hash_id)
    {
      info->final(ctx->vector[i].context, out);
      out = buffer;
    }  
  }
  
  //for(i = 0; i < ctx->hash_vector_size; i++) {
  //  struct rhash_hash_info* info = ctx->vector[i].hash_info;
  //  assert(info->final != 0);
  //  assert(info->info.digest_size < sizeof(buffer));
  //  info->final(ctx->vector[i].context, out);
  //  out = buffer;
  //}
}

/**
 * Store digest for given hash_id. 
 * If hash_id is zero, function stores digest for a hash with the lowest id found in the context.
 * For nonzero hash_id the context must contain it, otherwise function siliently does nothing.
 *
 * @param ctx rhash context
 * @param hash_id id of hash to retrive or zero for hash with the lowest available id
 * @param result buffer to put the hash into
 */
static void rhash_put_digest(rhash ctx, unsigned hash_id, unsigned char* result)
{
  unsigned i;
  rhash_vector_item *item;
  struct rhash_hash_info* info;
  unsigned char* digest;

  assert(ctx);
  assert(ctx->hash_vector_size > 0 && ctx->hash_vector_size <= RHASH_HASH_COUNT);

  if(hash_id == 0) {
    item = &ctx->vector[0]; /* get the first hash */
    info = item->hash_info;
  } else {
    for(i = 0;; i++) {
      if(i >= ctx->hash_vector_size) {
        return; /* hash_id not found, do nothing */
      }
      item = &ctx->vector[i];
      info = item->hash_info;
      if(info->info.hash_id == hash_id) break;
    }
  }
  digest = ((unsigned char*)item->context + info->digest_diff);
  if(info->info.flags & F_SWAP32) {
    u32_swap_copy(result, digest, info->info.digest_size);
  } else if(info->info.flags & F_SWAP64) {
    u64_swap_copy(result, 0, digest, info->info.digest_size);
  } else {
    memcpy(result, digest, info->info.digest_size);
  }
}

/**
 * Return pointer to the requested hash context.
 *
 * @param ctx rhash context containig all hash info
 * @param hash_id the id of the hash to request
 * @return requested hash context if found, NULL on error.
 */
RHASH_API void* rhash_get_context_ptr(struct rhash_context *ctx, unsigned hash_id)
{
  unsigned i;
  for(i = 0; i < ctx->hash_vector_size; i++) {
    struct rhash_hash_info* info = ctx->vector[i].hash_info;
    if(info->info.hash_id == hash_id) return ctx->vector[i].context;
  }
  return NULL;
}

/**
 * Set the callback function to be called from the 
 * rhash_file() and rhash_file_update() functions
 * on processing every file block. The file block
 * size is set internaly by rhash and now is 8 KiB.
 *
 * @param ctx rhash context
 * @param callback pointer to the callback function
 * @param callback_data pointer to data passed to the callback
 */
RHASH_API void rhash_set_callback(rhash ctx, rhash_callback_t callback, void* callback_data)
{
  ctx->callback = callback;
  ctx->callback_data = callback_data;
}


/* hi-level message hashing interface */

/**
 * Compute a hash of the given message.
 *
 * @param hash_id id of hash sum to compute
 * @param message the message to process
 * @param length message length
 * @return 0 on success, -1 on error
 */
RHASH_API int rhash_msg(unsigned hash_id, const void* message, size_t length, unsigned char* result)
{
  rhash ctx;
  hash_id &= RHASH_ALL_HASHES;
  ctx = rhash_init(hash_id);
  if(ctx == NULL) return -1;
  rhash_update(ctx, message, length);
  rhash_final(ctx, hash_id, result);
  rhash_free(ctx);
  return 0;
}

/**
 * Hash a file or stream. Multiple hashes can be computed.
 * First, inintialize ctx parameter with rhash_init() before calling
 * rhash_file_update(). Then use rhash_final() and rhash_print() 
 * to retrive hash values. Finaly call rhash_free() on ctx
 * to free allocated memory or call rhash_reset() to reuse ctx.
 *
 * @param ctx rhash context
 * @param fd descriptor of the file to hash
 * @return 0 on success, -1 on error and errno is set
 */
RHASH_API int rhash_file_update(rhash ctx, FILE* fd)
{
  const size_t block_size = 8192;
  unsigned char *buffer, *pmem;
  size_t length = 0, align8;
  int res = 0;

  if(ctx == NULL) {
    errno = EINVAL;
    return -1;
  }

  pmem = (unsigned char*)rsh_malloc(block_size + 8);
  align8 = ((unsigned char*)0 - pmem) & 7;
  buffer = pmem + align8;

  while(!feof(fd)) {
    length = fread(buffer, 1, block_size, fd);
    /* read can return -1 on error */
    if(length == (size_t)-1) {
      res = -1; /* note: fread sets errno */
      break;
    }
    rhash_update(ctx, buffer, length);

    if(ctx->callback)
      ((rhash_callback_t)ctx->callback)(ctx->callback_data, ctx->msg_size);
  }

  free(buffer);
  return res;
}

/**
 * Compute a single hash for given file.
 *
 * @param hash_id id of hash sum to compute
 * @param filepath path to the file to hash
 * @param result buffer to receive hash value with the lowest requested id
 * @return 0 on success, -1 on error and errno is set
 */
RHASH_API int rhash_file(unsigned hash_id, const char* filepath, unsigned char* result)
{
  FILE* fd;
  rhash ctx;
  int res;

  hash_id &= RHASH_ALL_HASHES;
  if(hash_id == 0) return -1;

  if((fd = fopen(filepath, "rb")) == NULL) return -1;

  if((ctx = rhash_init(hash_id)) == NULL) return -1;

  res = rhash_file_update(ctx, fd); /* hash the file */
  fclose(fd);

  rhash_final(ctx, hash_id, result);
  rhash_free(ctx);
  return res;
}

#ifdef _WIN32 /* windows only function */
#include <share.h>

/**
 * Compute a single hash for given file.
 *
 * @param hash_id id of hash sum to compute
 * @param filepath path to the file to hash
 * @param result buffer to receive hash value with the lowest requested id
 * @return 0 on success, -1 on error, -1 on error and errno is set
 */
RHASH_API int rhash_wfile(unsigned hash_id, const wchar_t* filepath, unsigned char* result)
{
  FILE* fd;
  rhash ctx;
  int res;

  hash_id &= RHASH_ALL_HASHES;
  if(hash_id == 0) return -1;

  if((fd = _wfsopen(filepath, L"rb", _SH_DENYWR)) == NULL) return -1;
  
  if((ctx = rhash_init(hash_id)) == NULL) return -1;

  res = rhash_file_update(ctx, fd); /* hash the file */
  fclose(fd);

  rhash_final(ctx, hash_id, result);
  rhash_free(ctx);
  return 0;
}
#endif


/* information functions */

/*rhash_info* rhash_info_by_index(int index)
{
  return (((unsigned)index) < RHASH_HASH_COUNT ? 
      &(rhash_info_table[index].info) : NULL);
}*/

/**
 * Returns information about a hash function by its hash_id.
 *
 * @param hash_id the id of hash algorithm
 * @return pointer to the rhash_info structure containing the information
 */
rhash_info* rhash_info_by_id(unsigned hash_id)
{
  hash_id &= RHASH_ALL_HASHES;
  /* check that only one bit is set */
  if(hash_id != (hash_id & -(int)hash_id)) return NULL;
  /* note: alternative condition is (hash_id == 0 || (hash_id & (hash_id - 1)) != 0) */
  return &(rhash_info_table[get_ctz(hash_id)].info);
}

/**
 * Detect default digest output format for given hash algorithm.
 *
 * @param hash_id the id of hash algorithm
 * @return 1 for base32 format, 0 for hexadecimal
 */
RHASH_API int rhash_is_base32(unsigned hash_id)
{
  /* fast method is just to test a bitmask */
  return ((hash_id & (RHASH_TTH|RHASH_AICH)) != 0);
}

/**
 * Returns size of binary digest for given hash algorithm.
 *
 * @param hash_id the id of hash algorithm
 * @return digest size in bytes
 */
RHASH_API int rhash_get_digest_size(unsigned hash_id)
{
  hash_id &= RHASH_ALL_HASHES;
  if(hash_id == 0 || (hash_id & (hash_id - 1)) != 0) return -1;
  return (int)rhash_info_table[get_ctz(hash_id)].info.digest_size;
}

/**
 * Returns length of digest hash string in default output format.
 *
 * @param hash_id the id of hash algorithm
 * @return the length of hash string
 */
RHASH_API int rhash_get_hash_length(unsigned hash_id)
{
  rhash_info* info = rhash_info_by_id(hash_id);
  return (int)(info ? (info->flags & F_BS32 ? 
      BASE32_LENGTH(info->digest_size) : info->digest_size * 2) : 0);
}

/**
 * Returns a name of given hash algorithm.
 *
 * @param hash_id the id of hash algorithm
 * @return algorithm name
 */
RHASH_API const char* rhash_get_name(unsigned hash_id)
{
  rhash_info* info = rhash_info_by_id(hash_id);
  return (info ? info->name : 0);
}

/* hash sum output */

/**
 * Print text presentation of a given hash sum to specified buffer
 *
 * @param output a buffer to print the hash to
 * @param context algorithms state
 * @param hash_id id of the sum to print
 * @param flags  controls how to print the sum, can contain flags 
 *               RHPR_UPPERCASE, RHPR_HEX, RHPR_BASE32, e.t.c.
 * @return number of writen characters on success, 0 on fail
 */
size_t rhash_print_bytes(char* output, unsigned char* bytes, size_t size, int flags)
{
  size_t str_len;
  int upper_case = (flags & RHPR_UPPERCASE);
  int format = (flags & ~(RHPR_UPPERCASE|RHPR_REVERSE));
  
  switch(format) {
    case RHPR_HEX:
      str_len = size * 2;
      byte_to_hex(output, bytes, (unsigned)size, upper_case);
      break;
    case RHPR_BASE32:
      str_len = BASE32_LENGTH(size);
      byte_to_base32(output, bytes, (unsigned)size, upper_case);
      break;
    case RHPR_BASE64:
      str_len = BASE64_LENGTH(size);
      byte_to_base64(output, bytes, (unsigned)size);
      break;
    default:
      str_len = size;
      memcpy(output, bytes, size);
      break;
  }
  return str_len;
}

/**
 * Print text presentation of a hash sum with given hash_id to output buffer.
 * If hash_id is zero, then print hash sum with the lowest id stored in hash
 * context. Function fails if hash_id doesn't exist within the context.
 *
 * @param output a buffer to print the hash to
 * @param context algorithms state
 * @param hash_id id of the hash sum to print or 0 to print the first hash
 *                saved in the context.
 * @param flags  controls how to print the sum, can contain flags 
 *               RHPR_UPPERCASE, RHPR_HEX, RHPR_BASE32, RHPR_BASE64, e.t.c.
 * @return number of writen characters on success, 0 on fail
 */
size_t RHASH_API rhash_print(char* output, rhash context, unsigned hash_id, int flags)
{
  rhash_info* info;
  unsigned char digest[80];
  size_t digest_size;

  info = (hash_id != 0 ? rhash_info_by_id(hash_id) :
      &context->vector[0].hash_info->info);

  if(info == NULL) return 0;
  digest_size = info->digest_size;
  assert(digest_size <= 64);

  /* note: use info->hash_id, cause hash_id can be 0 */
  rhash_put_digest(context, info->hash_id, digest);

  /* use default text presentation if not specified by flags */
  if((flags & ~(RHPR_UPPERCASE|RHPR_REVERSE)) == 0) {
    flags |= (info->flags & RHASH_INFO_BASE32 ? RHPR_BASE32 : RHPR_HEX);
  }

  if((flags & ~RHPR_UPPERCASE) == (RHPR_REVERSE|RHPR_HEX)) {
    /* reverse the digest */
    unsigned char *p = digest, *r = digest + digest_size - 1;
    char tmp;
    for(; p < r; p++, r--) {
      tmp = *p;
      *p = *r;
      *r = tmp;
    }
  }

  return rhash_print_bytes(output, digest, digest_size, flags);
}

#if defined(_WIN32) && defined(RHASH_EXPORTS)
#include <windows.h>
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      rhash_library_init();
      break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
#endif

/**
 * Process a rhash message.
 *
 * @param msg_id message identifier
 * @param dst message destination (can be NULL for generic messages)
 * @param ldata data depending on message
 * @param rdata data depending on message
 * @return message-specific data
 */
RHASH_API unsigned long rhash_transmit(unsigned msg_id, void* dst, unsigned long ldata, unsigned long rdata)
{
  (void)msg_id;
  (void)dst;
  (void)ldata;
  (void)rdata;

  switch(msg_id) {
#ifdef USE_OPENSSL
    case RMSG_SET_OPENSSL_MASK:
      openssl_hash_mask = (unsigned)ldata;
      break;
    case RMSG_GET_OPENSSL_MASK:
      return openssl_hash_mask;
#endif
    default:
      return -1;
  }
  return 0;
}
