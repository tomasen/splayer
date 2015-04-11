//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#pragma once
#include "parser.h"

class CDiscoMapDocument;

class CDiscoMapParser :
	public CParserBase
{
public:
	CDiscoMapParser(void);
	~CDiscoMapParser(void);
	
	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY( "DiscoveryClientResultsFile", OnDiscoveryClientResultsFile )
		XMLTAG_ENTRY( "Results", OnResults)
		XMLTAG_ENTRY( "DiscoveryClientResult", OnDiscoveryClientResult )
	END_XMLTAG_MAP()
	
	TAG_METHOD_DECL( OnDiscoveryClientResultsFile );
	TAG_METHOD_DECL( OnResults);
	TAG_METHOD_DECL( OnDiscoveryClientResult );

private:
	CAutoPtr<CDiscoMapDocument> m_pDocument;
public:
	CDiscoMapParser(ISAXXMLReader * pReader, CParserBase * pParent, DWORD dwLevel);

	CDiscoMapDocument * GetDiscoMapDocument(bool bReleaseOwnership = FALSE)
	{
		if (m_pDocument == NULL)
		{
			CreateDiscoMapDocument();
		}

		if (bReleaseOwnership != FALSE)
		{
			return m_pDocument.Detach();
		}
		return m_pDocument;
	}
	CDiscoMapDocument * CreateDiscoMapDocument(void);
};
