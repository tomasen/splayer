//
// XMLDocParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"
#include "XMLDocument.h"

class CXMLDocParser : public CParserBase
{
	XMLDOCTYPE m_docType;

public:

	CXMLDocParser();
	CXMLDocParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel);

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("schema", XSD_NAMESPACEA, OnSchema)
		XMLTAG_ENTRY_EX("definitions", WSDL_NAMESPACEA, OnDefinitions)
	END_XMLTAG_MAP()

	TAG_METHOD_DECL( OnSchema );
	TAG_METHOD_DECL( OnDefinitions );

	inline void SetExpectingDocType(XMLDOCTYPE docType)
	{
		m_docType = docType;
	}

	//
	// override OnUnrecognizedTag
	//
	HRESULT OnUnrecognizedTag(
		const wchar_t *wszNamespaceUri, int cchNamespaceUri,
		const wchar_t *wszLocalName, int cchLocalName,
		const wchar_t *wszQName, int cchQName,
		ISAXAttributes *pAttributes) throw();
};