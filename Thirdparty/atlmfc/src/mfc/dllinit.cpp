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
#include <stdarg.h>
#include "atlbuild.h"
#include "sal.h"



#ifndef _AFXDLL
	#error file must be compiled with _AFXDLL
#endif

/////////////////////////////////////////////////////////////////////////////
// _AFXDLL support

static AFX_EXTENSION_MODULE coreDLL;
#ifdef _AFX_OLE_IMPL
AFX_MODULE_STATE* AFXAPI _AfxGetOleModuleState();
#endif

/////////////////////////////////////////////////////////////////////////////
// CDynLinkLibrary class

IMPLEMENT_DYNAMIC(CDynLinkLibrary, CCmdTarget)

// Constructor - will wire into the current application's list
CDynLinkLibrary::CDynLinkLibrary(AFX_EXTENSION_MODULE& state, BOOL bSystem)
{
#ifndef _AFX_NO_OLE_SUPPORT
	m_factoryList.Construct(offsetof(COleObjectFactory, m_pNextFactory));
#endif
	m_classList.Construct(offsetof(CRuntimeClass, m_pNextClass));

	// copy info from AFX_EXTENSION_MODULE struct
	ASSERT(state.hModule != NULL);
	m_hModule = state.hModule;
	m_hResource = state.hResource;
	m_classList.m_pHead = state.pFirstSharedClass;
#ifndef _AFX_NO_OLE_SUPPORT
	m_factoryList.m_pHead = state.pFirstSharedFactory;
#endif
	m_bSystem = bSystem;

	// insert at the head of the list (extensions will go in front of core DLL)
	DEBUG_ONLY(m_pNextDLL = NULL);
	AfxLockGlobals(CRIT_DYNLINKLIST);
	m_pModuleState->m_libraryList.AddHead(this);
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
}

CDynLinkLibrary::CDynLinkLibrary(HINSTANCE hModule, HINSTANCE hResource)
{
#ifndef _AFX_NO_OLE_SUPPORT
	m_factoryList.Construct(offsetof(COleObjectFactory, m_pNextFactory));
#endif
	m_classList.Construct(offsetof(CRuntimeClass, m_pNextClass));

	m_hModule = hModule;
	m_hResource = hResource;
	m_classList.m_pHead = NULL;
#ifndef _AFX_NO_OLE_SUPPORT
	m_factoryList.m_pHead = NULL;
#endif
	m_bSystem = FALSE;

	// insert at the head of the list (extensions will go in front of core DLL)
	DEBUG_ONLY(m_pNextDLL = NULL);
	AfxLockGlobals(CRIT_DYNLINKLIST);
	m_pModuleState->m_libraryList.AddHead(this);
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
}


CDynLinkLibrary::~CDynLinkLibrary()
{
	// remove this frame window from the list of frame windows
	AfxLockGlobals(CRIT_DYNLINKLIST);
	m_pModuleState->m_libraryList.Remove(this);
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
}

/////////////////////////////////////////////////////////////////////////////
// CDynLinkLibrary diagnostics

#ifdef _DEBUG
void CDynLinkLibrary::AssertValid() const
{
	ASSERT(m_hModule != NULL);
}

