#include "stdafx.h"
#include "SPlayerDefs.h"
#include "PlayerPreference.h"
#include "../../../lib/splite/libsqlite/libsqlite.h"
#include "../Utils/Strings.h"
#include "../resource.h"

#define ResStr(id) CString(MAKEINTRESOURCE(id))

PlayerPreference::PlayerPreference(void):
  sqlite_setting(NULL)
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
  m_map_intvar[INTVAR_LOGO_AUTOSTRETCH]     = GetProfileInt(ResStr(IDS_R_SETTINGS), ResStr(IDS_RS_LOGOSTRETCH), 1);
  m_map_intvar[INTVAR_SHUFFLEPLAYLISTITEMS] = GetProfileInt(ResStr(IDS_R_SETTINGS), L"ShufflePlaylistItems", FALSE);
  m_map_intvar[INTVAR_AUTOLOADAUDIO]        = GetProfileInt(ResStr(IDS_R_SETTINGS), ResStr(IDS_RS_AUTOLOADAUDIO), TRUE);

  m_map_strvar[STRVAR_HOTKEYSCHEME]         = GetProfileString(L"Settings", L"HotkeyScheme", L"");
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
  WriteProfileInt(ResStr(IDS_R_SETTINGS), L"ShufflePlaylistItems", m_map_intvar[INTVAR_SHUFFLEPLAYLISTITEMS]);
  WriteProfileInt(ResStr(IDS_R_SETTINGS), ResStr(IDS_RS_AUTOLOADAUDIO), m_map_intvar[INTVAR_AUTOLOADAUDIO]);

  WriteProfileString(L"Settings", L"HotkeyScheme", m_map_strvar[STRVAR_HOTKEYSCHEME].c_str());

  Uninit();
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

long long PlayerPreference::GetInt64Var(int id)
{
  std::map<int, long long>::iterator it = m_map_int64var.find(id);
  if (it != m_map_int64var.end())
    return it->second;
  return 0;
}

void PlayerPreference::SetInt64Var(int id, long long value_in)
{
  m_map_int64var[id] = value_in;
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
  m_map_intvar[INTVAR_CL_SWITCHES]      = 0;
  m_map_intvar[INTVAR_PLAYLIST_CURRENT] = 0;

  m_map_int64var[INT64VAR_MAINWINDOW]   = 0;

  m_map_strvar[STRVAR_HOTKEYSCHEME]     = L"";

  m_map_strarray[STRARRAY_PLAYLIST] = std::vector<std::wstring>();
  std::map<int, std::vector<std::wstring>>::iterator it = m_map_strarray.find(STRARRAY_PLAYLIST);
  it->second.push_back(L"D:\\1-4.[VeryCD.com].RM");
  it->second.push_back(L"D:\\2-1.[VeryCD.com].RM");
  it->second.push_back(L"D:\\2-2.[VeryCD.com].RM");
  it->second.push_back(L"D:\\2-3.[VeryCD.com].RM");
  it->second.push_back(L"D:\\2-4.[VeryCD.com].RM");
  it->second.push_back(L"D:\\2-5.[VeryCD.com].RM");
  it->second.push_back(L"D:\\2-6.[VeryCD.com].RM");
  it->second.push_back(L"D:\\2-7.[VeryCD.com].RM");

  //
  if (!sqlite_setting)
  {
    wchar_t path[MAX_PATH];
    ::GetModuleFileName(NULL, path, MAX_PATH);
    ::PathRemoveFileSpec(path);

    wcscat_s(path, MAX_PATH, L"\\settings.db");

    sqlite_setting = new SQLITE3(Strings::WStringToUtf8String(std::wstring(path)));

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
  if(sqlite_setting)
    sqlite_setting->exec_sql("PRAGMA synchronous=ON");

  if (sqlite_setting)
    delete sqlite_setting;


}

BOOL PlayerPreference::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
  if (sqlite_setting)
    return sqlite_setting->WriteProfileInt(lpszSection, lpszEntry,  nValue);
  return FALSE;
}

UINT PlayerPreference::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
  if (sqlite_setting)
    return sqlite_setting->GetProfileInt(lpszSection, lpszEntry,  nDefault);
  return FALSE;
}

BOOL PlayerPreference::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
  if (sqlite_setting)
    return sqlite_setting->WriteProfileString(lpszSection, lpszEntry,
      lpszValue);
  return FALSE;
}

CString PlayerPreference::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
  if (sqlite_setting)
    return sqlite_setting->GetProfileString( lpszSection,  lpszEntry,
      lpszDefault);
  return L"";
}
