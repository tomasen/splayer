//
// Parser.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLDocument.h"
#include "XSDElement.h"
#include "Emit.h"
#include "SproxyColl.h"

#define PTM_WARNING_DISABLE \
	__pragma(warning( push )) \
	__pragma(warning( disable : 4867 ))

#define PTM_WARNING_RESTORE \
	__pragma(warning( pop ))

#define EMPTY_XMLTAG_MAP() \
	virtual const XMLTAG * GetXMLTAGMap() \
	{ \
		return NULL; \
	} 

#define EMPTY_XMLATTR_MAP() \
	virtual const XMLATTR * GetXMLATTRMap() \
	{ \
		return NULL; \
	}

#define BEGIN_XMLTAG_MAP() \
	virtual const XMLTAG * GetXMLTAGMap() \
	{ \
		PTM_WARNING_DISABLE \
		static const XMLTAG map[] = \
		{

#define END_XMLTAG_MAP() \
	{ NULL, NULL } }; \
	PTM_WARNING_RESTORE \
	return map; \
	}

#define _XMLTAG_ENTRY_EX(elementName, elementNamespace, elementFunc) \
	{ L ## elementName, elementName, sizeof(elementName)-1, \
	  L ## elementNamespace, elementNamespace, sizeof(elementNamespace)-1, \
	  static_cast<TAG_DISPATCH_FUNC>(elementFunc), NULL, FALSE },

#define XMLTAG_ENTRY_EX(elementName, elementNamespace, elementFunc) _XMLTAG_ENTRY_EX(elementName, elementNamespace, elementFunc)

#define _XMLTAG_ENTRY(elementName, elementFunc) \
	{ L ## elementName, elementName, sizeof(elementName)-1, \
		NULL, NULL, 0, \
	  static_cast<TAG_DISPATCH_FUNC>(elementFunc), NULL, FALSE },

#define XMLTAG_ENTRY(elementName, elementFunc) _XMLTAG_ENTRY(elementName, elementFunc)

#define BEGIN_XMLATTR_MAP() \
	virtual const XMLATTR * GetXMLATTRMap() \
	{ \
		PTM_WARNING_DISABLE \
		static const XMLTAG map[] = \
		{

#define END_XMLATTR_MAP() \
	{ NULL, NULL } }; \
	PTM_WARNING_RESTORE \
	return map; \
	}

#define _XMLATTR_ENTRY_EX(elementName, elementNamespace, elementFunc, required) \
	{ L ## elementName, elementName, sizeof(elementName)-1, \
	  L ## elementNamespace, elementNamespace, sizeof(elementNamespace)-1, \
	  NULL, static_cast<ATTR_DISPATCH_FUNC>(elementFunc), required },

#define XMLATTR_ENTRY_EX(elementName, elementNamespace, elementFunc) \
	_XMLATTR_ENTRY_EX(elementName, elementNamespace, elementFunc, FALSE)

#define XMLATTR_ENTRY_EX2(elementName, elementNamespace, elementFunc, required) \
	_XMLATTR_ENTRY_EX(elementName, elementNamespace, elementFunc, required)

#define _XMLATTR_ENTRY(elementName, elementFunc, required) \
	{ L ## elementName, elementName, sizeof(elementName)-1, \
		NULL, NULL, 0, \
	  NULL, static_cast<ATTR_DISPATCH_FUNC>(elementFunc), required },

#define XMLATTR_ENTRY(elementName, elementFunc) _XMLATTR_ENTRY(elementName, elementFunc, FALSE)
#define XMLATTR_ENTRY2(elementName, elementFunc, required) _XMLATTR_ENTRY(elementName, elementFunc, required)

class CParserBase;

typedef HRESULT (CParserBase::*TAG_DISPATCH_FUNC)(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t *wszQName, int cchQName,
	ISAXAttributes *pAttributes);

typedef HRESULT (CParserBase::*ATTR_DISPATCH_FUNC)(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t *wszQName, int cchQName,
	const wchar_t *wszValue, int cchValue,
	ISAXAttributes *pAttributes);

