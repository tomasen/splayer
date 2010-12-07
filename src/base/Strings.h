#ifndef STRINGS_H
#define STRINGS_H

#include <vector>

#define ResStr_STL(id) Strings::ResourceString(id)
#include <wtypes.h>

class Strings
{
public:
  static int Split(const wchar_t* input, const wchar_t* delimiter,
                   std::vector<std::wstring>& array_out);
  static std::wstring StringToWString(const std::string& s);
  static std::string WStringToString(const std::wstring& s);
  static std::wstring Utf8StringToWString(const std::string& s);
  static std::string WStringToUtf8String(const std::wstring& s);
  static void Trim(std::wstring& s);
  static std::wstring ResourceString(int id);
  static std::wstring Format(const wchar_t* format, ...);
  static void SetResourceHandle(HINSTANCE hInstance);
  static HINSTANCE m_hInstance;
};

#endif // STRINGS_H