#include "stdafx.h"
#include "MediaSQLite.h"
#include "..\..\..\svplib\SVPToolBox.h"

MediaSQLite::MediaSQLite()
{
  CSVPToolBox toolbox;
  try
  {
    m_db.open((LPCTSTR)(toolbox.GetPlayerPath(L"media.db")));

    m_db << L"CREATE TABLE IF NOT EXISTS media_data ("
            L"uniqueid integer PRIMARY KEY, path text, filename text,"
            L"thumbnailpath text, videotime integer)";
    m_db << L"CREATE TABLE IF NOT EXISTS detect_path (" \
            L"uniqueid integer PRIMARY KEY, path text, merit integer)";
  }
  catch (std::runtime_error const& err)
  {
    Logging(err.what());
  }
}

MediaSQLite::~MediaSQLite()
{

}

void MediaSQLite::exec(const std::wstring &sql)
{
  m_cs.lock();

  try
  {
    m_db << sql;
  }
  catch (std::runtime_error const& err)
  {
    Logging(err.what());
  }

  m_cs.unlock();
}

long long MediaSQLite::last_insert_rowid()
{
  m_cs.lock();

  long long lRet = m_db.last_insert_rowid();

  m_cs.unlock();

  return lRet;
}