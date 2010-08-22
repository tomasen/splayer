//
// CppCodeGenerator.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "CppCodeGenerator.h"
#include "Emit.h"
#include "resource.h"
#include "Element.h"

HRESULT CCppCodeGenerator::Generate(LPCWSTR wszFile, CCodeProxy *pProxy, bool bPragma, bool bNoClobber, bool bEmitNamespace, bool bGenProxy, const char *szNamespace)
{
    ATLASSERT( (wszFile != NULL) && (*wszFile != L'\0') );

    CWriteStreamOnFileA fileStream;
    HRESULT hr = fileStream.Init(wszFile, (bNoClobber != false) ? CREATE_NEW : CREATE_ALWAYS);
    if (FAILED(hr))
    {
        EmitErrorHr(hr);
        return hr;
    }

    CStencil s;
    HMODULE hModule = _AtlBaseModule.GetResourceInstance();
    if (hModule == NULL)
    {
        EmitError(IDS_SDL_INTERNAL);
        return E_FAIL;
    }
    HTTP_CODE hcErr = s.LoadFromResource(hModule, IDR_SPROXYSRF, "SRF");
    if (hcErr != HTTP_SUCCESS)
    {
        EmitError(IDS_SDL_INTERNAL);
        return E_FAIL;
    }

    if (s.ParseReplacements(this) == false)
    {
        EmitError(IDS_SDL_INTERNAL);
        return E_FAIL;
    }

    s.FinishParseReplacements();
    if (s.ParseSuccessful() == false)
    {
#ifdef _DEBUG

        CWriteStreamOnStdout errStream;
        s.RenderErrors(&errStream);

#endif
        EmitError(IDS_SDL_INTERNAL);
        return E_FAIL;
    }
    m_pProxy = pProxy;
    m_writeHelper.Attach(&fileStream);

    m_bPragma = bPragma;
    m_bGenProxy = bGenProxy;
    m_szNamespace = szNamespace;
    if (m_szNamespace && !*m_szNamespace)
    {
        m_bEmitNamespace = false;
    }
    else
    {
        m_bEmitNamespace = bEmitNamespace;
    }

    hcErr = s.Render(this, &fileStream);
    if (hcErr != HTTP_SUCCESS)
    {
        EmitError(IDS_SDL_INTERNAL);
        return E_FAIL;
    }

    return S_OK;
}