#define TAG_METHOD_DECL( name ) \
	HRESULT name(const wchar_t *wszNamespaceUri, int cchNamespaceUri, \
		const wchar_t *wszLocalName, int cchLocalName, \
		const wchar_t *wszQName, int cchQName, \
		ISAXAttributes *pAttributes)

#define TAG_METHOD_IMPL( class, name ) \
	HRESULT class ## :: ## name(const wchar_t *wszNamespaceUri, int cchNamespaceUri, \
		const wchar_t *wszLocalName, int cchLocalName, \
		const wchar_t *wszQName, int cchQName, \
		ISAXAttributes *pAttributes)

#define ATTR_METHOD_DECL( name ) \
	HRESULT name(const wchar_t *wszNamespaceUri, int cchNamespaceUri, \
		const wchar_t *wszLocalName, int cchLocalName, \
		const wchar_t *wszQName, int cchQName, \
		const wchar_t *wszValue, int cchValue, \
		ISAXAttributes *pAttributes)

#define ATTR_METHOD_IMPL( class, name ) \
	HRESULT class ## :: ## name(const wchar_t *wszNamespaceUri, int cchNamespaceUri, \
		const wchar_t *wszLocalName, int cchLocalName, \
		const wchar_t *wszQName, int cchQName, \
		const wchar_t *wszValue, int cchValue, \
		ISAXAttributes *pAttributes)

typedef NAMESPACEMAP URIMAP;

#ifndef _PREFIX_
#define TRACE_PARSE_ENTRY() \
	ATLTRACE( _T("%s") __FUNCTION__ _T("\n"), GetTabs(GetLevel()))
#else
#define TRACE_PARSE_ENTRY() \
	ATLTRACE( _T("%s") _T("\n"), GetTabs(GetLevel()))
#endif // _PREFIX_


//
// XMLTAG struct
// used for dispatching XML entries
//
struct XMLTAG
{
	LPCWSTR wszElemName;
	LPCSTR szElemName;
	int cchElem;
	
	LPCWSTR wszElemNamespace;
	LPCSTR szElemNamespace;
	int cchNamespaceUri;

	//
	// REVIEW: union doesn't work for initialization
	//
	TAG_DISPATCH_FUNC pfnTag;
	ATTR_DISPATCH_FUNC pfnAttr;

	//
	// attribute must be present
	//
	BOOL bRequired;
};

typedef XMLTAG XMLATTR;

//
// dummy classes for multiple inheritance
// (tell compiler to use big pointers for CParserBase member function pointers)
//
class CDummy1 {};
class CDummy2 {};

//
// The base parser class
//
class CParserBase : public ISAXContentHandler, public CDummy1, public CDummy2
{
private:
	
	CComPtr<CParserBase> m_spParent;
	CComPtr<ISAXXMLReader> m_spReader;
	CComPtr<ISAXLocator> m_spLocator;

	//
	// how many levels deep
	//
	DWORD m_dwLevel;

	//
	// reset to parent handler on endElement
	//
	DWORD m_dwReset;

	//
	// get only top-level attributes
	//
	BOOL m_bGotAttributes;

	//
	// dynamically allocated
	//
	BOOL m_bDynamicAlloc;

private:

	HRESULT DispatchElement(
		const XMLTAG *pMap,
		const wchar_t *wszNamespaceUri, int cchNamespaceUri,
		const wchar_t *wszLocalName, int cchLocalName,
		const wchar_t *wszQName, int cchQName,
		ISAXAttributes *pAttributes) throw ();

	HRESULT GetAttributes(const XMLATTR *pMap, ISAXAttributes *pAttributes);

	BOOL CheckTagElement(
		const XMLTAG *pTag,
		const wchar_t *wszNamespaceUri, int cchNamespaceUri,
		const wchar_t *wszLocalName, int cchLocalName,
		const wchar_t *wszQName, int cchQName);

	void CrackQName(const wchar_t *wszQName, int cchQName,
		wchar_t **pwszNs, int *pcchNs,
		wchar_t **pwszName, int *pcchName);

public:

