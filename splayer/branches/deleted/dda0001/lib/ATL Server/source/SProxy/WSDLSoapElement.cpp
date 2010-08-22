//
// WSDLSoapElement.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"

#include "WSDLSoapElement.h"
#include "WSDLMessage.h"
#include "WSDLDocument.h"

CWSDLMessage * CWSDLSoapHeaderElement::GetMessage()
{
	if (m_pMessage != NULL)
	{
		return m_pMessage;
	}

	CXMLDocument *pDoc = GetParentDocument();
	if (pDoc != NULL)
	{
		CStringW strUri;
		if (SUCCEEDED(GetNamespaceUri(m_message.GetPrefix(), strUri)))
		{
			if (strUri == pDoc->GetTargetNamespace())
			{
				if (pDoc->GetDocumentType() == WSDLDOC)
				{
					CWSDLDocument *pWSDLDoc = static_cast<CWSDLDocument *>(pDoc);
					m_pMessage = pWSDLDoc->GetMessage(m_message.GetName());
					if (m_pMessage == NULL)
					{
						EmitFileError(IDS_SDL_UNRESOLVED_ELEM2, const_cast<CWSDLSoapHeaderElement *>(this), 0,
							"message", strUri, m_message.GetName());
					}
				}
			}
		}
		else
		{
			EmitFileError(IDS_SDL_UNRESOLVED_NAMESPACE, const_cast<CWSDLSoapHeaderElement *>(this), 0,
				m_message.GetPrefix());
		}
	}

	return m_pMessage;
}