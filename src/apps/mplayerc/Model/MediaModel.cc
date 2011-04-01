#include "StdAfx.h"
#include "MediaModel.h"
#include <logging.h>
#include "MediaDB.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part

MediaModel::MediaModel()
{
}

MediaModel::~MediaModel()
{
}

////////////////////////////////////////////////////////////////////////////////
// Other tasks

int MediaModel::GetCount()
{
  int nCount = 0;

  MediaDB<int>::exec(L"SELECT count(*) FROM media_data", &nCount);

  return nCount;
}

void MediaModel::Add(MediaPath& mdData)
{
  // insert unique record
  int nRecordCount = 0;
  std::wstringstream ss;

  ss << L"SELECT count(*) FROM detect_path WHERE path='"
     << mdData.path << L"' and merit=" << mdData.merit;
  MediaDB<int>::exec(ss.str(), &nRecordCount);

  if (nRecordCount == 0)
  {
    ss.str(L"");
    ss << L"INSERT INTO detect_path(path, merit)"
       << L" VALUES('" << mdData.path << L"', " << mdData.merit << L")";

    MediaDB<>::exec(ss.str());
    MediaDB<>::last_insert_rowid(mdData.uniqueid);
  }
}

void MediaModel::Add(MediaPaths& data)
{
  MediaPaths::iterator it = data.begin();
  while (it != data.end())
  {
    Add(*it);
    ++it;
  }
}

void MediaModel::Add(MediaData& mdData)
{
  // add backslash
  if (mdData.path[mdData.path.size() - 1] != L'\\')
    mdData.path += L"\\";

  // insert unique record
  int nRecordCount = 0;
  std::wstringstream ss;

  ss << L"SELECT count(*) FROM media_data WHERE path='"
    << mdData.path << L"' and filename='" << mdData.filename << L"'";
  MediaDB<int>::exec(ss.str(), &nRecordCount);

  if (nRecordCount == 0)
  {
    ss.str(L"");
    ss << L"INSERT INTO media_data(path, filename, thumbnailpath, videotime)"
       << L" VALUES('" << mdData.path << L"', '" << mdData.filename << L"', '"
       << mdData.thumbnailpath << L"', " << mdData.videotime << L")";

    MediaDB<>::exec(ss.str());
    MediaDB<>::last_insert_rowid(mdData.uniqueid);
  }
}

void MediaModel::Add(MediaDatas& data)
{
  MediaDatas::iterator it = data.begin();
  while (it != data.end())
  {
    Add(*it);
    ++it;
  }
}

void MediaModel::FindAll(MediaPaths& data)
{
  std::vector<long long> vtUniqueID;
  std::vector<std::wstring > vtPath;
  std::vector<int> vtMerit;

  typedef MediaDB<long long, std::wstring, int> tpdMediaDBDB;
  tpdMediaDBDB::exec(L"SELECT uniqueid, path, merit FROM detect_path",
                      &vtUniqueID,  &vtPath,  &vtMerit);

  for (size_t i = 0; i < vtUniqueID.size(); ++i)
  {
    MediaPath mp;
    mp.uniqueid = vtUniqueID[i];
    mp.path = vtPath[i];
    mp.merit = vtMerit[i];

    data.push_back(mp);
  }
}

void MediaModel::FindAll(MediaDatas& data)
{
  std::vector<long long> vtUniqueID;
  std::vector<std::wstring > vtPath;
  std::vector<std::wstring > vtFilename;
  std::vector<std::wstring > vtThumbnailPath;
  std::vector<int> vtVideoTime;

  typedef MediaDB<long long, std::wstring, std::wstring, std::wstring, int> tpdMediaDBDB;
  tpdMediaDBDB::exec(L"SELECT uniqueid, path, filename, thumbnailpath, videotime FROM media_data",
                       &vtUniqueID,  &vtPath,  &vtFilename,  &vtThumbnailPath,  &vtVideoTime);

  for (size_t i = 0; i < vtUniqueID.size(); ++i)
  {
    MediaData md;
    md.uniqueid = vtUniqueID[i];
    md.path = vtPath[i];
    md.filename = vtFilename[i];
    md.thumbnailpath = vtThumbnailPath[i];
    md.videotime = vtVideoTime[i];

    data.push_back(md);
  }
}

