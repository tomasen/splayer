
#pragma once
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components


#define SVP_DEBUG_LOGFILEPATH _T("D:\\-=SVN=-\\SVPDebug.log")

extern void SVP_FetchSubFileByVideoFilePath(CString fnVideoFilePath);

extern void SVP_LogMsg(CString logmsg);


