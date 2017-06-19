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
#include "afxacceleratorkeyassignctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCAcceleratorKeyAssignCtrl

CMFCAcceleratorKeyAssignCtrl::CMFCAcceleratorKeyAssignCtrl() : m_Helper(&m_Accel)
{
	m_bIsDefined = FALSE;
	m_bIsFocused = FALSE;

	ResetKey();
}

CMFCAcceleratorKeyAssignCtrl::~CMFCAcceleratorKeyAssignCtrl()
{
}

BEGIN_MESSAGE_MAP(CMFCAcceleratorKeyAssignCtrl, CEdit)
	//{{AFX_MSG_MAP(CMFCAcceleratorKeyAssignCtrl)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCAcceleratorKeyAssignCtrl message handlers

BOOL CMFCAcceleratorKeyAssignCtrl::PreTranslateMessage(MSG* pMsg)
{
	BOOL bIsKeyPressed = FALSE;
	BOOL bIsFirstPress = FALSE;

	switch(pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		m_bIsFocused = TRUE;
		SetFocus();
		return TRUE;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		bIsKeyPressed = TRUE;
		bIsFirstPress = (pMsg->lParam &(1 << 30)) != 0;
		// To the key processing....

	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			if (bIsKeyPressed && m_bIsDefined && !bIsFirstPress)
			{
				ResetKey();
			}

			if (!m_bIsDefined)
			{
				switch(pMsg->wParam)
				{
				case VK_SHIFT:
					SetAccelFlag(FSHIFT, bIsKeyPressed);
					break;

				case VK_CONTROL:
					SetAccelFlag(FCONTROL, bIsKeyPressed);
					break;

				case VK_MENU:
					SetAccelFlag(FALT, bIsKeyPressed);
					break;

				default:
					if (!m_bIsFocused)
					{
						m_bIsFocused = TRUE;
						return TRUE;
					}

					m_Accel.key = (WORD) pMsg->wParam;

					if (bIsKeyPressed)
					{
						m_bIsDefined = TRUE;
						SetAccelFlag(FVIRTKEY, TRUE);
					}
				}
			}

			BOOL bDefaultProcess = FALSE;

			if ((m_Accel.fVirt & FCONTROL) == 0 && (m_Accel.fVirt & FSHIFT) == 0 && (m_Accel.fVirt & FALT) == 0 && (m_Accel.fVirt & FVIRTKEY))
			{
				switch(m_Accel.key)
				{
				case VK_ESCAPE:
					ResetKey();
					return TRUE;

				case VK_TAB:
				case VK_PROCESSKEY:
					bDefaultProcess = TRUE;
				}
			}

			if (!bDefaultProcess)
			{
				CString strKbd;
				m_Helper.Format(strKbd);

				SetWindowText(strKbd);
				return TRUE;
			}

			ResetKey();
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void CMFCAcceleratorKeyAssignCtrl::ResetKey()
{
	memset(&m_Accel, 0, sizeof(ACCEL));
	m_bIsDefined = FALSE;

	if (m_hWnd != NULL)
	{
		SetWindowText(_T(""));
	}
}

void CMFCAcceleratorKeyAssignCtrl::SetAccelFlag(BYTE bFlag, BOOL bOn)
{
	if (bOn)
	{
		m_Accel.fVirt |= bFlag;
	}
	else
	{
		m_Accel.fVirt &= ~bFlag;
	}
}

void CMFCAcceleratorKeyAssignCtrl::OnKillFocus(CWnd* pNewWnd)
{
	m_bIsFocused = FALSE;
	CEdit::OnKillFocus(pNewWnd);
}


