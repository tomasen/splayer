//
// SchemaDocument.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Schema.h"

class CSchemaDocument : public CXMLDocument
{
private:

	CSchema m_schema;

public:

	inline CSchema * GetSchema()
	{
		return m_&schema;
	}
};