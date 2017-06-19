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



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// COleObjectFactory implementation

BEGIN_INTERFACE_MAP(COleObjectFactory, CCmdTarget)
	INTERFACE_PART(COleObjectFactory, IID_IClassFactory, ClassFactory)
	INTERFACE_PART(COleObjectFactory, IID_IClassFactory2, ClassFactory)
END_INTERFACE_MAP()


COleObjectFactory::COleObjectFactory(REFCLSID clsid,
	CRuntimeClass* pRuntimeClass, BOOL bMultiInstance, LPCTSTR lpszProgID)
{
	CommonConstruct(clsid, pRuntimeClass, bMultiInstance,
		afxRegDefault, lpszProgID);
}

COleObjectFactory::COleObjectFactory(REFCLSID clsid,
	CRuntimeClass* pRuntimeClass, BOOL bMultiInstance, int nFlags,
	LPCTSTR lpszProgID)
{
	CommonConstruct(clsid, pRuntimeClass, bMultiInstance,
		nFlags, lpszProgID);
}

void COleObjectFactory::CommonConstruct(REFCLSID clsid,
	CRuntimeClass* pRuntimeClass, BOOL bMultiInstance, int nFlags,
	LPCTSTR lpszProgID)
{
	ASSERT(pRuntimeClass == NULL ||
		pRuntimeClass->IsDerivedFrom(RUNTIME_CLASS(CCmdTarget)));
	ASSERT(AfxIsValidAddress(&clsid, sizeof(CLSID), FALSE));
	ASSERT(lpszProgID == NULL || AfxIsValidString(lpszProgID));

	// initialize to unregistered state
	m_dwRegister = 0;   // not registered yet
	m_bRegistered = FALSE;
	m_clsid = clsid;
	m_pRuntimeClass = pRuntimeClass;
	m_bMultiInstance = bMultiInstance;
	m_nFlags = nFlags;

	m_lpszProgID = lpszProgID;
	m_bOAT = (BYTE) OAT_UNKNOWN;

	// licensing information
	m_bLicenseChecked = FALSE;
	m_bLicenseValid = FALSE;

	// add this factory to the list of factories
	m_pNextFactory = NULL;
	AFX_MODULE_STATE* pModuleState = _AFX_CMDTARGET_GETSTATE();
	AfxLockGlobals(CRIT_OBJECTFACTORYLIST);
	pModuleState->m_factoryList.AddHead(this);
	AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);

	ASSERT_VALID(this);
}


COleObjectFactory::~COleObjectFactory()
{
	AFX_BEGIN_DESTRUCTOR

	ASSERT_VALID(this);

	if (m_pModuleState == NULL)
		return;

	// deregister this class factory
	Revoke();

	// remove this class factory from the list of active class factories
#ifdef _AFXDLL
	AFX_MODULE_STATE* pModuleState = m_pModuleState;
#else
	AFX_MODULE_STATE* pModuleState = _AFX_CMDTARGET_GETSTATE();
#endif
	AfxLockGlobals(CRIT_OBJECTFACTORYLIST);
	BOOL bResult = pModuleState->m_factoryList.Remove(this);
	AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);
	if (bResult)
		return;

	// check CDynLinkLibrary objects in case it was transfered during init
#ifdef _AFXDLL
	AfxLockGlobals(CRIT_DYNLINKLIST);
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		if (pDLL->m_factoryList.Remove(this))
		{
			AfxUnlockGlobals(CRIT_DYNLINKLIST);
			return;
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif

	AFX_END_DESTRUCTOR
}


BOOL COleObjectFactory::Unregister()
{
	return TRUE;
}

BOOL COleObjectFactory::Register()
{
	ASSERT_VALID(this);
	ASSERT(!m_bRegistered);  // registering server/factory twice?
	ASSERT(m_clsid != CLSID_NULL);

	if (!afxContextIsDLL)
	{
		// In the application variants, the IClassFactory is registered
		//  with the OLE DLLs.

		SCODE sc = ::CoRegisterClassObject(m_clsid, &m_xClassFactory,
			CLSCTX_LOCAL_SERVER,
			m_bMultiInstance ? REGCLS_SINGLEUSE : REGCLS_MULTIPLEUSE,
			&m_dwRegister);
		if (sc != S_OK)
		{
#ifdef _DEBUG
			TRACE(traceOle, 0, _T("Warning: CoRegisterClassObject failed scode = %s.\n"),
				::AfxGetFullScodeString(sc));
#endif
			// registration failed.
			return FALSE;
		}
		ASSERT(m_dwRegister != 0);
	}

	++m_bRegistered;
	return TRUE;
}