void CDynLinkLibrary::Dump(CDumpContext& dc) const
{
	CCmdTarget::Dump(dc);

	dc << "m_hModule = " << (UINT_PTR)m_hModule;
	dc << "\nm_hResource = " << (UINT_PTR)m_hResource;

	if (m_hModule != NULL)
	{
		TCHAR szName[_MAX_PATH];
		GetModuleFileName(m_hModule, szName, _countof(szName));
		dc << "\nmodule name = " << szName;
	}
	else
		dc << "\nmodule name is unknown";

	dc << "\n";
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// special initialization and helper functions

// This is called in an extension DLL's DllMain
//  It makes a copy of the DLL's HMODULE, as well as a copy of the
//  runtime class objects that have been initialized by this
//  extension DLL as part of normal static object construction
//  executed before DllMain is entered.

BOOL AFXAPI AfxInitExtensionModule(AFX_EXTENSION_MODULE& state, HMODULE hModule)
{
	// only initialize once
	if (state.bInitialized)
	{
		AfxInitLocalData(hModule);
		return TRUE;
	}
	state.bInitialized = TRUE;

	// save the current HMODULE information for resource loading
	ASSERT(hModule != NULL);
	state.hModule = hModule;
	state.hResource = hModule;

	// save the start of the runtime class list
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	state.pFirstSharedClass = pModuleState->m_classList.GetHead();
	pModuleState->m_classList.m_pHead = pModuleState->m_pClassInit;

#ifndef _AFX_NO_OLE_SUPPORT
	// save the start of the class factory list
	state.pFirstSharedFactory = pModuleState->m_factoryList.GetHead();
	pModuleState->m_factoryList.m_pHead = pModuleState->m_pFactoryInit;
#endif

	return TRUE;
}


void AFXAPI AfxTermExtensionModule(AFX_EXTENSION_MODULE& state, BOOL bAll)
{
	// make sure initialized
	if (!state.bInitialized)
		return;

	// search for CDynLinkLibrary matching state.hModule and delete it
	ASSERT(state.hModule != NULL);
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_DYNLINKLIST);
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL; )
	{
		CDynLinkLibrary* pNextDLL = pDLL->m_pNextDLL;
		if (bAll || pDLL->m_hModule == state.hModule)
			delete pDLL;    // will unwire itself
		pDLL = pNextDLL;
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);

	// delete any local storage attached to this module
	AfxTermLocalData(state.hModule, TRUE);

	// remove any entries from the CWnd message map cache
	AfxResetMsgCache();
}

/////////////////////////////////////////////////////////////////////////////
// special LoadLibrary and FreeLibrary for loading MFC extension DLLs

HINSTANCE AFXAPI AfxLoadLibrary(LPCTSTR lpszModuleName)
{
	ASSERT(lpszModuleName != NULL);
	AfxLockGlobals(CRIT_LOCKSHARED);
 	HINSTANCE hInstLib = AfxCtxLoadLibrary(lpszModuleName);
	AfxUnlockGlobals(CRIT_LOCKSHARED);
	return hInstLib;
}

 
 
HINSTANCE AFXAPI AfxLoadLibraryEx(LPCTSTR lpFileName,  HANDLE hFile,  DWORD dwFlags)
{  
		
	ASSERT(lpFileName != NULL);
	AfxLockGlobals(CRIT_LOCKSHARED);
	HINSTANCE hInstLib = AfxCtxLoadLibraryEx(lpFileName,hFile,dwFlags);
	AfxUnlockGlobals(CRIT_LOCKSHARED);
	return hInstLib;
}
 

