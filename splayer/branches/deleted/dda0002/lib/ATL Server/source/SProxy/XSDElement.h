//
// XSDElement.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "XSDMappingTable.h"
#include "Emit.h"

enum XSDELEMENT_TYPE
{
	XSD_UNK = 0,

	XSD_ERR = 1,

	//
	// Permits the elements in the group to appear (or not appear) 
	// in any order in the containing element.
	//
	XSD_ALL = 2,

	//
	// Enables any element from the specified namespace(s) 
	// to appear in the containing complexType, sequence, or choice element.
	//
	XSD_ANY,

	//
	// Enables any attribute from the specified namespace(s) 
	// to appear in the containing complexType element.
	//
	XSD_ANYATTRIBUTE,

	//
	// Defines an annotation.
	//
	XSD_ANNOTATION,

	//
	// Specifies information to be used by applications 
	// within an annotation.
	//
	XSD_APPINFO,

	//
	// Declares an attribute.
	//
	XSD_ATTRIBUTE,

	//
	// Groups a set of attribute declarations so that they 
	// can be incorporated as a group into complex type definitions.
	//
	XSD_ATTRIBUTEGROUP,

	//
	// Permits one and only one of the elements contained in the group
	// to be present within the containing element.
	//
	XSD_CHOICE,

	//
	// Defines a complex type, which determines the set of attributes 
	// and the content of an element.
	//
	XSD_COMPLEXTYPE,

	//
	// Specifies information to be read by or used by humans 
	// within an annotation.
	//
	XSD_DOCUMENTATION,

	//
	// Declares an element.
	//
	XSD_ELEMENT,

	//
	// Specifies an XPATH expression that specifies value 
	// (or one of the values) used to enforce an identity 
	// constraint (unique, key, keyref).
	//
	XSD_FIELD,

	//
	// Groups a set of element declarations so that they can 
	// be incorporated as a group into complex type definitions.
	//
	XSD_GROUP,

	//
	// Identifies a namespace whose schema components are referenced 
	// by the containing schema.
	//
	XSD_IMPORT,

	//
	// Includes the specified schema document in the targetNamespace 
	// of the containing schema.
	//
	XSD_INCLUDE,

	//
	// Specifies that an attribute or element value (or set of values) 
	// must be a key within the specified scope. A key must be unique, 
	// non-nullable, and always present.
	//
	XSD_KEY,

	//
	// Specifies that an attribute or element value (or set of values) 
	// have a correspondence with those of the specified key or unique element.
	//
	XSD_KEYREF,

	//
	// Contains the definition of a schema.
	//
	XSD_SCHEMA,

	//
	// Specifies an XPATH expression that selects a set of elements for an 
	// identity constraint (unique, key, keyref).
	//
	XSD_SELECTOR,

	//
	// Requires the elements in the group to appear in the specified sequence 
	// within the containing element.
	//
	XSD_SEQUENCE,

	//
	// Defines a simple type, which determines the constraints on and 
	// information about the values of attributes or elements with 
	// text-only content.
	//
	XSD_SIMPLETYPE,

	//
	// Specifies that an attribute or element value (or set of values) 
	// must be unique within the specified scope. 
	//
	XSD_UNIQUE,

	XSD_COMPLEXCONTENT,
	XSD_SIMPLECONTENT,
	XSD_RESTRICTION,
	XSD_EXTENSION,

	XSD_UNSUPPORTED
};

class CSchema;

class CXSDElement : public CXMLElement
{
private:

	XSDELEMENT_TYPE m_elementType;
	CSchema * m_pParentSchema;

public:

	CXSDElement(CXMLElement * pParentElement = NULL, XSDELEMENT_TYPE elementType = XSD_UNK)
		:m_elementType(elementType)
	{
		SetParentElement(pParentElement);
	}

	inline XSDELEMENT_TYPE GetElementType()
	{
		return m_elementType;
	}

	inline void SetElementType(XSDELEMENT_TYPE elementType)
	{
		m_elementType = elementType;
	}

	inline CSchema * GetParentSchema()
	{
		return m_pParentSchema;
	}

	inline void SetParentSchema(CSchema * pParentSchema)
	{
		m_pParentSchema = pParentSchema;
	}
};

// XSD Mapping table
extern const __declspec(selectany) CXSDTypeLookup g_xsdLookup;

inline HRESULT GetXSDType(const CStringW& strUri, const CStringW& strName, XSDTYPE *pXSD)
{
	ATLASSERT( pXSD != NULL );
	*pXSD = XSDTYPE_ERR;
	if (strUri == XSD_NAMESPACEW)
	{
		const CXSDTypeLookup::HashNode *pNode = g_xsdLookup.Lookup(strName);
		if (pNode != NULL)
		{
			*pXSD = pNode->data.xsdType;
			return S_OK;
		}
		else // (pNode == NULL)
		{
			EmitError(IDS_SDL_UNRESOLVED_ELEM, strUri, strName);
			return E_FAIL;
		}
	}

	return S_FALSE;
}