BOOL PASCAL COleObjectFactory::UnregisterAll()
{
	BOOL bResult = TRUE;
	// register application factories
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_OBJECTFACTORYLIST);
	for (COleObjectFactory* pFactory = pModuleState->m_factoryList;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		// unregister any registered, non-doctemplate factories
		if (pFactory->IsRegistered() && !pFactory->Unregister())
		{
			bResult = FALSE;
		}
	}
	AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);
	return bResult;
}

BOOL PASCAL COleObjectFactory::RegisterAll()
{
	BOOL bResult = TRUE;
	// register application factories
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_OBJECTFACTORYLIST);
	COleObjectFactory* pFactory;
	for (pFactory = pModuleState->m_factoryList;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		// register any non-registered, non-doctemplate factories
		if (!pFactory->IsRegistered() &&
			pFactory->m_clsid != CLSID_NULL && !pFactory->Register())
		{
			bResult = FALSE;
		}
	}
	AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);
#ifdef _AFXDLL
	// register extension DLL factories
	AfxLockGlobals(CRIT_DYNLINKLIST);
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (COleObjectFactory* pDLLFactory = pDLL->m_factoryList;
			pDLLFactory != NULL; pDLLFactory = pDLLFactory->m_pNextFactory)
		{
			// register any non-registered, non-doctemplate factories
			if (!pDLLFactory->IsRegistered() &&
				pDLLFactory->m_clsid != CLSID_NULL && !pDLLFactory->Register())
			{
				bResult = FALSE;
			}
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif
	return bResult;
}


void COleObjectFactory::Revoke()
{
	ASSERT_VALID(this);

	if (m_bRegistered)
	{
		// revoke the registration of the class itself
		if (m_dwRegister != 0)
		{
			::CoRevokeClassObject(m_dwRegister);
			m_dwRegister = 0;
		}
		m_bRegistered = FALSE;
	}
}

void PASCAL COleObjectFactory::RevokeAll()
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_OBJECTFACTORYLIST);
	COleObjectFactory* pFactory;
	for (pFactory = pModuleState->m_factoryList;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		pFactory->Revoke();
	}
	AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);
#ifdef _AFXDLL
	AfxLockGlobals(CRIT_DYNLINKLIST);
	// register extension DLL factories
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (COleObjectFactory* pDLLFactory = pDLL->m_factoryList;
			pDLLFactory != NULL; pDLLFactory = pDLLFactory->m_pNextFactory)
		{
			pDLLFactory->Revoke();
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif
}


void COleObjectFactory::UpdateRegistry(LPCTSTR lpszProgID)
{
	ASSERT_VALID(this);
	ASSERT(lpszProgID == NULL || AfxIsValidString(lpszProgID));

	// use default prog-id if specific prog-id not given
	if (lpszProgID == NULL)
	{
		lpszProgID = m_lpszProgID;
		if (lpszProgID == NULL) // still no valid progID?
			return;
	}

	// call global helper to modify system registry
	//  (progid, shortname, and long name are all equal in this case)
	AfxOleRegisterServerClass(m_clsid, lpszProgID, lpszProgID, lpszProgID,
		OAT_DISPATCH_OBJECT);
}

BOOL PASCAL COleObjectFactory::UpdateRegistryAll(BOOL bRegister)
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_OBJECTFACTORYLIST);

	COleObjectFactory* pFactory;
	BOOL bRegisterSucceeded = TRUE;

	for (pFactory = pModuleState->m_factoryList;
		pFactory != NULL; pFactory = pFactory->m_pNextFactory)
	{
		if (!pFactory->UpdateRegistry(bRegister))
		{
			bRegisterSucceeded = FALSE;
		}
	}
	AfxUnlockGlobals(CRIT_OBJECTFACTORYLIST);

#ifdef _AFXDLL
	AfxLockGlobals(CRIT_DYNLINKLIST);
	// register extension DLL factories
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (COleObjectFactory* pDLLFactory = pDLL->m_factoryList;
			pDLLFactory != NULL; pDLLFactory = pDLLFactory->m_pNextFactory)
		{
			if (!pDLLFactory->UpdateRegistry(bRegister))
			{
				bRegisterSucceeded = FALSE;
			}
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif

	return bRegisterSucceeded;
}

CCmdTarget* COleObjectFactory::OnCreateObject()
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(m_pRuntimeClass, sizeof(CRuntimeClass), FALSE));
		// this implementation needs a runtime class

	// allocate object, throw exception on failure
	CCmdTarget* pTarget = (CCmdTarget*)m_pRuntimeClass->CreateObject();
	if (pTarget == NULL)
		AfxThrowMemoryException();

	// make sure it is a CCmdTarget
	ASSERT_KINDOF(CCmdTarget, pTarget);
	ASSERT_VALID(pTarget);

	// return the new CCmdTarget object
	return pTarget;
}

BOOL COleObjectFactory::IsLicenseValid()
{
	if (!m_bLicenseChecked)
	{
		m_bLicenseValid = (BYTE)VerifyUserLicense();
		m_bLicenseChecked = TRUE;
	}
	return m_bLicenseValid;
}

