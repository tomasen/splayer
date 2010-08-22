//
// CodeTypes.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XSDElement.h"

enum CODETYPE
{
	CODETYPE_ERR = 0,
	CODETYPE_UNK,
	CODETYPE_ENUM,
	CODETYPE_PARAMETER,
	CODETYPE_STRUCT,
	CODETYPE_UNION,
	CODETYPE_FIELD,
	CODETYPE_FUNCTION,
	CODETYPE_HEADER
};

enum CODEFLAGS
{
	CODEFLAG_ERR            = 0x00000000,
	CODEFLAG_UNK            = 0x00000001,
	CODEFLAG_IN             = 0x00000002,
	CODEFLAG_OUT            = 0x00000004,
	CODEFLAG_RETVAL         = 0x00000008,
	CODEFLAG_MUSTUNDERSTAND = 0x00000010,
	CODEFLAG_HEX            = 0x00000020,
	CODEFLAG_BASE64         = 0x00000040,
	CODEFLAG_FIXEDARRAY     = 0x00000080,
	CODEFLAG_DYNARRAY       = 0x00000100,

	CODEFLAG_STRUCT         = 0x00000200,
	CODEFLAG_UNION          = 0x00000400,
	CODEFLAG_ENUM           = 0x00000800,
	CODEFLAG_HEADER         = 0x00001000,

	// WSDL message styles
	CODEFLAG_DOCUMENT       = 0x00002000,
	CODEFLAG_RPC            = 0x00004000,

	// WSDL message uses
	CODEFLAG_LITERAL        = 0x00008000,
	CODEFLAG_ENCODED        = 0x00010000,

	// specific wire format for document/literal with 
	// <message ...><part ... element="..."/></message>
	CODEFLAG_PID            = 0x00020000, // ParametersInDocuments
	CODEFLAG_PAD            = 0x00040000, // ParametersAsDocuments

	// special processing required for document/literal with type=
	CODEFLAG_CHAIN          = 0x00080000,
	CODEFLAG_TYPE           = 0x00100000,
	CODEFLAG_ELEMENT        = 0x00200000,

	// one-way method
	CODEFLAG_ONEWAY         = 0x00400000,

	// MinOccurs=0/MaxOccurs=1 Wrapper
	CODEFLAG_DYNARRAYWRAPPER= 0x00800000,

	// nullable/nillable element
	CODEFLAG_NULLABLE       = 0x01000000
};

class CCodeElement
{
private:

	CStringW m_strName;
	DWORD    m_dwFlags;
	CODETYPE m_codeType;
	CCodeElement * m_pParentElement;

	// safe naming
	CStringA m_strSafeName;

public:

	CCodeElement();
	virtual ~CCodeElement() {}

	HRESULT SetName(const wchar_t *wszName, int cchName);
	HRESULT SetName(const CStringW& strName);
	const CStringW& GetName();
	CODETYPE GetCodeType();
	void SetCodeType(CODETYPE codeType);
	DWORD SetFlags(DWORD dwFlags);
	DWORD GetFlags();
	DWORD AddFlags(DWORD dwFlag);
	DWORD ClearFlags(DWORD dwFlags = 0);
	void SetParentElement(CCodeElement * pParentElement);
	CCodeElement * GetParentElement();
	const CCodeElement& operator=(const CCodeElement& that);

	CStringA& GetSafeName();
	HRESULT SetSafeName(const CStringA& strName);
};

class CCodeType
{
private:

	XSDTYPE m_xsdType;
	CStringA m_strCodeType;

	CStringA m_strSafeCodeType;

public:

	CCodeType();

	void SetXSDType(XSDTYPE xsdType);
	XSDTYPE GetXSDType();

	HRESULT SetCodeTypeName(const wchar_t *wszCodeType, int cchCodeType);
	HRESULT SetCodeTypeName(const CStringW& strCodeType);
	const CStringA& GetCodeTypeName();
	const CCodeType& operator=(const CCodeType& that);

	CStringA& GetSafeCodeTypeName();
	HRESULT SetSafeCodeTypeName(const CStringA& strSafeCodeType);
};

class CCodeTypedElement : public CCodeElement, public CCodeType
{
private:

	CXSDElement * m_pElement;
	CAtlArray<int> m_arrDims;
	
	CStringA m_strSizeIs;
	
	CStringW m_strNamespace;
	
public:

	CCodeTypedElement()
		:m_pElement(NULL)
	{
	}
	
	inline HRESULT SetNamespace(const CStringW& strNamespace)
	{
		m_strNamespace = strNamespace;
		return S_OK;
	}
	
	inline CStringW& GetNamespace()
	{
		return m_strNamespace;
	}

