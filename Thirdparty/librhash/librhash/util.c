/* util.c - memory, vector and strings utility functions
 * written by Alexei Kravchenko.
 *
 * Copyleft:
 * I, the author, hereby release this code into the public domain.
 * This applies worldwide. I grant any entity the right to use this work for
 * ANY PURPOSE, without any conditions, unless such conditions are required
 * by law.
 */

#include "unistd.h"
#include <stdlib.h> /* size_t for vc6.0 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

/* program control and error reporting functions */

static void report_error_default(const char* srcfile, int srcline, const char* format, ...);

void (*rsh_exit)(int code) = exit;
void (*report_error)(const char* srcfile, int srcline, const char* format, ...) = report_error_default;

/**
 * Print given library failure to stderr
 */
static void report_error_default(const char* srcfile, int srcline, const char* format, ...)
{
  va_list ap;
  fprintf(stderr, "RHash: error at %s:%u: ", srcfile, srcline);
  va_start(ap, format);
  vfprintf(stderr, format, ap); /* report the error to stderr */
  va_end(ap);
}

/* MEMORY FUNCTIONS */

/**
 * Allocates a buffer via malloc with reporting memory errors to stderr.
 *
 * @param size size of the block to allocate
 * @param srcfile source file to report error on fail
 * @param srcline source code line to be reported on fail
 * @return allocated block
 */
void* rhash_malloc(size_t size, const char* srcfile, int srcline)
{
  void* res = malloc(size);
  if(!res) {
    report_error(srcfile, srcline, "malloc(%u) failed\n", (unsigned)size);
    rsh_exit(2);
  }
  return res;
}

/**
 * Duplicate c-string with reporting memory errors to stderr.
 *
 * @param str the zero-terminated string to duplicate
 * @param srcfile source file to report error on fail
 * @param srcline source code line to be reported on fail
 * @return allocated memory buffer with copied string
 */
char* rhash_strdup(const char* str, const char* srcfile, int srcline)
{
#ifndef __STRICT_ANSI__
  void* res = strdup(str);
#else
  char* res = malloc(strlen(str)+1);
  if(res) strcpy(res, str);
#endif

  if(!res) {
    report_error(srcfile, srcline, "strdup(\"%s\") failed\n", str);
    rsh_exit(2);
  }
  return res;
}

/**
 * Reallocates a buffer via realloc with reporting memory errors to stderr.
 *
 * @param mem a memory block to re-allocate
 * @param srcfile source file to report error on fail
 * @param srcline source code line to be reported on fail
 * @return re-allocated memory buffer
 */
void* rhash_realloc(void* mem, size_t size, const char* srcfile, int srcline)
{
  void* res = realloc(mem, size);
  if(!res) {
    report_error(srcfile, srcline, "realloc(%p, %u) failed\n", mem, (unsigned)size);
    rsh_exit(2);
  }
  return res;
}

/* vector functions */

/**
 * Allocate an empty vector.
 *
 * @param destructor pointer to the cleanup/deallocate function called 
 *                   on each element when the vector is destructed
 * @return allocated vector
 */
vector_t* vector_new(void (*destructor)(void*))
{
  vector_t* ptr = (vector_t*)rsh_malloc(sizeof(vector_t));
  memset(ptr, 0, sizeof(vector_t));
  ptr->destructor = destructor;
  return ptr;
}

/**
 * Allocate an empty vector of pointers to memory blocks,
 * which will be deallocated at destruction time by calling free().
 *
 * @return allocated vector
 */
struct vector_t* vector_new_simple(void)
{
  return vector_new(free);
}

/**
 * Release memory allocated by vector, but the vector structure itself.
 *
 * @param vect the vector to free
 * @param destructor function to free vector items, 
 *                   NULL if items doesn't need to be freed
 */
void vector_destroy(vector_t* vect)
{
  if(!vect) return;
  if(vect->destructor) {
    unsigned i;
    for(i=0; i<vect->size; i++) vect->destructor(vect->array[i]);
  }
  free(vect->array);
  vect->size = vect->allocated = 0;
  vect->array = 0;
}

