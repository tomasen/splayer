#include "pch.h"
#include "strings.h"

using namespace sinet;

// Convert a std::wstring to a std::string (utf8)
std::string strings::wstring_utf8string(const std::wstring& s)
{
#ifdef WIN32 
  char* ch;
  UINT bytes = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0,
                                   NULL, NULL); 
  ch = new char[bytes];
  if(ch)
    bytes = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, ch, bytes,
                                NULL, NULL);
  std::string str = ch;
  delete[] ch;
  return str;
#endif
#ifdef _MAC_ 
  return std::string((const char*)Utf8(s.c_str()));
#endif
#ifdef __linux__
  return wchar_utf8(s);
#endif
}

// Convert a std::string to a std::wstring (utf8)
std::wstring strings::utf8string_wstring(const std::string& s)
{
#ifdef WIN32  
  wchar_t* wch;
  UINT bytes = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
  wch  = new wchar_t[bytes];
  if(wch)
    bytes = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wch, bytes);
  std::wstring str = wch;
  delete[] wch;
  return str;
#endif
#ifdef _MAC_
  return std::wstring((const wchar_t*)Utf8(s.c_str()));
#endif
#ifdef __linux__
  return utf8_wchar(s);
#endif
}
