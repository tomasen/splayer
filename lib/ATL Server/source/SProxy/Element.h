//
// Element.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XSDElement.h"
#include "QName.h"

class CComplexType;
class CSimpleType;

#define MAXOCCURS_UNBOUNDED -2

class CElement : public CXSDElement
{
private:

	CAtlPtrList<CXSDElement *> m_elements;
	CStringW m_strName;
	CStringW m_strID;
	CQName m_type;
	CQName m_ref;
	int m_nMinOccurs;
	int m_nMaxOccurs;
	BOOL m_bNullable;
	CStringW m_strArrayType;
	CStringW m_strSizeIs;

public:

	CElement()
		:m_nMinOccurs(-1), m_nMaxOccurs(-1), m_bNullable(FALSE)
	{
		SetElementType(XSD_ELEMENT);
	}

	CComplexType * AddComplexType(CComplexType * pComplexType = NULL);
	CSimpleType * AddSimpleType(CSimpleType * pSimpleType = NULL);
	CXSDElement * GetType();
	const wchar_t * GetTargetNamespace();

	inline POSITION GetFirstElement()
	{
		return m_elements.GetHeadPosition();
	}

	inline CXSDElement * GetNextElement(POSITION& pos)
	{
		return m_elements.GetNext(pos);
	}

	inline size_t GetNumChildren()
	{
		return m_elements.GetCount();
	}

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

	inline HRESULT SetType(const CStringW& strQName)
	{
		m_type.SetQName(strQName);

		return S_OK;
	}

	inline HRESULT SetType(const CStringW& strPrefix, const CStringW& strName)
	{
		m_type.SetQName(strPrefix, strName);

		return S_OK;
	}

	inline HRESULT SetType(const wchar_t *wszQName, int cchQName)
	{
		m_type.SetQName(wszQName, cchQName);

		return S_OK;
	}

	inline CQName& GetTypeName()
	{
		return m_type;
	}

	inline HRESULT SetRef(const CStringW& strQName)
	{
		m_ref.SetQName(strQName);

		return S_OK;
	}

	inline HRESULT SetRef(const CStringW& strPrefix, const CStringW& strName)
	{
		m_ref.SetQName(strPrefix, strName);

		return S_OK;
	}

	inline HRESULT SetRef(const wchar_t *wszQName, int cchQName)
	{
		m_ref.SetQName(wszQName, cchQName);

		return S_OK;
	}

	inline const CQName& GetRefName()
	{
		return m_ref;
	}

	inline BOOL HasMinOccurs()
	{
		return (m_nMinOccurs != -1 ? TRUE : FALSE);
	}

	inline BOOL HasMaxOccurs()
	{
		return (m_nMaxOccurs != -1 ? TRUE : FALSE);
	}

	inline void SetMinOccurs(int nMinOccurs)
	{
		m_nMinOccurs = nMinOccurs;
	}

	inline int GetMinOccurs()
	{
		return m_nMinOccurs;
	}

	inline void SetMaxOccurs(int nMaxOccurs)
	{
		m_nMaxOccurs = nMaxOccurs;
	}

	inline int GetMaxOccurs()
	{
		return m_nMaxOccurs;
	}

	inline BOOL GetNullable()
	{
		return m_bNullable;
	}

	inline void SetNullable(BOOL bNullable)
	{
		m_bNullable = bNullable;
	}

	inline HRESULT SetArrayType(const wchar_t *wszArrayType, int cchArrayType)
	{
		if (!wszArrayType)
		{
			return E_FAIL;
		}

		m_strArrayType.SetString(wszArrayType, cchArrayType);

		return S_OK;
	}

	inline HRESULT SetArrayType(const CStringW& strArrayType)
	{
		m_strArrayType = strArrayType;

		return S_OK;
	}
	
	inline HRESULT SetSizeIs(const wchar_t *wszSizeIs, int cchSizeIs)
	{
		if (!wszSizeIs)
		{
			return E_FAIL;
		}

		m_strSizeIs.SetString(wszSizeIs, cchSizeIs);

		return S_OK;
	}

	inline HRESULT SetSizeIs(const CStringW& strSizeIs)
	{
		m_strSizeIs = strSizeIs;

		return S_OK;
	}
	
	inline const CStringW& GetSizeIs()
	{
		return m_strSizeIs;
	}

	inline const CStringW& GetArrayType()
	{
		return m_strArrayType;
	}
};