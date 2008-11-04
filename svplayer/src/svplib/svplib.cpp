#include "svplib.h"
#include "SVPNet.h"
#include "SVPHash.h"

void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath, CAtlList<CString>* mSubLists){
	CSVPNet svpNet;
	CSVPhash svpHash;
	CString szFileHash  = svpHash.ComputerFileHash(fnVideoFilePath);
	if ( svpNet.QuerySubByVideoPathOrHash(fnVideoFilePath,szFileHash) ){
		return ;
	}

		//load sub file to sublist
		for(int i = 0; i < svpNet.svpToolBox.szaSubTmpFileList.GetCount(); i++){
			mSubLists->AddTail( svpNet.svpToolBox.getSubFileByTempid(i, fnVideoFilePath) );
		}
	
}


void SVP_LogMsg(CString logmsg, int level){


	CStdioFile f;
	
	
	if(f.Open(SVP_DEBUG_LOGFILEPATH, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate | CFile::typeText))
	{
		f.SeekToEnd();
		f.WriteString(logmsg+_T("\r\n"));
		f.Close();
	}
		
}

