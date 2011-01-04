//
// WSDLParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "Util.h"

#include "WSDLParser.h"
#include "WSDLTypesParser.h"
#include "WSDLMessageParser.h"
#include "WSDLPortTypeParser.h"
#include "WSDLBindingParser.h"
#include "WSDLServiceParser.h"

#include "WSDLDocument.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

CWSDLParser::CWSDLParser()
	:m_pDocument(NULL)
{
}

CWSDLParser::CWSDLParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel)
	:CParserBase(pReader, pParent, dwLevel), m_pDocument(NULL)
{
}

TAG_METHOD_IMPL( CWSDLParser, OnDefinitions )
{
	TRACE_PARSE_ENTRY();

	return S_OK;
}

TAG_METHOD_IMPL( CWSDLParser, OnImport )
{
	//
	// TODO: parse import (?)
	//
	TRACE_PARSE_ENTRY();

	CStringW strNs;
	HRESULT hr = GetAttribute(pAttributes, L"namespace", sizeof("namespace")-1, strNs);
	if (SUCCEEDED(hr))
	{
		CStringW strLoc;
		hr = GetAttribute(pAttributes, L"location", sizeof("location")-1, strLoc);
		if (SUCCEEDED(hr))
		{
			CStringW localStrLoc = GetDiscoMapDocument()->GetValue(strLoc);
			if(!localStrLoc.IsEmpty())
				strLoc = localStrLoc;

			if (m_importMap.SetAt(strNs, strLoc) != NULL)
			{
				return S_OK;
			}
			EmitErrorHr(E_OUTOFMEMORY);
		}
		else
		{
			OnMissingAttribute(TRUE, L"location", sizeof("location")-1, L"", 0);
		}
	}
	else
	{
		OnMissingAttribute(TRUE, L"location", sizeof("location")-1, L"", 0);
	}
	return E_FAIL;
}

TAG_METHOD_IMPL( CWSDLParser, OnDocumentation )
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

TAG_METHOD_IMPL( CWSDLParser, OnTypes )
{
	TRACE_PARSE_ENTRY();

	CWSDLDocument *pCurr = GetWSDLDocument();
	if (pCurr != NULL)
	{
		CWSDLType *pElem = pCurr->AddType();
		if (pElem != NULL)
		{
			SetXMLElementInfo(pElem, pCurr, GetLocator());
			pElem->SetParentDocument(pCurr);

			CAutoPtr<CWSDLTypesParser> p( new CWSDLTypesParser(GetReader(), this, GetLevel(), pElem));
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

TAG_METHOD_IMPL( CWSDLParser, OnMessage )
{
	TRACE_PARSE_ENTRY();

	CWSDLDocument *pCurr = GetWSDLDocument();
	if (pCurr != NULL)
	{
		CAutoPtr<CWSDLMessage> spElem;
		spElem.Attach( new CWSDLMessage );
		if (spElem != NULL)
		{
			SetXMLElementInfo(spElem, pCurr, GetLocator());
			spElem->SetParentDocument(pCurr);

			CAutoPtr<CWSDLMessageParser> p( new CWSDLMessageParser(GetReader(), this, GetLevel(), spElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					if (SUCCEEDED(p.Detach()->GetAttributes(pAttributes)))
					{
						if (pCurr->AddMessage(spElem) != NULL)
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

TAG_METHOD_IMPL( CWSDLParser, OnPortType )
{
	TRACE_PARSE_ENTRY();

	CWSDLDocument *pCurr = GetWSDLDocument();
	if (pCurr != NULL)
	{
		CAutoPtr<CWSDLPortType> spElem;
		spElem.Attach( new CWSDLPortType );
		if (spElem != NULL)
		{
			SetXMLElementInfo(spElem, pCurr, GetLocator());
			spElem->SetParentDocument(pCurr);

			CAutoPtr<CWSDLPortTypeParser> p( new CWSDLPortTypeParser(GetReader(), this, GetLevel(), spElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					if (SUCCEEDED(p.Detach()->GetAttributes(pAttributes)))
					{
						if (pCurr->AddPortType(spElem) != NULL)
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

TAG_METHOD_IMPL( CWSDLParser, OnBinding )
{
	TRACE_PARSE_ENTRY();

	CWSDLDocument *pCurr = GetWSDLDocument();
	if (pCurr != NULL)
	{
		CAutoPtr<CWSDLBinding> spElem;
		spElem.Attach( new CWSDLBinding );
		if (spElem != NULL)
		{
			SetXMLElementInfo(spElem, pCurr, GetLocator());
			spElem->SetParentDocument(pCurr);

			CWSDLBindingParser * p = new CWSDLBindingParser(GetReader(), this, GetLevel(), spElem);
			if (p != NULL)
			{
				if (SUCCEEDED(p->GetAttributes(pAttributes)))
				{
					if (pCurr->AddBinding(spElem) != NULL)
					{
						spElem.Detach();
						return S_OK;
					}
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

TAG_METHOD_IMPL( CWSDLParser, OnService )
{
	TRACE_PARSE_ENTRY();
	
	CWSDLDocument *pCurr = GetWSDLDocument();
	if (pCurr != NULL)
	{
		CAutoPtr<CWSDLService> spElem;
		spElem.Attach( new CWSDLService );
		if (spElem != NULL)
		{
			SetXMLElementInfo(spElem, pCurr, GetLocator());
			spElem->SetParentDocument(pCurr);

			CAutoPtr<CWSDLServiceParser> p( new CWSDLServiceParser(GetReader(), this, GetLevel(), spElem) );
			if (p != NULL)
			{
				if (g_ParserList.AddHead(p) != NULL)
				{
					if (SUCCEEDED(p.Detach()->GetAttributes(pAttributes)))
					{
						if (pCurr->AddService(spElem) != NULL)
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

ATTR_METHOD_IMPL( CWSDLParser, OnName )
{
	TRACE_PARSE_ENTRY();

	CWSDLDocument * pCurr = GetWSDLDocument();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL( CWSDLParser, OnTargetNamespace )
{
	TRACE_PARSE_ENTRY();

	CWSDLDocument * pCurr = GetWSDLDocument();
	if (pCurr != NULL)
	{
		return pCurr->SetTargetNamespace(wszValue, cchValue);
	}

	EmitError(IDS_SDL_INTERNAL);

	return E_FAIL;
}

CWSDLDocument * CWSDLParser::CreateWSDLDocument()
{
	m_pDocument.Attach( new CWSDLDocument );
	return m_pDocument;
}

HRESULT __stdcall CWSDLParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLDocument * pCurr = GetWSDLDocument();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}

	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}