	inline size_t AddDimension(int nDim)
	{
		return m_arrDims.Add(nDim);
	}

	inline int GetDimension(int i)
	{
		return m_arrDims[i];
	}

	inline void SetDimension(int i, int nDim)
	{
		m_arrDims[i] = nDim;
	}

	inline int GetDims()
	{
		return (int)m_arrDims.GetCount();
	}

	inline void ClearDims()
	{
		m_arrDims.RemoveAll();
	}
	
	inline CStringA& GetSizeIs()
	{
		return m_strSizeIs;
	}
	
	inline void SetSizeIs(const CStringA& str)
	{
		m_strSizeIs = str;
	}
	
	inline void SetSizeIs(const CStringW& str)
	{
		m_strSizeIs = CW2A(const_cast<LPWSTR>((LPCWSTR) str));
	}

	inline const CCodeTypedElement& operator=(const CCodeTypedElement& that)
	{
		if (this != &that)
		{
			CCodeElement::operator=(that);
			CCodeType::operator=(that);
			m_pElement = that.m_pElement;

			// REVIEW: (jasjitg) -- ugly!
			if (that.m_arrDims.GetCount() != 0)
			{
				m_arrDims.SetCount(that.m_arrDims.GetCount());
				for (size_t i=0; i<m_arrDims.GetCount(); i++)
				{
					m_arrDims[i] = that.m_arrDims[i];
				}
			}
		}

		return (*this);
	}

	inline CCodeTypedElement(const CCodeTypedElement& that)
	{
		*this = that;
	}

	inline void SetElement(CXSDElement *pElem)
	{
		m_pElement = pElem;
	}

	inline CXSDElement * GetElement()
	{
		return m_pElement;
	}
};

class CCodeElementContainer
{
private:

	CStringA m_strName;
	// safe name
	CStringA m_strSafeName;

	CStringA m_strResponseName; // for CCodeFunction only
	CStringA m_strSendName; // for CCodeFunction only
	CStringA m_strSoapAction; // for CCodeFunction only
	CAtlPtrList<CCodeTypedElement *> m_headers; // for CCodeFunction only

	DWORD m_dwCallFlags; // for CCodeFunction only

	// namespace of the type/function
	CStringA m_strNamespace;
	CAtlPtrList<CCodeTypedElement *> m_elements;

public:

	CCodeElementContainer()
		:m_dwCallFlags(0)
	{
	}

	DWORD GetCallFlags();
	void SetCallFlags(DWORD dwCallFlags);

	CStringA& GetSafeName();

	const CStringA& GetName();
	HRESULT SetName(const CStringA& str);
	HRESULT SetName(const CStringW& str);

	const CStringA& GetResponseName();
	HRESULT SetResponseName(const CStringA& str);
	HRESULT SetResponseName(const CStringW& str);

	const CStringA& GetSendName();
	HRESULT SetSendName(const CStringA& str);
	HRESULT SetSendName(const CStringW& str);

	CStringA& GetNamespace();
	HRESULT SetNamespace(const CStringA& str);
	HRESULT SetNamespace(const CStringW& str);
	
	const CStringA& GetSoapAction();
	HRESULT SetSoapAction(const CStringA& str);
	HRESULT SetSoapAction(const CStringW& str);

	POSITION GetFirstElement();
	CCodeTypedElement * GetNextElement(POSITION &pos);
	CCodeTypedElement * AddElement(CCodeTypedElement * p = NULL);
	int GetElementCount();

	POSITION GetFirstHeader();
	CCodeTypedElement * GetNextHeader(POSITION &pos);
	CCodeTypedElement * AddHeader(CCodeTypedElement *p = NULL);
	int GetHeaderCount();
};

typedef CCodeElementContainer CCodeFunction;
typedef CCodeElementContainer CCodeStruct;
typedef CCodeElementContainer CCodeEnum;

class CCodeProxy
{
private:

	CStringA m_strClassName;
	CStringA m_strAddressUri;
	CStringA m_strTargetNamespace;
	CStringA m_strServiceName;

	CAtlPtrList<CCodeStruct *> m_structs;
	CAtlPtrList<CCodeEnum *> m_enums;
	CAtlPtrList<CCodeFunction *> m_functions;
	CAtlPtrList<CCodeTypedElement *> m_headers;

public:

	const CStringA& GetClassName();
	HRESULT SetClassName(const CStringW &strName);
	HRESULT SetClassName(const char * szName);

	const CStringA& GetServiceName();
	HRESULT SetServiceName(const CStringW &strName);
	HRESULT SetServiceName(const char * szName);
	
