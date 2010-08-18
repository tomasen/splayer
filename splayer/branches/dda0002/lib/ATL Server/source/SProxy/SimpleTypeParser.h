//
// SimpleTypeParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"
#include "resource.h"

class CSimpleType;

class CSimpleTypeParser : public CParserBase
{
private:

	CSimpleType * m_pType;

public:

	inline CSimpleTypeParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CSimpleType * pType)
		:CParserBase(pReader, pParent, dwLevel), m_pType(pType)
	{
	}

	inline CSimpleType * GetSimpleType()
	{
		return m_pType;
	}

	inline void SetSimpleType(CSimpleType * pType)
	{
		m_pType = pType;
	}

	inline void MarkUnsupported(const wchar_t *wszQName, int cchQName)
	{
#ifdef _DEBUG
		int nLine;
		int nCol;
		GetLocator()->getLineNumber(&nLine);
		GetLocator()->getColumnNumber(&nCol);
		ATLTRACE( _T("%sUnsupported tag@(%d, %d) : %.*ws -- skipping element\n"), GetTabs(GetLevel()), 
			nLine, nCol,
			cchQName, wszQName );
#endif
	}

	/*
	annotation, length, enumeration, pattern, scale, period, duration, 
	maxLength, precision, minInclusive, minExclusive, maxInclusive, 
	maxExclusive, minLength, encoding
	*/
	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("enumeration", XSD_NAMESPACEA, OnEnumeration)
		XMLTAG_ENTRY_EX("annotation", XSD_NAMESPACEA, OnAnnotation)
		XMLTAG_ENTRY_EX("length", XSD_NAMESPACEA, OnLength)
		XMLTAG_ENTRY_EX("pattern", XSD_NAMESPACEA, OnPattern)
		XMLTAG_ENTRY_EX("scale", XSD_NAMESPACEA, OnScale)
		XMLTAG_ENTRY_EX("period", XSD_NAMESPACEA, OnPeriod)
		XMLTAG_ENTRY_EX("duration", XSD_NAMESPACEA, OnDuration)
		XMLTAG_ENTRY_EX("maxLength", XSD_NAMESPACEA, OnMaxLength)
		XMLTAG_ENTRY_EX("precision", XSD_NAMESPACEA, OnPrecision)
		XMLTAG_ENTRY_EX("minInclusive", XSD_NAMESPACEA, OnMinInclusive)
		XMLTAG_ENTRY_EX("minExclusive", XSD_NAMESPACEA, OnMinExclusive)
		XMLTAG_ENTRY_EX("maxInclusive", XSD_NAMESPACEA, OnMaxInclusive)
		XMLTAG_ENTRY_EX("maxExclusive", XSD_NAMESPACEA, OnMaxExclusive)
		XMLTAG_ENTRY_EX("minLength", XSD_NAMESPACEA, OnMinLength)
		XMLTAG_ENTRY_EX("encoding", XSD_NAMESPACEA, OnEncoding)

		// REVIEW: new one
		XMLTAG_ENTRY_EX("restriction", XSD_NAMESPACEA, OnRestriction)
	END_XMLTAG_MAP()

/*
<simpleType
  abstract = "boolean" 
  id = "ID" 
  name="NCName" 
  {any attributes with non-schema namespace}
>
*/
	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
		XMLATTR_ENTRY("id", OnID)
		XMLATTR_ENTRY("abstract", OnAbstract)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnAnnotation);
	TAG_METHOD_DECL(OnLength);
	TAG_METHOD_DECL(OnPattern);
	TAG_METHOD_DECL(OnEnumeration);
	TAG_METHOD_DECL(OnScale);
	TAG_METHOD_DECL(OnPeriod);
	TAG_METHOD_DECL(OnDuration);
	TAG_METHOD_DECL(OnMaxLength);
	TAG_METHOD_DECL(OnPrecision);
	TAG_METHOD_DECL(OnMinInclusive);
	TAG_METHOD_DECL(OnMinExclusive);
	TAG_METHOD_DECL(OnMaxInclusive);
	TAG_METHOD_DECL(OnMaxExclusive);
	TAG_METHOD_DECL(OnMinLength);
	TAG_METHOD_DECL(OnEncoding);
	TAG_METHOD_DECL(OnRestriction);

	ATTR_METHOD_DECL(OnName);
	ATTR_METHOD_DECL(OnID);
	ATTR_METHOD_DECL(OnAbstract);

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);
};