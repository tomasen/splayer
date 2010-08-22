#ifndef STRINGS_H
#define STRINGS_H

class Strings
{
public:
  static int Split(const wchar_t* input, const wchar_t* delimiter,
                   std::vector<std::wstring>& array_out);
};

#endif // STRINGS_H