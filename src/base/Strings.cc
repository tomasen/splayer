#include "Strings.h"
#include <Windows.h>

HINSTANCE Strings::m_hInstance = NULL;

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

void Strings::Trim(std::wstring& s)
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
void Strings::SetResourceHandle(HINSTANCE hInstance)
{
  m_hInstance = hInstance;
}
std::wstring Strings::ResourceString(int id)
{

  wchar_t* lpstrText = NULL;
  int nRes = 0;
  for(int nLen = 256; ; nLen *= 2)
  {
    try
    {
      lpstrText = new TCHAR[nLen];
    }
    catch(...) { }

    if(lpstrText == NULL)
      break;
    if (Strings::m_hInstance == NULL)
      Strings::m_hInstance = GetModuleHandle(NULL);

    nRes = ::LoadString(NULL, id, lpstrText, nLen);
    if(nRes < nLen - 1)
      break;
    delete [] lpstrText;
    lpstrText = NULL;
  }
  std::wstring ret_str;
  if(lpstrText != NULL)
  {
    if(nRes != 0)
      ret_str = lpstrText;
    delete [] lpstrText;
  }

  return std::wstring(ret_str);
}

std::wstring Strings::Format(const wchar_t* format, ...)
{
  va_list args;
  va_start(args, format);

  std::wstring formated;
  int ret = -1;
  int bufferlength = wcslen(format)+30;

  while(ret == -1)
  {
    wchar_t* buffer = (wchar_t*)calloc(bufferlength,sizeof(wchar_t));
    const int bufferSize = sizeof(buffer)/sizeof(buffer[0]) - 1;
    ret = _vsnwprintf_s(buffer, bufferlength, _TRUNCATE, format, args);
    if (ret >= 0)
      formated = buffer;
    free(buffer);
    bufferlength += 200;
  }
  va_end(args);
  return formated;
}
