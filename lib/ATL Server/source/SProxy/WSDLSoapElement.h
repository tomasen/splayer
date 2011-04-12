//
// WSDLSoapElement.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLElement.h"
#include "QName.h"
#include "Emit.h"

// this file contains common definitions for
//     soap:header
//     soap:headerfault
//     soap:body
//     soap:fault

enum SOAPUSE
{
	SOAPUSE_UNK = -1,
	SOAPUSE_LITERAL,
	SOAPUSE_ENCODED
};

inline SOAPUSE GetSoapUse(const wchar_t *wsz, int cch)
{
	struct _soapuse
	{
		wchar_t *wsz;
		int cch;
		SOAPUSE soapuse;
	};

	SOAPUSE retUse = SOAPUSE_UNK;

	// data driven is kind of overkill for two options, but makes it 
	// easy to extend later
	static const _soapuse s_uses[] =
	{
		{ L"literal", sizeof("literal")-1, SOAPUSE_LITERAL },
		{ L"encoded", sizeof("encoded")-1, SOAPUSE_ENCODED }
	};

	for (int i=0; i<(sizeof(s_uses)/sizeof(s_uses[0])); i++)
	{
		if (cch == s_uses[i].cch && !wcsncmp(wsz, s_uses[i].wsz, cch))
		{
			retUse = s_uses[i].soapuse;
			break;
		}
	}

	return retUse;
}

enum SOAPSTYLE
{
	SOAPSTYLE_UNK = -1,
	SOAPSTYLE_RPC,
	SOAPSTYLE_DOC
};

inline SOAPSTYLE GetStyle(const wchar_t *wsz, int cch)
{
	struct _style
	{
		wchar_t * wszStyle;
		int cchStyle;
		SOAPSTYLE style;
	};

	static const _style s_styles[] =
	{
		{ L"rpc", sizeof("rpc")-1, SOAPSTYLE_RPC }, 
		{ L"document", sizeof("document")-1, SOAPSTYLE_DOC }
	};

	for (int i=0; i<(sizeof(s_styles)/sizeof(s_styles[0])); i++)
	{
		if (cch == s_styles[i].cchStyle && !wcsncmp(wsz, s_styles[i].wszStyle, cch))
		{
			return s_styles[i].style;
		}
	}

	return SOAPSTYLE_UNK;
}

// forward declaration of CWSDLMessage
class CWSDLMessage;

// CWSDLSoapElement is the common base class for
//     soap:header
//     soap:headerfault
//     soap:body
//     soap:fault
class CWSDLSoapElement : public CXMLElement
{
private:

	SOAPUSE m_use;
	CStringW m_strEncodingStyle;
	CStringW m_strNamespace;

public:

	CWSDLSoapElement()
		: m_use(SOAPUSE_UNK)
	{
	}

	inline HRESULT SetUse(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_use = GetSoapUse(wszName, cchName);
		return (m_use != SOAPUSE_UNK) ? S_OK : E_FAIL;
	}

	inline HRESULT SetUse(const CStringW& strName)
	{
		return (GetSoapUse(strName, strName.GetLength()) != SOAPUSE_UNK) ? S_OK : E_FAIL;
	}

	inline const SOAPUSE GetUse()
	{
		return m_use;
	}

	inline HRESULT SetNamespace(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strNamespace.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetNamespace(const CStringW& strName)
	{
		m_strNamespace = strName;

		return S_OK;
	}

	inline CStringW& GetNamespace()
	{
		return m_strNamespace;
	}

	inline HRESULT SetEncodingStyle(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strEncodingStyle.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetEncodingStyle(const CStringW& strName)
	{
		m_strEncodingStyle = strName;

		return S_OK;
	}

	inline CStringW& GetEncodingStyle()
	{
		return m_strEncodingStyle;
	}
}; // class CWSDLSoapElement

// CWSDLSoapElementEx is an extended base class for
//     soap:body
//     soap:header
//     soap:headerfault
class CWSDLSoapElementEx : public CWSDLSoapElement
{
private:

	CStringW m_strParts;

public:

	inline HRESULT SetParts(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strParts.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetParts(const CStringW& strName)
	{
		m_strParts = strName;

		return S_OK;
	}

	inline CStringW& GetParts()
	{
		return m_strParts;
	}
};

class CWSDLMessage;

// CWSDLSoapHeaderElement is an extended class for
//     soap:header
//     soap:headerfault
class CWSDLSoapHeaderElement : public CWSDLSoapElementEx
{
private:

	CQName m_message;
	CWSDLMessage *m_pMessage;
	bool m_bRequired;

public:

	CWSDLSoapHeaderElement()
		: m_pMessage(NULL), m_bRequired(false)
	{
	}

	inline HRESULT SetMessage(const CStringW& strQName)
	{
		m_message.SetQName(strQName);

		return S_OK;
	}

	inline HRESULT SetMessage(const CStringW& strPrefix, const CStringW& strName)
	{
		m_message.SetQName(strPrefix, strName);

		return S_OK;
	}

	inline HRESULT SetMessage(const wchar_t *wszQName, int cchQName)
	{
		m_message.SetQName(wszQName, cchQName);

		return S_OK;
	}

	inline CQName& GetMessageName()
	{
		return m_message;
	}

	// TODO: determine how/when these get set
	inline void SetRequired(bool bRequired)
	{
		m_bRequired = bRequired;
	}

	inline bool GetRequired()
	{
		return m_bRequired;
	}

	CWSDLMessage * GetMessage();
}; // class CWSDLSoapHeaderElement

// CWSDLSoapFaultElement is an extended class for
//     soap:fault
class CWSDLSoapFaultElement : public CWSDLSoapElement
{
private:

	CStringW m_strName;

public:

	inline HRESULT SetName(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strName.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetName(const CStringW& strName)
	{
		m_strName = strName;

		return S_OK;
	}

	inline CStringW& GetName()
	{
		return m_strName;
	}
}; // class CWSDLSoapFaultElement

typedef CWSDLSoapElementEx CSoapBody;
typedef CWSDLSoapHeaderElement CSoapHeader;
typedef CWSDLSoapHeaderElement CSoapHeaderFault;
typedef CWSDLSoapFaultElement CSoapFault;