BOOL COleObjectFactory::UpdateRegistry(BOOL bRegister)
{
	if (m_lpszProgID == NULL)
		return FALSE;

	BOOL bResult = FALSE;

	if (bRegister)
	{
		// call global helper to modify system registry
		// progid, shortname, and long name are all equal in this case
		bResult = AfxOleRegisterServerClass(m_clsid, m_lpszProgID, m_lpszProgID,
											m_lpszProgID, OAT_DISPATCH_OBJECT);
		if (bResult == TRUE)
		{
			bResult = FALSE;
			const int nBufSize = 1024;
			TCHAR szScratch[nBufSize];
			HKEY hkeyClassID;

			LPOLESTR lpszClassID = NULL;
			if (SUCCEEDED(StringFromCLSID(m_clsid, &lpszClassID)))
			{
				CString strClassID(lpszClassID);

				int nLen;
				ATL_CRT_ERRORCHECK_SPRINTF(nLen = _sntprintf_s(szScratch, _countof(szScratch), _countof(szScratch) - 1, _T("CLSID\\%s"), strClassID.GetString()));
				if (nLen >= 0 && nLen < _countof(szScratch))
				{
					if (AfxRegOpenKeyEx(HKEY_CLASSES_ROOT, szScratch, 0,
						KEY_READ|KEY_WRITE, &hkeyClassID) == ERROR_SUCCESS)
					{
						bResult = AfxOleInprocRegisterHelper(NULL, hkeyClassID, m_nFlags);
						::RegCloseKey(hkeyClassID);
					}
				}
				
				CoTaskMemFree(lpszClassID);
			}
			if (!bResult)
			{
				AfxOleUnregisterServerClass(m_clsid, m_lpszProgID, m_lpszProgID,
					m_lpszProgID, OAT_DISPATCH_OBJECT);
			}
		}
	}
	else
	{
		bResult = AfxOleUnregisterServerClass(m_clsid, m_lpszProgID, m_lpszProgID,
											m_lpszProgID, OAT_DISPATCH_OBJECT);
	}
	return bResult;
}

BOOL COleObjectFactory::VerifyUserLicense()
{
	// May be overridden by subclass
	return TRUE;
}

BOOL COleObjectFactory::GetLicenseKey(DWORD, BSTR*)
{
	// May be overridden by subclass
	return FALSE;
}

BOOL COleObjectFactory::VerifyLicenseKey(BSTR bstrKey)
{
	// May be overridden by subclass

	BOOL bLicensed = FALSE;
	BSTR bstr = NULL;

    if (::SysStringLen(bstrKey) != 0 && GetLicenseKey(0, &bstr))
	{
        ASSERT(::SysStringLen(bstr) != 0);

		// if length and content match, it's good!

		UINT cch = SysStringByteLen(bstr);
		if ((cch == SysStringByteLen(bstrKey)) &&
			(memcmp(bstr, bstrKey, cch) == 0))
		{
			bLicensed = TRUE;
		}

		SysFreeString(bstr);
	}

	return bLicensed;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation of COleObjectFactory::IClassFactory interface

STDMETHODIMP_(ULONG) COleObjectFactory::XClassFactory::AddRef()
{
	METHOD_PROLOGUE_EX_(COleObjectFactory, ClassFactory)
	return pThis->InternalAddRef();
}

STDMETHODIMP_(ULONG) COleObjectFactory::XClassFactory::Release()
{
	METHOD_PROLOGUE_EX_(COleObjectFactory, ClassFactory)
	return pThis->InternalRelease();
}

STDMETHODIMP COleObjectFactory::XClassFactory::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleObjectFactory, ClassFactory)
	return pThis->InternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleObjectFactory::XClassFactory::CreateInstance(
	IUnknown* pUnkOuter, REFIID riid, LPVOID* ppvObject)
{
	return CreateInstanceLic(pUnkOuter, NULL, riid, NULL, ppvObject);
}

STDMETHODIMP COleObjectFactory::XClassFactory::LockServer(BOOL fLock)
{
	METHOD_PROLOGUE_EX(COleObjectFactory, ClassFactory)
	ASSERT_VALID(pThis);

	SCODE sc = E_UNEXPECTED;
	TRY
	{
		if (fLock)
			AfxOleLockApp();
		else
			AfxOleUnlockApp();
		sc = S_OK;
	}
	END_TRY

	return sc;
}

STDMETHODIMP COleObjectFactory::XClassFactory::GetLicInfo(
	LPLICINFO pLicInfo)
{
	METHOD_PROLOGUE_EX(COleObjectFactory, ClassFactory)
	ASSERT_VALID(pThis);

	BSTR bstr = NULL;
	pLicInfo->fLicVerified = pThis->IsLicenseValid();
	pLicInfo->fRuntimeKeyAvail = pThis->GetLicenseKey(0, &bstr);
    ::SysFreeString(bstr);

	return S_OK;
}

