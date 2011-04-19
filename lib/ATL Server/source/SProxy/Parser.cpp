//
// Parser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "Parser.h"
#include "SkipParser.h"
#include "resource.h"

////////////////////////////////////////////////////////////////////////
//
// CParserBase interface
//
////////////////////////////////////////////////////////////////////////

HRESULT CParserBase::DispatchElement(
	const XMLTAG *pEntry,
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t *wszQName, int cchQName,
	ISAXAttributes *pAttributes) throw()
{
	if (!pEntry)
	{
		return S_OK;
	}

	while (pEntry->szElemName)
	{
		if (CheckTagElement(pEntry, 
			wszNamespaceUri, cchNamespaceUri,
			wszLocalName, cchLocalName,
			wszQName, cchQName))
		{
			return (this->*pEntry->pfnTag)(wszNamespaceUri, 
				cchNamespaceUri, wszLocalName, cchLocalName, 
				wszQName, cchQName, pAttributes);
		}
		pEntry++;
	}

	//
	// unrecognized tag
	//
	return OnUnrecognizedTag(wszNamespaceUri, 
		cchNamespaceUri, wszLocalName, cchLocalName, 
		wszQName, cchQName, pAttributes);
}

HRESULT CParserBase::GetAttributes(const XMLATTR *pEntry, ISAXAttributes *pAttributes)
{
	if (!pAttributes)
	{
		EmitError(IDS_SDL_INTERNAL);
		return E_FAIL;
	}

	if (GetGotAttributes() != FALSE)
	{
		return S_OK;
	}

	if (!pEntry)
	{
		return S_OK;
	}

	int nAttrs = 0;
	HRESULT hr = pAttributes->getLength(&nAttrs);
	if (FAILED(hr))
	{
		EmitError(IDS_SDL_MSXML);
		return E_FAIL;
	}

	while (pEntry->wszElemName)
	{
		int i;
		for (i=0; i<nAttrs; i++)
		{
			const wchar_t *wszNamespaceUri = NULL;
			const wchar_t *wszLocalName = NULL;
			const wchar_t *wszQName = NULL;
			int cchUri = 0;
			int cchLocalName = 0;
			int cchQName = 0;

			hr = pAttributes->getName(i, &wszNamespaceUri, &cchUri, &wszLocalName, &cchLocalName, &wszQName, &cchQName);
			if (FAILED(hr))
			{
				EmitError(IDS_SDL_MSXML);
				return E_FAIL;
			}

			if (CheckTagElement(pEntry, 
					wszNamespaceUri, cchUri,
					wszLocalName, cchLocalName,
					wszQName, cchQName))
			{
				const wchar_t *wszValue = NULL;
				int cchValue = 0;

				hr = pAttributes->getValue(i, &wszValue, &cchValue);
				if (SUCCEEDED(hr))
				{
					hr = (this->*pEntry->pfnAttr)(wszNamespaceUri, 
						cchUri, wszLocalName, cchLocalName, 
						wszQName, cchQName, wszValue, cchValue,
						pAttributes);
				}
				else
				{
					EmitError(IDS_SDL_MSXML);
				}

				break;
			}
		}

		if (i >= nAttrs)
		{
			hr = OnMissingAttribute(pEntry->bRequired, pEntry->wszElemName, pEntry->cchElem, 
					pEntry->wszElemNamespace, pEntry->cchNamespaceUri);
		}

		if (FAILED(hr))
		{
			break;
		}

		pEntry++;
	}

	SetGotAttributes(TRUE);
	return hr;
}

HRESULT CParserBase::DispatchElement(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t *wszQName, int cchQName,
	ISAXAttributes *pAttributes) throw()
{

	if (FAILED(GetAttributes(pAttributes)))
	{
		return E_FAIL;
	}

	const XMLTAG * pEntry = GetXMLTAGMap();

	return DispatchElement(pEntry, wszNamespaceUri, 
		cchNamespaceUri, wszLocalName, cchLocalName, 
		wszQName, cchQName, pAttributes);
}

HRESULT CParserBase::GetAttributes(ISAXAttributes *pAttributes)
{
	const XMLATTR * pEntry = GetXMLATTRMap();
	return GetAttributes(pEntry, pAttributes);
}

