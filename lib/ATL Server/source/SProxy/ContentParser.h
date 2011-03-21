//
// ContentParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"
#include "Emit.h"
#include "resource.h"

// CContentParser parses the simpleContent tag and the complexContent tag

class CContent;

class CContentParser : public CParserBase
{
private:

	CContent *m_pContent;

public:

	inline CContentParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CContent *pContent = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pContent(pContent)
	{
	}

	inline CContent * GetContent()
	{
		return m_pContent;
	}

	inline CContent * SetContent(CContent *pContent)
	{
		m_pContent = pContent;
	}

	/*
	<complexContent 
	  id = ID 
	  mixed = boolean 
	  {any attributes with non-schema namespace . . .}>
	  Content: (annotation? , (restriction | extension))
	</complexContent>

	<simpleContent 
	  id = ID 
	  {any attributes with non-schema namespace . . .}>
	  Content: (annotation? , (restriction | extension))
	</simpleContent>
	*/
	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("annotation", XSD_NAMESPACEA, OnAnnotation)
		XMLTAG_ENTRY_EX("restriction", XSD_NAMESPACEA, OnRestriction)
		XMLTAG_ENTRY_EX("extension", XSD_NAMESPACEA, OnExtension)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("mixed", OnMixed)
		XMLATTR_ENTRY("id", OnID)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnAnnotation);
	TAG_METHOD_DECL(OnRestriction);
	TAG_METHOD_DECL(OnExtension);

	ATTR_METHOD_DECL(OnMixed);
	ATTR_METHOD_DECL(OnID);
};

typedef CContentParser CComplexContentParser;
typedef CContentParser CSimpleContentParser;