#include "svplib.h"
#include "SVPNet.h"
#include "SVPHash.h"

class CSVPSubUploadThreadData{
public:
	CString fnVideoFilePath;
	CString szSubPath;
};
void SVP_RealUploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath, CString szSubPath){

	CSVPNet svpNet;
	CSVPhash svpHash;
	CSVPToolBox svpToolBox;
	CString szFileHash  = svpHash.ComputerFileHash(fnVideoFilePath);

	CStringArray szaSubFiles;
	svpToolBox.FindAllSubfile(szSubPath, &szaSubFiles);

	CString szSubHash = svpHash.ComputerSubFilesFileHash(&szaSubFiles);
	SVP_LogMsg(CString("Got Sub Hash ") + svpToolBox.Implode( _T(" | "), &szaSubFiles) + _T(" -> ") + szSubHash );

	if ( svpNet.WetherNeedUploadSub(fnVideoFilePath,szFileHash, szSubHash) ){
		svpNet.UploadSubFileByVideoAndHash(fnVideoFilePath,szFileHash, szSubHash, &szaSubFiles);
		return ;
	}
}
UINT __cdecl SVPThreadUploadSubFile( LPVOID lpParam ) 
{ 

	CSVPSubUploadThreadData* pData = (CSVPSubUploadThreadData*)lpParam;
	SVP_RealUploadSubFileByVideoAndSubFilePath(pData->fnVideoFilePath,  pData->szSubPath);

	delete pData;
	return 0; 
} 

void SVP_CheckUpdaterExe(){
	//检查 updater.exe 是否可写
	CSVPToolBox svpToolBox;
	CSVPNet svpNet;
	CString szUpdaterPath = svpToolBox.GetPlayerPath(_T("Updater.exe"));
	if ( svpToolBox.isWriteAble(szUpdaterPath) ){
		//如果可写，将文件版本号+文件长度hash 发送给 http://svplayer.shooter.cn/api/updater.php 

		CString FileVersionHash = svpToolBox.getFileVersionHash(szUpdaterPath);

		// 服务器判断是否需要升级 updater.exe 。 如果不需要升级，返回404。如果需要升级，则返回updater.exe供下载
		if ( svpNet.CheckUpdaterExe(FileVersionHash, szUpdaterPath) ){
			
		}
		//运行升级程序
		ShellExecute( NULL, _T("open"), szUpdaterPath, _T("") , _T(""), SW_SHOW);	
	}

}

void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath, CStringArray* szSubArray){
	
	CSVPNet svpNet;
	CSVPhash svpHash;
	CString szFileHash  = svpHash.ComputerFileHash(fnVideoFilePath);
	if ( svpNet.QuerySubByVideoPathOrHash(fnVideoFilePath,szFileHash) ){
		return ;
	}

		//load sub file to sublist
		for(int i = 0; i < svpNet.svpToolBox.szaSubTmpFileList.GetCount(); i++){
			CString szSubFilePath = svpNet.svpToolBox.getSubFileByTempid(i, fnVideoFilePath);
			szSubArray->Add(szSubFilePath);
			SVP_LogMsg(CString(_T("Adding sub file ")) + szSubFilePath ); //TODO: if its vobsub, load perfered language
		}
	return;
}
void SVP_UploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath, CString szSubPath){
	CSVPSubUploadThreadData* pData = new CSVPSubUploadThreadData();
	pData->fnVideoFilePath = fnVideoFilePath;
	pData->szSubPath = szSubPath;
	AfxBeginThread(SVPThreadUploadSubFile, pData); 
}

void SVP_LogMsg(CString logmsg, int level){


	CStdioFile f;
	CSVPToolBox svpToolBox;
	CString szLogPath = svpToolBox.GetPlayerPath(_T("SVPDebug.log"));
	if(f.Open(szLogPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate | CFile::typeBinary))
	{
		f.SeekToEnd();
		f.WriteString(logmsg+_T("\r\n"));
		
		f.Flush();
		f.Close();
	}
		
}

