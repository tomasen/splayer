//
// CodeTypeBuilder.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "CodeTypes.h"
#include "WSDLDocument.h"

//
// ElementTraits class for CElement*, CComplexType*, CSimpleType*
//
template <typename T>
class CXSDElementPtrTraits :  public CElementTraitsBase<T>
{
public:
    typedef T* INARGTYPE;
    typedef T*& OUTARGTYPE;

    static ULONG Hash( INARGTYPE element )
    {
        ATLASSERT( element != NULL );

        ULONG nHash = 0;
        CXMLDocument *pDoc = element->GetParentDocument();
        if (pDoc != NULL)
        {
            nHash = XSDHash((LPCWSTR) pDoc->GetDocumentUri(), nHash);
        }

        CSchema *pSchema = element->GetParentSchema();
        if (pSchema != NULL)
        {
            nHash = XSDHash((LPCWSTR) pSchema->GetTargetNamespace(), nHash);
        }

        nHash = XSDHash((LPCWSTR) element->GetName(), nHash);

        return nHash;
    }

    static bool CompareElements( INARGTYPE element1, INARGTYPE element2 )
    {
        ATLASSERT( element1 != NULL );
        ATLASSERT( element2 != NULL );

        CXMLDocument *pDoc1 = element1->GetParentDocument();
        CXMLDocument *pDoc2 = element2->GetParentDocument();

        if (pDoc1 != NULL && pDoc2 != NULL)
        {
            CSchema *pSchema1 = element1->GetParentSchema();
            CSchema *pSchema2 = element2->GetParentSchema();

            if (pSchema1 != NULL && pSchema2 != NULL)
            {
                return ( pDoc1->GetDocumentUri()==pDoc2->GetDocumentUri() &&
                         pSchema1->GetTargetNamespace()==pSchema2->GetTargetNamespace() &&
                         element1->GetName()==element2->GetName());
            }
        }

        return false;
    }

    static int CompareElementsOrdered( INARGTYPE element1, INARGTYPE element2 )
    {
        ATLASSERT( element1 != NULL );
        ATLASSERT( element2 != NULL );

        CXMLDocument *pDoc1 = element1->GetParentDocument();
        CXMLDocument *pDoc2 = element2->GetParentDocument();

        int nRet = 1;

        if (pDoc1 != NULL && pDoc2 != NULL)
        {
            nRet = pDoc1->GetDocumentUri().Compare( pDoc2->GetDocumentUri() );
            if (nRet == 0)
            {
                CSchema *pSchema1 = element1->GetParentSchema();
                CSchema *pSchema2 = element2->GetParentSchema();

                if (pSchema1 != NULL && pSchema2 != NULL)
                {
                    nRet = pSchema1->GetTargetNamespace().Compare( pSchema2->GetTargetNamespace() );
                    if (nRet == 0)
                    {
                        nRet = element1->GetName().Compare( element2->GetName() );
                    }
                }
            }
        }

        return nRet;
    }
};

class CCodeTypeBuilder
{
private:

    CWSDLDocument * m_pDoc;
    CCodeProxy * m_pProxy;

    //
    // collections for the various code elements
    //

    typedef CAtlPtrList<CCodeFunction*> CODEFUNCTIONLIST;

    typedef CAtlPtrMap<CSimpleType*, CCodeEnum*, CXSDElementPtrTraits<CSimpleType> > CODEENUMMAP;
    typedef CAtlPtrMap<CComplexType*, CCodeStruct*, CXSDElementPtrTraits<CComplexType> > CODESTRUCTMAP;

    typedef CAtlMap<CComplexType*, CCodeTypedElement, CXSDElementPtrTraits<CComplexType> > CODETYPEMAP;
    typedef CAtlMap<CSimpleType*, CCodeTypedElement, CXSDElementPtrTraits<CSimpleType> > CODESIMPLETYPEMAP;

    // REVIEW: headers mapped by message (unique, I would think)
    typedef CAtlMap<CWSDLMessagePart*, CCodeTypedElement *> CODEHEADERPARTMAP;
    typedef CAtlMap<CXSDElement*, CCodeTypedElement *> CODEHEADERTYPEMAP;

    typedef CAtlMap<CComplexType*, int, CXSDElementPtrTraits<CComplexType> > PARSEMAP;

    // map for C++ struct, enum, function, parameter, enum entry, struct field names
    typedef CAtlMap<CStringA, int, CStringRefElementTraits<CStringA> > NAMEMAP;

