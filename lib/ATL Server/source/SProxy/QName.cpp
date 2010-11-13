//
// QName.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "QName.h"

void CQName::CrackQName(const wchar_t *wszQName, int cchQName)
{
	if (cchQName == -1)
	{
		cchQName = (int)wcslen(wszQName);
	}

	wchar_t * wszQNameTmp = (wchar_t *) wszQName;
	wchar_t * wszColon = wcschr(wszQNameTmp, L':');
	if (wszColon && (wszColon-wszQNameTmp) <= cchQName)
	{
		m_strPrefix.SetString(wszQNameTmp, (int)(wszColon-wszQNameTmp));
		m_strName.SetString(wszColon+1, (int)(cchQName-m_strPrefix.GetLength()-1));
	}
	else
	{
		m_strName.SetString(wszQName, cchQName);
	}
}

CQName::CQName(const CStringW& strQName)
{
	SetQName(strQName);
}

CQName::CQName(const CStringW& strPrefix, const CStringW& strName)
	:m_strPrefix(strPrefix), m_strName(strName)
{
}

CQName::CQName(const wchar_t *wszQName, int cchQName)
{
	SetQName(wszQName, cchQName);
}

void CQName::SetQName(const CStringW& strQName)
{
	CrackQName((LPCWSTR) strQName, strQName.GetLength());
}

void CQName::SetQName(const CStringW& strPrefix, const CStringW& strName)
{
	m_strPrefix = strPrefix;
	m_strName = strName;
}

void CQName::SetQName(const wchar_t *wszQName, int cchQName)
{
	CrackQName(wszQName, cchQName);
}