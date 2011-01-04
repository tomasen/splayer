//
// ElementParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "ElementParser.h"
#include "ComplexTypeParser.h"
#include "SimpleTypeParser.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"
#include "SimpleType.h"
#include "Emit.h"
#include "resource.h"
#include "Util.h"

TAG_METHOD_IMPL(CElementParser, OnSimpleType)
{
	TRACE_PARSE_ENTRY();

	CElement * pCurr = GetElement();
	if (pCurr != NULL)
	{
		CSimpleType * pElem = pCurr->AddSimpleType();
		if (pElem != NULL)
		{
			SetXSDElementInfo(pElem, pCurr, GetLocator());

			CAutoPtr<CSimpleTypeParser> p( new CSimpleTypeParser(GetReader(), this, GetLevel(), pElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					return p.Detach()->GetAttributes(pAttributes);
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}

TAG_METHOD_IMPL(CElementParser, OnComplexType)
{
	TRACE_PARSE_ENTRY();

	CElement * pCurr = GetElement();
	if (pCurr != NULL)
	{
		CComplexType * pElem = pCurr->AddComplexType();
		if (pElem != NULL)
		{
			SetXSDElementInfo(pElem, pCurr, GetLocator());

			CAutoPtr<CComplexTypeParser> p( new CComplexTypeParser(GetReader(), this, GetLevel(), pElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					return p.Detach()->GetAttributes(pAttributes);
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}

TAG_METHOD_IMPL(CElementParser, OnKey)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CElementParser, OnKeyRef)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CElementParser, OnUnique)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}


ATTR_METHOD_IMPL(CElementParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CElement * pCurr = GetElement();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CElementParser, OnType)
{
	TRACE_PARSE_ENTRY();

	CElement * pCurr = GetElement();
	if (pCurr != NULL)
	{
		return pCurr->SetType(wszValue, cchValue);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CElementParser, OnMinOccurs)
{
	TRACE_PARSE_ENTRY();

	int nMinOccurs = _wtoi(wszValue);
	if (nMinOccurs >= 0)
	{
		//
		// minOccurs must be >= 0
		//

		CElement * pCurr = GetElement();
		if (pCurr != NULL)
		{
			pCurr->SetMinOccurs(nMinOccurs);
			return S_OK;
		}
	}

	EmitInvalidValue("minOccurs", wszValue);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CElementParser, OnMaxOccurs)
{
	TRACE_PARSE_ENTRY();

	if (cchValue==sizeof("unbounded")-1 && !wcsncmp(wszValue, L"unbounded", cchValue))
	{
		CElement * pCurr = GetElement();
		if (pCurr != NULL)
		{
			pCurr->SetMaxOccurs(MAXOCCURS_UNBOUNDED);
			return S_OK;
		}
		return E_FAIL;
	}

	int nMaxOccurs = 0;
	nMaxOccurs = _wtoi(wszValue);
	if (nMaxOccurs >= 0)
	{
		//
		// maxOccurs must be >= 0
		//

		CElement * pCurr = GetElement();
		if (pCurr != NULL)
		{
			pCurr->SetMaxOccurs(nMaxOccurs);
			return S_OK;
		}
	}

	EmitInvalidValue("maxOccurs", wszValue);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CElementParser, OnNillable)
{
	TRACE_PARSE_ENTRY();

	CElement * pCurr = GetElement();
	if (pCurr != NULL)
	{
		bool bVal;
		HRESULT hr = GetBooleanValue(&bVal, wszValue, cchValue);
		if (SUCCEEDED(hr))
		{
			pCurr->SetNullable(bVal);
			return S_OK;
		}

		EmitInvalidValue("nillable", wszValue);
	}

	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}

ATTR_METHOD_IMPL(CElementParser, OnRef)
{
	TRACE_PARSE_ENTRY();	

	CElement * pCurr = GetElement();
	if (pCurr != NULL)
	{
		return pCurr->SetRef(wszValue, cchValue);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CElementParser, OnID)
{
	TRACE_PARSE_ENTRY();

	CElement * pCurr = GetElement();
	if (pCurr != NULL)
	{
		return pCurr->SetID(wszValue, cchValue);
	}

	return E_FAIL;
}


ATTR_METHOD_IMPL(CElementParser, OnAbstract)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CElementParser, OnBlock)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CElementParser, OnDefault)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CElementParser, OnEquivClass)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CElementParser, OnFinal)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CElementParser, OnFixed)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CElementParser, OnForm)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CElementParser, OnArrayType)
{
	TRACE_PARSE_ENTRY();

	CElement *pCurr = GetElement();
	if (pCurr != NULL)
	{
		return pCurr->SetArrayType(wszValue, cchValue);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CElementParser, OnSizeIs)
{
	TRACE_PARSE_ENTRY();
	
	CElement *pCurr = GetElement();
	if (pCurr != NULL)
	{
		return pCurr->SetSizeIs(wszValue, cchValue);
	}
	
	EmitError(IDS_SDL_INTERNAL);
	return E_FAIL;
}

HRESULT __stdcall CElementParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CElement * pCurr = GetElement();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}