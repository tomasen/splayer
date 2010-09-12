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
void SQLITE3::begin_transaction() 
{
    sqlite3_exec(db, "BEGIN;",0,0,0);
}
void SQLITE3::end_transaction() 
{
    sqlite3_exec(db, "END;",0,0,0);

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

    if(zErrMsg){
        SVP_LogMsg6("sqlite %s %d %d %s", zErrMsg ,nrow,ncol, s_exe.c_str());
    }
    if(vcol_head.size()!=0) { vcol_head.clear();  }
    if(vdata.size()!=0)     { vdata.clear(); }

    if( rc == SQLITE_OK ){
        for(int i=0; i < ncol; ++i)
            vcol_head.push_back((result[i]));   /* First row heading */
        for(int i=0; i < ncol*nrow; ++i){
             //SVP_LogMsg6("sqlite %x", result[ncol+i] );
            if(result[ncol+i]){
                vdata.push_back(result[ncol+i]);
            }else{
                //{null}
                vdata.push_back("{null}");
            }
        }
    }
    sqlite3_free_table(result);
    return rc;
}

SQLITE3::~SQLITE3(){
    sqlite3_close(db);
}
int SQLITE3::get_single_int_from_sql(CString szSQL, int nDefault)
{
    int iDestLen;
    char* buff = svpTool.CStringToUTF8(szSQL, &iDestLen);
    exec_sql(buff);
    free(buff);
    if(nrow == 1 && ncol == 1 && vdata.at(0).c_str() != "{null}"){
        UINT iret = atoi(vdata.at(0).c_str());
        return iret;
    }else{
        return nDefault;
    }
    return nDefault;
}

// Retrieve an integer value from INI file or registry.
UINT  SQLITE3::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, bool fallofftoreg )
{
    CString szSQL;
    szSQL.Format(_T("SELECT sval FROM settingint WHERE hkey = \"%s\" AND sect = \"%s\" "), lpszSection, lpszEntry );
    int iDestLen;
    char* buff = svpTool.CStringToUTF8(szSQL, &iDestLen);
    exec_sql(buff);
    free(buff);
    //SVP_LogMsg6("sql gotx %d %d",nrow,ncol);
    if(nrow == 1 && ncol == 1){
        //	SVP_LogMsg6("sql got %s",vdata.at(0).c_str());
        UINT iret = atoi(vdata.at(0).c_str());
        //SVP_LogMsg6("Get int %s %x %s",CStringA(lpszEntry), iret,vdata.at(0).c_str());
        return iret;
    }else if(fallofftoreg){
        //	SVP_LogMsg6("lalala");
        return AfxGetApp()->GetProfileInt(lpszSection,lpszEntry,nDefault);
    }else{
        return nDefault;
    }
    return nDefault;
}

int  SQLITE3::exec_sql_u(CString szSQL){
    int iDestLen;
    char* buff = svpTool.CStringToUTF8(szSQL, &iDestLen);
    int ret = exec_sql(buff);
    free(buff);
    return ret;
}
int  SQLITE3::exec_insert_update_sql_u(CString szSQL, CString szUpdate)
{
    //SVP_LogMsg5( szSQL );
    //SVP_LogMsg5( szUpdate );
    int iDestLen;
    char* buff = svpTool.CStringToUTF8(szSQL, &iDestLen);
    int ret = exec_sql(buff);
    free(buff);
    if(ret != SQLITE_OK)
    {
        char* buff2 = svpTool.CStringToUTF8(szUpdate, &iDestLen);
        ret = exec_sql(buff2);
        free(buff2);
    }
    return ret;
}
// Sets an integer value to INI file or registry.
BOOL  SQLITE3::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, bool fallofftoreg )
{
    CString szSQL;
    szSQL.Format(_T("REPLACE INTO settingint  ( hkey, sect, sval ) VALUES (\"%s\" , \"%s\" ,\"%d\" )"), lpszSection, lpszEntry ,nValue);
    int iDestLen;
    char* buff = svpTool.CStringToUTF8(szSQL, &iDestLen);
    exec_sql(buff);
    free(buff);
    if(zErrMsg){
        //	SVP_LogMsg6("lalala2");
        if(fallofftoreg){
            return AfxGetApp()->WriteProfileInt(lpszSection,lpszEntry,nValue);
        }else{
            return false;
        }
    }
    return true;
}

// Retrieve a string value from INI file or registry.
CString  SQLITE3::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                                   LPCTSTR lpszDefault, bool fallofftoreg )
{
    CString szSQL;
    szSQL.Format(_T("SELECT vstring FROM settingstring WHERE hkey = \"%s\" AND sect = \"%s\" "), lpszSection, lpszEntry );
    int iDestLen;
    char* buff = svpTool.CStringToUTF8(szSQL, &iDestLen);
    exec_sql(buff);
    free(buff);
    //SVP_LogMsg6("sql gotx %d %d",nrow,ncol);
    if(nrow == 1 && ncol == 1 ){
        //	SVP_LogMsg6("sql got %s",vdata.at(0).c_str());
		if(vdata.at(0).length() > 1){
			return  svpTool.UTF8ToCString((char*)vdata.at(0).c_str(), vdata.at(0).length()-1);
		}else{
			if (lpszDefault)
				return CString(lpszDefault);
			else
				return _T("");
		}

    }else if (fallofftoreg) {
        //	SVP_LogMsg6("lalala");
        return AfxGetApp()->GetProfileString(lpszSection,lpszEntry,lpszDefault);
    }
    else
    {
       if (lpszDefault)
           return CString(lpszDefault);
       else
           return _T("");
    }
}