    CODEFUNCTIONLIST m_functions;
    CODESTRUCTMAP m_structs;
    CODEENUMMAP m_enums;

    CODETYPEMAP m_codeTypes;
    CODESIMPLETYPEMAP m_codeEnums;

    CODEHEADERPARTMAP m_headersByPart;
    CODEHEADERTYPEMAP m_headersByType;

    // map of what we are currently parsing
    PARSEMAP m_currParse;

    // map of named elements we've encountered at the global scope
    NAMEMAP m_globalNameMap;

    // global counter for duplicate names
    int m_nNameCounter;

public:

    CCodeTypeBuilder(CWSDLDocument *pDoc = NULL, CCodeProxy * pProxy = NULL)
        :m_pDoc(pDoc), m_pProxy(pProxy), m_nNameCounter(0)
    {
    }

    inline HRESULT Initialize(CWSDLDocument *pDoc, CCodeProxy * pProxy = NULL)
    {
        if (!pDoc)
        {
            return E_INVALIDARG;
        }

        if (pProxy)
        {
            m_pProxy = pProxy;
        }

        m_pDoc = pDoc;

        return S_OK;
    }

    HRESULT Build(CCodeProxy * pCodeProxy = NULL, CWSDLDocument *pDoc = NULL);

private:

    HRESULT ProcessService(CWSDLService *pSvc);

    HRESULT ProcessPort(CWSDLPort *pPort);

    HRESULT ProcessBinding(CWSDLBinding *pBinding);

    HRESULT ProcessPortType(CWSDLPortType *pPortType, CWSDLBinding *pBinding);

    HRESULT ProcessMessage(
        CWSDLPortTypeIO *pIO,
        CWSDLPortTypeOperation *pBindingOp,
        CWSDLBinding *pBinding,
        CWSDLMessage *pMessage,
        CCodeFunction *pCodeFunc,
        DWORD dwFlags);

    // TODO (jasjitg): must respect parts= with all these
    HRESULT ProcessMessage_PID(
        CWSDLMessage *pMsg,
        CCodeFunction *pCodeFunc,
        DWORD dwFlags,
        DWORD dwCallFlags);

    HRESULT ProcessMessagePart_PID(
        CWSDLMessagePart *pPart,
        CCodeFunction *pCodeFunc,
        DWORD dwFlags,
        DWORD dwCallFlags);

    HRESULT ProcessMessage_PAD(
        CWSDLMessage *pMsg,
        CCodeFunction *pCodeFunc,
        DWORD dwFlags,
        DWORD dwCallFlags);

    HRESULT ProcessMessagePart_PAD(
        CWSDLMessagePart *pPart,
        CCodeFunction *pCodeFunc,
        DWORD dwFlags,
        DWORD dwCallFlags);

    HRESULT ProcessMessage_RPC_Encoded(
        CWSDLMessage *pMsg,
        CCodeFunction *pCodeFunc,
        DWORD dwFlags,
        DWORD dwCallFlags);

    HRESULT ProcessMessagePart_RPC_Encoded(
        CWSDLMessagePart *pPart,
        CCodeFunction *pCodeFunc,
        DWORD dwFlags,
        DWORD dwCallFlags);

    // REVIEW (jasjitg): not going to support these in this version
//  HRESULT ProcessMessage_RPC_Literal(
//      CWSDLMessage *pMsg,
//      CCodeFunction *pCodeFunc,
//      DWORD dwFlags,
//      DWORD dwCallFlags);
//
//  HRESULT ProcessMessagePart_RPC_Literal(
//      CWSDLMessagePart *pPart,
//      CCodeFunction *pCodeFunc,
//      DWORD dwFlags,
//      DWORD dwCallFlags);

    HRESULT ProcessElement(
        CElement *pElem,
        CCodeElementContainer *pContainer,
        DWORD dwFlags,
        CODETYPE parentCodeType,
        BOOL fTopLevel = FALSE,
        CWSDLMessagePart *pMsgPart = NULL);

    HRESULT ProcessComplexType(
        CComplexType *pType,
        CCodeElementContainer *pContainer,
        DWORD dwFlags);

    HRESULT ProcessSimpleType(
        CSimpleType *pType,
        XSDTYPE *pXSDType,
        LPDWORD pdwFlags);

    HRESULT SortStructs();

    HRESULT SortStructHelper(POSITION pos);

