#ifndef STRING_CAPI_H
#define STRING_CAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinet_dyn.h"

  typedef wchar_t* _string_t;

  SINET_DYN_API size_t _string_length(_string_t str);
  SINET_DYN_API _string_t _string_alloc(const wchar_t* str);
  SINET_DYN_API _string_t _string_alloc_length(const wchar_t* str, size_t len);
  SINET_DYN_API int _string_realloc(_string_t* oldstr, const wchar_t* newstr);
  SINET_DYN_API int _string_realloc_length(_string_t* oldstr, const wchar_t* newstr, size_t len);
  SINET_DYN_API void _string_free(_string_t str);

#ifdef __cplusplus
}
#endif


#endif // STRING_CAPI_H