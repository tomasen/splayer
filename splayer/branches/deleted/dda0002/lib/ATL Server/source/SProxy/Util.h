//
// Util.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"

HRESULT GetAttribute(
	ISAXAttributes *pAttributes, 
	const wchar_t *wszAttrName, int cchName, 
	const wchar_t **pwszValue, int *pcchValue,
	wchar_t *wszNamespace = NULL, int cchNamespace = 0);

HRESULT GetAttribute(
	ISAXAttributes *pAttributes, 
	const wchar_t *wszAttrName, int cchName, 
	CStringW &strValue,
	wchar_t *wszNamespace = NULL, int cchNamespace = 0);
	
inline HRESULT GetBooleanValue(bool *pbOut, const wchar_t *wsz, int cch)
{
	if (pbOut == NULL)
	{
		return E_POINTER;
	}
	
	if (wsz == NULL)
	{
		return E_INVALIDARG;
	}
	
	HRESULT hr = E_FAIL;
	
	switch (wsz[0])
	{
		case L'1':
		{
			if (cch == 1)
			{
				*pbOut = true;
				hr = S_OK;
			}
			break;
		}
		case L'0':
		{
			if (cch == 1)
			{
				*pbOut = false;
				hr = S_OK;
			}
			break;
		}
		case L't':
		{
			if ((cch==sizeof("true")-1) && (!wcsncmp(wsz, L"true", cch)))
			{
				*pbOut = true;
				hr = S_OK;
			}
			break;
		}
		case L'f':
		{
			if ((cch==sizeof("false")-1) && (!wcsncmp(wsz, L"false", cch)))
			{
				*pbOut = false;
				hr = S_OK;
			}
			break;
		}
	}
	
	return hr;
}