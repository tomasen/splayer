//
// ContentParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "ContentParser.h"
#include "Content.h"
#include "Attribute.h"
#include "ComplexTypeParser.h"
#include "Element.h"
#include "ComplexType.h"

// CContentParser parses the simpleContent tag and the complexContent tag

TAG_METHOD_IMPL(CContentParser, OnAnnotation)
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

TAG_METHOD_IMPL(CContentParser, OnRestriction)
{
	TRACE_PARSE_ENTRY();

	CContent *pCurr = GetContent();
	if (pCurr != NULL)
	{
		CComplexType * pElem = pCurr->AddType();
		if (pElem != NULL)
		{
			pElem->SetElementType(XSD_RESTRICTION);
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

TAG_METHOD_IMPL(CContentParser, OnExtension)
{
	TRACE_PARSE_ENTRY();
	
	int nLine = 0;
	int nCol = 0;
	GetLocator()->getLineNumber(&nLine);
	GetLocator()->getColumnNumber(&nCol);
	
	CContent *pCurr = GetContent();
	if (pCurr != NULL)
	{
		EmitFileError(IDS_SDL_BASE_EXTENSION,
			(LPCWSTR) pCurr->GetParentDocument()->GetDocumentUri(),
			nLine, nCol, 0);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CContentParser, OnMixed)
{
	TRACE_PARSE_ENTRY();

	CContent *pCurr = GetContent();
	if (pCurr != NULL)
	{
		if (SUCCEEDED(pCurr->SetMixed(wszValue, cchValue)))
		{
			return S_OK;
		}
		EmitInvalidValue("mixed", wszValue);
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CContentParser, OnID)
{
	TRACE_PARSE_ENTRY();

	CContent *pCurr = GetContent();
	if (pCurr != NULL)
	{
		return pCurr->SetID(wszValue, cchValue);
	}

	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}
