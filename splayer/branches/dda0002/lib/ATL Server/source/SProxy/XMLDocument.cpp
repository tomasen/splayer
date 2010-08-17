//
// XMLDocument.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "XMLDocument.h"
#include "Emit.h"

CXMLDocument::~CXMLDocument()
{
}

CXMLDocument::CXMLDocument(XMLDOCTYPE docType)
	:m_docType(docType)
{
}

BOOL CXMLDocument::AddDocument(const CStringW& strUri, CXMLDocument *pDoc)
{
	//
	// always try to add to the parent if possible
	//
	CXMLDocument * pParentDocument = GetParentDocument();
	if (pParentDocument != NULL)
	{
		return pParentDocument->AddDocument(strUri, pDoc);
	}

	if (m_docMap.SetAt(strUri, pDoc) != NULL)
	{
		return TRUE;
	}
	else
	{
		EmitErrorHr(E_OUTOFMEMORY);
		return FALSE;
	}
}

BOOL CXMLDocument::AddDocument(LPCWSTR wszUri, int cchUri, CXMLDocument *pDoc)
{
	CStringW strUri;
	strUri.SetString(wszUri, cchUri);

	return AddDocument(strUri, pDoc);
}


CXMLDocument * CXMLDocument::GetDocument(const CStringW& strUri)
{
	CXMLDocument *pDoc = NULL;
	const XMLDOCMAP::CPair *p = m_docMap.Lookup(strUri);
	if (p != NULL)
	{
		pDoc = const_cast<XMLDOCMAP::CPair *>(p)->m_value;
	}
	else
	{
		CXMLDocument * pParentDocument = GetParentDocument();
		if (pParentDocument != NULL)
		{
			pDoc = pParentDocument->GetDocument(strUri);
		}
	}

	return pDoc;
}

CXMLDocument * CXMLDocument::GetDocument(LPCWSTR wszUri, int cchUri)
{
	CStringW strUri;
	strUri.SetString(wszUri, cchUri);

	return GetDocument(strUri);
}