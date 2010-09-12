//
// CppCodeGenerator.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "CodeTypes.h"

struct CXSDEntry
{
	char * szType;
	int cchType;
	char * szCppType;
	int cchCppType;
};

extern bool g_bUseWchar_t;

#define DECLARE_XSD_ENTRY(__name, __cppType) \
	{ __name, sizeof(__name)-1, __cppType, sizeof(__cppType)-1 }, 

extern __declspec(selectany) const CXSDEntry g_soapTypes[] =
{
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_BOOLEAN", "bool")
	DECLARE_XSD_ENTRY("SOAPTYPE_FLOAT", "float")
	DECLARE_XSD_ENTRY("SOAPTYPE_DOUBLE", "double")
	DECLARE_XSD_ENTRY("SOAPTYPE_DECIMAL", "double")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_DURATION", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_HEXBINARY", "ATLSOAP_BLOB")
	DECLARE_XSD_ENTRY("SOAPTYPE_BASE64BINARY", "ATLSOAP_BLOB")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_ANYURI", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_ID", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_IDREF", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_ENTITY", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_NOTATION", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_QNAME", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_NORMALIZEDSTRING", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_TOKEN", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_LANGUAGE", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_IDREFS", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_ENTITIES", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_NMTOKEN", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_NMTOKENS", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_NAME", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_NCNAME", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_INTEGER", "__int64")
	DECLARE_XSD_ENTRY("SOAPTYPE_NONPOSITIVEINTEGER", "__int64")
	DECLARE_XSD_ENTRY("SOAPTYPE_NEGATIVEINTEGER", "__int64")
	DECLARE_XSD_ENTRY("SOAPTYPE_LONG", "__int64")
	DECLARE_XSD_ENTRY("SOAPTYPE_INT", "int")
	DECLARE_XSD_ENTRY("SOAPTYPE_SHORT", "short")
	DECLARE_XSD_ENTRY("SOAPTYPE_BYTE", "char")
	DECLARE_XSD_ENTRY("SOAPTYPE_NONNEGATIVEINTEGER", "unsigned __int64")
	DECLARE_XSD_ENTRY("SOAPTYPE_UNSIGNEDLONG", "unsigned __int64")
	DECLARE_XSD_ENTRY("SOAPTYPE_UNSIGNEDINT", "unsigned int")
	DECLARE_XSD_ENTRY("SOAPTYPE_UNSIGNEDSHORT", "wchar_t")
	DECLARE_XSD_ENTRY("SOAPTYPE_UNSIGNEDBYTE", "unsigned char")
	DECLARE_XSD_ENTRY("SOAPTYPE_POSITIVEINTEGER", "unsigned __int64")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_DATETIME", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_TIME", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_DATE", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_GMONTH", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_GYEARMONTH", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_GYEAR", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_GMONTHDAY", "BSTR")
	DECLARE_XSD_ENTRY("SOAPTYPE_STRING", "BSTR") //	DECLARE_XSD_ENTRY("SOAPTYPE_GDAY", "BSTR")
};

