//
// WSDLServicePortParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

class CWSDLPort;

class CWSDLServicePortParser : public CParserBase
{
private:

	CWSDLPort *m_pPort;

public:

	inline CWSDLServicePortParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CWSDLPort *pPort = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pPort(pPort)
	{
	}

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("address", SOAP_NAMESPACEA, OnSoapAddress)
		XMLTAG_ENTRY_EX("address", HTTP_NAMESPACEA, OnHttpAddress)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
		XMLATTR_ENTRY("binding", OnBinding)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnSoapAddress);
	TAG_METHOD_DECL(OnHttpAddress);

	ATTR_METHOD_DECL(OnName);
	ATTR_METHOD_DECL(OnBinding);

	inline CWSDLPort * GetPort()
	{
		return m_pPort;
	}

	inline void SetPort(CWSDLPort *pPort)
	{
		ATLASSERT( pPort != NULL );

		m_pPort = pPort;
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
		ISAXAttributes *pAttributes) throw();
};