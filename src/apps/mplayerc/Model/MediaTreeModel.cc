#include "stdafx.h"
#include "MediaTreeModel.h"
#include <boost/filesystem.hpp>
#include <regex>

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaTreeFolders MediaTreeModel::m_lsFolderTree;

MediaTreeModel::MediaTreeModel()
{
}

MediaTreeModel::~MediaTreeModel()
{
}

////////////////////////////////////////////////////////////////////////////////
// properties
MediaTreeFolders MediaTreeModel::mediaTreeFolders() const
{
  return m_lsFolderTree;
}

////////////////////////////////////////////////////////////////////////////////
// add media info to the tree and save the info to the database
void MediaTreeModel::addFolder(const MediaPath &mp)
{
  using std::wstring;
  using std::vector;

  wstring sPreferredPath = makePathPreferred(mp.path);

  vector<wstring> vtSplitPaths;
  splitPath(sPreferredPath, vtSplitPaths);

  for (int i = 0; i < vtSplitPaths.size(); ++i)
  {
    // if the path is exist in the list, then return
    bool bIsExist = false;
    MediaTreeFolders::iterator it = m_lsFolderTree.begin();
    while (it != m_lsFolderTree.end())
    {
      if (it->sFolderPath == vtSplitPaths[i])
      {
        bIsExist = true;
        break;
      }

      ++it;
    }

    // add path
    if (!bIsExist)
    {
      MediaTreeFolder treefolder;
      treefolder.sFolderPath = vtSplitPaths[i];
      //treefolder.sFolderHash = ;
      treefolder.tFolderCreateTime = ::time(0);

      m_lsFolderTree.push_back(treefolder);
    }
  }
}

void MediaTreeModel::addFile(const MediaData &md)
{
  // first, add the folder to the list if it isn't exist
  std::wstring sPreferredPath = makePathPreferred(md.path);

  MediaPath mpTemp;
  mpTemp.path = sPreferredPath;
  addFolder(mpTemp);

  // second, add the file
  MediaTreeFolders::iterator it = m_lsFolderTree.begin();
  while (it != m_lsFolderTree.end())
  {
    if (it->sFolderPath == sPreferredPath)
    {
      MediaTreeFile treefile;
      treefile.sFilename = md.filename;
      //treefile.sFileHash = ;
      //treefile.sFileUID = ;
      treefile.tFileCreateTime = ::time(0);
      it->lsFiles.push_back(treefile);
      break;
    }

    ++it;
  }
}

void MediaTreeModel::increaseMerit(const std::wstring &sPath)
{
  std::wstring sPreferredPath = makePathPreferred(sPath);

  MediaTreeFolders::iterator it = m_lsFolderTree.begin();
  while (it != m_lsFolderTree.end())
  {
    if (it->sFolderPath == sPreferredPath)
      it->nMerit += 1;

    ++it;
  }
}

void MediaTreeModel::setNextSpiderInterval(const std::wstring &sPath, time_t tInterval)
{
  MediaTreeFolders::iterator it = m_lsFolderTree.begin();

  while (it != m_lsFolderTree.end())
  {
    if (it->sFolderPath == sPath)
      it->tNextSpiderInterval = tInterval;

    ++it;
  }
}

void MediaTreeModel::saveToDB()
{
  // store path to the detect_path
  MediaTreeFolders::const_iterator itTreeFolders = m_lsFolderTree.begin();
  while (itTreeFolders != m_lsFolderTree.end())
  {
    MediaPath mp;
    mp.path = itTreeFolders->sFolderPath;
    mp.merit = itTreeFolders->nMerit;
    
    m_model.Add(mp);
    
    ++itTreeFolders;
  }

  // store file info to the media_data table
  itTreeFolders = m_lsFolderTree.begin();
  MediaTreeFiles::const_iterator itTreeFiles;
  while (itTreeFolders != m_lsFolderTree.end())
  {
    itTreeFiles = itTreeFolders->lsFiles.begin();
    while (itTreeFiles != itTreeFolders->lsFiles.end())
    {
      MediaData md;
      md.filename = itTreeFiles->sFilename;
      md.path = itTreeFolders->sFolderPath;
      // md.thumbnailpath = ;
      // md.videotime = ;

      m_model.Add(md);

      ++itTreeFiles;
    }

    ++itTreeFolders;
  }
}

void MediaTreeModel::splitPath(const std::wstring &sPath, std::vector<std::wstring> &vtResult)
{
  using namespace boost::filesystem;

  // analysis the path and add it to the detect_path table
  std::wstring sTemp(sPath);

  while (!sTemp.empty() && sTemp != L"\\\\")
  {
    // deal with the path
    if (is_directory(sTemp))
      vtResult.push_back(sTemp);

    // remove the back slash to avoid dead loop
    if (sTemp[sTemp.size() - 1] == L'\\')
      sTemp.erase(sTemp.end() - 1, sTemp.end());

    // remove the last suffix
    if (sTemp.find_last_of(L'\\') != std::wstring::npos)
      sTemp.erase(sTemp.begin() + sTemp.find_last_of(L'\\') + 1, sTemp.end());
    else
      sTemp.clear();
  }
}

////////////////////////////////////////////////////////////////////////////////
// helper functions
std::wstring MediaTreeModel::makePathPreferred(const std::wstring &sPath)
{
  using namespace boost::filesystem;
  using namespace std::tr1;
  using std::wstring;

  // ***************************************************************************
  // note:
  // we deal with normal path and UNC path, will erase the filename
  // ***************************************************************************
  // if it's a UNC path, then save the prefix: "\\"
  wstring sPrefix;
  if (regex_search(sPath, wregex(L"^\\\\\\\\")))
    sPrefix = L"\\\\";

  wstring sPathResult;
  if (!sPrefix.empty())
    sPathResult.assign(sPath.begin() + 2, sPath.end());
  else
    sPathResult = sPath;

  // modify the path, let it to be normalized
  // replace all '/' to '\'
  sPathResult = regex_replace(sPathResult, wregex(L"/"), std::wstring(L"\\"));

  // replace multi '\' to single '\'
  sPathResult = regex_replace(sPathResult, wregex(L"\\\\+"), std::wstring(L"\\"));

  // remove the filename in the end
  if (!is_directory(sPathResult))
    sPathResult = regex_replace(sPathResult, wregex(L"\\\\[^\\\\]+$"), std::wstring(L"\\"));

  // append slash to the end if the path
  // after this, the path looks like this: "c:\cjbw1234\test\"
  if (sPathResult[sPathResult.size() - 1] != L'\\')
    sPathResult += L"\\";

  // add the prefix
  sPathResult = sPrefix + sPathResult;

  return sPathResult;
}