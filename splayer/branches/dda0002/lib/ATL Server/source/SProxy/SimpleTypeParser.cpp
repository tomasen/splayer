//
// SimpleTypeParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "SimpleTypeParser.h"
#include "SimpleType.h"
#include "Util.h"
#include "Emit.h"
#include "resource.h"

TAG_METHOD_IMPL(CSimpleTypeParser, OnAnnotation)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnLength)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnPattern)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnEnumeration)
{
	TRACE_PARSE_ENTRY();

	CSimpleType * pCurr = GetSimpleType();
	if (pCurr != NULL)
	{
		CEnumeration *pElem = pCurr->AddEnumeration();
		if (pElem != NULL)
		{
			CStringW strValue;
			if (SUCCEEDED(GetAttribute(pAttributes, L"value", sizeof("value")-1, strValue)))
			{
				if (SUCCEEDED(pElem->SetValue(strValue)))
				{
					return SkipElement();
				}
			}
			OnMissingAttribute(TRUE, L"value", sizeof("value")-1, L"", 0);
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnScale)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnPeriod)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnDuration)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnMaxLength)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnPrecision)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnMinInclusive)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnMinExclusive)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnMaxInclusive)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnMaxExclusive)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnMinLength)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnEncoding)
{
	TRACE_PARSE_ENTRY();

	DisableReset();

	CSimpleType * pCurr = GetSimpleType();
	if (pCurr != NULL)
	{
		const wchar_t *wsz = NULL;
		int cch = 0;
		HRESULT hr = GetAttribute(pAttributes, L"value", sizeof("value")-1, &wsz, &cch);
		if ((SUCCEEDED(hr)) && (wsz != NULL))
		{
			return pCurr->SetEncodingType(wsz, cch);
		}
		
		OnMissingAttribute(TRUE, L"value", sizeof("value")-1, L"", 0);
	}

	EmitError(IDS_SDL_INTERNAL);
	return E_FAIL;
}

TAG_METHOD_IMPL(CSimpleTypeParser, OnRestriction)
{
	TRACE_PARSE_ENTRY();

	DisableReset();

	CSimpleType * pCurr = GetSimpleType();
	if (pCurr != NULL)
	{
		const wchar_t *wsz = NULL;
		int cch = 0;
		HRESULT hr = GetAttribute(pAttributes, L"base", sizeof("base")-1, 
			&wsz, &cch);
		if ((hr == S_OK) && (wsz != NULL))
		{
			return pCurr->SetBase(wsz, cch);
		}
		OnMissingAttribute(TRUE, L"base", sizeof("base")-1, L"", 0);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}


ATTR_METHOD_IMPL(CSimpleTypeParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CSimpleType * pCurr = GetSimpleType();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CSimpleTypeParser, OnID)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CSimpleTypeParser, OnAbstract)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

HRESULT __stdcall CSimpleTypeParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CSimpleType * pCurr = GetSimpleType();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}