//
// WSDLServiceParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

class CWSDLService;

class CWSDLServiceParser : public CParserBase
{
private:

	CWSDLService *m_pService;

public:

	inline CWSDLServiceParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CWSDLService *pService = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pService(pService)
	{
	}

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("documentation", WSDL_NAMESPACEA, OnDocumentation)
		XMLTAG_ENTRY_EX("port", WSDL_NAMESPACEA, OnPort)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnDocumentation);
	TAG_METHOD_DECL(OnPort);

	ATTR_METHOD_DECL(OnName);

	inline CWSDLService * GetService()
	{
		return m_pService;
	}

	inline void SetService(CWSDLService *pService)
	{
		ATLASSERT( pService != NULL );

		m_pService = pService;
	}

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);
	
	HRESULT OnUnrecognizedTag(
		const wchar_t *wszNamespaceUri, int cchNamespaceUri,
		const wchar_t *wszLocalName, int cchLocalName,
		const wchar_t *wszQName, int cchQName,
		ISAXAttributes *pAttributes) throw() ;
};