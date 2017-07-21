#include "pch.h"
#include "intlist_capi.h"

SINET_DYN_API _intlist_t _intlist_alloc(int* intlist, size_t intlist_size)
{
  if (intlist_size == 0)
    return NULL;

  std::vector<int>* real_list = new std::vector<int>(intlist_size);
  memcpy(&(*real_list)[0], intlist, intlist_size * sizeof(int));
  return real_list;
}

SINET_DYN_API int _intlist_size(_intlist_t intlist)
{
  if (!intlist)
    return 0;

  return reinterpret_cast<std::vector<int>*>(intlist)->size();
}

SINET_DYN_API void _intlist_free(_intlist_t intlist)
{
  if (intlist)
    delete reinterpret_cast<std::vector<int>*>(intlist);
}

SINET_DYN_API int* _intlist_get(_intlist_t intlist)
{
  if (!intlist)
    return NULL;

  std::vector<int>* real_list = 
    reinterpret_cast<std::vector<int>*>(intlist);

  if (real_list->size() == 0)
    return NULL;

  return &(*real_list)[0];
}