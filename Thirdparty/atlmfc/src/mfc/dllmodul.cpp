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

__declspec(selectany) int __mixedModuleStartup;

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// global data

// The following symbol used to force inclusion of this module for _USRDLL
#ifdef _X86_
extern "C" { int _afxForceUSRDLL; }
#else
extern "C" { int __afxForceUSRDLL; }
#endif

#ifdef _AFXDLL

static AFX_EXTENSION_MODULE controlDLL;

// force initialization early
#pragma warning(disable: 4074)
#pragma init_seg(lib)

/////////////////////////////////////////////////////////////////////////////
// static-linked version of AfxWndProc for use by this module

LRESULT CALLBACK AfxWndProcDllStatic(HWND, UINT, WPARAM, LPARAM);

class _AFX_DLL_MODULE_STATE : public AFX_MODULE_STATE
{
public:
	_AFX_DLL_MODULE_STATE() : AFX_MODULE_STATE(TRUE, AfxWndProcDllStatic, _MFC_VER)
		{ }
};

static _AFX_DLL_MODULE_STATE afxModuleState;

#undef AfxWndProc
LRESULT CALLBACK
AfxWndProcDllStatic(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(&afxModuleState);
	return AfxWndProc(hWnd, nMsg, wParam, lParam);
}

AFX_MODULE_STATE* AFXAPI AfxGetStaticModuleState()
{
	AFX_MODULE_STATE* pModuleState = &afxModuleState;
	return pModuleState;
}

#else

AFX_MODULE_STATE* AFXAPI AfxGetStaticModuleState()
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	return pModuleState;
}

#endif

/////////////////////////////////////////////////////////////////////////////
// export DllMain for the DLL

extern "C"
BOOL WINAPI InternalDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		BOOL bResult = FALSE;

#ifdef _AFXDLL
		// wire up resources from core DLL
		AfxCoreInitModule();
#endif

		_AFX_THREAD_STATE* pState = AfxGetThreadState();
		AFX_MODULE_STATE* pPrevModState = pState->m_pPrevModuleState;

		// Initialize DLL's instance(/module) not the app's
		if (!AfxWinInit(hInstance, NULL, _T(""), 0))
		{
			AfxWinTerm();
			goto Cleanup;       // Init Failed
		}

		// initialize the single instance DLL
		CWinApp* pApp; pApp = AfxGetApp();
		if (pApp != NULL && !pApp->InitInstance())
		{
			pApp->ExitInstance();
			AfxWinTerm();
			goto Cleanup;       // Init Failed
		}

		pState->m_pPrevModuleState = pPrevModState;
#ifdef _AFXDLL
		// wire up this DLL into the resource chain
		VERIFY(AfxInitExtensionModule(controlDLL, hInstance));
		TRY
		{
			CDynLinkLibrary* pDLL; 
			pDLL = new CDynLinkLibrary(controlDLL);
		}
		CATCH(CMemoryException, e)
		{
			DELETE_EXCEPTION(e);
			pApp->ExitInstance();
			AfxWinTerm();
			goto Cleanup;       // Init Failed
		}
		END_CATCH
#else
		AfxInitLocalData(hInstance);
#endif

		bResult = TRUE;

Cleanup:
		pState->m_pPrevModuleState = pPrevModState;
#ifdef _AFXDLL
		// restore previously-saved module state
		VERIFY(AfxSetModuleState(AfxGetThreadState()->m_pPrevModuleState) ==
			&afxModuleState);
		DEBUG_ONLY(AfxGetThreadState()->m_pPrevModuleState = NULL);
#endif
		return bResult;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
#ifdef _AFXDLL
		// set module state for cleanup
		ASSERT(AfxGetThreadState()->m_pPrevModuleState == NULL);
		AfxGetThreadState()->m_pPrevModuleState =
			AfxSetModuleState(&afxModuleState);
#endif

		CWinApp* pApp = AfxGetApp();
		if (pApp != NULL)
			pApp->ExitInstance();

#ifdef _DEBUG
		// check for missing AfxLockTempMap calls
		if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
		{
			TRACE(traceAppMsg, 0, "Warning: Temp map lock count non-zero (%ld).\n",
				AfxGetModuleThreadState()->m_nTempMapLock);
		}
