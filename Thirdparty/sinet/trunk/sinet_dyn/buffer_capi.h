#ifndef BUFFER_CAPI_H
#define BUFFER_CAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sinet_dyn.h"

  typedef void* _buffer_t;
  SINET_DYN_API _buffer_t _buffer_alloc(unsigned char* buffer, size_t buffer_size);
  SINET_DYN_API int _buffer_size(_buffer_t buffer);
  SINET_DYN_API void _buffer_free(_buffer_t buffer);
  SINET_DYN_API unsigned char* _buffer_get(_buffer_t buffer);

#ifdef __cplusplus
}
#endif

#endif // BUFFER_CAPI_H