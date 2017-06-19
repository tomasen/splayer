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


#define GetConnectionPtr(pTarget, pEntry) \
	(LPCONNECTIONPOINT)((char*)pTarget + pEntry->nOffset + \
			offsetof(CConnectionPoint, m_xConnPt))


/////////////////////////////////////////////////////////////////////////////
// CConnectionPoint

CConnectionPoint::CConnectionPoint() :
	m_pUnkFirstConnection(NULL),
	m_pConnections(NULL)
{
}

CConnectionPoint::~CConnectionPoint()
{
	AFX_BEGIN_DESTRUCTOR

		POSITION pos = GetStartPosition();
		while (pos != NULL)
		{
			LPUNKNOWN pUnk = GetNextConnection(pos);
			if (pUnk)
				pUnk->Release();
		}

		if (m_pConnections != NULL)
			delete m_pConnections;

	AFX_END_DESTRUCTOR
}

POSITION CConnectionPoint::GetStartPosition() const
{
	ASSERT(m_pConnections == NULL || m_pUnkFirstConnection == NULL);

	if (m_pUnkFirstConnection != NULL)
		return (POSITION)-1;

	if (m_pConnections == NULL || m_pConnections->GetSize() == 0)
		return NULL;

	return (POSITION)1;
}

LPUNKNOWN CConnectionPoint::GetNextConnection(POSITION& pos) const
{
	ASSERT(pos != NULL);

	if (pos == (POSITION)-1)
	{
		ASSERT(m_pUnkFirstConnection != NULL);
		ASSERT(m_pConnections == NULL);

		pos = NULL;
		return m_pUnkFirstConnection;
	}

	ASSERT(m_pConnections != NULL);
	ASSERT((LONG_PTR)pos > 0 && (LONG_PTR)pos <= m_pConnections->GetSize());

	int nIndex = int((LONG_PTR)pos - 1);
	pos = (POSITION)((LONG_PTR)pos + 1);
	if ((LONG_PTR)pos > m_pConnections->GetSize())
		pos = NULL;
	return (LPUNKNOWN)m_pConnections->GetAt(nIndex);
}

const CPtrArray* CConnectionPoint::GetConnections()
{
	ASSERT_VALID(this);
	if (m_pConnections == NULL)
		CreateConnectionArray();

	ASSERT(m_pConnections != NULL);
	return m_pConnections;
}

void CConnectionPoint::OnAdvise(BOOL)
{
	ASSERT_VALID(this);
}

int CConnectionPoint::GetMaxConnections()
{
	ASSERT_VALID(this);

	// May be overridden by subclass.
	return -1;
}

LPCONNECTIONPOINTCONTAINER CConnectionPoint::GetContainer()
{
	CCmdTarget* pCmdTarget = (CCmdTarget*)((BYTE*)this - m_nOffset);
#ifdef _DEBUG
	pCmdTarget->CCmdTarget::AssertValid();
#endif

	LPCONNECTIONPOINTCONTAINER pCPC = NULL;
	if (SUCCEEDED((HRESULT)pCmdTarget->ExternalQueryInterface(
			&IID_IConnectionPointContainer, (LPVOID*)&pCPC)))
	{
		ASSERT(pCPC != NULL);
	}

	return pCPC;
}

void CConnectionPoint::CreateConnectionArray()
{
	ASSERT(m_pConnections == NULL);

	m_pConnections = new CPtrArray;
	if (m_pUnkFirstConnection != NULL)
	{
		m_pConnections->Add(m_pUnkFirstConnection);
		m_pUnkFirstConnection = NULL;
	}

	ASSERT(m_pConnections != NULL);
	ASSERT(m_pUnkFirstConnection == NULL);
}

int CConnectionPoint::GetConnectionCount()
{
	if (m_pUnkFirstConnection != NULL)
		return 1;

	if (m_pConnections == NULL)
		return 0;

	UINT nCount = 0;
	POSITION posConnection = GetStartPosition();
	while (posConnection)
		if (GetNextConnection(posConnection))
			nCount++;
	return nCount;
}

HRESULT CConnectionPoint::QuerySinkInterface(LPUNKNOWN pUnkSink, 
	void** ppInterface)
{
	HRESULT hResult;

	ASSERT(ppInterface != NULL);
	if (ppInterface == NULL)
	{
		return E_POINTER;
	}
	*ppInterface = NULL;

	hResult = pUnkSink->QueryInterface(GetIID(), ppInterface);

	return hResult;
}

