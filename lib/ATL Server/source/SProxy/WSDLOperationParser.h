//
// WSDLOperationParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"
#include "WSDLPortTypeIO.h"


class CWSDLPortTypeOperation;

class CWSDLOperationParser : public CParserBase
{
private:

	CWSDLPortTypeOperation *m_pOperation;

public:
	
	inline CWSDLOperationParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CWSDLPortTypeOperation *pOperation = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pOperation(pOperation)
	{
	}

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("input", WSDL_NAMESPACEA, OnInput)
		XMLTAG_ENTRY_EX("output", WSDL_NAMESPACEA, OnOutput)
		XMLTAG_ENTRY_EX("fault", WSDL_NAMESPACEA, OnFault)
		XMLTAG_ENTRY_EX("operation", SOAP_NAMESPACEA, OnSoapOperation)
		XMLTAG_ENTRY_EX("operation", HTTP_NAMESPACEA, OnHttpOperation)
		XMLTAG_ENTRY_EX("documentation", WSDL_NAMESPACEA, OnDocumentation)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnDocumentation);
	TAG_METHOD_DECL(OnInput);
	TAG_METHOD_DECL(OnOutput);
	TAG_METHOD_DECL(OnFault);
	TAG_METHOD_DECL(OnSoapOperation);
	TAG_METHOD_DECL(OnHttpOperation);

	ATTR_METHOD_DECL(OnName);

	inline CWSDLPortTypeOperation * GetOperation()
	{
		return m_pOperation;
	}

	inline void SetOperation(CWSDLPortTypeOperation *pOperation)
	{
		ATLASSERT( pOperation != NULL );

		m_pOperation = pOperation;
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