#include "svplib.h"
#include "SVPNet.h"
#include "SVPHash.h"
#include <stdio.h> 
#include <shlobj.h>
#include <streams.h>
#include <afxtempl.h>
#include <errno.h>
#include "..\apps\mplayerc\mplayerc.h"

static CAtlList<std::wstring> * szGStatMsg = NULL;

static LONGLONG	m_PerfFrequency = 0;
LONGLONG SVPGetPerfCounter()
{
	LONGLONG		i64Ticks100ns;
	if (m_PerfFrequency == 0)
	{
		QueryPerformanceFrequency ((LARGE_INTEGER*)&m_PerfFrequency);
	}
	if (m_PerfFrequency != 0){
		QueryPerformanceCounter ((LARGE_INTEGER*)&i64Ticks100ns);
		i64Ticks100ns	= LONGLONG((double(i64Ticks100ns) * 10000000) / double(m_PerfFrequency) + 0.5);

		return i64Ticks100ns;
	}
	return 0;
}

void SVP_RealUploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath,
                                                CString szSubPath,
                                                int iDelayMS,
                                                CStringArray* szaPostTerms)
{
  //原参数倒入新参数
  std::vector<std::wstring> szaPostTerms_STL;
  if (szaPostTerms != NULL)
  {
    for (int i = 0; i < szaPostTerms -> GetCount(); i++)
      szaPostTerms_STL.push_back((LPCTSTR)szaPostTerms -> GetAt(i));
  }

  RealUploadSubFileByVideoAndSubFilePath_STL((LPCTSTR)fnVideoFilePath,
    (LPCTSTR)szSubPath,
    iDelayMS,
    &szaPostTerms_STL);

  //新参数倒回原参数
  if (szaPostTerms != NULL)
  {
    szaPostTerms -> RemoveAll();
    for (int i = 0; i < szaPostTerms_STL.size(); i++)
      szaPostTerms -> Add((szaPostTerms_STL.at(i)).c_str());
  }
  //CSVPNet svpNet;
  //CSVPhash svpHash;
  //CSVPToolBox svpToolBox;
  //CString szFileHash  = svpHash.ComputerFileHash_STL(fnVideoFilePath);

  //CStringArray szaSubFiles;
  //svpToolBox.FindAllSubfile(szSubPath, &szaSubFiles);

  //CString szSubHash = svpHash.ComputerSubFilesFileHash(&szaSubFiles);
  //if(szSubHash.IsEmpty())
  //  return;

  //SVP_LogMsg(CString("Got Sub Hash ") + svpToolBox.Implode( _T(" | "), &szaSubFiles) + _T(" -> ") + szSubHash );
  //for(int i = 1; i <= 7 ; i++){

  //  svpNet.iTryID = i;
  //  int chk = svpNet.WetherNeedUploadSub(fnVideoFilePath,szFileHash, szSubHash,iDelayMS);
  //  if ( chk > 0){

  //    if( 0 == svpNet.UploadSubFileByVideoAndHash(fnVideoFilePath,szFileHash, szSubHash, &szaSubFiles,iDelayMS, szaPostTerms) ){
  //      return ;
  //    }
  //  }else if(chk == 0 ){
  //    break;
  //  }

  //  SVP_LogMsg5(L"Upload Sub Fail Retrying %d", i);
  //  //Fail
  //  Sleep(2000);
  //}
}

