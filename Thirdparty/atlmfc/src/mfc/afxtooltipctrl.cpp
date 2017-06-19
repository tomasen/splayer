// This MFC Library source code supports the Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"
#include "multimon.h"
#include "afxtooltipctrl.h"

#include "afxtoolbarimages.h"
#include "afxtoolbar.h"
#include "afxtoolbarbutton.h"
#include "afxdrawmanager.h"
#include "afxvisualmanager.h"
#include "afxoutlookbarpane.h"
#include "afxribbonbutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef TTS_BALLOON
#define TTS_BALLOON 0x40
#endif

#ifndef TTM_SETTITLE
#define TTM_SETTITLE (WM_USER + 32)
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCToolTipCtrl

IMPLEMENT_DYNCREATE(CMFCToolTipCtrl, CToolTipCtrl)

CMFCToolTipCtrl::CMFCToolTipCtrl(CMFCToolTipInfo* pParams/* = NULL*/)
{
	SetParams(pParams);

	m_pToolBar = NULL;
	m_pToolBarImages = NULL;
	m_pHotButton = NULL;
	m_sizeImage = CSize(0, 0);
	m_ptMargin = CPoint(0, 0);
	m_ptLocation = CPoint(-1, -1);
	m_pRibbonButton = NULL;
	m_nRibbonImageType = 0;
	m_nFixedWidthRegular = 0;
	m_nFixedWidthWithImage = 0;
}

CMFCToolTipCtrl::~CMFCToolTipCtrl()
{
}

