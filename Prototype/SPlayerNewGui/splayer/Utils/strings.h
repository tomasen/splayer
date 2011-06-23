#ifndef STRINGS_H
#define STRINGS_H

namespace string_util
{

int Split(const wchar_t* input, const wchar_t* delimiter,
          std::vector<std::wstring>& array_out);
std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);
std::wstring Utf8StringToWString(const std::string& s);
std::string WStringToUtf8String(const std::wstring& s);
void Trim(std::wstring& s);

} // namespace string_util

#endif // STRINGS_H