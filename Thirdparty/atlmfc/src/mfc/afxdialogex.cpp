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

#include "afxdialogex.h"
#include "afxpopupmenu.h"
#include "afxtoolbarmenubutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDialogEx, CDialog)

/////////////////////////////////////////////////////////////////////////////
// CDialogEx dialog

#pragma warning(disable : 4355)

CDialogEx::CDialogEx() : m_Impl(*this)
{
	CommonConstruct();
}

CDialogEx::CDialogEx(UINT nIDTemplate, CWnd *pParent/*= NULL*/) : CDialog(nIDTemplate, pParent), m_Impl(*this)
{
	CommonConstruct();
}

CDialogEx::CDialogEx(LPCTSTR lpszTemplateName, CWnd *pParentWnd/*= NULL*/) : CDialog(lpszTemplateName, pParentWnd), m_Impl(*this)
{
	CommonConstruct();
}

#pragma warning(default : 4355)

void CDialogEx::CommonConstruct()
{
	m_hBkgrBitmap = NULL;
	m_sizeBkgrBitmap = CSize(0, 0);
	m_BkgrLocation = (BackgroundLocation) -1;
	m_bAutoDestroyBmp = FALSE;
}

void CDialogEx::SetBackgroundColor(COLORREF color, BOOL bRepaint)
{
	if (m_brBkgr.GetSafeHandle() != NULL)
	{
		m_brBkgr.DeleteObject();
	}

	if (color != (COLORREF)-1)
	{
		m_brBkgr.CreateSolidBrush(color);
	}

	if (bRepaint && GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
}

void CDialogEx::SetBackgroundImage(HBITMAP hBitmap, BackgroundLocation location, BOOL bAutoDestroy, BOOL bRepaint)
{
	if (m_bAutoDestroyBmp && m_hBkgrBitmap != NULL)
	{
		::DeleteObject(m_hBkgrBitmap);
	}

	m_hBkgrBitmap = hBitmap;
	m_BkgrLocation = location;
	m_bAutoDestroyBmp = bAutoDestroy;

	if (hBitmap != NULL)
	{
		BITMAP bmp;
		::GetObject(hBitmap, sizeof(BITMAP), (LPVOID) &bmp);

		m_sizeBkgrBitmap = CSize(bmp.bmWidth, bmp.bmHeight);
	}
	else
	{
		m_sizeBkgrBitmap = CSize(0, 0);
	}

	if (bRepaint && GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
}

BOOL CDialogEx::SetBackgroundImage(UINT uiBmpResId, BackgroundLocation location, BOOL bRepaint)
{
	HBITMAP hBitmap = NULL;

	if (uiBmpResId != 0)
	{
		hBitmap = ::LoadBitmap(AfxFindResourceHandle(MAKEINTRESOURCE(uiBmpResId), RT_BITMAP), 
			MAKEINTRESOURCE(uiBmpResId));
		if (hBitmap == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}

	SetBackgroundImage(hBitmap, location, TRUE /* Autodestroy */, bRepaint);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDialogEx, CDialog)
	//{{AFX_MSG_MAP(CDialogEx)
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogEx message handlers

void CDialogEx::OnActivate(UINT nState, CWnd *pWndOther, BOOL /*bMinimized*/)
{
	m_Impl.OnActivate(nState, pWndOther);
}

BOOL CDialogEx::OnNcActivate(BOOL bActive)
{
	m_Impl.OnNcActivate(bActive);

	// Do not call the base class because it will call Default()
	// and we may have changed bActive.
	return(BOOL) DefWindowProc(WM_NCACTIVATE, bActive, 0L);
}

BOOL CDialogEx::OnEraseBkgnd(CDC* pDC)
{
	if (m_brBkgr.GetSafeHandle() == NULL && m_hBkgrBitmap == NULL)
	{
		return CDialog::OnEraseBkgnd(pDC);
	}

	ASSERT_VALID(pDC);

	CRect rectClient;
	GetClientRect(rectClient);

	if (m_BkgrLocation != BACKGR_TILE || m_hBkgrBitmap == NULL)
	{
		if (m_brBkgr.GetSafeHandle() != NULL)
		{
			pDC->FillRect(rectClient, &m_brBkgr);
		}
		else
		{
			CDialog::OnEraseBkgnd(pDC);
		}
	}

	if (m_hBkgrBitmap == NULL)
	{
		return TRUE;
	}

	ASSERT(m_sizeBkgrBitmap != CSize(0, 0));

	if (m_BkgrLocation != BACKGR_TILE)
	{
		CPoint ptImage = rectClient.TopLeft();

		switch (m_BkgrLocation)
		{
		case BACKGR_TOPLEFT:
			break;

		case BACKGR_TOPRIGHT:
			ptImage.x = rectClient.right - m_sizeBkgrBitmap.cx;
			break;

		case BACKGR_BOTTOMLEFT:
			ptImage.y = rectClient.bottom - m_sizeBkgrBitmap.cy;
			break;

		case BACKGR_BOTTOMRIGHT:
			ptImage.x = rectClient.right - m_sizeBkgrBitmap.cx;
			ptImage.y = rectClient.bottom - m_sizeBkgrBitmap.cy;
			break;
		}

		pDC->DrawState(ptImage, m_sizeBkgrBitmap, m_hBkgrBitmap, DSS_NORMAL);
	}
	else
	{
		// Tile background image:
		for (int x = rectClient.left; x < rectClient.Width(); x += m_sizeBkgrBitmap.cx)
		{
			for (int y = rectClient.top; y < rectClient.Height(); y += m_sizeBkgrBitmap.cy)
			{
				pDC->DrawState(CPoint(x, y), m_sizeBkgrBitmap, m_hBkgrBitmap, DSS_NORMAL);
			}
		}
	}

	return TRUE;
}

void CDialogEx::OnDestroy()
{
	if (m_bAutoDestroyBmp && m_hBkgrBitmap != NULL)
	{
		::DeleteObject(m_hBkgrBitmap);
		m_hBkgrBitmap = NULL;
	}

	m_Impl.OnDestroy();

	CDialog::OnDestroy();
}

HBRUSH CDialogEx::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (m_brBkgr.GetSafeHandle() != NULL || m_hBkgrBitmap != NULL)
	{
#define AFX_MAX_CLASS_NAME 255
#define AFX_STATIC_CLASS _T("Static")
#define AFX_BUTTON_CLASS _T("Button")

		if (nCtlColor == CTLCOLOR_STATIC)
		{
			TCHAR lpszClassName [AFX_MAX_CLASS_NAME + 1];

			::GetClassName(pWnd->GetSafeHwnd(), lpszClassName, AFX_MAX_CLASS_NAME);
			CString strClass = lpszClassName;

			if (strClass == AFX_STATIC_CLASS)
			{
				pDC->SetBkMode(TRANSPARENT);
				return(HBRUSH) ::GetStockObject(HOLLOW_BRUSH);
			}

			if (strClass == AFX_BUTTON_CLASS)
			{
				// if ((pWnd->GetStyle() & BS_GROUPBOX) == 0)
				{
					pDC->SetBkMode(TRANSPARENT);
				}

				return(HBRUSH) ::GetStockObject(HOLLOW_BRUSH);
			}
		}
	}

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CDialogEx::PreTranslateMessage(MSG* pMsg)
{
	if (m_Impl.PreTranslateMessage(pMsg))
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CDialogEx::SetActiveMenu(CMFCPopupMenu* pMenu)
{
	m_Impl.SetActiveMenu(pMenu);
}

BOOL CDialogEx::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (m_Impl.OnCommand(wParam, lParam))
	{
		return TRUE;
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CDialogEx::OnSysColorChange()
{
	CDialog::OnSysColorChange();

	if (AfxGetMainWnd() == this)
	{
		afxGlobalData.UpdateSysColors();
	}
}

void CDialogEx::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDialog::OnSettingChange(uFlags, lpszSection);

	if (AfxGetMainWnd() == this)
	{
		afxGlobalData.OnSettingChange();
	}
}


