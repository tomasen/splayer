//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include "stdafx.h"
#include "util.h"
#include "DiscoMapparser.h"
#include "DiscoMapDocument.h"
CDiscoMapParser::CDiscoMapParser(void)
:m_pDocument(NULL)
{
}

CDiscoMapParser::~CDiscoMapParser(void)
{
}

CDiscoMapParser::CDiscoMapParser(ISAXXMLReader * pReader, CParserBase * pParent, DWORD dwLevel)
:CParserBase(pReader, pParent, dwLevel), m_pDocument(NULL)
{
}

CDiscoMapDocument * CDiscoMapParser::CreateDiscoMapDocument(void)
{
	m_pDocument.Attach( new CDiscoMapDocument );
	return m_pDocument;
}


TAG_METHOD_IMPL(CDiscoMapParser, OnDiscoveryClientResultsFile)
{
	TRACE_PARSE_ENTRY();
	return S_OK;
}

TAG_METHOD_IMPL(CDiscoMapParser, OnResults)
{
	TRACE_PARSE_ENTRY();
	return S_OK;
}

TAG_METHOD_IMPL(CDiscoMapParser, OnDiscoveryClientResult)
{
	TRACE_PARSE_ENTRY();

	CStringW strRT;
	HRESULT hr = GetAttribute(pAttributes, L"referenceType", sizeof("referenceType")-1, strRT);
	
	if(FAILED(hr))
		return hr;

	CDiscoMapDocument * pDoc = GetDiscoMapDocument();

	if(strRT == "System.Web.Services.Discovery.SchemaReference")
	{
		CStringW strURL;
		hr = GetAttribute(pAttributes, L"url", sizeof("url")-1, strURL);
		if(FAILED(hr))
			return hr;

		CStringW strFileName;
		hr = GetAttribute(pAttributes, L"filename", sizeof("filename")-1, strFileName);
		if(FAILED(hr))
			return hr;
		
		pDoc->AddSchema(strURL,strFileName);
		
		return S_OK;
	}
	
	if(strRT == "System.Web.Services.Discovery.ContractReference")
	{
		CStringW strWSDLFile;
		hr = GetAttribute(pAttributes, L"filename", sizeof("filename")-1, strWSDLFile);
		if(FAILED(hr))
			return hr;
		
		pDoc->SetWSDLFile(strWSDLFile);
		
	}

	return S_OK;
}
