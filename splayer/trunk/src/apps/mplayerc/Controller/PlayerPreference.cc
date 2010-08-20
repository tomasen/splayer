#include "stdafx.h"
#include "SPlayerDefs.h"
#include "PlayerPreference.h"
#include "../resource.h"

#define ResStr(id) CString(MAKEINTRESOURCE(id))

PlayerPreference::PlayerPreference(void):
  sqlite_setting(NULL),
  sqlite_local_record(NULL)
{
  // init
  Init();

  // read from registry

  //////////////////////////////////////////////////////////////////////////
  // WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! 
  //
  // The following setting retrieval is only temporary, it's brought
  // directly from mplayerc.cpp / mplayerc.h .
  // 
  // WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! 
  m_map_intvar[INTVAR_LOGO_AUTOSTRETCH] = GetProfileInt(ResStr(IDS_R_SETTINGS), ResStr(IDS_RS_LOGOSTRETCH), 1);
}

PlayerPreference::~PlayerPreference(void)
{
  // write into registry
  //////////////////////////////////////////////////////////////////////////
  // WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! 
  //
  // The following setting retrieval is only temporary, it's brought
  // directly from mplayerc.cpp / mplayerc.h .
  // 
  // WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! 
  WriteProfileInt(ResStr(IDS_R_SETTINGS), ResStr(IDS_RS_LOGOSTRETCH), m_map_intvar[INTVAR_LOGO_AUTOSTRETCH]);
}

int PlayerPreference::GetIntVar(int id)
{
  std::map<int, int>::iterator it = m_map_intvar.find(id);
  if (it != m_map_intvar.end())
    return it->second;
  return 0;
}

void PlayerPreference::SetIntVar(int id, int value_in)
{
  m_map_intvar[id] = value_in;
}

std::wstring PlayerPreference::GetStringVar(int id)
{
  std::map<int, std::wstring>::iterator it = m_map_strvar.find(id);
  if(it != m_map_strvar.end())
    return it->second;
  return L"";
}

void PlayerPreference::SetStringVar(int id, std::wstring &value_in)
{
  m_map_strvar[id] = value_in;
}

std::vector<int> PlayerPreference::GetIntArray(int id)
{
  std::map<int, std::vector<int>>::iterator it = m_map_intarray.find(id);
  if(it != m_map_intarray.end())
    return it->second;
  return std::vector<int>();
}

void PlayerPreference::SetIntArray(int id, std::vector<int> &value_in)
{
  m_map_intarray[id] = value_in;
}

std::vector<std::wstring> PlayerPreference::GetStrArray(int id)
{
  std::map<int, std::vector<std::wstring>>::iterator it = m_map_strarray.find(id);
  if(it != m_map_strarray.end())
    return it->second;
  return std::vector<std::wstring>();
}

void PlayerPreference::SetStrArray(int id, std::vector<std::wstring> &value_in)
{
  m_map_strarray[id] = value_in;
}

void PlayerPreference::Init()
{
  // default value logic
  m_map_intvar[INTVAR_CL_SWITCHES]  = 0;

  if (!sqlite_setting)
  {
    wchar_t path[256];
    char path_utf8[512];
    ::GetModuleFileName(NULL, path, 256);
    ::PathRemoveFileSpec(path);

    wcscat_s(path, 256, L"\\settings.db");
    ::WideCharToMultiByte(CP_UTF8, 0, path, lstrlen(path), path_utf8, 512, NULL, NULL);

    sqlite_setting = new SQLITE3(path_utf8);

    if (!sqlite_setting->db_open)
    {
      delete sqlite_setting;
      sqlite_setting = NULL;
    }
  }

	if (sqlite_setting)
  {
		sqlite_setting->exec_sql("CREATE TABLE IF NOT EXISTS \"settingint\" ( \"hkey\" TEXT,  \"sect\" TEXT,  \"sval\" INTEGER)");
		sqlite_setting->exec_sql("CREATE TABLE IF NOT EXISTS \"settingstring\" (  \"hkey\" TEXT,   \"sect\" TEXT,   \"vstring\" TEXT)");
		sqlite_setting->exec_sql("CREATE TABLE IF NOT EXISTS \"settingbin2\" (   \"skey\" TEXT,   \"sect\" TEXT,   \"vdata\" BLOB)");
		sqlite_setting->exec_sql("CREATE UNIQUE INDEX IF NOT EXISTS \"pkey\" on settingint (hkey ASC, sect ASC)");
		sqlite_setting->exec_sql("CREATE UNIQUE INDEX IF NOT EXISTS \"pkeystring\" on settingstring (hkey ASC, sect ASC)");
		sqlite_setting->exec_sql("CREATE UNIQUE INDEX IF NOT EXISTS \"pkeybin\" on settingbin2 (skey ASC, sect ASC)");
		sqlite_setting->exec_sql("PRAGMA synchronous=OFF");
		sqlite_setting->exec_sql("DROP TABLE  IF EXISTS  \"settingbin\"");
  }
}

void PlayerPreference::Uninit()
{
  if (sqlite_setting)
    sqlite_setting->exec_sql("PRAGMA synchronous=ON");

  if (sqlite_local_record)
  {
    wchar_t sql[256];
    swprintf_s(sql, 256, L"DELETE FROM histories WHERE modtime < '%d' ", time(NULL)-3600*24*30);
    sqlite_local_record->exec_sql_u(sql);
    sqlite_local_record->exec_sql("PRAGMA synchronous=ON");
  }
}

BOOL PlayerPreference::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
  if (sqlite_setting)
    return sqlite_setting->WriteProfileInt(lpszSection,  lpszEntry,  nValue);
  return FALSE;
}

UINT PlayerPreference::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
  if (sqlite_setting)
    return sqlite_setting->GetProfileInt(lpszSection,  lpszEntry,  nDefault);
  return FALSE;
}