BOOL CParserBase::CheckTagElement(
		const XMLTAG *pTag,
		const wchar_t *wszNamespaceUri, int cchNamespaceUri,
		const wchar_t *wszLocalName, int cchLocalName,
		const wchar_t *wszQName, int cchQName)
{
	if (!pTag->wszElemName)
	{
		EmitError(IDS_SDL_INTERNAL);
		return FALSE;
	}

	if (pTag->cchElem == cchLocalName && 
		!wcsncmp(pTag->wszElemName, wszLocalName, pTag->cchElem))
	{
		//
		// Don't need to check default namespace for tags since SAX gives it in wszElemNamespace
		//
		if ((!pTag->wszElemNamespace) ||
			(!pTag->cchNamespaceUri && !cchNamespaceUri) ||
			(pTag->cchNamespaceUri == cchNamespaceUri && 
			 !wcsncmp(pTag->wszElemNamespace, wszNamespaceUri, pTag->cchNamespaceUri)))
		{
			//
			// namespace and tag match
			// it is okay if there is no namespace ("") or if the user doesn't care (NULL)
			//
			return TRUE;
		}
	}

	return FALSE;
}

void CParserBase::CrackQName(const wchar_t *wszQName, int cchQName,
	wchar_t **pwszNs, int *pcchNs,
	wchar_t **pwszName, int *pcchName)
{
	wchar_t * wszQNameTmp = (wchar_t *) wszQName;
	wchar_t * wszColon = wcschr(wszQNameTmp, L':');
	if (wszColon && (wszColon-wszQNameTmp) <= cchQName)
	{
		*pwszNs = wszQNameTmp;
		*pcchNs = (int)(wszColon-wszQNameTmp);

		*pwszName = wszColon+1;
		*pcchName = cchQName-(*pcchNs)-1;
	}
	else
	{
		*pwszNs = NULL;
		*pcchNs = 0;
		*pwszName = wszQNameTmp;
		*pcchName = cchQName;
	}
}

HRESULT CParserBase::OnUnrecognizedTag(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t *wszQName, int cchQName,
	ISAXAttributes *pAttributes) throw()
{
	wszNamespaceUri;
	cchNamespaceUri;
	wszLocalName;
	cchLocalName;
	wszQName;
	cchQName;
	pAttributes;

	ATLTRACE( _T("%sUnrecoginzed Tag: %.*ws\n"), GetTabs(m_dwLevel), cchQName, wszQName );

	int nLine;
	int nCol;

	GetLocator()->getLineNumber(&nLine);
	GetLocator()->getColumnNumber(&nCol);

	EmitFileError(IDS_SDL_UNRECOGNIZED_TAG, GetWSDLFile(), nLine, nCol, 0, wszNamespaceUri, wszLocalName);
	return E_FAIL;
}

HRESULT CParserBase::OnMissingAttribute(BOOL bRequired, 
	const wchar_t *wszName, int cchName,
	const wchar_t *wszNamespace, int cchNamespace)
{
	if (bRequired != FALSE)
	{
		ATLTRACE( _T("%sMissing Required Attribute: name %.*ws, uri %.*ws\n"), 
			GetTabs(m_dwLevel), cchName, wszName, cchNamespace, wszNamespace );

		int nLine;
		int nCol;

		GetLocator()->getLineNumber(&nLine);
		GetLocator()->getColumnNumber(&nCol);

		EmitFileError(IDS_SDL_MISSING_ATTRIBUTE, GetWSDLFile(), nLine, nCol, 0, wszNamespace, wszName);
		return E_FAIL;
	}
	return S_OK;
}

void CParserBase::EmitInvalidValue(const char *szName, const wchar_t *wszValue)
{
	int nLine;
	int nCol;
	GetLocator()->getLineNumber(&nLine);
	GetLocator()->getColumnNumber(&nCol);
	EmitFileError(IDS_SDL_INVALID_VALUE, GetWSDLFile(), nLine, nCol, 0, szName, wszValue);
}

void CParserBase::EmitUnsupported(UINT nID, const wchar_t *wszUri, const wchar_t *wszName)
{
	int nLine;
	int nCol;
	GetLocator()->getLineNumber(&nLine);
	GetLocator()->getColumnNumber(&nCol);
	EmitFileWarning(nID, GetWSDLFile(), nLine, nCol, 0, wszUri, wszName);
}

void CParserBase::EmitSkip(const wchar_t *wszUri, const wchar_t *wszName)
{
	EmitUnsupported(IDS_SDL_UNSUPPORTED_TAG, wszUri, wszName);
}

