//
// WSDLPortTypeIO.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "QName.h"
#include "Emit.h"
#include "WSDLSoapElement.h"

class CWSDLMessage;

class CWSDLPortTypeIO : public CXMLElement
{
private:

	CStringW m_strName;
	CQName   m_message;

	CWSDLMessage * m_pMessage;

	CAutoPtr<CSoapBody> m_pSoapBody;

	CAtlPtrList<CSoapHeader *> m_headers;
	CAtlPtrList<CSoapFault *> m_faults;

public:

	inline CWSDLPortTypeIO()
		:m_pMessage(NULL)
	{
	}

	inline CSoapHeader * AddSoapHeader()
	{
		CAutoPtr<CSoapHeader> p ( new CSoapHeader );
		if (p != NULL)
		{
			if (m_headers.AddTail(p) != NULL)
			{
				return p.Detach();
			}
		}

		EmitErrorHr(E_OUTOFMEMORY);
		return NULL;
	}

	inline CSoapHeader * AddSoapHeader(CSoapHeader *p)
	{
		if (m_headers.AddTail(p) != NULL)
		{
			return p;
		}

		EmitErrorHr(E_OUTOFMEMORY);
		return NULL;
	}

	inline POSITION GetFirstSoapHeader()
	{
		return m_headers.GetHeadPosition();
	}

	inline CSoapHeader * GetNextSoapHeader(POSITION &pos)
	{
		return m_headers.GetNext(pos);
	}

	inline size_t GetNumSoapHeaders()
	{
		return m_headers.GetCount();
	}

	inline CSoapFault * AddSoapFault()
	{
		CAutoPtr<CSoapFault> p ( new CSoapFault );
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

	inline CSoapFault * AddSoapFault(CSoapFault *p)
	{
		if (m_faults.AddTail(p) != NULL)
		{
			return p;
		}

		EmitErrorHr(E_OUTOFMEMORY);
		return NULL;
	}

	inline POSITION GetFirstSoapFault()
	{
		return m_faults.GetHeadPosition();
	}

	inline CSoapFault * GetNextSoapFault(POSITION &pos)
	{
		return m_faults.GetNext(pos);
	}

	inline CSoapBody * AddSoapBody()
	{
		m_pSoapBody.Free();
		m_pSoapBody.Attach( new CSoapBody );
		return m_pSoapBody;
	}

	inline CSoapBody * AddSoapBody(CSoapBody *pBody)
	{
		m_pSoapBody.Free();
		m_pSoapBody.Attach( pBody );
		return m_pSoapBody;
	}

	inline CSoapBody * GetSoapBody()
	{
		return m_pSoapBody;
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

	inline HRESULT SetMessage(const CStringW& strQName)
	{
		m_message.SetQName(strQName);

		return S_OK;
	}

	inline HRESULT SetMessage(const CStringW& strPrefix, const CStringW& strName)
	{
		m_message.SetQName(strPrefix, strName);

		return S_OK;
	}

	inline HRESULT SetMessage(const wchar_t *wszQName, int cchQName)
	{
		m_message.SetQName(wszQName, cchQName);

		return S_OK;
	}

	inline const CQName& GetMessageName()
	{
		return m_message;
	}

	CWSDLMessage * GetMessage();
};


typedef CWSDLPortTypeIO CWSDLPortTypeInput;
typedef CWSDLPortTypeIO CWSDLPortTypeOutput;
typedef CWSDLPortTypeIO CWSDLPortTypeFault;