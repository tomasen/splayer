#include "svplib.h"
#include "SVPNet.h"
#include "SVPHash.h"
#include <stdio.h> 
#include <shlobj.h>
//#include "../apps/mplayerc/mplayerc.h"

static CAtlList<CString> * szGStatMsg = NULL;

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

void SVP_RealUploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath, CString szSubPath, int iDelayMS, CStringArray* szaPostTerms){
		
	CSVPNet svpNet;
	CSVPhash svpHash;
	CSVPToolBox svpToolBox;
	CString szFileHash  = svpHash.ComputerFileHash(fnVideoFilePath);

	CStringArray szaSubFiles;
	svpToolBox.FindAllSubfile(szSubPath, &szaSubFiles);

	CString szSubHash = svpHash.ComputerSubFilesFileHash(&szaSubFiles);
	if(szSubHash.IsEmpty())
		return;
	
	SVP_LogMsg(CString("Got Sub Hash ") + svpToolBox.Implode( _T(" | "), &szaSubFiles) + _T(" -> ") + szSubHash );

	if ( svpNet.WetherNeedUploadSub(fnVideoFilePath,szFileHash, szSubHash,iDelayMS) ){
		svpNet.UploadSubFileByVideoAndHash(fnVideoFilePath,szFileHash, szSubHash, &szaSubFiles,iDelayMS, szaPostTerms);
		return ;
	}
}

void SVP_RealCheckUpdaterExe(BOOL* bCheckingUpdater, UINT verbose ){

	
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
		SVP_LogMsg( _T("检测到新的版本，升级程序已启动") ); 
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
			AfxMessageBox(_T("目录无法写入，升级程序暂停"));
		}
	}
	*bCheckingUpdater = false;
}
class CCheckUpdaterPerm{
public:
	BOOL* bCheckingUpdater;
	UINT verbose;
};

UINT __cdecl SVPThreadCheckUpdaterExe( LPVOID lpParam ) 
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
void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath, CStringArray* szSubArray, CAtlList<CString> * szStatMsg, CString szLang ){
		
	szGStatMsg = szStatMsg;
	if(szLang.IsEmpty())
		SVP_LogMsg(_T("正在通过射手影音字幕智能匹配系统寻找中文字幕"), 31);
	else
		SVP_LogMsg(_T("正在通过射手影音字幕智能匹配系统寻找英文字幕"), 31);
	CSVPNet svpNet;
	CSVPhash svpHash;
	CString szFileHash  = svpHash.ComputerFileHash(fnVideoFilePath);
	for(int i = 1; i < 8; i++){
		int err =  svpNet.QuerySubByVideoPathOrHash(fnVideoFilePath,szFileHash, _T("") , szLang) ;
		if(err){
			Sleep(2300 * i);
			if(i >= 3)
				return;

			SVP_LogMsg(_T("正在重试..."), 31);
		}else{
			break;
		}
	}
	
		//load sub file to sublist
		CString szSubFilePath;
		int iSubTotal = 0;
		for(int i = 0; i < svpNet.svpToolBox.szaSubTmpFileList.GetCount(); i++){
			szSubFilePath = svpNet.svpToolBox.getSubFileByTempid(i, fnVideoFilePath);
			if(szSubFilePath == _T("EXIST")){
				SVP_LogMsg(_T("射手影音字幕智能匹配系统中的字幕与当前字幕相同 "), 31);
			}else if(szSubFilePath){
				szSubArray->Add(szSubFilePath);
				CPath fnPath(szSubFilePath);
				fnPath.StripPath();
				SVP_LogMsg(CString(_T("成功下载到字幕文件 ")) + (CString)fnPath , 31 ); //TODO: if its vobsub, load perfered language
				iSubTotal++;
			}else{
				SVP_LogMsg(_T("Fail to get sub file name"));
			}
		}

	if(iSubTotal) {
		CString szLog;
		szLog.Format(_T("通过射手影音字幕智能匹配系统成功下载到 %d 个字幕文件 "), iSubTotal);
		SVP_LogMsg( szLog, 31 ); 
	}

	szGStatMsg = NULL;
	return;
}


void SVP_LogMsg(CString logmsg, int level){

	if(szGStatMsg && (level & 16) ){
		szGStatMsg->AddTail(logmsg);
	}
	ULONGLONG logTick2 = SVPGetPerfCounter();
	CStdioFile f;
	CSVPToolBox svpToolBox;
	CString szLogPath = svpToolBox.GetPlayerPath(_T("SVPDebug.log"));
	if(f.Open(szLogPath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate | CFile::typeBinary))
	{
		f.SeekToEnd();
		CString szLog;
		szLog.Format(_T("[%ul] %s") , (UINT)logTick2, logmsg);
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
	CString szLogPath = svpToolBox.GetPlayerPath(_T("SVPDebug.log"));
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
	CString szLogPath = svpToolBox.GetPlayerPath(_T("SVPDebug2.log"));
	if(CHAR* buff = new CHAR[_vscprintf(fmt, args) + 1])
	{
		vsprintf(buff, fmt, args);
		if(FILE* f = _tfopen(szLogPath, _T("at")))
		{
			fseek(f, 0, 2);
			_ftprintf(f, _T("[%ull] %s\n"), (UINT)logTick2, CA2T(buff));
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
	//CString szLogPath = svpToolBox.GetPlayerPath(_T("SVPDebug2.log"));
	vswprintf(buf,fmt,args);
	SVP_LogMsg(buf);
	va_end(args);
}

void SVP_LogMsg4(BYTE* buff, __int64 iLen)
{
	
	CSVPToolBox svpToolBox;
	CString szLogPath = svpToolBox.GetPlayerPath(_T("SVPDebug4.log"));

	
		if(FILE* f = _tfopen(szLogPath, _T("at")))
		{
			fseek(f, 0, 2);
			fwrite( buff , sizeof(BYTE), iLen , f);
			fclose(f);
		}
	
	
}
BOOL SVP_ForbidenCoreAVCTrayIcon(){
	HRESULT hr;
	LPITEMIDLIST pidl;
	hr = SHGetSpecialFolderLocation( NULL, CSIDL_APPDATA, &pidl);
	if (hr)
		return false;

	TCHAR szPath[MAX_PATH];
	BOOL f = SHGetPathFromIDList(pidl, szPath);
	PathAddBackslash(szPath); 
	PathAppend(szPath, _T("coreavc.ini"));

	FILE*   fileHandle = _wfopen(  szPath , _T("r+") );
	if(fileHandle){
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
BOOL SVP_SetCoreAvcCUDA(BOOL useCUDA){
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

			if(szBuf.IsEmpty()){
				szBuf =  "[CoreAVC]\r\n";
				if(useCUDA){
					szBuf += "use_cuda=1";
				}else{
					szBuf += "use_cuda=-1";
				}
			}else{
				if(useCUDA){
					if (!szBuf.Replace(CStringA("use_cuda=-1") , CStringA("use_cuda=1 ")) && szBuf.Find(CStringA("use_cuda=1")) < 0 ){
						szBuf += CStringA(" use_cuda=1");
					}
				}else{
					szBuf.Replace(CStringA("use_cuda=1") , CStringA("use_cuda=-1"));
				}
			}

			//szBuf.Trim();
			szBuf.Append(" \0");
			fseek( fileHandle , SEEK_SET , SEEK_SET);
			fwrite(szBuf,sizeof( char ),szBuf.GetLength(), fileHandle ) ;
			SetEndOfFile(fileHandle);
			fclose(fileHandle);
		
		}

		
	
	return useCUDA;
}

