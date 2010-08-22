
#pragma once
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <atlcoll.h>

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>

#define SVP_MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define SVP_MAX(a, b)  (((a) > (b)) ? (a) : (b)) 

//#define SVP_DEBUG_LOGFILEPATH _T(".\\SVPDebug.log")

extern void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath, CStringArray* szSubArray, CAtlList<CString> * szStatMsg , CString szLang = _T("") );
//extern void SVP_UploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath, CString szSubPath, int iDelayMS );
extern void SVP_RealUploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath, CString szSubPath, int iDelayMS, CStringArray* szaPostTerms);
extern void SVP_LogMsg(CString logmsg, int level = 15);
extern void SVP_RealCheckUpdaterExe(BOOL* bCheckingUpdater, UINT verbose = 0);
extern void SVP_CheckUpdaterExe(BOOL* bCheckingUpdater, UINT verbose = 0);
extern BOOL SVP_CanUseCoreAvcCUDA(BOOL useCUDA);
//extern void SVP_RealCheckUpdaterExe(BOOL* bCheckingUpdater);
//extern BOOL SVP_SetCoreAvcCUDA(BOOL useCUDA);
//extern BOOL SVP_ForbidenCoreAVCTrayIcon();
extern void SVP_UploadPinRenderDeadEnd(CString szPinName, CString szReport);
extern void SVP_UploadCrashDmp(CString szDmppath, CString szLogPath);
extern void SVP_LogMsg2(LPCTSTR fmt, ...);
extern void SVP_LogMsg3(LPCSTR fmt, ...);
extern void SVP_LogMsg4(BYTE* buff, __int64 iLen);
extern void SVP_LogMsg5(LPCTSTR fmt, ...);
extern void SVP_LogMsg6(LPCSTR fmt, ...);
static UINT logTick = 0;