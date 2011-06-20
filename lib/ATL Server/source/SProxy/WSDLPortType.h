//
// WSDLPortType.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "WSDLPortTypeOperation.h"

class CWSDLPortType : public CXMLElement
{
private:

	CStringW m_strDocumentation;
	CStringW m_strName;

	typedef CAtlPtrMap<CStringW, CWSDLPortTypeOperation *, CStringRefElementTraits<CStringW> > PORTYPEOPERATIONMAP;

	PORTYPEOPERATIONMAP m_operations;

public:

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

};