//
// AttributeParser.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
#include "AttributeParser.h"
#include "Attribute.h"

TAG_METHOD_IMPL(CAttributeParser, OnAnnotation)
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

TAG_METHOD_IMPL(CAttributeParser, OnSimpleType)
{
	TRACE_PARSE_ENTRY();

	return SkipElement();
}

ATTR_METHOD_IMPL(CAttributeParser, OnForm)
{
	TRACE_PARSE_ENTRY();

	CAttribute * pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		return pCurr->SetAttributeForm(wszValue, cchValue);
	}

	Emit(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CAttributeParser, OnRef)
{
	TRACE_PARSE_ENTRY();

	CAttribute * pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		return pCurr->SetRef(wszValue, cchValue);
	}

	Emit(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CAttributeParser, OnName)
{
	TRACE_PARSE_ENTRY();

	CAttribute * pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		return pCurr->SetName(wszValue, cchValue);
	}

	Emit(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CAttributeParser, OnType)
{
	TRACE_PARSE_ENTRY();

	CAttribute * pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		return pCurr->SetType(wszValue, cchValue);
	}

	Emit(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CAttributeParser, OnUse)
{
	TRACE_PARSE_ENTRY();

	CAttribute * pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		return pCurr->SetAttributeUse(wszValue, cchValue);
	}

	Emit(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CAttributeParser, OnValue)
{
	TRACE_PARSE_ENTRY();

	CAttribute * pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		return pCurr->SetValue(wszValue, cchValue);
	}

	Emit(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CAttributeParser, OnID)
{
	TRACE_PARSE_ENTRY();

	CAttribute * pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		return pCurr->SetID(wszValue, cchValue);
	}

	Emit(IDS_SDL_INTERNAL);

	return E_FAIL;
}

ATTR_METHOD_IMPL(CAttributeParser, OnArrayType)
{
	TRACE_PARSE_ENTRY();

	CAttribute *pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		return pCurr->SetArrayType(wszValue, cchValue);
	}

	Emit(IDS_SDL_INTERNAL);

	return E_FAIL;
}

HRESULT CAttributeParser::ValidateElement()
{
	// emit a warning when we encounter an attribute that is 
	// not an arrayType attribute
	CAttribute *pCurr = GetAttribute();
	if (pCurr != NULL)
	{
		if (!pCurr->GetArrayType().GetLength())
		{
			EmitFileWarning(IDS_SDL_NO_ATTRIBUTES, pCurr, 0);
		}
	}
	
	return S_OK;
}