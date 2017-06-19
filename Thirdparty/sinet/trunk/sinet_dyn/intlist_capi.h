#ifndef INTLIST_CAPI_H
#define INTLIST_CAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinet_dyn.h"

  typedef void* _intlist_t;
  SINET_DYN_API _intlist_t _intlist_alloc(int* intlist, size_t intlist_size);
  SINET_DYN_API int _intlist_size(_intlist_t intlist);
  SINET_DYN_API void _intlist_free(_intlist_t intlist);
  SINET_DYN_API int* _intlist_get(_intlist_t intlist);

#ifdef __cplusplus
}
#endif

#endif // INTLIST_CAPI_H