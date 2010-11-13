//
// WSDLType.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "XMLElement.h"
#include "Schema.h"

class CWSDLType : public CXMLElement
{
private:

	CStringW m_strDocumentation;

	CAtlPtrList<CSchema *> m_schemas;

public:

	inline CSchema * AddSchema(CSchema * p = NULL)
	{
		CAutoPtr<CSchema> spOut;
		if (p== NULL)
		{
			spOut.Attach( new CSchema );
			p = spOut;
		}
		
		if (p != NULL)
		{
			if (m_schemas.AddTail(p) != NULL)
			{
				spOut.Detach();
				return p;
			}
		}

		return NULL;
	}

	inline POSITION GetFirstSchema()
	{
		return m_schemas.GetHeadPosition();
	}

	inline CSchema * GetNextSchema(POSITION& pos)
	{
		return m_schemas.GetNext(pos);
	}
};