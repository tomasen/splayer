//
// ComplexType.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XSDElement.h"
#include "QName.h"

// REVIEW: CComplexType is also used to represent the restriciton and extension tags --
// however the the restriction and extension tags do not allow the simpleContent and
// complexContent subtags and only the base and id attributes are supported.
// If in the future, CParserBase is modified to allow chaining, then the common
// functionality can be subsumed in a base class instead.

class CElement;
class CContent;
class CAttribute;

enum CONTENT_TYPE
{
	CONTENT_ERR = 0,
	CONTENT_UNK,
	CONTENT_ELEMENTONLY,
	CONTENT_TEXTONLY,
	CONTENT_MIXED,
	CONTENT_EMPTY
};

enum DERIVED_TYPE
{
	DERIVED_ERR = 0,
	DERIVED_UNK,
	DERIVED_EXTENSION,
	DERIVED_RESTRICTION
};

CONTENT_TYPE GetContentAttribute(const wchar_t * wszVal, int cchVal);
DERIVED_TYPE GetDerivedByAttribute(const wchar_t * wszVal, int cchVal);

class CComplexType : public CXSDElement
{
private:

	CStringW m_strName;
	CStringW m_strID;
	CQName m_base;
	
	CAtlPtrList<CElement *> m_elements;

	// attributes
	CAtlPtrList<CAttribute *> m_attributes;

	// simpleContent/complexContent tags
	CContent *m_pContent;

	CONTENT_TYPE m_contentType;
	DERIVED_TYPE m_derivedBy;

public:

	CComplexType()
		:m_contentType(CONTENT_UNK), m_derivedBy(DERIVED_UNK), m_pContent(NULL)
	{
		SetElementType(XSD_COMPLEXTYPE);
	}

	~CComplexType();

	CContent * AddContent(CContent *pContent = NULL);

	inline CContent * GetContent()
	{
		return m_pContent;
	}

	CElement * AddElement(CElement * pElement = NULL);
	CAttribute * AddAttribute(CAttribute * pAttribute = NULL);

	inline size_t GetNumAttributes()
	{
		return m_attributes.GetCount();
	}

	inline POSITION GetFirstElement()
	{
		return m_elements.GetHeadPosition();
	}

	inline CElement * GetNextElement(POSITION& pos)
	{
		return m_elements.GetNext(pos);
	}

	inline POSITION GetFirstAttribute()
	{
		return m_attributes.GetHeadPosition();
	}

	inline CAttribute * GetNextAttribute(POSITION& pos)
	{
		return m_attributes.GetNext(pos);
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

	inline DERIVED_TYPE GetDerivedBy()
	{
		return m_derivedBy;
	}

	inline HRESULT SetDerivedBy(DERIVED_TYPE derivedBy)
	{
		m_derivedBy = derivedBy;
		return S_OK;
	}

	inline HRESULT SetDerivedBy(const wchar_t * wszVal, int cchVal)
	{
		m_derivedBy = GetDerivedByAttribute(wszVal, cchVal);
		return (m_derivedBy != DERIVED_ERR) ? S_OK : E_FAIL;
	}

	inline CONTENT_TYPE GetContentType()
	{
		return m_contentType;
	}

	inline CONTENT_TYPE SetContentType(CONTENT_TYPE contentType)
	{
		m_contentType = contentType;
		return m_contentType;
	}

	inline CONTENT_TYPE SetContentType(const wchar_t * wszVal, int cchVal)
	{
		m_contentType = GetContentAttribute(wszVal, cchVal);
		return m_contentType;
	}
};

inline CONTENT_TYPE GetContentAttribute(const wchar_t * wszVal, int cchVal)
{
	struct _map
	{
		wchar_t * wszEntry;
		int cchEntry;
		CONTENT_TYPE data;
	};

	CONTENT_TYPE ret = CONTENT_ERR;

	static const _map s_entries[] =
	{
		{ L"elementOnly", sizeof("elementOnly")-1, CONTENT_ELEMENTONLY },
		{ L"textOnly", sizeof("textOnly")-1, CONTENT_TEXTONLY },
		{ L"mixed", sizeof("mixed")-1, CONTENT_MIXED },
		{ L"empty", sizeof("emtpy")-1, CONTENT_EMPTY }
	};

	for (int i=0; i<(sizeof(s_entries)/sizeof(s_entries[0])); i++)
	{
		if (cchVal == s_entries[i].cchEntry && !wcsncmp(wszVal, s_entries[i].wszEntry, cchVal))
		{
			ret = s_entries[i].data;
			break;
		}
	}

	return ret;
};

inline DERIVED_TYPE GetDerivedByAttribute(const wchar_t * wszVal, int cchVal)
{
	struct _map
	{
		wchar_t * wszEntry;
		int cchEntry;
		DERIVED_TYPE data;
	};

	DERIVED_TYPE ret = DERIVED_ERR;

	static const _map s_entries[] =
	{
		{ L"extension", sizeof("extension")-1, DERIVED_EXTENSION },
		{ L"restriction", sizeof("restriction")-1, DERIVED_RESTRICTION }
	};

	for (int i=0; i<(sizeof(s_entries)/sizeof(s_entries[0])); i++)
	{
		if (cchVal == s_entries[i].cchEntry && !wcsncmp(wszVal, s_entries[i].wszEntry, cchVal))
		{
			ret = s_entries[i].data;
			break;
		}
	}

	return ret;
};

typedef CComplexType CRestriction;
typedef CComplexType CExtension;