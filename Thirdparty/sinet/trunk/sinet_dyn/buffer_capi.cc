#include "pch.h"
#include "buffer_capi.h"

SINET_DYN_API _buffer_t _buffer_alloc(unsigned char* buffer, size_t buffer_size)
{
  if (buffer_size == 0)
    return NULL;

  std::vector<unsigned char>* real_buf = new std::vector<unsigned char>(buffer_size);
  memcpy(&(*real_buf)[0], buffer, buffer_size);
  return real_buf;
}

SINET_DYN_API int _buffer_size(_buffer_t buffer)
{
  if (!buffer)
    return 0;

  return reinterpret_cast<std::vector<unsigned char>*>(buffer)->size();
}

SINET_DYN_API void _buffer_free(_buffer_t buffer)
{
  if (buffer)
    delete reinterpret_cast<std::vector<unsigned char>*>(buffer);
}

SINET_DYN_API unsigned char* _buffer_get(_buffer_t buffer)
{
  if (!buffer)
    return NULL;

  std::vector<unsigned char>* real_buf = 
    reinterpret_cast<std::vector<unsigned char>*>(buffer);

  if (real_buf->size() == 0)
    return NULL;

  return &(*real_buf)[0];
}