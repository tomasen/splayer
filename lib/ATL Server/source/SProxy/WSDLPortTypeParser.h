//
// WSDLPortTypeParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

class CWSDLPortType;

class CWSDLPortTypeParser : public CParserBase
{
private:

	CWSDLPortType * m_pPortType;

public:

	inline CWSDLPortTypeParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CWSDLPortType * pPortType = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pPortType(pPortType)
	{
	}

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("documentation", WSDL_NAMESPACEA, OnDocumentation)
		XMLTAG_ENTRY_EX("operation", WSDL_NAMESPACEA, OnOperation)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnDocumentation);
	TAG_METHOD_DECL(OnOperation);

	ATTR_METHOD_DECL(OnName);

	inline CWSDLPortType * GetPortType()
	{
		return m_pPortType;
	}

	inline void SetPortType(CWSDLPortType * pPortType)
	{
		ATLASSERT( pPortType != NULL );

		m_pPortType = pPortType;
	}

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);
};