STDMETHODIMP_(ULONG) CConnectionPoint::XConnPt::Release()
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	// get parent class that contains connection point
	CCmdTarget* pParent = (CCmdTarget*)((BYTE*)pThis - pThis->m_nOffset);
	return (ULONG)pParent->ExternalRelease();
}

STDMETHODIMP_(ULONG) CConnectionPoint::XConnPt::AddRef()
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	// get parent class that contains connection point
	CCmdTarget* pParent = (CCmdTarget*)((BYTE*)pThis - pThis->m_nOffset);
	return (ULONG)pParent->ExternalAddRef();
}

STDMETHODIMP CConnectionPoint::XConnPt::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)

	ASSERT(AfxIsValidAddress(ppvObj, sizeof(LPVOID), FALSE));

	if (IsEqualIID(iid, IID_IUnknown) ||
		IsEqualIID(iid, IID_IConnectionPoint))
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}

	*ppvObj = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP CConnectionPoint::XConnPt::GetConnectionInterface(IID* pIID)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	ASSERT(AfxIsValidAddress(pIID, sizeof(IID)));

	*pIID = pThis->GetIID();
	return S_OK;
}

STDMETHODIMP CConnectionPoint::XConnPt::GetConnectionPointContainer(
	IConnectionPointContainer** ppCPC)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	ASSERT(AfxIsValidAddress(ppCPC, sizeof(LPCONNECTIONPOINT)));

	if ((*ppCPC = pThis->GetContainer()) != NULL)
		return S_OK;

	return E_FAIL;
}

STDMETHODIMP CConnectionPoint::XConnPt::Advise(
	LPUNKNOWN pUnkSink, DWORD* pdwCookie)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)
	ASSERT(AfxIsValidAddress(pUnkSink, sizeof(IUnknown), FALSE));
	ASSERT((pdwCookie == NULL) || AfxIsValidAddress(pdwCookie, sizeof(DWORD)));

	if (pUnkSink == NULL)
		return E_POINTER;

	LPUNKNOWN lpInterface;
	DWORD dwCookie;

	int cMaxConn = pThis->GetMaxConnections();
	if ((cMaxConn >= 0) && (pThis->GetConnectionCount() == cMaxConn))
	{
		return CONNECT_E_ADVISELIMIT;
	}

	HRESULT hResult;
	hResult = pThis->QuerySinkInterface(pUnkSink, (void**)&lpInterface);
	if (SUCCEEDED( hResult ))
	{
		if (pThis->m_pUnkFirstConnection == NULL &&
			pThis->m_pConnections == NULL)
		{
			pThis->m_pUnkFirstConnection = lpInterface;
			dwCookie = 1;
		}
		else
		{
			if (pThis->m_pConnections == NULL)
				pThis->CreateConnectionArray();

			dwCookie = 0;
			int i;
			for (i = 0; (i < pThis->m_pConnections->GetSize()) && (dwCookie == 0); i++)
			{
				if (pThis->m_pConnections->GetAt(i) == NULL)
				{
					pThis->m_pConnections->SetAt(i, lpInterface);
					dwCookie = i+1;
				}
			}
			if (dwCookie == 0)
			{
				dwCookie = (DWORD)pThis->m_pConnections->Add(lpInterface);
				dwCookie++;
			}
		}

		pThis->OnAdvise(TRUE);
		if (pdwCookie != NULL)
			*pdwCookie = dwCookie;
		return S_OK;
	}

	return hResult;
}

STDMETHODIMP CConnectionPoint::XConnPt::Unadvise(DWORD dwCookie)
{
	METHOD_PROLOGUE_EX_(CConnectionPoint, ConnPt)

	if (pThis->m_pUnkFirstConnection != NULL)
	{
		if (dwCookie == 1)
		{
			pThis->m_pUnkFirstConnection->Release();
			pThis->m_pUnkFirstConnection = NULL;
			pThis->OnAdvise(FALSE);
			return S_OK;
		}
		else
		{
			return CONNECT_E_NOCONNECTION;
		}
	}

	if (pThis->m_pConnections == NULL)
		return CONNECT_E_NOCONNECTION;

	LPUNKNOWN pUnkSink;
	int cConnections = (int)pThis->m_pConnections->GetSize();
	int i = dwCookie-1;
	if ((i >= 0) && (i < cConnections))
	{
		pUnkSink = (LPUNKNOWN)pThis->m_pConnections->GetAt(i);
		if (pUnkSink != NULL)
		{
			pUnkSink->Release();
			pThis->m_pConnections->SetAt(i, NULL);
			pThis->OnAdvise(FALSE);
			return S_OK;
		}
	}

	return CONNECT_E_NOCONNECTION;
}