	const CStringA& GetAddressUri();
	HRESULT SetAddressUri(const CStringW &strName);
	HRESULT SetAddressUri(const char * szName);

	const CStringA& GetTargetNamespace();
	HRESULT SetTargetNamespace(const CStringW &strName);
	HRESULT SetTargetNamespace(const char * szName);

	POSITION GetFirstStruct();
	CCodeStruct * GetNextStruct(POSITION &pos);
	CCodeStruct * AddStruct(CCodeStruct * p = NULL);

	POSITION GetFirstEnum();
	CCodeEnum * GetNextEnum(POSITION &pos);
	CCodeEnum * AddEnum(CCodeEnum * p = NULL);

	POSITION GetFirstFunction();
	CCodeFunction * GetNextFunction(POSITION &pos);
	CCodeFunction * AddFunction(CCodeFunction * p = NULL);

	POSITION GetFirstHeader();
	CCodeTypedElement * GetNextHeader(POSITION &pos);
	CCodeTypedElement * AddHeader(CCodeTypedElement *p = NULL);
};


//////////////////////////////////////////////////////////////////
//
// CCodeElement
//
//////////////////////////////////////////////////////////////////
inline CCodeElement::CCodeElement()
	:m_dwFlags(CODEFLAG_ERR)
{
}

inline CStringA& CCodeElement::GetSafeName()
{
	return m_strSafeName;
}

inline HRESULT CCodeElement::SetSafeName(const CStringA& strName)
{
	m_strSafeName = strName;
	return S_OK;
}

inline HRESULT CCodeElement::SetName(const wchar_t *wszName, int cchName)
{
	if (!wszName)
	{
		return E_FAIL;
	}

	m_strName.SetString(wszName, cchName);

	return S_OK;
}

inline HRESULT CCodeElement::SetName(const CStringW& strName)
{
	m_strName = strName;

	return S_OK;
}

inline const CStringW& CCodeElement::GetName()
{
	return m_strName;
}

inline CODETYPE CCodeElement::GetCodeType()
{
	return m_codeType;
}

inline void CCodeElement::SetCodeType(CODETYPE codeType)
{
	m_codeType = codeType;
}

inline DWORD CCodeElement::SetFlags(DWORD dwFlags)
{
	return m_dwFlags = dwFlags;
}

inline DWORD CCodeElement::GetFlags()
{
	return m_dwFlags;
}

inline DWORD CCodeElement::AddFlags(DWORD dwFlag)
{
	m_dwFlags |= dwFlag;
	return m_dwFlags;
}

inline DWORD CCodeElement::ClearFlags(DWORD dwFlags)
{
	m_dwFlags &= ~dwFlags;
	return m_dwFlags;
}

inline void CCodeElement::SetParentElement(CCodeElement * pParentElement)
{
	m_pParentElement = pParentElement;
}
inline CCodeElement * CCodeElement::GetParentElement()
{
	return m_pParentElement;
}

inline const CCodeElement& CCodeElement::operator=(const CCodeElement& that)
{
	if (this != &that)
	{
		m_strName = that.m_strName;
		m_strSafeName = that.m_strSafeName;
		m_dwFlags = that.m_dwFlags;
		m_codeType = that.m_codeType;
		m_pParentElement = that.m_pParentElement;
	}

	return (*this);
}

//////////////////////////////////////////////////////////////////
//
// CCodeType
//
//////////////////////////////////////////////////////////////////

inline CCodeType::CCodeType()
	:m_xsdType(XSDTYPE_ERR)
{
}

inline void CCodeType::SetXSDType(XSDTYPE xsdType)
{
	m_xsdType = xsdType;
}

inline XSDTYPE CCodeType::GetXSDType()
{
	return m_xsdType;
}

inline HRESULT CCodeType::SetCodeTypeName(const wchar_t *wszCodeType, int cchCodeType)
{
	if (!wszCodeType)
	{
		return E_FAIL;
	}

	CStringW strW;
	strW.SetString(wszCodeType, cchCodeType);
	m_strCodeType = CW2A(const_cast<LPWSTR>((LPCWSTR) strW));

	return S_OK;
}

inline HRESULT CCodeType::SetCodeTypeName(const CStringW& strCodeType)
{
	m_strCodeType = strCodeType;
	return S_OK;
}

inline const CStringA& CCodeType::GetCodeTypeName()
{
	return m_strCodeType;
}

inline CStringA& CCodeType::GetSafeCodeTypeName()
{
	return m_strSafeCodeType;
}

inline HRESULT CCodeType::SetSafeCodeTypeName(const CStringA& strName)
{
	m_strSafeCodeType = strName;
	return S_OK;
}

