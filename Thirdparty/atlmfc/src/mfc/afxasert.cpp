// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef _DEBUG   // entire file


// NOTE: in separate module so it can replaced if needed

BOOL AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine)
{
#ifndef _AFX_NO_DEBUG_CRT
	// we remove WM_QUIT because if it is in the queue then the message box
	// won't display
	MSG msg;
	BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	BOOL bResult = _CrtDbgReport(_CRT_ASSERT, lpszFileName, nLine, NULL, NULL);
	if (bQuit)
		PostQuitMessage((int)msg.wParam);
	return bResult;
#else
	// Not supported.
#error _AFX_NO_DEBUG_CRT is not supported.
#endif // _AFX_NO_DEBUG_CRT
}
#endif // _DEBUG