class CCppCodeGenerator : 
	public ITagReplacerImpl<CCppCodeGenerator>, 
	public CComObjectRootEx<CComSingleThreadModel>
{
public:
	
	BEGIN_COM_MAP(CCppCodeGenerator)
		COM_INTERFACE_ENTRY(IUnknown)
		COM_INTERFACE_ENTRY(ITagReplacer)
	END_COM_MAP()

private:

	CCodeProxy *m_pProxy;

	POSITION m_currFunctionPos;
	POSITION m_currParameterPos;
	POSITION m_currStructPos;
	POSITION m_currStructFieldPos;
	POSITION m_currEnumPos;
	POSITION m_currEnumFieldPos;
	POSITION m_currMemberPos;
	POSITION m_currHeaderPos;

	CWriteStreamHelper m_writeHelper;

	// generic counter
	int m_nCntr;
	
	// function counter
	int m_nFuncIndex;
	
	bool m_bPragma;
	bool m_bEmitNamespace;
	bool m_bGenProxy;

	const char *m_szNamespace;

protected:

public:

	CCppCodeGenerator(CCodeProxy *pProxy = NULL)
		: m_pProxy(pProxy),
		m_currFunctionPos(NULL),
		m_currParameterPos(NULL),
		m_currStructPos(NULL),
		m_currStructFieldPos(NULL),
		m_currEnumPos(NULL),
		m_currEnumFieldPos(NULL),
		m_currMemberPos(NULL),
		m_currHeaderPos(NULL),
		m_nCntr(0),
		m_nFuncIndex(-1),
		m_bPragma(true),
		m_bEmitNamespace(true),
		m_szNamespace(NULL),
		m_bGenProxy(true)
	{
	}

	HTTP_CODE WriteCString(const CStringA& str)
	{
		CStringA strTemp(str);
		strTemp.Replace("\\", "\\\\");
		strTemp.Replace("\"", "\\\"");
		strTemp.Replace("?", "\\?");
		strTemp.Replace("\r", "\\r");
		strTemp.Replace("\t", "\\t");
		strTemp.Replace("\'", "\\\'");
		strTemp.Replace("\n", "\\n");
		return (m_pStream->WriteStream((LPCSTR) strTemp, strTemp.GetLength(), NULL) == S_OK) ? HTTP_SUCCESS : HTTP_FAIL;
	}

	HTTP_CODE WriteCString(const CStringW& str)
	{
		CStringA strTemp(str);
		return WriteCString(strTemp);
	}

	inline ULONG AtlSoapHashStr(const wchar_t * sz)
	{
		ULONG nHash = 0;
		while (*sz != 0)
		{
			nHash = (nHash<<5)+nHash+(*sz);
			sz++;
		}

		return nHash;
	}

	HTTP_CODE WriteHash(ULONG nHash)
	{
		char szBuf[256];
		int nLen = sprintf(szBuf, "0x%.8X", nHash);
		return (m_pStream->WriteStream(szBuf, nLen, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_FAIL;
	}

	HTTP_CODE GetHashW(LPCSTR szStr)
	{
		return WriteHash(AtlSoapHashStr( CA2W(szStr) ));
	}

	HTTP_CODE GetHashW(LPCWSTR szStr)
	{
		return WriteHash(AtlSoapHashStr( szStr ));
	}

	CCodeEnum * GetCurrentEnum()
	{
		POSITION pos = m_currEnumPos;
		CCodeEnum *p = m_pProxy->GetNextEnum(pos);

		ATLASSERT( p != NULL );

		return p;
	}

	CCodeTypedElement * GetCurrentEnumElement()
	{
		CCodeEnum * p = GetCurrentEnum();

		POSITION pos = m_currEnumFieldPos;
		CCodeTypedElement *pElem = p->GetNextElement(pos);

		ATLASSERT( pElem != NULL );

		return pElem;
	}

	CCodeStruct * GetCurrentStruct()
	{
		POSITION pos = m_currStructPos;
		CCodeStruct *p = m_pProxy->GetNextStruct(pos);

		ATLASSERT( p != NULL );

		return p;
	}

	CCodeTypedElement * GetCurrentStructField()
	{
		CCodeStruct * p = GetCurrentStruct();

		POSITION pos = m_currStructFieldPos;
		CCodeTypedElement *pElem = p->GetNextElement(pos);

		ATLASSERT( pElem != NULL );

		return pElem;
	}

	CCodeTypedElement * GetCurrentMember()
	{
		POSITION pos = m_currMemberPos;
		CCodeTypedElement *p = m_pProxy->GetNextHeader(pos);
		
		ATLASSERT( p != NULL );

		return p;
	}

	HTTP_CODE IsFixedArray(CCodeTypedElement *p)
	{
		if (p->GetDims() != 0)
		{
			return HTTP_SUCCESS;
		}
		return HTTP_S_FALSE;
	}

	HTTP_CODE GetDimsDecl(CCodeTypedElement *p)
	{
		if (p->GetDims() != 0)
		{
			m_pStream->WriteStream("{", 1, NULL);
			for (int i=0; i<=p->GetDimension(0); i++)
			{
				if ((m_writeHelper.Write(p->GetDimension(i)) == FALSE) ||
					(S_OK != m_pStream->WriteStream(", ", 2, NULL)))
				{
					return HTTP_FAIL;
				}
			}
			m_pStream->WriteStream("}", 1, NULL);
		}

		return HTTP_SUCCESS;
	}

	HTTP_CODE GetAtlSoapType(CCodeTypedElement *p)
	{
		XSDTYPE xsdType = p->GetXSDType();

		ATLASSERT( xsdType != XSDTYPE_ERR );

		if (xsdType != XSDTYPE_UNK)
		{
			if (SUCCEEDED(m_pStream->WriteStream(
					g_soapTypes[xsdType].szType, 
					g_soapTypes[xsdType].cchType, NULL)))
			{
				return HTTP_SUCCESS;
			}
		}
		else
		{
			if (SUCCEEDED(m_pStream->WriteStream("SOAPTYPE_UNK", sizeof("SOAPTYPE_UNK")-1, NULL)))
			{
				return HTTP_SUCCESS;
			}
		}

		return HTTP_FAIL;
	}
	
	HTTP_CODE IsUDT(CCodeTypedElement *p)
	{
		ATLASSERT( p != NULL );
		ATLASSERT( p->GetXSDType() != XSDTYPE_ERR );

		if (p->GetXSDType() != XSDTYPE_UNK)
		{
			return HTTP_S_FALSE;
		}
		return HTTP_SUCCESS;
	}

	CCodeFunction * GetCurrentFunction()
	{
		POSITION pos = m_currFunctionPos;
		CCodeFunction *p = m_pProxy->GetNextFunction(pos);

		ATLASSERT( p != NULL );

		return p;
	}

	CCodeTypedElement * GetCurrentParameter()
	{
		CCodeFunction * p = GetCurrentFunction();

		POSITION pos = m_currParameterPos;
		CCodeTypedElement *pElem = p->GetNextElement(pos);

		ATLASSERT( pElem != NULL );

		return pElem;
	}

	CCodeTypedElement * GetCurrentHeader()
	{
		CCodeFunction * p = GetCurrentFunction();

		POSITION pos = m_currHeaderPos;
		CCodeTypedElement *pElem = p->GetNextHeader(pos);

		ATLASSERT( pElem != NULL );

		return pElem;
	}

	HTTP_CODE GetCppType(CCodeTypedElement *p)
	{
		XSDTYPE xsdType = p->GetXSDType();

		ATLASSERT( xsdType != XSDTYPE_ERR );

		if (xsdType != XSDTYPE_UNK)
		{
			if (!g_bUseWchar_t && xsdType == XSDTYPE_UNSIGNEDSHORT)
			{
				if (SUCCEEDED(m_pStream->WriteStream(
						"unsigned short", 
						sizeof("unsigned short")-1, NULL)))
				{
					return HTTP_SUCCESS;
				}
			}
			else 
			{
				if (SUCCEEDED(m_pStream->WriteStream(
						g_soapTypes[xsdType].szCppType, 
						g_soapTypes[xsdType].cchCppType, NULL)))
				{
					return HTTP_SUCCESS;
				}
			}
		}
		else
		{
			return WriteCString(p->GetCodeTypeName());
		}

		return HTTP_FAIL;
	}

	HTTP_CODE GetSafeCppType(CCodeTypedElement *p)
	{
		XSDTYPE xsdType = p->GetXSDType();

		ATLASSERT( xsdType != XSDTYPE_ERR );

		if (xsdType != XSDTYPE_UNK)
		{
			if (!g_bUseWchar_t && xsdType == XSDTYPE_UNSIGNEDSHORT)
			{
				if (SUCCEEDED(m_pStream->WriteStream(
						"unsigned short", 
						sizeof("unsigned short")-1, NULL)))
				{
					return HTTP_SUCCESS;
				}
			}
			else 
			{
				if (SUCCEEDED(m_pStream->WriteStream(
						g_soapTypes[xsdType].szCppType, 
						g_soapTypes[xsdType].cchCppType, NULL)))
				{
					return HTTP_SUCCESS;
				}
			}
		}
		else
		{
			return WriteCString(p->GetSafeCodeTypeName());
		}

		return HTTP_FAIL;
	}

	HTTP_CODE GetTypeSuffix(CCodeTypedElement *p)
	{
		if (p->GetDims() == 0)
		{
			return HTTP_SUCCESS;
		}

		HTTP_CODE hcErr = HTTP_SUCCESS;
		for (int i=1; (i<=p->GetDimension(0)) && (hcErr == HTTP_SUCCESS); i++)
		{
			hcErr = HTTP_FAIL;
			if (SUCCEEDED(m_pStream->WriteStream("[", 1, NULL)))
			{
				if (m_writeHelper.Write(p->GetDimension(i)) != FALSE)
				{
					m_pStream->WriteStream("]", 1, NULL);
					hcErr = HTTP_SUCCESS;
				}
			}
		}

		return hcErr;
	}

	HTTP_CODE OnGetNextEnum();
	HTTP_CODE OnGetEnumSafeQName();
	HTTP_CODE OnGetNextEnumElement();
	HTTP_CODE OnGetEnumElementHashW();
	HTTP_CODE OnGetEnumElementName();
	HTTP_CODE OnGetEnumElementValue();
	HTTP_CODE OnResetCounter();
	HTTP_CODE OnGetEnumNameHashW();
	HTTP_CODE OnGetEnumName();
	HTTP_CODE OnGetEnumQName();
	HTTP_CODE OnGetEnumCppName();
	HTTP_CODE OnGetEnumSafeCppQName();
	HTTP_CODE OnGetEnumElementCppName();
	HTTP_CODE OnGetEnumCppQName();

	HTTP_CODE OnGetNextStruct();
	HTTP_CODE OnGetStructSafeQName();
	HTTP_CODE OnGetNextStructField();
	HTTP_CODE OnGetStructFieldType();
	HTTP_CODE OnGetStructFieldSuffix();
	HTTP_CODE OnGetStructSafeCppQName();
	HTTP_CODE OnGetStructFieldCppType();

	HTTP_CODE OnIsFieldNullable();

	HTTP_CODE OnIsFieldFixedArray();
	HTTP_CODE OnGetStructFieldName();
	HTTP_CODE OnGetStructFieldDimsDecl();
	HTTP_CODE OnGetStructFieldHashW();
	HTTP_CODE OnGetStructFieldAtlSoapType();
	HTTP_CODE OnGetStructQName();
	HTTP_CODE OnIsFieldUDT();
	HTTP_CODE OnGetStructFieldTypeSafeQName();
	HTTP_CODE OnGetStructNameHashW();
	HTTP_CODE OnGetStructName();
	HTTP_CODE OnGetStructFieldCount();
	HTTP_CODE OnGetStructFieldCppName();
	HTTP_CODE OnGetStructCppQName();
	HTTP_CODE OnGetStructCppName();
	HTTP_CODE OnGetStructFieldTypeSafeCppQName();

	HTTP_CODE OnGetNextFunction();
	HTTP_CODE OnGetNextParameter();

	HTTP_CODE OnIsParameterNullable();

	HTTP_CODE OnIsParameterFixedArray();
	HTTP_CODE OnGetClassSafeQName();
	HTTP_CODE OnGetFunctionName();
	HTTP_CODE OnGetParameterName();
	HTTP_CODE OnGetParameterNameRaw();
	HTTP_CODE OnGetParameterDimsDecl();
	HTTP_CODE OnGetParameterType();
	HTTP_CODE OnIsParameterDynamicArray();
	HTTP_CODE OnGetParameterSuffix();
	HTTP_CODE OnIsInParameter();
	HTTP_CODE OnHasRetval();
	HTTP_CODE OnGetRetval();
	HTTP_CODE OnGetParameterHashW();
	HTTP_CODE OnGetFunctionCppName();
	HTTP_CODE OnGetParameterCppName();
	HTTP_CODE OnGetParameterCppType();

	HTTP_CODE OnGetParameterAtlSoapType();
	HTTP_CODE OnIsOutParameter();
	HTTP_CODE OnIsParameterUDT();

	HTTP_CODE OnGetParameterTypeQName();
	HTTP_CODE OnGetSizeIsIndex();
	HTTP_CODE OnGetCurrentParameterIndex();
	HTTP_CODE OnResetParameterIndex();
	HTTP_CODE OnNotIsRetval();
	HTTP_CODE OnGetFunctionNameHashW();
	HTTP_CODE OnGetFunctionResultNameHashW();
	HTTP_CODE OnGetFunctionResultName();
	HTTP_CODE OnGetFunctionSendName();
	HTTP_CODE OnGetExpectedParameterCount();
	HTTP_CODE OnGetParameterTypeCppQName();

	HTTP_CODE OnIsPAD();
	HTTP_CODE OnIsChain();
	HTTP_CODE OnIsPID();
	HTTP_CODE OnIsDocument();
	HTTP_CODE OnIsRpc();
	HTTP_CODE OnIsLiteral();
	HTTP_CODE OnIsEncoded();
	HTTP_CODE OnIsOneWay();

	HTTP_CODE OnGetClassName();
	HTTP_CODE OnNotIsParameterFixedArray();
	HTTP_CODE OnNotIsLastParameter();
	HTTP_CODE OnGetParameterFixedArraySize();
	HTTP_CODE OnGetDateTime();
	HTTP_CODE OnGetURL();
	HTTP_CODE OnGetSoapAction();
	HTTP_CODE OnGetNamespace();
	HTTP_CODE OnEmitNamespace();
	HTTP_CODE OnGetCppNamespace();
	HTTP_CODE OnClassHasHeaders();
	HTTP_CODE OnGetNextMember();
	HTTP_CODE OnGetMemberType();
	HTTP_CODE OnGetMemberName();
	HTTP_CODE OnGetMemberSuffix();
	HTTP_CODE OnIsMemberFixedArray();
	HTTP_CODE OnIsMemberUDT();
	HTTP_CODE OnIsMemberEnum();
	HTTP_CODE OnGetMemberCppType();
	HTTP_CODE OnGetMemberCppName();

	HTTP_CODE OnGetNextHeader();
	HTTP_CODE OnIsHeaderFixedArray();
	HTTP_CODE OnGetHeaderValue();
	HTTP_CODE OnGetHeaderDimsDecl();
	HTTP_CODE OnGetHeaderHashW();
	HTTP_CODE OnGetHeaderAtlSoapType();
	HTTP_CODE OnGetHeaderCppValue();

	HTTP_CODE OnIsHeaderNullable();
	HTTP_CODE OnIsInHeader();
	HTTP_CODE OnIsOutHeader();
	HTTP_CODE OnIsRequiredHeader();
	HTTP_CODE OnIsHeaderUDT();
	HTTP_CODE OnGetHeaderTypeQName();
	HTTP_CODE OnGetExpectedHeaderCount();
	HTTP_CODE OnHeaderHasNamespace();
	HTTP_CODE OnGetHeaderNamespace();
	HTTP_CODE OnGetHeaderNamespaceHashW();
	
	
	HTTP_CODE OnGetFunctionIndex();
	HTTP_CODE OnEmitPragma();
	HTTP_CODE OnGetEnumNamespaceHashW();
	HTTP_CODE OnEnumHasUniqueNamespace();
	HTTP_CODE OnGetEnumNamespace();
	HTTP_CODE OnGetStructNamespaceHashW();
	HTTP_CODE OnStructHasUniqueNamespace();
	HTTP_CODE OnGetStructNamespace();
	HTTP_CODE OnGetFunctionNamespaceHashW();
	HTTP_CODE OnGetFunctionNamespace();
	HTTP_CODE OnGetHeaderTypeCppQName();
	
	HTTP_CODE OnIsFieldDynamicArray();
	HTTP_CODE OnIsFieldDynamicArrayWrapper();
	HTTP_CODE OnIsFieldSizeIs();
	HTTP_CODE OnFieldHasSizeIs();
	HTTP_CODE OnGetFieldSizeIsName();
	HTTP_CODE OnGetFieldSizeIsIndex();
	HTTP_CODE OnGetCurrentFieldIndex();
	
	HTTP_CODE OnGenProxy();

	BEGIN_REPLACEMENT_METHOD_MAP(CCppCodeGenerator)
		REPLACEMENT_METHOD_ENTRY("GetNextEnum", OnGetNextEnum)
		REPLACEMENT_METHOD_ENTRY("GetEnumSafeQName", OnGetEnumSafeQName)
		REPLACEMENT_METHOD_ENTRY("GetNextEnumElement", OnGetNextEnumElement)
		REPLACEMENT_METHOD_ENTRY("GetEnumElementHashW", OnGetEnumElementHashW)
		REPLACEMENT_METHOD_ENTRY("GetEnumElementName", OnGetEnumElementName)
		REPLACEMENT_METHOD_ENTRY("GetEnumElementValue", OnGetEnumElementValue)
		REPLACEMENT_METHOD_ENTRY("ResetCounter", OnResetCounter)
		REPLACEMENT_METHOD_ENTRY("GetEnumNameHashW", OnGetEnumNameHashW)
		REPLACEMENT_METHOD_ENTRY("GetEnumName", OnGetEnumName)
		REPLACEMENT_METHOD_ENTRY("GetEnumQName", OnGetEnumQName)

		REPLACEMENT_METHOD_ENTRY("GetNextStruct", OnGetNextStruct)
		REPLACEMENT_METHOD_ENTRY("GetStructSafeQName", OnGetStructSafeQName)
		REPLACEMENT_METHOD_ENTRY("GetNextStructField", OnGetNextStructField)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldType", OnGetStructFieldType)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldSuffix", OnGetStructFieldSuffix)

		REPLACEMENT_METHOD_ENTRY("IsFieldNullable", OnIsFieldNullable)

		REPLACEMENT_METHOD_ENTRY("IsFieldFixedArray", OnIsFieldFixedArray)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldName", OnGetStructFieldName)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldDimsDecl", OnGetStructFieldDimsDecl)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldHashW", OnGetStructFieldHashW)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldName", OnGetStructFieldName)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldAtlSoapType", OnGetStructFieldAtlSoapType)
		REPLACEMENT_METHOD_ENTRY("GetStructQName", OnGetStructQName)
		REPLACEMENT_METHOD_ENTRY("IsFieldUDT", OnIsFieldUDT)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldTypeSafeQName", OnGetStructFieldTypeSafeQName)
		REPLACEMENT_METHOD_ENTRY("GetStructNameHashW", OnGetStructNameHashW)
		REPLACEMENT_METHOD_ENTRY("GetStructName", OnGetStructName)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldCount", OnGetStructFieldCount)

		REPLACEMENT_METHOD_ENTRY("GetNextFunction", OnGetNextFunction)
		REPLACEMENT_METHOD_ENTRY("GetNextParameter", OnGetNextParameter)

		REPLACEMENT_METHOD_ENTRY("IsParameterNullable", OnIsParameterNullable)

		REPLACEMENT_METHOD_ENTRY("IsParameterFixedArray", OnIsParameterFixedArray)
		REPLACEMENT_METHOD_ENTRY("GetClassSafeQName", OnGetClassSafeQName)
		REPLACEMENT_METHOD_ENTRY("GetFunctionName", OnGetFunctionName)
		REPLACEMENT_METHOD_ENTRY("GetParameterName", OnGetParameterName)
		REPLACEMENT_METHOD_ENTRY("GetParameterNameRaw", OnGetParameterNameRaw)
		REPLACEMENT_METHOD_ENTRY("GetParameterDimsDecl", OnGetParameterDimsDecl)
		REPLACEMENT_METHOD_ENTRY("GetParameterType", OnGetParameterType)
		REPLACEMENT_METHOD_ENTRY("IsParameterDynamicArray", OnIsParameterDynamicArray)
		REPLACEMENT_METHOD_ENTRY("GetParameterSuffix", OnGetParameterSuffix)
		REPLACEMENT_METHOD_ENTRY("IsInParameter", OnIsInParameter)
		REPLACEMENT_METHOD_ENTRY("HasRetval", OnHasRetval)
		REPLACEMENT_METHOD_ENTRY("GetRetval", OnGetRetval)
		REPLACEMENT_METHOD_ENTRY("GetParameterHashW", OnGetParameterHashW)

		REPLACEMENT_METHOD_ENTRY("GetParameterAtlSoapType", OnGetParameterAtlSoapType)
		REPLACEMENT_METHOD_ENTRY("IsOutParameter", OnIsOutParameter)
		REPLACEMENT_METHOD_ENTRY("IsParameterUDT", OnIsParameterUDT)

		REPLACEMENT_METHOD_ENTRY("GetParameterTypeQName", OnGetParameterTypeQName)
		REPLACEMENT_METHOD_ENTRY("GetSizeIsIndex", OnGetSizeIsIndex)
		REPLACEMENT_METHOD_ENTRY("GetCurrentParameterIndex", OnGetCurrentParameterIndex)
		REPLACEMENT_METHOD_ENTRY("ResetParameterIndex", OnResetParameterIndex)
		REPLACEMENT_METHOD_ENTRY("NotIsRetval", OnNotIsRetval)
		REPLACEMENT_METHOD_ENTRY("GetFunctionNameHashW", OnGetFunctionNameHashW)
		REPLACEMENT_METHOD_ENTRY("GetFunctionResultNameHashW", OnGetFunctionResultNameHashW)
		REPLACEMENT_METHOD_ENTRY("GetFunctionResultName", OnGetFunctionResultName)
		REPLACEMENT_METHOD_ENTRY("GetFunctionSendName", OnGetFunctionSendName)
		REPLACEMENT_METHOD_ENTRY("GetExpectedParameterCount", OnGetExpectedParameterCount)

		REPLACEMENT_METHOD_ENTRY("IsPAD", OnIsPAD)
		REPLACEMENT_METHOD_ENTRY("IsChain", OnIsChain)
		REPLACEMENT_METHOD_ENTRY("IsPID", OnIsPID)
		REPLACEMENT_METHOD_ENTRY("IsDocument", OnIsDocument)
		REPLACEMENT_METHOD_ENTRY("IsRpc", OnIsRpc)
		REPLACEMENT_METHOD_ENTRY("IsLiteral", OnIsLiteral)
		REPLACEMENT_METHOD_ENTRY("IsEncoded", OnIsEncoded)
		REPLACEMENT_METHOD_ENTRY("IsOneWay", OnIsOneWay)

		REPLACEMENT_METHOD_ENTRY("GetClassName", OnGetClassName)
		REPLACEMENT_METHOD_ENTRY("NotIsParameterFixedArray", OnNotIsParameterFixedArray)
		REPLACEMENT_METHOD_ENTRY("NotIsLastParameter", OnNotIsLastParameter)
		REPLACEMENT_METHOD_ENTRY("GetParameterFixedArraySize", OnGetParameterFixedArraySize)
		REPLACEMENT_METHOD_ENTRY("GetSoapAction", OnGetSoapAction)

		REPLACEMENT_METHOD_ENTRY("GetDateTime", OnGetDateTime)
		REPLACEMENT_METHOD_ENTRY("GetURL", OnGetURL)
		REPLACEMENT_METHOD_ENTRY("GetNamespace", OnGetNamespace)
		REPLACEMENT_METHOD_ENTRY("EmitNamespace", OnEmitNamespace)
		REPLACEMENT_METHOD_ENTRY("GetCppNamespace", OnGetCppNamespace)

		REPLACEMENT_METHOD_ENTRY("ClassHasHeaders", OnClassHasHeaders)
		REPLACEMENT_METHOD_ENTRY("GetNextMember", OnGetNextMember)
		REPLACEMENT_METHOD_ENTRY("GetMemberType", OnGetMemberType)
		REPLACEMENT_METHOD_ENTRY("GetMemberName", OnGetMemberName)
		REPLACEMENT_METHOD_ENTRY("GetMemberSuffix", OnGetMemberSuffix)
		REPLACEMENT_METHOD_ENTRY("IsMemberFixedArray", OnIsMemberFixedArray)
		REPLACEMENT_METHOD_ENTRY("IsMemberUDT", OnIsMemberUDT)
		REPLACEMENT_METHOD_ENTRY("IsMemberEnum", OnIsMemberEnum)

		REPLACEMENT_METHOD_ENTRY("GetNextHeader", OnGetNextHeader)
		REPLACEMENT_METHOD_ENTRY("IsHeaderFixedArray", OnIsHeaderFixedArray)
		REPLACEMENT_METHOD_ENTRY("GetHeaderValue", OnGetHeaderValue)
		REPLACEMENT_METHOD_ENTRY("GetHeaderDimsDecl", OnGetHeaderDimsDecl)
		REPLACEMENT_METHOD_ENTRY("GetHeaderHashW", OnGetHeaderHashW)
		REPLACEMENT_METHOD_ENTRY("GetHeaderAtlSoapType", OnGetHeaderAtlSoapType)

		REPLACEMENT_METHOD_ENTRY("IsHeaderNullable", OnIsHeaderNullable)
		REPLACEMENT_METHOD_ENTRY("IsInHeader", OnIsInHeader)
		REPLACEMENT_METHOD_ENTRY("IsOutHeader", OnIsOutHeader)
		REPLACEMENT_METHOD_ENTRY("IsRequiredHeader", OnIsRequiredHeader)
		REPLACEMENT_METHOD_ENTRY("IsHeaderUDT", OnIsHeaderUDT)
		REPLACEMENT_METHOD_ENTRY("GetHeaderTypeQName", OnGetHeaderTypeQName)
		REPLACEMENT_METHOD_ENTRY("GetExpectedHeaderCount", OnGetExpectedHeaderCount)
		REPLACEMENT_METHOD_ENTRY("GetFunctionIndex", OnGetFunctionIndex)
		REPLACEMENT_METHOD_ENTRY("HeaderHasNamespace", OnHeaderHasNamespace)
		REPLACEMENT_METHOD_ENTRY("GetHeaderNamespace", OnGetHeaderNamespace)
		REPLACEMENT_METHOD_ENTRY("GetHeaderNamespaceHashW", OnGetHeaderNamespaceHashW)
		
		
		REPLACEMENT_METHOD_ENTRY("EmitPragma", OnEmitPragma)

		REPLACEMENT_METHOD_ENTRY("GetEnumNamespaceHashW", OnGetEnumNamespaceHashW)
		REPLACEMENT_METHOD_ENTRY("EnumHasUniqueNamespace", OnEnumHasUniqueNamespace)
		REPLACEMENT_METHOD_ENTRY("GetEnumNamespace", OnGetEnumNamespace)

		REPLACEMENT_METHOD_ENTRY("GetStructNamespaceHashW", OnGetStructNamespaceHashW)
		REPLACEMENT_METHOD_ENTRY("StructHasUniqueNamespace", OnStructHasUniqueNamespace)
		REPLACEMENT_METHOD_ENTRY("GetStructNamespace", OnGetStructNamespace)
		REPLACEMENT_METHOD_ENTRY("GetFunctionNamespaceHashW", OnGetFunctionNamespaceHashW)
		REPLACEMENT_METHOD_ENTRY("GetFunctionNamespace", OnGetFunctionNamespace)
		
		REPLACEMENT_METHOD_ENTRY("IsFieldSizeIs", OnIsFieldSizeIs)
		REPLACEMENT_METHOD_ENTRY("IsFieldDynamicArray", OnIsFieldDynamicArray)
		REPLACEMENT_METHOD_ENTRY("IsFieldDynamicArrayWrapper", OnIsFieldDynamicArrayWrapper)
		REPLACEMENT_METHOD_ENTRY("FieldHasSizeIs", OnFieldHasSizeIs)
		REPLACEMENT_METHOD_ENTRY("GetFieldSizeIsIndex", OnGetFieldSizeIsIndex)
		REPLACEMENT_METHOD_ENTRY("GetCurrentFieldIndex", OnGetCurrentFieldIndex)
		REPLACEMENT_METHOD_ENTRY("GetFieldSizeIsName", OnGetFieldSizeIsName)

		// safe-naming functions
		REPLACEMENT_METHOD_ENTRY("GetHeaderTypeCppQName", OnGetHeaderTypeCppQName)
		REPLACEMENT_METHOD_ENTRY("GetMemberCppType", OnGetMemberCppType)
		REPLACEMENT_METHOD_ENTRY("GetMemberCppName", OnGetMemberCppName)
		REPLACEMENT_METHOD_ENTRY("GetParameterTypeCppQName", OnGetParameterTypeCppQName)
		REPLACEMENT_METHOD_ENTRY("GetFunctionCppName", OnGetFunctionCppName)
		REPLACEMENT_METHOD_ENTRY("GetParameterCppName", OnGetParameterCppName)
		REPLACEMENT_METHOD_ENTRY("GetParameterCppType", OnGetParameterCppType)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldCppName", OnGetStructFieldCppName)
		REPLACEMENT_METHOD_ENTRY("GetStructCppQName", OnGetStructCppQName)
		REPLACEMENT_METHOD_ENTRY("GetStructCppName", OnGetStructCppName)
		REPLACEMENT_METHOD_ENTRY("GetStructSafeCppQName", OnGetStructSafeCppQName)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldCppType", OnGetStructFieldCppType)
		REPLACEMENT_METHOD_ENTRY("GetEnumCppName", OnGetEnumCppName)
		REPLACEMENT_METHOD_ENTRY("GetEnumSafeCppQName", OnGetEnumSafeCppQName)
		REPLACEMENT_METHOD_ENTRY("GetEnumElementCppName", OnGetEnumElementCppName)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldTypeSafeCppQName", OnGetStructFieldTypeSafeCppQName)
		REPLACEMENT_METHOD_ENTRY("GetHeaderCppValue", OnGetHeaderCppValue)
		REPLACEMENT_METHOD_ENTRY("GetEnumCppQName", OnGetEnumCppQName)
		
		REPLACEMENT_METHOD_ENTRY("GenProxy", OnGenProxy)
	END_REPLACEMENT_METHOD_MAP()

	HRESULT Generate(LPCWSTR wszFile, CCodeProxy *pProxy, bool bPragma, bool bNoClobber, bool bEmitNamespace, bool bGenProxy, const char *szNamespace);
};


