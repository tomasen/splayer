//
// XMLElement.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "XMLElement.h"
#include "XMLDocument.h"
#include "Emit.h"

HRESULT CXMLElement::GetNamespaceUri(const CStringW &strPrefix, CStringW &strUri)
{
	if (m_namespaceMap.Lookup(strPrefix, strUri))
	{
		return S_OK;
	}

	if (m_pParentElement != NULL)
	{
		return m_pParentElement->GetNamespaceUri(strPrefix, strUri);
	}

	return E_FAIL;
}

LPCWSTR CXMLElement::GetNamespaceUri(LPCWSTR wszPrefix, int cchPrefix)
{
	if (cchPrefix == -1)
	{
		cchPrefix = (int)wcslen(wszPrefix);
	}
	
	CStringW strUri(wszPrefix, cchPrefix);
	
	CStringW strRet;
	if (SUCCEEDED(GetNamespaceUri(strUri, strRet)))
	{
		return (LPCWSTR) strRet;
	}
	return NULL;
}

HRESULT CXMLElement::SetNamespaceUri(const CStringW& strPrefix, CStringW &strUri)
{
	if (m_namespaceMap.SetAt(strPrefix, strUri) != NULL)
	{
		return S_OK;
	}
	EmitErrorHr(E_OUTOFMEMORY);
	return E_FAIL;
}

HRESULT CXMLElement::SetNamespaceUri(LPCWSTR wszPrefix, int cchPrefix, LPCWSTR wszUri, int cchUri)
{
	if (cchPrefix == -1)
	{
		cchPrefix = (int)wcslen(wszPrefix);
	}
	CStringW strPrefix(wszPrefix, cchPrefix);

	if (cchUri == -1)
	{
		cchUri = (int)wcslen(wszUri);
	}
	CStringW strUri(wszUri, cchUri);

	return SetNamespaceUri(strPrefix, strUri);
}