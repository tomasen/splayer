//
// WSDLOperationIOParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "Util.h"

#include "WSDLOperationIOParser.h"

#include "WSDLPortTypeIO.h"
#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"
#include "Emit.h"
#include "resource.h"

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnSoapBody)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeIO * pCurr = GetIO();
	if (pCurr != NULL)
	{
		CSoapBody *pBody = pCurr->AddSoapBody();
		if (pBody != NULL)
		{
			SetXMLElementInfo(pBody, pCurr, GetLocator());

			const wchar_t *wszValue;
			int cchValue;
			if (S_OK == GetAttribute(pAttributes, L"use", sizeof("use")-1, &wszValue, &cchValue))
			{
				if (S_OK != pBody->SetUse(wszValue, cchValue))
				{
					EmitInvalidValue("use", wszValue);
				}
			}
			if (S_OK == GetAttribute(pAttributes, L"parts", sizeof("parts")-1, &wszValue, &cchValue))
			{
				pBody->SetParts(wszValue, cchValue);
			}

			if (S_OK == GetAttribute(pAttributes, L"encodingStyle", sizeof("encodingStyle")-1, &wszValue, &cchValue))
			{
				pBody->SetEncodingStyle(wszValue, cchValue);
			}

			if (S_OK == GetAttribute(pAttributes, L"namespace", sizeof("namespace")-1, &wszValue, &cchValue))
			{
				pBody->SetNamespace(wszValue, cchValue);
			}
		}

		return SkipElement();
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnSoapHeader)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeIO * pCurr = GetIO();
	if (pCurr != NULL)
	{
		CSoapHeader *pElem = pCurr->AddSoapHeader();
		if (pElem != NULL)
		{
			SetXMLElementInfo(pElem, pCurr, GetLocator());
			const wchar_t *wszValue = NULL;
			int cchValue = 0;

			// message, part, and use are all required attributes
			if (S_OK == GetAttribute(pAttributes, L"message", sizeof("message")-1, &wszValue, &cchValue))
			{
				pElem->SetMessage(wszValue, cchValue);
				if (S_OK == GetAttribute(pAttributes, L"part", sizeof("part")-1, &wszValue, &cchValue))
				{
					pElem->SetParts(wszValue, cchValue);
					if (S_OK == GetAttribute(pAttributes, L"use", sizeof("use")-1, &wszValue, &cchValue))
					{
						if (S_OK == pElem->SetUse(wszValue, cchValue))
						{
							// encodingStyle, namespace, and required are optional attributes
							if (S_OK == GetAttribute(pAttributes, L"encodingStyle", sizeof("encodingStyle")-1, &wszValue, &cchValue))
							{
								pElem->SetEncodingStyle(wszValue, cchValue);
							}
							if (S_OK == GetAttribute(pAttributes, L"namespace", sizeof("namespace")-1, &wszValue, &cchValue))
							{
								pElem->SetNamespace(wszValue, cchValue);
							}
							if (S_OK == GetAttribute(pAttributes, L"required", 
								sizeof("required")-1, &wszValue, &cchValue, WSDL_NAMESPACEW, sizeof(WSDL_NAMESPACEA)-1))
							{
								bool bVal;
								if (S_OK == GetBooleanValue(&bVal, wszValue, cchValue))
								{
									pElem->SetRequired(bVal);
								}
								else
								{
									EmitInvalidValue("required", wszValue);
								}
							}

							return SkipElement();
						}
						EmitInvalidValue("use", wszValue);
					}
					else
					{
						OnMissingAttribute(TRUE, L"use", sizeof("use")-1, L"", 0);
					}
				}
				else
				{
					OnMissingAttribute(TRUE, L"part", sizeof("part")-1, L"", 0);
				}
			}
			else
			{
				OnMissingAttribute(TRUE, L"message", sizeof("message")-1, L"", 0);
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnSoapHeaderFault)
{
	TRACE_PARSE_ENTRY();

	// REVIEW: output warning here (probably not necessary)?
	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnSoapFault)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeIO * pCurr = GetIO();
	if (pCurr != NULL)
	{
		CSoapFault *pElem = pCurr->AddSoapFault();
		if (pElem != NULL)
		{
			SetXMLElementInfo(pElem, pCurr, GetLocator());

			const wchar_t *wszValue = NULL;
			int cchValue = 0;

			// name and use are required attributes
			if (S_OK == GetAttribute(pAttributes, L"name", sizeof("name")-1, &wszValue, &cchValue))
			{
				pElem->SetName(wszValue, cchValue);
				if (S_OK == GetAttribute(pAttributes, L"use", sizeof("use")-1, &wszValue, &cchValue))
				{
					if (S_OK == pElem->SetUse(wszValue, cchValue))
					{
						// encodingStyle and namespace are optional attributes
						if (S_OK == GetAttribute(pAttributes, L"encodingStyle", sizeof("encodingStyle")-1, &wszValue, &cchValue))
						{
							pElem->SetEncodingStyle(wszValue, cchValue);
						}
						if (S_OK == GetAttribute(pAttributes, L"namespace", sizeof("namespace")-1, &wszValue, &cchValue))
						{
							pElem->SetNamespace(wszValue, cchValue);
						}

						return SkipElement();
					}
					EmitInvalidValue("use", wszValue);
				}
			}
		}
	}

	EmitErrorHr(E_OUTOFMEMORY);

	return E_FAIL;
}

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnMimeContent)
{
	TRACE_PARSE_ENTRY();

	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnMimeXML)
{
	TRACE_PARSE_ENTRY();

	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnMimeMultipartRelated)
{
	TRACE_PARSE_ENTRY();

	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnHttpUrlEncoded)
{
	TRACE_PARSE_ENTRY();

	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

TAG_METHOD_IMPL(CWSDLOperationIOParser, OnDocumentation)
{
	TRACE_PARSE_ENTRY();

	EmitSkip(wszNamespaceUri, wszLocalName);

	return SkipElement();
}

ATTR_METHOD_IMPL(CWSDLOperationIOParser, OnName)
{
	TRACE_PARSE_ENTRY();
	
	CWSDLPortTypeIO * pCurr = GetIO();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	return E_FAIL;
}

ATTR_METHOD_IMPL(CWSDLOperationIOParser, OnMessage)
{
	TRACE_PARSE_ENTRY();

	CWSDLPortTypeIO * pCurr = GetIO();
	if (pCurr != NULL)
	{
		return pCurr->SetMessage(wszValue, cchValue);
	}

	return E_FAIL;
}

HRESULT __stdcall CWSDLOperationIOParser::startPrefixMapping(
     const wchar_t  *wszPrefix,
     int cchPrefix,
     const wchar_t  *wszUri,
     int cchUri)
{
	CWSDLPortTypeIO * pCurr = GetIO();
	if (pCurr != NULL)
	{
		return pCurr->SetNamespaceUri(wszPrefix, cchPrefix, wszUri, cchUri);
	}
	return E_FAIL;
}

HRESULT CWSDLOperationIOParser::OnUnrecognizedTag(
	const wchar_t *wszNamespaceUri, int cchNamespaceUri,
	const wchar_t *wszLocalName, int cchLocalName,
	const wchar_t * /*wszQName*/, int /*cchQName*/,
	ISAXAttributes * /*pAttributes*/) throw()
{
	CWSDLPortTypeIO * pCurr = GetIO();
	if (pCurr != NULL)
	{
		int nLine;
		int nCol;
		GetLocator()->getLineNumber(&nLine);
		GetLocator()->getColumnNumber(&nCol);
		
		EmitFileWarning(IDS_SDL_SKIP_EXTENSIBILITY, 
			pCurr->GetParentDocument()->GetDocumentUri(), 
			nLine, 
			nCol, 
			0, 
			wszNamespaceUri,
			wszLocalName);
	}
	return SkipElement();
}