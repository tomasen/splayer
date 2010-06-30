//
// WSDLTypesParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

class CWSDLType;

class CWSDLTypesParser : public CParserBase
{
private:

	CWSDLType * m_pType;

public:

	inline CWSDLTypesParser()
	{
	}

	inline CWSDLTypesParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CWSDLType * pType = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pType(pType)
	{
	}

	inline CWSDLType * GetType()
	{
		return m_pType;
	}

	inline void SetType(CWSDLType * pType)
	{
		m_pType = pType;
	}

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("documentation", WSDL_NAMESPACEA, OnDocumentation)
		XMLTAG_ENTRY_EX("schema", XSD_NAMESPACEA, OnSchema)
	END_XMLTAG_MAP()

	EMPTY_XMLATTR_MAP()

	TAG_METHOD_DECL(OnDocumentation);
	TAG_METHOD_DECL(OnSchema);

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