void MediaModel::FindOne(MediaData& data, const MediaFindCondition& condition)
{
  // If unique id is valid then search the id
  if (condition.uniqueid > 0)
  {
    try
    {
      long long nUniqueID = 0;
      std::wstring sPath;
      std::wstring sFilename;
      std::wstring sThumbnailPath;
      int nVideoTime = 0;

      std::wstringstream ss;
      ss << L" WHERE uniqueid=" << condition.uniqueid;

      typedef MediaDB<long long, std::wstring, std::wstring, std::wstring, int> tpdMediaDBDB;
      tpdMediaDBDB::exec(L"SELECT uniqueid, path, filename, thumbnailpath, videotime FROM media_data" + ss.str()
                        , &nUniqueID, &sPath, &sFilename, &sThumbnailPath, &nVideoTime);

      data.uniqueid = nUniqueID;
      data.path = sPath;
      data.filename = sFilename;
      data.thumbnailpath = sThumbnailPath;
      data.videotime = nVideoTime;
    }
    catch (std::runtime_error const& err)
    {
      Logging(err.what());
    }

    return;
  }

  // Search the file by its filename
  if (!condition.filename.empty())
  {
    try
    {
      long long nUniqueID = 0;
      std::wstring sPath;
      std::wstring sFilename;
      std::wstring sThumbnailPath;
      int nVideoTime = 0;

      std::wstringstream ss;
      ss << L" WHERE filename='" << condition.filename << L"'";

      typedef MediaDB<long long, std::wstring, std::wstring, std::wstring, int> tpdMediaDBDB;
      tpdMediaDBDB::exec(L"SELECT uniqueid, path, filename, thumbnailpath, videotime FROM media_data" + ss.str()
                        , &nUniqueID, &sPath, &sFilename, &sThumbnailPath, &nVideoTime);

      data.uniqueid = nUniqueID;
      data.path = sPath;
      data.filename = sFilename;
      data.thumbnailpath = sThumbnailPath;
      data.videotime = nVideoTime;
    }
    catch (std::runtime_error const& err)
    {
      Logging(err.what());
    }

    return;
  }
}

// limit_start represent the start number, limit_end represent the nums
void MediaModel::Find(MediaDatas& data, const MediaFindCondition& condition,
          int limit_start, int limit_end)
{
  // Use the unique id
  if (condition.uniqueid > 0)
  {
    std::vector<long long> vtUniqueID;
    std::vector<std::wstring > vtPath;
    std::vector<std::wstring > vtFilename;
    std::vector<std::wstring > vtThumbnailPath;
    std::vector<int> vtVideoTime;

    std::wstringstream ss;
    ss << L" WHERE uniqueid=" << condition.uniqueid
       << L" limit " << limit_start << L"," << limit_end;

    typedef MediaDB<long long, std::wstring, std::wstring, std::wstring, int> tpdMediaDBDB;
    tpdMediaDBDB::exec(L"SELECT uniqueid, path, filename, thumbnailpath, videotime FROM media_data" + ss.str()
                      , &vtUniqueID, &vtPath, &vtFilename, &vtThumbnailPath, &vtVideoTime);

    for (size_t i = 0; i < vtUniqueID.size(); ++i)
    {
      MediaData md;
      md.uniqueid = vtUniqueID[i];
      md.path = vtPath[i];
      md.filename = vtFilename[i];
      md.thumbnailpath = vtThumbnailPath[i];
      md.videotime = vtVideoTime[i];

      data.push_back(md);
    }

    return;
  }

  // Use the filename
  if (!condition.filename.empty())
  {
    std::vector<long long> vtUniqueID;
    std::vector<std::wstring > vtPath;
    std::vector<std::wstring> vtFilename;
    std::vector<std::wstring > vtThumbnailPath;
    std::vector<int> vtVideoTime;

    std::wstringstream ss;
    ss << L" WHERE filename='" << condition.filename << L"'"
      << L" limit " << limit_start << L"," << limit_end;

    typedef MediaDB<long long, std::wstring, std::wstring, std::wstring, int> tpdMediaDBDB;
    tpdMediaDBDB::exec(L"SELECT uniqueid, path, filename, thumbnailpath, videotime FROM media_data" + ss.str()
                      , &vtUniqueID, &vtPath, &vtFilename, &vtThumbnailPath, &vtVideoTime);

    for (size_t i = 0; i < vtUniqueID.size(); ++i)
    {
      MediaData md;
      md.uniqueid = vtUniqueID[i];
      md.path = vtPath[i];
      md.filename = vtFilename[i];
      md.thumbnailpath = vtThumbnailPath[i];
      md.videotime = vtVideoTime[i];

      data.push_back(md);
    }

    return;
  }
}

void MediaModel::Delete(const MediaFindCondition& condition)
{
  // Use the unique id
  if (condition.uniqueid > 0)
  {
    std::wstringstream ss;
    ss << L"delete from media_data where uniqueid = " << condition.uniqueid;
    MediaDB<>::exec(ss.str());

    return;
  }

  // Use the filename
  if (!condition.filename.empty())
  {
    std::wstringstream ss;
    ss << L"delete from media_data where filename = '" << condition.filename << L"'";
    MediaDB<>::exec(ss.str());

    return;
  }
}

void MediaModel::DeleteAll()
{
  std::wstringstream ss;
  ss << L"delete from media_data";
  MediaDB<>::exec(ss.str());
}