#include "StdAfx.h"
#include "MediaModel.h"
#include <logging.h>

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

MediaModel::MediaModel()
{
  // Create the database if it's not exist
  try
  {
    m_db.open(::GetModuleFolder() + L"media.db");

    m_db << L"CREATE TABLE IF NOT EXISTS media_data ("
            L"uniqueid integer PRIMARY KEY, path text, filename text,"
            L"thumbnailpath text, videotime integer)";
    m_db << L"CREATE TABLE IF NOT EXISTS detect_path (" \
            L"uniqueid integer PRIMARY KEY, path text, merit integer)";
  }
  catch (sqlitepp::exception* e)
  {
    e;
    Logging(L"MediaModel::MediaModel open database fail");
  }
}

MediaModel::~MediaModel()
{
}

////////////////////////////////////////////////////////////////////////////////
// Other tasks

int MediaModel::GetCount()
{
  int nCount = 0;

  try
  {
    sqlitepp::statement st(m_db);
    st << L"select count(*) from media_data", sqlitepp::into(nCount);
    st.exec();
  }
  catch (sqlitepp::exception* e)
  {
    e;
    Logging(L"MediaModel::GetCount fail");
  }

  return nCount;
}

void MediaModel::Add(MediaPath& mdData)
{
  // insert unique record
  try
  {
    m_db << L"INSERT INTO detect_path(path, merit)"
      << L" SELECT :path, :merit FROM detect_path"
      << L" WHERE NOT EXISTS(SELECT * FROM detect_path"
      << L" WHERE path='" << mdData.path << L"' and merit=" << mdData.merit << L")"
      , sqlitepp::use(mdData.path), sqlitepp::use(mdData.merit);

    mdData.uniqueid = m_db.last_insert_rowid();
  }
  catch (sqlitepp::exception* e)
  {
    e;
    Logging(L"MediaModel::Add fail");
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
  try
  {
    m_db << L"INSERT INTO media_data(path, filename, thumbnailpath, videotime)"
      << L" SELECT :path, :filename, :thumbnailpath, :videotime FROM media_data"
      << L" WHERE NOT EXISTS(SELECT * FROM media_data"
      << L" WHERE path='" << mdData.path << L"' and filename='" << mdData.filename << L"')"
      , sqlitepp::use(mdData.path), sqlitepp::use(mdData.filename)
      , sqlitepp::use(mdData.thumbnailpath), sqlitepp::use(mdData.videotime);

    mdData.uniqueid = m_db.last_insert_rowid();
  }
  catch (sqlitepp::exception* e)
  {
    e;
    Logging(L"MediaModel::Add fail");
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
  MediaPath mdTemp = {0};

  sqlitepp::statement st(m_db);
  st << L"select uniqueid, path, merit from "
    << L"detect_path"
    , sqlitepp::into(mdTemp.uniqueid)
    , sqlitepp::into(mdTemp.path)
    , sqlitepp::into(mdTemp.merit);

  while (st.exec())
    data.push_back(mdTemp);
}

void MediaModel::FindAll(MediaDatas& data)
{
  MediaData mdTemp = {0};

  sqlitepp::statement st(m_db);
  st << L"select uniqueid, path, filename, thumbnailpath, videotime from "
    << L"media_data"
    , sqlitepp::into(mdTemp.uniqueid)
    , sqlitepp::into(mdTemp.path)
    , sqlitepp::into(mdTemp.filename)
    , sqlitepp::into(mdTemp.thumbnailpath)
    , sqlitepp::into(mdTemp.videotime);

  while (st.exec())
    data.push_back(mdTemp);
}

void MediaModel::FindOne(MediaData& data, const MediaFindCondition& condition)
{
  // If unique id is valid then search the id
  if (condition.uniqueid > 0)
  {
    try
    {
      MediaData mdTemp = {0};

      sqlitepp::statement st(m_db);
      st << L"select uniqueid, path, filename, thumbnailpath, videotime "
        << L"from media_data where uniqueid = " << condition.uniqueid
        , sqlitepp::into(mdTemp.uniqueid)
        , sqlitepp::into(mdTemp.path)
        , sqlitepp::into(mdTemp.filename)
        , sqlitepp::into(mdTemp.thumbnailpath)
        , sqlitepp::into(mdTemp.videotime);

      st.exec();

      data = mdTemp;
    }
    catch (sqlitepp::exception* e)
    {
      e;
      Logging(L"MediaModel::FindOne fail");
    }

    return;
  }

  // Search the file by its filename
  if (!condition.filename.empty())
  {
    try
    {
      MediaData mdTemp = {0};

      sqlitepp::statement st(m_db);
      st << L"select uniqueid, path, filename, thumbnailpath, videotime "
        << L"from media_data where filename = '" << condition.filename << L"'"
        , sqlitepp::into(mdTemp.uniqueid)
        , sqlitepp::into(mdTemp.path)
        , sqlitepp::into(mdTemp.filename)
        , sqlitepp::into(mdTemp.thumbnailpath)
        , sqlitepp::into(mdTemp.videotime);

      st.exec();

      data = mdTemp;
    }
    catch (sqlitepp::exception* e)
    {
      e;
      Logging(L"MediaModel::FindOne fail");
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
    MediaData mdTemp = {0};

    sqlitepp::statement st(m_db);
    st << L"select uniqueid, path, filename, thumbnailpath, videotime from "
      << L"media_data where uniqueid = " << condition.uniqueid
      << L" limit " << limit_start << L"," << limit_end
      , sqlitepp::into(mdTemp.uniqueid)
      , sqlitepp::into(mdTemp.path)
      , sqlitepp::into(mdTemp.filename)
      , sqlitepp::into(mdTemp.thumbnailpath)
      , sqlitepp::into(mdTemp.videotime);

    while (st.exec())
    {
      data.push_back(mdTemp);
    }

    return;
  }

  // Use the filename
  if (!condition.filename.empty())
  {
    MediaData mdTemp = {0};

    sqlitepp::statement st(m_db);
    st << L"select uniqueid, path, filename, thumbnailpath, videotime from "
      << L"media_data where filename = '" << condition.filename << L"'"
      << L" limit " << limit_start << L"," << limit_end
      , sqlitepp::into(mdTemp.uniqueid)
      , sqlitepp::into(mdTemp.path)
      , sqlitepp::into(mdTemp.filename)
      , sqlitepp::into(mdTemp.thumbnailpath)
      , sqlitepp::into(mdTemp.videotime);

    while (st.exec())
    {
      data.push_back(mdTemp);
    }

    return;
  }
}

void MediaModel::Delete(const MediaFindCondition& condition)
{
  // Use the unique id
  if (condition.uniqueid > 0)
  {
    m_db << L"delete from media_data where uniqueid = " << condition.uniqueid;

    return;
  }

  // Use the filename
  if (!condition.filename.empty())
  {
    m_db << L"delete from media_data where filename = '" << condition.filename
      << L"'";

    return;
  }
}

void MediaModel::DeleteAll()
{
  m_db << L"delete from media_data";
}