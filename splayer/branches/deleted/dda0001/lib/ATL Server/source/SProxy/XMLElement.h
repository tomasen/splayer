//
// XMLElement.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"

class CXMLDocument;

// TODO: set locator information for each element
struct LOCATORINFO
{
	int nLine;
	int nCol;

	LOCATORINFO()
		:nLine(0), nCol(0)
	{
	}
};

class CXMLElement
{
private:

	CXMLDocument * m_pParentDocument;
	CXMLElement * m_pParentElement;
	NAMESPACEMAP m_namespaceMap;
	LOCATORINFO m_locInfo;

public:
	virtual ~CXMLElement() = 0 {};

	inline CXMLElement(CXMLDocument *pDoc = NULL, CXMLElement * pParentElement = NULL)
		:m_pParentDocument(pDoc), m_pParentElement(pParentElement)
	{
	}

	inline CXMLElement * GetParentElement()
	{
		return m_pParentElement;
	}

	inline void SetParentElement(CXMLElement * pParentElement)
	{
		m_pParentElement = pParentElement;
	}

	inline CXMLDocument * GetParentDocument()
	{
		return m_pParentDocument;
	}

	void SetParentDocument(CXMLDocument *pDoc)
	{
		m_pParentDocument = pDoc;
	}
	
	void SetLineNumber(int nLine)
	{
		m_locInfo.nLine = nLine;
	}

	void SetColumnNumber(int nCol)
	{
		m_locInfo.nCol = nCol;
	}

	int GetLineNumber()
	{
		return m_locInfo.nLine;
	}

	int GetColumnNumber()
	{
		return m_locInfo.nCol;
	}

	HRESULT GetNamespaceUri(const CStringW &strPrefix, CStringW &strUri);
	LPCWSTR GetNamespaceUri(LPCWSTR wszPrefix, int cchPrefix = -1);
	HRESULT SetNamespaceUri(const CStringW& strPrefix, CStringW &strUri);
	HRESULT SetNamespaceUri(LPCWSTR wszPrefix, int cchPrefix, LPCWSTR wszUri, int cchUri);
};