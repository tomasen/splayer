//
// ComplexType.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "Attribute.h"
#include "ComplexType.h"
#include "Element.h"
#include "Content.h"

CElement * CComplexType::AddElement(CElement * p)
{
	CAutoPtr<CElement> spOut;
	if (p == NULL)
	{
		spOut.Attach( new CElement );
		p = spOut;
	}
	
	if (p != NULL)
	{
		if (m_elements.AddTail(p) != NULL)
		{
			spOut.Detach();
			return p;
		}
	}

	return NULL;
}

CAttribute * CComplexType::AddAttribute(CAttribute * p)
{
	CAutoPtr<CAttribute> spOut;
	if (p == NULL)
	{
		spOut.Attach( new CAttribute );
		p = spOut;
	}
	
	if (p != NULL)
	{
		if (m_attributes.AddTail(p) != NULL)
		{
			spOut.Detach();
			return p;
		}
	}

	return NULL;
}

CContent * CComplexType::AddContent(CContent *pContent)
{
	if (pContent == NULL)
	{
		pContent = new CContent;
	}
	delete m_pContent;
	m_pContent = pContent;

	return m_pContent;
}

CComplexType::~CComplexType()
{
	delete m_pContent;
}