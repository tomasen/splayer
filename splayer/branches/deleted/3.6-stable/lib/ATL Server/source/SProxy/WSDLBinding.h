//
// WSDLBinding.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "QName.h"
#include "Emit.h"

#include "WSDLPortTypeOperation.h"

class CWSDLPortType;

class CSoapBinding : public CXMLElement
{
private:
	CStringW m_strTransport;
	SOAPSTYLE m_style;

	inline HRESULT ValidateStyle(const wchar_t *wsz, int cch)
	{
		m_style = ::GetStyle(wsz, cch);
		if (m_style != SOAPSTYLE_UNK)
		{
			return S_OK;
		}
		return E_FAIL;
	}

public:

	CSoapBinding()
		:m_style(SOAPSTYLE_UNK)
	{
	}

	inline HRESULT SetTransport(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strTransport.SetString(wszName, cchName);

		return S_OK;
	}

	inline void SetTransport(const CStringW& strName)
	{
		m_strTransport = strName;
	}

	inline const CStringW& GetTransport()
	{
		return m_strTransport;
	}

	inline HRESULT SetStyle(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		return ValidateStyle(wszName, cchName);
	}

	inline HRESULT SetStyle(const CStringW& strName)
	{
		return ValidateStyle(strName, strName.GetLength());
	}

	inline const SOAPSTYLE GetStyle()
	{
		return m_style;
	}
};

class CHttpBinding : public CXMLElement
{
private:

	CStringW m_strVerb;

public:

	inline HRESULT SetVerb(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strVerb.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetVerb(const CStringW& strName)
	{
		m_strVerb = strName;
		return S_OK;
	}

	inline const CStringW& GetVerb()
	{
		return m_strVerb;
	}
};

class CWSDLBinding : public CXMLElement
{
private:

	CStringW m_strName;
	CQName   m_type;

	CWSDLPortType * m_pPortType;

	CAutoPtr<CSoapBinding> m_pSoapBinding;
	CAutoPtr<CHttpBinding> m_pHttpBinding;

	typedef CAtlPtrMap<CStringW, CWSDLPortTypeOperation *, CStringRefElementTraits<CStringW> > PORTYPEOPERATIONMAP;

	PORTYPEOPERATIONMAP m_operations;

public:

	inline CWSDLBinding()
		:m_pPortType(NULL)
	{
	}

	CWSDLPortType * GetPortType();

	inline CWSDLPortTypeOperation * AddOperation(CWSDLPortTypeOperation *p)
	{
		if (p != NULL)
		{
			if (p->GetName().GetLength() != 0)
			{
				if (m_operations.SetAt(p->GetName(), p) != NULL)
				{
					return p;
				}
			}
		}

		EmitErrorHr(E_OUTOFMEMORY);
		return NULL;
	}

	inline CSoapBinding * AddSoapBinding(CSoapBinding *pBinding = NULL)
	{
		if (pBinding == NULL)
		{
			pBinding = new CSoapBinding;
		}

		m_pSoapBinding.Attach( pBinding );
		return m_pSoapBinding;
	}

	inline CSoapBinding * GetSoapBinding()
	{
		return m_pSoapBinding;
	}

	inline CHttpBinding * AddHttpBinding(CHttpBinding *pBinding = NULL)
	{
		if (pBinding == NULL)
		{
			pBinding = new CHttpBinding;
		}

		m_pHttpBinding.Attach( pBinding );
		return m_pHttpBinding;
	}

	inline CHttpBinding * GetHttpBinding()
	{
		return m_pHttpBinding;
	}

	inline CWSDLPortTypeOperation * GetOperation(const CStringW& strName)
	{
		const PORTYPEOPERATIONMAP::CPair *p = m_operations.Lookup(strName);
		if (p != NULL)
		{
			return p->m_value;
		}

		return NULL;
	}

	inline POSITION GetFirstOperation()
	{
		return m_operations.GetStartPosition();
	}

	inline CWSDLPortTypeOperation * GetNextOperation(POSITION &pos)
	{
		return m_operations.GetNextValue(pos);
	}

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

	inline HRESULT SetType(const CStringW& strQName)
	{
		m_type.SetQName(strQName);

		return S_OK;
	}

	inline HRESULT SetType(const CStringW& strPrefix, const CStringW& strName)
	{
		m_type.SetQName(strPrefix, strName);

		return S_OK;
	}

	inline HRESULT SetType(const wchar_t *wszQName, int cchQName)
	{
		m_type.SetQName(wszQName, cchQName);

		return S_OK;
	}

	inline const CQName& GetType()
	{
		return m_type;
	}
};