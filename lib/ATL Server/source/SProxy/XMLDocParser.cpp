//
// XMLDocParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "XMLDocParser.h"
#include "resource.h"

CXMLDocParser::CXMLDocParser()
	:m_docType(UNKDOC)
{
}

CXMLDocParser::CXMLDocParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel)
	:CParserBase(pReader, pParent, dwLevel), m_docType(UNKDOC)
{
}

TAG_METHOD_IMPL( CXMLDocParser, OnSchema )
{
	if (m_docType != UNKDOC && m_docType != SCHEMADOC)
	{
		ATLTRACE( _T("Not a schema document.  Unknown root document tag: %.*ws\n"), cchLocalName, wszLocalName );
	}
	return S_OK;
}

TAG_METHOD_IMPL( CXMLDocParser, OnDefinitions )
{
	if (m_docType != UNKDOC && m_docType != WSDLDOC)
	{
		ATLTRACE( _T("Not a WSDL document.  Unknown root document tag: %.*ws\n"), cchLocalName, wszLocalName );
	}
	return S_OK;
}

HRESULT CXMLDocParser::OnUnrecognizedTag(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t *wszQName, int cchQName,
	ISAXAttributes *pAttributes) throw()
{
	const wchar_t *wszUrl;
	if (SUCCEEDED(GetReader()->getBaseURL(&wszUrl)))
	{
		EmitError(IDS_SDL_UNRECOGNIZED_DOC, wszUrl, wszNamespaceUri, wszLocalName);
	}

	return CParserBase::OnUnrecognizedTag(wszNamespaceUri, cchNamespaceUri, 
		wszLocalName, cchLocalName,
		wszQName, cchQName,
		pAttributes);
}