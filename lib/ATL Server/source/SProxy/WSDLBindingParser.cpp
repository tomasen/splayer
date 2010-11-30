//
// WSDLBindingParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Util.h"

#include "WSDLBindingParser.h"
#include "WSDLOperationParser.h"

#include "WSDLBinding.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

TAG_METHOD_IMPL(CWSDLBindingParser, OnDocumentation)
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLBindingParser, OnOperation)
{
	TRACE_PARSE_ENTRY();
	
	CWSDLBinding * pCurr = GetBinding();
	if (pCurr != NULL)
	{
		CAutoPtr<CWSDLPortTypeOperation> spElem;
		spElem.Attach( new CWSDLPortTypeOperation );

		if (spElem != NULL)
		{
			SetXMLElementInfo(spElem, pCurr, GetLocator());

			CAutoPtr<CWSDLOperationParser> p( new CWSDLOperationParser(GetReader(), this, GetLevel(), spElem) );
			if (p)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					if (SUCCEEDED(p.Detach()->GetAttributes(pAttributes)))
					{
						if (pCurr->AddOperation(spElem) != NULL)
						{
							spElem.Detach();
							return S_OK;
						}
					}
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}

TAG_METHOD_IMPL(CWSDLBindingParser, OnSoapBinding)
{
	TRACE_PARSE_ENTRY();

	CWSDLBinding * pCurr = GetBinding();
	if (pCurr != NULL)
	{
		CSoapBinding *pBinding = pCurr->AddSoapBinding();
		if (pBinding != NULL)
		{
			SetXMLElementInfo(pBinding, pCurr, GetLocator());

			CStringW strTransport;
			if (S_OK == GetAttribute(pAttributes, L"transport", sizeof("transport")-1, strTransport))
			{
				pBinding->SetTransport(strTransport);
			}

			const wchar_t *wszStyle;
			int cchStyle;
			HRESULT hr = S_OK;
			if (S_OK == GetAttribute(pAttributes, L"style", sizeof("style")-1, &wszStyle, &cchStyle))
			{
				hr = pBinding->SetStyle(wszStyle, cchStyle);
				if (FAILED(hr))
				{
					EmitInvalidValue("style", wszStyle);
				}
			}

			if (SUCCEEDED(hr))
			{
				return SkipElement();
			}
		}

	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

TAG_METHOD_IMPL(CWSDLBindingParser, OnHttpBinding)
{
	TRACE_PARSE_ENTRY();

	CWSDLBinding * pCurr = GetBinding();
	if (pCurr != NULL)
	{
		CHttpBinding *pBinding = pCurr->AddHttpBinding();
		if (pBinding != NULL)
		{
			SetXMLElementInfo(pBinding, pCurr, GetLocator());

			CStringW strVerb;
			if (S_OK == GetAttribute(pAttributes, L"verb", sizeof("verb")-1, strVerb))
			{
				if (SUCCEEDED(pBinding->SetVerb(strVerb)))
				{
					return SkipElement();
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

//TAG_METHOD_IMPL(CWSDLBindingParser, OnSudsClass)
//{
//	TRACE_PARSE_ENTRY();
//	
//	return SkipElement();
//}
//
//TAG_METHOD_IMPL(CWSDLBindingParser, OnStkPreferredBinding)
//{
//	TRACE_PARSE_ENTRY();
//	
//	return SkipElement();
//}

ATTR_METHOD_IMPL(CWSDLBindingParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CWSDLBinding * pCurr = GetBinding();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CWSDLBindingParser, OnType)
{
	TRACE_PARSE_ENTRY();

	CWSDLBinding * pCurr = GetBinding();
	if (pCurr != NULL)
	{
		return pCurr->SetType(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

HRESULT __stdcall CWSDLBindingParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLBinding * pCurr = GetBinding();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}

HRESULT CWSDLBindingParser::OnUnrecognizedTag(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t * /*wszQName*/, int /*cchQName*/,
	ISAXAttributes * /*pAttributes*/) throw()
{
	CWSDLBinding *pCurr = GetBinding();
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