    HRESULT CreateSafeNames(CCodeElementContainer *pElem);
    HRESULT CheckGlobalNameMap(CStringA& strName, bool bAddToMap = false);
    HRESULT CheckNameMap(NAMEMAP &map, CStringA& strName, bool bAddToMap = false);

    HRESULT GetTypeFromElement(
        CElement *pElem,
        CCodeTypedElement *pCodeElem,
        CCodeElementContainer *pContainer,
        DWORD dwFlags);

    HRESULT ProcessSchemaElement(
        CXSDElement *pElem,
        CCodeTypedElement *pCodeElem,
        CCodeElementContainer *pContainer,
        DWORD dwFlags);

    HRESULT ProcessXSDElement(
        CXMLElement *pElem,
        CQName& typeName,
        CCodeTypedElement *pCodeElem);

    HRESULT ProcessMessagePart_Type(
        CWSDLMessagePart *pPart,
        CXSDElement *pXSDElement,
        XSDTYPE xsdType,
        CODETYPE codeType,
        const CStringW& strName,
        CCodeFunction *pCodeFunc,
        DWORD dwFlags,
        DWORD dwCallFlags);

    // Is it a PAD, PID, RPC, etc.
    HRESULT GetCallFlags(
        LPCWSTR wszParts,
        CWSDLMessage *pMessage,
        CWSDLPortTypeIO *pIO,
        CWSDLPortTypeOperation *pBindingOp,
        CWSDLBinding *pBinding,
        LPDWORD pdwFlags);

    HRESULT GetCallFlags(
        CWSDLMessage *pMessage,
        CWSDLMessagePart *pPart,
        CWSDLPortTypeIO *pIO,
        CWSDLPortTypeOperation *pBindingOp,
        CWSDLBinding *pBinding,
        LPDWORD pdwFlags);

    HRESULT CheckDocLiteralNamespace(
        CCodeFunction *pCodeFunc,
        CXSDElement *pXSDElement,
        DWORD dwFlags,
        DWORD dwCallFlags);

    CODEFLAGS IsArrayDefinition(CComplexType *pType);
    BOOL IsVarArrayDefinition(CComplexType *pType,DWORD dwFlags, CElement **ppElement);

    HRESULT ProcessArrayDefintion(
        CElement *pElem,
        CCodeElementContainer *pContainer,
        CCodeTypedElement *pCodeElem,
        DWORD dwFlags);

    HRESULT ProcessArray(
        CComplexType *pType,
        CCodeElementContainer *pContainer,
        CCodeTypedElement *pCodeElem,
        DWORD dwFlags);

    HRESULT ProcessVarArray(
        CElement *pElement,
        CCodeElementContainer *pContainer,
        CCodeTypedElement *pCodeElem,
        DWORD dwFlags);

    HRESULT GetTypeFromQName(
        CQName& type,
        CXSDElement *pXSDElement,
        CXSDElement **ppXSDElement,
        XSDTYPE *pXSD);

    HRESULT GetArrayDimensions(
        CAttribute *pAttribute,
        CCodeTypedElement *pCodeElem);

    HRESULT ProcessSoapHeaders(
        CCodeFunction *pElem,
        CWSDLPortTypeIO *pIO,
        CWSDLPortTypeOperation *pBindingOp,
        CWSDLBinding *pBinding,
        DWORD dwFlags);

    HRESULT GetElementInfo(CXMLElement *pElem, CQName& name, CStringW& strUri);

    HRESULT CheckDuplicateHeaders(CCodeFunction *pCodeFunc, CCodeTypedElement *pElem, DWORD dwFlags);

    HRESULT AddHeaderToFunction(CCodeFunction *pCodeFunc, CAutoPtr<CCodeTypedElement>& spElem, CWSDLMessagePart *pPart);
    CCodeTypedElement * GetParameterByName(CCodeFunction *pCodeFunc, const CStringW& strName);
    HRESULT GetNameFromSchemaElement(CXSDElement *pXSDElement, CStringW& strName);
    CCodeTypedElement * GetHeaderByName(CCodeFunction *pCodeFunc, const CStringW& strName);
    HRESULT CheckAndAddHeader(
        CCodeFunction *pCodeFunc,
        CAutoPtr<CCodeTypedElement>& spElem,
        DWORD dwFlags,
        CWSDLMessagePart *pPart);
};

inline ULONG XSDHash(LPCWSTR wsz, ULONG nHash)
{
    while (*wsz != 0)
    {
        nHash=(nHash<<5)+nHash+(*wsz);
        wsz++;
    }
    return nHash;
}