BOOL AFXAPI AfxFreeLibrary(HINSTANCE hInstLib)
{
	AfxLockGlobals(CRIT_LOCKSHARED);
	BOOL bResult = FreeLibrary(hInstLib);
	AfxUnlockGlobals(CRIT_LOCKSHARED);
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// Resource helpers


ULONG AFXAPI AfxGetDllVersion()
{
	return( _LIBS_BUILD );
}

HINSTANCE AFXAPI AfxFindResourceHandle(LPCTSTR lpszName, LPCTSTR lpszType)
{
	ASSERT(lpszName != NULL);
	ASSERT(lpszType != NULL);

	HINSTANCE hInst;

	// first check the main module state
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	if (!pModuleState->m_bSystem)
	{
		hInst = AfxGetResourceHandle();
		if (::FindResource(hInst, lpszName, lpszType) != NULL)
			return hInst;
	}

	// check for non-system DLLs in proper order
	AfxLockGlobals(CRIT_DYNLINKLIST);
	CDynLinkLibrary* pDLL;
	for (pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		if (!pDLL->m_bSystem && pDLL->m_hResource != NULL &&
			::FindResource(pDLL->m_hResource, lpszName, lpszType) != NULL)
		{
			// found it in a DLL
			AfxUnlockGlobals(CRIT_DYNLINKLIST);
			return pDLL->m_hResource;
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);

	// check language specific resource next
	hInst = pModuleState->m_appLangDLL;
	if (hInst != NULL && ::FindResource(hInst, lpszName, lpszType) != NULL)
		return hInst;

	// check the main system module state
	if (pModuleState->m_bSystem)
	{
		hInst = AfxGetResourceHandle();
		if (::FindResource(hInst, lpszName, lpszType) != NULL)
			return hInst;
	}

	// check for system DLLs in proper order
	AfxLockGlobals(CRIT_DYNLINKLIST);
	for (pDLL = pModuleState->m_libraryList; pDLL != NULL; pDLL = pDLL->m_pNextDLL)
	{
		if (pDLL->m_bSystem && pDLL->m_hResource != NULL &&
			::FindResource(pDLL->m_hResource, lpszName, lpszType) != NULL)
		{
			// found it in a DLL
			AfxUnlockGlobals(CRIT_DYNLINKLIST);
			return pDLL->m_hResource;
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);

	// if failed to find resource, return application resource
	return AfxGetResourceHandle();
}

HINSTANCE AFXAPI AfxFindStringResourceHandle(UINT nID)
{
	HINSTANCE hInst;

	// first check the main module state
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	if (!pModuleState->m_bSystem)
	{
		hInst = AfxGetResourceHandle();
		if (AtlGetStringResourceImage(hInst, nID) != NULL)
		{
			// found a non-zero string in app
			return hInst;
		}
	}

	// check non-system DLLs in proper order
	AfxLockGlobals(CRIT_DYNLINKLIST);
	CDynLinkLibrary* pDLL;
	for (pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		if (!pDLL->m_bSystem && (hInst = pDLL->m_hResource) != NULL &&
		  (AtlGetStringResourceImage(hInst, nID) != NULL))
		{
			AfxUnlockGlobals(CRIT_DYNLINKLIST);
			return hInst;
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);

	// check language specific DLL next
	hInst = pModuleState->m_appLangDLL;
	if (hInst != NULL && (AtlGetStringResourceImage(hInst, nID) != NULL))
	{
		// found a non-zero string in language DLL
		return hInst;
	}

	// check the system module state
	if (pModuleState->m_bSystem)
	{
		hInst = AfxGetResourceHandle();
		if (AtlGetStringResourceImage(hInst, nID) != NULL)
		{
			// found a non-zero string in app
			return hInst;
		}
	}

	// check system DLLs in proper order
	AfxLockGlobals(CRIT_DYNLINKLIST);
	for (pDLL = pModuleState->m_libraryList; pDLL != NULL; pDLL = pDLL->m_pNextDLL)
	{
		if (pDLL->m_bSystem && (hInst = pDLL->m_hResource) != NULL &&
		  (AtlGetStringResourceImage(hInst, nID) != NULL))
		{
			AfxUnlockGlobals(CRIT_DYNLINKLIST);
			return hInst;
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);

	// did not find it
	return NULL;
}

// AfxLoadString must not only check for the appropriate string segment
//   in the resource file, but also that the string is non-zero
int AFXAPI AfxLoadString(_In_ UINT nID, _Out_z_cap_post_count_(nMaxBuf, return + 1) LPWSTR lpszBuf, _In_ UINT nMaxBuf)
{
	ASSERT(AfxIsValidAddress(lpszBuf, nMaxBuf*sizeof(WCHAR)));
	if( lpszBuf == NULL || nMaxBuf == 0)
		AfxThrowInvalidArgException();

	HINSTANCE hInst;
	const ATLSTRINGRESOURCEIMAGE* pImage;
	int nCharsToCopy;

	hInst = AfxFindStringResourceHandle(nID);
	if (hInst == NULL)
	{
		lpszBuf[0] = L'\0';
		return 0;
	}

	pImage = AtlGetStringResourceImage(hInst, nID);
	ASSERT(pImage != NULL);
	ASSERT(pImage->nLength != 0);
	nCharsToCopy = min(nMaxBuf-1, pImage->nLength);
	Checked::memcpy_s(lpszBuf, (nMaxBuf-1)*sizeof(WCHAR), 
		pImage->achString, nCharsToCopy*sizeof(WCHAR));
	lpszBuf[nCharsToCopy] = L'\0';

	return nCharsToCopy;
}

int AFXAPI AfxLoadString(_In_ UINT nID, _Out_z_cap_post_count_(nMaxBuf, return + 1) LPSTR lpszBuf, _In_ UINT nMaxBuf)
{
	ASSERT(AfxIsValidAddress(lpszBuf, nMaxBuf*sizeof(CHAR)));
	if( lpszBuf == NULL || nMaxBuf == 0)
		AfxThrowInvalidArgException();

	HINSTANCE hInst;
	const ATLSTRINGRESOURCEIMAGE* pImage;
	int nBytes;

	hInst = AfxFindStringResourceHandle(nID);
	if (hInst == NULL)
	{
		lpszBuf[0] = L'\0';
		return 0;
	}

	pImage = AtlGetStringResourceImage(hInst, nID);
	ENSURE(pImage != NULL);
	ENSURE(pImage->nLength != 0);
	nBytes = ::WideCharToMultiByte(CP_ACP, 0, pImage->achString, pImage->nLength, lpszBuf, nMaxBuf-1, NULL, NULL);
	lpszBuf[nBytes] = '\0';

	return nBytes;
}

/////////////////////////////////////////////////////////////////////////////
// Library initialization and cleanup


extern "C" BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID);
extern "C" BOOL (WINAPI * const _pRawDllMain)(HINSTANCE , DWORD , LPVOID) = &RawDllMain;

extern "C"
BOOL WINAPI RawDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	hInstance;
	try
	{
		if (dwReason == DLL_PROCESS_ATTACH)
		{
			SetErrorMode(SetErrorMode(0) |
				SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

			// add a reference to thread local storage data
			AfxTlsAddRef();

			// make sure we have enough memory to attempt to start (8kb)
			void* pMinHeap = LocalAlloc(NONZEROLPTR, 0x2000);
			if (pMinHeap == NULL)
				return FALSE;   // fail if memory alloc fails
			LocalFree(pMinHeap);

			// cause early initialization of _afxCriticalSection
			if (!AfxCriticalInit())
				return FALSE;

#ifdef _AFX_OLE_IMPL
			// set module state before initialization
			AFX_MODULE_STATE* pModuleState = _AfxGetOleModuleState();
			_AFX_THREAD_STATE* pState = AfxGetThreadState();
			pState->m_pPrevModuleState = AfxSetModuleState(pModuleState);
#endif
		}
		else if (dwReason == DLL_PROCESS_DETACH)
		{
#ifdef _AFX_OLE_IMPL
			_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetDataNA();
			if (pThreadState != NULL)
			{
				// restore previously-saved module state
				VERIFY(AfxSetModuleState(pThreadState->m_pPrevModuleState) ==
					_AfxGetOleModuleState());
				DEBUG_ONLY(pThreadState->m_pPrevModuleState = NULL);
			}
#endif

			// free up the _afxCriticalSection
			AfxCriticalTerm();

			// remove reference from thread local data
			AfxTlsRelease();
		}
	}
	catch( CException* e )
	{
		e->Delete();
		return FALSE;
	}
	
	return TRUE;    // ok
}

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	try
	{
		if (dwReason == DLL_PROCESS_ATTACH)
		{

#ifdef _AFX_OLE_IMPL
			BOOL bRegister = !coreDLL.bInitialized;

			// shared initialization
			AFX_MODULE_STATE* pModuleState = _AfxGetOleModuleState();
			pModuleState->m_hCurrentInstanceHandle = hInstance;
			pModuleState->m_hCurrentResourceHandle = hInstance;
			pModuleState->m_pClassInit = pModuleState->m_classList.GetHead();
			pModuleState->m_pFactoryInit = pModuleState->m_factoryList.GetHead();
#endif

			// initialize this DLL's extension module
			VERIFY(AfxInitExtensionModule(coreDLL, hInstance));

#ifdef _AFX_OLE_IMPL
			AfxWinInit(hInstance, NULL, _T(""), 0);

			// Register class factories in context of private module state
			if (bRegister)
				COleObjectFactory::RegisterAll();
#endif

#ifdef _AFX_OLE_IMPL
			// restore previously-saved module state
			VERIFY(AfxSetModuleState(AfxGetThreadState()->m_pPrevModuleState) ==
				_AfxGetOleModuleState());
			DEBUG_ONLY(AfxGetThreadState()->m_pPrevModuleState = NULL);
#endif

			// wire up this DLL into the resource chain
			CDynLinkLibrary* pDLL = new CDynLinkLibrary(coreDLL, TRUE);
			ASSERT(pDLL != NULL);
			pDLL->m_factoryList.m_pHead = NULL;

			HINSTANCE hLangDLL = AfxLoadLangResourceDLL(_T("%s") _T("MFC") _T(_MFC_FILENAME_VER) _T("%s.DLL"), _T(""));

			AFX_MODULE_STATE* pState = AfxGetModuleState();
			pState->m_appLangDLL = hLangDLL;

#ifdef _AFX_OLE_IMPL
			// copy it to the private OLE state too
			pModuleState->m_appLangDLL = hLangDLL;
#endif
		}
		else if (dwReason == DLL_PROCESS_DETACH)
		{
			// free language specific DLL
			AFX_MODULE_STATE* pState = AfxGetModuleState();
			if (pState->m_appLangDLL != NULL)
			{
				::FreeLibrary(pState->m_appLangDLL);
				pState->m_appLangDLL = NULL;
			}

			// free the DLL info blocks
			CDynLinkLibrary* pDLL;
			while ((pDLL = pState->m_libraryList) != NULL)
				delete pDLL;
			ASSERT(pState->m_libraryList.IsEmpty());

			// cleanup module state for this process
			AfxTermExtensionModule(coreDLL);

#ifdef _AFX_OLE_IMPL
			// set module state for cleanup
			ASSERT(AfxGetThreadState()->m_pPrevModuleState == NULL);
			AfxGetThreadState()->m_pPrevModuleState =
				AfxSetModuleState(_AfxGetOleModuleState());
#endif

			// cleanup module state in OLE private module state
			AfxTermExtensionModule(coreDLL, TRUE);

			// free any local data for this process/thread
			AfxTermLocalData(NULL, TRUE);
		}
		else if (dwReason == DLL_THREAD_DETACH)
		{
			AfxTermThread();
		}
	}
	catch( CException* e )
	{
		if(e)
		{	
			e->Delete();
		}
		return FALSE;
	}

	return TRUE;    // ok
}

////////////////////////////////////////////////////////////////////////////
// Special initialization entry point for controls

void AFXAPI AfxCoreInitModule()
{
	ASSERT(AfxGetModuleState() != AfxGetAppModuleState());

	// construct new dynlink library in this context for core resources
	CDynLinkLibrary* pDLL = new CDynLinkLibrary(coreDLL, TRUE);
	ASSERT(pDLL != NULL);
	pDLL->m_factoryList.m_pHead = NULL;

	// borrow resources from language specific DLL if loaded
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AFX_MODULE_STATE* pAppState = AfxGetAppModuleState();
	if (pModuleState->m_appLangDLL == NULL)
		pModuleState->m_appLangDLL = pAppState->m_appLangDLL;
}

#ifdef _AFX_OLE_IMPL
// This CWinApp is required so this module state has a CWinApp object!
CWinApp _afxOleWinApp;

/////////////////////////////////////////////////////////////////////////////
// static-linked version of AfxWndProc for use by this module

#undef AfxWndProc

LRESULT CALLBACK
AfxWndProcDllOle(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(_AfxGetOleModuleState());
	return AfxWndProc(hWnd, nMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////

// force initialization early
#pragma warning(disable: 4074)
#pragma init_seg(lib)

static AFX_MODULE_STATE _afxOleModuleState(TRUE, &AfxWndProcDllOle,
	_MFC_VER, TRUE);

AFX_MODULE_STATE* AFXAPI _AfxGetOleModuleState()
{
	return &_afxOleModuleState;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Special code to wire up vector deleting destructors

static void _AfxForceVectorDelete()
{
#ifdef _DEBUG
	ASSERT(FALSE);  // never called
#endif

	new CBitmap[2];
	new CBitmapButton[2];
	new CBrush[2];
	new CButton[2];
	new CByteArray[2];
	new CComboBox[2];
	new CDC[2];
	new CDWordArray[2];
	new CDialog[2];
	new CDialogBar[2];
	new CEdit[2];
	new CFile[2];
	new CFont[2];
	new CFrameWnd[2];
	new CGdiObject[2];
	new CListBox[2];
	new CCheckListBox[2];
	new CMapPtrToPtr[2];
	new CMapPtrToWord[2];
	new CMapStringToOb[2];
	new CMapStringToPtr[2];
	new CMapStringToString[2];
	new CMapWordToOb[2];
	new CMapWordToPtr[2];
	new CMemFile[2];
	new CMenu[2];
	new CMetaFileDC[2];
	new CObArray[2];
	new CObList[2];
	new CPalette[2];
	new CPen[2];
	new CPtrArray[2];
	new CPtrList[2];
	new CRectTracker[2];
	new CRgn[2];
	new CScrollBar[2];
	new CSharedFile[2];
	new CSplitterWnd[2];
	new CStatic[2];
	new CStatusBar[2];
	new CStdioFile[2];
	new CString[2];
	new CStringArray[2];
	new CStringList[2];
	new CThreadSlotData[2];
	new CTime[2];
	new CTimeSpan[2];
	new CToolBar[2];
	new CUIntArray[2];
	new CWnd[2];
	new CWordArray[2];

	new CFileFind[2];
	new CInternetSession[2];

	new CDragListBox[2];
	new CStatusBarCtrl[2];
	new CListCtrl[2];
	new CTreeCtrl[2];
	new CSpinButtonCtrl[2];
	new CSliderCtrl[2];
	new CProgressCtrl[2];
	new CHeaderCtrl[2];
	new CHotKeyCtrl[2];
	new CToolTipCtrl[2];
	new CTabCtrl[2];
	new CAnimateCtrl[2];
	new CImageList[2];
	new CToolBarCtrl[2];
	new CRichEditCtrl[2];

	new CMirrorFile[2];
	new CDockState[2];

	new CListView[2];
	new CTreeView[2];
	new CCommandLineInfo[2];
	new CDocManager[2];

	new CPageSetupDialog[2];

	new CSemaphore[2];
	new CMutex[2];
	new CEvent[2];
	new CCriticalSection[2];

#ifdef _AFX_OLE_IMPL
	new COleDataSource[2];
	new COleDispatchDriver[2];
	new COleDropSource[2];
	new CMonikerFile[2];
	new COleResizeBar[2];
	new CAsyncMonikerFile[2];
	new CCachedDataPathProperty[2];
	new CDataPathProperty[2];
	new COleStreamFile[2];
	new COleTemplateServer[2];
	new COleDataObject[2];
	new COleDropTarget[2];
	new COleIPFrameWnd[2];

	new COleDocIPFrameWnd[2];

	new COleVariant[2];
	new CRichEditView[2];
	new CRichEditCntrItem[2];
#endif

#ifdef _AFX_DB_IMPL
	new CDatabase[2];
	new CLongBinary[2];
#endif

// Net
#ifdef _AFX_NET_IMPL
	new CAsyncSocket[2];
	new CSocket[2];
#endif
}
void (*_afxForceVectorDelete_mfc)() = &_AfxForceVectorDelete;

#include <delayimp.h>

extern "C"
{
FARPROC
WINAPI
Downlevel_DelayLoadFailureHook(
	UINT unReason,
	PDelayLoadInfo pDelayInfo
	);

PfnDliHook __pfnDliFailureHook2 = Downlevel_DelayLoadFailureHook;

}

/////////////////////////////////////////////////////////////////////////////