////////////////////////////////////////////////////////////////////////////////
//
// CWriteStreamOnFileA
//
////////////////////////////////////////////////////////////////////////////////

class CWriteStreamOnFileA : public IWriteStream
{
private:

	HANDLE m_hFile;

public:

	CWriteStreamOnFileA()
		:m_hFile(INVALID_HANDLE_VALUE)
	{
	}

	~CWriteStreamOnFileA()
	{
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hFile);
		}
	}

	HRESULT Init(LPCWSTR wszFile, DWORD dwCreationDisposition = CREATE_NEW)
	{
		if ((wszFile == NULL) || (*wszFile == L'\0'))
		{
			return E_INVALIDARG;
		}

		m_hFile = CreateFileW(wszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
			dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			return AtlHresultFromLastError();
		}

		return S_OK;
	}

	HRESULT WriteStream(LPCSTR szOut, int nLen, LPDWORD pdwWritten)
	{
		ATLASSERT( szOut != NULL );
		ATLASSERT( m_hFile != INVALID_HANDLE_VALUE );

		if (nLen < 0)
		{
			nLen = (int) strlen(szOut);
		}

		DWORD dwWritten = 0;
		if (WriteFile(m_hFile, szOut, nLen, &dwWritten, NULL) != FALSE)
		{
			if (pdwWritten != NULL)
			{
				*pdwWritten = dwWritten;
			}

			return S_OK;
		}

		return AtlHresultFromLastError();
	}

	HRESULT FlushStream()
	{
		ATLASSERT( m_hFile != INVALID_HANDLE_VALUE );

		if (FlushFileBuffers(m_hFile) != FALSE)
		{
			return S_OK;
		}

		return AtlHresultFromLastError();
	}
}; // class CWriteStreamOnFileA

#ifdef _DEBUG

class CWriteStreamOnStdout : public IWriteStream
{
public:

	HRESULT WriteStream(LPCSTR szOut, int nLen, LPDWORD pdwWritten)
	{
		ATLASSERT( szOut != NULL );

		if (nLen < 0)
		{
			nLen = (int) strlen(szOut);
		}

		printf("%.*s", nLen, szOut);
		return S_OK;
	}

	HRESULT FlushStream()
	{
		return S_OK;
	}

}; // class CWriteStreamOnStdout

#endif
