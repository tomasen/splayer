#include "pch.h"
#include "string_capi.h"

#include <limits.h>
#include <malloc.h>
#include <string.h>

typedef unsigned long dword_t;

SINET_DYN_API size_t _string_length(_string_t str)
{
  dword_t* ptr;

  if(!str)
    return 0;

  // The string length, in bytes, is placed in a dword_t immediately proceeding
  // the string value.
  ptr = (dword_t*)str;
  ptr--;

  return (size_t)(*ptr / sizeof(wchar_t));
}

SINET_DYN_API _string_t _string_alloc(const wchar_t* str)
{
  if(!str)
    return NULL;

  return _string_alloc_length(str, wcslen(str));
}

SINET_DYN_API _string_t _string_alloc_length(const wchar_t* str,
                                              size_t len)
{
  dword_t size, *ptr;
  wchar_t* newstr;

  // Check that the size can fit in a dword_t.
  if(len >= (UINT_MAX - sizeof(wchar_t) - sizeof(dword_t)) / sizeof(wchar_t))
    return NULL;

  // Get the size of the string in bytes.
  size = sizeof(wchar_t) * len;

  // Allocate the new buffer including space for the proceeding dword_t size
  // value and the terminating nul.
  ptr = (dword_t*)malloc(sizeof(dword_t) + size + sizeof(wchar_t));
  if(!ptr)
    return NULL;

  // Set the size as the first value in the newly allocated memory and
  // increment to the string location.
  *ptr = size;
  ptr++;

  if(str != NULL)
  {
    // Copy the string to the buffer.
    memcpy(ptr, str, size);
  }
  else
  {
    // Initialize the string to zeros.
    memset(ptr, 0, size);
  }

  newstr = (wchar_t*)ptr;

  // Nul-terminate the string.
  newstr[len] = '\0';

  return (_string_t)newstr;
}

SINET_DYN_API int _string_realloc(_string_t* oldstr, const wchar_t* newstr)
{
  if(!oldstr)
    return 0;

  // Free the old string.
  _string_free(*oldstr);

  // Copy the new string.
  *oldstr = _string_alloc(newstr);

  return 1;
}

SINET_DYN_API int _string_realloc_length(_string_t* oldstr,
                                          const wchar_t* newstr,
                                          size_t len)
{
  if(!oldstr)
    return 0;

  // Check that the size can fit in a dword_t.
  if(len >= (UINT_MAX - sizeof(wchar_t) - sizeof(dword_t)) / sizeof(wchar_t))
    return 0;

  if(*oldstr)
  {
    dword_t newsize, *oldptr, *newptr;

    // Get the new size of the string in bytes.
    newsize = sizeof(wchar_t) * len;

    // Adjust the pointer to account for the dword_t immediately proceeding the
    // string value.
    oldptr = (dword_t*)*oldstr;
    oldptr--;

    // Re-allocate the buffer including space for the proceeding dword_t size
    // value and the terminating nul.
    newptr = (dword_t*)realloc(
      oldptr, sizeof(dword_t) + newsize + sizeof(wchar_t));
    if(!newptr)
      return 0;

    // Set the size as the first value in the newly allocated memory and
    // increment to the string location.
    *newptr = newsize;
    newptr++;

    // Set the string pointer to the beginning on the string in the newly
    // allocated memory.
    *oldstr = (_string_t)newptr;

    if(newstr)
    {
      // Copy the new string value.  Use of memmove() ensures that any
      // overlapping region in the old string will be copied before being
      // overwritten.
      memmove(*oldstr, newstr, newsize);

      // Nul-terminate the string.
      *oldstr[len] = '\0';
    }
  }
  else
  {
    // Allocate the string.
    *oldstr = _string_alloc_length(newstr, len);
  }

  return 1;
}

SINET_DYN_API void _string_free(_string_t str)
{
  dword_t* ptr;

  if(!str)
    return;

  // The size is placed in a dword_t immediately proceeding the string value.
  ptr = (dword_t*)str;
  ptr--;

  free(ptr);
}
