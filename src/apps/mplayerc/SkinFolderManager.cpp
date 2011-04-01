#include "stdafx.h"
#include "SkinFolderManager.h"
#include "UnCompressZip.h"

std::wstring SkinFolderManager::m_path = L"";
std::map<std::wstring, std::wstring> SkinFolderManager::m_skinnametobmp_map;

SkinFolderManager::SkinFolderManager(void):m_count(0),m_bfolderempty(TRUE)
{
}

SkinFolderManager::~SkinFolderManager(void)
{
}

void SkinFolderManager::SeachFile(wchar_t* lpath)
{
  UnCompressZip unzip;
  wchar_t szFind[MAX_PATH];
  WIN32_FIND_DATA FindFileData;
  wcscpy(szFind,lpath);
  wcscat(szFind,L"*.*");

  HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
  if (INVALID_HANDLE_VALUE == hFind)
    return;

  while(TRUE)
  {
    std::wstring skinbmp = L"";
    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (FindFileData.cFileName[0] != '.' && FindFileData.cFileName[0] != '$')
      {
        m_bfolderempty = FALSE;
        ++m_count;
        //m_foldername.push_back(FindFileData.cFileName);
        m_foldername = FindFileData.cFileName;
        CString path(lpath);
        path += FindFileData.cFileName;
        path += L"\\";
        SeachFile(path.GetBuffer(MAX_PATH));
        path.ReleaseBuffer();
      }
    }
    else
    {
      m_bfolderempty = FALSE;
      std::wstring scwstr(FindFileData.cFileName);
      //if (scwstr.Right(scwstr.Find('.')) == L"zip")
      if (scwstr.find(L".zip") != std::wstring::npos)
      {
        //CString folderstr = scwstr.Left(scwstr.Find('.'));
        int pos = scwstr.find(L".");
        std::wstring folderstr = scwstr.substr(0, pos);
        m_foldername = folderstr;
        folderstr = lpath + folderstr;
        scwstr = lpath + scwstr;
        char* scstr;
      
        DWORD  num = WideCharToMultiByte(CP_ACP,0,scwstr.c_str(),-1,NULL,0,NULL,0);
        scstr = (char*)calloc(num + 1,sizeof(char));
        memset(scstr, 0, num*sizeof(char));
        WideCharToMultiByte(CP_ACP,0, scwstr.c_str(), -1, scstr, num,NULL,0);
        scstr[num] = '\0';
      
        unzip.SetUnCompressPath(folderstr.c_str());
        unzip.unzOpen64(scstr);

        ::DeleteFile(scwstr.c_str());
        ++m_count;
        folderstr += L"\\";
        CString path = folderstr.c_str();
        SeachFile(path.GetBuffer(MAX_PATH));
        path.ReleaseBuffer();
      }
      else
      {
        //CString bmp = scwstr.Left(scwstr.Find('.'));
        std::wstring bmp = scwstr.substr(0, scwstr.find(L"."));
        if (bmp == m_foldername)
          skinbmp = lpath + scwstr;
      }
    }

    if (m_foldername != L"" && skinbmp != L"")
      m_skinnametobmp_map[m_foldername] = skinbmp;

    if (!FindNextFile(hFind,&FindFileData))  
      break;
  }
    
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
