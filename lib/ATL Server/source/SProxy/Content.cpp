//
// Content.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "Content.h"
#include "Attribute.h"
#include "Element.h" // required for CComplexType::m_elements.RemoveAll()
#include "ComplexType.h"

CComplexType * CContent::AddType(CComplexType *pType)
{
	if (pType == NULL)
	{
		pType = new CComplexType;
	}
	delete m_pType;
	m_pType = pType;

	return m_pType;
}

CComplexType * CContent::GetType()
{
	return m_pType;
}

CContent::~CContent()
{
	delete m_pType;
}