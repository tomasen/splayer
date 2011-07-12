#include "stdafx.h"
#include "SPlayerDefs.h"
#include "PlayerPreference.h"
#include "../Model/appSQLlite.h"
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
  m_map_intvar[INTVAR_LEFTCLICK2PAUSE]  = GetProfileInt(ResStr(IDS_R_SETTINGS), L"LeftClickToPause", FALSE);
  m_map_intvar[INTVAR_TOGGLEFULLSCRENWHENPLAYBACKSTARTED]  = GetProfileInt(ResStr(IDS_R_SETTINGS), L"ToggleFullScreenWhenPlaybackStarted", FALSE);
  m_map_intvar[INTVAR_MAP_CENTERCH2LR]                  = GetProfileInt(ResStr(IDS_R_SETTINGS), L"MapCenterChToLR", TRUE);
  m_map_intvar[INTVAR_CHECKFILEASSOCONSTARTUP]             = GetProfileInt(ResStr(IDS_R_SETTINGS), L"CheckFileAssocOnStartUp", 0);
  m_map_intvar[INTVAR_PLAYAD]               = GetProfileInt(ResStr(IDS_R_SETTINGS), L"DisableAds", 0);

  m_map_strvar[STRVAR_HOTKEYSCHEME]         = GetProfileString(L"Settings", L"HotkeyScheme", L"");
  m_map_strvar[STRVAR_GETSNAPTIMEURL]       = L"http://webpj:8080/test_snapshot.php";
  m_map_strvar[STRVAR_GETSNAPTIMEURL_ACT]   = L"?action=get_snapshot_preferedtimes&videohash=%s&videolength=%d";
  m_map_strvar[STRVAR_UPLOADUSRBHVURL]      = L"http://webpj:8080/test_logfile.php";
  m_map_strvar[STRVAR_UPLOADUSRBHVURL_ACT]  = L"?action=ensureupload";
  m_map_strvar[STRVAR_APIURL]               = L"https://www.shooter.cn/api/v2";

  m_map_strvar[STRVAR_SUBTITLE_SAVEMETHOD]  = GetProfileString(L"Settings", L"SubtitleSaveMethod", L"");  // subtitle save method
  m_map_strvar[STRVAR_SUBTITLE_SAVE_CUSTOMPATH] = GetProfileString(L"Settings", L"SubtitleSaveFolder", L"");  // subtitle save folder

  m_map_strvar[STRVAR_AD] = GetProfileString(L"Settings", L"Ad", L"");   // ad
  m_map_strvar[STRVAR_HIDEAD] = GetProfileString(L"Settings", L"HideAd", L""); // hide ads
  m_map_strvar[STRVAR_TIMEBMP_TYPE] = GetProfileString(L"Settings", L"TimeBmpType", L"Display_TimeLeft");

  m_map_strvar[STRVAR_USER_ACCOUNT_NAME] = GetProfileString(L"Settings", L"UserAccountName", L"false");  // default value is 'false'

  m_map_strvar[STRVAR_MAINSUBTITLEFONT] = GetProfileString(L"Settings", L"MainSubtitleFont", L"");
  m_map_strvar[STRVAR_SECONDARYSUBTITLEFONT] = GetProfileString(L"Settings", L"SecondarySubtitleFont", L"");
}

