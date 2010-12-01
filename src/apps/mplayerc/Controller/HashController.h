#ifndef HASHCONTROLLER_H
#define HASHCONTROLLER_H

#include "LazyInstance.h"
#include <CriticalSection.h>

//////////////////////////////////////////////////////////////////////////
//
//  HashController is a global instance controller that calculates
//  hash for the currently playback video file.
//
class HashController:
  public LazyInstanceImpl<HashController>
{
public:
  HashController(void);

  // Returning current file hash, will automatically calculate if it's not done
  std::wstring GetSPHash(const wchar_t* filename);
  
  std::wstring GetMD5Hash(const wchar_t* filename);
  std::wstring GetMD5Hash(const char* data, int len);

  std::wstring GetVersionHash(const wchar_t* filename);
private:
  CriticalSection m_cs;

  std::wstring m_filename;
  std::wstring m_hash;
};

#endif // HASHCONTROLLER_H