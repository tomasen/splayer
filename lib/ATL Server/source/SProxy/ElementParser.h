//
// ElementParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"
#include "Emit.h"
#include "resource.h"

class CElement;

class CElementParser : public CParserBase
{
private:

	CElement * m_pElem;

public:

	inline CElementParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CElement * pElem)
		:CParserBase(pReader, pParent, dwLevel), m_pElem(pElem)
	{
	}

	inline CElement * GetElement()
	{
		return m_pElem;
	}

	inline void SetElement(CElement * pElem)
	{
		m_pElem = pElem;
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
	simpleType, complexType, key, keyref, unique
	*/
	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("simpleType", XSD_NAMESPACEA, OnSimpleType)
		XMLTAG_ENTRY_EX("complexType", XSD_NAMESPACEA, OnComplexType)
		XMLTAG_ENTRY_EX("key", XSD_NAMESPACEA, OnKey)
		XMLTAG_ENTRY_EX("keyRef", XSD_NAMESPACEA, OnKeyRef)
		XMLTAG_ENTRY_EX("unique", XSD_NAMESPACEA, OnUnique)
	END_XMLTAG_MAP()

	/*
	<element
	  abstract = "boolean"
	  block = "#all or (possibly empty) subset of {equivClass, extension, restriction}"
	  default = "string"
	  equivClass = "QName"
	  final = "#all or (possibly empty) subset of {extension, restriction}"
	  fixed = "string"
	  form = "qualified | unqualified"
	  id = "ID"
	  maxOccurs = "nonNegativeInteger | unbounded"
	  minOccurs = "nonNegativeInteger"
	  name = "NCName"
	  nullable = "boolean"
	  ref = "QName"
	  type = "QName"
	  {any attributes with non-schema namespace}
	>
	*/
	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
		XMLATTR_ENTRY("type", OnType)
		XMLATTR_ENTRY("minOccurs", OnMinOccurs)
		XMLATTR_ENTRY("maxOccurs", OnMaxOccurs)
		XMLATTR_ENTRY("nillable", OnNillable)
		XMLATTR_ENTRY("ref", OnRef)
		XMLATTR_ENTRY("id", OnID)
		XMLATTR_ENTRY("abstract", OnAbstract)
		XMLATTR_ENTRY("block", OnBlock)
		XMLATTR_ENTRY("default", OnDefault)
		XMLATTR_ENTRY("equivClass", OnEquivClass)
		XMLATTR_ENTRY("final", OnFinal)
		XMLATTR_ENTRY("fixed", OnFixed)
		XMLATTR_ENTRY("form", OnForm)
		XMLATTR_ENTRY_EX("arrayType", SOAP_NAMESPACEA, OnArrayType)
		XMLATTR_ENTRY_EX("SizeIs", ATLS_NAMESPACEA, OnSizeIs)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnSimpleType);
	TAG_METHOD_DECL(OnComplexType);
	TAG_METHOD_DECL(OnKey);
	TAG_METHOD_DECL(OnKeyRef);
	TAG_METHOD_DECL(OnUnique);

	ATTR_METHOD_DECL(OnName);
	ATTR_METHOD_DECL(OnType);
	ATTR_METHOD_DECL(OnMinOccurs);
	ATTR_METHOD_DECL(OnMaxOccurs);
	ATTR_METHOD_DECL(OnNillable);
	ATTR_METHOD_DECL(OnRef);
	ATTR_METHOD_DECL(OnID);
	ATTR_METHOD_DECL(OnAbstract);
	ATTR_METHOD_DECL(OnBlock);
	ATTR_METHOD_DECL(OnDefault);
	ATTR_METHOD_DECL(OnEquivClass);
	ATTR_METHOD_DECL(OnFinal);
	ATTR_METHOD_DECL(OnFixed);
	ATTR_METHOD_DECL(OnForm);
	ATTR_METHOD_DECL(OnArrayType);
	ATTR_METHOD_DECL(OnSizeIs);

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);
};