//
// Content.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

// CContent represents the complexContent and the simpleContent 

#pragma once

#include "stdafx.h"
#include "XSDElement.h"
#include "QName.h"
#include "Util.h"

class CComplexType;

class CContent : public CXSDElement
{
private:

	// restriction/extension
	CComplexType * m_pType;

	BOOL m_bMixed;
	CStringW m_strID;

protected:

public:

	CContent()
		:m_pType(NULL), m_bMixed(FALSE)
	{
	}

	CComplexType * AddType(CComplexType *pContent = NULL);

	CComplexType * GetType();

	~CContent();

	inline HRESULT SetID(const wchar_t *wsz, int cch)
	{
		m_strID.SetString(wsz, cch);
		return S_OK;
	}

	inline HRESULT SetID(const CStringW& strID)
	{
		m_strID = strID;
		return S_OK;
	}

	inline HRESULT SetMixed(const wchar_t *wszValue, int cchValue)
	{
		bool bVal;
		HRESULT hr = GetBooleanValue(&bVal, wszValue, cchValue);
		if (SUCCEEDED(hr))
		{
			m_bMixed = (bVal == true) ? TRUE : FALSE;
		}

		// unknown string
		return hr;
	}

	inline HRESULT SetMixed(BOOL bMixed)
	{
		m_bMixed = bMixed;
	}

	inline BOOL GetMixed()
	{
		return m_bMixed;
	}
}; // class CContent