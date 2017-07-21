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

typedef void (*PFNMFCM_ReleaseManagedReferences)(IUnknown*);

/////////////////////////////////////////////////////////////////////////////
// Debug helpers

#ifdef _DEBUG
// Helper for converting IID into useful string.  Only for debugging.
LPCTSTR AFXAPI AfxGetIIDString(REFIID iid)
{
	USES_CONVERSION;
	static TCHAR szIID[100];
	szIID[0]			= NULL;
	DWORD dwSize		= sizeof(szIID);
	HKEY hKey			= NULL;
	LPOLESTR pszGUID	= NULL;

	ENSURE(SUCCEEDED(StringFromCLSID(iid, &pszGUID)));
	ENSURE(pszGUID);
	CString strGUID(pszGUID);
	// Attempt to find it in the Interfaces section
	if ((-1 != _stprintf_s(szIID,_countof(szIID), _T("Interface\\%s"), strGUID.GetString()))
	    && (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szIID, 0, KEY_READ, &hKey)))
	{
		RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szIID, &dwSize);
		RegCloseKey(hKey);
	}
	// Attempt to find it in the CLSID section
	else
	{
		if ((-1 != _stprintf_s(szIID,_countof(szIID), _T("CLSID\\%s"), strGUID.GetString()))
		    && (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szIID, 0, KEY_READ, &hKey)))
		{
			RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szIID, &dwSize);
			RegCloseKey(hKey);
		}
		else
		{
			_tcscpy_s(szIID, _countof(szIID), strGUID.GetString());
		}
	}
	CoTaskMemFree(pszGUID);
	return szIID;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Component object model helpers

/////////////////////////////////////////////////////////////////////////////
// IUnknown client helpers

LPUNKNOWN AFXAPI _AfxQueryInterface(LPUNKNOWN lpUnknown, REFIID iid)
{
	ASSERT(lpUnknown != NULL);

	LPUNKNOWN lpW = NULL;
	if (lpUnknown->QueryInterface(iid, (LPLP)&lpW) != S_OK)
		return NULL;

	return lpW;
}

DWORD AFXAPI _AfxRelease(LPUNKNOWN* lplpUnknown)
{
	ASSERT(lplpUnknown != NULL);
	if (*lplpUnknown != NULL)
	{
		DWORD dwRef = (*lplpUnknown)->Release();
		*lplpUnknown = NULL;
		return dwRef;
	}
	return 0;
}


void AFXAPI _AfxReleaseManagedRefs(LPUNKNOWN lpUnk)
{
	ASSERT(lpUnk != NULL);

#ifdef _DEBUG
	#ifdef _UNICODE
		const TCHAR szModuleName[] = _T("mfcm") _T(_MFC_FILENAME_VER) _T("ud.dll");
	#else
		const TCHAR szModuleName[] = _T("mfcm") _T(_MFC_FILENAME_VER) _T("d.dll");
	#endif
#else
	#ifdef _UNICODE
		const TCHAR szModuleName[] = _T("mfcm") _T(_MFC_FILENAME_VER) _T("u.dll");
	#else
		const TCHAR szModuleName[] = _T("mfcm") _T(_MFC_FILENAME_VER) _T(".dll");
	#endif
#endif

	HMODULE hModule = GetModuleHandle( szModuleName );
	if( NULL != hModule )
	{
		PFNMFCM_ReleaseManagedReferences pfnMFCM_ReleaseManagedReferences = NULL;
		pfnMFCM_ReleaseManagedReferences = (PFNMFCM_ReleaseManagedReferences)GetProcAddress(
																						hModule,
																						"AfxmReleaseManagedReferences");

		if( NULL != pfnMFCM_ReleaseManagedReferences )
		{
			pfnMFCM_ReleaseManagedReferences( lpUnk );
		}
	}
}

#define GetInterfacePtr(pTarget, pEntry) \
	((LPUNKNOWN)((BYTE*)pTarget + pEntry->nOffset))

#define GetAggregatePtr(pTarget, pEntry) \
	(*(LPUNKNOWN*)((BYTE*)pTarget + pEntry->nOffset))

/////////////////////////////////////////////////////////////////////////////
// CCmdTarget interface map implementation

// support for aggregation
class CInnerUnknown : public IUnknown
{
public:
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj);
};

// calling this function enables an object to be aggregatable
void CCmdTarget::EnableAggregation()
{
	// construct an CInnerUnknown just to get to the vtable
	CInnerUnknown innerUnknown;

	// copy the vtable & make sure initialized
	ASSERT(sizeof(m_xInnerUnknown) == sizeof(CInnerUnknown));
	m_xInnerUnknown = *(DWORD_PTR*)&innerUnknown;
}

DWORD CCmdTarget::ExternalAddRef()
{
	// delegate to controlling unknown if aggregated
	if (m_pOuterUnknown != NULL)
		return m_pOuterUnknown->AddRef();

	return InternalAddRef();
}

