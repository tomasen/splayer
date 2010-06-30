//
// WSDLMessagePart.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "WSDLMessagePart.h"
#include "WSDLDocument.h"

#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

//
// TODO: import stuff, errors
//
HRESULT CWSDLMessagePart::GetElement(CElement **ppElement)
{
	ATLASSERT( ppElement != NULL );
	*ppElement = NULL;
	if (m_pElement != NULL)
	{
		*ppElement = m_pElement;
		return S_OK;
	}

	CXMLDocument *pDoc = GetParentDocument();
	if (pDoc != NULL)
	{
		CStringW strUri;
		if (m_element.GetName().GetLength() != 0)
		{
			if (SUCCEEDED(GetNamespaceUri(m_element.GetPrefix(), strUri)))
			{
				//
				// TODO: fix this part to handle imports (includes?)
				//
//					if (strUri == pDoc->GetTargetNamespace())
//					{
					if (pDoc->GetDocumentType() == WSDLDOC)
					{
						CWSDLDocument *pWSDLDoc = static_cast<CWSDLDocument *>(pDoc);
						m_pElement = pWSDLDoc->GetElement(m_element.GetName(), strUri);
					}
//					}
			}
		}
	}

//	return m_pElement;
	*ppElement = m_pElement;
	if (m_pElement != NULL)
	{
		return S_OK;
	}
	return E_FAIL;
}

//
// TODO: import stuff, errors
//
HRESULT CWSDLMessagePart::GetType(CXSDElement **ppElement, XSDTYPE *pXSD)
{
	ATLASSERT( pXSD != NULL );
	ATLASSERT( ppElement != NULL );

	*pXSD = XSDTYPE_ERR;
	*ppElement = NULL;

	if (m_pXSDElement != NULL)
	{
		// we've already processed it as a simpleType or complexType
		*ppElement = m_pXSDElement;
		return S_OK;
	}

	// REVIEW (jasjitg): maybe cache the XSDTYPE, as well... ?

	CXMLDocument *pDoc = GetParentDocument();
	CStringW strUri;
	if (pDoc != NULL)
	{
		if (m_type.GetName().GetLength() != 0)
		{
			if (SUCCEEDED(GetNamespaceUri(m_type.GetPrefix(), strUri)))
			{
				// check CLR/FX-specific namespaces
				if ((strUri == FX_TYPES_NAMESPACEW) ||
					(strUri == CLR_TYPES_NAMESPACEW))
				{
					// treat their custom types as strings
					*pXSD = XSDTYPE_STRING;
					EmitFileWarning(IDS_SDL_CUSTOM_TYPE, const_cast<CWSDLMessagePart *>(this), 0,
						strUri, m_type.GetName());
				}
				else
				{
					//
					// TODO: fix this part to handle imports (includes?)
					//
					if (S_FALSE == GetXSDType(strUri, m_type.GetName(), pXSD))
					{
						if (pDoc->GetDocumentType() == WSDLDOC)
						{
							CWSDLDocument *pWSDLDoc = static_cast<CWSDLDocument *>(pDoc);

							// complexType or simpleType
							m_pXSDElement = pWSDLDoc->GetComplexType(m_type.GetName(), strUri);
							if (m_pXSDElement == NULL)
							{
								m_pXSDElement = pWSDLDoc->GetSimpleType(m_type.GetName(), strUri);
							}
						}
					}
				}
			}
		}
	}

	*ppElement = m_pXSDElement;
	if ((m_pXSDElement != NULL) || (*pXSD != XSDTYPE_ERR))
	{
		return S_OK;
	}

	if (strUri.GetLength() == 0)
	{
		EmitFileError(IDS_SDL_UNRESOLVED_NAMESPACE, const_cast<CWSDLMessagePart *>(this), 0, m_type.GetPrefix());
	}
	else
	{
		EmitFileError(IDS_SDL_UNRESOLVED_ELEM2, const_cast<CWSDLMessagePart *>(this), 0, "part", strUri, m_type.GetName());
	}
	return E_FAIL;
}