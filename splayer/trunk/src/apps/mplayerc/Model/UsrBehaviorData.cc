#include "stdafx.h"
#include "UsrBehaviorData.h"
#include "sqlitepp\sqlitepp\session.hpp"

UsrBehaviorData::~UsrBehaviorData()
{
  std::wstring uid   = L"MYID";
  wchar_t dbname[128];
  swprintf_s(dbname, 128, DATABASE_NAME, uid.c_str(), GetWeekCount());
  struct _stat buf;

  if (_wstat(dbname, &buf) != 0)
  {
    sqlitepp::session db(dbname);
    // cannot find the db file, means today is in a new week
    SetEnvironmentData();
    OutputDebugString(dbname);
    OutputDebugString(L"\n");
    // create a new db file
    db << "create table usrbhv ("
      << "id integer, data text, time double)";
    // begin the thread to upload the behavior data of last week
  }

  sqlitepp::session db(dbname);
  for (std::vector<UsrBehaviorEntry>::iterator it = ubhv_entries.begin();
    it != ubhv_entries.end(); it++)
  {
    db << "insert into usrbhv values(" << (*it).id
      << ", '" << (*it).data.c_str()
      << "', " << (*it).time << ")";
  }
}

void UsrBehaviorData::AppendEntry(int id, std::wstring data)
{
  SYSTEMTIME st;
  ::GetSystemTime(&st);
  double vt;
  ::SystemTimeToVariantTime(&st, &vt);
  UsrBehaviorEntry ube = {id, data, vt};
  ubhv_entries.push_back(ube);
}

void UsrBehaviorData::SetEnvironmentData()
{
  // create a new db file
  // insert into environment_table values (uuid, svnrev, ...);
}

int UsrBehaviorData::GetWeekCount()
{
  struct tm tnow;
  time_t t;
  time(&t);
  localtime_s(&tnow, &t);

  return tnow.tm_yday / 7 + 1;
}