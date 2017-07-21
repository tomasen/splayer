// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "sal.h"



/////////////////////////////////////////////////////////////////////////////

BOOL AFXAPI AfxWinInit(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance,
	_In_z_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	ASSERT(hPrevInstance == NULL);


	// handle critical errors and avoid Windows message boxes
	SetErrorMode(SetErrorMode(0) |
		SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

	// set resource handles
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	pModuleState->m_hCurrentInstanceHandle = hInstance;
	pModuleState->m_hCurrentResourceHandle = hInstance;
	pModuleState->CreateActivationContext();

	// fill in the initial state for the application
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL)
	{
		// Windows specific initialization (not done if no CWinApp)
		pApp->m_hInstance = hInstance;
		hPrevInstance; // Obsolete.
		pApp->m_lpCmdLine = lpCmdLine;
		pApp->m_nCmdShow = nCmdShow;
		pApp->SetCurrentHandles();
	}

	// initialize thread specific data (for main thread)
	if (!afxContextIsDLL)
		AfxInitThread();

	// Initialize CWnd::m_pfnNotifyWinEvent
	HMODULE hModule = ::GetModuleHandle(_T("user32.dll"));
	if (hModule != NULL)
	{
		CWnd::m_pfnNotifyWinEvent = (CWnd::PFNNOTIFYWINEVENT)::GetProcAddress(hModule, "NotifyWinEvent");
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// CWinApp Initialization

void CWinApp::SetCurrentHandles()
{
	ASSERT(this == afxCurrentWinApp);
	ASSERT(afxCurrentAppName == NULL);

	AFX_MODULE_STATE* pModuleState = _AFX_CMDTARGET_GETSTATE();
	pModuleState->m_hCurrentInstanceHandle = m_hInstance;
	pModuleState->m_hCurrentResourceHandle = m_hInstance;

	// Note: there are a number of _tcsdup (aka strdup) calls that are
	// made here for the exe path, help file path, etc.  In previous
	// versions of MFC, this memory was never freed.  In this and future
	// versions this memory is automatically freed during CWinApp's
	// destructor.  If you are freeing the memory yourself, you should
	// either remove the code or set the pointers to NULL after freeing
	// the memory.

	// get path of executable
	TCHAR szBuff[_MAX_PATH];
	DWORD dwRet = ::GetModuleFileName(m_hInstance, szBuff, _MAX_PATH);
	ASSERT( dwRet != 0 && dwRet != _MAX_PATH );
	if( dwRet == 0 || dwRet == _MAX_PATH )
		AfxThrowUserException();

	LPTSTR lpszExt = ::PathFindExtension(szBuff);
	ASSERT(lpszExt != NULL);
	if( lpszExt == NULL )
		AfxThrowUserException();

	ASSERT(*lpszExt == '.');
	*lpszExt = 0;       // no suffix

	TCHAR szExeName[_MAX_PATH];
	TCHAR szTitle[256];
	// get the exe title from the full path name [no extension]
	dwRet = AfxGetFileName(szBuff, szExeName, _MAX_PATH);
	ASSERT( dwRet == 0 );
	if( dwRet != 0 )
		AfxThrowUserException();

	if (m_pszExeName == NULL)
	{
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
		m_pszExeName = _tcsdup(szExeName); // save non-localized name
		AfxEnableMemoryTracking(bEnable);
		if(!m_pszExeName)
		{
			AfxThrowMemoryException();
		}
	}

	// m_pszAppName is the name used to present to the user
	if (m_pszAppName == NULL)
	{
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
		if (AfxLoadString(AFX_IDS_APP_TITLE, szTitle) != 0)
		{
			m_pszAppName = _tcsdup(szTitle);    // human readable title
		}
		else
		{
			m_pszAppName = _tcsdup(m_pszExeName);   // same as EXE
		}
		AfxEnableMemoryTracking(bEnable);
		if(!m_pszAppName)
		{
			AfxThrowMemoryException();
		}
	}

	pModuleState->m_lpszCurrentAppName = m_pszAppName;
	ASSERT(afxCurrentAppName != NULL);

	// get path of .HLP file or .CHM (HtmlHelp) file
	if (m_pszHelpFilePath == NULL)
	{
		if (m_eHelpType == afxHTMLHelp)
			Checked::tcscpy_s(lpszExt, _countof(szBuff) - (lpszExt - szBuff), _T(".CHM"));
		else
			Checked::tcscpy_s(lpszExt, _countof(szBuff) - (lpszExt - szBuff), _T(".HLP"));
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
		m_pszHelpFilePath = _tcsdup(szBuff);
		AfxEnableMemoryTracking(bEnable);
		if(!m_pszHelpFilePath)
		{
			AfxThrowMemoryException();
		}
		*lpszExt = '\0';       // back to no suffix
	}

	if (m_pszProfileName == NULL)
	{
		Checked::tcscat_s(szExeName, _countof(szExeName), _T(".INI")); // will be enough room in buffer
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
		m_pszProfileName = _tcsdup(szExeName);
		AfxEnableMemoryTracking(bEnable);
		if(!m_pszProfileName)
		{
			AfxThrowMemoryException();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFile implementation helpers

#ifdef AfxGetFileName
#undef AfxGetFileName
#endif

UINT AFXAPI AfxGetFileName(LPCTSTR lpszPathName, _Out_opt_cap_(nMax) LPTSTR lpszTitle, UINT nMax)
{
	ASSERT(lpszTitle == NULL ||
		AfxIsValidAddress(lpszTitle, nMax));
	ASSERT(AfxIsValidString(lpszPathName));

	ENSURE_ARG(lpszPathName != NULL);

	// always capture the complete file name including extension (if present)
	LPTSTR lpszTemp = ::PathFindFileName(lpszPathName);

	// lpszTitle can be NULL which just returns the number of bytes
	if (lpszTitle == NULL)
		return lstrlen(lpszTemp)+1;

	// otherwise copy it into the buffer provided
	Checked::tcsncpy_s(lpszTitle, nMax, lpszTemp, _TRUNCATE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

#pragma init_seg( lib )

#ifdef _DEBUG
ATL::CTraceCategory traceMultiApp(_T("MultiApp"));
ATL::CTraceCategory traceAppMsg(_T("AppMsg"));
ATL::CTraceCategory traceWinMsg(_T("WinMsg"));
ATL::CTraceCategory traceCmdRouting(_T("CmdRouting"));
ATL::CTraceCategory traceOle(_T("Ole"));
ATL::CTraceCategory traceDatabase(_T("Database"));
ATL::CTraceCategory traceInternet(_T("Internet"));
ATL::CTraceCategory traceDumpContext(_T("CDumpContext"));
ATL::CTraceCategory traceMemory(_T("Memory"));
ATL::CTraceCategory traceGdi(_T("GDI"));
ATL::CTraceCategory traceUser(_T("User"));
ATL::CTraceCategory traceKernel(_T("Kernel"));
ATL::CTraceCategory traceHtml(_T("HTML"));
ATL::CTraceCategory traceSocket(_T("Socket"));
#endif // _DEBUG

