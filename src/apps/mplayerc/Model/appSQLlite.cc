#include "stdafx.h"
#include "appSQLlite.h"

#include <vector>
#include <string>
#include <iostream>

#include "..\Utils\Strings.h"


using namespace sqlitepp;

SQLliteapp::SQLliteapp(std::wstring dbfile)
{
  try
  {
    m_dbfile = dbfile;
    m_db.open(m_dbfile);
    db_open=1;
  }
  catch(...)
  {
    db_open = 0;
  }
}

SQLliteapp::~SQLliteapp(void)
{
  m_db.close();
}


void SQLliteapp::end_transaction()
{
  m_db << "END;";
}

void SQLliteapp::begin_transaction()
{
  m_db << "BEGIN;";
}


int SQLliteapp::exec_sql(std::wstring s_exe)
{
  vdata.clear();
  nrow = 0;

  try
  {
    sqlitepp::string_t str;
    sqlitepp::statement st(m_db);
    st << s_exe, sqlitepp::into(str);

    while(st.exec())
    {
      vdata.push_back(str);
      str.clear();
    }

    nrow = vdata.size();
    return 1;
  }
  catch(...)
  {
  }
  return 0;
}

int SQLliteapp::exec_sql_u(CString szSQL)
{
  std::wstring sql = szSQL.GetBuffer();
  return exec_sql(sql);
}
int SQLliteapp::get_single_int_from_sql(CString szSQL, int nDefault)
{
  exec_sql(szSQL.GetBuffer());
  if(nrow == 1)
    return  atoi(Strings::WStringToString(vdata.at(0)).c_str());
  else
    return nDefault;
}

int SQLliteapp::exec_insert_update_sql_u(CString szSQL, CString szUpdate)
{
  int ret = exec_sql_u(szSQL);
  if(ret != 1)
    ret = exec_sql_u(szUpdate);

  return ret;
}

int SQLliteapp::exec_insert_update_u(CString szSQL)
{
  try
  {
    sqlitepp::statement st(m_db);
    st << szSQL.GetBuffer();
    st.exec();
    return 1;
  }
  catch(...)
  {
  }
  return 0;
}


UINT SQLliteapp::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, bool fallofftoreg)
{
  if(!lpszSection || !lpszEntry)
    return nDefault;

  CString szSQL;
  szSQL.Format(_T("SELECT sval FROM settingint WHERE hkey = '%s' AND sect = '%s' "), lpszSection, lpszEntry );

  try
  {
    sqlitepp::statement st(m_db);
    sqlitepp::string_t str;
    st << szSQL.GetBuffer(), sqlitepp::into(str);
    st.exec();

    if (str.empty())
      return nDefault;
    return atoi(Strings::WStringToString(str).c_str());
  }
  catch(...)
  {
    if(fallofftoreg)
      return AfxGetApp()->GetProfileInt(lpszSection,lpszEntry,nDefault);
  }
  return nDefault;
}

BOOL SQLliteapp::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue,  bool fallofftoreg)
{
  if(!lpszSection || !lpszEntry)
    return false;

  CString szSQL;
  szSQL.Format(_T("INSERT OR REPLACE INTO settingint (hkey, sect, sval ) VALUES ('%s', '%s' ,'%d')"), lpszSection, lpszEntry ,nValue);

  try
  {
    m_db << szSQL.GetBuffer();
    return true;
  }
  catch(...)
  {
    if(fallofftoreg)
      return AfxGetApp()->WriteProfileInt(lpszSection,lpszEntry,nValue);
  }
  return false;
}


CString SQLliteapp::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, bool fallofftoreg)
{
  if(!lpszSection || !lpszEntry)
  {
    if (lpszDefault)
      return CString(lpszDefault);
    else
      return L"";
  }

  CString szSQL;
  szSQL.Format(_T("SELECT vstring FROM settingstring WHERE hkey = '%s' AND sect = '%s' "), lpszSection, lpszEntry );

  try
  {
    sqlitepp::statement st(m_db);
    sqlitepp::string_t str;

    st << szSQL.GetBuffer(), sqlitepp::into(str);
    st.exec();
    if(!str.empty())
      return lpszDefault;
  }
  catch(...)
  {
    if(fallofftoreg)
      return AfxGetApp()->GetProfileString(lpszSection,lpszEntry,lpszDefault);
  }

  if (lpszDefault)
    return CString(lpszDefault);
  else
    return L"";
}

BOOL SQLliteapp::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
  if(!lpszSection || !lpszEntry)
    return false;

  CString szSQL;
  if(lpszValue)
    szSQL.Format(L"INSERT OR REPLACE INTO settingstring(hkey, sect, vstring) VALUES ('%s', '%s', '%s')",lpszSection, lpszEntry, lpszValue);
  else
    szSQL.Format(L"DELETE FROM settingstring WHERE hkey = '%s' AND sect = '%s'",lpszSection, lpszEntry);

  try
  {
    m_db << szSQL.GetBuffer();
    return true;
  }
  catch(...)
  {
    return AfxGetApp()->WriteProfileString(lpszSection,lpszEntry,lpszValue);
  }
  return false;
}

BOOL SQLliteapp::GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                                  LPBYTE* ppData, UINT* pBytes, bool fallofftoreg)
{
  if(!lpszSection || !lpszEntry)
    return false;

  CString szSQL;
  szSQL.Format(L"SELECT vdata FROM settingbin2 WHERE skey = '%s' AND sect = '%s')", lpszSection, lpszEntry);
  *ppData = NULL;

  try
  {
    std::vector<char> bin;
    sqlitepp::statement st(m_db);
    st << szSQL.GetBuffer(), sqlitepp::into(bin);
    st.exec();

    *pBytes = bin.size();

    if(bin.size() == 0)
      return false;

    char *p= new char[bin.size()];
    *ppData = (LPBYTE)p;
    memcpy(p, &bin[0], bin.size());

    return true;
  }
  catch(...)
  {
    if(fallofftoreg)
      AfxGetApp()->GetProfileBinary(lpszSection,lpszEntry,ppData,pBytes);
  }
  return false;
}

BOOL SQLliteapp::WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
  if(!lpszSection || !lpszEntry)
    return false;

  CString szSQL;
  szSQL.Format(L"INSERT OR REPLACE INTO settingbin2(skey, sect, vdata) VALUES (:skey, :sect, :vdata)");

  try
  {
    std::vector<char> bin;
    for(int i=0;i<nBytes;++i)
      bin.push_back(pData[i]);
    sqlitepp::string_t s1 = lpszSection;
    sqlitepp::string_t s2 = lpszEntry;

    sqlitepp::statement st(m_db);
    st << szSQL.GetBuffer(), sqlitepp::use(s1), sqlitepp::use(s2); sqlitepp::use(bin);
    st.exec();

    return true;
  }
  catch(...)
  {
    return AfxGetApp()->WriteProfileBinary(lpszSection,lpszEntry,pData,nBytes);
  }
  return false;
}

