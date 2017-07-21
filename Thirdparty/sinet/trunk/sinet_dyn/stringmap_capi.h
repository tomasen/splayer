#ifndef STRINGMAP_CAPI_H
#define STRINGMAP_CAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinet_dyn.h"
#include "string_capi.h"

  typedef void* _stringmap_t;
  SINET_DYN_API _stringmap_t _stringmap_alloc();
  SINET_DYN_API void _stringmap_free(_stringmap_t stringmap);
  SINET_DYN_API int _stringmap_get_size(_stringmap_t stringmap);
  SINET_DYN_API _string_t _stringmap_get_key(_stringmap_t stringmap, int index);
  SINET_DYN_API _string_t _stringmap_get_value(_stringmap_t stringmap, int index);
  SINET_DYN_API _string_t _stringmap_get_find(_stringmap_t stringmap, _string_t key);
  SINET_DYN_API void _stringmap_append(_stringmap_t stringmap, _string_t key, _string_t value);

#ifdef __cplusplus
}
#endif

#endif // STRINGMAP_CAPI_H