inline const CCodeType& CCodeType::operator=(const CCodeType& that)
{
	if (this != &that)
	{
		m_strCodeType = that.m_strCodeType;
		m_strSafeCodeType = that.m_strSafeCodeType;
		m_xsdType = that.m_xsdType;
	}

	return (*this);
}

//////////////////////////////////////////////////////////////////
//
// CCodeElementContainer
//
//////////////////////////////////////////////////////////////////

inline DWORD CCodeElementContainer::GetCallFlags()
{
	return m_dwCallFlags;
}


inline CStringA& CCodeElementContainer::GetSafeName()
{
	return m_strSafeName;
}

inline void CCodeElementContainer::SetCallFlags(DWORD dwCallFlags)
{
	m_dwCallFlags = dwCallFlags;
}

inline const CStringA& CCodeElementContainer::GetName()
{
	return m_strName;
}

inline HRESULT CCodeElementContainer::SetName(const CStringA& str)
{
	m_strName = str;
	return S_OK;
}

inline HRESULT CCodeElementContainer::SetName(const CStringW& str)
{
	m_strName = CW2A( const_cast<LPWSTR>((LPCWSTR) str) );
	return S_OK;
}

inline const CStringA& CCodeElementContainer::GetResponseName()
{
	return m_strResponseName;
}

inline HRESULT CCodeElementContainer::SetResponseName(const CStringA& str)
{
	m_strResponseName = str;
	return S_OK;
}

inline HRESULT CCodeElementContainer::SetResponseName(const CStringW& str)
{
	m_strResponseName = CW2A( const_cast<LPWSTR>((LPCWSTR) str) );
	return S_OK;
}

inline const CStringA& CCodeElementContainer::GetSendName()
{
	return m_strSendName;
}

inline HRESULT CCodeElementContainer::SetSendName(const CStringA& str)
{
	m_strSendName = str;
	return S_OK;
}

inline HRESULT CCodeElementContainer::SetSendName(const CStringW& str)
{
	m_strSendName = CW2A( const_cast<LPWSTR>((LPCWSTR) str) );
	return S_OK;
}

inline CStringA& CCodeElementContainer::GetNamespace()
{
	return m_strNamespace;
}

inline HRESULT CCodeElementContainer::SetNamespace(const CStringA& str)
{
	m_strNamespace = str;
	return S_OK;
}

inline HRESULT CCodeElementContainer::SetNamespace(const CStringW& str)
{
	m_strNamespace = CW2A( const_cast<LPWSTR>((LPCWSTR) str) );
	return S_OK;
}

inline const CStringA& CCodeElementContainer::GetSoapAction()
{
	return m_strSoapAction;
}

inline HRESULT CCodeElementContainer::SetSoapAction(const CStringA& str)
{
	m_strSoapAction = str;
	return S_OK;
}

inline HRESULT CCodeElementContainer::SetSoapAction(const CStringW& str)
{
	m_strSoapAction = CW2A( const_cast<LPWSTR>((LPCWSTR) str) );
	return S_OK;
}

inline POSITION CCodeElementContainer::GetFirstElement()
{
	return m_elements.GetHeadPosition();
}

inline CCodeTypedElement * CCodeElementContainer::GetNextElement(POSITION &pos)
{
	return m_elements.GetNext(pos);
}

inline CCodeTypedElement * CCodeElementContainer::AddElement(CCodeTypedElement * p)
{
	CAutoPtr<CCodeTypedElement> spOut;
	if (p == NULL)
	{
		spOut.Attach( new CCodeTypedElement );
		p = spOut;
	}
	
	if (p != NULL)
	{
		if (m_elements.AddTail(p) != NULL)
		{
			spOut.Detach();
			return p;
		}
	}

	return NULL;
}

inline int CCodeElementContainer::GetElementCount()
{
	return (int)m_elements.GetCount();
}

inline POSITION CCodeElementContainer::GetFirstHeader()
{
	return m_headers.GetHeadPosition();
}

inline CCodeTypedElement * CCodeElementContainer::GetNextHeader(POSITION &pos)
{
	return m_headers.GetNext(pos);
}

inline CCodeTypedElement * CCodeElementContainer::AddHeader(CCodeTypedElement * p)
{
	CAutoPtr<CCodeTypedElement> spOut;
	if (p == NULL)
	{
		spOut.Attach( new CCodeTypedElement );
		p = spOut;
	}
	
	if (p != NULL)
	{
		if (m_headers.AddTail(p) != NULL)
		{
			spOut.Detach();
			return p;
		}
	}

	return NULL;
}

inline int CCodeElementContainer::GetHeaderCount()
{
	return (int)m_headers.GetCount();
}

