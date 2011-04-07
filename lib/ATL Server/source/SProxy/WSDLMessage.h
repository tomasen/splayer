//
// WSDLMessage.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "WSDLMessagePart.h"
#include "Emit.h"

class CWSDLMessage : public CXMLElement
{
private:
	
	CStringW m_strDocumentation;
	CStringW m_strName;
	CAtlPtrList<CWSDLMessagePart *> m_parts;

public:

	inline POSITION GetFirstPart()
	{
		return m_parts.GetHeadPosition();
	}

	inline CWSDLMessagePart * GetNextPart(POSITION& pos)
	{
		return m_parts.GetNext(pos);
	}

	inline CWSDLMessagePart * GetPartByName(CStringW& strName)
	{
		POSITION pos = m_parts.GetHeadPosition();
		while (pos != NULL)
		{
			if (m_parts.GetAt(pos)->GetName() == strName)
			{
				break;
			}
			m_parts.GetNext(pos);
		}

		if (pos != NULL)
		{
			return m_parts.GetAt(pos);
		}
		return NULL;
	}
	
	inline CWSDLMessagePart * AddPart()
	{
		CAutoPtr<CWSDLMessagePart> p ( new CWSDLMessagePart );
		if (p != NULL)
		{
			if (m_parts.AddTail(p) != NULL)
			{
				return p.Detach();
			}
		}

		EmitErrorHr(E_OUTOFMEMORY);
		return NULL;
	}

	inline CWSDLMessagePart * AddPart(CWSDLMessagePart *part)
	{
		if (m_parts.AddTail(part) != NULL)
		{
			return part;
		}

		EmitErrorHr(E_OUTOFMEMORY);
		return NULL;
	}

	inline size_t GetNumParts()
	{
		return m_parts.GetCount();
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