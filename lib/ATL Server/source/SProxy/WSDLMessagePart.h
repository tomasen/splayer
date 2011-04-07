//
// WSDLPart.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "QName.h"
#include "XMLElement.h"
#include "XSDElement.h"

class CElement;
class CXSDElement;

class CWSDLMessagePart : public CXMLElement
{
private:

	CStringW m_strName;
	CQName   m_element;
	CQName   m_type;

	CElement * m_pElement;
	CXSDElement * m_pXSDElement;

public:

	inline CWSDLMessagePart()
		:m_pElement(NULL), m_pXSDElement(NULL)
	{
	}

	inline CWSDLMessagePart(CXMLDocument *pDoc)
		:CXMLElement(pDoc)
	{
	}

	inline CWSDLMessagePart(const CWSDLMessagePart& that)
	{
		*this = that;
	}

	inline const CWSDLMessagePart& operator=(const CWSDLMessagePart& that)
	{
		if (this != &that)
		{
			m_strName = that.m_strName;
			m_element = that.m_element;
		}

		return *this;
	}

	inline void SetName(const CStringW& strName)
	{
		m_strName = strName;
	}

	inline const CStringW& GetName()
	{
		return m_strName;
	}

	inline void SetElement(const CStringW& strQName)
	{
		m_element.SetQName(strQName);
	}

	inline void SetElement(const CStringW& strPrefix, const CStringW& strName)
	{
		m_element.SetQName(strPrefix, strName);
	}

	inline void SetElement(const wchar_t *wszQName, int cchQName)
	{
		m_element.SetQName(wszQName, cchQName);
	}

	inline CQName& GetElementName()
	{
		return m_element;
	}

	inline void SetType(const CStringW& strQName)
	{
		m_type.SetQName(strQName);
	}

	inline void SetType(const CStringW& strPrefix, const CStringW& strName)
	{
		m_type.SetQName(strPrefix, strName);
	}

	inline void SetType(const wchar_t *wszQName, int cchQName)
	{
		m_type.SetQName(wszQName, cchQName);
	}

	inline CQName& GetTypeName()
	{
		return m_type;
	}

	//
	// TODO: import stuff
	//
	HRESULT GetElement(CElement **ppElement);
	HRESULT GetType(CXSDElement **ppElement, XSDTYPE *pXSD);
};