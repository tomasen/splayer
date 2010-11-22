#ifndef CONTENTTYPE_H
#define CONTENTTYPE_H

class ContentType
{
public:
  static std::wstring Get(const wchar_t* fn, std::vector<std::wstring>* redir = NULL);

};

#endif // CONTENTTYPE_H