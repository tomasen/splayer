#include "StdAfx.h"
#include "Strings.h"

int Strings::Split(const wchar_t* input, const wchar_t* delimiter,
                   std::vector<std::wstring>& array_out)
{
  int pos = 0;
  int newpos = -1;
  int sizes2 = lstrlen(delimiter);
  int isize = lstrlen(input);

  std::vector<int> positions;

  // wcsstr returns the ptr to the found delimiter,
  // minus |input| to get the index of the delimiter character
  newpos = wcsstr(input, delimiter) - input;

  if (newpos < 0)
    return 0;

  int numfound = 0;

  while (newpos > pos)
  {
    numfound++;
    positions.push_back(newpos);
    pos = newpos;
    newpos = wcsstr(input + pos + sizes2, delimiter) - input;
  }

  for (int i=0; i <= (int)positions.size(); i++)
  {
    std::wstring s;
    if (i == 0)
      s.assign(input + i, positions[i] - i);
    else
    {
      int offset = positions[i-1] + sizes2;
      if (offset < isize)
      {
        if (i == positions.size())
          s.assign(input + offset);
        else if (i > 0)
          s.assign(input + positions[i-1] + sizes2, positions[i] - positions[i-1] - sizes2);
      }
    }
    array_out.push_back(s);
  }
  return numfound;
}

// Convert a std::string to a std::wstring
std::wstring Strings::StringToWString(const std::string& s)
{
  wchar_t* wch;
  UINT bytes = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size()+1, NULL, 0);
  wch  = new wchar_t[bytes];
  if(wch)
    bytes = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size()+1, wch, bytes);
  std::wstring str = wch;
  delete [] wch;
  return str;
}

// Convert a std::wstring to a std::string
std::string Strings::WStringToString(const std::wstring& s)
{
  char* ch;
  UINT bytes = WideCharToMultiByte(CP_ACP, 0, s.c_str(), -1, NULL, 0,
    NULL, NULL); 
  ch = new char[bytes];
  if(ch)
    bytes = WideCharToMultiByte(CP_ACP, 0, s.c_str(), -1, ch, bytes,
    NULL, NULL);
  std::string str = ch;
  delete[] ch;
  return str;
}

// Convert a std::string to a std::wstring (utf8)
std::wstring Strings::Utf8StringToWString(const std::string& s)
{
  wchar_t* wch;
  UINT bytes = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
  wch  = new wchar_t[bytes];
  if(wch)
    bytes = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wch, bytes);
  std::wstring str = wch;
  delete[] wch;
  return str;
}

// Convert a std::wstring to a std::string (utf8)
std::string Strings::WStringToUtf8String(const std::wstring& s)
{
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
}
