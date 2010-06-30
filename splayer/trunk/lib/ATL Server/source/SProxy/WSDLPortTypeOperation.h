//
// WSDLPortTypeOperation.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "WSDLPortTypeIO.h"
#include "Emit.h"

//
// TODO: merge CSoapOperation with CSoapBinding if possible (the code is very similar)
//

class CSoapOperation
{
private:
	CStringW m_strSoapAction;
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

	CSoapOperation()
		:m_style(SOAPSTYLE_UNK)
	{
	}

	inline HRESULT SetSoapAction(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strSoapAction.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetSoapAction(const CStringW& strName)
	{
		m_strSoapAction = strName;

		return S_OK;
	}

	inline const CStringW& GetSoapAction()
	{
		return m_strSoapAction;
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

class CHttpOperation
{
private:

	CStringW m_strLocation;

public:

	inline HRESULT SetLocation(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strLocation.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetLocation(const CStringW& strName)
	{
		m_strLocation = strName;

		return S_OK;
	}

	inline const CStringW& GetLocation()
	{
		return m_strLocation;
	}
};


class CWSDLPortTypeOperation : public CXMLElement
{
private:

	CStringW            m_strName;
	CStringW            m_strDocumentation;
	CStringW            m_strParameterOrder;
	CAutoPtr<CWSDLPortTypeInput>  m_pInput;
	CAutoPtr<CWSDLPortTypeOutput> m_pOutput;
	CAtlPtrList<CWSDLPortTypeFault *> m_faults;
	CAutoPtr<CSoapOperation> m_pSoapOperation;
	CAutoPtr<CHttpOperation> m_pHttpOperation;

public:

	inline CSoapOperation * AddSoapOperation(CSoapOperation *pBinding = NULL)
	{
		if (pBinding == NULL)
		{
			pBinding = new CSoapOperation;
		}

		m_pSoapOperation.Free();
		m_pSoapOperation.Attach( pBinding );
		return m_pSoapOperation;
	}

	inline CSoapOperation * GetSoapOperation()
	{
		return m_pSoapOperation;
	}

	inline CHttpOperation * AddHttpOperation(CHttpOperation *pBinding = NULL)
	{
		if (pBinding == NULL)
		{
			pBinding = new CHttpOperation;
		}

		m_pHttpOperation.Free();
		m_pHttpOperation.Attach( pBinding );
		return m_pHttpOperation;
	}

	inline CHttpOperation * GetHttpOperation()
	{
		return m_pHttpOperation;
	}

	inline CWSDLPortTypeInput * AddInput()
	{
		if (!m_pInput)
		{
			m_pInput.Free();
			m_pInput.Attach( new CWSDLPortTypeInput );
		}

		return m_pInput;
	}

	inline CWSDLPortTypeOutput * AddOutput()
	{
		if (!m_pOutput)
		{
			m_pOutput.Free();
			m_pOutput.Attach( new CWSDLPortTypeOutput );
		}

		return m_pOutput;
	}

	inline CWSDLPortTypeInput * GetInput()
	{
		return m_pInput;
	}

	inline CWSDLPortTypeOutput * GetOutput()
	{
		return m_pOutput;
	}

	inline HRESULT SetName(const CStringW& strName)
	{
		m_strName = strName;
		return S_OK;
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

	inline const CStringW& GetName()
	{
		return m_strName;
	}

	inline const CStringW& GetParameterOrder()
	{
		return m_strParameterOrder;
	}

	inline HRESULT SetParameterOrder(const CStringW& str)
	{
		m_strParameterOrder = str;
		return S_OK;
	}

	inline HRESULT SetParameterOrder(const wchar_t *wsz, int cch)
	{
		if (!wsz)
		{
			return E_FAIL;
		}

		m_strParameterOrder.SetString(wsz, cch);

		return S_OK;
	}

	inline CWSDLPortTypeFault * AddFault()
	{
		CAutoPtr<CWSDLPortTypeFault> p ( new CWSDLPortTypeFault );
		if (p != NULL)
		{
			if (m_faults.AddTail(p) != NULL)
			{
				return p.Detach();
			}
		}

		EmitErrorHr(E_OUTOFMEMORY);
		return NULL;
	}

	inline CWSDLPortTypeFault * AddFault(CWSDLPortTypeFault *p)
	{
		if (m_faults.AddTail(p) != NULL)
		{
			return p;
		}

		EmitErrorHr(E_OUTOFMEMORY);
		return NULL;
	}

	inline POSITION GetFirstFault()
	{
		return m_faults.GetHeadPosition();
	}

	inline CWSDLPortTypeFault * GetNextFault(POSITION &pos)
	{
		return m_faults.GetNext(pos);
	}
};