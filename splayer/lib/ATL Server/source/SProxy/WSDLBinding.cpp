//
// WSDLBinding.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "WSDLBinding.h"
#include "WSDLPortType.h"
#include "WSDLDocument.h"
#include "Emit.h"
#include "resource.h"

#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

CWSDLPortType * CWSDLBinding::GetPortType()
{
	if (m_pPortType != NULL)
	{
		return m_pPortType;
	}

	CXMLDocument *pDoc = GetParentDocument();
	if (pDoc != NULL)
	{
		CStringW strUri;
		if (SUCCEEDED(GetNamespaceUri(m_type.GetPrefix(), strUri)))
		{
			if (strUri == pDoc->GetTargetNamespace())
			{
				if (pDoc->GetDocumentType() == WSDLDOC)
				{
					CWSDLDocument *pWSDLDoc = static_cast<CWSDLDocument *>(pDoc);
					m_pPortType = pWSDLDoc->GetPortType(m_type.GetName());
				}
				if (m_pPortType == NULL)
				{
					EmitFileError(IDS_SDL_UNRESOLVED_ELEM2, const_cast<CWSDLBinding *>(this), 0, 
						"portType", strUri, m_type.GetName());
				}
			}
		}
		else
		{
			EmitFileError(IDS_SDL_UNRESOLVED_NAMESPACE, const_cast<CWSDLBinding *>(this), 0, m_type.GetPrefix());
		}
	}

	return m_pPortType;
}