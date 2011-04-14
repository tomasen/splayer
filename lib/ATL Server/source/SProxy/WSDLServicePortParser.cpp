//
// WSDLServicePortParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Util.h"

#include "WSDLServicePortParser.h"

#include "WSDLPort.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

TAG_METHOD_IMPL(CWSDLServicePortParser, OnSoapAddress)
{
	TRACE_PARSE_ENTRY();

	CWSDLPort * pCurr = GetPort();
	if (pCurr != NULL)
	{
		CStringW strAddress;
		if (S_OK == GetAttribute(pAttributes, L"location", sizeof("location")-1, strAddress))
		{
			if (SUCCEEDED(pCurr->SetSoapAddress(strAddress)))
			{
				return SkipElement();
			}
		}
		OnMissingAttribute(TRUE, L"location", sizeof("location")-1, L"", 0);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

TAG_METHOD_IMPL(CWSDLServicePortParser, OnHttpAddress)
{
	TRACE_PARSE_ENTRY();

	CWSDLPort * pCurr = GetPort();
	if (pCurr != NULL)
	{
		CStringW strAddress;
		if (S_OK == GetAttribute(pAttributes, L"location", sizeof("location")-1, strAddress))
		{
			if (SUCCEEDED(pCurr->SetHttpAddress(strAddress)))
			{
				return SkipElement();
			}
		}
		OnMissingAttribute(TRUE, L"location", sizeof("location")-1, L"", 0);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CWSDLServicePortParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CWSDLPort * pCurr = GetPort();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CWSDLServicePortParser, OnBinding)
{
	TRACE_PARSE_ENTRY();

	CWSDLPort * pCurr = GetPort();
	if (pCurr != NULL)
	{
		return pCurr->SetBinding(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

HRESULT __stdcall CWSDLServicePortParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLPort * pCurr = GetPort();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}

HRESULT CWSDLServicePortParser::OnUnrecognizedTag(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t * /*wszQName*/, int /*cchQName*/,
	ISAXAttributes * /*pAttributes*/) throw()
{
	CWSDLPort * pCurr = GetPort();
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