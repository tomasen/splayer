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
    return true;
  }
  catch(...)
  {
  }
  return false;
}


int SQLliteapp::get_single_int_from_sql(std::wstring szSQL, int nDefault)
{
  exec_sql(szSQL);
  if(nrow == 1)
    return  atoi(Strings::WStringToString(vdata.at(0)).c_str());
  else
    return nDefault;
}

int SQLliteapp::exec_insert_update_sql_u(std::wstring szSQL, std::wstring szUpdate)
{
  int ret = exec_sql(szSQL);
  if(ret != true)
    ret = exec_sql(szUpdate);

  return ret;
}


UINT SQLliteapp::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, bool fallofftoreg)
{
  sqlitepp::string_t str;
  sqlitepp::string_t str2 = lpszSection;
  sqlitepp::string_t str3 = lpszEntry;

  try
  {
    sqlitepp::statement st(m_db);

    st << "SELECT sval FROM settingint WHERE hkey = :hkey AND sect = :sect", 
             sqlitepp::into(str), sqlitepp::use(str2), sqlitepp::use(str3);

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
  sqlitepp::string_t str1 = lpszSection;
  sqlitepp::string_t str2 = lpszEntry;

  try
  {
    m_db << "INSERT OR REPLACE INTO settingint (hkey, sect, sval ) VALUES (:hkey, :sect ,:sval)",
              sqlitepp::use(str1), sqlitepp::use(str2), sqlitepp::use(nValue);
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
  sqlitepp::string_t str;
  sqlitepp::string_t str2 = lpszSection;
  sqlitepp::string_t str3 = lpszEntry;

  try
  {
    sqlitepp::statement st(m_db);

    st << "SELECT vstring FROM settingstring WHERE hkey = :hkey AND sect = :sect ",
            sqlitepp::into(str), sqlitepp::use(str2), sqlitepp::use(str3);
    st.exec();

    if(!str.empty())
      return CString(str.c_str());
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
  sqlitepp::string_t str = lpszSection;
  sqlitepp::string_t str2 = lpszEntry;
  sqlitepp::string_t str3 = lpszValue;

  try
  {
    if(lpszValue)
      m_db << "INSERT OR REPLACE INTO settingstring(hkey, sect, vstring) VALUES (:hkey, :sect, :vstring)",
                sqlitepp::use(str), sqlitepp::use(str2),sqlitepp::use(str3);
    else
      m_db << "DELETE FROM settingstring WHERE hkey = :hkey AND sect = :sect",
                sqlitepp::use(str), sqlitepp::use(str2);
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
  std::vector<char> bin;
  sqlitepp::string_t str = lpszSection;
  sqlitepp::string_t str2 = lpszEntry;
  *ppData = NULL;

  try
  {
    sqlitepp::statement st(m_db);
    st << "SELECT vdata FROM settingbin2 WHERE skey = :skey AND sect = :sect)",
            sqlitepp::into(bin), sqlitepp::use(str), sqlitepp::use(str2);
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
  std::vector<char> bin(nBytes);
  memcpy(&bin[0],pData,nBytes);
  sqlitepp::string_t s1 = lpszSection;
  sqlitepp::string_t s2 = lpszEntry;

  try
  {
    sqlitepp::statement st(m_db);
    st << "REPLACE INTO settingbin2 (skey, sect, vdata) VALUES (:skey,:sect,:vdata)",
            sqlitepp::use(s1), sqlitepp::use(s2); sqlitepp::use(bin);
    st.exec();

    return true;
  }
  catch(...)
  {
    return AfxGetApp()->WriteProfileBinary(lpszSection,lpszEntry,pData,nBytes);
  }
  return false;
}