/////////////////////////////////////////////////////////////////////////////
// CEnumConnections

class CEnumConnections : public CEnumArray
{
public:
	CEnumConnections(const void* pvEnum, UINT nSize);
	~CEnumConnections();
	void AddConnection(CONNECTDATA* pConn);

protected:
	virtual BOOL OnNext(void* pv);
	virtual CEnumArray* OnClone();

	UINT m_nMaxSize;    // number of items allocated (>= m_nSize)

	DECLARE_INTERFACE_MAP()
};

BEGIN_INTERFACE_MAP(CEnumConnections, CEnumArray)
	INTERFACE_PART(CEnumConnections, IID_IEnumConnections, EnumVOID)
END_INTERFACE_MAP()


CEnumConnections::CEnumConnections(const void* pvEnum, UINT nSize) :
	CEnumArray(sizeof(CONNECTDATA), pvEnum, nSize, TRUE)
{
	m_nMaxSize = 0;
}

CEnumConnections::~CEnumConnections()
{
	if (m_pClonedFrom == NULL)
	{
		UINT iCP;
		CONNECTDATA* ppCP = (CONNECTDATA*)(VOID *)m_pvEnum;
		for (iCP = 0; iCP < m_nSize; iCP++)
			RELEASE(ppCP[iCP].pUnk);
	}
	// destructor will free the actual array (if it was not a clone)
}

BOOL CEnumConnections::OnNext(void* pv)
{
	if (!CEnumArray::OnNext(pv))
		return FALSE;

	// outgoing connection point needs to be AddRef'ed
	//  (the caller has responsibility to release it)

	((CONNECTDATA*)pv)->pUnk->AddRef();
	return TRUE;
}

CEnumArray* CEnumConnections::OnClone()
{
	ASSERT_VALID(this);
	CEnumConnections* pClone;
	pClone = new CEnumConnections(m_pvEnum, m_nSize);
	pClone->m_bNeedFree = FALSE;
	ASSERT(pClone != NULL);
	ASSERT(!pClone->m_bNeedFree);   // clones should never free themselves
	pClone->m_nCurPos = m_nCurPos;

	// finally, return the clone to OLE
	ASSERT_VALID(pClone);
	return pClone;
}

void CEnumConnections::AddConnection(CONNECTDATA* pConn)
{
	ASSERT(m_nSize <= m_nMaxSize);

	if (m_nSize == m_nMaxSize)
	{
		// not enough space for new item -- allocate more
		CONNECTDATA* pListNew = new CONNECTDATA[m_nSize+2];
		m_nMaxSize += 2;
		if (m_nSize > 0)
		{
			Checked::memcpy_s(pListNew, m_nMaxSize*sizeof(CONNECTDATA), 
				m_pvEnum, m_nSize*sizeof(CONNECTDATA));
		}
		delete m_pvEnum;
		m_pvEnum = (BYTE*)pListNew;
	}

	// add this item to the list
	ASSERT(m_nSize < m_nMaxSize);
	((CONNECTDATA*)m_pvEnum)[m_nSize] = *pConn;
	pConn->pUnk->AddRef();
	++m_nSize;
}

