//
// WSDLPortTypeIOParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

class CWSDLPortTypeIO;

class CWSDLOperationIOParser : public CParserBase
{
private:

	CWSDLPortTypeIO * m_pIO;

public:

	inline CWSDLOperationIOParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CWSDLPortTypeIO * pIO = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pIO(pIO)
	{
	}

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("body", SOAP_NAMESPACEA, OnSoapBody)
		XMLTAG_ENTRY_EX("header", SOAP_NAMESPACEA, OnSoapHeader)
		XMLTAG_ENTRY_EX("headerfault", SOAP_NAMESPACEA, OnSoapHeaderFault)
		XMLTAG_ENTRY_EX("fault", SOAP_NAMESPACEA, OnSoapFault)
		XMLTAG_ENTRY_EX("content", MIME_NAMESPACEA, OnMimeContent)
		XMLTAG_ENTRY_EX("mimeXml", MIME_NAMESPACEA, OnMimeXML)
		XMLTAG_ENTRY_EX("multipartRelated", MIME_NAMESPACEA, OnMimeMultipartRelated)
		XMLTAG_ENTRY_EX("urlEncoded", HTTP_NAMESPACEA, OnHttpUrlEncoded)
		XMLTAG_ENTRY_EX("documentation", WSDL_NAMESPACEA, OnDocumentation)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
		XMLATTR_ENTRY("message", OnMessage)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnSoapBody);
	TAG_METHOD_DECL(OnDocumentation);
	TAG_METHOD_DECL(OnSoapHeader);
	TAG_METHOD_DECL(OnSoapHeaderFault);
	TAG_METHOD_DECL(OnSoapFault);
	TAG_METHOD_DECL(OnMimeContent);
	TAG_METHOD_DECL(OnMimeXML);
	TAG_METHOD_DECL(OnMimeMultipartRelated);
	TAG_METHOD_DECL(OnHttpUrlEncoded);

	ATTR_METHOD_DECL(OnName);
	ATTR_METHOD_DECL(OnMessage);

	inline CWSDLPortTypeIO * GetIO()
	{
		return m_pIO;
	}

	inline void SetIO(CWSDLPortTypeIO * pIO)
	{
		ATLASSERT( pIO != NULL );

		m_pIO = pIO;
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