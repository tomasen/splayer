//
// Emit.cpp
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "Emit.h"
#include "errordefs.h"
#include "XMLDocument.h"

void EmitError(UINT uID, ...)
{
	va_list arglist;
	va_start(arglist, uID);
	g_Emit.EmitError(uID, arglist);
	va_end(arglist);
}

void EmitErrorHr(HRESULT hr)
{
	g_Emit.EmitErrorHr(hr);
}

void EmitWarning(UINT uID, ...)
{
	va_list arglist;
	va_start(arglist, uID);
	g_Emit.EmitWarning(uID, arglist);
	va_end(arglist);
}

void Emit(UINT uID, ...)
{
	va_list arglist;
	va_start(arglist, uID);
	g_Emit.Emit(uID, arglist);
	va_end(arglist);
}

bool SetEmitWarnings(bool bWarn)
{
	return g_Emit.SetEmitWarnings(bWarn);
}

void EmitCmdLineError(UINT uID, ...)
{
	va_list arglist;
	va_start(arglist, uID);
	g_Emit.EmitCmdLineError(uID, arglist);
	va_end(arglist);
}

void EmitCmdLineWarning(UINT uID, ...)
{
	va_list arglist;
	va_start(arglist, uID);
	g_Emit.EmitCmdLineWarning(uID, arglist);
	va_end(arglist);
}

void EmitFileWarning(UINT uID, LPCWSTR wszFile, int nLine, int nCol, UINT uIDExtra, ...)
{
	va_list arglist;
	va_start(arglist, uIDExtra);
	g_Emit.EmitFileWarning(uID, wszFile, nLine, nCol, uIDExtra, arglist);
	va_end(arglist);
}

void EmitFileError(UINT uID, LPCWSTR wszFile, int nLine, int nCol, UINT uIDExtra, ...)
{
	va_list arglist;
	va_start(arglist, uIDExtra);
	g_Emit.EmitFileError(uID, wszFile, nLine, nCol, uIDExtra, arglist);
	va_end(arglist);
}

void EmitFileWarning(UINT uID, CXMLElement *pElem, UINT uIDExtra, ...)
{
	va_list arglist;
	va_start(arglist, uIDExtra);
	g_Emit.EmitFileWarning(uID, pElem->GetParentDocument()->GetDocumentUri(), pElem->GetLineNumber(), pElem->GetColumnNumber(), uIDExtra, arglist);
	va_end(arglist);
}

void EmitFileError(UINT uID, CXMLElement *pElem, UINT uIDExtra, ...)
{
	va_list arglist;
	va_start(arglist, uIDExtra);
	g_Emit.EmitFileError(uID, pElem->GetParentDocument()->GetDocumentUri(), pElem->GetLineNumber(), pElem->GetColumnNumber(), uIDExtra, arglist);
	va_end(arglist);
}