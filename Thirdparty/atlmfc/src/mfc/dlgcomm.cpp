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
#include <dlgs.h>



#define new DEBUG_NEW

UINT _afxMsgLBSELCHANGE = 0;
UINT _afxMsgSHAREVI = 0;
UINT _afxMsgFILEOK = 0;
UINT _afxMsgCOLOROK = 0;
UINT _afxMsgHELP = 0;
UINT _afxMsgSETRGB = 0;

BEGIN_MESSAGE_MAP(CCommonDialog, CDialog)
	//{{AFX_MSG_MAP(CCommonDialog)
	ON_WM_HELPINFO()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

UINT_PTR CALLBACK
_AfxCommDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == NULL)
		return 0;

	// If a m_pAlternateWndInit is set, this is the first time we've
	// seend the hWnd given to us in the callback. We'll need to
	// subclass it and reset the m_pAlternateWndInit flag.

	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	if (pThreadState->m_pAlternateWndInit != NULL && CWnd::FromHandlePermanent(hWnd) == NULL)
	{
		ASSERT_KINDOF(CFileDialog, pThreadState->m_pAlternateWndInit);
		pThreadState->m_pAlternateWndInit->SubclassWindow(hWnd);
		pThreadState->m_pAlternateWndInit = NULL;
	}

	if (message == WM_INITDIALOG)
	{
		_afxMsgLBSELCHANGE = ::RegisterWindowMessage(LBSELCHSTRING);
		_afxMsgSHAREVI = ::RegisterWindowMessage(SHAREVISTRING);
		_afxMsgFILEOK = ::RegisterWindowMessage(FILEOKSTRING);
		_afxMsgCOLOROK = ::RegisterWindowMessage(COLOROKSTRING);
		_afxMsgHELP = ::RegisterWindowMessage(HELPMSGSTRING);
		_afxMsgSETRGB = ::RegisterWindowMessage(SETRGBSTRING);
		return (UINT)AfxDlgProc(hWnd, message, wParam, lParam);
	}

	if (message == _afxMsgHELP ||
	   (message == WM_COMMAND && LOWORD(wParam) == pshHelp))
	{
		// just translate the message into the AFX standard help command.
		SendMessage(hWnd, WM_COMMAND, ID_HELP, 0);
		return 1;
	}

	if (message < 0xC000)
	{
		// not a ::RegisterWindowMessage message
		return 0;
	}

	// assume it is already wired up to a permanent one
	CDialog* pDlg = (CDialog*)CWnd::FromHandlePermanent(hWnd);
	if(!pDlg)
	{
		// someone else sent us a registered message. Ignore it
		return 0;
	}
	ASSERT_KINDOF(CDialog, pDlg);

	if (pDlg->IsKindOf(RUNTIME_CLASS(CFileDialog)))
	{
		// If we're exploring then we are not interested in the Registered messages
		if (((CFileDialog*)pDlg)->m_ofn.Flags & OFN_EXPLORER)
			return 0;
	}

	// RegisterWindowMessage - does not copy to lastState buffer, so
	// CWnd::GetCurrentMessage and CWnd::Default will NOT work
	// while in these handlers

	// Dispatch special commdlg messages through our virtual callbacks
	if (message == _afxMsgSHAREVI)
	{
		ASSERT_KINDOF(CFileDialog, pDlg);
		return ((CFileDialog*)pDlg)->OnShareViolation((LPCTSTR)lParam);
	}
	else if (message == _afxMsgFILEOK)
	{
		ASSERT_KINDOF(CFileDialog, pDlg);

		((CFileDialog*)pDlg)->m_pofnTemp = (OPENFILENAME*)lParam;
		BOOL bResult = ((CFileDialog*)pDlg)->OnFileNameOK();
		((CFileDialog*)pDlg)->m_pofnTemp = NULL;

		return bResult;
	}
	else if (message == _afxMsgLBSELCHANGE)
	{
		ASSERT_KINDOF(CFileDialog, pDlg);
		((CFileDialog*)pDlg)->OnLBSelChangedNotify((UINT)wParam, LOWORD(lParam),
				HIWORD(lParam));
		return 0;
	}
	else if (message == _afxMsgCOLOROK)
	{
		ASSERT_KINDOF(CColorDialog, pDlg);
		return ((CColorDialog*)pDlg)->OnColorOK();
	}
	else if (message == _afxMsgSETRGB)
	{
		// nothing to do here, since this is a SendMessage
		return 0;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
// CCommonDialog - common dialog helper class

void CCommonDialog::OnOK()
{
	ASSERT_VALID(this);

	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	// Common dialogs do not require ::EndDialog
	Default();
}

void CCommonDialog::OnCancel()
{
	ASSERT_VALID(this);

	// Common dialogs do not require ::EndDialog
	Default();
}

BOOL CCommonDialog::OnHelpInfo(HELPINFO*)
{
   //IA64: Couldn't find docs for retval of OnHelpInfo
	return (Default() != 0);
}

void CCommonDialog::OnPaint()
{
	// Don't call CDialog::OnPaint
	Default();
}

IMPLEMENT_DYNAMIC(CCommonDialog, CDialog)

////////////////////////////////////////////////////////////////////////////
