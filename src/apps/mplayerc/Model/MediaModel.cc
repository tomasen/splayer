#include "StdAfx.h"
#include "MediaModel.h"
#include <logging.h>
#include "MediaSQLite.h"

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

  MediaSQLite::GetInstance()->exec(L"SELECT count(*) FROM media_data", nCount);

  return nCount;
}

void MediaModel::Add(MediaPath& mdData)
{
  // insert unique record
  int nRecordCount = 0;
  std::wstringstream ss;

  ss << L"SELECT count(*) FROM detect_path WHERE path='"
     << mdData.path << L"' and merit=" << mdData.merit;
  MediaSQLite::GetInstance()->exec(ss.str(), nRecordCount);

  if (nRecordCount == 0)
  {
    ss.str(L"");
    ss << L"INSERT INTO detect_path(path, merit)"
       << L" VALUES('" << mdData.path << L"', " << mdData.merit << L")";

    MediaSQLite::GetInstance()->exec(ss.str());

    mdData.uniqueid = MediaSQLite::GetInstance()->last_insert_rowid();
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
  // insert unique record
  int nRecordCount = 0;
  std::wstringstream ss;

  ss << L"SELECT count(*) FROM media_data WHERE path='"
    << mdData.path << L"' and filename='" << mdData.filename << L"'";
  MediaSQLite::GetInstance()->exec(ss.str(), nRecordCount);

  if (nRecordCount == 0)
  {
    ss.str(L"");
    ss << L"INSERT INTO media_data(path, filename, thumbnailpath, videotime)"
       << L" VALUES('" << mdData.path << L"', '" << mdData.filename << L"', '"
       << mdData.thumbnailpath << L"', " << mdData.videotime << L")";

    MediaSQLite::GetInstance()->exec(ss.str());

    mdData.uniqueid = MediaSQLite::GetInstance()->last_insert_rowid();
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

  MediaSQLite::GetInstance()->exec(L"SELECT uniqueid FROM detect_path", vtUniqueID);
  MediaSQLite::GetInstance()->exec(L"SELECT path FROM detect_path", vtPath);
  MediaSQLite::GetInstance()->exec(L"SELECT merit FROM detect_path", vtMerit);

  for (size_t i = 0; i < vtUniqueID.size(); ++i)
  {
    MediaPath mp = {0};
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
  std::vector<std::wstring> vtFilename;
  std::vector<std::wstring > vtThumbnailPath;
  std::vector<int> vtVideoTime;

  MediaSQLite::GetInstance()->exec(L"SELECT uniqueid FROM media_data", vtUniqueID);
  MediaSQLite::GetInstance()->exec(L"SELECT path FROM media_data", vtPath);
  MediaSQLite::GetInstance()->exec(L"SELECT filename FROM media_data", vtFilename);
  MediaSQLite::GetInstance()->exec(L"SELECT thumbnailpath FROM media_data", vtThumbnailPath);
  MediaSQLite::GetInstance()->exec(L"SELECT videotime FROM media_data", vtVideoTime);

  for (size_t i = 0; i < vtUniqueID.size(); ++i)
  {
    MediaData md = {0};
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
      long long nUniqueID;
      std::wstring sPath;
      std::wstring sFilename;
      std::wstring sThumbnailPath;
      int nVideoTime;

      std::wstringstream ss;
      ss << L" WHERE uniqueid=" << condition.uniqueid;
      MediaSQLite::GetInstance()->exec(L"SELECT uniqueid FROM media_data" + ss.str(), nUniqueID);
      MediaSQLite::GetInstance()->exec(L"SELECT path FROM media_data" + ss.str(), sPath);
      MediaSQLite::GetInstance()->exec(L"SELECT filename FROM media_data" + ss.str(), sFilename);
      MediaSQLite::GetInstance()->exec(L"SELECT thumbnailpath FROM media_data" + ss.str(), sThumbnailPath);
      MediaSQLite::GetInstance()->exec(L"SELECT videotime FROM media_data" + ss.str(), nVideoTime);

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
      long long nUniqueID;
      std::wstring sPath;
      std::wstring sFilename;
      std::wstring sThumbnailPath;
      int nVideoTime;

      std::wstringstream ss;
      ss << L" WHERE filename='" << condition.filename << L"'";
      MediaSQLite::GetInstance()->exec(L"SELECT uniqueid FROM media_data" + ss.str(), nUniqueID);
      MediaSQLite::GetInstance()->exec(L"SELECT path FROM media_data" + ss.str(), sPath);
      MediaSQLite::GetInstance()->exec(L"SELECT filename FROM media_data" + ss.str(), sFilename);
      MediaSQLite::GetInstance()->exec(L"SELECT thumbnailpath FROM media_data" + ss.str(), sThumbnailPath);
      MediaSQLite::GetInstance()->exec(L"SELECT videotime FROM media_data" + ss.str(), nVideoTime);

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
    std::vector<std::wstring> vtFilename;
    std::vector<std::wstring > vtThumbnailPath;
    std::vector<int> vtVideoTime;

    std::wstringstream ss;
    ss << L" WHERE uniqueid=" << condition.uniqueid
       << L" limit " << limit_start << L"," << limit_end;
    MediaSQLite::GetInstance()->exec(L"SELECT uniqueid FROM media_data" + ss.str(), vtUniqueID);
    MediaSQLite::GetInstance()->exec(L"SELECT path FROM media_data" + ss.str(), vtPath);
    MediaSQLite::GetInstance()->exec(L"SELECT filename FROM media_data" + ss.str(), vtFilename);
    MediaSQLite::GetInstance()->exec(L"SELECT thumbnailpath FROM media_data" + ss.str(), vtThumbnailPath);
    MediaSQLite::GetInstance()->exec(L"SELECT videotime FROM media_data" + ss.str(), vtVideoTime);

    for (size_t i = 0; i < vtUniqueID.size(); ++i)
    {
      MediaData md = {0};
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
    MediaSQLite::GetInstance()->exec(L"SELECT uniqueid FROM media_data" + ss.str(), vtUniqueID);
    MediaSQLite::GetInstance()->exec(L"SELECT path FROM media_data" + ss.str(), vtPath);
    MediaSQLite::GetInstance()->exec(L"SELECT filename FROM media_data" + ss.str(), vtFilename);
    MediaSQLite::GetInstance()->exec(L"SELECT thumbnailPath FROM media_data" + ss.str(), vtThumbnailPath);
    MediaSQLite::GetInstance()->exec(L"SELECT videotime FROM media_data" + ss.str(), vtVideoTime);

    for (size_t i = 0; i < vtUniqueID.size(); ++i)
    {
      MediaData md = {0};
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
    MediaSQLite::GetInstance()->exec(ss.str());

    return;
  }

  // Use the filename
  if (!condition.filename.empty())
  {
    std::wstringstream ss;
    ss << L"delete from media_data where filename = '" << condition.filename << L"'";
    MediaSQLite::GetInstance()->exec(ss.str());

    return;
  }
}

void MediaModel::DeleteAll()
{
  std::wstringstream ss;
  ss << L"delete from media_data";
  MediaSQLite::GetInstance()->exec(ss.str());
}