void RealUploadSubFileByVideoAndSubFilePath_STL(std::wstring fnVideoFilePath,
                                      std::wstring szSubPath,
                                      int iDelayMS,
                                      std::vector<std::wstring>* szaPostTerms)
{
  CSVPNet svpNet;
  CSVPhash svpHash;
  CSVPToolBox svpToolBox;
  std::wstring szFileHash = svpHash.ComputerFileHash_STL(fnVideoFilePath);

  std::vector<std::wstring> szaSubFiles;
  svpToolBox.FindAllSubfile(szSubPath.c_str(), &szaSubFiles);

  std::wstring szSubHash = svpHash.ComputerSubFilesFileHash(&szaSubFiles);
  if (szSubHash.empty())
    return;

  SVP_LogMsg(CString("Got Sub Hash ")
    + CString(svpToolBox.Implode(_T(" | "), &szaSubFiles).c_str())
    + _T(" -> ")
    + CString(szSubHash.c_str()));

  for (int i = 1; i <= 7; i++)
  {
    svpNet.iTryID = i;
    int chk = svpNet.WetherNeedUploadSub(fnVideoFilePath.c_str(),
      szFileHash.c_str(), szSubHash.c_str(), iDelayMS);
    if (chk > 0)
      if (0 == svpNet.UploadSubFileByVideoAndHash(fnVideoFilePath,szFileHash,
        szSubHash, &szaSubFiles, iDelayMS, szaPostTerms))
        return ;
    else if(0 == chk)
      break;
    SVP_LogMsg5(L"Upload Sub Fail Retrying %d", i);
    //Fail
    Sleep(2000);
  }
}

void SVP_RealCheckUpdaterExe(BOOL* bCheckingUpdater, UINT verbose )
{
	//检查 updater.exe 是否可写
	CSVPToolBox svpToolBox;
	CSVPNet svpNet;
	CString szUpdaterPath = svpToolBox.GetPlayerPath_STL(_T("Updater.exe")).c_str();
	if ( svpToolBox.isWriteAble(szUpdaterPath) ){
		//如果可写，将文件版本号+文件长度hash 发送给 http://svplayer.shooter.cn/api/updater.php 

		CString FileVersionHash = svpToolBox.getFileVersionHash(szUpdaterPath);

		// 服务器判断是否需要升级 updater.exe 。 如果不需要升级，返回404。如果需要升级，则返回updater.exe供下载
		if ( svpNet.CheckUpdaterExe(FileVersionHash, szUpdaterPath) ){

		}
		SVP_LogMsg( ResStr(IDS_LOG_MSG_DETECT_NEWER_VERSION_UPDATER_STARTED) ); 
		CString szPerm = _T("");
		if(verbose){
			szPerm = _T(" /verbose ");
		}else{
			szPerm = _T(" /hide ");
		}

		//运行升级程序
		ShellExecute( NULL, _T("open"), szUpdaterPath, szPerm , _T(""), SW_HIDE);	
	}else{
		if( verbose ){
			AfxMessageBox(ResStr(IDS_MSG_WARN_UPDATER_CANT_WRITE_DIR));
		}
	}
	*bCheckingUpdater = false;
}
class CCheckUpdaterPerm{
public:
	BOOL* bCheckingUpdater;
	UINT verbose;
};

UINT __cdecl SVPThreadCheckUpdaterExe(LPVOID lpParam)
{ 

	CCheckUpdaterPerm * ccup =(CCheckUpdaterPerm*) lpParam;
	SVP_RealCheckUpdaterExe( ccup->bCheckingUpdater, ccup->verbose);
	szGStatMsg = NULL;
	delete ccup;
	return 0; 
}
void SVP_CheckUpdaterExe(BOOL* bCheckingUpdater , UINT verbose){
	if(*bCheckingUpdater){
		return;
	}
	*bCheckingUpdater = true;
	CCheckUpdaterPerm * ccup = new CCheckUpdaterPerm( );
	ccup->bCheckingUpdater = bCheckingUpdater;
	ccup->verbose = verbose;
	AfxBeginThread( SVPThreadCheckUpdaterExe, (LPVOID)ccup, THREAD_PRIORITY_LOWEST);
}

class CSVPPinRenderDeadEndData{
public:
	CString szPinName;
	CString szReport;
};