/**
 * Release all memory allocated by vector.
 *
 * @param vect the vector to free
 * @param destructor function to free vector items, 
 *                   NULL if items doesn't need to be freed
 */
void vector_free(vector_t* vect)
{
  vector_destroy(vect);
  free(vect);
}

/**
 * Add an item to vector.
 *
 * @param vect vector to add item to
 * @param item the item to add
 */
void vector_add_ptr(vector_t* vect, void* item)
{
  /* check if vect contains enough space for next item */
  if(vect->size >= vect->allocated) {
    size_t size = (vect->allocated==0 ? 128 : vect->allocated * 2);
    vect->array = (void**)rsh_realloc(vect->array, size * sizeof(void*));
    vect->allocated = size;
  }
  /* add new item to the vector */
  vect->array[vect->size] = item;
  vect->size++;
}

/**
 * Add a sized item to vector.
 *
 * @param vect pointer to the vector to add item to
 * @param item the size of a vector item
 */
void vector_item_add_empty(struct vector_t* vect, size_t item_size)
{
  /* check if vect contains enough space for next item */
  if(vect->size >= vect->allocated) {
    size_t size = (vect->allocated==0 ? 128 : vect->allocated * 2);
    vect->array = (void**)rsh_realloc(vect->array, size * item_size);
    vect->allocated = size;
  }
  vect->size++;
}

/**
 * Initialize empty blocks vector.
 *
 * @param bvector pointer to the blocks vector
 */
void blocks_vector_init(blocks_vector_t* bvector)
{
  memset(bvector, 0, sizeof(*bvector));
  bvector->blocks.destructor = free;
}

/**
 * Free memory allocated by blocks vector, the function
 * doesn't deallocate memory additionally allocated for each element.
 *
 * @param bvector pointer to the blocks vector
 */
void blocks_vector_destroy(blocks_vector_t* bvector)
{
  vector_destroy(&bvector->blocks);
}

/* STRING BUFFER FUNCTIONS */

/**
 * Allocate an empty string buffer.
 *
 * @return allocated string buffer
 */
strbuf_t* str_new(void)
{
  strbuf_t *res = (strbuf_t*)malloc(sizeof(strbuf_t));
  memset(res, 0, sizeof(strbuf_t));
  return res;
}

/**
 * Free memory allocated by string buffer object
 *
 * @param pointer to the string buffer to destroy
 */
void str_free(strbuf_t* ptr)
{
  if(ptr) {
    free(ptr->str);
    free(ptr);
  }
}

/**
 * Grow, if needed, internal buffer of the given string to ensure it contains
 * at least new_size number bytes.
 *
 * @param str pointer to the string-buffer object
 * @param new_size number of bytes buffer must contain
 */
void strbuf_ensure_size(strbuf_t *str, size_t new_size)
{
  if(new_size >= (size_t)str->allocated) {
    if(new_size < 64) new_size = 64;
    str->str = (char*)rsh_realloc(str->str, new_size);
    str->allocated = new_size;
  }
}

/**
 * Append a sequence of single-byte characters of the specified length to
 * string buffer. The array is fully copied even if it contains the '\0'
 * character. The function ensures the string buffer still contains
 * null-termited string.
 *
 * @param str pointer to the string buffer
 * @param text the text to append
 * @param len number of character to append.
 */
void str_append_n(strbuf_t *str, const char* text, size_t length)
{
  str_ensure_size(str, str->len + length + 1);
  memcpy(str->str + str->len, text, length);
  str->len += length;
  str->str[str->len] = '\0';
}

/**
 * Append a null-terminated string to the string string buffer.
 *
 * @param str pointer to the string buffer
 * @param text the null-terminated string to append
 */
void str_append(strbuf_t *str, const char* text)
{
  str_append_n(str, text, strlen(text));
}
