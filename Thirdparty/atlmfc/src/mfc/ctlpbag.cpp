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
// CBlobProperty

class CBlobProperty : public IPersistStream
{
public:
	CBlobProperty(HGLOBAL pBlob = NULL);
	HGLOBAL GetBlob();

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	STDMETHOD(GetClassID)(LPCLSID);
	STDMETHOD(IsDirty)();
	STDMETHOD(Load)(LPSTREAM);
	STDMETHOD(Save)(LPSTREAM, BOOL);
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER*);

protected:
	long m_dwRef;
	HGLOBAL m_hBlob;
};

CBlobProperty::CBlobProperty(HGLOBAL hBlob) :
	m_hBlob(hBlob),
	m_dwRef(1)
{
}

HGLOBAL CBlobProperty::GetBlob()
{
	return m_hBlob;
}

STDMETHODIMP_(ULONG) CBlobProperty::AddRef()
{
	return InterlockedIncrement(&m_dwRef);
}

STDMETHODIMP_(ULONG) CBlobProperty::Release()
{
	if (InterlockedDecrement(&m_dwRef) > 0)
		return m_dwRef;

	delete this;
	return 0;
}

STDMETHODIMP CBlobProperty::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IPersist) ||
		IsEqualIID(riid, IID_IPersistStream))
	{
		AddRef();
		*ppvObj = this;
		return S_OK;
	}

	*ppvObj = NULL;
	return E_NOINTERFACE;
}

AFX_STATIC_DATA const CLSID _afx_CLSID_BlobProperty =
{ 0xf6f07540, 0x42ec, 0x11ce, { 0x81, 0x35, 0x0, 0xaa, 0x0, 0x4b, 0xb8, 0x51 } };

STDMETHODIMP CBlobProperty::GetClassID(LPCLSID pClsid)
{
	ENSURE_ARG(pClsid != NULL);
	*pClsid = _afx_CLSID_BlobProperty;
	return S_OK;
}

STDMETHODIMP CBlobProperty::IsDirty()
{
	return S_OK;
}

STDMETHODIMP CBlobProperty::Load(LPSTREAM pStream)
{
	ENSURE_ARG(pStream != NULL);
	ULONG cb;
	ULONG cbRead;
	HRESULT hr = pStream->Read(&cb, sizeof(ULONG), &cbRead);

	if (FAILED(hr))
		return hr;

	if (sizeof(ULONG) != cbRead)
		return E_FAIL;

	HGLOBAL hBlobNew = GlobalAlloc(GMEM_MOVEABLE, sizeof(ULONG)+cb);
	if (hBlobNew == NULL)
		return E_OUTOFMEMORY;

	void* pBlobNew = GlobalLock(hBlobNew);
	*(ULONG*)pBlobNew = cb;
	hr = pStream->Read(((ULONG*)pBlobNew)+1, cb, &cbRead);
	GlobalUnlock(hBlobNew);

	if (FAILED(hr))
	{
		GlobalFree(hBlobNew);
		return hr;
	}
	if (cb != cbRead)
	{
		GlobalFree(hBlobNew);
		return E_FAIL;
	}

	GlobalFree(m_hBlob);
	m_hBlob = hBlobNew;
	return S_OK;
}

STDMETHODIMP CBlobProperty::Save(LPSTREAM pStream, BOOL)
{
	ENSURE_ARG(pStream != NULL);

	void* pBlob = GlobalLock(m_hBlob);
	if (pBlob == NULL)
		return E_OUTOFMEMORY;

	ULONG cb = ULONG(sizeof(ULONG)) + *(ULONG*)pBlob;
	ULONG cbWritten;
	HRESULT hr = pStream->Write(pBlob, cb, &cbWritten);

	GlobalUnlock(m_hBlob);

	if (FAILED(hr))
		return hr;

	if (cb != cbWritten)
		return E_FAIL;

	return S_OK;
}

STDMETHODIMP CBlobProperty::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
	ENSURE_ARG(pcbSize != NULL);

	void* pBlob = GlobalLock(m_hBlob);
	if (pBlob == NULL)
		return E_OUTOFMEMORY;

	pcbSize->HighPart = 0;
	pcbSize->LowPart = ULONG(sizeof(ULONG)) + *(ULONG*)pBlob;

	GlobalUnlock(m_hBlob);
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CPropbagPropExchange

