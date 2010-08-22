//
// WSDLMessageParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "Util.h"
#include "WSDLMessageParser.h"

#include "WSDLMessage.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

TAG_METHOD_IMPL(CWSDLMessageParser, OnDocumentation)
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLMessageParser, OnPart)
{
	TRACE_PARSE_ENTRY();

	CWSDLMessage *pCurr = GetMessage();
	if (pCurr != NULL)
	{
		CWSDLMessagePart *pPart = pCurr->AddPart();
		if (pPart != NULL)
		{
			SetXMLElementInfo(pPart, pCurr, GetLocator());

			CStringW strName;
			if (S_OK == GetAttribute(pAttributes, L"name", sizeof("name")-1, strName))
			{
				pPart->SetName(strName);
				
				CStringW strElement;
				if (S_OK == GetAttribute(pAttributes, L"element", sizeof("element")-1, strElement))
				{
					pPart->SetElement(strElement);
				}
				CStringW strType;
				if (S_OK == GetAttribute(pAttributes, L"type", sizeof("type")-1, strType))
				{
					pPart->SetType(strType);
				}
//				else
//				{
//					OnMissingAttribute(TRUE, L"element", sizeof("element")-1, L"", 0);
//				}

				return SkipElement();
			}
			OnMissingAttribute(TRUE, L"name", sizeof("name")-1, L"", 0);
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CWSDLMessageParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CWSDLMessage *pCurr = GetMessage();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

HRESULT __stdcall CWSDLMessageParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLMessage *pCurr = GetMessage();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}