void PlayerPreference::Update()
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
  WriteProfileInt(ResStr(IDS_R_SETTINGS), L"ToggleFullScreenWhenPlaybackStarted", m_map_intvar[INTVAR_TOGGLEFULLSCRENWHENPLAYBACKSTARTED]);
  WriteProfileInt(ResStr(IDS_R_SETTINGS), L"LeftClickToPause", m_map_intvar[INTVAR_LEFTCLICK2PAUSE]);
  WriteProfileInt(ResStr(IDS_R_SETTINGS), L"MapCenterChToLR", m_map_intvar[INTVAR_MAP_CENTERCH2LR]);
  WriteProfileInt(ResStr(IDS_R_SETTINGS), L"CheckFileAssocOnStartUp", m_map_intvar[INTVAR_CHECKFILEASSOCONSTARTUP]);
  WriteProfileInt(ResStr(IDS_R_SETTINGS), L"DisableAds", m_map_intvar[INTVAR_PLAYAD]);

  WriteProfileString(L"Settings", L"HotkeyScheme", m_map_strvar[STRVAR_HOTKEYSCHEME].c_str());
  WriteProfileString(L"Settings", L"SubtitleSaveMethod", m_map_strvar[STRVAR_SUBTITLE_SAVEMETHOD].c_str());     // subtitle save method
  WriteProfileString(L"Settings", L"SubtitleSaveFolder", m_map_strvar[STRVAR_SUBTITLE_SAVE_CUSTOMPATH].c_str());  // subtitle save folder

  WriteProfileString(L"Settings", L"LastSpiderPath", m_map_strvar[STRVAR_LASTSPIDERPATH].c_str());  // last spider path
  WriteProfileString(L"Settings", L"Ad", m_map_strvar[STRVAR_AD].c_str());
  WriteProfileString(L"Settings", L"HideAd", m_map_strvar[STRVAR_HIDEAD].c_str());
  WriteProfileString(L"Settings", L"TimeBmpType", m_map_strvar[STRVAR_TIMEBMP_TYPE].c_str());

  WriteProfileString(L"Settings", L"MainSubtitleFont", m_map_strvar[STRVAR_MAINSUBTITLEFONT].c_str());
  WriteProfileString(L"Settings", L"SecondarySubtitleFont", m_map_strvar[STRVAR_SECONDARYSUBTITLEFONT].c_str());
}

PlayerPreference::~PlayerPreference(void)
{
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

    sqlite_setting = new SQLliteapp(path);

    if (!sqlite_setting->db_open)
    {
      delete sqlite_setting;
      sqlite_setting = NULL;
    }
  }

	if (sqlite_setting)
  {
		sqlite_setting->exec_sql(L"CREATE TABLE IF NOT EXISTS \"settingint\" ( \"hkey\" TEXT,  \"sect\" TEXT,  \"sval\" INTEGER)");
		sqlite_setting->exec_sql(L"CREATE TABLE IF NOT EXISTS \"settingstring\" (  \"hkey\" TEXT,   \"sect\" TEXT,   \"vstring\" TEXT)");
		sqlite_setting->exec_sql(L"CREATE TABLE IF NOT EXISTS \"settingbin2\" (   \"skey\" TEXT,   \"sect\" TEXT,   \"vdata\" BLOB)");
		sqlite_setting->exec_sql(L"CREATE UNIQUE INDEX IF NOT EXISTS \"pkey\" on settingint (hkey ASC, sect ASC)");
		sqlite_setting->exec_sql(L"CREATE UNIQUE INDEX IF NOT EXISTS \"pkeystring\" on settingstring (hkey ASC, sect ASC)");
		sqlite_setting->exec_sql(L"CREATE UNIQUE INDEX IF NOT EXISTS \"pkeybin\" on settingbin2 (skey ASC, sect ASC)");
    sqlite_setting->exec_sql(L"PRAGMA synchronous=OFF");
    sqlite_setting->exec_sql(L"DELETE FROM \"settingint\" WHERE \"hkey\" ISNULL OR \"sect\" ISNULL");
    sqlite_setting->exec_sql(L"DELETE FROM \"settingstring\" WHERE \"hkey\" ISNULL OR \"sect\" ISNULL");
    sqlite_setting->exec_sql(L"DELETE FROM \"settingbin2\" WHERE \"hkey\" ISNULL OR \"sect\" ISNULL");
		sqlite_setting->exec_sql(L"DROP TABLE  IF EXISTS  \"settingbin\"");
  }
}

void PlayerPreference::Uninit()
{
  if(sqlite_setting)
    sqlite_setting->exec_sql(L"PRAGMA synchronous=ON");

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