BEGIN_MESSAGE_MAP(CMFCToolTipCtrl, CToolTipCtrl)
	//{{AFX_MSG_MAP(CMFCToolTipCtrl)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(TTN_SHOW, &CMFCToolTipCtrl::OnShow)
	ON_NOTIFY_REFLECT(TTN_POP, &CMFCToolTipCtrl::OnPop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolTipCtrl message handlers

void CMFCToolTipCtrl::OnPaint()
{
	if (m_Params.m_bBalloonTooltip)
	{
		CToolTipCtrl::OnPaint();
		return;
	}

	CPaintDC dcPaint(this); // device context for painting

	CMemDC memDC (dcPaint, this);
	CDC* pDC = &memDC.GetDC ();


	CRect rect;
	GetClientRect(rect);

	CRect rectMargin;
	GetMargin(rectMargin);

	CRect rectText = rect;

	rectText.DeflateRect(rectMargin);
	rectText.DeflateRect(m_ptMargin.x, m_ptMargin.y);

	COLORREF clrLine = m_Params.m_clrBorder == (COLORREF)-1 ? ::GetSysColor(COLOR_INFOTEXT) : m_Params.m_clrBorder;
	COLORREF clrText = m_Params.m_clrText == (COLORREF)-1 ? ::GetSysColor(COLOR_INFOTEXT) : m_Params.m_clrText;

	// Fill background:
	OnFillBackground(pDC, rect, clrText, clrLine);

	CPen penLine(PS_SOLID, 1, clrLine);
	CPen* pOldPen = pDC->SelectObject(&penLine);

	// Draw border:
	OnDrawBorder(pDC, rect, clrLine);

	// Draw icon:
	if (m_sizeImage != CSize(0, 0) && m_Params.m_bDrawIcon)
	{
		CRect rectImage = rectText;
		rectImage.right = rectImage.left + m_sizeImage.cx;
		rectImage.bottom = rectImage.top + m_sizeImage.cy;

		OnDrawIcon(pDC, rectImage);

		rectText.left += m_sizeImage.cx + m_ptMargin.x;
	}

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(clrText);

	// Draw label:
	int nTextHeight = OnDrawLabel(pDC, rectText, FALSE).cy;

	// Draw separator + description:
	if (!m_strDescription.IsEmpty() && m_Params.m_bDrawDescription)
	{
		CRect rectDescr = rectText;
		rectDescr.top += nTextHeight + 3 * m_ptMargin.y / 2;

		if (m_Params.m_bDrawSeparator)
		{
			OnDrawSeparator(pDC, rectDescr.left, rectDescr.right, rectDescr.top - m_ptMargin.y / 2);
		}

		OnDrawDescription(pDC, rectDescr, FALSE);
	}

	pDC->SelectObject(pOldPen);
}

void CMFCToolTipCtrl::OnShow(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	if (m_Params.m_bVislManagerTheme)
	{
		CMFCVisualManager::GetInstance()->GetToolTipInfo(m_Params);
		m_Params.m_bVislManagerTheme = TRUE;
	}

	if (m_Params.m_bBalloonTooltip)
	{
		return;
	}

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);

	GetHotButton();

	m_sizeImage = m_Params.m_bDrawIcon ? GetIconSize() : CSize(0, 0);
	m_ptMargin = m_Params.m_bRoundedCorners ? CPoint(6, 4) : CPoint(4, 2);

	CRect rectMargin;
	GetMargin(rectMargin);

	CRect rectText;
	GetClientRect(rectText);

	CClientDC dc(this);
	CSize sizeText = OnDrawLabel(&dc, rectText, TRUE);

	int cx = sizeText.cx;
	int cy = sizeText.cy;

	CSize sizeDescr(0, 0);

	if (!m_Params.m_bDrawDescription || m_strDescription.IsEmpty())
	{
		cy = max(cy, m_sizeImage.cy);
	}
	else
	{
		sizeDescr = OnDrawDescription(&dc, rectText, TRUE);

		cy += sizeDescr.cy + 2 * m_ptMargin.y;
		cx = max(cx, sizeDescr.cx);
		cy = max(cy, m_sizeImage.cy);
	}

	if (m_sizeImage.cx > 0 && m_Params.m_bDrawIcon)
	{
		cx += m_sizeImage.cx + m_ptMargin.x;
	}

	cx += 2 * m_ptMargin.x;
	cy += 2 * m_ptMargin.y;

	const int nFixedWidth = GetFixedWidth();
	if (nFixedWidth > 0 && sizeDescr != CSize(0, 0))
	{
		cx = max(cx, nFixedWidth);
	}

	CRect rectWindow;
	GetWindowRect(rectWindow);

	int x = rectWindow.left;
	int y = rectWindow.top;

	if (m_ptLocation != CPoint(-1, -1))
	{
		x = m_ptLocation.x;
		y = m_ptLocation.y;

		*pResult = 1;
	}

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromPoint(rectWindow.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	int nBottom = max(ptCursor.y + cy + ::GetSystemMetrics(SM_CYCURSOR), y + cy + 2);
	if (nBottom > rectScreen.bottom)
	{
		y = ptCursor.y - cy - 1;
		*pResult = 1;
	}

	if (x + cx + 2 > rectScreen.right)
	{
		if ((*pResult) == 1) // Y has been changed
		{
			x = ptCursor.x - cx - 1;
		}
		else
		{
			x = rectScreen.right - cx - 1;
			*pResult = 1;
		}
	}

	if ((*pResult) == 1)
	{
		SetWindowPos(NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if (m_Params.m_bRoundedCorners)
	{
		CRgn rgn;
		rgn.CreateRoundRectRgn(0, 0, cx + 1, cy + 1, 4, 4);

		SetWindowRgn(rgn, FALSE);
	}
}

void CMFCToolTipCtrl::OnPop(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	m_pHotButton = NULL;
	m_pToolBarImages = NULL;
	m_strDescription.Empty();
	m_ptLocation = CPoint(-1, -1);
	m_pRibbonButton = NULL;
	m_nRibbonImageType = 0;
	*pResult = 0;
}

void CMFCToolTipCtrl::OnFillBackground(CDC* pDC, CRect rect, COLORREF& /*clrText*/, COLORREF& /*clrLine*/)
{
	ASSERT_VALID(pDC);

	if (m_Params.m_clrFill == (COLORREF)-1)
	{
		::FillRect(pDC->GetSafeHdc(), rect, ::GetSysColorBrush(COLOR_INFOBK));
	}
	else
	{
		if (m_Params.m_clrFillGradient == (COLORREF)-1)
		{
			CBrush br(m_Params.m_clrFill);
			pDC->FillRect(rect, &br);
		}
		else
		{
			CDrawingManager dm(*pDC);

			dm.FillGradient2(rect, m_Params.m_clrFillGradient, m_Params.m_clrFill, m_Params.m_nGradientAngle == -1 ? 90 : m_Params.m_nGradientAngle);
		}
	}
}

CSize CMFCToolTipCtrl::GetIconSize()
{

	if (m_pRibbonButton != NULL)
	{
		ASSERT_VALID(m_pRibbonButton);

		if (!m_pRibbonButton->IsDrawTooltipImage())
		{
			return CSize(0, 0);
		}

		if (m_pRibbonButton->GetIcon() != NULL)
		{
			m_nRibbonImageType = m_pRibbonButton->IsLargeImage() ? CMFCRibbonButton::RibbonImageLarge : CMFCRibbonButton::RibbonImageSmall;

			return m_pRibbonButton->GetImageSize((CMFCRibbonBaseElement::RibbonImageType)m_nRibbonImageType);
		}

		CSize sizeLarge(0, 0);

		if (m_pRibbonButton->IsLargeImage() && m_pRibbonButton->GetImageIndex(TRUE) >= 0)
		{
			sizeLarge = m_pRibbonButton->GetImageSize(CMFCRibbonButton::RibbonImageLarge);
		}

		if (sizeLarge != CSize(0, 0))
		{
			m_nRibbonImageType = CMFCRibbonButton::RibbonImageLarge;
			return sizeLarge;
		}

		CSize sizeSmall(0, 0);

		if (m_pRibbonButton->GetImageIndex(FALSE) >= 0)
		{
			sizeSmall = m_pRibbonButton->GetImageSize(CMFCRibbonButton::RibbonImageSmall);
		}

		m_nRibbonImageType = CMFCRibbonButton::RibbonImageSmall;

		return sizeSmall;
	}

	if (m_pHotButton == NULL || m_pToolBarImages == NULL || m_pToolBarImages->GetCount() == 0)
	{
		return CSize(0, 0);
	}

	ASSERT_VALID(m_pHotButton);

	return m_pHotButton->GetImage() >= 0 ? m_pToolBarImages->GetImageSize() : CSize(0, 0);
}

void CMFCToolTipCtrl::OnDrawBorder(CDC* pDC, CRect rect, COLORREF clrLine)
{
	ASSERT_VALID(pDC);

	if (!m_Params.m_bRoundedCorners)
	{
		pDC->Draw3dRect(rect, clrLine, clrLine);
		return;
	}

	const int nOffset = 2;

	pDC->MoveTo(rect.left + nOffset, rect.top);
	pDC->LineTo(rect.right - nOffset - 1, rect.top);

	pDC->LineTo(rect.right - 1, rect.top + nOffset);
	pDC->LineTo(rect.right - 1, rect.bottom - 1 - nOffset);

	pDC->LineTo(rect.right - nOffset - 1, rect.bottom - 1);
	pDC->LineTo(rect.left + nOffset, rect.bottom - 1);

	pDC->LineTo(rect.left, rect.bottom - 1 - nOffset);
	pDC->LineTo(rect.left, rect.top + nOffset);

	pDC->LineTo(rect.left + nOffset, rect.top);
}

BOOL CMFCToolTipCtrl::OnDrawIcon(CDC* pDC, CRect rectImage)
{
	ASSERT_VALID(pDC);

	if (m_pRibbonButton != NULL)
	{
		ASSERT_VALID(m_pRibbonButton);

		BOOL bIsDisabled = m_pRibbonButton->m_bIsDisabled;
		m_pRibbonButton->m_bIsDisabled = FALSE;

		m_pRibbonButton->DrawImage(pDC, (CMFCRibbonBaseElement::RibbonImageType) m_nRibbonImageType, rectImage);

		m_pRibbonButton->m_bIsDisabled = bIsDisabled;
		return TRUE;
	}

	if (m_pHotButton == NULL || m_pToolBarImages == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pHotButton);
	ASSERT_VALID(m_pToolBarImages);

	CAfxDrawState ds;
	m_pToolBarImages->PrepareDrawImage(ds);

	UINT nSaveStyle = m_pHotButton->m_nStyle;
	BOOL bSaveText = m_pHotButton->m_bText;
	BOOL bSaveImage = m_pHotButton->m_bImage;

	BOOL bSaveLargeIcons = CMFCToolBar::m_bLargeIcons;
	CMFCToolBar::m_bLargeIcons = FALSE;

	m_pHotButton->m_bText = FALSE;
	m_pHotButton->m_bImage = TRUE;

	m_pHotButton->m_nStyle = 0;

	m_pHotButton->CMFCToolBarButton::OnDraw(pDC, rectImage, m_pToolBarImages);

	m_pHotButton->m_nStyle = nSaveStyle;
	m_pHotButton->m_bText = bSaveText;
	m_pHotButton->m_bImage = bSaveImage;

	CMFCToolBar::m_bLargeIcons = bSaveLargeIcons;

	m_pToolBarImages->EndDrawImage(ds);
	return TRUE;
}

CSize CMFCToolTipCtrl::OnDrawLabel(CDC* pDC, CRect rect, BOOL bCalcOnly)
{
	ASSERT_VALID(pDC);

	CSize sizeText(0, 0);

	CString strText;
	GetWindowText(strText);

	strText.Replace(_T("\t"), _T("    "));

	BOOL bDrawDescr = m_Params.m_bDrawDescription && !m_strDescription.IsEmpty();

	CFont* pOldFont = (CFont*) pDC->SelectObject(m_Params.m_bBoldLabel && bDrawDescr ? &afxGlobalData.fontBold : &afxGlobalData.fontTooltip);

	if (strText.Find(_T('\n')) >= 0) // Multi-line text
	{
		UINT nFormat = DT_NOPREFIX;
		if (bCalcOnly)
		{
			nFormat |= DT_CALCRECT;
		}

		if (m_pRibbonButton != NULL)
		{
			nFormat |= DT_NOPREFIX;
		}

		int nHeight = pDC->DrawText(strText, rect, nFormat);
		sizeText = CSize(rect.Width(), nHeight);
	}
	else
	{
		if (bCalcOnly)
		{
			sizeText = pDC->GetTextExtent(strText);
		}
		else
		{
			UINT nFormat = DT_LEFT | DT_NOCLIP | DT_SINGLELINE;

			if (!bDrawDescr)
			{
				nFormat |= DT_VCENTER;
			}

			if (m_pRibbonButton != NULL)
			{
				nFormat |= DT_NOPREFIX;
			}

			sizeText.cy = pDC->DrawText(strText, rect, nFormat);
			sizeText.cx = rect.Width();
		}
	}

	pDC->SelectObject(pOldFont);

	return sizeText;
}

CSize CMFCToolTipCtrl::OnDrawDescription(CDC* pDC, CRect rect, BOOL bCalcOnly)
{
	ASSERT_VALID(pDC);

	CSize sizeText(0, 0);

	if (!m_Params.m_bDrawDescription)
	{
		return sizeText;
	}

	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontTooltip);
	int nFixedWidth = GetFixedWidth ();

	if (nFixedWidth > 0 && m_sizeImage.cx <= 32)
	{
		rect.right = rect.left + nFixedWidth;

		if (m_sizeImage.cx > 0 && m_Params.m_bDrawIcon)
		{
			rect.right -= m_sizeImage.cx + m_ptMargin.x;
		}
	}
	else
	{
		rect.right = rect.left + m_Params.m_nMaxDescrWidth;
	}

	UINT nFormat = DT_WORDBREAK;
	if (bCalcOnly)
	{
		nFormat |= DT_CALCRECT;
	}

	int nDescrHeight = pDC->DrawText(m_strDescription, rect, nFormat);
	pDC->SelectObject(pOldFont);

	return CSize(rect.Width(), nDescrHeight);
}

void CMFCToolTipCtrl::OnDrawSeparator(CDC* pDC, int x1, int x2, int y)
{
	ASSERT_VALID(pDC);

	pDC->MoveTo(x1, y);
	pDC->LineTo(x2, y);
}

int CMFCToolTipCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolTipCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, CWnd::FromHandlePermanent(lpCreateStruct->hwndParent));

	if (m_pToolBar != NULL && m_pToolBar->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPane)))
	{
		m_pToolBar = NULL;
	}

	ModifyStyle(WS_BORDER, 0);

	if (m_Params.m_bBalloonTooltip)
	{
		ModifyStyle(0, TTS_BALLOON);
	}

	return 0;
}

