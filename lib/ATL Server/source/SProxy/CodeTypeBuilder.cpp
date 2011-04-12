//
// CodeTypeBuilder.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "CodeTypeBuilder.h"

// CXSDTypeLookup CCodeTypeBuilder::m_xsdLookup;
const CStringW cStrMinMaxOccursArray=L"MIN_MAX_OCCURS_ARRAY";

HRESULT CCodeTypeBuilder::Build(CCodeProxy * pCodeProxy, CWSDLDocument *pDoc)
{
    if (pDoc != NULL)
    {
        m_pDoc = pDoc;
    }

    if (pCodeProxy != NULL)
    {
        m_pProxy = pCodeProxy;
    }

    if (m_pDoc == NULL || m_pProxy == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    POSITION pos = m_pDoc->GetFirstService();
    while (pos != NULL)
    {
        CWSDLService *p = m_pDoc->GetNextService(pos);
        if (p != NULL)
        {
            hr = ProcessService(p);
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }

    // check if anything was generated
    if (m_pProxy->GetClassName().GetLength() == 0)
    {
        EmitError(IDS_SDL_NO_GENERATE);
        return E_FAIL;
    }

    //
    // sort structs by dependency
    //

    hr = SortStructs();

    POSITION oldpos = NULL;

    if (SUCCEEDED(hr))
    {
        //
        // Add functions to proxy
        //

        pos = m_functions.GetHeadPosition();
        oldpos = NULL;
        while (pos != NULL)
        {
            oldpos = pos;
            m_functions.GetNext(pos);
            if (m_pProxy->AddFunction(m_functions.GetAt(oldpos)))
            {
                m_functions.RemoveAt(oldpos);
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        //
        // Add enums to proxy
        //

        m_enums.DisableAutoRehash();
        pos = m_enums.GetStartPosition();
        oldpos = NULL;
        while (pos != NULL)
        {
            oldpos = pos;
            CODEENUMMAP::CPair *p = m_enums.GetNext(pos);
            if (m_pProxy->AddEnum(p->m_value) != NULL)
            {
                m_enums.RemoveAtPos(oldpos);
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
        m_enums.EnableAutoRehash();
    }

    if (SUCCEEDED(hr))
    {
        //
        // Add headers to proxy
        //

        m_headersByType.DisableAutoRehash();
        pos = m_headersByType.GetStartPosition();
        oldpos = NULL;
        while (pos != NULL)
        {
            oldpos = pos;
            CODEHEADERTYPEMAP::CPair *p = m_headersByType.GetNext(pos);
            if (m_pProxy->AddHeader(p->m_value) != NULL)
            {
                m_headersByType.RemoveAtPos(oldpos);
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
        m_headersByType.EnableAutoRehash();
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pProxy->SetTargetNamespace(m_pDoc->GetTargetNamespace());
    }

    return hr;
}

HRESULT CCodeTypeBuilder::ProcessService(CWSDLService *pSvc)
{
    HRESULT hr = S_FALSE;
    POSITION pos = pSvc->GetFirstPort();
    while (pos != NULL)
    {
        CWSDLPort *p = pSvc->GetNextPort(pos);
        if (m_pProxy->GetClassName().GetLength() != 0)
        {
            EmitFileWarning(IDS_SDL_ONE_PORT, p, 0);
            continue;
        }
        if (p != NULL)
        {
            hr = ProcessPort(p);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        if (hr == S_OK)
        {
            if (pSvc->GetName().GetLength() != 0)
            {
                CString strClassName;
                strClassName.Append("C", 1);
                strClassName+= pSvc->GetName();

                if (FAILED(m_pProxy->SetClassName(strClassName)))
                {
                    return E_FAIL;
                }
                if (FAILED(m_pProxy->SetServiceName(pSvc->GetName())))
                {
                    return E_FAIL;
                }
            }
            else
            {
                if (FAILED(m_pProxy->SetClassName("CAtlServerProxy")))
                {
                    return E_FAIL;
                }
                if (FAILED(m_pProxy->SetServiceName("AtlServerProxy")))
                {
                    return E_FAIL;
                }
            }
        }
    }

    return S_OK;
}

HRESULT CCodeTypeBuilder::ProcessPort(CWSDLPort *pPort)
{
    ATLASSERT( pPort != NULL );

    // TODO: handle other kinds of transport (SMTP)
    if (pPort->GetSoapAddress().GetLength() != 0)
    {
        if (SUCCEEDED(m_pProxy->SetAddressUri(pPort->GetSoapAddress())))
        {
            CWSDLBinding * pBinding = pPort->GetBinding();
            if (pBinding != NULL)
            {
                return ProcessBinding(pBinding);
            }
        }

        return E_FAIL;
    }

    EmitFileWarning(IDS_SDL_SOAP_PORT_ONLY, pPort, 0);

    return S_FALSE;
}

HRESULT CCodeTypeBuilder::ProcessBinding(CWSDLBinding *pBinding)
{
    //
    // REVIEW: right now only support the SOAP binding
    //

    CSoapBinding *pSoapBinding = pBinding->GetSoapBinding();

    if (pSoapBinding != NULL)
    {
        CWSDLPortType * pPortType = pBinding->GetPortType();
        if (pPortType != NULL)
        {
            return ProcessPortType(pPortType, pBinding);
        }

        return E_FAIL;
    }

    EmitFileWarning(IDS_SDL_SOAP_BINDING_ONLY, pBinding, 0);

    return S_OK;
}

HRESULT CCodeTypeBuilder::ProcessPortType(CWSDLPortType *pPortType, CWSDLBinding *pBinding)
{
    HRESULT hr = S_OK;
    POSITION pos = pPortType->GetFirstOperation();
    while (pos != NULL)
    {
        CWSDLPortTypeOperation * p = pPortType->GetNextOperation(pos);
        CWSDLPortTypeInput *pBindingInput = NULL;
        CWSDLPortTypeOutput *pBindingOutput = NULL;
        CStringW strSoapAction;
        CStringW strMethodNamespace;
        if (p != NULL)
        {
            hr = E_FAIL;
            CWSDLPortTypeOperation *pBindingOp = pBinding->GetOperation(p->GetName());
            if (pBindingOp != NULL)
            {
                hr = S_OK;
                pBindingInput = pBindingOp->GetInput();
                if (pBindingInput != NULL)
                {
                    hr = E_FAIL;
                    CSoapBody *pBody = pBindingInput->GetSoapBody();
                    if (pBody != NULL)
                    {
                        // TODO (jasjitg): support all uses and styles
                        if ((pBody->GetUse() == SOAPUSE_LITERAL) ||
                            ((pBody->GetUse() == SOAPUSE_ENCODED) &&
                             (pBody->GetEncodingStyle() == SOAP_ENCODINGSTYLEW)))
                        {
                            strMethodNamespace = pBody->GetNamespace();
                            hr = S_OK;
                            CSoapOperation *pSoapOperation = pBindingOp->GetSoapOperation();
                            if (pSoapOperation != NULL)
                            {
                                strSoapAction = pSoapOperation->GetSoapAction();
                            }
                        }
                    }
                }

                if (SUCCEEDED(hr))
                {
                    pBindingOutput = pBindingOp->GetOutput();
                    if (pBindingOutput != NULL)
                    {
                        hr = E_FAIL;
                        CSoapBody *pBody = pBindingOutput->GetSoapBody();
                        if (pBody != NULL)
                        {
                            // TODO (jasjitg): support all uses and styles
                            if ((pBody->GetUse() == SOAPUSE_LITERAL) ||
                                ((pBody->GetUse() == SOAPUSE_ENCODED) &&
                                (pBody->GetEncodingStyle() == SOAP_ENCODINGSTYLEW)))
                            {
                                if ((strMethodNamespace.GetLength() != 0) &&
                                    (strMethodNamespace != pBody->GetNamespace()))
                                {
                                    // ATL Server does not support input/output operations with different namespaces
                                    EmitFileError(IDS_SDL_IO_DIFF_NAMESPACES, pBody, 0);
                                    hr = E_FAIL;
                                }
                                else
                                {
                                    if (strSoapAction.GetLength() == 0)
                                    {
                                        CSoapOperation *pSoapOperation = pBindingOp->GetSoapOperation();
                                        if (pSoapOperation != NULL)
                                        {
                                            strSoapAction = pSoapOperation->GetSoapAction();
                                            hr = S_OK;
                                        }
                                    }
                                    else
                                    {
                                        hr = S_OK;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (strMethodNamespace.GetLength() == 0)
            {
                strMethodNamespace = m_pDoc->GetTargetNamespace();
            }

            CAutoPtr<CCodeFunction> spElem;
            if (SUCCEEDED(hr))
            {
                ATLASSERT( pBindingOp != NULL );
                spElem.Attach( new CCodeFunction );
                if (spElem != NULL)
                {
                    if (FAILED(spElem->SetNamespace(strMethodNamespace)))
                    {
                        return E_FAIL;
                    }

                    CWSDLPortTypeInput *pInput = p->GetInput();
                    if (pInput != NULL)
                    {
                        //
                        // process input
                        //

                        // TODO: WSDL1.1 only process parts specified by body (if parts attribute exists)
                        CWSDLMessage *pMsg = pInput->GetMessage();
                        if (pMsg != NULL)
                        {
                            hr = ProcessMessage(pBindingInput, pBindingOp, pBinding, pMsg, spElem, CODEFLAG_IN);
                        }

                        if (SUCCEEDED(hr))
                        {
                            hr = ProcessSoapHeaders(spElem, pBindingInput, pBindingOp, pBinding, CODEFLAG_IN);
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        CWSDLPortTypeOutput *pOutput = p->GetOutput();

                        if (pOutput != NULL)
                        {
                            //
                            // process output
                            //

                            // TODO: WSDL1.1 only process parts specified by body (if parts attribute exists)
                            CWSDLMessage *pMsg = pOutput->GetMessage();
                            if (pMsg != NULL)
                            {
                                hr = ProcessMessage(pBindingOutput, pBindingOp, pBinding, pMsg, spElem, CODEFLAG_OUT);
                            }

                            if (SUCCEEDED(hr))
                            {
                                hr = ProcessSoapHeaders(spElem, pBindingOutput, pBindingOp, pBinding, CODEFLAG_OUT);
                            }
                        }
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            //
            // Add spElem to function list
            //
            if (SUCCEEDED(hr) && spElem != NULL)
            {
                hr = spElem->SetName(p->GetName());
                if (pBindingOutput == NULL)
                {
                    // if the output is NULL, it is a one-way method
                    spElem->SetCallFlags(spElem->GetCallFlags() | CODEFLAG_ONEWAY);
                }
                if (SUCCEEDED(hr))
                {
                    if (SUCCEEDED(spElem->SetSoapAction(strSoapAction)))
                    {
                        // safe-naming
                        hr = CreateSafeNames(spElem);
                        if (SUCCEEDED(hr))
                        {
                            if (m_functions.AddTail(spElem) != NULL)
                            {
                                spElem.Detach();
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
            }

            if (FAILED(hr))
            {
                EmitErrorHr(hr);
                break;
            }
        }
    }

    return hr;
}

// TODO: respect the parts= attribute
HRESULT CCodeTypeBuilder::ProcessMessage(
    CWSDLPortTypeIO *pIO,
    CWSDLPortTypeOperation *pBindingOp,
    CWSDLBinding *pBinding,
    CWSDLMessage *pMessage,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags)
{
    ATLASSERT( pIO != NULL );
    ATLASSERT( pBindingOp != NULL );
    ATLASSERT( pBinding != NULL );
    ATLASSERT( pMessage != NULL );
    ATLASSERT( pCodeFunc != NULL );

    DWORD dwCallFlags = 0;
    HRESULT hr = GetCallFlags(NULL, pMessage, pIO, pBindingOp, pBinding, &dwCallFlags);
    if (SUCCEEDED(hr))
    {
        bool bInternalError = false;

        if ((pCodeFunc->GetCallFlags() & CODEFLAG_CHAIN) &&
            ((dwCallFlags & CODEFLAG_CHAIN)==0))
        {
            // REVIEW: sproxy.exe limitation -- may fix in future
            return E_FAIL;
        }

        pCodeFunc->SetCallFlags(dwCallFlags);

        if (dwCallFlags & CODEFLAG_PID)
        {
            hr = ProcessMessage_PID(pMessage, pCodeFunc, dwFlags, dwCallFlags);
        }
        else if (dwCallFlags & CODEFLAG_PAD)
        {
            hr = ProcessMessage_PAD(pMessage, pCodeFunc, dwFlags, dwCallFlags);
        }
        else if (dwCallFlags & CODEFLAG_RPC)
        {
            if (dwCallFlags & CODEFLAG_ENCODED)
            {
                hr = ProcessMessage_RPC_Encoded(pMessage, pCodeFunc, dwFlags, dwCallFlags);
            }
//          else if (dwCallFlags & CODEFLAG_LITERAL)
//          {
//              hr = ProcessMessage_RPC_Literal(pMessage, pCodeFunc, dwFlags, dwCallFlags);
//          }
            else
            {
                bInternalError = true;
            }

            if (SUCCEEDED(hr))
            {
                if (dwFlags & CODEFLAG_IN)
                {
                    hr = pCodeFunc->SetSendName(pBindingOp->GetName());
                }
                else if (dwFlags & CODEFLAG_OUT)
                {
                    hr = pCodeFunc->SetResponseName(pBindingOp->GetName());
                }
                else
                {
                    bInternalError = true;
                }
            }
        }
        else
        {
            bInternalError = true;
        }

        if (bInternalError != false)
        {
            ATLASSERT( FALSE );
            EmitError(IDS_SDL_INTERNAL);
            hr = E_FAIL;
        }
    }

    return hr;
}

//
// PID means ParametersInDocument
// structure of the referenced message and part is:
//
//   <message name="...">
//     <part name="parameters" element="s0:MyFunctionElement"/>
//   </message>
//
// where MyFunctionElement looks like:
//
//   <element name="MyFunctionElement">
//     <complexType>
//       <sequence> <!-- or "all" -->
//         <element name="..." type="qname"/>
//         <!-- ... etc. -->
//       </sequence>
//     </complexType>
//   </element>
//
// for the referenced message and part for input operation from the binding/port,
// each element of the complexType represents an in-parameter.
// for the referenced message and part for output operation from the binding/port,
// each element of the complexType represents an out-parameter.
// as a proxy-generating convention, two "parameters" that have the same in the
// input element and output element are considered the same parameter--that logic
// is in ProcessElement.  The wrapper element under <SOAP:Body> for the request,
// is the name of the referenced element for the input operation message part.
// The wrapper element under <SOAP:Body> for the response is the name of the
// referenced element for the output operation message part.
// See sections 2.3 and 3.5 in the WSDL 1.1 spec for more details.
//
HRESULT CCodeTypeBuilder::ProcessMessage_PID(
    CWSDLMessage *pMsg,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    ATLASSERT( pMsg != NULL );
    ATLASSERT( pCodeFunc != NULL );
    ATLASSERT( dwCallFlags & CODEFLAG_PID );

    // REVIEW (jasjitg): maybe ATLASSERT is too lenient here
    ATLASSERT( pMsg->GetNumParts() == 1 );

    dwCallFlags;

    POSITION pos = pMsg->GetFirstPart();
    if (pos != NULL)
    {
        CWSDLMessagePart *pPart = pMsg->GetNextPart(pos);
        if (pPart != NULL)
        {
            return ProcessMessagePart_PID(pPart, pCodeFunc, dwFlags, dwCallFlags);
        }
    }

    EmitError(IDS_SDL_INTERNAL);
    return E_FAIL;
}

HRESULT CCodeTypeBuilder::ProcessMessagePart_PID(
    CWSDLMessagePart *pPart,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    ATLASSERT( pPart != NULL );
    ATLASSERT( pCodeFunc != NULL );

    HRESULT hr = E_FAIL;
    CODETYPE codeType = (dwFlags & CODEFLAG_HEADER) ? CODETYPE_HEADER : CODETYPE_UNK;
    if (pPart->GetElementName().GetName().GetLength() != 0)
    {
        CElement *pElem;
        hr = pPart->GetElement(&pElem);
        if (SUCCEEDED(hr))
        {
            ATLASSERT( pElem != NULL );

            hr = CheckDocLiteralNamespace(pCodeFunc, pElem, dwFlags, dwCallFlags);
            if (hr == S_OK)
            {
                hr = ProcessElement(pElem, pCodeFunc, dwFlags | dwCallFlags, codeType, TRUE, pPart);
                if (SUCCEEDED(hr))
                {
                    // do not reset these if processing headers
                    if ((dwFlags & CODEFLAG_HEADER) == 0)
                    {
                        if (dwFlags & CODEFLAG_OUT)
                        {
                            hr = pCodeFunc->SetResponseName(pElem->GetName());
                        }
                        else if (dwFlags & CODEFLAG_IN)
                        {
                            hr = pCodeFunc->SetSendName(pElem->GetName());
                        }
                    }
                }
            }
        }
    }
    else
    {
        EmitError(IDS_SDL_INTERNAL);
    }

    return hr;
}

// TODO (jasjitg): must respect parts= with all these

//
// PAD means ParametersAsDocument
// each message part represents a parameter
// NOTE: if any referenced part references its schema component
// using the type= attribute it MUST be the only referenced message
// part, per the second example in section 2.3.1 in the WSDL 1.1 spec.
//
// structure of the referenced message and part is:
//
//   <message name="...">
//     <part name="x" element="s0:x"/>
//     <part name="y" element="s0:B"/>
//   </message>
//
// where schema looks like (showing element= example -- see 2.3.1 for type= example):
//
//   <element name="x" type="xsd:int"/>
//   <element name="B">
//     <complexType>
//       <sequence> <!-- or "all" -->
//         <element name="C" type="xsd:int"/>
//         <element name="D" type="xsd:string"/>
//         <!-- ... etc. -->
//       </sequence>
//     </complexType>
//   </element>
//
// the expected wire format is:
//
// <Envelope>
//   <Body>
//     <A>123</A>
//     <B><C>123</C><D>some string</D></B>
//   </Body>
// </Envelope>
//
// see 2.3, 2.3.1, and 3.5 in the WSDL 1.1 spec for more details.
//
HRESULT CCodeTypeBuilder::ProcessMessage_PAD(
    CWSDLMessage *pMsg,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    ATLASSERT( pMsg != NULL );
    ATLASSERT( pCodeFunc != NULL );
    ATLASSERT( dwCallFlags & CODEFLAG_PAD );

    bool bType = false;
    POSITION pos = pMsg->GetFirstPart();
    while (pos != NULL)
    {
        if (bType != false)
        {
            EmitFileError(IDS_SDL_PAD_TYPE, pMsg, 0);
            return E_FAIL;
        }

        CWSDLMessagePart *pCheck = pMsg->GetNextPart(pos);
        ATLASSERT( pCheck != NULL );
        if (pCheck->GetTypeName().GetName().GetLength() != 0)
        {
            bType = true;
        }
    }

    HRESULT hr = S_OK;
    if (dwFlags & CODEFLAG_IN)
    {
        // clear the namespace--it should be set from the schema
        pCodeFunc->GetNamespace().Empty();
    }

    if (bType != false)
    {
        // REVIEW (jasjitg): not implemented correctly in atlsoap.h!!!

        // REVIEW (jasjitg): comment/assert below...
        // must have CODEFLAG_CHAIN
        ATLASSERT( dwCallFlags & CODEFLAG_CHAIN );

        // referenced by type= ==> only one message part
        pos = pMsg->GetFirstPart();
        CWSDLMessagePart *pPart = pMsg->GetNextPart(pos);
        ATLASSERT( pPart != NULL );

        hr = ProcessMessagePart_PAD(pPart, pCodeFunc, dwFlags, dwCallFlags);
    }
    else // referenced by element=
    {
        pos = pMsg->GetFirstPart();
        while (pos != NULL)
        {
            CWSDLMessagePart *pPart = pMsg->GetNextPart(pos);
            ATLASSERT( pPart != NULL );

            hr = ProcessMessagePart_PAD(pPart, pCodeFunc, dwFlags, dwCallFlags);

            if (FAILED(hr))
            {
                break;
            }
        }
    }

    if (FAILED(hr))
    {
        EmitErrorHr(E_FAIL);
    }

    return hr;
}

HRESULT CCodeTypeBuilder::ProcessMessagePart_PAD(
    CWSDLMessagePart *pPart,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    ATLASSERT( pPart != NULL );
    ATLASSERT( pCodeFunc != NULL );

    HRESULT hr = E_FAIL;
    bool isHeader = (dwFlags & CODEFLAG_HEADER) != 0;
    CODETYPE codeType = isHeader ? CODETYPE_HEADER : CODETYPE_UNK;
    if (pPart->GetTypeName().GetName().GetLength() == 0)
    {
        CElement *pElement = NULL;
        hr = pPart->GetElement(&pElement);
        if (SUCCEEDED(hr))
        {
            ATLASSERT( pElement != NULL );

            // set/verify namespace
            hr = CheckDocLiteralNamespace(pCodeFunc, pElement, dwFlags, dwCallFlags);
            if (hr == S_OK)
            {
                hr = ProcessElement(pElement, pCodeFunc, dwFlags | dwCallFlags, codeType, TRUE, pPart);
            }
        }
    }
    else
    {
        CXSDElement *pXSDElement = NULL;
        XSDTYPE xsdType = XSDTYPE_ERR;

        if (SUCCEEDED(pPart->GetType(&pXSDElement, &xsdType)))
        {
            // must be complexType, or else wire format will not be valid by SOAP section 4.3
            if ((pXSDElement != NULL) &&
                (pXSDElement->GetElementType() == XSD_COMPLEXTYPE))
            {
                hr = CheckDocLiteralNamespace(pCodeFunc, pXSDElement, dwFlags, dwCallFlags);
                if (hr == S_OK)
                {
                    CStringW strName;
                    hr = GetNameFromSchemaElement(pXSDElement, strName);
                    if (SUCCEEDED(hr))
                    {
                        // REVIEW (jasjitg): issues here about in/out parameters, etc.
                        hr = ProcessMessagePart_Type(pPart, pXSDElement, XSDTYPE_ERR, codeType,
                            pPart->GetName(), pCodeFunc, dwFlags, dwCallFlags);
                    }
                    else
                    {
                        EmitError(IDS_SDL_INTERNAL);
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                EmitFileError(IDS_SDL_PAD_INVALID_SOAP, pPart, 0);
                hr = E_FAIL;
            }
        }
    }

    return hr;
}

// TODO (jasjitg): must respect parts= with all these
// TODO (jasjitg): must pass input/output so that send/response names can be set
HRESULT CCodeTypeBuilder::ProcessMessage_RPC_Encoded(
    CWSDLMessage *pMsg,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    ATLASSERT( pMsg != NULL );
    ATLASSERT( pCodeFunc != NULL );
    ATLASSERT( dwCallFlags & CODEFLAG_RPC );
    ATLASSERT( dwCallFlags & CODEFLAG_ENCODED );

    pMsg;
    pCodeFunc;
    dwFlags;
    dwCallFlags;

    POSITION pos = pMsg->GetFirstPart();
    HRESULT hr = S_OK;
    while (pos != NULL)
    {
        CWSDLMessagePart *pPart = pMsg->GetNextPart(pos);
        ATLASSERT( pPart != NULL );

        hr = ProcessMessagePart_RPC_Encoded(pPart, pCodeFunc, dwFlags, dwCallFlags);
        if (FAILED(hr))
        {
            break;
        }
    }

    return hr;
}

HRESULT CCodeTypeBuilder::ProcessMessagePart_RPC_Encoded(
    CWSDLMessagePart *pPart,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    HRESULT hr = S_OK;
    CODETYPE codeType = (dwFlags & CODEFLAG_HEADER) ? CODETYPE_HEADER : CODETYPE_UNK;
    if (pPart->GetTypeName().GetName().GetLength() != 0)
    {
        CXSDElement *pXSDElement = NULL;
        XSDTYPE xsdType = XSDTYPE_ERR;

        hr = pPart->GetType(&pXSDElement, &xsdType);
        if (SUCCEEDED(hr))
        {
            hr = ProcessMessagePart_Type(pPart, pXSDElement, xsdType, codeType,
                pPart->GetName(), pCodeFunc, dwFlags, dwCallFlags);
        }
    }
    else
    {
        EmitFileError(IDS_SDL_RPC_ENCODED_TYPE, pPart, 0);
        hr = E_FAIL;
    }

    return hr;
}

// REVIEW (jasjitg): not going to support these in this version
#if 0
HRESULT CCodeTypeBuilder::ProcessMessage_RPC_Literal(
    CWSDLMessage *pMsg,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    ATLASSERT( pMsg != NULL );
    ATLASSERT( pCodeFunc != NULL );
    ATLASSERT( dwCallFlags & CODEFLAG_RPC );
    ATLASSERT( dwCallFlags & CODEFLAG_LITERAL );

    pMsg;
    pCodeFunc;
    dwFlags;
    dwCallFlags;

    POSITION pos = pMsg->GetFirstPart();
    HRESULT hr = S_OK;
    while (pos != NULL)
    {
        CWSDLMessagePart *pPart = pMsg->GetNextPart(pos);
        ATLASSERT( pPart != NULL );

        hr = ProcessMessagePart_RPC_Literal(pPart, pCodeFunc, dwFlags, dwCallFlags);

        if (FAILED(hr))
        {
            break;
        }
    }

    return hr;
}

HRESULT CCodeTypeBuilder::ProcessMessagePart_RPC_Literal(
    CWSDLMessagePart *pPart,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    HRESULT hr = S_OK;
    CODETYPE codeType = (dwFlags & CODEFLAG_HEADER) ? CODETYPE_HEADER : CODETYPE_UNK;
    if (pPart->GetElementName().GetName().GetLength() != 0)
    {
        CElement *pElement = NULL;
        hr = pPart->GetElement(&pElement);
        if (SUCCEEDED(hr))
        {
            ATLASSERT( pElement != NULL );
            hr = ProcessElement(pElement, pCodeFunc, dwFlags | dwCallFlags | CODEFLAG_ELEMENT, codeType, TRUE, pPart);
        }
    }
    else if (pPart->GetTypeName().GetName().GetLength() != 0)
    {
        CXSDElement *pXSDElement = NULL;
        XSDTYPE xsdType = XSDTYPE_ERR;

        hr = pPart->GetType(&pXSDElement, &xsdType);
        if (SUCCEEDED(hr))
        {
            hr = ProcessMessagePart_Type(pPart, pXSDElement, xsdType, codeType,
                pPart->GetName(), pCodeFunc, dwFlags, dwCallFlags);
        }
    }
    else
    {
        // we should check this in CWSDLMessageParser::OnPart
        ATLASSERT( FALSE );
        EmitErrorHr(E_FAIL);
        hr = E_FAIL;
    }

    return hr;
}
#endif

HRESULT CCodeTypeBuilder::ProcessMessagePart_Type(
    CWSDLMessagePart *pPart,
    CXSDElement *pXSDElement,
    XSDTYPE xsdType,
    CODETYPE codeType,
    const CStringW& strName,
    CCodeFunction *pCodeFunc,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    CCodeTypedElement *pCodeElem = NULL;
    CAutoPtr<CCodeTypedElement> spCodeElem;
    bool bFound = false;
    if (codeType != CODETYPE_HEADER)
    {
        pCodeElem = GetParameterByName(pCodeFunc, strName);
        if (pCodeElem == NULL)
        {
            pCodeElem = pCodeFunc->AddElement();
        }
        else
        {
            bFound = true;
        }
    }
    else
    {
        // REVIEW: I'm not sure about this... (!!!)
        pCodeElem = GetHeaderByName(pCodeFunc, strName);
        if (pCodeElem == NULL)
        {
            spCodeElem.Attach( new CCodeTypedElement );
            if (spCodeElem == NULL)
            {
                EmitErrorHr(E_OUTOFMEMORY);
                return E_OUTOFMEMORY;
            }

            pCodeElem = spCodeElem;
            pCodeElem->SetName(strName);
        }
        else
        {
            bFound = true;
        }
    }

    HRESULT hr = E_FAIL;
    if (pCodeElem != NULL)
    {
        dwFlags |= dwCallFlags;
        if (!bFound)
        {
            if (pXSDElement != NULL)
            {
                hr = ProcessSchemaElement(pXSDElement, pCodeElem, pCodeFunc, dwFlags);
            }
            else if (xsdType != XSDTYPE_ERR)
            {
                hr = S_OK;
                pCodeElem->SetXSDType(xsdType);
            }
        }
        else
        {
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
            pCodeElem->AddFlags(dwFlags);
            if (!bFound)
            {
                hr = pCodeElem->SetName(strName);
            }
            if (SUCCEEDED(hr) && spCodeElem != NULL)
            {
                ATLASSERT( codeType == CODETYPE_HEADER );
                hr = CheckAndAddHeader(pCodeFunc, spCodeElem, dwFlags, pPart);
            }
        }
    }
    else
    {
        EmitErrorHr(E_OUTOFMEMORY);
    }

    return hr;
}

HRESULT CCodeTypeBuilder::ProcessElement(
    CElement *pElem,
    CCodeElementContainer *pContainer,
    DWORD dwFlags,
    CODETYPE parentCodeType,
    BOOL fTopLevel,
    CWSDLMessagePart *pMsgPart
    )
{
    HRESULT hr = E_FAIL;

    //
    // REVIEW: element must have a name
    //         (could do this check in CElementParser)
    //
    if (pElem->GetName().GetLength() == 0)
    {
        return E_FAIL;
    }

    if ((fTopLevel != FALSE) &&
        (parentCodeType == CODETYPE_FUNCTION) &&
        (pElem->GetNumChildren() == 0))
    {
        return S_OK;
    }

    POSITION pos = pElem->GetFirstElement();
    CAutoPtr<CCodeTypedElement> spCodeElem;

    if (pContainer != NULL)
    {
        spCodeElem.Attach( new CCodeTypedElement );
        if (spCodeElem == NULL)
        {
            EmitErrorHr(E_OUTOFMEMORY);
            return E_OUTOFMEMORY;
        }
    }

    CCodeTypedElement typedElem;

    CODETYPE codeType;
    if ((parentCodeType == CODETYPE_FUNCTION) || (pElem->GetNumChildren() == 0))
    {
        fTopLevel = FALSE;
        if (dwFlags & CODEFLAG_HEADER)
        {
            codeType = CODETYPE_HEADER;
        }
        else
        {
            codeType = CODETYPE_PARAMETER;
        }
    }
    else if (parentCodeType == CODETYPE_STRUCT)
    {
        codeType = CODETYPE_FIELD;
    }
    else if (parentCodeType == CODETYPE_HEADER)
    {
        fTopLevel = FALSE;
        codeType = CODETYPE_HEADER;
    }
    else
    {
        codeType = CODETYPE_UNK;
    }

    if (pos == NULL)
    {
        hr = GetTypeFromElement(pElem, spCodeElem, pContainer, dwFlags);
    }

    bool bCheckArray = false;
    if ((pos != NULL) && (pElem->GetTypeName().GetName().GetLength() == 0) && (fTopLevel != TRUE))
    {
        POSITION checkPos = pos;
        CXSDElement *p = pElem->GetNextElement(checkPos);
        if (p->GetElementType() == XSD_COMPLEXTYPE)
        {
            spCodeElem->SetNamespace(p->GetParentSchema()->GetTargetNamespace());
            bCheckArray = true;
        }
    }

    if (bCheckArray != false)
    {
        ATLASSERT( spCodeElem != NULL );

        hr = ProcessArrayDefintion(pElem, pContainer, spCodeElem, dwFlags);
        if (hr == S_FALSE)
        {
            spCodeElem->SetXSDType(XSDTYPE_STRING);
            hr = S_OK;
        }
    }
    else
    {
        if (dwFlags & CODEFLAG_PAD)
        {
            fTopLevel = FALSE;
        }

        while (pos != NULL)
        {
            CXSDElement *p = pElem->GetNextElement(pos);
            if (p == NULL)
            {
                return E_FAIL;
            }

            switch (p->GetElementType())
            {
                case XSD_COMPLEXTYPE:
                {
                    hr = ProcessComplexType(static_cast<CComplexType *>(p), pContainer, dwFlags);

                    if ((fTopLevel != TRUE) && (SUCCEEDED(hr)) && (spCodeElem != NULL))
                    {
                        CODETYPEMAP::CPair *pPair = m_codeTypes.Lookup(static_cast<CComplexType *>(p));
                        ATLASSERT( pPair != NULL );
                        *spCodeElem = pPair->m_value;
                    }

                    break;
                }
                case XSD_SIMPLETYPE:
                {
                    XSDTYPE xsdType = XSDTYPE_UNK;
                    DWORD dwTypeFlags = 0;
                    hr = ProcessSimpleType(static_cast<CSimpleType *>(p), &xsdType, &dwTypeFlags);

                    if ((fTopLevel != TRUE) && (SUCCEEDED(hr)) && (spCodeElem != NULL))
                    {
                        if (xsdType != XSDTYPE_UNK)
                        {
                            spCodeElem->SetXSDType(xsdType);
                            spCodeElem->SetElement(pElem);
                            if (spCodeElem->GetNamespace().GetLength() == 0)
                            {
                                spCodeElem->SetNamespace(p->GetParentSchema()->GetTargetNamespace());
                            }
                        }
                        else
                        {
                            CODESIMPLETYPEMAP::CPair *pPair = m_codeEnums.Lookup(static_cast<CSimpleType *>(p));
                            ATLASSERT( pPair != NULL );
                            *spCodeElem = pPair->m_value;
                        }
                    }
                    break;
                }
                case XSD_UNSUPPORTED:
                {
                    if ((fTopLevel != TRUE) && (spCodeElem != NULL))
                    {
                        spCodeElem->SetXSDType(XSDTYPE_STRING);
                        spCodeElem->SetElement(pElem);
                        hr = S_OK;
                    }
                    break;
                }
                default:
                {
                    hr = E_FAIL;
                    break;
                }
            }

            if (FAILED(hr))
            {
                break;
            }
        }
    }

    if (fTopLevel != TRUE && SUCCEEDED(hr) && spCodeElem != NULL)
    {
        CStringW strChkName;
        if ((dwFlags & CODEFLAG_RPC) && (pMsgPart != NULL))
        {
            ATLASSERT( dwFlags & CODEFLAG_LITERAL );
            strChkName = pMsgPart->GetName();
        }
        else
        {
            strChkName = pElem->GetName();
        }

        // check if in/out parameter
        if (codeType == CODETYPE_PARAMETER)
        {
            CCodeTypedElement *p = GetParameterByName(pContainer, strChkName);
            if (p != NULL)
            {
                p->AddFlags(dwFlags);
                return S_OK;
            }
        }

        // REVIEW: safe-naming ...
        if (FAILED(spCodeElem->SetName(strChkName)))
        {
            return E_FAIL;
        }

        spCodeElem->SetSizeIs(pElem->GetSizeIs());

        spCodeElem->AddFlags(dwFlags);
        if (spCodeElem->GetCodeType() == CODETYPE_UNK)
        {
            spCodeElem->SetCodeType(codeType);
        }
//      if (codeType == CODETYPE_PARAMETER)
//      {
//          spCodeElem->SetElement(pElem);
//      }

        if (codeType != CODETYPE_HEADER)
        {
            if (pContainer->AddElement(spCodeElem) != NULL)
            {
                spCodeElem.Detach();
            }
        }
        else // header
        {
            hr = CheckAndAddHeader(pContainer, spCodeElem, dwFlags, pMsgPart);
        }

        if (spCodeElem != NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

HRESULT CCodeTypeBuilder::ProcessComplexType(
    CComplexType *pType,
    CCodeElementContainer *pContainer,
    DWORD dwFlags)
{
    HRESULT hr = S_OK;
    CODETYPE codeType = CODETYPE_FUNCTION;
    CAutoPtr<CCodeElementContainer> spElem;

    //
    // any named complexType will be considered its own struct,
    // regardless of whether pContainer is null or not
    //
    if (pType->GetName().GetLength() != 0)
    {
        if (m_structs.Lookup(pType) != NULL)
        {
            //
            // we've already processed this element
            //

            return S_OK;
        }

        if (m_currParse.Lookup(pType) != NULL)
        {
            // recursive types are not supported
            EmitFileError(IDS_SDL_RECURSIVE_TYPE, pType, 0, pType->GetParentSchema()->GetTargetNamespace(), pType->GetName());

            return E_FAIL;
        }

        // add to the list of types we are currently parsing
        if (m_currParse.SetAt(pType, 1) == NULL)
        {
            EmitErrorHr(E_OUTOFMEMORY);
            return E_FAIL;
        }

        spElem.Attach( new CCodeElementContainer );
        if (spElem == NULL)
        {
            EmitErrorHr(E_OUTOFMEMORY);
            return E_OUTOFMEMORY;
        }

        ATLASSERT( pType->GetParentSchema() != NULL );

        // set the type name
        hr = spElem->SetName(pType->GetName());
        if (FAILED(hr))
        {
            return hr;
        }

        // set the type namespace
        hr = spElem->SetNamespace(pType->GetParentSchema()->GetTargetNamespace());
        if (FAILED(hr))
        {
            return hr;
        }

        pContainer = spElem;
        codeType = CODETYPE_STRUCT;
    }

    bool bArrayOfElem = false;
//  if (spElem == NULL)
//  {
        POSITION pos = pType->GetFirstElement();
        while (pos != NULL)
        {
            hr = E_FAIL;
            CElement *pElem = pType->GetNextElement(pos);

            if (pElem != NULL)
            {
                hr = ProcessElement(pElem, pContainer, dwFlags, codeType, FALSE, NULL);
                if (pElem->GetArrayType() == cStrMinMaxOccursArray)
                {
                    bArrayOfElem = true;
                }
            }

            if (FAILED(hr))
            {
                break;
            }
        }
//  }

    if (SUCCEEDED(hr) && spElem != NULL)
    {
        hr = CreateSafeNames(spElem);
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
            if (m_structs.SetAt(pType, spElem) != NULL)
            {
                CStringA strSafeName = spElem->GetSafeName();
                spElem.Detach();
                CCodeTypedElement typedElem;
                typedElem.SetCodeType(codeType);
                typedElem.SetElement(pType);
                if ((SUCCEEDED(typedElem.SetCodeTypeName(pType->GetName()))) &&
                    (SUCCEEDED(typedElem.SetSafeCodeTypeName(strSafeName))))
                {
                    typedElem.SetXSDType(XSDTYPE_UNK);
                    if (m_codeTypes.SetAt(pType, typedElem) != NULL)
                    {
                        hr = S_OK;
                    }
                }
            }
        }
    }

    if( true == bArrayOfElem )
    {
        CODETYPEMAP::CPair *pPair = m_codeTypes.Lookup(pType);
        ATLASSERT( pPair != NULL );
        CCodeTypedElement& codeElem = pPair->m_value;
        codeElem.SetFlags(codeElem.GetFlags() | CODEFLAG_DYNARRAYWRAPPER);
    }
    // remove from the list of types we are currently parsing
    m_currParse.RemoveKey(pType);

    return hr;
}

HRESULT CCodeTypeBuilder::ProcessSimpleType(
    CSimpleType *pType,
    XSDTYPE *pXSDType,
    LPDWORD pdwFlags)
{
    ATLASSERT( pType != NULL );
    ATLASSERT( pXSDType != NULL );
    ATLASSERT( pdwFlags != NULL );

    // check CLR/FX-specific namespaces
    ATLASSERT( pType->GetParentSchema() != NULL );

    if ((pType->GetParentSchema()->GetTargetNamespace() == FX_TYPES_NAMESPACEW) ||
        (pType->GetParentSchema()->GetTargetNamespace() == CLR_TYPES_NAMESPACEW))
    {
        // treat their custom types as strings
        *pXSDType = XSDTYPE_STRING;

        return S_OK;
    }

    HRESULT hr = S_OK;
    CAutoPtr<CCodeElementContainer> spElem;

    *pXSDType = XSDTYPE_UNK;

    //
    // any named simpleType will be considered its own enum
    // REVIEW (jasjitg): inefficient when type is binary
    //
    if (pType->GetName().GetLength() != 0)
    {
        if (m_enums.Lookup(pType) != NULL)
        {
            //
            // we've already processed this element
            //

            return S_OK;
        }

        spElem.Attach( new CCodeElementContainer );
        if (spElem == NULL)
        {
            EmitErrorHr(E_OUTOFMEMORY);
            return E_OUTOFMEMORY;
        }

        hr = spElem->SetName(pType->GetName());
        if (FAILED(hr))
        {
            return hr;
        }
    }

    POSITION pos = pType->GetFirstEnumeration();
    if (pos != NULL)
    {
        ATLASSERT( pType->GetParentSchema() != NULL );
        if (FAILED(spElem->SetNamespace(pType->GetParentSchema()->GetTargetNamespace())))
        {
            EmitErrorHr(E_OUTOFMEMORY);
            return E_OUTOFMEMORY;
        }

        NAMEMAP scopedMap;

        while (pos != NULL)
        {
            CEnumeration * p = pType->GetNextEnumeration(pos);
            ATLASSERT( p != NULL );

            CCodeTypedElement *pNewElem = spElem->AddElement();
            if (pNewElem != NULL)
            {
                if (FAILED(pNewElem->SetCodeTypeName(p->GetValue())))
                {
                    hr = E_FAIL;
                }
                else
                {
                    if ((FAILED(CreateSafeCppName(pNewElem->GetSafeCodeTypeName(), pNewElem->GetCodeTypeName()))) ||
                        (FAILED(CheckNameMap(scopedMap, pNewElem->GetSafeCodeTypeName(), true))))
                    {
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            if (FAILED(hr))
            {
                break;
            }
        }

        if (SUCCEEDED(hr) && spElem != NULL)
        {
            hr = CreateSafeNames(spElem);
            if (SUCCEEDED(hr))
            {
                hr = E_FAIL;
                if (m_enums.SetAt(pType, spElem) != NULL)
                {
                    CStringA strSafeName = spElem->GetSafeName();
                    spElem.Detach();
                    CCodeTypedElement typedElem;
                    typedElem.SetCodeType(CODETYPE_ENUM);
                    typedElem.SetElement(pType);
                    if ((SUCCEEDED(typedElem.SetCodeTypeName(pType->GetName()))) &&
                        (SUCCEEDED(typedElem.SetSafeCodeTypeName(strSafeName))))
                    {
                        typedElem.SetXSDType(XSDTYPE_UNK);
                        if (m_codeEnums.SetAt(pType, typedElem) != NULL)
                        {
                            hr = S_OK;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // derived types

        CXSDElement *pXSDOut = NULL;
        hr = GetTypeFromQName(pType->GetBase(), pType, &pXSDOut, pXSDType);
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
            if (*pXSDType != XSDTYPE_ERR)
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}

HRESULT CCodeTypeBuilder::SortStructs()
{
    POSITION pos = m_structs.GetStartPosition();
    POSITION oldpos = NULL;
    m_structs.DisableAutoRehash();

    while (pos != NULL)
    {
        oldpos = pos;
        m_structs.GetNext(pos);
        HRESULT hr = SortStructHelper(oldpos);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_structs.EnableAutoRehash();

    return S_OK;
}

HRESULT CCodeTypeBuilder::SortStructHelper(POSITION currPos)
{
    ATLASSERT( currPos != NULL );

    CCodeStruct *pStruct = m_structs.GetValueAt(currPos);
    if (pStruct == NULL)
    {
        return S_OK;
    }

    HRESULT hr = S_OK;

    POSITION pos = pStruct->GetFirstElement();
    while (pos != NULL)
    {
        CCodeTypedElement *pElem = pStruct->GetNextElement(pos);

        ATLASSERT( pElem != NULL );

        CXSDElement *pXSD = pElem->GetElement();
        if (pXSD != NULL && pXSD->GetElementType() == XSD_COMPLEXTYPE)
        {
            CComplexType *pType = static_cast<CComplexType*>(pXSD);
            CODESTRUCTMAP::CPair *p = m_structs.Lookup(pType);

            if (p != NULL)
            {
                //
                // it's okay for it to be NULL -- it means it's already been visited
                //
                if (p->m_value != NULL)
                {
                    hr = SortStructHelper(p);
                }
            }
            // REVIEW (jasjitg): !!!
//          else
//          {
//              hr = E_FAIL;
//          }
        }

        if (FAILED(hr))
        {
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (m_pProxy->AddStruct(pStruct) != NULL)
        {
            CODESTRUCTMAP::CPair *pRem = m_structs.GetNext(currPos);
            pRem->m_value = NULL;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

HRESULT CCodeTypeBuilder::GetTypeFromElement(
    CElement *pElem,
    CCodeTypedElement *pCodeElem,
    CCodeElementContainer *pContainer,
    DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    CXSDElement *pRet = pElem->GetType();
    if (pRet == NULL)
    {
        hr = ProcessXSDElement(pElem, pElem->GetTypeName(), pCodeElem);
    }
    else
    {
        hr = ProcessSchemaElement(pRet, pCodeElem, pContainer, dwFlags);
        if (FAILED(hr))
        {
            EmitFileError(IDS_SDL_UNRESOLVED_ELEM, pElem, 0,
                pElem->GetParentSchema()->GetTargetNamespace(),
                pElem->GetTypeName().GetName());
        }
    }
    if (pElem->GetArrayType() == cStrMinMaxOccursArray)
    {
        pCodeElem->SetFlags((pCodeElem->GetFlags() & ~CODEFLAG_FIXEDARRAY) | CODEFLAG_DYNARRAY);
    }
	if (pElem->GetNullable())
	{
		pCodeElem->AddFlags(CODEFLAG_NULLABLE);		
	}
    return hr;
}

HRESULT CCodeTypeBuilder::ProcessXSDElement(
    CXMLElement *pElem,
    CQName& typeName,
    CCodeTypedElement *pCodeElem)
{
    CStringW strUri;
    if(typeName.GetPrefix().IsEmpty() && typeName.GetName().IsEmpty())
    {
        pCodeElem->SetXSDType(XSDTYPE_STRING);
        EmitFileWarning(IDS_SDL_DEFAULT_TYPE, pElem, 0);
        return S_OK;
    }

    if (SUCCEEDED(pElem->GetNamespaceUri(typeName.GetPrefix(), strUri)))
    {
        // check CLR/FX-specific namespaces
        if ((strUri == FX_TYPES_NAMESPACEW) ||
            (strUri == CLR_TYPES_NAMESPACEW))
        {
            // treat their custom types as strings
            pCodeElem->SetXSDType(XSDTYPE_STRING);
            EmitFileWarning(IDS_SDL_CUSTOM_TYPE, pElem, 0,
                strUri, typeName.GetName());
            return S_OK;
        }
        XSDTYPE xsdType;
        HRESULT hr = GetXSDType(strUri, typeName.GetName(), &xsdType);
        if (hr == S_OK)
        {
            pCodeElem->SetXSDType(xsdType);
            return S_OK;
        }
    }

    EmitFileError(IDS_SDL_UNRESOLVED_ELEM, pElem, 0, strUri, typeName.GetName());

    return E_FAIL;
}

HRESULT CCodeTypeBuilder::ProcessSchemaElement(
    CXSDElement *pElem,
    CCodeTypedElement *pCodeElem,
    CCodeElementContainer *pContainer,
    DWORD dwFlags)
{
    ATLASSERT( pCodeElem != NULL );

    HRESULT hr = E_FAIL;
    switch (pElem->GetElementType())
    {
        case XSD_COMPLEXTYPE:
        {
            CComplexType *pCplx = static_cast<CComplexType *>(pElem);
            DWORD dwTypeFlags = IsArrayDefinition(pCplx);
            CElement *pVarArrElement = NULL;
            if (dwTypeFlags & CODEFLAG_FIXEDARRAY)
            {
                hr = ProcessArray(pCplx, pContainer, pCodeElem, dwFlags);
                if (pCodeElem->GetNamespace().GetLength() == 0)
                {
                    pCodeElem->SetNamespace(pElem->GetParentSchema()->GetTargetNamespace());
                }
            }
            else if (dwTypeFlags & CODEFLAG_DYNARRAY)
            {
                EmitFileWarning(IDS_SDL_INVALID_ARRAY_DESC, pElem, 0);
                pCodeElem->SetXSDType(XSDTYPE_STRING);
                if (pCodeElem->GetNamespace().GetLength() == 0)
                {
                    pCodeElem->SetNamespace(pElem->GetParentSchema()->GetTargetNamespace());
                }
                hr = S_OK;
            }
            else if ((dwTypeFlags == CODEFLAG_ERR) && (IsVarArrayDefinition(pCplx,dwFlags, &pVarArrElement) != FALSE))
            {
                ATLASSERT( pVarArrElement != NULL );
                hr = ProcessVarArray(pVarArrElement, pContainer, pCodeElem, dwFlags);
                if (pCodeElem->GetNamespace().GetLength() == 0)
                {
                    pCodeElem->SetNamespace(pElem->GetParentSchema()->GetTargetNamespace());
                }
            }
            else
            {
                hr = ProcessComplexType(pCplx, pContainer, dwFlags);
                if (SUCCEEDED(hr) && (pContainer != NULL))
                {
                    CODETYPEMAP::CPair *pPair = m_codeTypes.Lookup(pCplx);
                    ATLASSERT( pPair != NULL );
                    *pCodeElem = pPair->m_value;
                }
            }

            if (FAILED(hr))
            {
                break;
            }

            break;
        }
        case XSD_SIMPLETYPE:
        {
            XSDTYPE xsdType = XSDTYPE_UNK;
            DWORD dwTypeFlags = 0;
            hr = ProcessSimpleType(static_cast<CSimpleType *>(pElem), &xsdType, &dwTypeFlags);

            if (FAILED(hr))
            {
                break;
            }

            if (xsdType != XSDTYPE_UNK)
            {
                pCodeElem->SetXSDType(xsdType);
                pCodeElem->SetElement(pElem);
                if (pCodeElem->GetNamespace().GetLength() == 0)
                {
                    pCodeElem->SetNamespace(pElem->GetParentSchema()->GetTargetNamespace());
                }
            }
            else if (pContainer != NULL)
            {
                CODESIMPLETYPEMAP::CPair *pPair = m_codeEnums.Lookup(static_cast<CSimpleType *>(pElem));
                ATLASSERT( pPair != NULL );
                *pCodeElem = pPair->m_value;
            }

            break;
        }
        case XSD_UNSUPPORTED:
        {
            if (pContainer != NULL)
            {
                pCodeElem->SetXSDType(XSDTYPE_STRING);
                pCodeElem->SetElement(pElem);
            }
            break;
        }
        default:
        {
            hr = E_FAIL;
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (pCodeElem->GetElement() == NULL)
        {
            pCodeElem->SetElement(pElem);
        }
    }

    return hr;
}

//    - PAD or PID (style="document", use="literal")
//        - if one part and part element references an element w/o a type attribute and child is a complexType child that is not an array descriptor
//          or type attribute references a complexType that is not an array descriptor
//            - it is a PID
//        - else it is a PAD
//
//    - Also need a function to determine the element given the element QName and the message part
//        - it will need to handle the case where the QName::prefix is XSD

HRESULT CCodeTypeBuilder::GetCallFlags(
        LPCWSTR wszParts,
        CWSDLMessage *pMessage,
        CWSDLPortTypeIO *pIO,
        CWSDLPortTypeOperation *pBindingOp,
        CWSDLBinding *pBinding,
        LPDWORD pdwFlags)
{
    // TODO (jasjitg): support the parts= attribute

//  ATLASSERT( wszParts != NULL );
    ATLASSERT( pMessage != NULL );
    ATLASSERT( pIO != NULL );
    ATLASSERT( pBindingOp != NULL );
    ATLASSERT( pBinding != NULL );
    ATLASSERT( pdwFlags != NULL );

    POSITION pos = pMessage->GetFirstPart();
    CWSDLMessagePart *pPart = NULL;
    if (pos != NULL)
    {
        pPart = pMessage->GetNextPart(pos);
        ATLASSERT( pPart != NULL );
    }

    // REVIEW: ... semantics of message with no parts?
    return GetCallFlags(pMessage, pPart, pIO, pBindingOp, pBinding, pdwFlags);
}

HRESULT CCodeTypeBuilder::GetCallFlags(
    CWSDLMessage *pMessage,
    CWSDLMessagePart *pPart,
    CWSDLPortTypeIO *pIO,
    CWSDLPortTypeOperation *pBindingOp,
    CWSDLBinding *pBinding,
    LPDWORD pdwFlags)
{
    ATLASSERT( pMessage != NULL );
//  ATLASSERT( pPart != NULL );
    ATLASSERT( pIO != NULL );
    ATLASSERT( pBindingOp != NULL );
    ATLASSERT( pBinding != NULL );
    ATLASSERT( pdwFlags != NULL );

    SOAPUSE use = SOAPUSE_UNK;
    SOAPSTYLE style = SOAPSTYLE_UNK;
    CStringW strEncodingStyle;

    CSoapBody *pBody = pIO->GetSoapBody();
    if (pBody != NULL)
    {
        use = pBody->GetUse();
        strEncodingStyle = pBody->GetEncodingStyle();
    }
    if (use == SOAPUSE_UNK)
    {
        EmitError(IDS_SDL_INTERNAL);
        return E_FAIL;
    }

    // get style
    CSoapOperation *pSoapOp = pBindingOp->GetSoapOperation();
    if (pSoapOp != NULL)
    {
        style = pSoapOp->GetStyle();
    }
    if (style == SOAPSTYLE_UNK)
    {
        // get from parent
        CSoapBinding *pSoapBinding = pBinding->GetSoapBinding();
        if (pSoapBinding != NULL)
        {
            style = pSoapBinding->GetStyle();
        }
    }
    if (style == SOAPSTYLE_UNK)
    {
        // if not specified by <soap:operation .../> or <soap:binding .../>
        // default to document
        style = SOAPSTYLE_DOC;
    }

    if ((use == SOAPUSE_ENCODED) && (style != SOAPSTYLE_RPC))
    {
        EmitFileError(IDS_SDL_DOC_ENCODED, pBindingOp, 0);
        return E_FAIL;
    }

    // REVIEW (jasjitg): not supported properly in atlsoap!!!
    if ((use == SOAPUSE_LITERAL) && (style != SOAPSTYLE_DOC))
    {
        // REVIEW (jasjitg): may support this (?)
        EmitFileError(IDS_SDL_RPC_LITERAL, pBindingOp, 0);
        return E_FAIL;
    }

    DWORD dwFlags = 0;
    if (use == SOAPUSE_ENCODED)
    {
        if (pBody->GetEncodingStyle() != SOAP_ENCODINGSTYLEW)
        {
            EmitFileError(IDS_SDL_ENCODINGSTYLE, pBody, 0, pBody->GetEncodingStyle());
            return E_FAIL;
        }

        dwFlags |= CODEFLAG_ENCODED;
    }
    else
    {
        ATLASSERT( use == SOAPUSE_LITERAL );
        dwFlags |= CODEFLAG_LITERAL;
    }

    if (style == SOAPSTYLE_RPC)
    {
        dwFlags |= CODEFLAG_RPC;
    }
    else
    {
        ATLASSERT( style == SOAPSTYLE_DOC );
        dwFlags |= CODEFLAG_DOCUMENT;
    }

    if (style == SOAPSTYLE_DOC)
    {
        if ((pPart != NULL) &&
            (pMessage->GetNumParts() == 1) &&
            (pPart->GetElementName().GetName().GetLength() != 0) &&
            (pPart->GetName() == L"parameters"))
        {
            // hack to determine ParametersAsDocument or ParametersInDocument
            // basically, the hack is to see if the message part is named "parameters"
            // (URT does it in the same way, basically)
            dwFlags |= CODEFLAG_PID;
        }
        else
        {
            dwFlags |= CODEFLAG_PAD;

            if ((pPart != NULL) &&
                (pPart->GetTypeName().GetName().GetLength() != 0))
            {
                // if referenced by type, see if the type is a simpleType or complexType
                CXSDElement *pXSDElement = NULL;
                XSDTYPE xsdType = XSDTYPE_ERR;
                if (FAILED(pPart->GetType(&pXSDElement, &xsdType)))
                {
                    return E_FAIL;
                }

                if (pXSDElement != NULL)
                {
                    // it is a simpleType or complexType, which will require special processing
                    // at runtime
                    dwFlags |= CODEFLAG_CHAIN;
                }
            }
        }
    }

    *pdwFlags = dwFlags;
    return S_OK;
}

//
// WSDL array defintions are of the form:
//
// <complexType ...>
//   <complexContent>
//      <restriction base="soapenc:Array">
//         <attribute ref="soapenc:arrayType" wsdl:arrayType="<type_qname>[...]"/>
//      </restriction>
//   </complexContent>
// </complexType>
//
CODEFLAGS CCodeTypeBuilder::IsArrayDefinition(CComplexType *pType)
{
    ATLASSERT( pType != NULL );

    CContent *pContent = pType->GetContent();
    if (pContent != NULL)
    {
        CComplexType *pRestriction = pContent->GetType();
        if ((pRestriction != NULL) && (pRestriction->GetElementType() == XSD_RESTRICTION))
        {
            CQName& base = pRestriction->GetBase();
            if (base.GetName().GetLength() != 0)
            {
                CStringW strUri;
                if ((SUCCEEDED(pRestriction->GetNamespaceUri(base.GetPrefix(), strUri))) &&
                    (strUri == SOAP_ENCODINGSTYLEW) && (base.GetName() == L"Array"))
                {
                    if (pRestriction->GetNumAttributes() == 1)
                    {
                        POSITION pos = pRestriction->GetFirstAttribute();
                        ATLASSERT( pos != NULL );

                        CAttribute *pAttribute = pRestriction->GetNextAttribute(pos);
                        ATLASSERT( pAttribute != NULL );

                        CQName& ref = pAttribute->GetRefName();
                        if (ref.GetName().GetLength() != 0)
                        {
                            if ((SUCCEEDED(pAttribute->GetNamespaceUri(ref.GetPrefix(), strUri))) &&
                                (strUri == SOAP_ENCODINGSTYLEW) && (ref.GetName() == L"arrayType"))
                            {
                                if (pAttribute->GetArrayType().GetLength() != 0)
                                {
                                    // it is an array definition
                                    return CODEFLAG_FIXEDARRAY;
                                }
                            }
                        }
                    }
                }

                // dynamic array (maybe)
                return CODEFLAG_DYNARRAY;
            }
        }
    }

    return CODEFLAG_ERR;
}

BOOL CCodeTypeBuilder::IsVarArrayDefinition(CComplexType *pType,DWORD dwFlags, CElement **ppElement)
{
    ATLASSERT( pType != NULL );
    ATLASSERT( ppElement != NULL );
    POSITION pos = pType->GetFirstElement();
    BOOL bFirstElement=TRUE;
    while (pos != NULL)
    {
        CElement *pElem = pType->GetNextElement(pos);
        ATLASSERT( pElem != NULL );

        //If doc\lit sees an unbounded single elem within complexType -
        //complexType is var array. All other unbounded elems (in both doc/lit and rpc/enc) are var arrays themselves (childs of complexType struct).
        // b1) unbounded <==> max-min >= 1, except when min=0 and max=1 - this is considered single scalar
        // (not array).
        //Example: If Method(Message?) is Rpc/enc - in
        //<s:complexType name="XAgentRegBean">
        //  that contain only 1 elem, the 2 lines below are equiv (in terms of proxy output).
        //     <s:element minOccurs="0" maxOccurs="unbounded" name="agentParams" type="s:string" />
        //     <s:element minOccurs="1" maxOccurs="1" name="agentParams" type="s1:ArrayOfString" />
        // If Method is Doc/Lit (PID or PAD), then first line generates
        // XAgentRegBean itself is string[], but second line XAgentRegBean is a
        //struct with member string[].

        if (((pElem->GetMinOccurs()==0) &&
            (pElem->GetMaxOccurs() != 1) &&
            ((pElem->GetMaxOccurs()==MAXOCCURS_UNBOUNDED) ||
             (pElem->GetMinOccurs() <= pElem->GetMaxOccurs()))) ||
            (pElem->GetMinOccurs() > 0 &&
             (pElem->GetMaxOccurs()==MAXOCCURS_UNBOUNDED ||
              pElem->GetMinOccurs() < pElem->GetMaxOccurs())))
        {
            *ppElement = pElem;
            if (dwFlags & CODEFLAG_DOCUMENT && bFirstElement && pos == NULL)
            {
                // should be the only element
                return TRUE;
            }
            (*ppElement)->SetArrayType(cStrMinMaxOccursArray);
        }
        bFirstElement=FALSE;
    } //end while

    *ppElement = NULL;
    return FALSE;
}

HRESULT CCodeTypeBuilder::ProcessArrayDefintion(
    CElement *pElem,
    CCodeElementContainer *pContainer,
    CCodeTypedElement *pCodeElem,
    DWORD dwFlags)
{
    ATLASSERT( pElem != NULL );

    POSITION pos = pElem->GetFirstElement();
    ATLASSERT( pos != NULL ); // pre-condition
    CXSDElement *p  = pElem->GetNextElement(pos);

    if (pCodeElem->GetNamespace().GetLength()==0)
    {
        pCodeElem->SetNamespace(pElem->GetParentSchema()->GetTargetNamespace());
    }

    if ((p == NULL) || (p->GetElementType() != XSD_COMPLEXTYPE))
    {
        EmitFileWarning(IDS_SDL_INVALID_ARRAY_DESC, pElem, 0);
        return S_FALSE;
    }

    HRESULT hr = E_FAIL;
    CComplexType *pArrElem = static_cast<CComplexType *>(p);
    POSITION arrPos = pArrElem->GetFirstElement();
    CElement *pArrDesc = NULL;
    if (arrPos != NULL)
    {
        pArrDesc = pArrElem->GetNextElement(arrPos);
        ATLASSERT( pArrDesc != NULL );

        hr = ProcessVarArray(pArrDesc, pContainer, pCodeElem, dwFlags);
        if (arrPos != NULL)
        {
            hr = E_FAIL;
        }
    }
//  else if (IsVarArrayDefinition(pArrElem, &pArrDesc))
//  {
//      hr = ProcessVarArray(pArrDesc, pContainer, pCodeElem, dwFlags);
//  }
    else
    {
        CODEFLAGS dwArrayFlags = IsArrayDefinition(pArrElem);
        if (dwArrayFlags & CODEFLAG_FIXEDARRAY)
        {
            hr = ProcessArray(pArrElem, pContainer, pCodeElem, dwArrayFlags);
        }
        else if (dwArrayFlags & CODEFLAG_DYNARRAY)
        {
            hr = S_FALSE;
        }
    }

    if (hr == S_FALSE)
    {
        EmitFileWarning(IDS_SDL_INVALID_ARRAY_DESC, pElem, 0);
    }

    return hr;
}

HRESULT CCodeTypeBuilder::ProcessVarArray(
    CElement *pArrDesc,
    CCodeElementContainer *pContainer,
    CCodeTypedElement *pCodeElem,
    DWORD dwFlags)
{
    ATLASSERT( pArrDesc != NULL );
    ATLASSERT( pContainer != NULL );
    ATLASSERT( pCodeElem != NULL );

    HRESULT hr = S_OK;
    if (pArrDesc->HasMinOccurs() && pArrDesc->HasMaxOccurs())
    {
        hr = GetTypeFromElement(pArrDesc, pCodeElem, pContainer, 0);
        if (SUCCEEDED(hr))
        {
            ATLASSERT( pCodeElem != NULL );
            if ((pArrDesc->GetMinOccurs() == pArrDesc->GetMaxOccurs()) &&
                (pArrDesc->GetMinOccurs() != MAXOCCURS_UNBOUNDED)) // REVIEW: error?
            {
                // treat as fixed-size one-dimensional
                pCodeElem->AddDimension(1);
                pCodeElem->AddDimension(pArrDesc->GetMaxOccurs());
                pCodeElem->AddFlags(CODEFLAG_FIXEDARRAY);
            }
            else
            {
                pCodeElem->AddFlags(CODEFLAG_DYNARRAY);
            }
        }
    }

    return hr;
}

// NOTE: precondition: IsArrayDefintion(pType, ...) is TRUE
// the expected format is assumed
HRESULT CCodeTypeBuilder::ProcessArray(
    CComplexType *pType,
    CCodeElementContainer *pContainer,
    CCodeTypedElement *pCodeElem,
    DWORD dwFlags)
{
    ATLASSERT( pType != NULL );
    ATLASSERT( pContainer != NULL );
    ATLASSERT( pCodeElem != NULL );
    ATLASSERT( IsArrayDefinition(pType) != FALSE );

    CContent *pContent = pType->GetContent();
    ATLASSERT( pContent != NULL );

    CComplexType *pRestriction = pContent->GetType();
    ATLASSERT( pRestriction != NULL );
    ATLASSERT( pRestriction->GetElementType() == XSD_RESTRICTION );
    ATLASSERT( pRestriction->GetNumAttributes() == 1 );

    POSITION pos = pRestriction->GetFirstAttribute();
    ATLASSERT( pos != NULL );

    CAttribute *pAttribute = pRestriction->GetNextAttribute(pos);
    ATLASSERT( pAttribute != NULL );

    CStringW strArrayType = pAttribute->GetArrayType();

    LPCWSTR wszArrayType = strArrayType;
    LPCWSTR wszTmp = wcschr(wszArrayType, L'[');

    HRESULT hr = E_FAIL;
    if (wszTmp != NULL)
    {
        // lookup element
        CQName type(wszArrayType, (int)(wszTmp-wszArrayType));
        CXSDElement *pXSDElement;
        XSDTYPE xsdType = XSDTYPE_UNK;
        hr = GetTypeFromQName(type, pAttribute, &pXSDElement, &xsdType);
        if (SUCCEEDED(hr))
        {
            if (pXSDElement != NULL)
            {
                pCodeElem->SetElement(pXSDElement);
                hr = ProcessSchemaElement(pXSDElement, pCodeElem, pContainer, dwFlags);
            }
            else
            {
                ATLASSERT( xsdType != XSDTYPE_ERR );
                pCodeElem->SetXSDType(xsdType);
            }

            if (SUCCEEDED(hr))
            {
                hr = GetArrayDimensions(pAttribute, pCodeElem);
                if (FAILED(hr))
                {
                    EmitFileError(IDS_SDL_INVALID_ARRAY_DESC_ERR, pAttribute, 0);
                }
            }
        }
    }

    return hr;
}

HRESULT CCodeTypeBuilder::GetTypeFromQName(
    CQName& type,
    CXSDElement *pXSDElement,
    CXSDElement **ppXSDElement,
    XSDTYPE *pXSD)
{
    ATLASSERT( pXSDElement != NULL );
    ATLASSERT( ppXSDElement != NULL );
    ATLASSERT( pXSD != NULL );

    *ppXSDElement = NULL;
    *pXSD = XSDTYPE_ERR;

    HRESULT hr = E_FAIL;
    CStringW strUri;
    hr = pXSDElement->GetNamespaceUri(type.GetPrefix(), strUri);
    if (SUCCEEDED(hr))
    {
        hr = GetXSDType(strUri, type.GetName(), pXSD);
        if (hr == S_FALSE)
        {
            hr = E_FAIL;
            *ppXSDElement = pXSDElement->GetParentSchema()->GetNamedItem(strUri, type.GetName());
            if (*ppXSDElement != NULL)
            {
                hr = S_OK;
            }
        }
    }

    if (FAILED(hr))
    {
        EmitFileError(IDS_SDL_UNRESOLVED_ELEM, pXSDElement, 0,
            strUri,
            type.GetName());
    }
    return hr;
}

HRESULT CCodeTypeBuilder::GetArrayDimensions(
    CAttribute *pAttribute,
    CCodeTypedElement *pCodeElem)
{
    ATLASSERT( pAttribute != NULL );
    ATLASSERT( pCodeElem != NULL );
    ATLASSERT( pAttribute->GetArrayType().GetLength() != 0 );

    LPCWSTR wszArrayType = pAttribute->GetArrayType();
    LPCWSTR wszChr = wcschr(wszArrayType, L'[');
    if (wszChr == NULL)
    {
        return S_FALSE;
    }
    if (wszChr[1] == ']')
    {
        if (wszChr[2] != '\0')
        {
            // jagged array, or something else
            pCodeElem->SetFlags((pCodeElem->GetFlags() & ~(CODEFLAG_FIXEDARRAY | CODEFLAG_DYNARRAY)));
            return S_FALSE;
        }

        // var array
        pCodeElem->SetFlags((pCodeElem->GetFlags() & ~CODEFLAG_FIXEDARRAY) | CODEFLAG_DYNARRAY);
        return S_OK;
    }
    int nDimCnt = 0;
    pCodeElem->AddDimension(0);
    while ((wszChr != NULL) && (*wszChr))
    {
        wszChr++;
        nDimCnt++;

        pCodeElem->AddDimension(_wtoi(wszChr));
        int nDim = _wtoi(wszChr);
        if (nDim <= 0)
        {
            // some unknown array description
            pCodeElem->ClearDims();
            return E_FAIL;
        }
        wszChr = wcschr(wszChr, L',');
    }
    pCodeElem->SetDimension(0, nDimCnt);

    return S_OK;
}

HRESULT CCodeTypeBuilder::ProcessSoapHeaders(
    CCodeFunction *pElem,
    CWSDLPortTypeIO *pIO,
    CWSDLPortTypeOperation *pBindingOp,
    CWSDLBinding *pBinding,
    DWORD dwFlags)
{
    ATLASSERT( pElem != NULL );
    ATLASSERT( pIO != NULL );

    HRESULT hr = S_OK;
    POSITION pos = pIO->GetFirstSoapHeader();
    while ((pos != NULL) && (hr == S_OK))
    {
        CSoapHeader *pHeader = pIO->GetNextSoapHeader(pos);
        ATLASSERT( pHeader != NULL );

        if ((pHeader->GetNamespace().GetLength() != 0) &&
            (pElem->GetNamespace() != CW2A(pHeader->GetNamespace())))
        {
            // ATL Server does not support headers with a
            // namespace different from their associated function
            EmitFileError(IDS_SDL_HEADER_DIFF_NAMESPACES, pHeader, 0);
            return E_FAIL;
        }

        DWORD dwAddFlags = CODEFLAG_HEADER;
        if (pHeader->GetRequired() != false)
        {
            dwAddFlags |= CODEFLAG_MUSTUNDERSTAND;
        }
        CWSDLMessage *pMessage = pHeader->GetMessage();
        if (pMessage != NULL)
        {
            CWSDLMessagePart *pPart = pMessage->GetPartByName(pHeader->GetParts());
            if (pPart != NULL)
            {
                DWORD dwCallFlags;
                hr = GetCallFlags(pMessage, pPart, pIO, pBindingOp, pBinding, &dwCallFlags);
                if (SUCCEEDED(hr))
                {
                    if (dwCallFlags & CODEFLAG_CHAIN)
                    {
                        if (pIO->GetNumSoapHeaders() > 1)
                        {
                            EmitFileError(IDS_SDL_PAD_INVALID_SOAP, pIO, 0);
                            return E_FAIL;
                        }
                    }

                    CODEHEADERPARTMAP::CPair *p = m_headersByPart.Lookup(pPart);
                    if (p == NULL)
                    {
                        if (dwCallFlags & CODEFLAG_PID)
                        {
                            hr = ProcessMessagePart_PID(pPart, pElem, dwFlags | dwAddFlags, dwCallFlags);
                        }
                        else if (dwCallFlags & CODEFLAG_PAD)
                        {
                            hr = ProcessMessagePart_PAD(pPart, pElem, dwFlags | dwAddFlags, dwCallFlags);
                        }
                        else if ((dwCallFlags & (CODEFLAG_RPC | CODEFLAG_ENCODED)) == (CODEFLAG_RPC | CODEFLAG_ENCODED))
                        {
                            hr = ProcessMessagePart_RPC_Encoded(pPart, pElem, dwFlags | dwAddFlags, dwCallFlags);
                        }
//                      else if ((dwCallFlags & (CODEFLAG_RPC | CODEFLAG_LITERAL)) == (CODEFLAG_RPC | CODEFLAG_LITERAL))
//                      {
//                          hr = ProcessMessagePart_RPC_Literal(pPart, pElem, dwFlags | dwAddFlags, dwCallFlags);
//                      }
                        else
                        {
                            ATLASSERT( FALSE );
                            EmitError(IDS_SDL_INTERNAL);
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        hr = CheckDuplicateHeaders(pElem, p->m_value, dwFlags | dwAddFlags);
                        if (SUCCEEDED(hr))
                        {
                            // REVIEW (jasjitg): can maybe make this better (inefficient)
                            CAutoPtr<CCodeTypedElement> spElem( new CCodeTypedElement );
                            if (spElem != NULL)
                            {
                                *spElem = *(p->m_value);
                                spElem->AddFlags(dwFlags | dwCallFlags);
                                hr = AddHeaderToFunction(pElem, spElem, pPart);
                            }
                            else
                            {
                                EmitErrorHr(E_OUTOFMEMORY);
                                hr = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
                else
                {
                    hr = E_FAIL;
                    EmitError(IDS_SDL_INTERNAL);
                }
            }
            else
            {
                CStringW strUri;
                if (SUCCEEDED(GetElementInfo(pHeader, pHeader->GetMessageName(), strUri)))
                {
                    EmitFileError(IDS_SDL_UNRESOLVED_MSGPART, pHeader, 0,
                        pHeader->GetParts(), strUri, pHeader->GetMessageName().GetName());
                }
                else
                {
                    EmitErrorHr(E_FAIL);
                }

                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

HRESULT CCodeTypeBuilder::GetElementInfo(CXMLElement *pElem, CQName& name, CStringW& strUri)
{
    ATLASSERT( pElem != NULL );
    return pElem->GetNamespaceUri(name.GetPrefix(), strUri);
}

HRESULT CCodeTypeBuilder::CheckDuplicateHeaders(CCodeFunction *pCodeFunc, CCodeTypedElement *pElem, DWORD dwFlags)
{
    ATLASSERT( pCodeFunc != NULL );
    ATLASSERT( pElem != NULL );

    CCodeTypedElement *p = GetHeaderByName(pCodeFunc, pElem->GetName());
    if (p != NULL)
    {
        // REVIEW: I'm not sure about this...
        DWORD dwFlagsTmp = dwFlags & CODEFLAG_IN;
        if (p->GetFlags() & dwFlagsTmp)
        {
            CStringW strUri;
            EmitFileError(IDS_SDL_SOAPHEADER_DUPNAME, pElem->GetElement(), 0,
                pElem->GetElement()->GetParentSchema()->GetTargetNamespace(), pElem->GetName());
            return E_FAIL;
        }
    }

    return S_OK;
}

CCodeTypedElement * CCodeTypeBuilder::GetHeaderByName(CCodeFunction *pCodeFunc, const CStringW& strName)
{
    ATLASSERT( pCodeFunc != NULL );
    POSITION pos = pCodeFunc->GetFirstHeader();
    while (pos != NULL)
    {
        CCodeTypedElement *p = pCodeFunc->GetNextHeader(pos);
        if (p->GetName() == strName)
        {
            return p;
        }
    }

    return NULL;
}

HRESULT CCodeTypeBuilder::AddHeaderToFunction(CCodeFunction *pCodeFunc, CAutoPtr<CCodeTypedElement> &spElem, CWSDLMessagePart *pPart)
{
    ATLASSERT( pCodeFunc != NULL );
    ATLASSERT( spElem != NULL );
    ATLASSERT( pPart != NULL );

    // look for header element in type->header map
    CXSDElement *pElemType = spElem->GetElement();
    CCodeTypedElement *pMember = NULL;
    const CODEHEADERTYPEMAP::CPair *pTypePair = m_headersByType.Lookup(pElemType);
    if (pTypePair != NULL)
    {
        pMember = pTypePair->m_value;
    }

    // safe-naming
    if (pMember == NULL)
    {
        // name header members <typename>Value, like C# proxy generator
        CStringW valueName(spElem->GetName());
        valueName.Append(L"Value");

        if ((FAILED(CreateSafeCppName(spElem->GetSafeName(), valueName))) ||
            (FAILED(CheckGlobalNameMap(spElem->GetSafeName(), true))))
        {
            EmitErrorHr(E_OUTOFMEMORY);
            return E_OUTOFMEMORY;
        }

        // add to m_headersByType
        CAutoPtr<CCodeTypedElement> spNewElem( new CCodeTypedElement );
        if (spNewElem != NULL)
        {
            *spNewElem = *spElem;
            spNewElem->SetFlags(spNewElem->GetFlags() & ~(CODEFLAG_IN | CODEFLAG_OUT));
            if (m_headersByType.SetAt(pElemType, spNewElem) != NULL)
            {
                spNewElem.Detach();
            }
            else
            {
                EmitErrorHr(E_OUTOFMEMORY);
                return E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        spElem->SetSafeName(pMember->GetSafeName());
    }

    // look for header element in part->header map
    const CODEHEADERPARTMAP::CPair *pPartPair = m_headersByPart.Lookup(pPart);
    if (pPartPair == NULL)
    {
        // add to m_headersByPart
        CAutoPtr<CCodeTypedElement> spNewElem( new CCodeTypedElement );
        if (spNewElem != NULL)
        {
            *spNewElem = *spElem;
            spNewElem->SetFlags(spNewElem->GetFlags() & ~(CODEFLAG_IN | CODEFLAG_OUT));
            if (m_headersByPart.SetAt(pPart, spNewElem) != NULL)
            {
                spNewElem.Detach();
            }
            else
            {
                EmitErrorHr(E_OUTOFMEMORY);
                return E_OUTOFMEMORY;
            }
        }
    }

    // finally, add header to function
    CCodeTypedElement *pHeader = GetHeaderByName(pCodeFunc, spElem->GetName());
    if (pHeader != NULL)
    {
        pHeader->AddFlags(spElem->GetFlags());
    }
    else
    {
        if (pCodeFunc->AddHeader(spElem) != NULL)
        {
            spElem.Detach();
        }
        else
        {
            EmitErrorHr(E_OUTOFMEMORY);
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}

CCodeTypedElement * CCodeTypeBuilder::GetParameterByName(CCodeFunction *pCodeFunc, const CStringW& strName)
{
    ATLASSERT( pCodeFunc != NULL );

    POSITION pos = pCodeFunc->GetFirstElement();
    while (pos != NULL)
    {
        CCodeTypedElement *p = pCodeFunc->GetNextElement(pos);
        ATLASSERT( p != NULL );

        if (p->GetName() == strName)
        {
            return p;
        }
    }

    return NULL;
}

HRESULT CCodeTypeBuilder::GetNameFromSchemaElement(CXSDElement *pXSDElement, CStringW& strName)
{
    ATLASSERT( pXSDElement != NULL );

    HRESULT hr = S_OK;
    XSDELEMENT_TYPE elementType = pXSDElement->GetElementType();
    if (elementType == XSD_COMPLEXTYPE)
    {
        strName = static_cast<CComplexType *>(pXSDElement)->GetName();
    }
    else if (elementType == XSD_SIMPLETYPE)
    {
        strName = static_cast<CSimpleType *>(pXSDElement)->GetName();
    }
    else
    {
        ATLASSERT( FALSE );
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CCodeTypeBuilder::CheckAndAddHeader(
    CCodeFunction *pCodeFunc,
    CAutoPtr<CCodeTypedElement>& spElem,
    DWORD dwFlags,
    CWSDLMessagePart *pPart)
{
    HRESULT hr = CheckDuplicateHeaders(pCodeFunc, spElem, dwFlags);
    if (SUCCEEDED(hr))
    {
        hr = AddHeaderToFunction(pCodeFunc, spElem, pPart);
    }

    return hr;
}

HRESULT CCodeTypeBuilder::CheckDocLiteralNamespace(
    CCodeFunction *pCodeFunc,
    CXSDElement *pXSDElement,
    DWORD dwFlags,
    DWORD dwCallFlags)
{
    ATLASSERT( pCodeFunc != NULL );
    ATLASSERT( pXSDElement != NULL );

    if ((dwFlags & CODEFLAG_IN) || (dwFlags & CODEFLAG_HEADER) || (pCodeFunc->GetNamespace().GetLength() == 0))
    {
        if ((pCodeFunc->GetNamespace().GetLength() == 0) ||
            (dwCallFlags & CODEFLAG_PID) ||
            (dwFlags & CODEFLAG_HEADER))
        {
            pCodeFunc->SetNamespace(pXSDElement->GetParentSchema()->GetTargetNamespace());
        }
        else if ((dwCallFlags & CODEFLAG_PAD)==0)
        {
            EmitError(IDS_SDL_INTERNAL);
            return E_FAIL;
        }
    }
    else
    {
        ATLASSERT( (dwFlags & CODEFLAG_OUT) || (dwCallFlags & CODEFLAG_PAD) );

        if (wcscmp(CA2W(pCodeFunc->GetNamespace()), pXSDElement->GetParentSchema()->GetTargetNamespace()))
        {
            EmitFileError(IDS_SDL_IO_DIFF_NAMESPACES, pXSDElement, 0);
            return E_FAIL;
        }
    }
    return S_OK;
}

HRESULT CCodeTypeBuilder::CreateSafeNames(CCodeElementContainer *pElem)
{
    ATLASSERT( pElem != NULL );

    // map for names in current scope
    NAMEMAP scopedMap;

    HRESULT hr = CreateSafeCppName(pElem->GetSafeName(), pElem->GetName());
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CheckGlobalNameMap(pElem->GetSafeName(), true);

    // add it to the map of the current scope
    if (scopedMap.SetAt(pElem->GetSafeName(), 0) == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // iterate over the elements
    // NOTE: there is a precondition that the SafeCodeTypeName has already been set properly
    POSITION pos = pElem->GetFirstElement();
    while (pos != NULL)
    {
        CCodeTypedElement *p = pElem->GetNextElement(pos);
        ATLASSERT( p != NULL );

        hr = CreateSafeCppName(p->GetSafeName(), p->GetName());
        if (SUCCEEDED(hr))
        {
            hr = CheckGlobalNameMap(p->GetSafeName(), false);
            if (SUCCEEDED(hr))
            {
                hr = CheckNameMap(scopedMap, p->GetSafeName(), true);
            }
        }

        if (FAILED(hr))
        {
            break;
        }
    }

    return hr;
}

HRESULT CCodeTypeBuilder::CheckGlobalNameMap(CStringA& strName, bool bAddToMap)
{
    return CheckNameMap(m_globalNameMap, strName, bAddToMap);
}

HRESULT CCodeTypeBuilder::CheckNameMap(NAMEMAP &map, CStringA& strName, bool bAddToMap)
{
    // name exists in global scope, rename it
    if (map.Lookup(strName) != NULL)
    {
        // first, add a prefix (_)
        CStringA strNewName("_");
        strNewName.Append(strName);
        strName = strNewName;
    }
    if (map.Lookup(strName) != NULL)
    {
        // then, if that is not enough, use m_nNameCounter
        char szBuf[512];

        strName.Append(_itoa(m_nNameCounter, szBuf, 10));
        m_nNameCounter++;
    }

    if (bAddToMap != false)
    {
        // add it to the map of the global scope
        // REVIEW: necessary?
        if (map.SetAt(strName, 0) == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}
