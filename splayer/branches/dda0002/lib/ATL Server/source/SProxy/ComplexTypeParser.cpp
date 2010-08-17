//
// ComplexTypeParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"


#include "ContentParser.h"
#include "AttributeParser.h"
#include "ComplexTypeParser.h"
#include "ElementParser.h"

#include "Emit.h"
#include "resource.h"

TAG_METHOD_IMPL(CComplexTypeParser, OnElement)
{
	TRACE_PARSE_ENTRY();

	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		CElement * pElem = pCurr->AddElement();
		if (pElem != NULL)
		{
			SetXSDElementInfo(pElem, pCurr, GetLocator());

			CAutoPtr<CElementParser> p( new CElementParser(GetReader(), this, GetLevel(), pElem) );
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

TAG_METHOD_IMPL(CComplexTypeParser, OnAll)
{
	TRACE_PARSE_ENTRY();

	DisableReset();

	return S_OK;
}

TAG_METHOD_IMPL(CComplexTypeParser, OnChoice)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitString(wszNamespaceUri, wszLocalName);

	CComplexType *pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		if (pCurr->GetElementType() == XSD_COMPLEXTYPE)
		{
			pCurr->SetElementType(XSD_UNSUPPORTED);
			return SkipElement();
		}
		return E_FAIL;
	}

	EmitError(IDS_SDL_INTERNAL);
	return E_FAIL;
}

TAG_METHOD_IMPL(CComplexTypeParser, OnAnnotation)
{
	TRACE_PARSE_ENTRY();
	
	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnLength)
{
	TRACE_PARSE_ENTRY();
	EmitSkip(wszNamespaceUri, wszLocalName);
	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnEnumeration)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnPattern)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnScale)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnPeriod)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnDuration)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnMaxLength)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnPrecision)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnMinInclusive)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnMinExclusive)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnMaxInclusive)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnMaxExclusive)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnMinLength)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnEncoding)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnGroup)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnSequence)
{
	TRACE_PARSE_ENTRY();

	DisableReset();

	return S_OK;
}

TAG_METHOD_IMPL(CComplexTypeParser, OnAttribute)
{
	TRACE_PARSE_ENTRY();

	CComplexType *pCurr = GetComplexType();

	if (pCurr != NULL)
	{
		CAttribute *pElem = pCurr->AddAttribute();
		if (pElem != NULL)
		{
			SetXSDElementInfo(pElem, pCurr, GetLocator());

			CAutoPtr<CAttributeParser> p( new CAttributeParser(GetReader(), this, GetLevel(), pElem) );
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

TAG_METHOD_IMPL(CComplexTypeParser, OnAttributeGroup)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnAnyAttribute)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnSimpleContent)
{
	TRACE_PARSE_ENTRY();

/*
	CComplexType *pCurr = GetComplexType();

	if (pCurr != NULL)
	{
		if (pCurr->GetElementType() == XSD_COMPLEXTYPE)
		{
			CContent *pElem = pCurr->AddContent();
			pElem->SetParentDocument(pCurr->GetParentDocument());
			pElem->SetParentElement(pCurr);
			pElem->SetElementType(XSD_SIMPLECONTENT);

			CContentParser * p = new CContentParser(GetReader(), this, GetLevel(), pElem);
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					return p->GetAttributes(pAttributes);
				}
			}
		}
		else
		{
			return OnUnrecognizedTag(wszNamespaceUri, 
				cchNamespaceUri, wszLocalName, cchLocalName, 
				wszQName, cchQName, pAttributes);
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
*/

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);
	return SkipElement();
}

TAG_METHOD_IMPL(CComplexTypeParser, OnAny)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitString(wszNamespaceUri, wszLocalName);

	CComplexType *pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		if (pCurr->GetElementType() == XSD_COMPLEXTYPE)
		{
			pCurr->SetElementType(XSD_UNSUPPORTED);
			return SkipElement();
		}
		return E_FAIL;
	}

	EmitError(IDS_SDL_INTERNAL);
	return E_FAIL;
}

TAG_METHOD_IMPL(CComplexTypeParser, OnComplexContent)
{
	TRACE_PARSE_ENTRY();

	CComplexType *pCurr = GetComplexType();

	if (pCurr != NULL)
	{
		if (pCurr->GetElementType() == XSD_COMPLEXTYPE)
		{
			CContent *pElem = pCurr->AddContent();
			if (pElem != NULL)
			{
				SetXSDElementInfo(pElem, pCurr, GetLocator());
				pElem->SetElementType(XSD_COMPLEXCONTENT);

				CAutoPtr<CContentParser> p( new CContentParser(GetReader(), this, GetLevel(), pElem) );
				if (p != NULL)
				{
					if (g_ParserList.AddHead(p) != NULL)
					{
						return p.Detach()->GetAttributes(pAttributes);
					}
				}
			}
		}
		else
		{
			return OnUnrecognizedTag(wszNamespaceUri, 
				cchNamespaceUri, wszLocalName, cchLocalName, 
				wszQName, cchQName, pAttributes);
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CComplexTypeParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		HRESULT hr = S_OK;
		if (pCurr->GetElementType() == XSD_COMPLEXTYPE)
		{
			hr = pCurr->SetName(wszValue, cchValue);
		}
		return hr;
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CComplexTypeParser, OnID)
{
	TRACE_PARSE_ENTRY();

	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		return pCurr->SetID(wszValue, cchValue);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CComplexTypeParser, OnAbstract)
{
	TRACE_PARSE_ENTRY();

	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL && pCurr->GetElementType() == XSD_COMPLEXTYPE)
	{
		MarkUnsupported(wszQName, cchQName);
	}

	return S_OK;
}

ATTR_METHOD_IMPL(CComplexTypeParser, OnBase)
{
	TRACE_PARSE_ENTRY();

	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		return pCurr->SetBase(wszValue, cchValue);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CComplexTypeParser, OnBlock)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CComplexTypeParser, OnContent)
{
	TRACE_PARSE_ENTRY();

	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		HRESULT hr = S_OK;
		if (pCurr->GetElementType() == XSD_COMPLEXTYPE)
		{
			hr = pCurr->SetContentType(wszValue, cchValue);
		}
		return hr;
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CComplexTypeParser, OnDerivedBy)
{
	TRACE_PARSE_ENTRY();

	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		HRESULT hr = S_OK;
		if (pCurr->GetElementType() == XSD_COMPLEXTYPE)
		{
			if (FAILED(pCurr->SetDerivedBy(wszValue, cchValue)))
			{
				EmitInvalidValue("derivedBy", wszValue);
				hr = E_FAIL;
			}
		}
		return hr;
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CComplexTypeParser, OnFinal)
{
	TRACE_PARSE_ENTRY();

	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL && pCurr->GetElementType() == XSD_COMPLEXTYPE)
	{
		MarkUnsupported(wszQName, cchQName);
	}

	return S_OK;
}

HRESULT __stdcall CComplexTypeParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CComplexType * pCurr = GetComplexType();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}