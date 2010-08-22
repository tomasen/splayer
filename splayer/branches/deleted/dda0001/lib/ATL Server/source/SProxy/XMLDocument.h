//
// XMLDocument.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"

enum XMLDOCTYPE
{
	//
	// generic (unknown) document type
	//
	UNKDOC = 1, 

	//
	// XML schema document
	//
	SCHEMADOC,
	
	//
	// WSDL document
	//
	WSDLDOC,

	//
	// DiscoMap document
	DMDOC
};

class CXMLDocument;
typedef CAtlPtrMap<CStringW, CXMLDocument *, CStringRefElementTraits<CStringW> > XMLDOCMAP;

class CXMLDocument : public CXMLElement
{
private:

	XMLDOCTYPE m_docType;
	CStringW m_strDocumentUri;
	CStringW m_strTargetNamespace;

	//
	// mapping of URI's to CXMLDocuments
	//
	XMLDOCMAP m_docMap;

public:
	virtual ~CXMLDocument() = 0;
	CXMLDocument(XMLDOCTYPE docType = UNKDOC);

	BOOL AddDocument(const CStringW& strUri, CXMLDocument *pDoc);
	BOOL AddDocument(LPCWSTR wszUri, int cchUri, CXMLDocument *pDoc);
	CXMLDocument * GetDocument(const CStringW& strUri);
	CXMLDocument * GetDocument(LPCWSTR wszUri, int cchUri);
	
	inline void SetDocumentType(XMLDOCTYPE docType)
	{
		m_docType = docType;
	}

	inline XMLDOCTYPE GetDocumentType()
	{
		return m_docType;
	}

	inline HRESULT SetTargetNamespace(const wchar_t *wszTargetNamespace, int cchTargetNamespace)
	{
		if (!wszTargetNamespace)
		{
			return E_FAIL;
		}

		m_strTargetNamespace.SetString(wszTargetNamespace, cchTargetNamespace);

		return S_OK;
	}

	inline HRESULT SetTargetNamespace(const CStringW& strTargetNamespace)
	{
		m_strTargetNamespace = strTargetNamespace;

		return S_OK;
	}

	inline const CStringW& GetTargetNamespace()
	{
		return m_strTargetNamespace;
	}

	inline HRESULT SetDocumentUri(const wchar_t *wszDocumentUri, int cchDocumentUri)
	{
		if (!wszDocumentUri)
		{
			return E_FAIL;
		}

		m_strDocumentUri.SetString(wszDocumentUri, cchDocumentUri);

		return S_OK;
	}

	inline HRESULT SetDocumentUri(const CStringW& strDocumentUri)
	{
		m_strDocumentUri = strDocumentUri;

		return S_OK;
	}

	inline const CStringW& GetDocumentUri()
	{
		return m_strDocumentUri;
	}
};