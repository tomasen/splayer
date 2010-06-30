//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include "stdafx.h"
#include "DiscoMapDocument.h"

CDiscoMapDocument::CDiscoMapDocument(void)
{
	SetDocumentType(DMDOC);
}

CDiscoMapDocument::~CDiscoMapDocument(void)
{
}

void CDiscoMapDocument::SetWSDLFile(const CStringW & wsdlFile)
{
	CPathW cp;
	cp.Combine(GetPath(),wsdlFile);

	m_wsdlFile = LPCWSTR(cp);
}

void CDiscoMapDocument::AddSchema(const CStringW & url, const CStringW & filename)
{
	CPathW cp;
	cp.Combine(GetPath(),filename);

	m_schemaMap.SetAt(url,LPCWSTR(cp));
}

CStringW & CDiscoMapDocument::GetWSDLFile(void)
{
	return m_wsdlFile;
}

CStringW & CDiscoMapDocument::GetValue(const CStringW & value)
{
	return m_schemaMap[value];
}

CStringW & CDiscoMapDocument::GetPath(void)
{
	if(m_strPath.IsEmpty())
	{
		CStringW strDoc(GetDocumentUri());
	
		wchar_t * pBuf = m_strPath.GetBuffer(MAX_PATH) ;
		wchar_t * p = NULL ;
		int nLength=GetFullPathNameW(LPCWSTR(strDoc),MAX_PATH,pBuf,&p);
		ATLENSURE(nLength!=0 && nLength<=MAX_PATH);
		ATLENSURE(p);
		*p = 0;
		m_strPath.ReleaseBuffer();
		
	}
	return m_strPath;
}
