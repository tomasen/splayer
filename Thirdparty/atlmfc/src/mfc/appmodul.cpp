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

#ifdef _AFXDLL
	#if defined(_M_IX86)
		#pragma comment(linker, "/include:__forceMFCManifestRTM")
	#else
		#pragma comment(linker, "/include:_forceMFCManifestRTM")
	#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// export WinMain to force linkage to this module
extern int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	_In_ LPTSTR lpCmdLine, int nCmdShow);

extern "C" int WINAPI
_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	_In_ LPTSTR lpCmdLine, int nCmdShow)
#pragma warning(suppress: 4985)
{
	// call shared/exported WinMain
	return AfxWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

/////////////////////////////////////////////////////////////////////////////
// initialize app state such that it points to this module's core state

BOOL AFXAPI AfxInitialize(BOOL bDLL, DWORD dwVersion)
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	pModuleState->m_bDLL = (BYTE)bDLL;
	ASSERT(dwVersion <= _MFC_VER);
	UNUSED(dwVersion);  // not used in release build
#ifdef _AFXDLL
	pModuleState->m_dwVersion = dwVersion;
#endif
#ifdef _MBCS
	// set correct multi-byte code-page for Win32 apps
	if (!bDLL)
		_setmbcp(_MB_CP_ANSI);
#endif //_MBCS
	return TRUE;
}

// force initialization early
#pragma warning(disable: 4074)
#pragma init_seg(lib)

#ifndef _AFXDLL
void AFX_CDECL _AfxTermAppState()
{
	// terminate local data and critical sections
	AfxTermLocalData(NULL, TRUE);
	AfxCriticalTerm();

	// release the reference to thread local storage data
	AfxTlsRelease();
}
#endif

#ifndef _AFXDLL
char _afxInitAppState = (char)(AfxInitialize(FALSE, _MFC_VER), atexit(&_AfxTermAppState));
#else
char _afxInitAppState = (char)(AfxInitialize(FALSE, _MFC_VER));
#endif

/////////////////////////////////////////////////////////////////////////////
