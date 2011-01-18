//
// Attribute.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XSDElement.h"
#include "QName.h"

enum ATTRIBUTEFORM
{
	ATTRFORM_UNK = 0,
	ATTRFORM_QUALIFIED,
	ATTRFORM_UNQUALIFIED
};

inline ATTRIBUTEFORM GetAttributeForm(const wchar_t *wsz, int cch)
{
	struct _attrform
	{
		wchar_t *wsz;
		int cch;
		ATTRIBUTEFORM attrform;
	};

	ATTRIBUTEFORM retForm = ATTRFORM_UNK;

	// data driven is kind of overkill for two options, but makes it 
	// easy to extend later
	static const _attrform s_forms[] =
	{
		{ L"qualified", sizeof("qualified")-1, ATTRFORM_QUALIFIED },
		{ L"unqualified", sizeof("unqualified")-1, ATTRFORM_UNQUALIFIED }
	};

	for (int i=0; i<(sizeof(s_forms)/sizeof(s_forms[0])); i++)
	{
		if (cch == s_forms[i].cch && !wcsncmp(wsz, s_forms[i].wsz, cch))
		{
			retForm = s_forms[i].attrform;
			break;
		}
	}

	return retForm;
}

enum ATTRIBUTEUSE
{
	ATTRUSE_UNK = 0,
	ATTRUSE_PROHIBITED,
	ATTRUSE_OPTIONAL,
	ATTRUSE_REQUIRED,
	ATTRUSE_DEFAULT,
	ATTRUSE_FIXED
};

inline ATTRIBUTEUSE GetAttributeUse(const wchar_t *wsz, int cch)
{
	struct _attruse
	{
		wchar_t *wsz;
		int cch;
		ATTRIBUTEUSE attruse;
	};

	ATTRIBUTEUSE retUse = ATTRUSE_UNK;

	// data driven is kind of overkill for two options, but makes it 
	// easy to extend later
	static const _attruse s_uses[] =
	{
		{ L"prohibited", sizeof("prohibited")-1, ATTRUSE_PROHIBITED },
		{ L"optional", sizeof("optional")-1, ATTRUSE_OPTIONAL },
		{ L"required", sizeof("required")-1, ATTRUSE_REQUIRED },
		{ L"default", sizeof("default")-1, ATTRUSE_DEFAULT },
		{ L"fixed", sizeof("fixed")-1, ATTRUSE_FIXED },
	};

	for (int i=0; i<(sizeof(s_uses)/sizeof(s_uses[0])); i++)
	{
		if (cch == s_uses[i].cch && !wcsncmp(wsz, s_uses[i].wsz, cch))
		{
			retUse = s_uses[i].attruse;
			break;
		}
	}

	return retUse;
}

class CAttribute : public CXSDElement
{
private:

	ATTRIBUTEFORM m_attrForm;
	ATTRIBUTEUSE m_attrUse;
	CStringW m_strName;
	CQName m_ref;
	CQName m_type;
	CStringW m_strValue;
	CStringW m_strID;

	// WSDL:arrayType attribute
	CStringW m_strArrayType;

protected:

public:

	// REVIEW: set to defaults?
	CAttribute()
		:m_attrForm(ATTRFORM_UNK), m_attrUse(ATTRUSE_UNK)
	{
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

	inline const CStringW& GetArrayType()
	{
		return m_strArrayType;
	}

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

	inline const CStringW& GetValue()
	{
		return m_strValue;
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

	inline CQName& GetRefName()
	{
		return m_ref;
	}

	inline HRESULT SetAttributeForm(const wchar_t *wsz, int cch)
	{
		m_attrForm = ::GetAttributeForm(wsz, cch);
		if (m_attrForm != ATTRFORM_UNK)
		{
			return S_OK;
		}

		return E_FAIL;
	}

	inline HRESULT SetAttributeForm(const CStringW& str)
	{
		return SetAttributeForm(str, str.GetLength());
	}

	inline ATTRIBUTEFORM GetAttributeForm()
	{
		return m_attrForm;
	}

	inline HRESULT SetAttributeUse(const wchar_t *wsz, int cch)
	{
		m_attrUse = ::GetAttributeUse(wsz, cch);
		if (m_attrUse != ATTRUSE_UNK)
		{
			return S_OK;
		}

		return E_FAIL;
	}

	inline HRESULT SetAttributeUse(const CStringW& str)
	{
		return SetAttributeUse(str, str.GetLength());
	}

	inline ATTRIBUTEUSE GetAttributeUse()
	{
		return m_attrUse;
	}
}; // class CAttribute