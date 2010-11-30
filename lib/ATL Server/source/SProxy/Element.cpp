//
// Element.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "Element.h"
#include "ComplexType.h"
#include "SimpleType.h"
#include "Schema.h"
#include "Attribute.h"
#include "Content.h"

CComplexType * CElement::AddComplexType(CComplexType * p)
{
	CAutoPtr<CComplexType> spOut;
	if (p== NULL)
	{
		spOut.Attach( new CComplexType );
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

CSimpleType * CElement::AddSimpleType(CSimpleType * p)
{
	CAutoPtr<CSimpleType> spOut;
	if (p== NULL)
	{
		spOut.Attach( new CSimpleType );
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


CXSDElement * CElement::GetType()
{
	CXSDElement *pRet = NULL;
	if (m_type.GetName().GetLength())
	{
		CSchema *pSchema = GetParentSchema();
		if (pSchema != NULL)
		{
			CStringW strUri;
			if (SUCCEEDED(/*pSchema->*/GetNamespaceUri(m_type.GetPrefix(), strUri)))
			{
				pRet = pSchema->GetNamedItem(strUri, m_type.GetName());
			}
		}
	}

	//
	// TODO: appropriate errors
	//

	return pRet;
}

const wchar_t * CElement::GetTargetNamespace()
{
	if (m_type.GetName().GetLength())
	{
//		CSchema *pSchema = GetParentSchema();
//		if (pSchema != NULL)
//		{
			CStringW strUri;
			if (SUCCEEDED(/*pSchema->*/GetNamespaceUri(m_type.GetPrefix(), strUri)))
			{
				return strUri;
			}
//		}
	}

	EmitFileError(IDS_SDL_UNRESOLVED_NAMESPACE, const_cast<CElement*>(this), 0, m_type.GetPrefix());

	return NULL;
}