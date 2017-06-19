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



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CReflectorWnd

BOOL CReflectorWnd::Create(const CRect& rect, HWND hWndParent)
{
	// make sure the default window class is registered
	VERIFY(AfxDeferRegisterClass(AFX_WNDOLECONTROL_REG));
	return CreateEx(0, AFX_WNDOLECONTROL, NULL,
		WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top, hWndParent, 0);
}

void CReflectorWnd::PostNcDestroy()
{
	if (m_pCtrl != NULL)
		m_pCtrl->OnReflectorDestroyed();
	delete this;
}

LRESULT CReflectorWnd::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	case WM_NOTIFY:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
	case WM_COMPAREITEM:
	case WM_HSCROLL:
	case WM_VSCROLL:
	case WM_PARENTNOTIFY:
		if (m_pCtrl != NULL)
			return m_pCtrl->SendMessage(OCM__BASE + uMsg, wParam, lParam);
		break;

	case WM_SETFOCUS:
		if (m_pCtrl != NULL)
		{
			m_pCtrl->SetFocus();
			return 0;
		}
		break;
	}

	return CWnd::WindowProc(uMsg, wParam, lParam);
}

void CReflectorWnd::SetControl(COleControl* pCtrl)
{
	m_pCtrl = pCtrl;
}

LRESULT CParkingWnd::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndSource = NULL;

	switch (uMsg)
	{
	case WM_COMMAND:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
	case WM_HSCROLL:
	case WM_VSCROLL:
		hWndSource = (HWND)lParam;
		break;

	case WM_NOTIFY:
		hWndSource = ((NMHDR*)lParam)->hwndFrom;
		break;

	case WM_DRAWITEM:
		hWndSource = ((DRAWITEMSTRUCT*)lParam)->hwndItem;
		break;

	case WM_MEASUREITEM:
		m_idMap.Lookup((void*)(DWORD_PTR)HIWORD(wParam), (void*&)hWndSource);
		break;

	case WM_DELETEITEM:
		hWndSource = ((DELETEITEMSTRUCT*)lParam)->hwndItem;
		break;

	case WM_COMPAREITEM:
		hWndSource = ((COMPAREITEMSTRUCT*)lParam)->hwndItem;
		break;

	case WM_PARENTNOTIFY:
		switch (LOWORD(wParam))
		{
		case WM_CREATE:
			m_idMap.SetAt((void*)(DWORD_PTR)HIWORD(wParam), (HWND)lParam);
			hWndSource = (HWND)lParam;
			break;

		case WM_DESTROY:
			m_idMap.RemoveKey((void*)(DWORD_PTR)HIWORD(wParam));
			hWndSource = (HWND)lParam;
			break;

		default:
			m_idMap.Lookup((void*)(DWORD_PTR)HIWORD(wParam), (void*&)hWndSource);
			break;
		}
	}

	if (hWndSource != NULL)
		return ::SendMessage(hWndSource, OCM__BASE + uMsg, wParam, lParam);
	else
		return CWnd::WindowProc(uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// Force any extra compiler-generated code into AFX_INIT_SEG

