//
// ErrorHandler.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"

class CErrorHandler : public ISAXErrorHandler
{
private:
	const wchar_t * m_wszLocation;
public:
	CErrorHandler()
		:m_wszLocation(NULL)
	{
	}

	void SetLocation(const wchar_t * wszLocation)
	{
		m_wszLocation = wszLocation;
	}

	virtual ~CErrorHandler() {}

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	HRESULT __stdcall error( 
		ISAXLocator *pLocator,
		const wchar_t *wszErrorMessage,
		HRESULT hrErrorCode);

	HRESULT __stdcall fatalError(
		ISAXLocator  *pLocator,
		const wchar_t *wszErrorMessage,
		HRESULT hrErrorCode);

	HRESULT __stdcall ignorableWarning(
		ISAXLocator  *pLocator,
		const wchar_t *wszErrorMessage,
		HRESULT hrErrorCode);
};
