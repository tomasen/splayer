//
// WSDLPortTypeParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Util.h"

#include "WSDLPortTypeParser.h"
#include "WSDLOperationParser.h"

#include "WSDLPortType.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

TAG_METHOD_IMPL(CWSDLPortTypeParser, OnDocumentation)
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLPortTypeParser, OnOperation)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortType * pCurr = GetPortType();
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

ATTR_METHOD_IMPL(CWSDLPortTypeParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortType *pCurr = GetPortType();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

HRESULT __stdcall CWSDLPortTypeParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLPortType * pCurr = GetPortType();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}