void SVP_RealUploadPinRenderDeadEnd(CString szPinName, CString szReport){
	CSVPNet svpNet;
	svpNet.UploadPinRenderDeadEndReport(  szPinName,  szReport);
}
UINT __cdecl SVPThreadUploadPinRenderDeadEnd( LPVOID lpParam ) 
{ 
	CSVPPinRenderDeadEndData * svpdata = (CSVPPinRenderDeadEndData *) lpParam;
	SVP_RealUploadPinRenderDeadEnd(svpdata->szPinName, svpdata->szReport);
	delete svpdata;
	return 0; 
}
void SVP_UploadPinRenderDeadEnd(CString szPinName, CString szReport){
	CSVPPinRenderDeadEndData * svpdata = new CSVPPinRenderDeadEndData();
	svpdata->szPinName = szPinName;
	svpdata->szReport = szReport;
	AfxBeginThread( SVPThreadUploadPinRenderDeadEnd, (LPVOID)svpdata, THREAD_PRIORITY_LOWEST);
}
class CSVPCrashDmpData{
public:
	CString szDmpPath;
	CString szLogPath;
};
void SVP_RealUploadCrashDmp(CString szDmppath, CString szLogPath){
	CSVPNet svpNet;
	svpNet.UploadCrashDmp(  szDmppath,  szLogPath);
}
UINT __cdecl SVPThreadUploadCrashDmp( LPVOID lpParam ) 
{ 
	CSVPCrashDmpData * svpdata = (CSVPCrashDmpData *) lpParam;
	SVP_RealUploadCrashDmp(svpdata->szDmpPath, svpdata->szLogPath);
	delete svpdata;
	return 0; 
}
void SVP_UploadCrashDmp(CString szDmppath, CString szLogPath){
	CSVPCrashDmpData * svpdata = new CSVPCrashDmpData();
	svpdata->szDmpPath = szDmppath;
	svpdata->szLogPath = szLogPath;
	AfxBeginThread( SVPThreadUploadCrashDmp, (LPVOID)svpdata, THREAD_PRIORITY_LOWEST);
}
#define fansub_search_buf 30000
bool isSpecFanSub(CString szPath, CString szOEM){

	CString szExt = szPath.Right(4);
	szExt.MakeLower();
	if(szExt == _T(".srt") || szExt == _T(".ssa") || szExt == _T(".ass")){
		//SVP_LogMsg5(_T("SpecFanSub %s search %s"),szPath, szOEM);
		CFile subFile;
		if(!subFile.Open( szPath, CFile::modeRead|CFile::typeText)){
		//	SVP_LogMsg5(_T("Cant read Sub File %s"),szPath);
			return FALSE;
		}
		char qBuff[fansub_search_buf];
		memset(qBuff, 0, fansub_search_buf);

		subFile.Read( qBuff, fansub_search_buf );

		bool ret = false;
		if( strstr(qBuff, CStringA(szOEM) )){
			SVP_LogMsg5(_T("SpecFanSub Found %s"),szOEM);
			ret = true;
		}
		subFile.Close();
		return ret;
	}

	return FALSE;
}