DWORD CCmdTarget::InternalRelease()
{
	ASSERT(GetInterfaceMap() != NULL);

	if (m_dwRef == 0)
		return 0;

	LONG lResult = InterlockedDecrement(&m_dwRef);
	if (lResult == 0)
	{
		AFX_MANAGE_STATE(m_pModuleState);
		OnFinalRelease();
	}
	return lResult;
}

DWORD CCmdTarget::ExternalRelease()
{
	// delegate to controlling unknown if aggregated
	if (m_pOuterUnknown != NULL)
		return m_pOuterUnknown->Release();

	return InternalRelease();
}

// special QueryInterface used in implementation (does not AddRef)
LPUNKNOWN CCmdTarget::GetInterface(const void* iid)
{
	// allow general hook first chance
	LPUNKNOWN lpUnkHook;
	if ((lpUnkHook = GetInterfaceHook(iid)) != NULL)
	{
#ifdef _DEBUG
		if (IsTracingEnabled(traceOle, 1))
		{
			LPCTSTR strIID = AfxGetIIDString(*(IID*)(iid));
			TRACE(traceOle, 1, _T("QueryInterface(%s) succeeded\n"), strIID);
		}
#endif
		return lpUnkHook;
	}

	const AFX_INTERFACEMAP* pMap = GetInterfaceMap();
	ASSERT(pMap != NULL);
	DWORD lData1 = ((IID*)iid)->Data1;

	// IUnknown is a special case since nobody really implements *only* it!
	BOOL bUnknown = ((DWORD*)&IID_IUnknown)[0] == lData1 &&
		((DWORD*)iid)[1] == ((DWORD*)&IID_IUnknown)[1] &&
		((DWORD*)iid)[2] == ((DWORD*)&IID_IUnknown)[2] &&
		((DWORD*)iid)[3] == ((DWORD*)&IID_IUnknown)[3];
	if (bUnknown)
	{
		do
		{
			const AFX_INTERFACEMAP_ENTRY* pEntry = pMap->pEntry;
			ASSERT(pEntry != NULL);
			while (pEntry->piid != NULL)
			{
				// check INTERFACE_ENTRY macro
				LPUNKNOWN lpUnk = GetInterfacePtr(this, pEntry);

				// check vtable pointer (can be NULL)
				if (*(DWORD*)lpUnk != 0)
				{
#ifdef _DEBUG
					if (IsTracingEnabled(traceOle, 1))
					{
						LPCTSTR strIID = AfxGetIIDString(*(IID*)(iid));
						TRACE(traceOle, 1, _T("QueryInterface(%s) succeeded\n"), strIID);
					}
#endif
					return lpUnk;
				}

				// entry did not match -- keep looking
				++pEntry;
			}
#ifdef _AFXDLL
			if (pMap->pfnGetBaseMap == NULL)
				break;
			pMap = (*pMap->pfnGetBaseMap)();
		} while (1);
#else
		} while ((pMap = pMap->pBaseMap) != NULL);
#endif

#ifdef _DEBUG
		if (IsTracingEnabled(traceOle, 1))
		{
			LPCTSTR strIID = AfxGetIIDString(*(IID*)(iid));
			TRACE(traceOle, 1, _T("QueryInterface(%s) failed\n"), strIID);
		}
#endif
		// interface ID not found, fail the call
		return NULL;
	}

	// otherwise, walk the interface map to find the matching IID
	do
	{
		const AFX_INTERFACEMAP_ENTRY* pEntry = pMap->pEntry;
		ASSERT(pEntry != NULL);
		while (pEntry->piid != NULL)
		{
			if (((DWORD*)pEntry->piid)[0] == lData1 &&
				((DWORD*)pEntry->piid)[1] == ((DWORD*)iid)[1] &&
				((DWORD*)pEntry->piid)[2] == ((DWORD*)iid)[2] &&
				((DWORD*)pEntry->piid)[3] == ((DWORD*)iid)[3])
			{
				// check INTERFACE_ENTRY macro
				LPUNKNOWN lpUnk = GetInterfacePtr(this, pEntry);

				// check vtable pointer (can be NULL)
				if (*(DWORD*)lpUnk != 0)
				{
#ifdef _DEBUG
					if (IsTracingEnabled(traceOle, 1))
					{
						LPCTSTR strIID = AfxGetIIDString(*(IID*)(iid));
						TRACE(traceOle, 1, _T("QueryInterface(%s) succeeded\n"), strIID);
					}
#endif
					return lpUnk;
				}
			}

			// entry did not match -- keep looking
			++pEntry;
		}
#ifdef _AFXDLL
		if (pMap->pfnGetBaseMap == NULL)
			break;
		pMap = (*pMap->pfnGetBaseMap)();
	} while (1);
#else
	} while ((pMap = pMap->pBaseMap) != NULL);
#endif

#ifdef _DEBUG
	if (IsTracingEnabled(traceOle, 1))
	{
		LPCTSTR strIID = AfxGetIIDString(*(IID*)(iid));
		TRACE(traceOle, 1, _T("QueryInterface(%s) failed\n"), strIID);
	}
#endif
	// interface ID not found, fail the call
	return NULL;
}