	//
	// CParserBase interface
	//
	inline CParserBase()
		:m_dwLevel(0), m_dwReset(0), m_bDynamicAlloc(TRUE), m_bGotAttributes(FALSE)
	{
	}

	inline CParserBase(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel)
		:m_spReader(pReader), m_spParent(pParent), m_dwLevel(dwLevel+1), m_dwReset(0), m_bDynamicAlloc(TRUE), m_bGotAttributes(FALSE)
	{
		if (m_spReader.p != NULL)
		{
			m_spReader->putContentHandler(this);
		}
	}

	inline HRESULT Initialize(ISAXXMLReader *pReader, CParserBase *pParent, DWORD dwLevel, BOOL bDynamicAlloc = FALSE)
	{
		m_spReader = pReader;
		m_spParent = pParent;
		m_dwLevel = dwLevel+1;
		m_bDynamicAlloc = bDynamicAlloc;
		m_bGotAttributes = FALSE;
		m_dwReset = 0;

		if (m_spReader.p != NULL)
		{
			return m_spReader->putContentHandler(this);
		}

		return E_FAIL;
	}

	virtual ~CParserBase()
	{
		if ((m_spParent.p != NULL) && (m_spReader.p != NULL))
		{
			m_spReader->putContentHandler(m_spParent);
		}
	}

	EMPTY_XMLTAG_MAP()
	EMPTY_XMLATTR_MAP()

	HRESULT DispatchElement(
		const wchar_t *wszNamespaceUri, int cchNamespaceUri,
		const wchar_t *wszLocalName, int cchLocalName,
		const wchar_t *wszQName, int cchQName,
		ISAXAttributes *pAttributes) throw ();

	HRESULT GetAttributes(ISAXAttributes *pAttributes);

	inline CParserBase * GetParentHandler()
	{
		return m_spParent;
	}

	inline void SetParentHandler(CParserBase *pParent)
	{
		m_spParent = pParent;
	}

	ISAXXMLReader * GetReader()
	{
		return m_spReader;
	}

	inline void SetReader(ISAXXMLReader *pReader)
	{
		m_spReader = pReader;
	}

	inline DWORD GetLevel()
	{
		return m_dwLevel;
	}

	inline void SetLevel(DWORD dwLevel)
	{
		m_dwLevel = dwLevel;
	}

	virtual HRESULT OnUnrecognizedTag(
		const wchar_t *wszNamespaceUri, int cchNamespaceUri,
		const wchar_t *wszLocalName, int cchLocalName,
		const wchar_t *wszQName, int cchQName,
		ISAXAttributes *pAttributes) throw ();

	virtual HRESULT OnMissingAttribute(BOOL bRequired, 
		const wchar_t *wszName, int cchName,
		const wchar_t *wszNamespace, int cchNamespace);

	HRESULT SkipElement();
	void EmitInvalidValue(const char *szName, const wchar_t *wszValue);
	void EmitUnsupported(UINT nID, const wchar_t *wszUri, const wchar_t *wszName);
	void EmitSkip(const wchar_t *wszUri, const wchar_t *wszName);
	void EmitString(const wchar_t *wszUri, const wchar_t *wszName);

	inline ISAXLocator * GetLocator()
	{
		if (m_spLocator.p != NULL)
		{
			return m_spLocator;
		}
		if (m_spParent.p != NULL)
		{
			return m_spParent->GetLocator();
		}
		return NULL;
	}

	virtual HRESULT ValidateElement()
	{
		return S_OK;
	}

	inline void SetGotAttributes(BOOL bGotAttributes)
	{
		m_bGotAttributes = bGotAttributes;
	}

	inline BOOL GetGotAttributes()
	{
		return m_bGotAttributes;
	}

	inline DWORD GetReset()
	{
		return m_dwReset;
	}

	inline void EnableReset(BOOL bForce = FALSE)
	{
		if (bForce != FALSE)
		{
			m_dwReset = 0;
		}
		else if (m_dwReset > 0)
		{
			m_dwReset--;
		}
	}

	inline void DisableReset(DWORD dwCnt = 1)
	{
		m_dwReset+= dwCnt;
	}

