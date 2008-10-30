#include "svplib.h"
#include "SVPNet.h"
#include "SVPHash.h"

void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath){
	CSVPNet svpNet;
	svpNet.QuerySubByVideoPathOrHash(fnVideoFilePath, _T("d8ewy3y28ry378"));
}


void SVP_LogMsg(CString logmsg){


	CStdioFile f;
	
	
	if(f.Open(SVP_DEBUG_LOGFILEPATH, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate | CFile::typeText))
	{
		f.SeekToEnd();
		f.WriteString(logmsg+_T("\r\n"));
		f.Close();
	}
		
}

