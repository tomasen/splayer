//
// Util.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "Util.h"
#include "CppKeywordTable.h"

HRESULT GetAttribute(
	ISAXAttributes *pAttributes, 
	const wchar_t *wszAttrName, int cchName, 
	const wchar_t **pwszValue, int *pcchValue,
	wchar_t *wszNamespace, int cchNamespace)
{
	if (!pAttributes || !wszAttrName || !pwszValue || !pcchValue)
	{
		return E_INVALIDARG;
	}

	*pwszValue = NULL;
	*pcchValue = 0;
	if (!wszNamespace)
	{
		return (pAttributes->getValueFromQName(wszAttrName, cchName, pwszValue, pcchValue) == S_OK ? S_OK : E_FAIL);
	}
	return (pAttributes->getValueFromName(wszNamespace, cchNamespace, 
		wszAttrName, cchName, pwszValue, pcchValue) == S_OK ? S_OK : E_FAIL);
}

HRESULT GetAttribute(
	ISAXAttributes *pAttributes, 
	const wchar_t *wszAttrName, int cchName, 
	CStringW &strValue,
	wchar_t *wszNamespace, int cchNamespace)
{
	const wchar_t *wszValue = NULL;
	int cchValue = 0;

	if (!pAttributes || !wszAttrName)
	{
		return E_INVALIDARG;
	}

	HRESULT hr;
	if (!wszNamespace)
	{
		hr = (pAttributes->getValueFromQName(wszAttrName, cchName, &wszValue, &cchValue) == S_OK ? S_OK : E_FAIL);
	}
	else
	{
		hr = (pAttributes->getValueFromName(wszNamespace, cchNamespace, 
			wszAttrName, cchName, &wszValue, &cchValue) == S_OK ? S_OK : E_FAIL);
	}

	if (hr == S_OK)
	{
		strValue.SetString(wszValue, cchValue);
	}

	return hr;
}

inline int IsUnsafeCppChar(char ch)
{
	return (!isalnum((unsigned char)ch) && ch != '_');
}

void _CreateSafeCppName(
	char *szSafeName, 
	const char *szName, 
	const wchar_t *wszName, 
	size_t nMaxLen)
{
	ATLASSERT( szSafeName != NULL );
	ATLASSERT( szName != NULL );
	ATLASSERT( wszName != NULL );

	static CCppKeywordLookup cppkwLookup;
	const CCppKeywordLookup::HashNode *p = cppkwLookup.Lookup(wszName);

	char *pszOut = szSafeName;

	size_t nLen = 0;
	if (p != NULL)
	{
		// append 3 underscores for a reserved keyword -- ugly, but hey...
		memcpy(pszOut, "___", 3);
		pszOut+= 3;
		nLen+= 3;
	}

	if (isdigit((unsigned char)(*szName)))
	{
		*pszOut++ = '_';
		nLen++;
	}

	while (*szName)
	{
		if (nLen == nMaxLen)
		{
			// just truncate
			break;
		}

		if (IsUnsafeCppChar(*szName))
		{
			*pszOut = '_';
		}
		else
		{
			*pszOut = *szName;
		}
		pszOut++;
		szName++;
		nLen++;
	}

	*pszOut = '\0';
}

size_t AllocateNameString(char **ppszName, const char *szName)
{
	ATLASSERT( ppszName != NULL );
	ATLASSERT( szName != NULL );

	size_t nMaxLen = 3*strlen(szName);
	*ppszName = (char *)malloc(nMaxLen+1);
	if (!*ppszName)
	{
		return 0;
	}

	return nMaxLen;
}

size_t AllocateNameString(char **ppszName, const wchar_t *wszName)
{
	ATLASSERT( ppszName != NULL );
	ATLASSERT( wszName != NULL );

	size_t nMaxLen = 3*wcslen(wszName);
	*ppszName = (char *)malloc(nMaxLen+1);
	if (!*ppszName)
	{
		return 0;
	}

	return nMaxLen;
}

HRESULT CreateSafeCppName(char **ppszName, const char *szName)
{
	ATLASSERT( ppszName != NULL );
	ATLASSERT( szName != NULL );

	size_t nMaxLen = AllocateNameString(ppszName, szName);
	if (!nMaxLen)
	{
		return E_OUTOFMEMORY;
	}

	_CreateSafeCppName(*ppszName, szName, CA2W( szName ), nMaxLen);
	return S_OK;
}

HRESULT CreateSafeCppName(char **ppszName, const wchar_t *wszName)
{
	ATLASSERT( ppszName != NULL );
	ATLASSERT( wszName != NULL );
	
	size_t nMaxLen = AllocateNameString(ppszName, wszName);
	if (!nMaxLen)
	{
		return E_OUTOFMEMORY;
	}

	_CreateSafeCppName(*ppszName, CW2A( wszName ), wszName, nMaxLen);
	return S_OK;
}

HRESULT CreateSafeCppName(CStringA& strSafeName, const wchar_t *wszName)
{
	ATLASSERT( wszName != NULL );
	
	size_t nMaxLen = 3*wcslen(wszName);
	
	char *szSafeName = strSafeName.GetBuffer((int) nMaxLen);
	
	_CreateSafeCppName(szSafeName, CW2A( wszName ), wszName, nMaxLen);
	
	strSafeName.ReleaseBuffer();

	return S_OK;
}

HRESULT CreateSafeCppName(CStringA& strSafeName, const char *szName)
{
	ATLASSERT( szName != NULL );
	
	size_t nMaxLen = 3*strlen(szName);
	
	char *szSafeName = strSafeName.GetBuffer((int) nMaxLen);
	
	_CreateSafeCppName(szSafeName, szName, CA2W( szName ), nMaxLen);
	
	strSafeName.ReleaseBuffer();

	return S_OK;
}