#endif
		AfxLockTempMaps();
		AfxUnlockTempMaps(-1);

		// terminate the library before destructors are called
		AfxWinTerm();

#ifdef _AFXDLL
		AfxTermExtensionModule(controlDLL, TRUE);
		if (__mixedModuleStartup)
		{
			// restore module state after cleanup
			_AFX_THREAD_STATE* pState = AfxGetThreadState();
			VERIFY(AfxSetModuleState(pState->m_pPrevModuleState) ==
				&afxModuleState);
			DEBUG_ONLY(pState->m_pPrevModuleState = NULL);
		}
#else
		AfxTermLocalData(hInstance, TRUE);
#endif
	}
	else if (dwReason == DLL_THREAD_DETACH)
	{
#ifdef _AFXDLL
		AFX_MANAGE_STATE_NO_INIT_MANAGED(&afxModuleState);
#endif

#ifdef _DEBUG
		// check for missing AfxLockTempMap calls
		if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
		{
			TRACE(traceAppMsg, 0, "Warning: Temp map lock count non-zero (%ld).\n",
				AfxGetModuleThreadState()->m_nTempMapLock);
		}
#endif
		AfxLockTempMaps();
		AfxUnlockTempMaps(-1);

		AfxTermThread(hInstance);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// initialize app state such that it points to this module's core state

extern "C" BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID);
extern "C" BOOL (WINAPI * const _pActualRawDllMain)(HINSTANCE , DWORD , LPVOID) = &RawDllMain;

extern "C"
BOOL WINAPI RawDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	hInstance;
	if (dwReason == DLL_PROCESS_ATTACH)
	{
#ifdef _AFXDLL
		// make sure we have enough memory to attempt to start (8kb)
		void* pMinHeap = LocalAlloc(NONZEROLPTR, 0x2000);
		if (pMinHeap == NULL)
			return FALSE;   // fail if memory alloc fails
		LocalFree(pMinHeap);

		// set module state before initialization
		_AFX_THREAD_STATE* pState = AfxGetThreadState();
		pState->m_pPrevModuleState = AfxSetModuleState(&afxModuleState);
	}
	else if (dwReason == DLL_PROCESS_DETACH && !__mixedModuleStartup)
	{
		// restore module state after cleanup
		_AFX_THREAD_STATE* pState = AfxGetThreadState();
		VERIFY(AfxSetModuleState(pState->m_pPrevModuleState) ==
			&afxModuleState);
		DEBUG_ONLY(pState->m_pPrevModuleState = NULL);

#endif //_AFXDLL
	}
	return TRUE;
}

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (__mixedModuleStartup)
	{
		// Skip initialization in mixed mode (done in PostDllMain, see afxdisp.h)
		if (dwReason == DLL_PROCESS_ATTACH)
		{
			// save the module instance
			AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
			pModuleState->m_hCurrentInstanceHandle = hInstance;
#ifdef _AFXDLL
			// restore previously-saved module state
			VERIFY(AfxSetModuleState(AfxGetThreadState()->m_pPrevModuleState) ==
				&afxModuleState);
			DEBUG_ONLY(AfxGetThreadState()->m_pPrevModuleState = NULL);
#endif
			return TRUE;
		}
		else if (dwReason == DLL_PROCESS_DETACH)
		{
			return TRUE;
		}
		else
		{
			return InternalDllMain(hInstance, dwReason, lpReserved);
		}
	}
	else
	{
		return InternalDllMain(hInstance, dwReason, lpReserved);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Special case for static library startup/termination

#ifndef _AFXDLL

// force initialization early
#pragma warning(disable: 4074)
#pragma init_seg(lib)

void AFX_CDECL AfxTermDllState()
{
	// terminate local data and critical sections
	AfxTermLocalData(NULL, TRUE);
	AfxCriticalTerm();

	// release the reference to thread local storage data
	AfxTlsRelease();
}

char _afxTermDllState = (char)(AfxTlsAddRef(), atexit(&AfxTermDllState));

#endif // !_AFXDLL

/////////////////////////////////////////////////////////////////////////////
