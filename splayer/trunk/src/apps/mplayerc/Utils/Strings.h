#ifndef STRINGS_H
#define STRINGS_H

class Strings
{
public:
  static int Split(const wchar_t* input, const wchar_t* delimiter,
                   std::vector<std::wstring>& array_out);
  static std::wstring StringToWString(const std::string& s);
  static std::string WStringToString(const std::wstring& s);
  static std::wstring Utf8StringToWString(const std::string& s);
  static std::string WStringToUtf8String(const std::wstring& s);

};

#endif // STRINGS_H