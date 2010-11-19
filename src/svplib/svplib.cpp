#include "svplib.h"
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


class CCheckUpdaterPerm{
public:
	BOOL* bCheckingUpdater;
	UINT verbose;
};


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
