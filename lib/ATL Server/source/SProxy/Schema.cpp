//
// Schema.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Attribute.h"
#include "Schema.h"
#include "Element.h"
#include "SimpleType.h"
#include "ComplexType.h"
#include "WSDLDocument.h"

CComplexType * CSchema::AddComplexType(CComplexType * p)
{
	if (p != NULL)
	{
		if (p->GetName().GetLength() != 0)
		{
			if (m_complexTypes.SetAt(p->GetName(), p) != NULL)
			{
				return p;
			}
		}
	}

	//
	// TODO: error
	//
	return NULL;
}

CSimpleType * CSchema::AddSimpleType(CSimpleType * p)
{
	if (p != NULL)
	{
		if (p->GetName().GetLength() != 0)
		{
			if (m_simpleTypes.SetAt(p->GetName(), p) != NULL)
			{
				return p;
			}
		}
	}

	return NULL;
}

CElement * CSchema::AddElement(CElement * p)
{
	if (p != NULL)
	{
		if (p->GetName().GetLength() != 0)
		{
			if (m_elements.SetAt(p->GetName(), p) != NULL)
			{
				return p;
			}
		}
	}

	return NULL;
}

CXSDElement * CSchema::GetNamedItemFromParent(const CStringW& strUri, const CStringW& strName)
{
	CXSDElement *pRet = NULL;
	CXMLDocument *pParentDoc = GetParentDocument();
	if ((pParentDoc != NULL) && (pParentDoc->GetDocumentType() == WSDLDOC))
	{
		CWSDLDocument *pWSDLDocument = static_cast<CWSDLDocument *>(pParentDoc);
		pRet = pWSDLDocument->GetComplexType(strName, strUri);
		if (pRet == NULL)
		{
			pRet = pWSDLDocument->GetSimpleType(strName, strUri);
			if (pRet == NULL)
			{
				pRet = pWSDLDocument->GetElement(strName, strUri);
			}
		}
	}
	return pRet;
}