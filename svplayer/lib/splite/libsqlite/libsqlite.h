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

	int exec_sql(std::string s_exe) ;


	// Retrieve an integer value from INI file or registry.
	UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);

	// Sets an integer value to INI file or registry.
	BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);

	// Retrieve a string value from INI file or registry.
	CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPCTSTR lpszDefault = NULL);

	// Sets a string value to INI file or registry.
	BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPCTSTR lpszValue);

	// Retrieve an arbitrary binary value from INI file or registry.
	BOOL GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPBYTE* ppData, UINT* pBytes);

	// Sets an arbitrary binary value to INI file or registry.
	BOOL WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPBYTE pData, UINT nBytes);
};
