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



/////////////////////////////////////////////////////////////////////////////
// Support for MFC/COM in DLLs

SCODE AFXAPI AfxDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	*ppv = NULL;
	DWORD lData1 = rclsid.Data1;

	// search factories defined in the application
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_OBJECTFACTORYLIST);
	COleObjectFactory* pFactory;
	for (pFactory = pModuleState->m_factoryList;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		if (pFactory->m_bRegistered != 0 &&
			lData1 == pFactory->m_clsid.Data1 &&
			((DWORD*)&rclsid)[1] == ((DWORD*)&pFactory->m_clsid)[1] &&
			((DWORD*)&rclsid)[2] == ((DWORD*)&pFactory->m_clsid)[2] &&
			((DWORD*)&rclsid)[3] == ((DWORD*)&pFactory->m_clsid)[3])
		{
			// found suitable class factory -- query for correct interface
			SCODE sc = pFactory->InternalQueryInterface(&riid, ppv);
			AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);
			return sc;
		}
	}
	AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);
#ifdef _AFXDLL
	AfxLockGlobals(CRIT_DYNLINKLIST);
	// search factories defined in extension DLLs
	CDynLinkLibrary* pDLL;
	for (pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (COleObjectFactory* pDLLFactory = pDLL->m_factoryList;
			pDLLFactory != NULL; pDLLFactory = pDLLFactory->m_pNextFactory)
		{
			if (pDLLFactory->m_bRegistered != 0 &&
				lData1 == pDLLFactory->m_clsid.Data1 &&
				((DWORD*)&rclsid)[1] == ((DWORD*)&pDLLFactory->m_clsid)[1] &&
				((DWORD*)&rclsid)[2] == ((DWORD*)&pDLLFactory->m_clsid)[2] &&
				((DWORD*)&rclsid)[3] == ((DWORD*)&pDLLFactory->m_clsid)[3])
			{
				// found suitable class factory -- query for correct interface
				SCODE sc = pDLLFactory->InternalQueryInterface(&riid, ppv);
				AfxUnlockGlobals(CRIT_DYNLINKLIST);
				return sc;
			}
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif

	// factory not registered -- return error
	return CLASS_E_CLASSNOTAVAILABLE;
}

SCODE AFXAPI AfxDllCanUnloadNow(void)
{
	// return S_OK only if no outstanding objects active
	if (!AfxOleCanExitApp())
		return S_FALSE;

	// check if any class factories with >1 reference count
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_OBJECTFACTORYLIST);
	COleObjectFactory* pFactory;
	for (pFactory = pModuleState->m_factoryList;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		if (pFactory->m_dwRef > 1)
		{
			AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);
			return S_FALSE;
		}
	}
	AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);
#ifdef _AFXDLL
	AfxLockGlobals(CRIT_DYNLINKLIST);
	// search factories defined in extension DLLs
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (COleObjectFactory* pDLLFactory = pDLL->m_factoryList;
			pDLLFactory != NULL; pDLLFactory = pDLLFactory->m_pNextFactory)
		{
			if (pDLLFactory->m_dwRef > 1)
			{
				AfxUnlockGlobals(CRIT_DYNLINKLIST);
				return S_FALSE;
			}
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif

	TRACE(traceOle, 0, "Info: AfxDllCanUnloadNow returning S_OK\n");
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