	inline BOOL GetDynamicAlloc()
	{
		return m_bDynamicAlloc;
	}

	inline void SetDynamicAlloc(BOOL bDynamicAlloc)
	{
		m_bDynamicAlloc = bDynamicAlloc;
	}

	//
	// IUnknown interface
	//
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	
	//
	// ISAXContentHandler interface
	//
	HRESULT __stdcall putDocumentLocator(ISAXLocator  *pLocator);

	HRESULT __stdcall startDocument();

	HRESULT __stdcall endDocument();

	HRESULT __stdcall startPrefixMapping(
	     const wchar_t  *wszPrefix,
	     int cchPrefix,
	     const wchar_t  *wszUri,
	     int cchUri);

	HRESULT __stdcall endPrefixMapping( 
	     const wchar_t  *wszPrefix,
	     int cchPrefix);

	HRESULT __stdcall startElement( 
	     const wchar_t  *wszNamespaceUri,
	     int cchNamespaceUri,
	     const wchar_t  *wszLocalName,
	     int cchLocalName,
	     const wchar_t  *wszQName,
	     int cchQName,
	     ISAXAttributes  *pAttributes);

	HRESULT __stdcall endElement( 
	     const wchar_t  *wszNamespaceUri,
	     int cchNamespaceUri,
	     const wchar_t  *wszLocalName,
	     int cchLocalName,
	     const wchar_t  *wszQName,
	     int cchQName);

	HRESULT __stdcall characters( 
	     const wchar_t  *wszChars,
	     int cchChars);

	HRESULT __stdcall ignorableWhitespace( 
	     const wchar_t  *wszChars,
	     int cchChars);

	HRESULT __stdcall processingInstruction( 
	     const wchar_t  *wszTarget,
	     int cchTarget,
	     const wchar_t  *wszData,
	     int cchData);

	HRESULT __stdcall skippedEntity( 
	     const wchar_t  *wszName,
	     int cchName);
};

inline HRESULT CopyNamespaceMap(const NAMESPACEMAP& in, NAMESPACEMAP& out)
{
	out.InitHashTable(in.GetHashTableSize());
	out.DisableAutoRehash();
	POSITION pos = in.GetStartPosition();
	while (pos != NULL)
	{
		const NAMESPACEMAP::CPair *p = in.GetNext(pos);
		if (!out.SetAt(p->m_key, p->m_value))
		{
			return E_FAIL;
		}
	}
	out.EnableAutoRehash();

	return S_OK;
}

// for cleanup purposes
#ifndef _PREFIX_
__declspec(selectany) CAtlPtrList<CParserBase *> g_ParserList;
#else
CAtlPtrList<CParserBase *> g_ParserList;
#endif // _PREFIX_

inline void SetLocatorInfo(CXMLElement *pElem, ISAXLocator *pLocator)
{
	ATLASSERT( pLocator != NULL );
	ATLASSERT( pElem != NULL );
	
	int nLine = 0;
	int nCol = 0;

	pLocator->getLineNumber(&nLine);
	pLocator->getColumnNumber(&nCol);

	pElem->SetLineNumber(nLine);
	pElem->SetColumnNumber(nCol);
}


ATL_NOINLINE inline void SetXMLElementInfo(CXMLElement *pElem, CXMLElement *pParent, ISAXLocator *pLocator)
{
	ATLASSERT( pElem != NULL );
	ATLASSERT( pParent != NULL );
	ATLASSERT( pLocator != NULL );

	pElem->SetParentElement(pParent);
	pElem->SetParentDocument(pParent->GetParentDocument());
	SetLocatorInfo(pElem, pLocator);
}

ATL_NOINLINE inline void SetXSDElementInfo(CXSDElement *pElem, CXSDElement *pParent, ISAXLocator *pLocator)
{
	ATLASSERT( pElem != NULL );
	ATLASSERT( pParent != NULL );
	ATLASSERT( pLocator != NULL );

	pElem->SetParentSchema(pParent->GetParentSchema());
	pElem->SetParentElement(pParent);
	pElem->SetParentDocument(pParent->GetParentDocument());
	SetLocatorInfo(pElem, pLocator);
}