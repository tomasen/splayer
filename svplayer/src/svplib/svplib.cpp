#include "svplib.h"

void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath){

}


void SVP_LogMsg(CString logmsg){


	CStdioFile f;
	
	
	if(f.Open(_T("C:\\SVPDebug.log"), CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate | CFile::typeText))
	{
		f.SeekToEnd();
		f.WriteString(logmsg);
		f.Close();
	}



		
}

