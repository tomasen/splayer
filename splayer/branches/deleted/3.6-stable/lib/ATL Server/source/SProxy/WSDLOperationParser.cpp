//
// WSDLOperationParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Util.h"

#include "WSDLOperationParser.h"
#include "WSDLOperationIOParser.h"

#include "WSDLPortTypeOperation.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

TAG_METHOD_IMPL(CWSDLOperationParser, OnDocumentation)
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLOperationParser, OnInput)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeOperation * pCurr = GetOperation();
	if (pCurr != NULL)
	{
		CWSDLPortTypeInput * pElem = pCurr->AddInput();
		if (pElem != NULL)
		{
			SetXMLElementInfo(pElem, pCurr, GetLocator());

			CAutoPtr<CWSDLOperationIOParser> p( new CWSDLOperationIOParser(GetReader(), this, GetLevel(), pElem) );
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

TAG_METHOD_IMPL(CWSDLOperationParser, OnOutput)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeOperation * pCurr = GetOperation();
	if (pCurr != NULL)
	{
		CWSDLPortTypeOutput * pElem = pCurr->AddOutput();
		if (pElem != NULL)
		{
			SetXMLElementInfo(pElem, pCurr, GetLocator());
			CAutoPtr<CWSDLOperationIOParser> p( new CWSDLOperationIOParser(GetReader(), this, GetLevel(), pElem) );
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

TAG_METHOD_IMPL(CWSDLOperationParser, OnFault)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeOperation * pCurr = GetOperation();
	if (pCurr != NULL)
	{
		CWSDLPortTypeFault * pElem = pCurr->AddFault();
		if (pElem != NULL)
		{
			SetXMLElementInfo(pElem, pCurr, GetLocator());
			CAutoPtr<CWSDLOperationIOParser> p( new CWSDLOperationIOParser(GetReader(), this, GetLevel(), pElem) );
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

TAG_METHOD_IMPL(CWSDLOperationParser, OnSoapOperation)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeOperation * pCurr = GetOperation();
	if (pCurr != NULL)
	{
		CSoapOperation *pOperation = pCurr->AddSoapOperation();
		if (pOperation != NULL)
		{
			CStringW strSoapAction;
			if (S_OK == GetAttribute(pAttributes, L"soapAction", sizeof("soapAction")-1, strSoapAction))
			{
				pOperation->SetSoapAction(strSoapAction);
			}

			const wchar_t *wszStyle;
			int cchStyle;
			HRESULT hr = S_OK;
			if (S_OK == GetAttribute(pAttributes, L"style", sizeof("style")-1, &wszStyle, &cchStyle))
			{
				hr = pOperation->SetStyle(wszStyle, cchStyle);
				if (FAILED(hr))
				{
					EmitInvalidValue("style", wszStyle);
				}
			}

			if (SUCCEEDED(hr))
			{
				return SkipElement();
			}
			else
			{
				OnMissingAttribute(TRUE, L"style", sizeof("style")-1, L"", 0);
			}
		}

	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

TAG_METHOD_IMPL(CWSDLOperationParser, OnHttpOperation)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeOperation * pCurr = GetOperation();
	if (pCurr != NULL)
	{
		CHttpOperation *pOperation = pCurr->AddHttpOperation();
		if (pOperation != NULL)
		{
			CStringW strLocation;
			if (S_OK == GetAttribute(pAttributes, L"location", sizeof("location")-1, strLocation))
			{
				if (SUCCEEDED(pOperation->SetLocation(strLocation)))
				{
					return SkipElement();
				}
			}
			OnMissingAttribute(TRUE, L"location", sizeof("location")-1, L"", 0);
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CWSDLOperationParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeOperation * pCurr = GetOperation();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

HRESULT __stdcall CWSDLOperationParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLPortTypeOperation * pCurr = GetOperation();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}

HRESULT CWSDLOperationParser::OnUnrecognizedTag(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t * /*wszQName*/, int /*cchQName*/,
	ISAXAttributes * /*pAttributes*/) throw()
{
	CWSDLPortTypeOperation * pCurr = GetOperation();
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