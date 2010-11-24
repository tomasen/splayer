#include <stdafx.h>
#include "HashController.h"
#include <sphash.h>
#include "../Utils/Strings.h"
#include <io.h> 
#include <fcntl.h>
#include <sys/stat.h>

HashController::HashController(void)
{

}

std::wstring HashController::GetSPHash(const wchar_t* filename)
{
  AutoCSLock lock(m_cs);
  m_filename = filename;
  // return hash if it's set
  // calculate if it's not set
  char str[300];
  int len;

  // double null terminated
  m_filename.push_back(0);
  m_filename.push_back(0);
  hash_file(HASH_MOD_VIDEO_STR, HASH_ALGO_MD5, m_filename.c_str(), str, &len);
  if (len == 0)
    return L"";

  std::string md5str(str);
  m_hash = Strings::StringToWString(md5str);
  return m_hash;
}
std::wstring HashController::GetVersionHash(const wchar_t* filename)
{
  AutoCSLock lock(m_cs);
  DWORD             dwHandle;
  UINT              dwLen;
  UINT              uLen;
  LPVOID            lpBuffer;
  VS_FIXEDFILEINFO  *lpBuffer2;
  DWORD             dwMajor   ;
  DWORD             dwMinor   ;
  DWORD             dwRelease ;
  DWORD             dwBuild   ;

  dwBuild = 0;
  dwLen  = GetFileVersionInfoSize(filename, &dwHandle);

  TCHAR * lpData = (TCHAR*) malloc(dwLen);
  if(!lpData)
    return _T("");
  memset((char*)lpData, 0 , dwLen);

  /* GetFileVersionInfo() requires a char *, but the api doesn't
  * indicate that it will modify it */
  if(GetFileVersionInfo(filename, dwHandle, dwLen, lpData) != 0)
  {
    if(VerQueryValue(lpData, _T("\\"), &lpBuffer, &uLen) != 0)
    {
      lpBuffer2 = (VS_FIXEDFILEINFO *)lpBuffer;
      dwMajor   = HIWORD(lpBuffer2->dwFileVersionMS);
      dwMinor   = LOWORD(lpBuffer2->dwFileVersionMS);
      dwRelease = HIWORD(lpBuffer2->dwFileVersionLS);
      dwBuild   = LOWORD(lpBuffer2->dwFileVersionLS);
    }
  }
  long iFileLen;
  int fp;
  if( _wsopen_s ( &fp, filename, _O_RDONLY, _SH_DENYNO,
    _S_IREAD ) == 0 )
  {

    iFileLen = filelength(fp);
    _close( fp);

  }

  wchar_t hashstr[4096];
  swprintf_s(hashstr, 4096, L"%d.%d.%d.%d.%d", dwMajor, dwMinor, dwRelease, dwBuild,iFileLen);
  return std::wstring(hashstr);
}
std::wstring HashController::GetMD5Hash(const wchar_t* filename)
{
  AutoCSLock lock(m_cs);
  m_filename = filename;
  // return hash if it's set
  // calculate if it's not set
  char str[300];
  int len;
    
  // double null terminated
  m_filename.push_back(0);
  m_filename.push_back(0);
  hash_file(HASH_MOD_FILE_STR, HASH_ALGO_MD5, m_filename.c_str(), str, &len);
  if (len == 0)
    return L"";

  std::string md5str(str);
  m_hash = Strings::StringToWString(md5str);
  return m_hash;
}


std::wstring HashController::GetMD5Hash(const char* data, int len)
{
  AutoCSLock lock(m_cs);

  char *buff = new char[len+1];
  if( buff == NULL )
    return L"";

  strcpy_s(buff, len+1, data);
  hash_data(HASH_MOD_BINARY_STR, HASH_ALGO_MD5, buff, &len);

  m_hash = Strings::StringToWString(buff);
  delete[] buff;
  return m_hash;
}
