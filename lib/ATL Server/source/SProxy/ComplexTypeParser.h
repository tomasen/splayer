//
// ComplexTypeParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"
#include "Emit.h"
#include "resource.h"

class CComplexType;

class CComplexTypeParser : public CParserBase
{
private:

	CComplexType * m_pComplexType;

public:

	inline CComplexTypeParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CComplexType * pComplexType = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pComplexType(pComplexType)
	{
	}

	inline CComplexType * GetComplexType()
	{
		return m_pComplexType;
	}

	inline void SetComplexType(CComplexType * pComplexType)
	{
		m_pComplexType = pComplexType;
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
	maxExclusive, minLength, encoding, element, group, all, choice, 
	sequence, attribute, attributeGroup, anyAttribute 
	*/
	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("element", XSD_NAMESPACEA, OnElement)
		XMLTAG_ENTRY_EX("all", XSD_NAMESPACEA, OnAll)
		XMLTAG_ENTRY_EX("choice", XSD_NAMESPACEA, OnChoice)
		XMLTAG_ENTRY_EX("annotation", XSD_NAMESPACEA, OnAnnotation)
		XMLTAG_ENTRY_EX("length", XSD_NAMESPACEA, OnLength)
		XMLTAG_ENTRY_EX("enumeration", XSD_NAMESPACEA, OnEnumeration)
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
		XMLTAG_ENTRY_EX("group", XSD_NAMESPACEA, OnGroup)
		XMLTAG_ENTRY_EX("sequence", XSD_NAMESPACEA, OnSequence)
		XMLTAG_ENTRY_EX("attribute", XSD_NAMESPACEA, OnAttribute)
		XMLTAG_ENTRY_EX("attributeGroup", XSD_NAMESPACEA, OnAttributeGroup)
		XMLTAG_ENTRY_EX("anyAttribute", XSD_NAMESPACEA, OnAnyAttribute)

		// REVIEW: new ones
		XMLTAG_ENTRY_EX("complexContent", XSD_NAMESPACEA, OnComplexContent)
		XMLTAG_ENTRY_EX("simpleContent", XSD_NAMESPACEA, OnSimpleContent)
		XMLTAG_ENTRY_EX("any", XSD_NAMESPACEA, OnAny)
	END_XMLTAG_MAP()


	/*
	<complexType
	  abstract = "boolean" 
	  base = "QName" 
	  block = "#all | subset of {extension, restriction}"
	  content = "elementOnly | textOnly | mixed | empty"
	  derivedBy = "extension | restriction"
	  final = "#all | subset of {extension, restriction}"
	  id = "ID" 
	  name ="NCName" 
	>
	*/
	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
		XMLATTR_ENTRY("id", OnID)
		XMLATTR_ENTRY("abstract", OnAbstract)
		XMLATTR_ENTRY("base", OnBase)
		XMLATTR_ENTRY("block", OnBlock)
		XMLATTR_ENTRY("content", OnContent)
		XMLATTR_ENTRY("derivedBy", OnDerivedBy)
		XMLATTR_ENTRY("final", OnFinal)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnElement);
	TAG_METHOD_DECL(OnAll);
	TAG_METHOD_DECL(OnChoice);
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
	TAG_METHOD_DECL(OnGroup);
	TAG_METHOD_DECL(OnSequence);
	TAG_METHOD_DECL(OnAttribute);
	TAG_METHOD_DECL(OnAttributeGroup);
	TAG_METHOD_DECL(OnAnyAttribute);
	
	// new ones
	TAG_METHOD_DECL(OnComplexContent);
	TAG_METHOD_DECL(OnSimpleContent);
	TAG_METHOD_DECL(OnAny);


	ATTR_METHOD_DECL(OnName);
	ATTR_METHOD_DECL(OnID);
	ATTR_METHOD_DECL(OnAbstract);
	ATTR_METHOD_DECL(OnBase);
	ATTR_METHOD_DECL(OnBlock);
	ATTR_METHOD_DECL(OnContent);
	ATTR_METHOD_DECL(OnDerivedBy);
	ATTR_METHOD_DECL(OnFinal);

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);
};