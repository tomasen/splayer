//
// Schema.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "XSDElement.h"
#include "ComplexType.h"
#include "SimpleType.h"
#include "Element.h"
//#include "UnsupportedElement.h"

#include "Emit.h"
#include "resource.h"

class CSchema : public CXSDElement
{
private:

	typedef CAtlPtrMap<CStringW, CComplexType *, CStringRefElementTraits<CStringW> > COMPLEXTYPEMAP;
	typedef CAtlPtrMap<CStringW, CSimpleType *, CStringRefElementTraits<CStringW> > SIMPLETYPEMAP;
	typedef CAtlPtrMap<CStringW, CElement *, CStringRefElementTraits<CStringW> > ELEMENTMAP;
//	typedef CAtlPtrMap<CStringW, CUnsupportedElement *, CStringRefElementTraits<CStringW> > UNSUPPORTEDMAP;

	COMPLEXTYPEMAP m_complexTypes;
	SIMPLETYPEMAP m_simpleTypes;
	ELEMENTMAP m_elements;
//	UNSUPPORTEDMAP m_unsupportedTypes;

	CStringW m_strTargetNamespace;
	CStringW m_strName;
	CStringW m_strID;

public:

	CComplexType * AddComplexType(CComplexType * p);
	CSimpleType * AddSimpleType(CSimpleType * p);
	CElement * AddElement(CElement * p);

	CXSDElement * GetNamedItemFromParent(const CStringW& strUri, const CStringW& strName);

	inline CXSDElement * GetNamedItem(const CStringW& strUri, const CStringW& strName)
	{
		CXSDElement *pRet = NULL;
		if (strUri.GetLength() == 0 || strUri == GetTargetNamespace())
		{
			CStringW str;
			pRet = GetComplexType(str, strName);
			if (pRet == NULL)
			{
				pRet = GetSimpleType(str, strName);
				if (pRet == NULL)
				{
					pRet = GetElement(str, strName);
//					if (pRet == NULL)
//					{
//						pRet = GetUnsupportedElement(str, strName);
//					}
				}
			}
		}
		else
		{
			pRet = GetNamedItemFromParent(strUri, strName);
		}

		return pRet;
	}

	inline CComplexType * GetComplexType(const CStringW& strUri, const CStringW& strName)
	{
		if (strUri.GetLength() == 0 || strUri == GetTargetNamespace())
		{
			const COMPLEXTYPEMAP::CPair *p = m_complexTypes.Lookup(strName);
			if (p != NULL)
			{
				return p->m_value;
			}
		}

		return NULL;
	}

	inline CSimpleType * GetSimpleType(const CStringW& strUri, const CStringW& strName)
	{
		if (strUri.GetLength() == 0 || strUri == GetTargetNamespace())
		{
			const SIMPLETYPEMAP::CPair *p = m_simpleTypes.Lookup(strName);
			if (p != NULL)
			{
				return p->m_value;
			}
		}

		return NULL;
	}

	inline CElement * GetElement(const CStringW& strUri, const CStringW& strName)
	{
		if (strUri.GetLength() == 0 || strUri == GetTargetNamespace())
		{
			const ELEMENTMAP::CPair *p = m_elements.Lookup(strName);
			if (p != NULL)
			{
				return p->m_value;
			}
		}

		return NULL;
	}

//	inline CUnsupportedElement * GetUnsupportedElement(const CStringW& strUri, const CStringW& strName)
//	{
//		if (strUri.GetLength() == 0 || strUri == GetTargetNamespace())
//		{
//			const UNSUPPORTEDMAP::CPair *p = m_unsupportedTypes.Lookup(strName);
//			if (p != NULL)
//			{
//				return p->m_value;
//			}
//		}
//
//		return NULL;
//	}

	inline HRESULT SetName(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strName.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetName(const CStringW& strName)
	{
		m_strName = strName;

		return S_OK;
	}

	inline const CStringW& GetName()
	{
		return m_strName;
	}

	inline HRESULT SetTargetNamespace(const wchar_t *wszTargetNamespace, int cchTargetNamespace)
	{
		if (!wszTargetNamespace)
		{
			return E_FAIL;
		}

		m_strTargetNamespace.SetString(wszTargetNamespace, cchTargetNamespace);

		return S_OK;
	}

	inline HRESULT SetTargetNamespace(const CStringW& strTargetNamespace)
	{
		m_strTargetNamespace = strTargetNamespace;

		return S_OK;
	}

	inline const CStringW& GetTargetNamespace()
	{
		return m_strTargetNamespace;
	}

	inline HRESULT SetID(const wchar_t *wszID, int cchID)
	{
		if (!wszID)
		{
			return E_FAIL;
		}

		m_strID.SetString(wszID, cchID);

		return S_OK;
	}

	inline HRESULT SetID(const CStringW& strID)
	{
		m_strID = strID;

		return S_OK;
	}

	inline const CStringW& GetID()
	{
		return m_strID;
	}
};