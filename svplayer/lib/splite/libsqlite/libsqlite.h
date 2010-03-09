// libsqlite.h

#pragma once
#include "stdafx.h"
#include "../sqlite3.h"
#include <stdlib.h>
#include <vector>
#include "../../../src/svplib/svplib.h"
#include "../../../src/svplib/SVPToolBox.h"

class SQLITE3 {
private:
	sqlite3 *db;
	CSVPToolBox svpTool;

public:
	char *zErrMsg;
	char **result;
	int rc;
	int nrow,ncol;
	int db_open;

	std::vector<std::string> vcol_head;
	std::vector<std::string> vdata;


	SQLITE3(std::string tablename);
	~SQLITE3();

	void begin_transaction() ;
	void end_transaction() ;
	int exec_sql(std::string s_exe) ;

	int exec_sql_u(CString szSQL);
    int exec_insert_update_sql_u(CString szSQL, CString szUpdate);

    int get_single_int_from_sql(CString szSQL, int nDefault);

	// Retrieve an integer value from INI file or registry.
	UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, bool fallofftoreg = true);

	// Sets an integer value to INI file or registry.
	BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, bool fallofftoreg = true);

	// Retrieve a string value from INI file or registry.
	CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPCTSTR lpszDefault = NULL, bool fallofftoreg = true);

	// Sets a string value to INI file or registry.
	BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPCTSTR lpszValue);

	// Retrieve an arbitrary binary value from INI file or registry.
	BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPBYTE* ppData, UINT* pBytes, bool fallofftoreg = true);

	// Sets an arbitrary binary value to INI file or registry.
	BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPBYTE pData, UINT nBytes);
};
