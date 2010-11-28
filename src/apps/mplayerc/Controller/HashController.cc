#include "stdafx.h"
#include "HashController.h"
#include <sphash.h>
#include "../Utils/Strings.h"

HashController::HashController(void)
{

}

//void HashController::SetFileName(const wchar_t* filename)
//{
//  AutoCSLock lock(m_cs);
//  m_hash = L"";
//  m_filename.assign(filename);
//}

std::wstring HashController::GetSPHash(const wchar_t* filename)
{
  AutoCSLock lock(m_cs);
  m_filename = filename;
  // return hash if it's set
  // calculate if it's not set
  if (m_hash.empty())
  {
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
  }
  return m_hash;
}

std::wstring HashController::GetMD5Hash(const wchar_t* filename)
{
  AutoCSLock lock(m_cs);
  m_filename = filename;
  // return hash if it's set
  // calculate if it's not set
  if (m_hash.empty())
  {
    char str[300];
    int len;
    
    // double null terminated
    m_filename.push_back(0);
    m_filename.push_back(0);
    hash_file(HASH_MOD_FILE_BIN, HASH_ALGO_MD5, m_filename.c_str(), str, &len);
    if (len == 0)
      return L"";

    std::string md5str(str);
    m_hash = Strings::StringToWString(md5str);
  }
  return m_hash;
}


std::wstring HashController::GetMD5Hash(const char* data, int len)
{
  AutoCSLock lock(m_cs);
  int buff_size = len+1;
  if (buff_size < 300)
    buff_size = 300;
  char *buff = new char[buff_size];
  if( buff == NULL )
    return L"";

  memcpy_s(buff, buff_size, data, len);
  hash_data(HASH_MOD_BINARY_STR, HASH_ALGO_MD5, buff, &len);

  std::string hex(buff, len);
  m_hash = Strings::StringToWString(hex);
  delete[] buff;
  return m_hash;
}
