//
// SimpleType.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XSDElement.h"
#include "QName.h"

// typedef CStringW CEnumeration;
// typedef CStringElementTraits<CStringW> CEnumerationElementTraits;

class CEnumeration
{
private:

	CStringW m_strValue;

public:

	inline HRESULT SetValue(const wchar_t *wszValue, int cchValue)
	{
		if (!wszValue)
		{
			return E_FAIL;
		}

		m_strValue.SetString(wszValue, cchValue);

		return S_OK;
	}

	inline HRESULT SetValue(const CStringW& strValue)
	{
		m_strValue = strValue;

		return S_OK;
	}

	const CStringW& GetValue()
	{
		return m_strValue;
	}

	const CStringW& GetValue() const
	{
		return m_strValue;
	}
};

class CEnumerationElementTraits :
	public CElementTraitsBase<CEnumeration>
{
public:
	
	static ULONG Hash( INARGTYPE e )
	{
		LPCWSTR wsz = (LPCWSTR) e.GetValue();
		ULONG nHash = 0;
		while (*wsz != 0)
		{
			nHash = (nHash<<5)+nHash+(*wsz);
			wsz++;
		}

		return nHash;
	}

	static bool CompareElements(INARGTYPE e1, INARGTYPE e2)
	{
		return e1.GetValue() == e2.GetValue();
	}

	static int CompareElementsOrdered(INARGTYPE e1, INARGTYPE e2)
	{
		return (CStringW::StrTraits::StringCompare(e1.GetValue(), e2.GetValue()));
	}
};


enum ENCODING_TYPE
{
	ENCODING_ERR = 0,
	ENCODING_UNK,
	ENCODING_HEX,
	ENCODING_BASE64
};

ENCODING_TYPE GetEncodingAttribute(const wchar_t *wszVal, int cchVal);

class CSimpleType : public CXSDElement
{
private:

	CAtlList<CEnumeration, CEnumerationElementTraits > m_enumerations;
	ENCODING_TYPE m_encType;
	int m_nLength;
	int m_nMaxLength;
	int m_nMinLength;

	CStringW m_strName;
	CQName m_base;

public:

	CSimpleType()
		:m_encType(ENCODING_UNK), m_nLength(-1), m_nMaxLength(-1), m_nMinLength(-1)
	{
		SetElementType(XSD_SIMPLETYPE);
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

	inline HRESULT SetBase(const CStringW& strQName)
	{
		m_base.SetQName(strQName);

		return S_OK;
	}

	inline HRESULT SetBase(const CStringW& strPrefix, const CStringW& strName)
	{
		m_base.SetQName(strPrefix, strName);

		return S_OK;
	}

	inline HRESULT SetBase(const wchar_t *wszQName, int cchQName)
	{
		m_base.SetQName(wszQName, cchQName);

		return S_OK;
	}

	inline CQName& GetBase()
	{
		return m_base;
	}

	inline const CStringW& GetName()
	{
		return m_strName;
	}

	inline ENCODING_TYPE GetEncodingType()
	{
		return m_encType;
	}

	inline ENCODING_TYPE SetEncodingType(ENCODING_TYPE encType)
	{
		return m_encType = encType;
	}

	inline HRESULT SetEncodingType(const wchar_t * wszVal, int cchVal)
	{
		m_encType = GetEncodingAttribute(wszVal, cchVal);
		if ((m_encType != ENCODING_ERR) &&
			(m_encType != ENCODING_UNK))
		{
			return S_OK;
		}
		return E_FAIL;
	}

	inline CEnumeration * AddEnumeration(CEnumeration* pEnum = NULL)
	{
		POSITION pos = NULL;
		if (pEnum != NULL)
		{
			pos = m_enumerations.AddTail(*pEnum);
		}
		else
		{
			CEnumeration e;
			pos = m_enumerations.AddTail(e);
		}

		if (pos != NULL)
		{
			return &(m_enumerations.GetAt(pos));
		}

		return NULL;
	}

	inline POSITION GetFirstEnumeration()
	{
		return m_enumerations.GetHeadPosition();
	}

	inline CEnumeration * GetNextEnumeration(POSITION &pos)
	{
		return &m_enumerations.GetNext(pos);
	}

	inline BOOL HasLength()
	{
		return (m_nLength != -1 ? TRUE : FALSE);
	}

	inline BOOL HasMinLength()
	{
		return (m_nMinLength != -1 ? TRUE : FALSE);
	}

	inline BOOL HasMaxLength()
	{
		return (m_nMaxLength != -1 ? TRUE : FALSE);
	}

	inline int GetLength()
	{
		return m_nLength;
	}

	inline void SetLength(int nLength)
	{
		m_nLength = nLength;
	}

	inline int GetMinLength()
	{
		return m_nMinLength;
	}

	inline void SetMinLength(int nLength)
	{
		m_nMinLength = nLength;
	}

	inline int GetMaxLength()
	{
		return m_nMaxLength;
	}

	inline void SetMaxLength(int nLength)
	{
		m_nMaxLength = nLength;
	}
};

inline ENCODING_TYPE GetEncodingAttribute(const wchar_t *wszVal, int cchVal)
{
	ENCODING_TYPE ret = ENCODING_ERR;
	if (cchVal == sizeof("hex")-1 && !wcsncmp(wszVal, L"hex", cchVal))
	{
		ret = ENCODING_HEX;
	}
	else if (cchVal == sizeof("base64")-1 && !wcsncmp(wszVal, L"base64", cchVal))
	{
		ret = ENCODING_BASE64;
	}

	return ret;
}