// Sets a string value to INI file or registry.
BOOL  SQLITE3::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                                  LPCTSTR lpszValue)
{
    CString szSQL;
    szSQL.Format(_T(""), lpszSection, lpszEntry ,lpszValue);
    int iDestLen;
    char* buff1 = svpTool.CStringToUTF8(lpszSection, &iDestLen);
    char* buff2 = svpTool.CStringToUTF8(lpszEntry, &iDestLen);

    char* buff ;
    if(lpszValue){
        char* buff3 = svpTool.CStringToUTF8(lpszValue, &iDestLen);
        buff = sqlite3_mprintf("REPLACE INTO settingstring  ( hkey, sect, vstring ) VALUES (\"%q\" , \"%q\" , %Q )",
            buff1,buff2,buff3);
        free(buff3);
    }else{
        AfxGetApp()->WriteProfileString(lpszSection,lpszEntry,lpszValue);
        buff = sqlite3_mprintf("DELETE FROM settingstring WHERE hkey = \"%q\" AND sect = \"%q\"",
            buff1,buff2);
    }

    exec_sql(buff);
    if (buff) {
        sqlite3_free(buff);
    }

    free(buff1);
    free(buff2);

    if(zErrMsg){
        //	SVP_LogMsg6("lalala2");
        return AfxGetApp()->WriteProfileString(lpszSection,lpszEntry,lpszValue);
    }
    return true;
}

// Retrieve an arbitrary binary value from INI file or registry.
BOOL  SQLITE3::GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                                LPBYTE* ppData, UINT* pBytes, bool fallofftoreg)
{
    CString szSQL;
    szSQL.Format(_T("SELECT vdata FROM settingbin2 WHERE skey = \"%s\" AND sect = \"%s\" ") , lpszSection, lpszEntry);
    int iDestLen;
    char* buff = svpTool.CStringToUTF8(szSQL, &iDestLen);
    char *tail;
    sqlite3_stmt *stmt=0;
    int err = 1;
    if(sqlite3_prepare_v2(db, buff, strlen(buff), &stmt, 0) == SQLITE_OK)
    {
        //printf("Could not prepare statement.\n");
        //return;
        rc = sqlite3_step(stmt);
        if( rc==SQLITE_ROW )
        {
            int newBlobSize = sqlite3_column_bytes(stmt, 0);
            if(newBlobSize > 0){
                void* buffx = malloc(newBlobSize);
                *pBytes = newBlobSize;    
                memcpy(buffx, sqlite3_column_blob(stmt, 0), newBlobSize);
                *ppData = (LPBYTE)buffx;
                //SVP_LogMsg6("lalalax got bin %d", newBlobSize);
                err = 0;
            }
        } 
        sqlite3_finalize(stmt); 
    }
    free(buff);
    if(!err){
        return true;
    }else if (fallofftoreg){
        //SVP_LogMsg6("lalalax");
        return AfxGetApp()->GetProfileBinary(lpszSection,lpszEntry,ppData,pBytes);
    }
    else
        return false;
}


// Sets an arbitrary binary value to INI file or registry.
BOOL  SQLITE3::WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
                                  LPBYTE pData, UINT nBytes)
{
    CString szSQL;
    szSQL.Format(_T("REPLACE INTO settingbin2  ( skey, sect, vdata ) VALUES (\"%s\" , \"%s\" , ?)"), lpszSection, lpszEntry );
    char *tail;
    sqlite3_stmt *stmt=0;
//	SVP_LogMsg5(L" sqlite bind: %s %d\n", szSQL, nBytes);

    int iDestLen, err = 0;
    char* buff = svpTool.CStringToUTF8(szSQL, &iDestLen);
    //exec_sql(buff);
	if( sqlite3_prepare_v2(db, buff,strlen(buff)+1, &stmt, NULL) == SQLITE_OK &&
		sqlite3_reset(stmt) == SQLITE_OK && sqlite3_bind_blob(stmt, 1, pData,nBytes, NULL) == SQLITE_OK){
			if(sqlite3_step(stmt)!=SQLITE_DONE) {
				SVP_LogMsg6("Error message: %s\n", sqlite3_errmsg(db));
				err = 1;
			}
			sqlite3_finalize(stmt); 
	}else{
			SVP_LogMsg6("Error message sqlite bind: %s\n", sqlite3_errmsg(db));
			err = 1;
	}

    free(buff);
    if(err){
        SVP_LogMsg6("lalala2");
        return AfxGetApp()->WriteProfileBinary(lpszSection,lpszEntry,pData,nBytes);
    }
    return true;
}