BOOL CMFCToolTipCtrl::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
    GetClientRect (rect);

    COLORREF clrDummy;
    OnFillBackground (pDC, rect, clrDummy, clrDummy);

	return TRUE;
}

void CMFCToolTipCtrl::SetParams(CMFCToolTipInfo* pParams)
{
	ASSERT_VALID(this);

	if (pParams == NULL)
	{
		CMFCToolTipInfo paramsDefault;
		m_Params = paramsDefault;
	}
	else
	{
		m_Params = *pParams;
	}
}

void CMFCToolTipCtrl::SetDescription(const CString strDescription)
{
	ASSERT_VALID(this);

	GetHotButton();
	m_strDescription = strDescription;

	m_strDescription.Replace(_T("\t"), _T("    "));
}

void CMFCToolTipCtrl::GetHotButton()
{
	m_pHotButton = NULL;
	m_pToolBarImages = NULL;

	if (m_pRibbonButton != NULL)
	{
		return;
	}

	if (m_pToolBar != NULL)
	{
		CPoint ptToolBar;

		::GetCursorPos(&ptToolBar);
		m_pToolBar->ScreenToClient(&ptToolBar);

		m_pHotButton = m_pToolBar->GetButton(m_pToolBar->HitTest(ptToolBar));

		if (m_pHotButton != NULL)
		{
			if (m_pToolBar->IsLocked())
			{
				m_pToolBarImages = m_pToolBar->GetLockedMenuImages();
			}
			else
			{
				if (m_pHotButton->m_bUserButton)
				{
					m_pToolBarImages = CMFCToolBar::GetUserImages();
				}
				else
				{
					m_pToolBarImages = CMFCToolBar::GetMenuImages();
					if (m_pToolBarImages == NULL || m_pToolBarImages->GetCount() <= 0)
					{
						m_pToolBarImages = CMFCToolBar::GetImages();
					}
				}
			}
		}
	}
}

void CMFCToolTipCtrl::SetLocation(CPoint pt)
{
	ASSERT_VALID(this);
	m_ptLocation = pt;
}

void CMFCToolTipCtrl::SetHotRibbonButton(CMFCRibbonButton* pRibbonButton)
{
	ASSERT_VALID(this);
	m_pRibbonButton = pRibbonButton;
}

int CMFCToolTipCtrl::GetFixedWidth()
{
	ASSERT_VALID(this);

	if (m_sizeImage.cx <= (int)(afxGlobalData.GetRibbonImageScale() * 32))
	{
		return m_nFixedWidthRegular;
	}
	else
	{
		return m_nFixedWidthWithImage;
	}
}



