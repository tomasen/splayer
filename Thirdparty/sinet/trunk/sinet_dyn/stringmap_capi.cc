#include "pch.h"
#include "stringmap_capi.h"

SINET_DYN_API _stringmap_t _stringmap_alloc()
{
  std::map<std::wstring, std::wstring>* map = new std::map<std::wstring, std::wstring>();
  return reinterpret_cast<_stringmap_t>(map);
}

SINET_DYN_API void _stringmap_free(_stringmap_t stringmap)
{
  if (!stringmap)
    delete reinterpret_cast<std::map<std::wstring, std::wstring>*>(stringmap);
}

SINET_DYN_API int _stringmap_get_size(_stringmap_t stringmap)
{
  if (stringmap)
    return reinterpret_cast<std::map<std::wstring, std::wstring>*>(stringmap)->size();
  return 0;
}

SINET_DYN_API _string_t _stringmap_get_key(_stringmap_t stringmap, int index)

{
  if (stringmap)
  {
    std::map<std::wstring, std::wstring>::iterator it = 
      reinterpret_cast<std::map<std::wstring, std::wstring>*>(stringmap)->begin();
    advance(it, index);
    return _string_alloc(it->first.c_str());
  }
  return L"";
}

SINET_DYN_API _string_t _stringmap_get_value(_stringmap_t stringmap, int index)
{
  if (stringmap)
  {
    std::map<std::wstring, std::wstring>::iterator it = 
      reinterpret_cast<std::map<std::wstring, std::wstring>*>(stringmap)->begin();
    advance(it, index);
    return _string_alloc(it->second.c_str());
  }
  return L"";
}

SINET_DYN_API _string_t _stringmap_get_find(_stringmap_t stringmap, _string_t key)
{
  if (stringmap)
  {
    std::map<std::wstring, std::wstring>* pmap =
      reinterpret_cast<std::map<std::wstring, std::wstring>*>(stringmap);
    if (pmap->find(*(reinterpret_cast<std::wstring*>(&key))) != pmap->end())
    {
      std::wstring str = (*pmap)[*(reinterpret_cast<std::wstring*>(&key))];
      return _string_alloc(str.c_str());
    }
  }
  return L"";
}

SINET_DYN_API void _stringmap_append(_stringmap_t stringmap, wchar_t* key, wchar_t* value )
{
  if (stringmap)
    (*(reinterpret_cast<std::map<std::wstring, std::wstring>*>(stringmap)))[key] = value;
}