void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath,
                                     CStringArray* szSubArray,
                                     CAtlList<CString>* szStatMsg,
                                     CString szLang)
{
  //原参数倒入新参数
  std::vector<std::wstring> szSubArray_STL;
  for (int i = 0; i < szSubArray -> GetCount(); i++)
    szSubArray_STL.push_back((LPCTSTR)szSubArray -> GetAt(i));
  CAtlList<std::wstring> szStatMsg_STL;
  for (int i = 0; i < szStatMsg -> GetCount(); i++)
    szStatMsg_STL.AddTail(
      (LPCTSTR)szStatMsg -> GetAt(szStatMsg -> FindIndex(i)));

  FetchSubFileByVideoFilePath_STL((LPCTSTR)fnVideoFilePath,
    &szSubArray_STL,
    &szStatMsg_STL,
    (LPCTSTR)szLang);

  //新参数倒回原参数
  szSubArray -> RemoveAll();
  for (std::vector<std::wstring>::iterator Iter = szSubArray_STL.begin();
    Iter != szSubArray_STL.end(); Iter++)
    szSubArray -> Add((*Iter).c_str());
  szStatMsg -> RemoveAll();
  for (int i = 0; i < szStatMsg_STL.GetCount(); i++)
    szStatMsg -> AddTail(
    szStatMsg_STL.GetAt(szStatMsg_STL.FindIndex(i)).c_str());

  //szSubArray->RemoveAll();
  //szSubArray->Add()
  //szGStatMsg = szStatMsg;
  //AppSettings& s = AfxGetAppSettings();
  //if(szLang.IsEmpty() && !(s.iLanguage == 0 || s.iLanguage == 2)){
  //  szLang = _T("eng");
  //}

  //if(szLang.IsEmpty())
  //  SVP_LogMsg(ResStr(IDS_LOG_MSG_USING_SVPSUB_SYSTEM_LOOKINGFOR_SUB), 31);
  //else
  //  SVP_LogMsg(ResStr(IDS_LOG_MSG_USING_SVPSUB_SYSTEM_LOOKINGFOR_ENGSUB), 31);
  //CSVPNet svpNet;
  //CSVPhash svpHash;
  //CString szFileHash  = svpHash.ComputerFileHash_STL(fnVideoFilePath);
  //SVP_LogMsg5(L"FileHash %s for %s ", szFileHash , fnVideoFilePath);

  //for(int i = 1; i <= 7; i++){
  //  svpNet.iTryID = i;
  //  int err =  svpNet.QuerySubByVideoPathOrHash_STL(fnVideoFilePath,szFileHash, _T("") , szLang) ;
  //  if(err){
  //    Sleep(2300 );


  //    SVP_LogMsg(ResStr(IDS_LOG_MSG_USING_SVPSUB_SYSTEM_RETRYING));//, 31
  //  }else{
  //    break;
  //  }
  //}

  ////load sub file to sublist
  //CString szSubFilePath;
  //int iSubTotal = 0;
  //for(int i = 0; i < svpNet.svpToolBox.szaSubTmpFileList.GetCount(); i++){
  //  szSubFilePath = svpNet.svpToolBox.getSubFileByTempid_STL(i, fnVideoFilePath);
  //  if(szSubFilePath == _T("EXIST")){
  //    SVP_LogMsg(ResStr(IDS_LOG_MSG_SVPSUB_SAMEAS_CURRENT), 31);
  //  }else if(szSubFilePath){
  //    szSubArray->Add(szSubFilePath);
  //    CPath fnPath(szSubFilePath);
  //    fnPath.StripPath();
  //    SVP_LogMsg(CString(ResStr(IDS_LOG_MSG_SVPSUB_SUCCESS_DOWNLOAD_SUB)) + (CString)fnPath , 31 ); //TODO: if its vobsub, load perfered language
  //    iSubTotal++;
  //  }else{
  //    SVP_LogMsg( svpNet.m_lastFailedMsg, 31);
  //    SVP_LogMsg(_T("Fail to get sub file name"));
  //  }
  //}

  //if(iSubTotal) {
  //  CString szLog;
  //  szLog.Format(ResStr(IDS_LOG_MSG_SVPSUB_GOT_N_SUB_FILE_DOWNLOADED), iSubTotal);
  //  SVP_LogMsg( szLog, 31 ); 
  //}

  //if(iSubTotal > 1){

  //  CString szSVPSubPerf = s.szSVPSubPerf;
  //  //SVP_LogMsg5(_T("SpecFanSub %s"), szSVPSubPerf);
  //  if(!szSVPSubPerf.IsEmpty()){
  //    for(int i = 0; i < szSubArray->GetCount(); i++){
  //      CString subFilePath = szSubArray->GetAt(i);
  //      if(isSpecFanSub(subFilePath , szSVPSubPerf)){
  //        CString szFirst =  szSubArray->GetAt(0);
  //        szSubArray->SetAt(0, subFilePath);
  //        szSubArray->SetAt(i, szFirst);
  //        break;
  //      }
  //    }
  //  }
  //}
  //szGStatMsg = NULL;
  //return;
}

