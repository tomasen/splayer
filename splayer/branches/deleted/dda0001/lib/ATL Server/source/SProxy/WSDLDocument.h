//
// WSDLDocument.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "XMLDocument.h"
#include "XMLElement.h"
#include "WSDLType.h"
#include "WSDLMessage.h"
#include "WSDLPortType.h"
#include "WSDLBinding.h"
#include "WSDLService.h"
#include "Emit.h"
#include "resource.h"

#include "Attribute.h"
#include "Content.h"
#include "SimpleType.h"
#include "ComplexType.h"
#include "Element.h"
//#include "UnsupportedElement.h"

class CWSDLDocument : public CXMLDocument
{
private:

	typedef CAtlPtrMap<CStringW, CWSDLMessage *, CStringRefElementTraits<CStringW> > MESSAGEMAP;
	typedef CAtlPtrMap<CStringW, CWSDLPortType *, CStringRefElementTraits<CStringW> > PORTMAP;
	typedef CAtlPtrMap<CStringW, CWSDLBinding *, CStringRefElementTraits<CStringW> > BINDINGMAP;
	typedef CAtlPtrMap<CStringW, CWSDLService *, CStringRefElementTraits<CStringW> > SERVICEMAP;

	CAtlPtrList<CWSDLType *> m_types;

	MESSAGEMAP m_messages;
	PORTMAP m_portTypes;
	BINDINGMAP m_bindings;
	SERVICEMAP m_services;

	CStringW m_strName;

public:

	inline CWSDLDocument()
	{
		SetDocumentType(WSDLDOC);
	}

	inline HRESULT SetName(const wchar_t *wszName, int cchName)
	{
		if (!wszName)
		{
			return E_FAIL;
		}

		m_strName.SetString(wszName, cchName);

		return S_OK;
	}

	inline HRESULT SetName(const CStringW& strName)
	{
		m_strName = strName;

		return S_OK;
	}

	inline const CStringW& GetName()
	{
		return m_strName;
	}

	inline CWSDLType * AddType(CWSDLType * p = NULL)
	{
		CAutoPtr<CWSDLType> spOut;
		if (p == NULL)
		{
			spOut.Attach( new CWSDLType );
			p = spOut;
		}

		if (p != NULL)
		{
			if (m_types.AddTail(p) != NULL)
			{
				spOut.Detach();
				return p;
			}
		}

		return NULL;
	}

	inline POSITION GetFirstType()
	{
		return m_types.GetHeadPosition();
	}

	inline CWSDLType * GetNextType(POSITION& pos)
	{
		return m_types.GetNext(pos);
	}

	inline CComplexType * GetComplexType(const CStringW& strName, const CStringW& strUri)
	{
		if (strUri != GetTargetNamespace())
		{
			//
			// TODO: call import handler
			//
		}

		POSITION pos = GetFirstType();
		while (pos != NULL)
		{
			CWSDLType *pType = GetNextType(pos);
			if (pType != NULL)
			{
				POSITION schemaPos = pType->GetFirstSchema();
				while (schemaPos != NULL)
				{
					CSchema *pSchema = pType->GetNextSchema(schemaPos);
					if (pSchema != NULL)
					{
						CComplexType *pRet = pSchema->GetComplexType(strUri, strName);
						if (pRet != NULL)
						{
							return pRet;
						}
					}
				}
			}
		}

		return NULL;
	}

	inline CElement * GetElement(const CStringW& strName, const CStringW& strUri)
	{
		if (strUri != GetTargetNamespace())
		{
			//
			// TODO: call import handler
			//
		}

		POSITION pos = GetFirstType();
		while (pos != NULL)
		{
			CWSDLType *pType = GetNextType(pos);
			if (pType != NULL)
			{
				POSITION schemaPos = pType->GetFirstSchema();
				while (schemaPos != NULL)
				{
					CSchema *pSchema = pType->GetNextSchema(schemaPos);
					if (pSchema != NULL)
					{
						CElement *pRet = pSchema->GetElement(strUri, strName);
						if (pRet != NULL)
						{
							return pRet;
						}
					}
				}
			}
		}

		return NULL;
	}

