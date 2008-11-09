#include "svplib.h"
#include "SVPNet.h"
#include "SVPHash.h"

void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath, CStringArray* szSubArray){
	CSVPNet svpNet;
	CSVPhash svpHash;
	CString szFileHash  = svpHash.ComputerFileHash(fnVideoFilePath);
	if ( svpNet.QuerySubByVideoPathOrHash(fnVideoFilePath,szFileHash) ){
		return ;
	}

		//load sub file to sublist
		for(int i = 0; i < svpNet.svpToolBox.szaSubTmpFileList.GetCount(); i++){
			szSubArray->Add(svpNet.svpToolBox.getSubFileByTempid(i, fnVideoFilePath));
		}
	return;
}

void SVP_UploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath, CString szSubPath){
		CSVPNet svpNet;
		CSVPhash svpHash;
		CSVPToolBox svpToolBox;
		CString szFileHash  = svpHash.ComputerFileHash(fnVideoFilePath);
		//TODO: Find all sub files
		CStringArray szaSubFiles;
		svpToolBox.FindAllSubfile(szSubPath, &szaSubFiles);
		//TODO: Computer Subfile hash
		//TODO: Build Subfile Package

		if ( svpNet.UploadSubFileByVideoAndHash(fnVideoFilePath,szFileHash,szSubPath) ){
			return ;
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

