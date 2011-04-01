#include "StdAfx.h"
#include "MediaSpiderDisk.h"
#include "..\Controller\PlayerPreference.h"
#include "..\Controller\SPlayerDefs.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part

MediaSpiderDisk::MediaSpiderDisk()
{
  // Init the last search path from the database
  m_sLastSearchPath = PlayerPreference::GetInstance()->GetStringVar(STRVAR_LASTSPIDERPATH);

  // Init the media type and the exclude folders & files from the database
  // Warning: the case is sensitive !!!
  SetSupportExtension(L".avi");
  SetSupportExtension(L".wmv");
  SetSupportExtension(L".mkv");
  SetSupportExtension(L".rmvb");
  SetSupportExtension(L".rm");
  SetSupportExtension(L".asf");
  SetSupportExtension(L".mov");
  SetSupportExtension(L".mp4");
  SetSupportExtension(L".mpeg");
  SetSupportExtension(L".3gp");
  SetSupportExtension(L".divx");

  SetExcludePath(L"c:\\Windows\\");
  SetExcludePath(L"C:\\Program Files\\");
  SetExcludePath(L"C:\\Program Files (x86)\\");
}

MediaSpiderDisk::~MediaSpiderDisk()
{
  // Store the last search path, if the path is NULL, then represent the last
  // search is a complete search
  PlayerPreference::GetInstance()->SetStringVar(STRVAR_LASTSPIDERPATH, m_sLastSearchPath);
}

////////////////////////////////////////////////////////////////////////////////
// Main thread and the helper functions
void MediaSpiderDisk::_Thread()
{
  SearchDisk(L"\\\\FILE_01\\reflections\\-=HDTV=-\\");
  return;

  using std::vector;
  using std::wstring;

  // ---------------------------------------------------------------------------
  // Search the whole disk
  // First find all the drive num
  vector<wstring> vtDrives;

  TCHAR *pszDrives = new TCHAR[512];
  ::memset(pszDrives, 0, 512 * sizeof(TCHAR));

  DWORD dwRet = ::GetLogicalDriveStrings(510, pszDrives);

  TCHAR *pszTemp = pszDrives;
  int nLen = ::wcslen(pszTemp);
  while (nLen > 0)
  {
    TCHAR tszSingleDrive[10] = {0};
    ::wcscpy(tszSingleDrive, pszTemp);

    vtDrives.push_back(wstring(tszSingleDrive));

    pszTemp += nLen + 1;

    nLen = ::wcslen(pszTemp);
  }

  delete [] pszDrives;

  // ---------------------------------------------------------------------------
  // Recursion search the disk
  vector<wstring>::const_iterator it = vtDrives.begin();

  // Adjust the iterator to avoid searching the old drives
  // If the last path is empty, it will do a complete search
  if (!m_sLastSearchPath.empty())
  {
    TCHAR szDrive[10] = {0};
    _wsplitpath(m_sLastSearchPath.c_str(), szDrive, 0, 0, 0);

    std::wstring sDrive = szDrive;
    sDrive += L"\\";

    if (!sDrive.empty())
    {
      while (it != vtDrives.end())
      {
        if (*it > sDrive)    // Only in this situation should break
        {
          break;
        }

        ++it;
      }
    }
  }

  // Start searching the items in each drive
  // First search the last path
  if (!m_sLastSearchPath.empty())
  {
    SearchDisk(m_sLastSearchPath);
    m_sLastSearchPath = L"";
  }

  // Second search the other path
  while (it != vtDrives.end())
  {
    // Search the disk, this function is a recursion function
    SearchDisk(*it);

    // Increment
    ++it;
  }

  // Reset the search path to NULL when normally finish all the jobs
  m_sLastSearchPath = L"";
}

void MediaSpiderDisk::SearchDisk(const std::wstring& sPath)
{
  using std::vector;
  using std::wstring;

  // see if need to be stop
  if (_Exit_state(0))
    return;  

  // Search the disk, this is the really function that do the main job
  WIN32_FIND_DATA fdFindData = {0};
  HANDLE hFindPos = ::FindFirstFile((sPath + L"*").c_str(), &fdFindData);

  if (INVALID_HANDLE_VALUE == hFindPos)
  {
    return;
  }

  do
  {
    // sleep for a moment
    ::Sleep(100);

    // Not search the "." and the ".."
    if ((::wcscmp(fdFindData.cFileName, L".") == 0) ||
      (::wcscmp(fdFindData.cFileName, L"..") == 0))
    {
      continue;
    }

    // Normal search
    wstring sFullPath = sPath + fdFindData.cFileName;

    // Store current full path into the data member
    m_sLastSearchPath = sFullPath;

    if (fdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
    {
      // Is a folder
      sFullPath += L"\\";
      if (!IsExcludePath(sFullPath))
      {
        SearchDisk(sFullPath);
      }
    } 
    else
    {
      // Is a file, judge if it's a media file
      // If it's a media file, then add it into the database
      if (!IsExcludePath(sFullPath))
      {
        if (IsSupportExtension(sFullPath))
        {
          // Add it into the database
          // m_model...
          TCHAR szPath[MAX_PATH] = {0};
          TCHAR szDrive[10] = {0};
          TCHAR szDir[MAX_PATH] = {0};
          
          ::_wsplitpath(sFullPath.c_str(), szDrive, szDir, 0, 0);

          ::wcscat(szPath, szDrive);
          ::wcscat(szPath, szDir);

          MediaData mdData;
          mdData.filename = fdFindData.cFileName;
          mdData.path = szPath;
          // mdData.thumbnailpath = ;
          // mdData.videotime = ;
          m_treeModel.addFile(mdData);
        }
      }
    }
  } while (::FindNextFile(hFindPos, &fdFindData));
}

void MediaSpiderDisk::SetSearchPath(const std::wstring& sPath)
{
  m_sLastSearchPath = sPath;
}