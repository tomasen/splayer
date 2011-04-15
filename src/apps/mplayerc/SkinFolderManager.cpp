#include "stdafx.h"
#include "SkinFolderManager.h"
#include "UnCompressZip.h"
#include "Strings.h"
#include "logging.h"

std::wstring SkinFolderManager::m_path = L"";
std::map<std::wstring, std::wstring> SkinFolderManager::m_skinnametobmp_map;

SkinFolderManager::SkinFolderManager(void):m_count(0),m_bfolderempty(TRUE)
{
}

SkinFolderManager::~SkinFolderManager(void)
{
}

std::wstring SkinFolderManager::UnSkinzip(std::wstring path)
{
  UnCompressZip zip;

  std::transform(path.begin(), path.end(), path.begin(), tolower);
  std::wstring dir = path.substr(0, path.find(L".zip"));
  dir += L"\\";

  zip.SetUnCompressPath(dir.c_str());
  zip.unzOpen64(Strings::WStringToString(path).c_str());
  ::DeleteFile(path.c_str());

  return dir;
}

void SkinFolderManager::SeachFile(const wchar_t* lpath)
{
  WIN32_FIND_DATA FindFileData;
  std::wstring searchpath = lpath;
  searchpath += L"*.*";

  HANDLE hFind=::FindFirstFile(searchpath.c_str(), &FindFileData);
  if (INVALID_HANDLE_VALUE == hFind)
    return;

  do
  {
    std::wstring filename = FindFileData.cFileName;
    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (filename == L"." || filename == L"..")
        continue;
      std::wstring dir = lpath;
      dir += filename;
      dir += L"\\";
      // Only support single-level directory
      m_foldername = filename;
      SeachFile(dir.c_str());
    }
    else if ((std::transform(filename.begin(), filename.end(), filename.begin(), tolower)
            , filename.find(L".zip")) != std::wstring::npos)
    {
      std::wstring dir = lpath;
      dir += filename;

      m_foldername = filename.substr(0, filename.find(L".zip"));
      SeachFile(UnSkinzip(dir).c_str());
    }
    else if (!filename.empty())
    {
      // load resource
      std::wstring res = filename.substr(0, filename.find(L"."));
      if (m_foldername == res)
      {
        res = lpath + filename;
        m_skinnametobmp_map[m_foldername] = res;
      }
    }
  }
  while(FindNextFile(hFind, &FindFileData));
  FindClose(hFind);
}

int  SkinFolderManager::GetFileCount()
{
  return m_count;
}

BOOL SkinFolderManager::IsFolderEmpty()
{
  return m_bfolderempty;
}

std::map<std::wstring, std::wstring>& SkinFolderManager::ReturnSkinMap()
{
  return m_skinnametobmp_map;
}

void SkinFolderManager::SetSkinPath(CString lpFoldername)
{
  m_path = lpFoldername;
}

void SkinFolderManager::DeleteFolder(LPCTSTR lpFolderName)
{
  std::wstring s = m_path;
  s += lpFolderName;
  s += L"\\";
  DeleteFile(s.c_str());
  RemoveDirectory(s.c_str());
}

void SkinFolderManager::DeleteFile(LPCTSTR lpath)
{
  wchar_t szFind[MAX_PATH];
  WIN32_FIND_DATA FindFileData;
  wcscpy(szFind,lpath);
  wcscat(szFind,L"*.*");

  HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
  if (INVALID_HANDLE_VALUE == hFind)
    return;

  while(TRUE)
  {
   if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (FindFileData.cFileName[0] != '.' && FindFileData.cFileName[0] != '$')
      {
        CString path(lpath);
        path += FindFileData.cFileName;
        path += L"\\";
        SeachFile(path.GetBuffer());
        RemoveDirectory(path);
      }
    }
    else
    {
      CString file(lpath);
      file += FindFileData.cFileName;
      ::DeleteFile(file);
    }

   if (!FindNextFile(hFind,&FindFileData))  
      break;
  }

  FindClose(hFind);
}

void SkinFolderManager::ClearMap()
{
  m_skinnametobmp_map.clear();
}
