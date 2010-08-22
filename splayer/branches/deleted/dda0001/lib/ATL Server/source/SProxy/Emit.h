//
// Emit.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"
class CXMLElement;

void EmitError(UINT uID, ...);
void EmitErrorHr(HRESULT hr);

void EmitWarning(UINT uID, ...);

void Emit(UINT uID, ...);

bool SetEmitWarnings(bool bWarn);

void EmitCmdLineError(UINT uID, ...);
void EmitCmdLineWarning(UINT uID, ...);

void EmitFileWarning(UINT uID, LPCWSTR wszFile, int nLine, int nCol, UINT uIDExtra, ...);
void EmitFileError(UINT uID, LPCWSTR wszFile, int nLine, int nCol, UINT uIDExtra, ...);

void EmitFileWarning(UINT uID, CXMLElement *pElem, UINT uIDExtra, ...);
void EmitFileError(UINT uID, CXMLElement *pElem, UINT uIDExtra, ...);