void FetchSubFileByVideoFilePath_STL(std::wstring fnVideoFilePath,
                                     std::vector<std::wstring>* szSubArray,
                                     CAtlList<std::wstring> * szStatMsg,
                                     std::wstring szLang)
{
  szGStatMsg = szStatMsg;
  AppSettings& s = AfxGetAppSettings();
  if (szLang.empty() && !(s.iLanguage == 0 || s.iLanguage == 2))
    szLang = _T("eng");

  if (szLang.empty())
    SVP_LogMsg(ResStr(IDS_LOG_MSG_USING_SVPSUB_SYSTEM_LOOKINGFOR_SUB), 31);
  else
    SVP_LogMsg(ResStr(IDS_LOG_MSG_USING_SVPSUB_SYSTEM_LOOKINGFOR_ENGSUB), 31);
  CSVPNet svpNet;
  CSVPhash svpHash;
  std::wstring szFileHash  = svpHash.ComputerFileHash_STL(fnVideoFilePath);
  SVP_LogMsg5(L"FileHash %s for %s ", szFileHash.c_str(),
    fnVideoFilePath.c_str());

  for (int i = 1; i <= 7; i++)
  {
    svpNet.iTryID = i;
    int err = svpNet.QuerySubByVideoPathOrHash_STL(
      fnVideoFilePath, szFileHash, L"", szLang);
    if (err)
    {
      Sleep(2300);
      SVP_LogMsg(ResStr(IDS_LOG_MSG_USING_SVPSUB_SYSTEM_RETRYING));//, 31
    }
    else
      break;
  }

  //load sub file to sublist
  std::wstring szSubFilePath;
  int iSubTotal = 0;
  for (int i = 0; i < svpNet.svpToolBox.szaSubTmpFileList.GetCount(); i++)
  {
    szSubFilePath = svpNet.svpToolBox.getSubFileByTempid_STL(i,
      fnVideoFilePath);
    if (szSubFilePath == L"EXIST")
      SVP_LogMsg(ResStr(IDS_LOG_MSG_SVPSUB_SAMEAS_CURRENT), 31);
    else if (!szSubFilePath.empty())
    {
      szSubArray -> push_back(szSubFilePath);
      CPath fnPath(szSubFilePath.c_str());
      fnPath.StripPath();
      SVP_LogMsg(CString(ResStr(IDS_LOG_MSG_SVPSUB_SUCCESS_DOWNLOAD_SUB))
        + (CString)fnPath , 31 ); //TODO: if its vobsub, load perfered language
      iSubTotal++;
    }
    else
    {
      SVP_LogMsg(svpNet.m_lastFailedMsg, 31);
      SVP_LogMsg(_T("Fail to get sub file name"));
    }
  }

  if (iSubTotal)
  {
    wchar_t szLog[128];
    swprintf_s(szLog, 128,
      ResStr(IDS_LOG_MSG_SVPSUB_GOT_N_SUB_FILE_DOWNLOADED), iSubTotal);
    SVP_LogMsg(szLog, 31); 
  }

  if (iSubTotal > 1)
  {
    std::wstring szSVPSubPerf = (LPCTSTR)s.szSVPSubPerf;
    //SVP_LogMsg5(_T("SpecFanSub %s"), szSVPSubPerf);
    if (!szSVPSubPerf.empty())
    {
      //for(int i = 0; i < szSubArray -> size(); i++)
      //{
      //  std::wstring subFilePath = szSubArray -> at(i);
      //  if(isSpecFanSub(subFilePath.c_str() , szSVPSubPerf.c_str()))
      //  {
      //    CString szFirst =  szSubArray->GetAt(0);
      //    szSubArray->SetAt(0, subFilePath);
      //    szSubArray->SetAt(i, szFirst);
      //    break;
      //  }
      //}
      for (std::vector<std::wstring>::iterator iter = szSubArray -> begin();
        iter != szSubArray -> end(); iter++)
        if (isSpecFanSub((*iter).c_str(), szSVPSubPerf.c_str()))
        {
          std::wstring szFirst = szSubArray -> at(0);
          (*szSubArray)[0] = szSubFilePath;
          *iter = szFirst;
          break;
        }
    }
  }
  szGStatMsg = NULL;
  return;
}

