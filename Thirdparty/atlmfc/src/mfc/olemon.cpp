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

#pragma inline_depth(0)

/////////////////////////////////////////////////////////////////////////////
// _AfxBindHost for CMonikerFile implementation

class _AfxBindHost: public IBindHost
{
private:
	long m_dwRef;
public:
	inline _AfxBindHost() : m_dwRef(0) {}

	inline ~_AfxBindHost() { ASSERT(m_dwRef == 0); }

	STDMETHOD_(ULONG, AddRef)()
	{
		return InterlockedIncrement(&m_dwRef);
	}

	STDMETHOD_(ULONG, Release)()
	{
		unsigned long lResult = InterlockedDecrement(&m_dwRef);
		if (lResult == 0)
			delete this;
		return lResult;
	}

	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject)
	{
		if (!ppvObject)
			return E_POINTER;

		// check for the interfaces this object knows about
		if (iid == IID_IUnknown || iid == IID_IBindHost)
		{
			*ppvObject = (IBindHost*)this;
			InterlockedIncrement(&m_dwRef);
			return S_OK;
		}

		// otherwise, incorrect IID, and thus error
		return E_NOINTERFACE;
	}

	STDMETHOD(CreateMoniker)(
	/* [in] */ LPOLESTR szName,
	/* [in] */ IBindCtx __RPC_FAR *pBC,
	/* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk,
	/* [in] */ DWORD dwReserved)
	{
		UNUSED_ALWAYS(dwReserved);
		UNUSED_ALWAYS(pBC);

		if (!szName || !ppmk) return E_POINTER;
		if (!*szName) return E_INVALIDARG;

		*ppmk = NULL;
		HRESULT hr = S_OK;

		hr = CreateURLMoniker(NULL, szName, ppmk);
		if (SUCCEEDED(hr) && !*ppmk)
			hr = E_FAIL;
		return hr;
	}

	STDMETHOD(MonikerBindToStorage)(
	/* [in] */ IMoniker __RPC_FAR *pMk,
	/* [in] */ IBindCtx __RPC_FAR *pBC,
	/* [in] */ IBindStatusCallback __RPC_FAR *pBSC,
	/* [in] */ REFIID riid,
	/* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj)
	{
		if (!pMk || !ppvObj) return E_POINTER;

		*ppvObj = NULL;
		HRESULT hr = S_OK;
		IPTR(IBindCtx) BindCtx;
		if (pBC)
		{
			BindCtx = pBC;
			if (pBSC)
			{
#ifdef _DEBUG
				IPTR(IBindStatusCallback) pBSCPrev;
#endif
				hr = RegisterBindStatusCallback(BindCtx, pBSC,
#ifdef _DEBUG
					&pBSCPrev,
#else
					NULL,
#endif
					0);
				ASSERT(!pBSCPrev);
				if (FAILED(hr))
					return hr;
			}
		}
		else
		{
			if (pBSC)
				hr = CreateAsyncBindCtx(0, pBSC, NULL, &BindCtx);
			else
				hr = CreateBindCtx(0, &BindCtx);
			if (SUCCEEDED(hr) && !BindCtx)
				hr = E_FAIL;
			if (FAILED(hr))
				return hr;
		}
		return pMk->BindToStorage(BindCtx, NULL, riid, ppvObj);
	}

private:
	STDMETHOD(MonikerBindToObject)(
	/* [in] */ IMoniker __RPC_FAR *pMk,
	/* [in] */ IBindCtx __RPC_FAR *pBC,
	/* [in] */ IBindStatusCallback __RPC_FAR *pBSC,
	/* [in] */ REFIID riid,
	/* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj)
	{
		ASSERT(FALSE);
		UNUSED_ALWAYS(pMk);
		UNUSED_ALWAYS(pBC);
		UNUSED_ALWAYS(pBSC);
		UNUSED_ALWAYS(riid);
		UNUSED_ALWAYS(ppvObj);
		return E_NOTIMPL;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CMonikerFile implementation

CMonikerFile::~CMonikerFile()
{
	AFX_BEGIN_DESTRUCTOR

	ASSERT_VALID(this);
	Close();

	AFX_END_DESTRUCTOR
}

void CMonikerFile::Flush()
{
	ASSERT_VALID(this);
	ENSURE(GetStream() != NULL);

	GetStream()->Commit(0);
}

BOOL CMonikerFile::Open(LPCTSTR lpszUrl, IBindHost* pBindHost,
	IBindStatusCallback* pBSC, IBindCtx* pBindCtx, CFileException* pError)
{
	ASSERT_VALID(this);
	Close(); // These objects are reopenable
	return Attach(lpszUrl, pBindHost, pBSC, pBindCtx, pError);
}

BOOL CMonikerFile::Attach(LPCTSTR lpszUrl, IBindHost* pBindHost,
	IBindStatusCallback* pBSC, IBindCtx* pBindCtx, CFileException* pError)
{
	ASSERT(!m_Moniker);
	ASSERT(!GetStream());
	USES_CONVERSION_EX;

	ASSERT(NULL == lpszUrl || AfxIsValidString(lpszUrl));
	ASSERT(NULL == pError ||
		AfxIsValidAddress(pError, sizeof(CFileException)));
	ASSERT(NULL != pBindHost);

	if (pBindHost == NULL)
	{
		// Throwing exception here instead of returning FALSE because we cannot
		// fill pError here. So we need to indicate to the caller that 
		// something really bad has happened. Previously the code would AV.
		AfxThrowInvalidArgException();	
	}

	// Check for empty path
	if (!lpszUrl || !*lpszUrl)
	{
		if (pError)
		{
			pError->m_cause = CFileException::badPath;
			pError->m_strFileName = lpszUrl;
		}
		return FALSE;
	}

	// Create the moniker
	HRESULT hr;
	IPTR(IMoniker) pMoniker;

	LPOLESTR lpoleszUrl = T2OLE_EX((LPTSTR) lpszUrl, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if (lpoleszUrl == NULL)
	{
		// OUT OF MEMORY
		if (pError)
		{
			pError->m_cause = CFileException::none; // Because there is no error code for out of memory here and its not something wrong with the file
			pError->m_strFileName = lpszUrl;
		}
	}
	
	hr = pBindHost->CreateMoniker(lpoleszUrl, pBindCtx,
		reinterpret_cast<IMoniker**>(&pMoniker), 0);

	if (FAILED(hr))
	{
		if (pError) _AfxFillOleFileException(pError, hr);
		return FALSE;
	}

	return Attach(pMoniker, pBindHost, pBSC, pBindCtx, pError);
}

BOOL CMonikerFile::Open(LPCTSTR lpszUrl, CFileException* pError)
{
	IPTR(IBindHost) pBindHost(CreateBindHost(), FALSE);
	IPTR(IBindCtx) pBindCtx(CreateBindContext(pError), FALSE);

	return Open(lpszUrl, pBindHost, NULL, pBindCtx, pError);
}

BOOL CMonikerFile::Open(IMoniker* pMoniker, CFileException* pError)
{
	Close(); // These objects are reopenable
	IPTR(IBindHost) pBindHost(CreateBindHost(), FALSE);
	IPTR(IBindCtx) pBindCtx(CreateBindContext(pError), FALSE);

	return Attach(pMoniker, pBindHost, NULL, pBindCtx, pError);
}

BOOL CMonikerFile::Open(IMoniker* pMoniker, IBindHost* pBindHost,
	IBindStatusCallback* pBSC, IBindCtx* pBindCtx, CFileException* pError)
{
	Close();
	return Attach(pMoniker, pBindHost, pBSC, pBindCtx, pError);
}

BOOL CMonikerFile::Attach(IMoniker* pMoniker, IBindHost* pBindHost,
	IBindStatusCallback* pBSC, IBindCtx* pBindCtx, CFileException* pError)
{
	ASSERT_VALID(this);
	ASSERT(pMoniker);
	ASSERT(!m_Moniker);
	ASSERT(!GetStream());
	m_Moniker=pMoniker;

	IPTR(IStream) pStream;
	if (pBindHost == NULL)
	{
		// Throwing exception here instead of returning FALSE because we cannot
		// fill pError here. So we need to indicate to the caller that 
		// something really bad has happened. Previously the code would AV.
		AfxThrowInvalidArgException();
	}
	HRESULT hr=pBindHost->MonikerBindToStorage(pMoniker, pBindCtx, pBSC,
		IID_IStream, reinterpret_cast<void**>(&pStream));
	if (FAILED(hr))
	{
		if (pError) _AfxFillOleFileException(pError, hr);
		return FALSE;
	}

	// If this is really a CAsyncMonikerFile, then we may have attached to the stream
	// within MonikerBindToStorage, in which case we want to avoid doing so here.
	if (pStream.GetInterfacePtr() && !GetStream())
	{
		// Attach this to the stream, transferring the reference
		// COleStreamFile::Attach doesn't increment pStream's refcount
		COleStreamFile::Attach(pStream);
		pStream.Detach();
	}

	return PostBindToStream(pError);
}

void CMonikerFile::Close()
{
	if (m_Moniker.GetInterfacePtr())
		m_Moniker.Release();
	COleStreamFile::Close();
}

BOOL CMonikerFile::Detach(CFileException* pError)
{
	ASSERT_VALID(this);
	ASSERT(!!m_Moniker);

	TRY
	{
		Close();
	}
	CATCH (CFileException, e)
	{
		if (pError)
		{
			pError->m_cause=e->m_cause;
			pError->m_lOsError=e->m_lOsError;
			pError->m_strFileName=e->m_strFileName;
		}
		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH;

	return TRUE;
}

IBindHost* CMonikerFile::CreateBindHost()
{
	IBindHost* pBindHost = new _AfxBindHost();
	pBindHost->AddRef();
	return pBindHost;
}

IBindCtx* CMonikerFile::CreateBindContext(CFileException* pError)
{
	UNUSED_ALWAYS(pError);
	return NULL;
}

// So that CMonikerFile can check for a null pStream and CAsyncMonikerFile can ignore it
BOOL CMonikerFile::PostBindToStream(CFileException* pError)
{
	if (!GetStream())
	{
		if (pError) _AfxFillOleFileException(pError, E_UNEXPECTED);
		TRY
		{
			Close();
		}
		CATCH_ALL(e)
		{
			DELETE_EXCEPTION(e);
		}
		END_CATCH_ALL
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMonikerFile diagnostics

#ifdef _DEBUG
void CMonikerFile::AssertValid() const
{
	COleStreamFile::AssertValid();
}

void CMonikerFile::Dump(CDumpContext& dc) const
{
	COleStreamFile::Dump(dc);

	dc << "\nm_Moniker = " << m_Moniker.GetInterfacePtr();
	dc << "\n";
}
#endif

////////////////////////////////////////////////////////////////////////////

#ifndef _AFX_ENABLE_INLINES

// expand inlines for OLE general APIs
#define _AFXOLEMONIKER_INLINE
#include "afxole.inl"

#endif //!_AFX_ENABLE_INLINES


IMPLEMENT_DYNAMIC(CMonikerFile, COleStreamFile)

////////////////////////////////////////////////////////////////////////////
