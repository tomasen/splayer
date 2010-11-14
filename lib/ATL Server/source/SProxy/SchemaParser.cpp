//
// SchemaParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "SchemaParser.h"
#include "ComplexTypeParser.h"
#include "ElementParser.h"
#include "SimpleTypeParser.h"
#include "Schema.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

TAG_METHOD_IMPL(CSchemaParser, OnInclude)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSchemaParser, OnImport)
{
	TRACE_PARSE_ENTRY();

	//
	// TODO: do imports here (or delay?)
	//

	return SkipElement();
}

TAG_METHOD_IMPL(CSchemaParser, OnAnnotation)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSchemaParser, OnRedefine)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSchemaParser, OnAttribute)
{
	TRACE_PARSE_ENTRY();
/*
	// TODO: investigate supporting this stuff
	CSchema *pCurr = GetSchema();

	if (pCurr != NULL)
	{
		CAttribute *pElem = pCurr->AddAttribute();
		if (pElem != NULL)
		{
			pElem->SetParentDocument(pCurr->GetParentDocument());
			pElem->SetParentElement(pCurr);

			CAutoPtr<CAttributeParser> p( new CAttributeParser(GetReader(), this, GetLevel(), pElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					p.Detach();
					return p->GetAttributes(pAttributes);
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
*/

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	//
	// TODO: investigate supporting this
	//

	return SkipElement();
}

TAG_METHOD_IMPL(CSchemaParser, OnAttributeGroup)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSchemaParser, OnElement)
{
	TRACE_PARSE_ENTRY();

	CSchema * pCurr = GetSchema();
	if (pCurr != NULL)
	{
		CAutoPtr<CElement> spElem;
		spElem.Attach( new CElement );
		if (spElem != NULL)
		{
			SetXSDElementInfo(spElem, pCurr, GetLocator());
			spElem->SetParentSchema(pCurr);

			CAutoPtr<CElementParser> p( new CElementParser(GetReader(), this, GetLevel(), spElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					if (SUCCEEDED(p.Detach()->GetAttributes(pAttributes)))
					{
						if (spElem->GetName().GetLength() != 0)
						{
							if (pCurr->AddElement(spElem) != NULL)
							{
								spElem.Detach();
								return S_OK;
							}
						}
						EmitNamedElementError("element");
					}
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

TAG_METHOD_IMPL(CSchemaParser, OnGroup)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSchemaParser, OnNotation)
{
	TRACE_PARSE_ENTRY();

	MarkUnsupported(wszQName, cchQName);
	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CSchemaParser, OnSimpleType)
{
	TRACE_PARSE_ENTRY();

	CSchema * pCurr = GetSchema();
	if (pCurr != NULL)
	{
		CAutoPtr<CSimpleType> spElem;
		spElem.Attach( new CSimpleType );
		if (spElem != NULL)
		{
			SetXSDElementInfo(spElem, pCurr, GetLocator());
			spElem->SetParentSchema(pCurr);

			CAutoPtr<CSimpleTypeParser> p( new CSimpleTypeParser(GetReader(), this, GetLevel(), spElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					if (SUCCEEDED(p.Detach()->GetAttributes(pAttributes)))
					{
						if (spElem->GetName().GetLength() != 0)
						{
							if (pCurr->AddSimpleType(spElem) != NULL)
							{
								spElem.Detach();
								return S_OK;
							}
						}
						EmitNamedElementError("simpleType");
					}
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}

TAG_METHOD_IMPL(CSchemaParser, OnComplexType)
{
	TRACE_PARSE_ENTRY();

	CSchema * pCurr = GetSchema();
	if (pCurr != NULL)
	{
		CAutoPtr<CComplexType> spElem;
		spElem.Attach( new CComplexType );
		if (spElem != NULL)
		{
			SetXSDElementInfo(spElem, pCurr, GetLocator());
			spElem->SetParentSchema(pCurr);

			CAutoPtr<CComplexTypeParser> p( new CComplexTypeParser(GetReader(), this, GetLevel(), spElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					if (SUCCEEDED(p.Detach()->GetAttributes(pAttributes)))
					{
						if (spElem->GetName().GetLength() != 0)
						{
							if (pCurr->AddComplexType(spElem) != NULL)
							{
								spElem.Detach();
								return S_OK;
							}
						}
						EmitNamedElementError("complexType");
					}
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}


ATTR_METHOD_IMPL(CSchemaParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CSchema * pCurr = GetSchema();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CSchemaParser, OnTargetNamespace)
{
	TRACE_PARSE_ENTRY();

	CSchema * pCurr = GetSchema();
	if (pCurr != NULL)
	{
		return pCurr->SetTargetNamespace(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CSchemaParser, OnID)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CSchemaParser, OnAttributeFormDefault)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CSchemaParser, OnBlockDefault)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CSchemaParser, OnElementFormDefault)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CSchemaParser, OnFinalDefault)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

ATTR_METHOD_IMPL(CSchemaParser, OnVersion)
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

HRESULT __stdcall CSchemaParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CSchema * pCurr = GetSchema();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}

void CSchemaParser::EmitNamedElementError(const char *szElem)
{
	int nLine = 0;
	int nCol = 0;
	GetLocator()->getLineNumber(&nLine);
	GetLocator()->getColumnNumber(&nCol);
	CSchema *pCurr = GetSchema();
	if (pCurr != NULL)
	{
		EmitFileError(IDS_SDL_SCHEMALEVEL_NAME, 
			(LPCWSTR) pCurr->GetParentDocument()->GetDocumentUri(), 
			nLine, nCol, 0, szElem);
	}
	EmitError(IDS_SDL_INTERNAL);
}