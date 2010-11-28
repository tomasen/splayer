//
// SkipParser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Parser.h"

class CSkipParser : public CParserBase
{
public:

	inline CSkipParser(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel)
		:CParserBase(pReader, pParent, dwLevel)
	{
	}

	virtual HRESULT OnUnrecognizedTag(
		const wchar_t *wszNamespaceUri, int cchNamespaceUri,
		const wchar_t *wszLocalName, int cchLocalName,
		const wchar_t *wszQName, int cchQName,
		ISAXAttributes *pAttributes) throw()
	{
		DisableReset();
		return S_OK;
	}

	BEGIN_XMLTAG_MAP()
	END_XMLTAG_MAP()
};