class CPropbagPropExchange : public CPropExchange
{
public:
	CPropbagPropExchange(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog,
		BOOL bLoading, BOOL bSaveAllProperties=FALSE);
	~CPropbagPropExchange();

// Operations
	virtual BOOL ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
				void* pvProp, const void* pvDefault = NULL);
	virtual BOOL ExchangeBlobProp(LPCTSTR pszPropName, HGLOBAL* phBlob,
				HGLOBAL hBlobDefault = NULL);
	virtual BOOL ExchangeFontProp(LPCTSTR pszPropName, CFontHolder& font,
				const FONTDESC* pFontDesc, LPFONTDISP pFontDispAmbient);
	virtual BOOL ExchangePersistentProp(LPCTSTR pszPropName,
				LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault);

// Implementation
	LPPROPERTYBAG m_pPropBag;
	LPERRORLOG m_pErrorLog;
	BOOL m_bSaveAllProperties;
};

CPropbagPropExchange::CPropbagPropExchange(LPPROPERTYBAG pPropBag,
	LPERRORLOG pErrorLog, BOOL bLoading, BOOL bSaveAllProperties) :
	m_pPropBag(pPropBag),
	m_pErrorLog(pErrorLog),
	m_bSaveAllProperties(bSaveAllProperties)
{
	m_bLoading = bLoading;
	if (pPropBag != NULL)
		pPropBag->AddRef();
	if (pErrorLog != NULL)
		pErrorLog->AddRef();
}

CPropbagPropExchange::~CPropbagPropExchange()
{
	RELEASE(m_pPropBag);
	RELEASE(m_pErrorLog);
}

BOOL CPropbagPropExchange::ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
	void* pvProp, const void* pvDefault)
{
	ASSERT_POINTER(m_pPropBag, IPropertyBag);
	ASSERT(AfxIsValidAddress(pvProp, 1, FALSE));
	ASSERT((pvDefault == NULL) || AfxIsValidAddress(pvDefault, 1, FALSE));

	if (m_pPropBag == NULL)
		return FALSE;

	ENSURE_ARG(AfxIsValidString(pszPropName));
	ENSURE_ARG(pvProp != NULL);

	BOOL bSuccess = TRUE;
	VARIANT var;
	AfxVariantInit(&var);
	V_VT(&var) = vtProp;

	if (vtProp == VT_LPSTR)
		V_VT(&var) = VT_BSTR;

	const CStringW strPropName(pszPropName);
	if (m_bLoading)
	{		
		if (FAILED(m_pPropBag->Read(strPropName.GetString(), &var, m_pErrorLog)))
			return _AfxCopyPropValue(vtProp, pvProp, pvDefault);

		switch (vtProp)
		{
		case VT_UI1:
			*(BYTE*)pvProp = V_UI1(&var);
			break;

		case VT_I2:
			*(short*)pvProp = V_I2(&var);
			break;

		case VT_I4:
			*(long*)pvProp = V_I4(&var);
			break;

		case VT_BOOL:
			*(BOOL*)pvProp = (BOOL)V_BOOL(&var);
			break;

		case VT_LPSTR:
		case VT_BSTR:
			*(CString*)pvProp = V_BSTR(&var);
			break;

		case VT_CY:
			*(CY*)pvProp = V_CY(&var);
			break;

		case VT_R4:
			Checked::memcpy_s(pvProp, sizeof(float), &V_R4(&var), sizeof(float));
			break;

		case VT_R8:
			Checked::memcpy_s(pvProp, sizeof(double), &V_R8(&var), sizeof(double));
			break;

		default:
			bSuccess = FALSE;
		}
	}
	else
	{
		if (m_bSaveAllProperties ||
			!_AfxIsSamePropValue(vtProp, pvProp, pvDefault))
		{
			switch (vtProp)
			{
			case VT_UI1:
				V_UI1(&var) = *(BYTE*)pvProp;
				break;

			case VT_I2:
				V_I2(&var) = *(short*)pvProp;
				break;

			case VT_I4:
				V_I4(&var) = *(long*)pvProp;
				break;

			case VT_BOOL:
				V_BOOL(&var) = (VARIANT_BOOL)*(BOOL*)pvProp;
				break;

			case VT_LPSTR:
			case VT_BSTR:
				V_BSTR(&var) = ((CString*)pvProp)->AllocSysString();
				break;

			case VT_CY:
				V_CY(&var) = *(CY*)pvProp;
				break;

			case VT_R4:
				Checked::memcpy_s(&V_R4(&var), sizeof(float), pvProp, sizeof(float));
				break;

			case VT_R8:
				Checked::memcpy_s(&V_R8(&var), sizeof(double), pvProp, sizeof(double));
				break;

			default:
				return FALSE;
			}
			bSuccess = SUCCEEDED(m_pPropBag->Write(strPropName.GetString(), &var));
		}
	}

	VariantClear(&var);
	return bSuccess;
}

