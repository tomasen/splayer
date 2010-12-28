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
  if (!::PathFileExists((::GetModuleFolder() + L"media_data.db").c_str()))
  {
    try
    {
      m_db.open(::GetModuleFolder() + L"media_data.db");

      m_db << L"create table media_data ("
        L"uniqueid integer PRIMARY KEY, path text, filename text,"
        L"thumbnailpath text, videotime integer)";
    }
    catch (sqlitepp::exception* e)
    {
      e;
      Logging(L"MediaModel::MediaModel create database fail");
    }
  }
  else
  {
    try
    {
      m_db.open(::GetModuleFolder() + L"media_data.db");
    }
    catch (sqlitepp::exception* e)
    {
      e;
      Logging(L"MediaModel::MediaModel open database fail");
    }
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

void MediaModel::Add(const MediaData& mdData)
{
  try
  {
    m_db << L"insert into media_data values(null, '"
      << mdData.path << L"', '" << mdData.filename << L"', '"
      << mdData.thumbnailpath << L"', " << mdData.videotime << L")";
  }
  catch (sqlitepp::exception* e)
  {
    e;
    Logging(L"MediaModel::Add fail");
  }
}

void MediaModel::Add(const MediaDatas& data)
{
  MediaDatas::const_iterator it = data.begin();
  while (it != data.end())
  {
    Add(*it);

    ++it;
  }
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
//   for (int i=0;i<limit_end;i++)
//   {
//     wchar_t filename[80], imgpath[80];
//     wsprintf(filename, L"filename_%d", i);
//     wsprintf(imgpath, L"thumbnailpath_%d", i);
//     MediaData rs;
//     rs.filename = filename;
//     rs.thumbnailpath = imgpath;
//     data.push_back(rs);
//   }
//   return;
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