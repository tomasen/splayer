#pragma once

#include <vector>
#include <string>

#include <..\..\..\Thirdparty\sqlitepp\sqlitepp\sqlitepp.hpp>

class SQLliteapp
{
public:
  SQLliteapp(std::wstring m_dbfile);
  ~SQLliteapp(void);

  int exec_sql(std::wstring s_exe); // just get ONE col
  int exec_sql_u(CString szSQL);

  int exec_insert_update_u(CString szSQL);
  int exec_insert_update_sql_u(CString szSQL, CString szUpdate);
  int get_single_int_from_sql(CString szSQL, int nDefault); //just get ONE col

  void begin_transaction();
  void end_transaction();

  // Retrieve an integer value from INI file or registry.
  UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, bool fallofftoreg = true);
  // Sets an integer value to INI file or registry.
  BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, bool fallofftoreg = true);

  // Retrieve a string value from INI file or registry.
  CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL, bool fallofftoreg = true);
  // Sets a string value to INI file or registry.
  BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

  // Retrieve an arbitrary binary value from INI file or registry.
  BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
    LPBYTE* ppData, UINT* pBytes, bool fallofftoreg = true);
  // Sets an arbitrary binary value to INI file or registry.
  BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
    LPBYTE pData, UINT nBytes);


private:
  //sqlitepp::transaction m_tran;
  sqlitepp::session m_db;
  std::wstring m_dbfile;

public:
  int db_open;
  int nrow;
  std::vector<std::wstring> vdata;
};