BOOL CPropbagPropExchange::ExchangeBlobProp(LPCTSTR pszPropName,
	HGLOBAL* phBlob, HGLOBAL hBlobDefault)
{
	ASSERT_POINTER(m_pPropBag, IPropertyBag);
	ASSERT_POINTER(phBlob, HGLOBAL);

	if (m_pPropBag == NULL)
		return FALSE;

	ENSURE_ARG(AfxIsValidString(pszPropName));
	ENSURE_ARG(phBlob != NULL);

	BOOL bSuccess = FALSE;
	VARIANT var;
	AfxVariantInit(&var);
	V_VT(&var) = VT_UNKNOWN;

	const CStringW strPropName(pszPropName);
	if (m_bLoading)
	{
		if (*phBlob != NULL)
		{
			GlobalFree(*phBlob);
			*phBlob = NULL;
		}

		CBlobProperty* pBlobProp = new CBlobProperty;
		V_UNKNOWN(&var) = pBlobProp;

		if (SUCCEEDED(m_pPropBag->Read(strPropName.GetString(), &var, m_pErrorLog)))
		{
			*phBlob = pBlobProp->GetBlob();
			bSuccess = TRUE;
		}
		else
		{
			if (hBlobDefault != NULL)
				bSuccess = _AfxCopyBlob(phBlob, hBlobDefault);
		}

		pBlobProp->Release();
	}
	else
	{
		CBlobProperty* pBlobProp = new CBlobProperty(*phBlob);
		V_UNKNOWN(&var) = pBlobProp;
		bSuccess = SUCCEEDED(m_pPropBag->Write(strPropName.GetString(), &var));
		pBlobProp->Release();
	}
	return bSuccess;
}

BOOL CPropbagPropExchange::ExchangeFontProp(LPCTSTR pszPropName,
	CFontHolder& font, const FONTDESC* pFontDesc,
	LPFONTDISP pFontDispAmbient)
{
	ASSERT_POINTER(m_pPropBag, IPropertyBag);
	ASSERT_POINTER(&font, CFontHolder);
	ASSERT_NULL_OR_POINTER(pFontDesc, FONTDESC);
	ASSERT_NULL_OR_POINTER(pFontDispAmbient, IFontDisp);

	if (m_pPropBag == NULL)
		return FALSE;

	ENSURE_ARG(AfxIsValidString(pszPropName));

	BOOL bSuccess = FALSE;
	VARIANT var;
	AfxVariantInit(&var);
	V_VT(&var) = VT_UNKNOWN;

	
	const CStringW strPropName(pszPropName);
	if (m_bLoading)
	{
		LPFONT pFont = NULL;

		bSuccess =
			SUCCEEDED(m_pPropBag->Read(strPropName.GetString(), &var,
				m_pErrorLog)) &&
			SUCCEEDED(V_UNKNOWN(&var)->QueryInterface(IID_IFont,
				(LPVOID*)&pFont));

		if (bSuccess)
		{
			ASSERT_POINTER(pFont, IFont);
			font.SetFont(pFont);
		}
		else
		{
			// Initialize font to its default state
			font.InitializeFont(pFontDesc, pFontDispAmbient);
		}
		VariantClear(&var);
	}
	else
	{
		if ((font.m_pFont == NULL) ||
			(_AfxIsSameFont(font, pFontDesc, pFontDispAmbient) &&
				!m_bSaveAllProperties))
		{
			bSuccess = TRUE;
		}
		else
		{
			V_UNKNOWN(&var) = font.m_pFont;
			bSuccess = SUCCEEDED(m_pPropBag->Write(strPropName.GetString(), &var));
		}
	}

	return bSuccess;
}

