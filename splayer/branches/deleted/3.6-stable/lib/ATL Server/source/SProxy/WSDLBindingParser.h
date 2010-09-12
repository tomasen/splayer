//
// WSDLBindingParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

class CWSDLBinding;

class CWSDLBindingParser : public CParserBase
{
private:

	CWSDLBinding *m_pBinding;

public:

	inline CWSDLBindingParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CWSDLBinding *pBinding = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pBinding(pBinding)
	{
	}

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("binding", SOAP_NAMESPACEA, OnSoapBinding)
		XMLTAG_ENTRY_EX("operation", WSDL_NAMESPACEA, OnOperation)
		XMLTAG_ENTRY_EX("binding", HTTP_NAMESPACEA, OnHttpBinding)
		XMLTAG_ENTRY_EX("documentation", WSDL_NAMESPACEA, OnDocumentation)
		
		// extensibility elements
//		XMLTAG_ENTRY_EX("class", SUDS_NAMESPACEA, OnSudsClass)
//		XMLTAG_ENTRY_EX("binding", STK_PREFERREDENCODING_NAMESPACEA, OnStkPreferredBinding)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
		XMLATTR_ENTRY("type", OnType)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnDocumentation);
	TAG_METHOD_DECL(OnOperation);
	TAG_METHOD_DECL(OnSoapBinding);
	TAG_METHOD_DECL(OnHttpBinding);
	
//	TAG_METHOD_DECL(OnSudsClass);
//	TAG_METHOD_DECL(OnStkPreferredBinding);

	ATTR_METHOD_DECL(OnName);
	ATTR_METHOD_DECL(OnType);

	inline CWSDLBinding * GetBinding()
	{
		return m_pBinding;
	}

	inline void SetBinding(CWSDLBinding * pBinding)
	{
		ATLASSERT( pBinding != NULL );

		m_pBinding = pBinding;
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