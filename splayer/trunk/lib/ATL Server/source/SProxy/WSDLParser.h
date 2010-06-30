//
// WSDLParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

typedef NAMESPACEMAP IMPORTMAP;

class CWSDLDocument;

class CWSDLParser : public CParserBase
{
private:

	IMPORTMAP m_importMap;

	CAutoPtr<CWSDLDocument> m_pDocument;

public:
	CWSDLParser();
	CWSDLParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel);

	//
	// Parsing maps
	//
	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX( "definitions", WSDL_NAMESPACEA, OnDefinitions )
		XMLTAG_ENTRY(    "import", OnImport )
		XMLTAG_ENTRY_EX( "documentation", WSDL_NAMESPACEA, OnDocumentation )
		XMLTAG_ENTRY_EX( "types", WSDL_NAMESPACEA, OnTypes )
		XMLTAG_ENTRY_EX( "message", WSDL_NAMESPACEA, OnMessage )
		XMLTAG_ENTRY_EX( "portType", WSDL_NAMESPACEA, OnPortType )
		XMLTAG_ENTRY_EX( "binding", WSDL_NAMESPACEA, OnBinding )
		XMLTAG_ENTRY_EX( "service", WSDL_NAMESPACEA, OnService )
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY( "name", OnName )
		XMLATTR_ENTRY( "targetNamespace", OnTargetNamespace )
	END_XMLATTR_MAP()

	//
	// Parse functions
	//
	TAG_METHOD_DECL( OnDefinitions );
	TAG_METHOD_DECL( OnImport );
	TAG_METHOD_DECL( OnDocumentation );
	TAG_METHOD_DECL( OnTypes );
	TAG_METHOD_DECL( OnMessage );
	TAG_METHOD_DECL( OnPortType );
	TAG_METHOD_DECL( OnBinding );
	TAG_METHOD_DECL( OnService );

	ATTR_METHOD_DECL( OnName );
	ATTR_METHOD_DECL( OnTargetNamespace );

	inline void SetWSDLDocument(CWSDLDocument *pDoc)
	{
		m_pDocument.Free();
		m_pDocument.Attach(pDoc);
	}

	inline CWSDLDocument * GetWSDLDocument(BOOL bReleaseOwnership = FALSE)
	{
		if (m_pDocument == NULL)
		{
			CreateWSDLDocument();
		}

		if (bReleaseOwnership != FALSE)
		{
			return m_pDocument.Detach();
		}
		return m_pDocument;
	}

	CWSDLDocument * CreateWSDLDocument();

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);
};