BOOL CPropbagPropExchange::ExchangePersistentProp(LPCTSTR pszPropName,
	LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault)
{
	ASSERT_POINTER(m_pPropBag, IPropertyBag);
	ASSERT_POINTER(ppUnk, LPUNKNOWN);
	ASSERT_NULL_OR_POINTER(pUnkDefault, IUnknown);

	if (m_pPropBag == NULL)
		return FALSE;

	ENSURE_ARG(AfxIsValidString(pszPropName));
	ENSURE_ARG(ppUnk != NULL);

	BOOL bSuccess = FALSE;
	VARIANT var;
	AfxVariantInit(&var);
	V_VT(&var) = VT_UNKNOWN;
	
	const CStringW strPropName(pszPropName);
	if (m_bLoading)
	{
		RELEASE(*ppUnk);
		*ppUnk = NULL;

		bSuccess =
			SUCCEEDED(m_pPropBag->Read(strPropName.GetString(), &var,
				m_pErrorLog)) &&
			SUCCEEDED(V_UNKNOWN(&var)->QueryInterface(iid, (LPVOID*)ppUnk));

		if (!bSuccess)
		{
			// Use default value.
			if (pUnkDefault != NULL)
			{
				bSuccess = SUCCEEDED(pUnkDefault->QueryInterface(iid,
					(LPVOID*)ppUnk));
			}
			else
			{
				bSuccess = TRUE;
			}
		}
		VariantClear(&var);
	}
	else
	{
		if ((*ppUnk == NULL) ||
			(_AfxIsSameUnknownObject(iid, *ppUnk, pUnkDefault) &&
				!m_bSaveAllProperties))
		{
			bSuccess = TRUE;
		}
		else
		{
			V_UNKNOWN(&var) = *ppUnk;
			bSuccess = SUCCEEDED(m_pPropBag->Write(strPropName.GetString(), &var));
		}
	}

	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::XPersistPropertyBag

STDMETHODIMP_(ULONG) COleControl::XPersistPropertyBag::AddRef()
{
	METHOD_PROLOGUE_EX_(COleControl, PersistPropertyBag)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleControl::XPersistPropertyBag::Release()
{
	METHOD_PROLOGUE_EX_(COleControl, PersistPropertyBag)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP COleControl::XPersistPropertyBag::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControl, PersistPropertyBag)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleControl::XPersistPropertyBag::GetClassID(LPCLSID lpClassID)
{
	METHOD_PROLOGUE_EX_(COleControl, PersistPropertyBag)
	return pThis->GetClassID(lpClassID);
}

STDMETHODIMP COleControl::XPersistPropertyBag::InitNew()
{
	METHOD_PROLOGUE_EX(COleControl, PersistPropertyBag)

	// Delegate to OnResetState.
	pThis->OnResetState();

	// Unless IOleObject::SetClientSite is called after this, we can
	// count on ambient properties being available while loading.
	pThis->m_bCountOnAmbients = TRUE;

	// Properties have been initialized
	pThis->m_bInitialized = TRUE;

	// Uncache cached ambient properties
	if (_afxAmbientCache)
	{
		_afxAmbientCache->Cache(NULL);
	}

	return S_OK;
}

STDMETHODIMP COleControl::XPersistPropertyBag::Load(LPPROPERTYBAG pPropBag,
	LPERRORLOG pErrorLog)
{
	METHOD_PROLOGUE_EX(COleControl, PersistPropertyBag)

	HRESULT hr;

	TRY
	{
		CPropbagPropExchange px(pPropBag, pErrorLog, TRUE);
		pThis->DoPropExchange(&px);
		hr = S_OK;
	}
	CATCH_ALL(e)
	{
		hr = E_FAIL;
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	// Properties have probably changed
	pThis->BoundPropertyChanged(DISPID_UNKNOWN);
	pThis->InvalidateControl();

	// Clear the modified flag.
	pThis->m_bModified = FALSE;

	// Properties have been initialized
	pThis->m_bInitialized = TRUE;

	// Uncache cached ambient properties
	_afxAmbientCache->Cache(NULL);

	return hr;
}

STDMETHODIMP COleControl::XPersistPropertyBag::Save(LPPROPERTYBAG pPropBag,
	BOOL fClearDirty, BOOL fSaveAllProperties)
{
	METHOD_PROLOGUE_EX(COleControl, PersistPropertyBag)

	HRESULT hr;

	TRY
	{
		CPropbagPropExchange px(pPropBag, NULL, FALSE, fSaveAllProperties);
		pThis->DoPropExchange(&px);
		hr = S_OK;
	}
	CATCH_ALL(e)
	{
		hr = E_FAIL;
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	// Bookkeeping:  Clear the dirty flag, if requested.
	if (fClearDirty)
		pThis->m_bModified = FALSE;

	return hr;
}
