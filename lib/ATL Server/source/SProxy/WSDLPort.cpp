//
// WSDLPort.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "WSDLPort.h"
#include "WSDLDocument.h"

#include "Attribute.h"
#include "Content.h"
#include "Element.h"
#include "ComplexType.h"

CWSDLBinding * CWSDLPort::GetBinding()
{
	if (m_pBinding != NULL)
	{
		return m_pBinding;
	}

	CXMLDocument *pDoc = GetParentDocument();
	if (pDoc != NULL)
	{
		CStringW strUri;
		if (SUCCEEDED(GetNamespaceUri(m_binding.GetPrefix(), strUri)))
		{
			if (strUri == pDoc->GetTargetNamespace())
			{
				if (pDoc->GetDocumentType() == WSDLDOC)
				{
					CWSDLDocument *pWSDLDoc = static_cast<CWSDLDocument *>(pDoc);
					m_pBinding = pWSDLDoc->GetBinding(m_binding.GetName());
					if (m_pBinding == NULL)
					{
						EmitFileError(IDS_SDL_UNRESOLVED_ELEM2, const_cast<CWSDLPort *>(this), 0,
							"binding", strUri, m_binding.GetName());
					}
				}
			}
			else
			{
				EmitFileError(IDS_SDL_UNRESOLVED_ELEM2, const_cast<CWSDLPort *>(this), 0, "binding", strUri, m_binding.GetName());
			}
		}
		else
		{
			EmitFileError(IDS_SDL_UNRESOLVED_NAMESPACE, const_cast<CWSDLPort *>(this), 0, m_binding.GetPrefix());
		}
	}

	return m_pBinding;
}