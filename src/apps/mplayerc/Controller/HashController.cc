#include <stdafx.h>
#include "HashController.h"
#include <sphash.h>
#include "../Utils/Strings.h"

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
