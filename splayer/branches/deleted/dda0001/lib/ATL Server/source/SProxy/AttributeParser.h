//
// AttributeParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"
#include "Emit.h"
#include "resource.h"

class CAttribute;

class CAttributeParser : public CParserBase
{
private:

	CAttribute *m_pAttribute;

public:

	inline CAttributeParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, CAttribute *pAttribute = NULL)
		:CParserBase(pReader, pParent, dwLevel), m_pAttribute(pAttribute)
	{
	}

	inline CAttribute * GetAttribute()
	{
		return m_pAttribute;
	}

	inline void SetAttribute(CAttribute *pAttribute)
	{
		m_pAttribute = pAttribute;
	}
	
	HRESULT ValidateElement();

	/*
	<attribute 
	  form = (qualified | unqualified)
	  id = ID 
	  name = NCName 
	  ref = QName 
	  type = QName 
	  use = (prohibited | optional | required | default | fixed) : optional
	  value = string 
	  {any attributes with non-schema namespace . . .}>
	  Content: (annotation? , (simpleType?))
	</attribute>
	*/
	BEGIN_XMLTAG_MAP()
		XMLTAG_ENTRY_EX("annotation", XSD_NAMESPACEA, OnAnnotation)
		XMLTAG_ENTRY_EX("simpleType", XSD_NAMESPACEA, OnSimpleType)
	END_XMLTAG_MAP()

	BEGIN_XMLATTR_MAP()
		XMLATTR_ENTRY("form", OnForm)
		XMLATTR_ENTRY("ref", OnRef)
		XMLATTR_ENTRY("name", OnName)
		XMLATTR_ENTRY_EX("arrayType", WSDL_NAMESPACEA, OnArrayType)
		XMLATTR_ENTRY("type", OnType)
		XMLATTR_ENTRY("use", OnUse)
		XMLATTR_ENTRY("value", OnValue)
		XMLATTR_ENTRY("id", OnID)
	END_XMLATTR_MAP()

	TAG_METHOD_DECL(OnAnnotation);
	TAG_METHOD_DECL(OnSimpleType);

	ATTR_METHOD_DECL(OnForm);
	ATTR_METHOD_DECL(OnRef);
	ATTR_METHOD_DECL(OnArrayType);
	ATTR_METHOD_DECL(OnName);
	ATTR_METHOD_DECL(OnType);
	ATTR_METHOD_DECL(OnUse);
	ATTR_METHOD_DECL(OnValue);
	ATTR_METHOD_DECL(OnID);
};