	inline CSimpleType * GetSimpleType(const CStringW& strName, const CStringW& strUri)
	{
		if (strUri != GetTargetNamespace())
		{
			//
			// TODO: call import handler
			//
		}

		POSITION pos = GetFirstType();
		while (pos != NULL)
		{
			CWSDLType *pType = GetNextType(pos);
			if (pType != NULL)
			{
				POSITION schemaPos = pType->GetFirstSchema();
				while (schemaPos != NULL)
				{
					CSchema *pSchema = pType->GetNextSchema(schemaPos);
					if (pSchema != NULL)
					{
						CSimpleType *pRet = pSchema->GetSimpleType(strUri, strName);
						if (pRet != NULL)
						{
							return pRet;
						}
					}
				}
			}
		}

		return NULL;
	}

//	inline CUnsupportedElement * GetUnsupportedElement(const CStringW& strName, const CStringW& strUri)
//	{
//		if (strUri != GetTargetNamespace())
//		{
//			//
//			// TODO: call import handler
//			//
//		}
//
//		POSITION pos = GetFirstType();
//		while (pos != NULL)
//		{
//			CWSDLType *pType = GetNextType(pos);
//			if (pType != NULL)
//			{
//				POSITION schemaPos = pType->GetFirstSchema();
//				while (schemaPos != NULL)
//				{
//					CSchema *pSchema = pType->GetNextSchema(schemaPos);
//					if (pSchema != NULL)
//					{
//						CUnsupportedElement *pRet = pSchema->GetUnsupportedElement(strUri, strName);
//						if (pRet != NULL)
//						{
//							return pRet;
//						}
//					}
//				}
//			}
//		}
//
////		EmitError(IDS_SDL_UNRESOLVED_ELEM, strUri, strName);
//		return NULL;
//	}

	inline CWSDLMessage * AddMessage(CWSDLMessage * p)
	{
		if (p != NULL)
		{
			if (p->GetName().GetLength() != 0)
			{
				if (m_messages.SetAt(p->GetName(), p) != NULL)
				{
					return p;
				}
			}
		}

		return NULL;
	}

	inline CWSDLMessage * GetMessage(const CStringW& strName)
	{
		const MESSAGEMAP::CPair * p = m_messages.Lookup(strName);
		if (p != NULL)
		{
			return p->m_value;
		}

		return NULL;
	}

	inline CWSDLPortType * AddPortType(CWSDLPortType * p)
	{
		if (p != NULL)
		{
			if (p->GetName().GetLength() != 0)
			{
				if (m_portTypes.SetAt(p->GetName(), p) != NULL)
				{
					return p;
				}
			}
		}

		return NULL;
	}

	inline CWSDLPortType * GetPortType(const CStringW& strName)
	{
		const PORTMAP::CPair * p = m_portTypes.Lookup(strName);
		if (p != NULL)
		{
			return p->m_value;
		}

		return NULL;
	}

	inline CWSDLBinding * AddBinding(CWSDLBinding * p)
	{
		if (p != NULL)
		{
			if (p->GetName().GetLength() != 0)
			{
				if (m_bindings.SetAt(p->GetName(), p) != NULL)
				{
					return p;
				}
			}
		}

		return NULL;
	}

	inline POSITION GetFirstBinding()
	{
		return m_bindings.GetStartPosition();
	}

	inline CWSDLBinding * GetNextBinding(POSITION &pos)
	{
		return m_bindings.GetNextValue(pos);
	}

	inline CWSDLBinding * GetBinding(const CStringW &strName)
	{
		const BINDINGMAP::CPair * p = m_bindings.Lookup(strName);
		if (p != NULL)
		{
			return p->m_value;
		}

		return NULL;
	}

	inline CWSDLService * AddService(CWSDLService * p)
	{
		if (p != NULL)
		{
			if (p->GetName().GetLength() != 0)
			{
				if (m_services.SetAt(p->GetName(), p) != NULL)
				{
					return p;
				}
			}
		}

		return NULL;
	}

	inline POSITION GetFirstService()
	{
		return m_services.GetStartPosition();
	}

	inline CWSDLService * GetNextService(POSITION &pos)
	{
		return m_services.GetNextValue(pos);
	}

	inline CWSDLService * GetService(const CStringW &strName)
	{
		const SERVICEMAP::CPair * p = m_services.Lookup(strName);
		if (p != NULL)
		{
			return p->m_value;
		}

		return NULL;
	}
};