LPUNKNOWN CCmdTarget::QueryAggregates(const void* iid)
{
	const AFX_INTERFACEMAP* pMap = GetInterfaceMap();
	ASSERT(pMap != NULL);

	// walk the Interface map to call aggregates
	do
	{
		const AFX_INTERFACEMAP_ENTRY* pEntry = pMap->pEntry;
		// skip non-aggregate entries
		ASSERT(pEntry != NULL);
		while (pEntry->piid != NULL)
			++pEntry;

		// call QueryInterface for each aggregate entry
		while (pEntry->nOffset != (size_t)-1)
		{
			LPUNKNOWN lpQuery = GetAggregatePtr(this, pEntry);
			// it is ok to have aggregate but not created yet
			if (lpQuery != NULL)
			{
				LPUNKNOWN lpUnk = NULL;
				if (lpQuery->QueryInterface(*(IID*)iid, (LPLP)&lpUnk)
					== S_OK && lpUnk != NULL)
				{
					// QueryInterface successful...
					return lpUnk;
				}
			}

			// entry did not match -- keep looking
			++pEntry;
		}
#ifdef _AFXDLL
		if (pMap->pfnGetBaseMap == NULL)
			break;
		pMap = (*pMap->pfnGetBaseMap)();
	} while (1);
#else
	} while ((pMap = pMap->pBaseMap) != NULL);
#endif

	// interface ID not found, fail the call
	return NULL;
}

// real implementation of QueryInterface
DWORD CCmdTarget::InternalQueryInterface(const void* iid, LPVOID* ppvObj)
{
	// check local interfaces
	if ((*ppvObj = GetInterface(iid)) != NULL)
	{
		// interface was found -- add a reference
		ExternalAddRef();
		return S_OK;
	}

	// check aggregates
	if ((*ppvObj = QueryAggregates(iid)) != NULL)
		return S_OK;

	// interface ID not found, fail the call
	return (DWORD)E_NOINTERFACE;
}

// QueryInterface that is exported to normal clients
DWORD CCmdTarget::ExternalQueryInterface(const void* iid,
	LPVOID* ppvObj)
{
	// delegate to controlling unknown if aggregated
	if (m_pOuterUnknown != NULL)
	{
		HRESULT hRes = m_pOuterUnknown->QueryInterface(*(IID*)iid, ppvObj);
#ifdef _DEBUG
		if (IsTracingEnabled(traceOle, 1))
		{
			LPCTSTR strIID = AfxGetIIDString(*(IID*)(iid));
			if (SUCCEEDED(hRes))
				TRACE(traceOle, 1, _T("QueryInterface(%s) succeeded\n"), strIID);
			else
				TRACE(traceOle, 1, _T("QueryInterface(%s) failed\n"), strIID);
		}
#endif
		return hRes;
	}

	return InternalQueryInterface(iid, ppvObj);
}

/////////////////////////////////////////////////////////////////////////////
// Inner IUnknown implementation (for aggregation)

STDMETHODIMP_(ULONG) CInnerUnknown::AddRef()
{
	METHOD_PROLOGUE_(CCmdTarget, InnerUnknown)
	return pThis->InternalAddRef();
}

STDMETHODIMP_(ULONG) CInnerUnknown::Release()
{
	METHOD_PROLOGUE(CCmdTarget, InnerUnknown)
	return pThis->InternalRelease();
}

STDMETHODIMP CInnerUnknown::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_(CCmdTarget, InnerUnknown)

	if (iid == IID_IUnknown)
	{
		// QueryInterface on inner IUnknown for IID_IUnknown must
		//  return inner IUnknown.
		pThis->InternalAddRef();
		*ppvObj = this;
		return S_OK;
	}
	return pThis->InternalQueryInterface(&iid, ppvObj);
}

/////////////////////////////////////////////////////////////////////////////
// other helper functions

// ExternalDisconnect is used to remove RPC connections in destructors.  This
//  insures that no RPC calls will go to the object after it has been
//  deleted.
void CCmdTarget::ExternalDisconnect()
{
	if (m_dwRef == 0)   // already in disconnected state?
		return;

	// get IUnknown pointer for the object
	LPUNKNOWN lpUnknown = (LPUNKNOWN)GetInterface(&IID_IUnknown);
	ASSERT(lpUnknown != NULL);

	// disconnect the object
	InterlockedIncrement(&m_dwRef);  // protect object from destruction
	CoDisconnectObject(lpUnknown, 0);
	m_dwRef = 0;    // now in disconnected state
}

// GetControllingUnknown is used when creating aggregate objects,
//  usually from OnCreateAggregates.  The outer, or controlling, unknown
//  is one of the parameters to CoCreateInstance and other OLE creation
//  functions which support aggregation.
LPUNKNOWN CCmdTarget::GetControllingUnknown()
{
	if (m_pOuterUnknown != NULL)
		return m_pOuterUnknown; // aggregate of m_pOuterUnknown

	LPUNKNOWN lpUnknown = (LPUNKNOWN)GetInterface(&IID_IUnknown);
	return lpUnknown;   // return our own IUnknown implementation
}

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

// expand inlines for OLE general APIs
#define _AFXDISP_INLINE
#include "afxole.inl"

#endif //!_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////
