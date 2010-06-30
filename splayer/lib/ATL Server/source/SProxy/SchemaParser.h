//
// SchemaParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"
#include "Emit.h"
#include "resource.h"

class CSchema;

class CSchemaParser : public CParserBase
{
private:

	CSchema * m_pSchema;

public:

	inline CSchemaParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CSchema *pSchema = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pSchema(pSchema)
	{
	}

	inline CSchema * GetSchema()
	{
		return m_pSchema;
	}

	inline void SetSchema(CSchema *pSchema)
	{
		m_pSchema = pSchema;
	}

	void EmitNamedElementError(const char *szElem);

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
	include, import, annotation, redefine 
	attribute, attributeGroup, element, group, notation, simpleType, complexType
	*/

	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("include", XSD_NAMESPACEA, OnInclude)
		XMLTAG_ENTRY_EX("import", XSD_NAMESPACEA, OnImport)
		XMLTAG_ENTRY_EX("annotation", XSD_NAMESPACEA, OnAnnotation)
		XMLTAG_ENTRY_EX("redefine", XSD_NAMESPACEA, OnRedefine)
		XMLTAG_ENTRY_EX("attribute", XSD_NAMESPACEA, OnAttribute)
		XMLTAG_ENTRY_EX("attributeGroup", XSD_NAMESPACEA, OnAttributeGroup)
		XMLTAG_ENTRY_EX("element", XSD_NAMESPACEA, OnElement)
		XMLTAG_ENTRY_EX("group", XSD_NAMESPACEA, OnGroup)
		XMLTAG_ENTRY_EX("notation", XSD_NAMESPACEA, OnNotation)
		XMLTAG_ENTRY_EX("simpleType", XSD_NAMESPACEA, OnSimpleType)
		XMLTAG_ENTRY_EX("complexType", XSD_NAMESPACEA, OnComplexType)
	END_XMLTAG_MAP()

	/*
	<schema
	  attributeFormDefault = "{ qualified | unqualified }"
	  blockDefault = "#all or (possibly empty) subset of {equivClass, extension, restriction}"
	  elementFormDefault = "{ qualified | unqualified }"
	  finalDefault = "#all or (possibly empty) subset of {extension, restriction}"
	  id = "ID"
	  name="schema-name" 
	  targetNamespace = "uriReference"
	  version = "string"
	  xmlns="uriReference"
	  {any attributes with non-schema namespace}
	>
	*/
	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("name", OnName)
		XMLATTR_ENTRY("targetNamespace", OnTargetNamespace)
		XMLATTR_ENTRY("id", OnID)
		XMLATTR_ENTRY("attributeFormDefault", OnAttributeFormDefault)
		XMLATTR_ENTRY("blockDefault", OnBlockDefault)
		XMLATTR_ENTRY("elementFormDefault", OnElementFormDefault)
		XMLATTR_ENTRY("finalDefault", OnFinalDefault)
		XMLATTR_ENTRY("version", OnVersion)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnInclude);
	TAG_METHOD_DECL(OnImport);
	TAG_METHOD_DECL(OnAnnotation);
	TAG_METHOD_DECL(OnRedefine);
	TAG_METHOD_DECL(OnAttribute);
	TAG_METHOD_DECL(OnAttributeGroup);
	TAG_METHOD_DECL(OnElement);
	TAG_METHOD_DECL(OnGroup);
	TAG_METHOD_DECL(OnNotation);
	TAG_METHOD_DECL(OnSimpleType);
	TAG_METHOD_DECL(OnComplexType);

	ATTR_METHOD_DECL(OnName);
	ATTR_METHOD_DECL(OnTargetNamespace);
	ATTR_METHOD_DECL(OnID);
	ATTR_METHOD_DECL(OnAttributeFormDefault);
	ATTR_METHOD_DECL(OnBlockDefault);
	ATTR_METHOD_DECL(OnElementFormDefault);
	ATTR_METHOD_DECL(OnFinalDefault);
	ATTR_METHOD_DECL(OnVersion);

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);
};