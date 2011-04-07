#include "stdafx.h"
#include "MediaSpiderFolderTree.h"
#include "..\Controller\PlayerPreference.h"
#include "..\Controller\SPlayerDefs.h"
#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

////////////////////////////////////////////////////////////////////////////////
// Normal part
MediaSpiderFolderTree::MediaSpiderFolderTree()
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

MediaSpiderFolderTree::~MediaSpiderFolderTree()
{
  // Store the last search path, if the path is NULL, then represent the last
  // search is a complete search
  PlayerPreference::GetInstance()->SetStringVar(STRVAR_LASTSPIDERPATH, m_sLastSearchPath);
}

bool _sort_tree_folders(const MediaTreeFolder &treeFolder1, const MediaTreeFolder &treeFolder2)
{
  return treeFolder1.nMerit > treeFolder2.nMerit;  // descending order
}

//void cDebug(const std::wstring &sDebugInfo, bool bAutoBreak = true)
//{
//  if (::GetStdHandle(STD_OUTPUT_HANDLE) == 0)
//    ::AllocConsole();
//
//  HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
//
//  if (bAutoBreak)
//  {
//    ::WriteConsole(h, sDebugInfo.c_str(), sDebugInfo.size(), 0, 0);
//    ::WriteConsole(h, L"\n", 1, 0, 0);
//  } 
//  else
//  {
//    ::WriteConsole(h, sDebugInfo.c_str(), sDebugInfo.size(), 0, 0);
//  }
//}

void MediaSpiderFolderTree::_Thread()
{
  using namespace boost::lambda;
  using std::wstring;
  using std::vector;
  using std::list;

  while (true)
  {
    // see if need to be stop
    if (_Exit_state(0))
      return;

    // sort the path according the merit by descending order
    MediaTreeFolders treeFolders = m_treeModel.mediaTreeFolders();
    treeFolders.sort(bind(&MediaTreeFolder::nMerit, _1) > bind(&MediaTreeFolder::nMerit, _2));

    //// for test
    //MediaTreeFolders::const_iterator itTest = treeFolders.begin();
    //while (itTest != treeFolders.end())
    //{
    //  std::wstringstream ss;
    //  ss << L"folder = " << itTest->sFolderPath << L", tFolderCreateTime = " << itTest->tFolderCreateTime;
    //  cDebug(ss.str());

    //  ++itTest;
    //}
    //cDebug(L"");

    // search the media files
    MediaTreeFolders::const_iterator it = treeFolders.begin();
    while (it != treeFolders.end())
    {
      // see if need to be stop
      if (_Exit_state(0))
        return;

      // search the path for media files
      if (it->tNextSpiderInterval == 0)
        Search(it->sFolderPath);
      else if (((::time(0) - it->tFolderCreateTime) % it->tNextSpiderInterval) == 0)
        Search(it->sFolderPath);

      ++it;
    }

    // sleep for a moment
    ::Sleep(300);
  }
}

void MediaSpiderFolderTree::Search(const std::wstring &sFolder)
{
  using std::wstring;
  using std::vector;
  using namespace boost::filesystem;

  // see if need to be stop
  if (_Exit_state(0))
    return;  

  // if the folder is not exist or the folder is been exclude, then return
  if (!is_directory(sFolder) || IsExcludePath(sFolder))
    return;

  // search the folder
  wpath folder(sFolder);
  directory_iterator itCur(folder);
  directory_iterator itEnd;

  while (itCur != itEnd)
  {
    if (is_regular_file(itCur->path()) && IsSupportExtension(itCur->path().wstring()))
    {
      // add it to the folder tree
      MediaData md;
      md.path = sFolder;
      md.filename = itCur->path().filename().wstring();
      
      m_treeModel.addFile(md);
    }

    ++itCur;

    // sleep for a moment
    ::Sleep(50);
  }
}