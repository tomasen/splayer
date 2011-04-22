#include <Windows.h>
#include "ResLoader.h"

#include <Shlwapi.h>
#include <atlimage.h>
#pragma comment(lib, "shlwapi.lib")

HINSTANCE ResLoader::hResourceHandle = NULL;
HINSTANCE ResLoader::hMainInstance = NULL;

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
HBITMAP ResLoader::LoadBitmap(const std::wstring& sBitmapPath)
{
  std::wstring sFullPath = GetModuleFolder() + sBitmapPath;

  if (::PathFileExists(sFullPath.c_str()))
  {
    // If the file is exist on disk, then load it
    return LoadBitmapFromDisk(sBitmapPath);
  } 
  else
    return LoadBitmapFromModule(sBitmapPath);

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

HBITMAP ResLoader::LoadBitmapFromModule(const std::wstring& sBitmapName)
{
  HINSTANCE hInstance = hResourceHandle;
  HBITMAP hBitmap = NULL;
  if (hInstance)
    hBitmap = (HBITMAP)::LoadImage(hInstance, sBitmapName.c_str(), IMAGE_BITMAP, 0, 0, 
                                   LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
  if (!hBitmap)
  {
    hInstance = hMainInstance;
    if (!hInstance)
      hMainInstance = GetModuleHandle(NULL);
    hBitmap = (HBITMAP)::LoadImage(hInstance, sBitmapName.c_str(), IMAGE_BITMAP, 0, 0, 
                                   LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
  }
  return hBitmap;
}