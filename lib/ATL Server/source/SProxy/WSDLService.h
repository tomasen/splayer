//
// WSDLService.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "WSDLPort.h"

class CWSDLService : public CXMLElement
{
private:
	
	CStringW m_strDocumentation;
	CStringW m_strName;

	CAtlPtrList<CWSDLPort *> m_ports;

public:

	inline CWSDLPort * AddPort(CWSDLPort * p = NULL)
	{
		CAutoPtr<CWSDLPort> spOut;
		if (p == NULL)
		{
			spOut.Attach( new CWSDLPort );
			p = spOut;
		}
		if (p != NULL)
		{
			if (m_ports.AddTail(p) != NULL)
			{
				spOut.Detach();
				return p;
			}
		}

		return NULL;
	}

	POSITION GetFirstPort()
	{
		return m_ports.GetHeadPosition();
	}

	CWSDLPort * GetNextPort(POSITION &pos)
	{
		return m_ports.GetNext(pos);
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
};