//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#pragma once
#include "xmldocument.h"

class CDiscoMapDocument :
	public CXMLDocument
{
public:
	CDiscoMapDocument(void);
	~CDiscoMapDocument(void);
private:
	typedef CAtlMap<CStringW, CStringW> SCHEMAMAP;

public:
	void SetWSDLFile(const CStringW & wsdlFile);
private:
	CStringW m_wsdlFile;
	SCHEMAMAP m_schemaMap;
public:
	void AddSchema(const CStringW & url, const CStringW & filename);
	CStringW & GetWSDLFile(void);
	CStringW & GetValue(const CStringW & value);
private:
	CStringW m_strPath;
protected:
	CStringW & GetPath(void);
};
