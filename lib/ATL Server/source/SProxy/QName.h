//
// QName.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"

class CQName
{
private:
	
	CStringW m_strPrefix;
	CStringW m_strName;

	void CrackQName(const wchar_t *wszQName, int cchQName);

public:
	
	CQName()
	{
	}

	CQName(const CStringW& strQName);
	CQName(const CStringW& strPrefix, const CStringW& strName);
	CQName(const wchar_t *wszQName, int cchQName);

	inline CQName(const CQName& that)
	{
		*this = that;
	}

	inline const CQName& operator=(const CQName& that)
	{
		if (this != &that)
		{
			m_strPrefix = that.m_strPrefix;
			m_strName = that.m_strName;
		}

		return *this;
	}

	void SetQName(const CStringW& strQName);
	void SetQName(const CStringW& strPrefix, const CStringW& strName);
	void SetQName(const wchar_t *wszQName, int cchQName);

	inline void SetPrefix(const CStringW& strPrefix)
	{
		m_strPrefix = strPrefix;
	}

	inline CStringW& GetPrefix()
	{
		return m_strPrefix;
	}

	inline CStringW& GetName()
	{
		return m_strName;
	}

	inline void SetName(const CStringW& strName)
	{
		m_strName = strName;
	}
};