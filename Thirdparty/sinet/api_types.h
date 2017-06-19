#ifndef API_TYPES_H
#define API_TYPES_H

namespace sinet
{

enum postdataelem_type_t
{
  PDE_TYPE_EMPTY = 0,
  PDE_TYPE_TEXT,
  PDE_TYPE_BYTES,
  PDE_TYPE_FILE,
};

typedef std::map<std::wstring,std::wstring> si_stringmap;
typedef std::vector<unsigned char>          si_buffer;

}

#endif // API_TYPES_H