void SVP_LogMsg(CString logmsg, int level){

	if(szGStatMsg && (level & 16) ){
		szGStatMsg->AddTail((LPCTSTR)logmsg);
	}
	ULONGLONG logTick2 = SVPGetPerfCounter();
	CStdioFile f;
	CSVPToolBox svpToolBox;
	CString szLogPath = svpToolBox.GetPlayerPath_STL(_T("SVPDebug.log")).c_str();
	 /* must use Binary mode because its saved in unicode */
	if(f.Open(szLogPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate | CFile::typeBinary))
	{
        if (0 == f.SeekToEnd() ){
            char uft16mark[2] = {0xff, 0xfe};
            f.Write(uft16mark , 2);
        }
		CString szLog;
		szLog.Format(_T("[%f] %s") , (double)logTick2/10000, logmsg);
		f.WriteString(szLog + _T("\r\n"));
		
		f.Flush();
		f.Close();
	}
		
}
void SVP_LogMsg2(LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	CSVPToolBox svpToolBox;
	CString szLogPath = svpToolBox.GetPlayerPath_STL(_T("SVPDebug.log")).c_str();
	if(TCHAR* buff = new TCHAR[_vsctprintf(fmt, args) + 1])
	{
		_vstprintf(buff, fmt, args);
		if(FILE* f = _tfopen(szLogPath, _T("at")))
		{
			fseek(f, 0, 2);
			_ftprintf(f, _T("%s\n"), buff);
			fclose(f);
		}
		delete [] buff;
	}
	va_end(args);
}
void SVP_LogMsg3(LPCSTR fmt, ...)
{
	ULONGLONG logTick2 = SVPGetPerfCounter();
	va_list args;
	va_start(args, fmt);
	CSVPToolBox svpToolBox;
	CString szLogPath = svpToolBox.GetPlayerPath_STL(_T("SVPDebug2.log")).c_str();
	if(CHAR* buff = new CHAR[_vscprintf(fmt, args) + 1])
	{
		vsprintf(buff, fmt, args);
		if(FILE* f = _tfopen(szLogPath, _T("at")))
		{
			fseek(f, 0, 2);
			_ftprintf(f, _T("[%f] %s\n"), (double)logTick2/10000, CA2T(buff));
			fclose(f);
		}
		delete [] buff;
	}
	va_end(args);
}
void SVP_LogMsg5(LPCTSTR fmt, ...)
{
	wchar_t buf[2048];
	va_list args;
	va_start(args, fmt);
	//CSVPToolBox svpToolBox;
	//CString szLogPath = svpToolBox.GetPlayerPath_STL(_T("SVPDebug2.log"));
	vswprintf(buf,fmt,args);
	SVP_LogMsg(buf);
	va_end(args);
}
void SVP_LogMsg6(LPCSTR fmt, ...)
{
	char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf,fmt,args);
	SVP_LogMsg(CStringW(CStringA(buf)));
	va_end(args);
}
void SVP_LogMsg4(BYTE* buff, __int64 iLen)
{
	
	CSVPToolBox svpToolBox;
	CString szLogPath = svpToolBox.GetPlayerPath_STL(_T("SVPDebug4.log")).c_str();

	
		if(FILE* f = _tfopen(szLogPath, _T("at")))
		{
			fseek(f, 0, 2);
			fwrite( buff , sizeof(BYTE), iLen , f);
			fclose(f);
		}
	
	
}
/*
BOOL SVP_ForbidenCoreAVCTrayIcon(){
    return false;
    //作废

	HRESULT hr;
	LPITEMIDLIST pidl;
	hr = SHGetSpecialFolderLocation( NULL, CSIDL_APPDATA, &pidl);
	if (hr)
		return false;

	TCHAR szPath[MAX_PATH];
	BOOL f = SHGetPathFromIDList(pidl, szPath);
	PathAddBackslash(szPath); 
	PathAppend(szPath, _T("coreavc.ini"));

	FILE* fileHandle;
	errno_t reterrno = _wfopen_s( &fileHandle, szPath , _T("r+") );
	if(reterrno == 0){
		char szStr[28093];
		int iRead = fread_s(szStr, 28093, sizeof( char ), 28093, fileHandle);
		CStringA szBuf(szStr , iRead) ;

		if(szBuf.IsEmpty()){
			szBuf =  "[CoreAVC]\r\n";
			
			szBuf += "use_tray=0";
			
		}else{
			szBuf.Replace(CStringA("use_tray=1") , CStringA("use_tray=0"));
			
		}

		szBuf.Trim();
		fseek( fileHandle , SEEK_SET , SEEK_SET);
		fwrite(szBuf,sizeof( char ),szBuf.GetLength(), fileHandle ) ;
		SetEndOfFile(fileHandle);
		fclose(fileHandle);

	}

	return true;
}

BOOL SVP_CanUseCoreAvcCUDA(BOOL useCUDA){
    return false;
    //作废
	HRESULT hr;
	LPITEMIDLIST pidl;
	hr = SHGetSpecialFolderLocation( NULL, CSIDL_APPDATA, &pidl);
	if (hr)
		return false;
	

	TCHAR szPath[MAX_PATH];
	BOOL f = SHGetPathFromIDList(pidl, szPath);
	PathAddBackslash(szPath); 
	PathAppend(szPath, _T("coreavc.ini"));

	CSVPToolBox svpTool;
	
		//TODO: check VGA card and driver version
		//$mooi(vcardmake) $mooi(vcardproc)   $mooi(vcarddriver)
		//if ($1 == vcardmake) { return $wmiget(Win32_VideoController).AdapterCompatibility }
		//if ($1 == vcardproc) { return $wmiget(Win32_VideoController).VideoProcessor }
		//if ($1 == vcarddriver) { return $wmiget(Win32_VideoController).DriverVersion }

		if(useCUDA){
			useCUDA = svpTool.CanUseCUDAforCoreAVC();
		}

		FILE*   fileHandle = _wfopen(  szPath , _T("r+") );
		if(fileHandle){
			char szStr[28093];
			int iRead = fread_s(szStr, 28093, sizeof( char ), 28093, fileHandle);
			CStringA szBuf(szStr , iRead) ;

			if(1 || szBuf.IsEmpty()){
				szBuf =  "[CoreAVC]\r\n";
				if(useCUDA){
					szBuf += "Settings=use_cuda=1 use_tray=0";
				}else{
					szBuf += "Settings=use_cuda=-1 use_tray=0";
				}
			}else{
				if(useCUDA){
					if (!szBuf.Replace(CStringA("use_cuda=-1") , CStringA("use_cuda=1 ")) && szBuf.Find(CStringA("use_cuda=1")) < 0 ){
						if (!szBuf.Replace(CStringA("Settings=") , CStringA("Settings=use_cuda=1 "))){
							szBuf += CStringA(" use_cuda=1");
						}
					}
				}else{
					szBuf.Replace(CStringA("use_cuda=1") , CStringA("use_cuda=-1"));
				}
			}
			fclose(fileHandle);
			_wunlink(szPath);
			//szBuf.Append(" \0");
			//szBuf.Trim();
			 fileHandle = _wfopen(  szPath , _T("w") );
			if(fileHandle){
				fwrite(szBuf,sizeof( char ),szBuf.GetLength(), fileHandle ) ;
				fclose(fileHandle);
			}
		}

		
	
	return useCUDA;
}
*/

BOOL SVP_CanUseCoreAvcCUDA(BOOL useCUDA)
{
    CSVPToolBox svpTool;
    if(useCUDA){
        useCUDA = svpTool.CanUseCUDAforCoreAVC();
    }

    return useCUDA;

}
