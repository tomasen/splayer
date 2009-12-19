// This is the main DLL file.



#include "libsqlite.h"


	SQLITE3::SQLITE3 (std::string tablename): zErrMsg(0), rc(0),db_open(0) {
		rc = sqlite3_open(tablename.c_str(), &db);
		if( rc ){
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
		}
		db_open=1;
	}

	int SQLITE3::exec_sql(std::string s_exe) {
		rc = sqlite3_get_table(
			db,              /* An open database */
			s_exe.c_str(),       /* SQL to be executed */
			&result,       /* Result written to a char *[]  that this points to */
			&nrow,             /* Number of result rows written here */
			&ncol,          /* Number of result columns written here */
			&zErrMsg          /* Error msg written here */
			);

		if(vcol_head.size()<0) { vcol_head.clear();  }
		if(vdata.size()<0)     { vdata.clear(); }

		if( rc == SQLITE_OK ){
			for(int i=0; i < ncol; ++i)
				vcol_head.push_back((result[i]));   /* First row heading */
			for(int i=0; i < ncol*nrow; ++i)
				vdata.push_back(result[ncol+i]);
		}
		sqlite3_free_table(result);
		return rc;
	}

	SQLITE3::~SQLITE3(){
		sqlite3_close(db);
	}


	// Retrieve an integer value from INI file or registry.
	UINT  SQLITE3::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
	{

		return nDefault;
	}

	// Sets an integer value to INI file or registry.
	BOOL  SQLITE3::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
	{
		return false;
	}

	// Retrieve a string value from INI file or registry.
	CString  SQLITE3::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPCTSTR lpszDefault )
	{
		return lpszDefault ;
	}

	// Sets a string value to INI file or registry.
	BOOL  SQLITE3::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPCTSTR lpszValue)
	{
		return false;
	}

	// Retrieve an arbitrary binary value from INI file or registry.
	BOOL  SQLITE3::GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPBYTE* ppData, UINT* pBytes)
	{
		return false;
	}


	// Sets an arbitrary binary value to INI file or registry.
	BOOL  SQLITE3::WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
		LPBYTE pData, UINT nBytes)
	{
		return false;
	}