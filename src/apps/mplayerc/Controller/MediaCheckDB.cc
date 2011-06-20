#include "stdafx.h"
#include "MediaCheckDB.h"
#include "..\..\..\base\logging.h"
#include "..\Model\MediaDB.h"
#include <boost/filesystem.hpp>
#include <regex>

using namespace boost::filesystem;
using namespace std::tr1;

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaCheckDB::MediaCheckDB()
{
}

MediaCheckDB::~MediaCheckDB()
{
}

bool MediaCheckDB::ShouldExit()
{
  // see if need to be stop
  if (_Exit_state(0))
    return true;
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// main functions
// deal with the detect_path, make sure it exist
// analysis the media files' path and add some to the detect_path
void MediaCheckDB::_Thread()
{
  while (true)
  {
    // need stop?
    if (ShouldExit())
      return;

    //// analysis the media files' path and add some to the detect_path
    //AddInfoToDetectPath();

    // make sure the detect_path is exist, otherwise delete it
    CheckDetectPath();

    // make sure the media_data's file is exist, otherwise delete it
    CheckMediaData();

    // sleep for a moment
    ::Sleep(3000);
  }
}

void MediaCheckDB::CheckDetectPath()
{
  // -------------------------------------------------------------------------
  // make sure the detect_path is exist, otherwise delete it
  try
  {
    std::vector<std::wstring> vtPath;

    // get the path
    MediaDB<std::wstring>::exec(L"SELECT path FROM detect_path", &vtPath);

    // delete those records which path is null
    MediaDB<>::exec(L"DELETE FROM detect_path WHERE path is null");

    // delete the records which path is not exist in file system
    std::vector<std::wstring>::iterator it = vtPath.begin();
    while (it != vtPath.end())
    {
      // need stop?
      if (ShouldExit())
        return;

      if (!it->empty() && !::PathFileExists(it->c_str()))
      {
        std::wstringstream ss;
        ss << L"DELETE FROM detect_path WHERE path='" << it->c_str() << L"'";
        MediaDB<>::exec(ss.str());
      }

      ++it;

      // sleep for a moment
      ::Sleep(100);
    }
  }
  catch (std::runtime_error const& err)
  {
    Logging(err.what());
  }
}

void MediaCheckDB::CheckMediaData()
{
  // -------------------------------------------------------------------------
  // make sure the media_data's file is exist, otherwise delete it
  try
  {
    std::vector<std::wstring> vtPath;
    std::vector<std::wstring> vtFilename;

    // get the path and filename
    MediaDB<std::wstring, std::wstring>::exec(L"SELECT path, filename FROM media_data", &vtPath, &vtFilename);

    // delete those records which path or filename is null
    MediaDB<>::exec(L"DELETE FROM media_data WHERE path is null or filename is null");

    // delete the records which path and filename is not exist in file system
    std::vector<std::wstring>::iterator itPath = vtPath.begin();
    std::vector<std::wstring>::iterator itFilename = vtFilename.begin();
    while (itPath != vtPath.end())
    {
      // need stop?
      if (ShouldExit())
        return;

      std::wstring sFullPath = *itPath;
      if (sFullPath[sFullPath.size() - 1] != L'\\')
        sFullPath += L"\\";
      sFullPath += *itFilename;

      if (!sFullPath.empty() && !::PathFileExists(sFullPath.c_str()))
      {
        std::wstringstream ss;
        ss << L"DELETE FROM media_data WHERE path='"
          << *itPath << L"' and filename='" << *itFilename << L"'";
        MediaDB<>::exec(ss.str());
      }

      ++itPath;
      ++itFilename;

      // sleep for a moment
      ::Sleep(100);
    }
  }
  catch (std::runtime_error const& err)
  {
    Logging(err.what());
  }
}

//void MediaCheckDB::AddInfoToDetectPath()
//{
//  using std::wstring;
//  using std::wstringstream;
//  using namespace boost::filesystem;
//
//  // -------------------------------------------------------------------------
//  // analysis the media files' path and add some to the detect_path
//  try
//  {
//    std::vector<std::wstring> vtPath;
//
//    // get the path
//    MediaDB<std::wstring>::exec(L"SELECT path FROM media_data", &vtPath);
//
//    // analysis the path
//    std::vector<std::wstring>::iterator it = vtPath.begin();
//    while (it != vtPath.end())
//    {
//      // need stop?
//      if (ShouldExit())
//        return;
//
//      // analysis the path and add it to the detect_path table
//      std::wstring sTemp(*it);
//
//      while (!sTemp.empty() && sTemp != L"\\\\")
//      {
//        // deal with the path
//        if (is_directory(sTemp))
//        {
//          MediaPath mp;
//          mp.path = sTemp;
//          m_treeModel.addFolder(mp);
//        }
//
//        // remove the back slash to avoid dead loop
//        if (sTemp[sTemp.size() - 1] == L'\\')
//          sTemp.erase(sTemp.end() - 1, sTemp.end());
//
//        // remove the last suffix
//        if (sTemp.find_last_of(L'\\') != std::wstring::npos)
//          sTemp.erase(sTemp.begin() + sTemp.find_last_of(L'\\') + 1, sTemp.end());
//        else
//          sTemp.clear();
//      }
//
//      ++it;
//
//      // sleep for a moment
//      ::Sleep(30);
//    }
//  }
//  catch (std::runtime_error const& err)
//  {
//    Logging(err.what());
//  }
//}