void CParserBase::EmitString(const wchar_t *wszUri, const wchar_t *wszName)
{
	EmitUnsupported(IDS_SDL_UNSUPPORTED_STRING, wszUri, wszName);
}

HRESULT CParserBase::SkipElement()
{
	CAutoPtr<CSkipParser> p( new CSkipParser(m_spReader, this, GetLevel()) );
	if (p != NULL)
	{
		if (g_ParserList.AddHead(p) != NULL)
		{
			p.Detach();
			return S_OK;
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////
//
// IUnknown
//
////////////////////////////////////////////////////////////////////////

HRESULT __stdcall CParserBase::QueryInterface(REFIID riid, void **ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}

	if (InlineIsEqualGUID(riid, __uuidof(ISAXContentHandler)) ||
		InlineIsEqualGUID(riid, __uuidof(IUnknown)))
	{
		*ppv = static_cast<IUnknown*>(this);
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG __stdcall CParserBase::AddRef()
{
	return 1;
}

ULONG __stdcall CParserBase::Release()
{
	return 1;
}

////////////////////////////////////////////////////////////////////////
//
// ISAXContentHandler
//
////////////////////////////////////////////////////////////////////////

HRESULT __stdcall CParserBase::putDocumentLocator(ISAXLocator  *pLocator)
{
	m_spLocator = pLocator;
	return S_OK;
}
        
HRESULT __stdcall CParserBase::startDocument()
{
	return S_OK;
}

HRESULT __stdcall CParserBase::endDocument()
{
	return S_OK;
}

HRESULT __stdcall CParserBase::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	return S_OK;
}

HRESULT __stdcall CParserBase::endPrefixMapping( 
     const wchar_t  *wszPrefix,
     int cchPrefix)
{
	wszPrefix;
	cchPrefix;

	// REVIEW: remove from map?

	return S_OK;
}

HRESULT __stdcall CParserBase::startElement( 
     const wchar_t  *wszNamespaceUri,
     int cchNamespaceUri,
     const wchar_t  *wszLocalName,
     int cchLocalName,
     const wchar_t  *wszQName,
     int cchQName,
     ISAXAttributes  *pAttributes)
{
	return DispatchElement(wszNamespaceUri, cchNamespaceUri, 
		wszLocalName, cchLocalName, 
		wszQName, cchQName, 
		pAttributes);
}

HRESULT __stdcall CParserBase::endElement( 
     const wchar_t  *wszNamespaceUri,
     int cchNamespaceUri,
     const wchar_t  *wszLocalName,
     int cchLocalName,
     const wchar_t  *wszQName,
     int cchQName)
{
	HRESULT hr = ValidateElement();

	if (GetReset() > 0)
	{
		EnableReset();
		return hr;
	}

	//
	// Restore the parent handler
	//
	CComPtr<CParserBase> spParentHandler = GetParentHandler();
	if (spParentHandler.p != NULL)
	{
		m_spReader->putContentHandler(spParentHandler);
	}

	if (GetDynamicAlloc() != FALSE)
	{
		POSITION pos = g_ParserList.Find(this);
		if (pos != NULL)
		{
			g_ParserList.RemoveAt(pos);
		}
		delete this;
	}

	return hr;
}

HRESULT __stdcall CParserBase::characters( 
     const wchar_t  *wszChars,
     int cchChars)
{
	//
	// REVIEW: what to do here?
	//
//	ATLTRACE( _T("CParserBase::characters: %*.ws\n"), cchChars, wszChars );
	return S_OK;
}

HRESULT __stdcall CParserBase::ignorableWhitespace( 
     const wchar_t  *wszChars,
     int cchChars)
{
//	ATLTRACE( _T("CParserBase::ignorableWhitespace: %.*ws\n"), cchChars, wszChars );
	return S_OK;
}

HRESULT __stdcall CParserBase::processingInstruction( 
     const wchar_t  *wszTarget,
     int cchTarget,
     const wchar_t  *wszData,
     int cchData)
{
	wszTarget;
	cchTarget;
	wszData;
	cchData;

	ATLTRACE( _T("CParserBase::processingInstruction: target: %.*ws, data: %.*ws\n"), cchTarget, wszTarget, cchData, wszData );
	return S_OK;
}

HRESULT __stdcall CParserBase::skippedEntity( 
     const wchar_t  *wszName,
     int cchName)
{
	wszName;
	cchName;

	ATLTRACE( _T("CParserBase::skippedEntity: %.*ws\n"), wszName, cchName );
	return S_OK;
}