STDMETHODIMP CConnectionPoint::XConnPt::EnumConnections(LPENUMCONNECTIONS* ppEnum)
{
	METHOD_PROLOGUE_EX(CConnectionPoint, ConnPt)
	CEnumConnections* pEnum = NULL;
	CONNECTDATA cd;

	TRY
	{
		pEnum = new CEnumConnections(NULL, 0);

		if (pThis->m_pUnkFirstConnection != NULL)
		{
			cd.pUnk = pThis->m_pUnkFirstConnection;
			cd.dwCookie = 1;
			pEnum->AddConnection(&cd);
		}

		if (pThis->m_pConnections != NULL)
		{
			int cConnections = (int)pThis->m_pConnections->GetSize();
			for (int i = 0; i < cConnections; i++)
			{
				cd.pUnk = (LPUNKNOWN)(pThis->m_pConnections->GetAt(i));
				cd.dwCookie = i+1;
				if (cd.pUnk != NULL)
				{
					pEnum->AddConnection(&cd);
				}
			}
		}
	}
	CATCH (CException, e)
	{
		delete pEnum;
		pEnum = NULL;
	}
	END_CATCH

	if (pEnum != NULL)
	{
		// create and return the IEnumConnectionPoints object
		*ppEnum = (IEnumConnections*)&pEnum->m_xEnumVOID;
	}
	else
	{
		// no connections: return NULL
		*ppEnum = NULL;
	}
	return (pEnum != NULL) ? S_OK : E_OUTOFMEMORY;
}


/////////////////////////////////////////////////////////////////////////////
// CEnumConnPoints

class CEnumConnPoints : public CEnumArray
{
public:
	CEnumConnPoints(const void* pvEnum, UINT nSize);
	~CEnumConnPoints();
	void AddConnPoint(LPCONNECTIONPOINT pConnPt);

protected:
	virtual BOOL OnNext(void* pv);

	UINT m_nMaxSize;    // number of items allocated (>= m_nSize)

	DECLARE_INTERFACE_MAP()
};

BEGIN_INTERFACE_MAP(CEnumConnPoints, CEnumArray)
	INTERFACE_PART(CEnumConnPoints, IID_IEnumConnectionPoints, EnumVOID)
END_INTERFACE_MAP()


CEnumConnPoints::CEnumConnPoints(const void* pvEnum, UINT nSize) :
	CEnumArray(sizeof(LPCONNECTIONPOINT), pvEnum, nSize, TRUE)
{
	m_nMaxSize = 0;
}

CEnumConnPoints::~CEnumConnPoints()
{
	if (m_pClonedFrom == NULL)
	{
		UINT iCP;
		LPCONNECTIONPOINT* ppCP =
			(LPCONNECTIONPOINT*)(VOID *)m_pvEnum;
		for (iCP = 0; iCP < m_nSize; iCP++)
			RELEASE(ppCP[iCP]);
	}
	// destructor will free the actual array (if it was not a clone)
}

BOOL CEnumConnPoints::OnNext(void* pv)
{
	if (!CEnumArray::OnNext(pv))
		return FALSE;

	// outgoing connection point needs to be AddRef'ed
	//  (the caller has responsibility to release it)

	(*(LPCONNECTIONPOINT*)pv)->AddRef();
	return TRUE;
}

void CEnumConnPoints::AddConnPoint(LPCONNECTIONPOINT pConnPt)
{
	ASSERT(m_nSize <= m_nMaxSize);

	if (m_nSize == m_nMaxSize)
	{
		// not enough space for new item -- allocate more
		LPCONNECTIONPOINT* pListNew = new LPCONNECTIONPOINT[m_nSize+2];
		m_nMaxSize += 2;
		if (m_nSize > 0)
		{
			Checked::memcpy_s(pListNew, m_nMaxSize*sizeof(LPCONNECTIONPOINT), 
				m_pvEnum, m_nSize*sizeof(LPCONNECTIONPOINT));
		}
		delete m_pvEnum;
		m_pvEnum = (BYTE*)pListNew;
	}

	// add this item to the list
	ASSERT(m_nSize < m_nMaxSize);
	((LPCONNECTIONPOINT*)m_pvEnum)[m_nSize] = pConnPt;
	pConnPt->AddRef();
	++m_nSize;
}


/////////////////////////////////////////////////////////////////////////////
// COleConnPtContainer

class COleConnPtContainer : public IConnectionPointContainer
{
public:
#ifndef _AFX_NO_NESTED_DERIVATION
	// required for METHOD_PROLOGUE_EX
	size_t m_nOffset;
	COleConnPtContainer::COleConnPtContainer()
		{ m_nOffset = offsetof(CCmdTarget, m_xConnPtContainer); }
#endif

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	STDMETHOD(EnumConnectionPoints)(LPENUMCONNECTIONPOINTS* ppEnum);
	STDMETHOD(FindConnectionPoint)(REFIID iid, LPCONNECTIONPOINT* ppCP);
};

