//
// WSDLPort.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "QName.h"
#include "WSDLBinding.h"

class CWSDLPort : public CXMLElement
{
private:
	CStringW m_strName;
	CQName   m_binding;

	CStringW m_strSoapAddress;
	CStringW m_strHttpAddress;

	CWSDLBinding * m_pBinding;

public:

	CWSDLPort(CWSDLBinding *pBinding = NULL)
		:m_pBinding(pBinding)
	{
	}

	inline HRESULT SetBinding(const CStringW& strQName)
	{
		m_binding.SetQName(strQName);
		return S_OK;
	}

	inline HRESULT SetBinding(const CStringW& strPrefix, const CStringW& strName)
	{
		m_binding.SetQName(strPrefix, strName);
		return S_OK;
	}

	inline HRESULT SetBinding(const wchar_t *wszQName, int cchQName)
	{
		m_binding.SetQName(wszQName, cchQName);
		return S_OK;
	}

	inline const CQName& GetBindingName()
	{
		return m_binding;
	}

	CWSDLBinding * GetBinding();

	inline HRESULT SetName(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strName.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetName(const CStringW& strName)
	{
		m_strName = strName;

		return S_OK;
	}

	inline const CStringW& GetName()
	{
		return m_strName;
	}

	inline HRESULT SetSoapAddress(const wchar_t *wszSoapAddress, int cchSoapAddress)
	{
		if (!wszSoapAddress)
		{
			return E_FAIL;
		}

		m_strSoapAddress.SetString(wszSoapAddress, cchSoapAddress);

		return S_OK;
	}

	inline HRESULT SetSoapAddress(const CStringW& strSoapAddress)
	{

		wchar_t wszTmp[ATL_URL_MAX_URL_LENGTH];
		if(AtlEscapeUrl(strSoapAddress,wszTmp,0,ATL_URL_MAX_URL_LENGTH-1,ATL_URL_BROWSER_MODE) == FALSE)
			return E_FAIL;
		m_strSoapAddress = wszTmp;	
		return S_OK;
	}

	inline const CStringW& GetSoapAddress()
	{
		return m_strSoapAddress;
	}

	inline HRESULT SetHttpAddress(const wchar_t *wszHttpAddress, int cchHttpAddress)
	{
		if (!wszHttpAddress)
		{
			return E_FAIL;
		}

		m_strHttpAddress.SetString(wszHttpAddress, cchHttpAddress);

		return S_OK;
	}

	inline HRESULT SetHttpAddress(const CStringW& strHttpAddress)
	{
		m_strHttpAddress = strHttpAddress;

		return S_OK;
	}

	inline const CStringW& GetHttpAddress()
	{
		return m_strHttpAddress;
	}
};