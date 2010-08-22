//
// WSDLMessageParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

class CWSDLMessage;

class CWSDLMessageParser : public CParserBase
{
private:

	CWSDLMessage * m_pMessage;

public:

	inline CWSDLMessageParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CWSDLMessage *pMessage = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pMessage(pMessage)
	{
	}

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("documentation", WSDL_NAMESPACEA, OnDocumentation)
		XMLTAG_ENTRY_EX("part", WSDL_NAMESPACEA, OnPart)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnDocumentation);
	TAG_METHOD_DECL(OnPart);

	ATTR_METHOD_DECL(OnName);

	inline CWSDLMessage * GetMessage()
	{
		return m_pMessage;
	}

	inline void SetMessage(CWSDLMessage *pMessage)
	{
		ATLASSERT( m_pMessage == NULL );

		m_pMessage = pMessage;
	}

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);
};