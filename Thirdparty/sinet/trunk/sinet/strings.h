#ifndef STRINGS_H
#define STRINGS_H

namespace sinet
{


class strings
{
public:
  static std::string wstring_utf8string(const std::wstring& s);
  static std::wstring utf8string_wstring(const std::string& s);
};


} // namespace sinet

#endif // STRINGS_H