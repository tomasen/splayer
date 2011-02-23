#include <Windows.h>
#include "ResLoader.h"

#include <Shlwapi.h>
#include <atlimage.h>
#pragma comment(lib, "shlwapi.lib")

////////////////////////////////////////////////////////////////////////////////
// Global helper functions
static std::wstring GetModuleFolder(HMODULE hModuleHandle = 0)
{
  TCHAR szModuleFullPath[MAX_PATH] = {0};
  ::GetModuleFileName(hModuleHandle, szModuleFullPath, MAX_PATH);

  TCHAR szDrive[10] = {0};
  TCHAR szDir[MAX_PATH] = {0};

  ::_wsplitpath(szModuleFullPath, szDrive, szDir, 0, 0);

  std::wstring sResult;
  sResult += szDrive;
  sResult += szDir;

  return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Normal part
ResLoader::ResLoader()
{

}

ResLoader::~ResLoader()
{

}

////////////////////////////////////////////////////////////////////////////////
// Load bitmap
HBITMAP ResLoader::LoadBitmap(const std::wstring& sBitmapPath,
                              const std::wstring& sResModuleName /* = L"" */)
{
  std::wstring sFullPath = GetModuleFolder() + sBitmapPath;

  if (::PathFileExists(sFullPath.c_str()))
  {
    // If the file is exist on disk, then load it
    return LoadBitmapFromDisk(sBitmapPath);
  } 
  else
  {
    // If the file is in exe
    TCHAR szFileName[MAX_PATH] = {0};
    TCHAR szExt[MAX_PATH] = {0};

    ::_wsplitpath(sFullPath.c_str(), 0, 0, szFileName, szExt);
    ::wcscat(szFileName, szExt);

    return LoadBitmapFromModule(szFileName, sResModuleName);
  }

  // Nothing found or load failure, return NULL
  return 0;
}

HBITMAP ResLoader::LoadBitmapFromDisk(const std::wstring& sBitmapPath)
{
  std::wstring sFullPath = GetModuleFolder() + sBitmapPath;

  CImage igImage;
  igImage.Load(sFullPath.c_str());

  HBITMAP hBitmap = (HBITMAP)igImage;

  igImage.Detach();

  return hBitmap;
}

HBITMAP ResLoader::LoadBitmapFromModule(const std::wstring& sBitmapName,
                             const std::wstring& sResModuleName /* = L"" */)
{
  LPCTSTR pcsz = sResModuleName.empty() ? 0 :sResModuleName.c_str();
  HBITMAP hBitmap = 0;
  hBitmap = (HBITMAP)::LoadImage(::GetModuleHandle(pcsz),
          sBitmapName.c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);

  return hBitmap;
}