//
// WSDLServiceParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Util.h"

#include "WSDLServiceParser.h"
#include "WSDLServicePortParser.h"

#include "WSDLService.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

TAG_METHOD_IMPL(CWSDLServiceParser, OnDocumentation)
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLServiceParser, OnPort)
{
	TRACE_PARSE_ENTRY();

	CWSDLService *pCurr = GetService();
	if (pCurr != NULL)
	{
		CWSDLPort *pElem = pCurr->AddPort();
		if (pElem != NULL)
		{
			SetXMLElementInfo(pElem, pCurr, GetLocator());

			CAutoPtr<CWSDLServicePortParser> p( new CWSDLServicePortParser(GetReader(), this, GetLevel(), pElem) );
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

ATTR_METHOD_IMPL(CWSDLServiceParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CWSDLService * pCurr = GetService();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

HRESULT __stdcall CWSDLServiceParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLService * pCurr = GetService();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}

HRESULT CWSDLServiceParser::OnUnrecognizedTag(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t * /*wszQName*/, int /*cchQName*/,
	ISAXAttributes * /*pAttributes*/) throw()
{
	CWSDLService * pCurr = GetService();
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