HTTP_CODE CCppCodeGenerator::OnGetNextEnum()
{
    if (m_currEnumPos == NULL)
    {
        m_nCntr = 0;
        m_currEnumPos = m_pProxy->GetFirstEnum();
    }
    else
    {
        m_pProxy->GetNextEnum(m_currEnumPos);
    }

    if (m_currEnumPos != NULL)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetEnumSafeQName()
{
    return WriteCString(GetCurrentEnum()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetNextEnumElement()
{
    CCodeEnum *p = GetCurrentEnum();

    if (m_currEnumFieldPos == NULL)
    {
        m_currEnumFieldPos = p->GetFirstElement();
    }
    else
    {
        p->GetNextElement(m_currEnumFieldPos);
    }

    if (m_currEnumFieldPos != NULL)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetEnumElementHashW()
{
    return GetHashW(GetCurrentEnumElement()->GetCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetEnumElementName()
{
    return WriteCString(GetCurrentEnumElement()->GetCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetEnumElementValue()
{
    m_writeHelper << m_nCntr++;
    return HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnResetCounter()
{
    m_nCntr = 0;
    return HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnGetEnumNameHashW()
{
    return GetHashW(GetCurrentEnum()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetEnumName()
{
    return WriteCString(GetCurrentEnum()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetEnumQName()
{
    return WriteCString(GetCurrentEnum()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetNextStruct()
{
    if (m_currStructPos == NULL)
    {
        m_currStructPos = m_pProxy->GetFirstStruct();
    }
    else
    {
        m_pProxy->GetNextStruct(m_currStructPos);
    }

    if (m_currStructPos != NULL)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetStructSafeQName()
{
    return WriteCString(GetCurrentStruct()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetNextStructField()
{
    CCodeStruct *p = GetCurrentStruct();

    if (m_currStructFieldPos == NULL)
    {
        m_currStructFieldPos = p->GetFirstElement();
    }
    else
    {
        p->GetNextElement(m_currStructFieldPos);
    }

    if (m_currStructFieldPos != NULL)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldType()
{
    return GetCppType(GetCurrentStructField());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldSuffix()
{
    return GetTypeSuffix(GetCurrentStructField());
}

HTTP_CODE CCppCodeGenerator::OnIsFieldNullable()
{
    if ((GetCurrentStructField()->GetFlags() & CODEFLAG_NULLABLE) ||
        (GetCurrentStructField()->GetXSDType() == XSDTYPE_STRING) ||
        (GetCurrentStructField()->GetXSDType() == XSDTYPE_BASE64BINARY) ||
        (GetCurrentStructField()->GetXSDType() == XSDTYPE_HEXBINARY) ||
        (OnIsFieldDynamicArray() == HTTP_SUCCESS))
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsFieldDynamicArray()
{
    if (GetCurrentStructField()->GetFlags() & CODEFLAG_DYNARRAY)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsFieldDynamicArrayWrapper()
{
    if (GetCurrentStructField()->GetFlags() & CODEFLAG_DYNARRAYWRAPPER)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnFieldHasSizeIs()
{
    if (GetCurrentStructField()->GetSizeIs().GetLength() != 0)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetFieldSizeIsName()
{
    return WriteCString(GetCurrentStructField()->GetSizeIs());
}

HTTP_CODE CCppCodeGenerator::OnGetFieldSizeIsIndex()
{
    CCodeStruct *p = GetCurrentStruct();
    int nCntr = -1;

    POSITION pos = p->GetFirstElement();
    while (pos != NULL)
    {
        nCntr++;
        CCodeTypedElement *pElem = p->GetNextElement(pos);
        ATLASSERT( pElem != NULL );
        if (pElem->GetName() == ((LPCSTR)GetCurrentStructField()->GetSizeIs()))
        {
            m_writeHelper << nCntr;
            return HTTP_SUCCESS;
        }
    }

    return HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnIsFieldSizeIs()
{
    CCodeStruct *p = GetCurrentStruct();

    POSITION pos = p->GetFirstElement();
    while (pos != NULL)
    {
        CCodeTypedElement *pElem = p->GetNextElement(pos);
        ATLASSERT( pElem != NULL );
        if (GetCurrentStructField()->GetName() == ((LPCSTR)pElem->GetSizeIs()))
        {
            return HTTP_SUCCESS;
        }
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetCurrentFieldIndex()
{
    CCodeStruct *p = GetCurrentStruct();
    int nCntr = 0;
    POSITION pos = p->GetFirstElement();
    while ((pos != NULL) && (pos != m_currStructFieldPos))
    {
        nCntr++;
        CCodeTypedElement *pElem = p->GetNextElement(pos);
        if ((pElem->GetFlags() & CODEFLAG_DYNARRAY) && (pElem->GetSizeIs().GetLength()==0))
        {
            nCntr++;
        }
    }
    m_writeHelper << nCntr;
    return HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnIsFieldFixedArray()
{
    return IsFixedArray(GetCurrentStructField());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldName()
{
    return WriteCString(GetCurrentStructField()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldDimsDecl()
{
    return GetDimsDecl(GetCurrentStructField());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldHashW()
{
    return GetHashW(GetCurrentStructField()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldAtlSoapType()
{
    return GetAtlSoapType(GetCurrentStructField());
}

HTTP_CODE CCppCodeGenerator::OnGetStructQName()
{
    return WriteCString(GetCurrentStruct()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnIsFieldUDT()
{
    return IsUDT(GetCurrentStructField());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldTypeSafeQName()
{
    return WriteCString(GetCurrentStructField()->GetCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructNameHashW()
{
    return GetHashW(GetCurrentStruct()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructName()
{
    return WriteCString(GetCurrentStruct()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldCount()
{
    return (m_writeHelper.Write(GetCurrentStruct()->GetElementCount()) != FALSE) ? HTTP_SUCCESS : HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnGetNextFunction()
{
    if (m_currFunctionPos == NULL)
    {
        m_currFunctionPos = m_pProxy->GetFirstFunction();
        m_nFuncIndex = 0;
    }
    else
    {
        m_pProxy->GetNextFunction(m_currFunctionPos);
        m_nFuncIndex++;
    }

    if (m_currFunctionPos != NULL)
    {
        return HTTP_SUCCESS;
    }
    m_nFuncIndex = 0;
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetNextParameter()
{
    CCodeFunction *p = GetCurrentFunction();

    if (m_currParameterPos == NULL)
    {
        m_nCntr = 0;
        m_currParameterPos = p->GetFirstElement();
    }
    else
    {
        m_nCntr++;
        p->GetNextElement(m_currParameterPos);
    }

    if (m_currParameterPos != NULL)
    {
        return HTTP_SUCCESS;
    }
    m_nCntr = 0;
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsParameterFixedArray()
{
    return IsFixedArray(GetCurrentParameter());
}

HTTP_CODE CCppCodeGenerator::OnGetClassSafeQName()
{
    return WriteCString(m_pProxy->GetClassName());
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionName()
{
    return WriteCString(GetCurrentFunction()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetParameterName()
{
    CCodeTypedElement *p = GetCurrentParameter();
    if (p->GetName() == "return")
    {
        return (m_pStream->WriteStream("__retval", sizeof("__retval")-1, NULL) == S_OK) ?
            HTTP_SUCCESS : HTTP_FAIL;
    }

    return WriteCString(GetCurrentParameter()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetParameterNameRaw()
{
    return WriteCString(GetCurrentParameter()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetParameterDimsDecl()
{
    return GetDimsDecl(GetCurrentParameter());
}

HTTP_CODE CCppCodeGenerator::OnGetParameterType()
{
    return GetCppType(GetCurrentParameter());
}

HTTP_CODE CCppCodeGenerator::OnIsParameterDynamicArray()
{
    if (GetCurrentParameter()->GetFlags() & CODEFLAG_DYNARRAY)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetParameterSuffix()
{
    return GetTypeSuffix(GetCurrentParameter());
}

HTTP_CODE CCppCodeGenerator::OnIsInParameter()
{
    if (GetCurrentParameter()->GetFlags() & CODEFLAG_IN)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnHasRetval()
{
    // meaningless for client side stuff
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetRetval()
{
    // should never get called
    ATLASSERT( FALSE );
    return HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnGetParameterHashW()
{
    return GetHashW(GetCurrentParameter()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetParameterAtlSoapType()
{
    return GetAtlSoapType(GetCurrentParameter());
}

HTTP_CODE CCppCodeGenerator::OnIsParameterNullable()
{
    if ((GetCurrentParameter()->GetFlags() & CODEFLAG_NULLABLE) ||
        (GetCurrentParameter()->GetXSDType() == XSDTYPE_STRING) ||
        (GetCurrentParameter()->GetXSDType() == XSDTYPE_BASE64BINARY) ||
        (GetCurrentParameter()->GetXSDType() == XSDTYPE_HEXBINARY) ||
        (GetCurrentParameter()->GetFlags() & CODEFLAG_DYNARRAY))
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsOutParameter()
{
    if (GetCurrentParameter()->GetFlags() & CODEFLAG_OUT)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsParameterUDT()
{
    return IsUDT(GetCurrentParameter());
}

/*
HTTP_CODE CCppCodeGenerator::OnNotIsParameterUDT()
{
    return (OnIsParameterUDT() == HTTP_SUCCESS) ? HTTP_S_FALSE : HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnIsRpcLiteralWithElement()
{
    if ((GetCurrentParameter()->GetFlags() & (CODEFLAG_LITERAL | CODEFLAG_RPC | CODEFLAG_ELEMENT)) ==
        (CODEFLAG_LITERAL | CODEFLAG_RPC | CODEFLAG_ELEMENT))
    {
        return HTTP_SUCCESS;
    }

    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnNotIsParameterUDT_AND_IsRpcLiteralWithElement()
{
    return ((OnNotIsParameterUDT() == HTTP_SUCCESS) &&
            (OnIsRpcLiteralWithElement() == HTTP_SUCCESS))
            ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetParameterElementHashW()
{
    CXSDElement *pXSDElement = GetCurrentParameter()->GetElement();
    if (pXSDElement->GetElementType() == XSD_ELEMENT)
    {
        CElement *pElem = static_cast<CElement *>(pXSDElement);
        return GetHashW(pElem->GetName());
    }
    EmitError(IDS_SDL_INTERNAL);
    return HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnGetParameterElementName()
{
    CXSDElement *pXSDElement = GetCurrentParameter()->GetElement();
    if (pXSDElement->GetElementType() == XSD_ELEMENT)
    {
        CElement *pElem = static_cast<CElement *>(pXSDElement);
        return WriteCString(pElem->GetName());
    }
    EmitError(IDS_SDL_INTERNAL);
    return HTTP_FAIL;
}
*/

HTTP_CODE CCppCodeGenerator::OnGetParameterTypeQName()
{
    return WriteCString(GetCurrentParameter()->GetCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetSizeIsIndex()
{
    // should never get called
    ATLASSERT( FALSE );
    return HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnGetCurrentParameterIndex()
{
    if (m_writeHelper.Write(m_nCntr++) != FALSE)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnResetParameterIndex()
{
    // nothing to do here on the client side
    return HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnNotIsRetval()
{
    // always true on the client side
    return HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionNameHashW()
{
    return GetHashW(GetCurrentFunction()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionResultNameHashW()
{
    return GetHashW(GetCurrentFunction()->GetResponseName());
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionResultName()
{
    return WriteCString(GetCurrentFunction()->GetResponseName());
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionSendName()
{
    return WriteCString(GetCurrentFunction()->GetSendName());
}

HTTP_CODE CCppCodeGenerator::OnGetExpectedParameterCount()
{
    int nCnt = 0;
    while (OnGetNextParameter() == HTTP_SUCCESS)
    {
        if (OnIsOutParameter() == HTTP_SUCCESS)
        {
            nCnt++;
        }
    }

    if (m_writeHelper.Write(nCnt) != FALSE)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnIsPAD()
{
    return (GetCurrentFunction()->GetCallFlags() & CODEFLAG_PAD) ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsChain()
{
    return (GetCurrentFunction()->GetCallFlags() & CODEFLAG_CHAIN) ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsPID()
{
    return (GetCurrentFunction()->GetCallFlags() & CODEFLAG_PID) ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsDocument()
{
    return (GetCurrentFunction()->GetCallFlags() & CODEFLAG_DOCUMENT) ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsRpc()
{
    return (GetCurrentFunction()->GetCallFlags() & CODEFLAG_RPC) ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsOneWay()
{
    return (GetCurrentFunction()->GetCallFlags() & CODEFLAG_ONEWAY) ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsLiteral()
{
    return (GetCurrentFunction()->GetCallFlags() & CODEFLAG_LITERAL) ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsEncoded()
{
    return (GetCurrentFunction()->GetCallFlags() & CODEFLAG_ENCODED) ? HTTP_SUCCESS : HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetClassName()
{
    return WriteCString(m_pProxy->GetClassName());
}

HTTP_CODE CCppCodeGenerator::OnNotIsParameterFixedArray()
{
    return (IsFixedArray(GetCurrentParameter()) == HTTP_SUCCESS) ? HTTP_S_FALSE : HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnNotIsLastParameter()
{
    CCodeFunction *p = GetCurrentFunction();
    POSITION pos = m_currParameterPos;
    p->GetNextElement(pos);
    if (pos == NULL)
    {
        return HTTP_S_FALSE;
    }
    return HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnGetParameterFixedArraySize()
{
    CCodeTypedElement *p = GetCurrentParameter();
    if (p->GetDims() != 0)
    {
        int i = 1;
        for (int j=1; j<=p->GetDimension(0); j++)
        {
            i*= p->GetDimension(j);
        }

        if (m_writeHelper.Write(i) != FALSE)
        {
            return HTTP_SUCCESS;
        }
    }
    return HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnGetDateTime()
{
    SYSTEMTIME systime;
    SYSTEMTIME loctime;
    TIME_ZONE_INFORMATION tz;
    memset(&systime, 0x00, sizeof(systime));
    memset(&loctime, 0x00, sizeof(loctime));
    memset(&tz, 0x00, sizeof(tz));
    GetSystemTime(&systime);
    GetTimeZoneInformation(&tz);
    SystemTimeToTzSpecificLocalTime(&tz, &systime, &loctime);

    char szDate[256];
    int n = sprintf(szDate, "%.02d/%.02d/%d@%.02d:%.02d:%d",
        loctime.wMonth, loctime.wDay, loctime.wYear, loctime.wHour,
        loctime.wMinute, loctime.wSecond);

    return (m_pStream->WriteStream(szDate, n, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_FAIL;
}

HTTP_CODE CCppCodeGenerator::OnGetURL()
{
    return WriteCString(m_pProxy->GetAddressUri());
}

HTTP_CODE CCppCodeGenerator::OnGetSoapAction()
{
    CStringA m_strSoapAction = GetCurrentFunction()->GetSoapAction();
    return WriteCString(m_strSoapAction);
}

HTTP_CODE CCppCodeGenerator::OnGetNamespace()
{
    return WriteCString(m_pProxy->GetTargetNamespace());
}

HTTP_CODE CCppCodeGenerator::OnEmitNamespace()
{
    if (m_bEmitNamespace != false)
    {
        return HTTP_SUCCESS;
    }

    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetCppNamespace()
{
    ATLASSERT( m_bEmitNamespace != false );

    if ((m_szNamespace != NULL) && (*m_szNamespace))
    {
        return WriteCString(CStringA(m_szNamespace));
        //return (m_writeHelper.Write(m_szNamespace) == TRUE) ? HTTP_SUCCESS : HTTP_FAIL;
    }

    return WriteCString(m_pProxy->GetServiceName());
}

HTTP_CODE CCppCodeGenerator::OnClassHasHeaders()
{
    POSITION pos = m_pProxy->GetFirstHeader();
    if (pos != NULL)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetNextMember()
{
    if (m_currMemberPos == NULL)
    {
        m_currMemberPos = m_pProxy->GetFirstHeader();
    }
    else
    {
        m_pProxy->GetNextHeader(m_currMemberPos);
    }

    if (m_currMemberPos != NULL)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetMemberType()
{
    return GetCppType(GetCurrentMember());
}

HTTP_CODE CCppCodeGenerator::OnGetMemberName()
{
    return WriteCString(GetCurrentMember()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetMemberSuffix()
{
    return GetTypeSuffix(GetCurrentMember());
}

HTTP_CODE CCppCodeGenerator::OnIsMemberFixedArray()
{
    if (GetCurrentMember()->GetDims() != 0)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsMemberUDT()
{
    if (GetCurrentMember()->GetCodeType() == CODETYPE_STRUCT)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsMemberEnum()
{
    if (GetCurrentMember()->GetCodeType() == CODETYPE_ENUM)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetNextHeader()
{
    CCodeFunction *p = GetCurrentFunction();

    if (m_currHeaderPos == NULL)
    {
        m_currHeaderPos = p->GetFirstHeader();
    }
    else
    {
        p->GetNextHeader(m_currHeaderPos);
    }

    if (m_currHeaderPos != NULL)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsHeaderFixedArray()
{
    return IsFixedArray(GetCurrentHeader());
}

HTTP_CODE CCppCodeGenerator::OnGetHeaderValue()
{
    return WriteCString(GetCurrentHeader()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetHeaderDimsDecl()
{
    return GetDimsDecl(GetCurrentHeader());
}

HTTP_CODE CCppCodeGenerator::OnGetHeaderHashW()
{
    return GetHashW(GetCurrentHeader()->GetName());
}

HTTP_CODE CCppCodeGenerator::OnGetHeaderAtlSoapType()
{
    return GetAtlSoapType(GetCurrentHeader());
}

HTTP_CODE CCppCodeGenerator::OnIsInHeader()
{
    if (GetCurrentHeader()->GetFlags() & CODEFLAG_IN)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsOutHeader()
{
    if (GetCurrentHeader()->GetFlags() & CODEFLAG_OUT)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsRequiredHeader()
{
    if (GetCurrentHeader()->GetFlags() & CODEFLAG_MUSTUNDERSTAND)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsHeaderNullable()
{
    if (GetCurrentHeader()->GetXSDType() == XSDTYPE_STRING)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnIsHeaderUDT()
{
    return IsUDT(GetCurrentHeader());
}

HTTP_CODE CCppCodeGenerator::OnGetHeaderTypeQName()
{
    return WriteCString(GetCurrentHeader()->GetCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetExpectedHeaderCount()
{
    CCodeFunction *p = GetCurrentFunction();
    POSITION pos = p->GetFirstHeader();
    int nCnt = 0;
    while (pos != NULL)
    {
        CCodeTypedElement *pElem = p->GetNextHeader(pos);
        if (pElem->GetFlags() & CODEFLAG_MUSTUNDERSTAND)
        {
            nCnt++;
        }
    }

    m_writeHelper << nCnt;

    return HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnHeaderHasNamespace()
{
    if (GetCurrentHeader()->GetNamespace().GetLength() != 0)
    {
        return HTTP_SUCCESS;
    }

    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetHeaderNamespace()
{
    return WriteCString(GetCurrentHeader()->GetNamespace());
}

HTTP_CODE CCppCodeGenerator::OnGetHeaderNamespaceHashW()
{
    return GetHashW(GetCurrentHeader()->GetNamespace());
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionIndex()
{
    m_writeHelper << m_nFuncIndex;
    return HTTP_SUCCESS;
}

HTTP_CODE CCppCodeGenerator::OnEmitPragma()
{
    if (m_bPragma != false)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetEnumNamespaceHashW()
{
    return GetHashW(GetCurrentEnum()->GetNamespace());
}

HTTP_CODE CCppCodeGenerator::OnEnumHasUniqueNamespace()
{
    if (GetCurrentEnum()->GetNamespace() != m_pProxy->GetTargetNamespace())
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetEnumNamespace()
{
    return WriteCString(GetCurrentEnum()->GetNamespace());
}

HTTP_CODE CCppCodeGenerator::OnGetStructNamespaceHashW()
{
    return GetHashW(GetCurrentStruct()->GetNamespace());
}

HTTP_CODE CCppCodeGenerator::OnStructHasUniqueNamespace()
{
    if (GetCurrentStruct()->GetNamespace() != m_pProxy->GetTargetNamespace())
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}

HTTP_CODE CCppCodeGenerator::OnGetStructNamespace()
{
    return WriteCString(GetCurrentStruct()->GetNamespace());
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionNamespaceHashW()
{
    return GetHashW(GetCurrentFunction()->GetNamespace());
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionNamespace()
{
    return WriteCString(GetCurrentFunction()->GetNamespace());
}

// safe-naming stuffs

HTTP_CODE CCppCodeGenerator::OnGetHeaderTypeCppQName()
{
    return WriteCString(GetCurrentHeader()->GetSafeCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetMemberCppType()
{
    return GetSafeCppType(GetCurrentMember());
}

HTTP_CODE CCppCodeGenerator::OnGetMemberCppName()
{
    return WriteCString(GetCurrentMember()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetParameterTypeCppQName()
{
    return WriteCString(GetCurrentParameter()->GetSafeCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetFunctionCppName()
{
    return WriteCString(GetCurrentFunction()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetParameterCppName()
{
    CCodeTypedElement *p = GetCurrentParameter();
    if (p->GetName() == "return")
    {
        return (m_pStream->WriteStream("__retval", sizeof("__retval")-1, NULL) == S_OK) ?
            HTTP_SUCCESS : HTTP_FAIL;
    }

    return WriteCString(GetCurrentParameter()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetParameterCppType()
{
    return GetSafeCppType(GetCurrentParameter());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldCppName()
{
    return WriteCString(GetCurrentStructField()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructCppQName()
{
    return WriteCString(GetCurrentStruct()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructCppName()
{
    return WriteCString(GetCurrentStruct()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructSafeCppQName()
{
    return WriteCString(GetCurrentStruct()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldCppType()
{
    return GetSafeCppType(GetCurrentStructField());
}

HTTP_CODE CCppCodeGenerator::OnGetEnumCppName()
{
    return WriteCString(GetCurrentEnum()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetEnumSafeCppQName()
{
    return WriteCString(GetCurrentEnum()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetEnumElementCppName()
{
    return WriteCString(GetCurrentEnumElement()->GetSafeCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetStructFieldTypeSafeCppQName()
{
    return WriteCString(GetCurrentStructField()->GetSafeCodeTypeName());
}

HTTP_CODE CCppCodeGenerator::OnGetHeaderCppValue()
{
    return WriteCString(GetCurrentHeader()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGetEnumCppQName()
{
    return WriteCString(GetCurrentEnum()->GetSafeName());
}

HTTP_CODE CCppCodeGenerator::OnGenProxy()
{
    if (m_bGenProxy != false)
    {
        return HTTP_SUCCESS;
    }
    return HTTP_S_FALSE;
}