STDMETHODIMP COleObjectFactory::XClassFactory::RequestLicKey(
	DWORD dwReserved, BSTR* pbstrKey)
{
	METHOD_PROLOGUE_EX(COleObjectFactory, ClassFactory)
	ASSERT_VALID(pThis);

	ASSERT(pbstrKey != NULL);

	*pbstrKey = NULL;

	if (pThis->IsLicenseValid())
	{
		if (pThis->GetLicenseKey(dwReserved, pbstrKey))
			return S_OK;
		else
			return E_FAIL;
	}
	else
		return CLASS_E_NOTLICENSED;
}

STDMETHODIMP COleObjectFactory::XClassFactory::CreateInstanceLic(
	LPUNKNOWN pUnkOuter, LPUNKNOWN /* pUnkReserved */, REFIID riid,
	BSTR bstrKey, LPVOID* ppvObject)
{
	METHOD_PROLOGUE_EX(COleObjectFactory, ClassFactory)
	ASSERT_VALID(pThis);

	if (ppvObject == NULL)
		return E_POINTER;
	*ppvObject = NULL;

    if (((::SysStringLen(bstrKey)!=0) && !pThis->VerifyLicenseKey(bstrKey)) ||
		((::SysStringLen(bstrKey)==0) && !pThis->IsLicenseValid()))
    {
		return CLASS_E_NOTLICENSED;
    }

	// outer objects must ask for IUnknown only
	if (pUnkOuter != NULL && riid != IID_IUnknown)
		return CLASS_E_NOAGGREGATION;

	// attempt to create the object
	CCmdTarget* pTarget = NULL;
	SCODE sc = E_OUTOFMEMORY;
	TRY
	{
		// attempt to create the object
		pTarget = pThis->OnCreateObject();
		if (pTarget != NULL)
		{
			// check for aggregation on object not supporting it
			sc = CLASS_E_NOAGGREGATION;
			if (pUnkOuter == NULL || pTarget->m_xInnerUnknown != 0)
			{
				// create aggregates used by the object
				pTarget->m_pOuterUnknown = pUnkOuter;
				sc = E_OUTOFMEMORY;
				if (pTarget->OnCreateAggregates())
					sc = S_OK;
			}
		}
	}
	END_TRY

	// finish creation
	if (sc == S_OK)
	{
		DWORD dwRef = 1;
		if (pUnkOuter != NULL)
		{
			// return inner unknown instead of IUnknown
			*ppvObject = &pTarget->m_xInnerUnknown;
		}
		else
		{
			// query for requested interface
			sc = pTarget->InternalQueryInterface(&riid, ppvObject);
			if (sc == S_OK)
			{
				dwRef = pTarget->InternalRelease();
				ASSERT(dwRef != 0);
			}
		}
		if (dwRef != 1)
			TRACE(traceOle, 0, "Warning: object created with reference of %ld\n", dwRef);
	}

	// cleanup in case of errors
	if (sc != S_OK)
		delete pTarget;

	return sc;
}

//////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
void COleObjectFactory::AssertValid() const
{
	CCmdTarget::AssertValid();
	ASSERT(m_lpszProgID == NULL || AfxIsValidString(m_lpszProgID));
	ASSERT(m_pRuntimeClass == NULL ||
		AfxIsValidAddress(m_pRuntimeClass, sizeof(CRuntimeClass), FALSE));
	ASSERT(m_pNextFactory == NULL ||
		AfxIsValidAddress(m_pNextFactory, sizeof(COleObjectFactory)));
}

void COleObjectFactory::Dump(CDumpContext& dc) const
{
	USES_CONVERSION;

	CCmdTarget::Dump(dc);

	dc << "m_pNextFactory = " << (void*)m_pNextFactory;
	dc << "\nm_dwRegister = " << m_dwRegister;
	dc << "\nm_bRegistered = " << m_bRegistered;
	LPOLESTR lpszClassID = NULL;
	if (StringFromCLSID(m_clsid, &lpszClassID) == S_OK)
	{
		dc << "\nm_clsid = " << CString(lpszClassID);
		CoTaskMemFree(lpszClassID);
	}
	dc << "\nm_pRuntimeClass = " << m_pRuntimeClass;
	dc << "\nm_bMultiInstance = " << m_bMultiInstance;
	dc << "\nm_lpszProgID = " << m_lpszProgID;
	dc << "\nm_bLicenseChecked = " << m_bLicenseChecked;
	dc << "\nm_bLicenseValid = " << m_bLicenseValid;

	dc << "\n";
}
#endif //_DEBUG


IMPLEMENT_DYNAMIC(COleObjectFactory, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
