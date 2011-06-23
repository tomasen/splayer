#include "StdAfx.h"
#include "strings.h"

namespace string_util
{

std::wstring GetWord(std::wstring & s, const std::wstring delim = L" ")
{
  // find delimiter  
  std::wstring::size_type i (s.find(delim));

  // split into before and after delimiter
  std::wstring w (s.substr(0, i));

  // if no delimiter, remainder is empty
  if (i == std::wstring::npos)
    s.erase ();
  else
    // erase up to the delimiter
    s.erase (0, i + delim.size());

  // return first word in line
  return w;
}

int Split(const wchar_t* input, const wchar_t* delimiter,
          std::vector<std::wstring>& array_out)
{
  array_out.clear (); // ensure vector empty

  std::wstring s1     = input;
  std::wstring delim  = delimiter;
  // no string? no elements
  if (s1.empty())
    return 0;

  // add to vector while we have a delimiter
  while (!s1.empty() && s1.find(delim) != std::wstring::npos)
    array_out.push_back(GetWord(s1, delim));

  // add final element
  array_out.push_back(s1);
  return array_out.size();
}

// Convert a std::string to a std::wstring
std::wstring StringToWString(const std::string& s)
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
std::string WStringToString(const std::wstring& s)
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
std::wstring Utf8StringToWString(const std::string& s)
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
std::string WStringToUtf8String(const std::wstring& s)
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

void Trim(std::wstring& s)
{
  std::wstring::size_type first = s.find_first_not_of(L" \n\t\r\0xb");
  if (first == std::wstring::npos)
    s = L"";
  else
  {
    std::wstring::size_type last = s.find_last_not_of(L" \n\t\r\0xb");
    s = s.substr(first, last - first + 1);
  }
}

} // namespace string_util