//////////////////////////////////////////////////////////////////
//
// CCodeProxy
//
//////////////////////////////////////////////////////////////////

inline POSITION CCodeProxy::GetFirstStruct()
{
	return m_structs.GetHeadPosition();
}

inline CCodeStruct * CCodeProxy::GetNextStruct(POSITION &pos)
{
	return m_structs.GetNext(pos);
}

inline CCodeStruct * CCodeProxy::AddStruct(CCodeStruct * p)
{
	CAutoPtr<CCodeStruct> spOut;
	if (p == NULL)
	{
		spOut.Attach( new CCodeStruct);
		p = spOut;
	}
	
	if (p != NULL)
	{
		if (m_structs.AddTail(p) != NULL)
		{
			spOut.Detach();
			return p;
		}
	}

	return NULL;
}

inline POSITION CCodeProxy::GetFirstEnum()
{
	return m_enums.GetHeadPosition();
}

inline CCodeEnum * CCodeProxy::GetNextEnum(POSITION &pos)
{
	return m_enums.GetNext(pos);
}

inline CCodeEnum * CCodeProxy::AddEnum(CCodeEnum * p)
{
	CAutoPtr<CCodeEnum> spOut;
	if (p == NULL)
	{
		spOut.Attach( new CCodeEnum);
		p = spOut;
	}
	
	if (p != NULL)
	{
		if (m_enums.AddTail(p) != NULL)
		{
			spOut.Detach();
			return p;
		}
	}

	return NULL;
}

inline POSITION CCodeProxy::GetFirstFunction()
{
	return m_functions.GetHeadPosition();
}

inline CCodeFunction * CCodeProxy::GetNextFunction(POSITION &pos)
{
	return m_functions.GetNext(pos);
}

inline CCodeFunction * CCodeProxy::AddFunction(CCodeFunction * p)
{
	CAutoPtr<CCodeFunction> spOut;
	if (p == NULL)
	{
		spOut.Attach( new CCodeFunction );
		p = spOut;
	}
	
	if (p != NULL)
	{
		if (m_functions.AddTail(p) != NULL)
		{
			spOut.Detach();
			return p;
		}
	}

	return NULL;
}

inline POSITION CCodeProxy::GetFirstHeader()
{
	return m_headers.GetHeadPosition();
}

inline CCodeTypedElement * CCodeProxy::GetNextHeader(POSITION &pos)
{
	return m_headers.GetNext(pos);
}

inline CCodeTypedElement * CCodeProxy::AddHeader(CCodeTypedElement * p)
{
	CAutoPtr<CCodeTypedElement> spOut;
	if (p == NULL)
	{
		spOut.Attach( new CCodeTypedElement );
		p = spOut;
	}
	
	if (p != NULL)
	{
		if (m_headers.AddTail(p) != NULL)
		{
			spOut.Detach();
			return p;
		}
	}

	return NULL;
}

inline const CStringA& CCodeProxy::GetClassName()
{
	return m_strClassName;
}

inline HRESULT CCodeProxy::SetClassName(const CStringW &strName)
{
	return CreateSafeCppName(m_strClassName, strName);
}

inline HRESULT CCodeProxy::SetClassName(const char * szName)
{
	return CreateSafeCppName(m_strClassName, szName);
}

inline const CStringA& CCodeProxy::GetServiceName()
{
	return m_strServiceName;
}

inline HRESULT CCodeProxy::SetServiceName(const CStringW &strName)
{
	return CreateSafeCppName(m_strServiceName, strName);
}

inline HRESULT CCodeProxy::SetServiceName(const char * szName)
{
	return CreateSafeCppName(m_strServiceName, szName);
}

inline const CStringA& CCodeProxy::GetAddressUri()
{
	return m_strAddressUri;
}

inline HRESULT CCodeProxy::SetAddressUri(const CStringW &strName)
{
	m_strAddressUri = CW2A(const_cast<LPWSTR>((LPCWSTR) strName));
	return S_OK;
}

inline HRESULT CCodeProxy::SetAddressUri(const char * szName)
{
	m_strAddressUri = szName;
	return S_OK;
}

inline const CStringA& CCodeProxy::GetTargetNamespace()
{
	return m_strTargetNamespace;
}

inline HRESULT CCodeProxy::SetTargetNamespace(const CStringW &strName)
{
	m_strTargetNamespace = CW2A(const_cast<LPWSTR>((LPCWSTR) strName));
	return S_OK;
}

inline HRESULT CCodeProxy::SetTargetNamespace(const char * szName)
{
	m_strTargetNamespace = szName;
	return S_OK;
}