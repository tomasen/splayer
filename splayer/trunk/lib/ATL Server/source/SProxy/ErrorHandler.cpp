//
// ErrorHandler.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "ErrorHandler.h"
#include "Emit.h"
#include "resource.h"
#include "Parser.h"

HRESULT __stdcall CErrorHandler::QueryInterface(REFIID riid, void **ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}

	if (InlineIsEqualGUID(riid, __uuidof(ISAXErrorHandler)) ||
		InlineIsEqualGUID(riid, __uuidof(IUnknown)))
	{
		*ppv = static_cast<IUnknown*>(this);
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG __stdcall CErrorHandler::AddRef()
{
	return 1;
}

ULONG __stdcall CErrorHandler::Release()
{
	return 1;
}

HRESULT __stdcall CErrorHandler::error( 
	ISAXLocator *pLocator,
	const wchar_t *wszErrorMessage,
	HRESULT hrErrorCode)
{
	int nLine;
	int nCol;

	pLocator->getLineNumber(&nLine);
	pLocator->getColumnNumber(&nCol);
	ATLTRACE( _T("error@(%d, %d): %ws\n"), nLine, nCol, wszErrorMessage );

	EmitFileError(IDS_SDL_PARSE_ERROR, m_wszLocation, nLine, nCol, IDS_SDL_PARSE, wszErrorMessage);
	g_ParserList.RemoveAll();

	return E_FAIL;
}
        
HRESULT __stdcall CErrorHandler::fatalError(
	ISAXLocator  *pLocator,
	const wchar_t *wszErrorMessage,
	HRESULT hrErrorCode)
{
	int nLine;
	int nCol;

	pLocator->getLineNumber(&nLine);
	pLocator->getColumnNumber(&nCol);
	ATLTRACE( _T("fatalError@(%d, %d): %ws\n"), nLine, nCol, wszErrorMessage );

	EmitFileError(IDS_SDL_PARSE_ERROR, m_wszLocation, nLine, nCol, IDS_SDL_PARSE, wszErrorMessage);
	g_ParserList.RemoveAll();

	return E_FAIL;
}

HRESULT __stdcall CErrorHandler::ignorableWarning(
	ISAXLocator  *pLocator,
	const wchar_t *wszErrorMessage,
	HRESULT hrErrorCode)
{

	int nLine;
	int nCol;

	pLocator->getLineNumber(&nLine);
	pLocator->getColumnNumber(&nCol);
	
	ATLTRACE( _T("ignorableWarning@(%d, %d): %ws\n"), nLine, nCol, wszErrorMessage );

	EmitFileWarning(IDS_SDL_PARSE_WARNING, m_wszLocation, nLine, nCol, IDS_SDL_PARSE, wszErrorMessage);

	return S_OK;
}