STDMETHODIMP_(ULONG) COleConnPtContainer::AddRef()
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleConnPtContainer::Release()
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP COleConnPtContainer::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleConnPtContainer::EnumConnectionPoints(
	LPENUMCONNECTIONPOINTS* ppEnum)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)

	CEnumConnPoints* pEnum = NULL;

	TRY
	{
		pEnum = new CEnumConnPoints(NULL, 0);

		// Add connection points that aren't in the connection map
		CPtrArray ptrArray;
		if (pThis->GetExtraConnectionPoints(&ptrArray))
		{
			for (int i = 0; i < ptrArray.GetSize(); i++)
				pEnum->AddConnPoint((LPCONNECTIONPOINT)ptrArray.GetAt(i));
		}

		// walk the chain of connection maps
		const AFX_CONNECTIONMAP* pMap = pThis->GetConnectionMap();
		const AFX_CONNECTIONMAP_ENTRY* pEntry;

#ifdef _AFXDLL
		for (;;)
#else
		while (pMap != NULL)
#endif
		{
			pEntry = pMap->pEntry;

			while (pEntry->piid != NULL)
			{
				pEnum->AddConnPoint(GetConnectionPtr(pThis, pEntry));
				++pEntry;
			}
#ifdef _AFXDLL
			if (pMap->pfnGetBaseMap == NULL)
				break;
			pMap = (*pMap->pfnGetBaseMap)();
#else
			pMap = pMap->pBaseMap;
#endif
		}
	}
	CATCH (CException, e)
	{
		delete pEnum;
		pEnum = NULL;
	}
	END_CATCH

	if (pEnum != NULL)
	{
		// create and return the IEnumConnectionPoints object
		*ppEnum = (IEnumConnectionPoints*)&pEnum->m_xEnumVOID;
	}
	else
	{
		// no connection points: return NULL
		*ppEnum = NULL;
	}

	return (pEnum != NULL) ? S_OK : CONNECT_E_NOCONNECTION;
}

STDMETHODIMP COleConnPtContainer::FindConnectionPoint(
	REFIID iid, LPCONNECTIONPOINT* ppCP)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, ConnPtContainer)
	ASSERT(ppCP != NULL);

	if ((*ppCP = pThis->GetConnectionHook(iid)) != NULL)
	{
		(*ppCP)->AddRef();
		return S_OK;
	}

	const AFX_CONNECTIONMAP* pMap = pThis->GetConnectionMap();
	const AFX_CONNECTIONMAP_ENTRY* pEntry;

#ifdef _AFXDLL
	for (;;)
#else
	while (pMap != NULL)
#endif
	{
		pEntry = pMap->pEntry;

		while (pEntry->piid != NULL)
		{
			if (IsEqualIID(iid, *(IID*)(pEntry->piid)))
			{
				*ppCP = GetConnectionPtr(pThis, pEntry);
				(*ppCP)->AddRef();
				return S_OK;
			}
			++pEntry;
		}
#ifdef _AFXDLL
		if (pMap->pfnGetBaseMap == NULL)
			break;
		pMap = (*pMap->pfnGetBaseMap)();
#else
		pMap = pMap->pBaseMap;
#endif
	}

	return CONNECT_E_NOCONNECTION;
}


/////////////////////////////////////////////////////////////////////////////
// Wiring CCmdTarget to COleConnPtContainer

// enable this object for OLE connections, called from derived class ctor
void CCmdTarget::EnableConnections()
{
	ASSERT(GetConnectionMap() != NULL);   // must have DECLARE_DISPATCH_MAP

	// construct an COleConnPtContainer instance just to get to the vtable
	COleConnPtContainer cpc;

	// vtable pointer should be already set to same or NULL
	ASSERT(m_xConnPtContainer.m_vtbl == NULL||
		*(DWORD_PTR*)&cpc == m_xConnPtContainer.m_vtbl);
	// verify that sizes match
	ASSERT(sizeof(m_xConnPtContainer) == sizeof(COleConnPtContainer));

	// copy the vtable (and other data) to make sure it is initialized
	m_xConnPtContainer.m_vtbl = *(DWORD_PTR*)&cpc;
	*(COleConnPtContainer*)&m_xConnPtContainer = cpc;
}

/////////////////////////////////////////////////////////////////////////////
// Force any extra compiler-generated code into AFX_INIT_SEG

