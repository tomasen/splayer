//
// WSDLTypesParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

#include "WSDLTypesParser.h"
#include "SchemaParser.h"

#include "WSDLType.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

TAG_METHOD_IMPL(CWSDLTypesParser, OnDocumentation)
{
	TRACE_PARSE_ENTRY();
	
	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLTypesParser, OnSchema)
{
	TRACE_PARSE_ENTRY();

	CWSDLType *pCurr = GetType();
	if (pCurr != NULL)
	{
		CSchema *pElem = pCurr->AddSchema();
		if (pElem != NULL)
		{
			SetXMLElementInfo(pElem, pCurr, GetLocator());

			CAutoPtr<CSchemaParser> p( new CSchemaParser(GetReader(), this, GetLevel(), pElem) );
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

HRESULT __stdcall CWSDLTypesParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLType *pCurr = GetType();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}

HRESULT CWSDLTypesParser::OnUnrecognizedTag(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t * /*wszQName*/, int /*cchQName*/,
	ISAXAttributes * /*pAttributes*/) throw()
{
	CWSDLType *pCurr = GetType();
	if (pCurr != NULL)
	{
		int nLine;
		int nCol;
		GetLocator()->getLineNumber(&nLine);
		GetLocator()->getColumnNumber(&nCol);
		
		EmitFileWarning(IDS_SDL_SKIP_EXTENSIBILITY, 
			pCurr->GetParentDocument()->GetDocumentUri(), 
			nLine, 
			nCol, 
			0, 
			wszNamespaceUri,
			wszLocalName);
	}
	return SkipElement();
}