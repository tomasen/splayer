
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

#include <string>
#include <vector>

#define SVP_MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define SVP_MAX(a, b)  (((a) > (b)) ? (a) : (b)) 

//#define SVP_DEBUG_LOGFILEPATH _T(".\\SVPDebug.log")
void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath,
                                     CStringArray* szSubArray,
                                     CAtlList<CString> * szStatMsg,
                                     CString szLang = _T(""));
void FetchSubFileByVideoFilePath_STL(std::wstring fnVideoFilePath,
                                     std::vector<std::wstring>* szSubArray,
                                     CAtlList<std::wstring> * szStatMsg,
                                     std::wstring szLang = L"");
//extern void SVP_UploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath, CString szSubPath, int iDelayMS );
void SVP_RealUploadSubFileByVideoAndSubFilePath(CString fnVideoFilePath,
                                                CString szSubPath,
                                                int iDelayMS,
                                                CStringArray* szaPostTerms);
void RealUploadSubFileByVideoAndSubFilePath_STL(std::wstring fnVideoFilePath,
                                      std::wstring szSubPath,
                                      int iDelayMS,
                                      std::vector<std::wstring>* szaPostTerms);

void SVP_LogMsg(CString logmsg, int level = 15);
void SVP_RealCheckUpdaterExe(BOOL* bCheckingUpdater, UINT verbose = 0);
void SVP_CheckUpdaterExe(BOOL* bCheckingUpdater, UINT verbose = 0);
BOOL SVP_CanUseCoreAvcCUDA(BOOL useCUDA);
//extern void SVP_RealCheckUpdaterExe(BOOL* bCheckingUpdater);
//extern BOOL SVP_SetCoreAvcCUDA(BOOL useCUDA);
//extern BOOL SVP_ForbidenCoreAVCTrayIcon();
void SVP_UploadPinRenderDeadEnd(CString szPinName, CString szReport);
void SVP_UploadCrashDmp(CString szDmppath, CString szLogPath);
void SVP_LogMsg2(LPCTSTR fmt, ...);
void SVP_LogMsg3(LPCSTR fmt, ...);
void SVP_LogMsg4(BYTE* buff, __int64 iLen);
void SVP_LogMsg5(LPCTSTR fmt, ...);
void SVP_LogMsg6(LPCSTR fmt, ...);
static UINT logTick = 0;