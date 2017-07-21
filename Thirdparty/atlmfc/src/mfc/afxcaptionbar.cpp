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

#include "afxcaptionbar.h"
#include "afxvisualmanager.h"
#include "afxtoolbar.h"
#include "afxtrackmouse.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmdichildwndex.h"
#include "afxglobalutils.h"
#include "afxtooltipmanager.h"
#include "afxtooltipctrl.h"
#include "afxribbonres.h"

const int nMenuArrowWidth = 10;
const int nButtonHorzMargin = 10;
const int nButtonVertMargin = 5;
const int nMessageBarMargin = 4;

const int nIdToolTipClose = 1;
const int nIdToolTipText = 2;
const int nIdToolTipImage = 3;
const int nIdToolTipButton = 4;

IMPLEMENT_DYNCREATE(CMFCCaptionBar, CPane)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCCaptionBar::CMFCCaptionBar()
{
	m_pToolTip = NULL;

	m_clrBarText = (COLORREF)-1;
	m_clrBarBackground = (COLORREF)-1;
	m_clrBarBorder = (COLORREF)-1;

	m_nBorderSize = 4;
	m_nMargin = 4;
	m_nHorzElementOffset = 4;

	m_hIcon = NULL;
	m_hFont = NULL;

	m_nDefaultHeight = -1;
	m_nCurrentHeight = 0;

	m_btnAlignnment = ALIGN_LEFT;
	m_iconAlignment = ALIGN_LEFT;
	m_textAlignment = ALIGN_LEFT;

	m_bStretchImage = FALSE;

	m_bFlatBorder = FALSE;
	m_uiBtnID = 0;

	m_bIsBtnPressed = FALSE;
	m_bIsBtnForcePressed = FALSE;
	m_bIsBtnHighlighted = FALSE;

	m_bTracked = FALSE;
	m_bBtnEnabled = TRUE;
	m_bBtnHasDropDownArrow = TRUE;

	m_rectImage.SetRectEmpty();
	m_rectText.SetRectEmpty();
	m_rectDrawText.SetRectEmpty();
	m_rectButton.SetRectEmpty();
	m_bTextIsTruncated = FALSE;

	m_bIsMessageBarMode = FALSE;

	m_bIsCloseBtnPressed = FALSE;
	m_bIsCloseBtnHighlighted= FALSE;
	m_bCloseTracked = FALSE;

	m_rectClose.SetRectEmpty();
}

CMFCCaptionBar::~CMFCCaptionBar()
{
}

BOOL CMFCCaptionBar::Create(DWORD dwStyle, CWnd* pParentWnd, UINT uID, int nHeight, BOOL bIsMessageBarMode)
{
	ENSURE( AfxIsExtendedFrameClass(pParentWnd) || pParentWnd->IsKindOf(RUNTIME_CLASS(CDialog)) );

	SetPaneStyle(CBRS_ALIGN_TOP);
	m_nDefaultHeight = nHeight;

	if (bIsMessageBarMode)
	{
		m_dwStyle |= CBRS_HIDE_INPLACE;
	}

	if (!CPane::Create(NULL, dwStyle, CRect(0, 0, 0, 0), pParentWnd, uID, 0))
	{
		return FALSE;
	}

	// register with parent frames' dock manager!!!
	if (pParentWnd->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		((CFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		((CMDIFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		((COleIPFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		((COleIPFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		((CMDIChildWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(CDialog)))
	{
		afxGlobalUtils.m_bDialogApp = TRUE;
	}
	else
	{
		ASSERT(FALSE);
	}

	m_bIsMessageBarMode = bIsMessageBarMode;

	if (m_bIsMessageBarMode)
	{
		m_nBorderSize = 0;
	}

	return TRUE;
}

//{{AFX_MSG_MAP(CMFCCaptionBar)
BEGIN_MESSAGE_MAP(CMFCCaptionBar, CPane)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_SETFONT, &CMFCCaptionBar::OnSetFont)
	ON_MESSAGE(WM_GETFONT, &CMFCCaptionBar::OnGetFont)
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCCaptionBar::OnMouseLeave)
	ON_REGISTERED_MESSAGE(AFX_WM_UPDATETOOLTIPS, &CMFCCaptionBar::OnUpdateToolTips)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CMFCCaptionBar::OnNeedTipText)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

void CMFCCaptionBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

int CMFCCaptionBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_CAPTIONBAR);

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		CRect rectDummy(0, 0, 0, 0);

		m_pToolTip->SetMaxTipWidth(640);

		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipClose);
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipText);
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipImage);
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipButton);
	}

	SetWindowText(_T("Caption Bar"));
	return 0;
}

void CMFCCaptionBar::OnSize(UINT nType, int cx, int cy)
{
	CPane::OnSize(nType, cx, cy);
	RecalcLayout();
	InvalidateRect(NULL);
}

void CMFCCaptionBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	lpncsp->rgrc[0].bottom -= m_nBorderSize;
	lpncsp->rgrc[0].top  += m_nBorderSize;
}

void CMFCCaptionBar::OnPaint()
{
	CPaintDC dcPaint(this);
	CMemDC memDC(dcPaint, this);
	CDC& dc = memDC.GetDC();

	CRect rectClient;
	GetClientRect(rectClient);

	OnDrawBackground(&dc, rectClient);

	int nOldBkMode = dc.SetBkMode(TRANSPARENT);
	COLORREF clrOldText = dc.SetTextColor(m_clrBarText != (COLORREF) -1 ? m_clrBarText : CMFCVisualManager::GetInstance()->GetCaptionBarTextColor(this));

	CFont* pOldFont = dc.SelectObject(m_hFont == NULL ? &afxGlobalData.fontRegular : CFont::FromHandle(m_hFont));

	OnDrawButton(&dc, m_rectButton, m_strBtnText, m_bBtnEnabled);
	OnDrawText(&dc, m_rectDrawText, m_strText);

	if (!m_rectImage.IsRectEmpty())
	{
		OnDrawImage(&dc, m_rectImage);
	}

	if (!m_rectClose.IsRectEmpty())
	{
		COLORREF clrText = CMFCVisualManager::GetInstance()->OnFillCaptionBarButton(&dc, this, m_rectClose, m_bIsCloseBtnPressed, m_bIsCloseBtnHighlighted, FALSE, FALSE, TRUE);

		CMenuImages::IMAGE_STATE imageState;

		if (GetRValue(clrText) > 192 && GetGValue(clrText) > 192 && GetBValue(clrText) > 192)
		{
			imageState = CMenuImages::ImageWhite;
		}
		else
		{
			imageState = CMenuImages::ImageBlack;
		}

		CMenuImages::Draw(&dc, CMenuImages::IdClose, m_rectClose, imageState);

		CMFCVisualManager::GetInstance()->OnDrawCaptionBarButtonBorder(&dc, this, m_rectClose, m_bIsCloseBtnPressed, m_bIsCloseBtnHighlighted, FALSE, FALSE, TRUE);
	}

	dc.SelectObject(pOldFont);
	dc.SetTextColor(clrOldText);
	dc.SetBkMode(nOldBkMode);
}

void CMFCCaptionBar::OnNcPaint()
{
	CWindowDC dcWin(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectWindow;
	GetWindowRect(rectWindow);

	CRect rectBorder = rectWindow;

	ScreenToClient(rectWindow);

	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dcWin.ExcludeClipRect(rectClient);

	rectBorder.OffsetRect(-rectBorder.left, -rectBorder.top);

	int nTop = rectBorder.top;
	rectBorder.top = rectBorder.bottom - m_nBorderSize;
	OnDrawBorder(&dcWin, rectBorder);

	rectBorder.top = nTop;
	rectBorder.bottom = rectBorder.top + m_nBorderSize;

	OnDrawBorder(&dcWin, rectBorder);
	dcWin.SelectClipRgn(NULL);
}

void CMFCCaptionBar::OnDrawBackground(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, this, rect, rect);

	if (m_bIsMessageBarMode)
	{
		CRect rectInfo = rect;
		rectInfo.DeflateRect(nMessageBarMargin, nMessageBarMargin);
		rectInfo.right -= m_rectClose.Width();

		CMFCVisualManager::GetInstance()->OnDrawCaptionBarInfoArea(pDC, this, rectInfo);
	}
}

void CMFCCaptionBar::OnDrawBorder(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	rect.InflateRect(2, 0);

	CMFCVisualManager::GetInstance()->OnDrawCaptionBarBorder(pDC, this, rect, m_clrBarBorder, m_bFlatBorder);
}

void CMFCCaptionBar::OnDrawButton(CDC* pDC, CRect rect, const CString& strButton, BOOL bEnabled)
{
	ASSERT_VALID(pDC);

	COLORREF clrText = CMFCVisualManager::GetInstance()->OnFillCaptionBarButton(pDC, this, rect, m_bIsBtnPressed || m_bIsBtnForcePressed, m_bIsBtnHighlighted, !m_bBtnEnabled, m_bBtnHasDropDownArrow, FALSE);

	CRect rectText = rect;
	rectText.DeflateRect(m_nHorzElementOffset, 0);

	if (m_bIsMessageBarMode)
	{
		rectText.DeflateRect(nButtonHorzMargin, 0);
	}

	if (m_uiBtnID != 0 && bEnabled && m_bBtnHasDropDownArrow)
	{
		rectText.right -= nMenuArrowWidth;
	}

	COLORREF clrTextOld = (COLORREF)-1;
	if (clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor(clrText);
	}

	pDC->DrawText(strButton, rectText, DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}

	if (m_uiBtnID != 0 && bEnabled)
	{
		if (m_uiBtnID != 0 && m_bBtnHasDropDownArrow)
		{
			// Draw menu triangle:
			CRect rectArrow = rect;
			rectArrow.bottom -= m_nMargin;
			rectArrow.top = rectArrow.bottom - nMenuArrowWidth;
			rectArrow.left = rectText.right;

			int iXMiddle = rectArrow.left + rectArrow.Width() / 2;

			rectArrow.DeflateRect(0, rectArrow.Height() / 3);
			rectArrow.DeflateRect(rectArrow.Height() / 3, rectArrow.Height() / 3);
			rectArrow.left = iXMiddle - rectArrow.Height() - 1;
			rectArrow.right = iXMiddle + rectArrow.Height() + 1;

			int iHalfWidth = (rectArrow.Width() % 2 != 0) ? (rectArrow.Width() - 1) / 2 : rectArrow.Width() / 2;

			CPoint pts [3];
			pts[0].x = rectArrow.left;
			pts[0].y = rectArrow.top;
			pts[1].x = rectArrow.right;
			pts[1].y = rectArrow.top;
			pts[2].x = rectArrow.left + iHalfWidth;
			pts[2].y = rectArrow.bottom + 1;

			CBrush brArrow(pDC->GetTextColor());

			CPen* pOldPen = (CPen*) pDC->SelectStockObject(NULL_PEN);
			CBrush* pOldBrush = (CBrush*) pDC->SelectObject(&brArrow);

			pDC->SetPolyFillMode(WINDING);
			pDC->Polygon(pts, 3);

			pDC->SelectObject(pOldBrush);
			pDC->SelectObject(pOldPen);
		}

		CMFCVisualManager::GetInstance()->OnDrawCaptionBarButtonBorder(pDC, this, rect, m_bIsBtnPressed || m_bIsBtnForcePressed, m_bIsBtnHighlighted, !m_bBtnEnabled, m_bBtnHasDropDownArrow, FALSE);
	}
}

void CMFCCaptionBar::OnDrawText(CDC* pDC, CRect rect, const CString& strText)
{
	ASSERT_VALID(pDC);

	if (m_arTextParts.GetSize() == 1)
	{
		pDC->DrawText(strText, rect, DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);
		return;
	}

	int x = rect.left;

	BOOL bIsBold = FALSE;

	for (int i = 0; i < m_arTextParts.GetSize(); i++)
	{
		if (!m_arTextParts [i].IsEmpty())
		{
			CFont* pOldFont = NULL;

			if (bIsBold)
			{
				pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
			}

			CRect rectPart = rect;
			rectPart.left = x;

			CSize sizePart = pDC->GetTextExtent(m_arTextParts [i]);

			pDC->DrawText(m_arTextParts [i], rectPart, DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);

			if (pOldFont != NULL)
			{
				pDC->SelectObject(pOldFont);
			}

			x += sizePart.cx;
		}

		bIsBold = !bIsBold;
	}
}

void CMFCCaptionBar::OnDrawImage(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	if (m_hIcon != NULL)
	{
		DrawIconEx(pDC->GetSafeHdc(), rect.left, rect.top, m_hIcon, rect.Width(), rect.Height(), NULL, (HBRUSH)NULL, DI_NORMAL) ;
	}
	else if (m_Bitmap.GetCount() > 0)
	{
		CSize sizeDest;
		if (m_bStretchImage)
		{
			sizeDest = rect.Size();
		}
		else
		{
			sizeDest = m_rectImage.Size();
		}

		CAfxDrawState ds;
		m_Bitmap.PrepareDrawImage(ds, sizeDest);
		m_Bitmap.Draw(pDC, rect.left, rect.top, 0);
		m_Bitmap.EndDrawImage(ds);
	}
}

void CMFCCaptionBar::OnSysColorChange()
{
}

CSize CMFCCaptionBar::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	RecalcLayout();
	return CSize(32767, m_nCurrentHeight);
}

void CMFCCaptionBar::SetButton(LPCTSTR lpszLabel, UINT uiCmdUI, BarElementAlignment btnAlignment, BOOL bHasDropDownArrow)
{
	ENSURE(lpszLabel != NULL);

	m_strBtnText = lpszLabel;
	m_uiBtnID = uiCmdUI;
	m_btnAlignnment = btnAlignment;
	m_bBtnHasDropDownArrow = bHasDropDownArrow;

	AdjustLayout();
}

void CMFCCaptionBar::EnableButton(BOOL bEnable)
{
	m_bBtnEnabled = bEnable;

	if (GetSafeHwnd() != NULL)
	{
		CRect rectButton = m_rectButton;

		RecalcLayout();

		InvalidateRect(rectButton);
		InvalidateRect(m_rectButton);

		UpdateWindow();
	}
}

void CMFCCaptionBar::SetButtonPressed(BOOL bPresed)
{
	m_bIsBtnForcePressed = bPresed;

	if (GetSafeHwnd() != NULL)
	{
		InvalidateRect(m_rectButton);
		UpdateWindow();
	}
}

void CMFCCaptionBar::SetButtonToolTip(LPCTSTR lpszToolTip, LPCTSTR lpszDescription)
{
	m_strButtonToolTip = lpszToolTip == NULL ? _T("") : lpszToolTip;
	m_strButtonDescription = lpszDescription == NULL ? _T("") : lpszDescription;

	UpdateTooltips();
}

void CMFCCaptionBar::RemoveButton()
{
	m_strBtnText.Empty();
	AdjustLayout();
}

void CMFCCaptionBar::SetIcon(HICON hIcon, BarElementAlignment iconAlignment)
{
	m_Bitmap.Clear();

	m_hIcon = hIcon;
	m_iconAlignment = iconAlignment;

	AdjustLayout();
}

void CMFCCaptionBar::RemoveIcon()
{
	m_hIcon = NULL;
	AdjustLayout();
}

void CMFCCaptionBar::SetBitmap(HBITMAP hBitmap, COLORREF clrTransparent, BOOL bStretch, BarElementAlignment bmpAlignment)
{
	ENSURE(hBitmap != NULL);

	m_hIcon = NULL;
	m_Bitmap.Clear();

	BITMAP bmp;
	::GetObject(hBitmap, sizeof(BITMAP), (LPVOID) &bmp);

	m_Bitmap.SetImageSize(CSize(bmp.bmWidth, bmp.bmHeight));
	m_Bitmap.SetTransparentColor(clrTransparent);
	m_Bitmap.AddImage(hBitmap, clrTransparent == (COLORREF)-1);

	m_bStretchImage = bStretch;

	m_iconAlignment = bmpAlignment;

	AdjustLayout();
}

void CMFCCaptionBar::SetBitmap(UINT uiBmpResID, COLORREF clrTransparent, BOOL bStretch, BarElementAlignment bmpAlignment)
{
	m_hIcon = NULL;
	m_Bitmap.Clear();

	m_Bitmap.SetTransparentColor(clrTransparent);
	m_Bitmap.Load(uiBmpResID);
	m_Bitmap.SetSingleImage();

	m_bStretchImage = bStretch;
	m_iconAlignment = bmpAlignment;

	AdjustLayout();
}

void CMFCCaptionBar::RemoveBitmap()
{
	m_Bitmap.Clear();

	AdjustLayout();
}

void CMFCCaptionBar::SetImageToolTip(LPCTSTR lpszToolTip, LPCTSTR lpszDescription)
{
	m_strImageToolTip = lpszToolTip == NULL ? _T("") : lpszToolTip;
	m_strImageDescription = lpszDescription == NULL ? _T("") : lpszDescription;

	UpdateTooltips();
}

void CMFCCaptionBar::SetText(const CString& strText, BarElementAlignment textAlignment)
{
	BOOL bWasEmptyText = m_strText.IsEmpty();

	m_arTextParts.RemoveAll();

	int iStart = 0;

	for (int i = 0; i < strText.GetLength(); i++)
	{
		if (strText [i] == _T('\b'))
		{
			m_arTextParts.Add(strText.Mid(iStart, i - iStart));
			iStart = i + 1;
		}
	}

	m_arTextParts.Add(strText.Mid(iStart));

	m_strText = strText;
	m_textAlignment = textAlignment;

	if (m_nCurrentHeight == 0 || m_strText.IsEmpty() || bWasEmptyText)
	{
		AdjustLayout();
	}
	else
	{
		RecalcLayout();
		RedrawWindow();
	}
}

void CMFCCaptionBar::RemoveText()
{
	m_strText.Empty();
	AdjustLayout();
}

afx_msg LRESULT CMFCCaptionBar::OnSetFont(WPARAM wParam, LPARAM /*lParam*/)
{
	m_hFont = (HFONT) wParam;

	AdjustLayout();
	return 0;
}

afx_msg LRESULT CMFCCaptionBar::OnGetFont(WPARAM, LPARAM)
{
	return(LRESULT) m_hFont;
}

CMFCCaptionBar::BarElementAlignment CMFCCaptionBar::GetAlignment(BarElement elem)
{
	switch (elem)
	{
	case ELEM_BUTTON:
		return m_btnAlignnment;

	case ELEM_TEXT:
		return m_textAlignment;

	case ELEM_ICON:
		return m_iconAlignment;
	}

	ASSERT(FALSE);
	return ALIGN_INVALID;
}

void CMFCCaptionBar::RecalcLayout()
{
	CClientDC dc(NULL);

	CFont* pOldFont = dc.SelectObject(m_hFont == NULL ? &afxGlobalData.fontRegular : CFont::FromHandle(m_hFont));

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	int nTextHeight = tm.tmHeight + 2;
	CSize sizeImage = GetImageSize();

	// the height is set to the default(provided by the user in Create)
	// or calculated if it is -1
	if (m_nDefaultHeight != -1)
	{
		m_nCurrentHeight = m_nDefaultHeight;
	}
	else
	{
		if (!m_strBtnText.IsEmpty() && m_bIsMessageBarMode)
		{
			nTextHeight += 2 * nButtonVertMargin;
		}

		m_nCurrentHeight = max(nTextHeight, sizeImage.cy) + m_nMargin * 2 + m_nBorderSize;
	}

	if (m_bIsMessageBarMode)
	{
		m_nCurrentHeight += nMessageBarMargin * 2;
	}

	// for left and center alignment: icon, button, text
	// for right alignment: text, button, icon

	CRect rectClient;
	GetClientRect(rectClient);
	if (rectClient.IsRectEmpty())
	{
		return;
	}

	if (m_bIsMessageBarMode)
	{
		CSize sizeMenuImage = CMenuImages::Size();
		sizeMenuImage.cx += 2 * nMessageBarMargin;
		sizeMenuImage.cy += 2 * nMessageBarMargin;

		m_rectClose = CRect(CPoint(rectClient.right - sizeImage.cx, rectClient.top + nMessageBarMargin), sizeMenuImage);

		rectClient.DeflateRect(nMessageBarMargin, nMessageBarMargin);
		rectClient.right -= m_rectClose.Width();
	}

	BOOL bButtonLeftOfIcon = FALSE;
	BOOL bTextLeftOfButton = FALSE;
	BOOL bTextLeftOfIcon = FALSE;

	BOOL bIconCenter = FALSE;
	BOOL bButtonCenter = FALSE;
	BOOL bButtonAfterText = FALSE;
	BOOL bTextCenter = FALSE;

	// differs from the current height, because the border size is non-client area
	int nBaseLine = rectClient.CenterPoint().y;
	int nCenterOffset = rectClient.CenterPoint().x;

	int nNextXOffsetLeft  = rectClient.left + m_nMargin;
	int nNextXOffsetRight = rectClient.right - m_nMargin;
	int nNextXOffsetCenter = nCenterOffset;

	if (IsImageSet())
	{
		if (sizeImage.cy < rectClient.Height())
		{
			// center the icon if its height lesser than client area height
			m_rectImage.top = nBaseLine - sizeImage.cy / 2;
		}
		else
		{
			// otherwise, clip it from the buttom
			m_rectImage.top = rectClient.top + m_nMargin;
		}

		if (!m_bStretchImage)
		{
			m_rectImage.bottom = m_rectImage.top + sizeImage.cy;
		}
		else
		{
			m_rectImage.bottom = rectClient.bottom - m_nMargin;
		}

		switch (m_iconAlignment)
		{
		case ALIGN_LEFT:
			m_rectImage.left = nNextXOffsetLeft;
			m_rectImage.right = m_rectImage.left + sizeImage.cx;
			nNextXOffsetLeft = m_rectImage.right + m_nHorzElementOffset;
			break;

		case ALIGN_RIGHT:
			m_rectImage.left = nNextXOffsetRight - sizeImage.cx;
			m_rectImage.right = m_rectImage.left + sizeImage.cx;
			nNextXOffsetRight = m_rectImage.left - m_nHorzElementOffset;
			// only in this case button and text is at the left side of the icon
			bButtonLeftOfIcon = TRUE;
			bTextLeftOfIcon = TRUE;
			break;

		case ALIGN_CENTER:
			bIconCenter = TRUE;
			nNextXOffsetCenter -= sizeImage.cx / 2;

			if (m_btnAlignnment == ALIGN_LEFT)
			{
				bButtonLeftOfIcon = TRUE;
			}

			if (m_textAlignment == ALIGN_LEFT)
			{
				bTextLeftOfIcon = TRUE;
			}
			break;

		default:
			ASSERT(FALSE);
		}
	}

	int nButtonWidth = 0;

	if (!m_strBtnText.IsEmpty())
	{
		nButtonWidth = dc.GetTextExtent(m_strBtnText).cx + 2 * m_nHorzElementOffset;

		if (m_bIsMessageBarMode)
		{
			nButtonWidth += 2 * nButtonHorzMargin;
		}

		if (m_uiBtnID != 0 && m_bBtnEnabled && m_bBtnHasDropDownArrow)
		{
			nButtonWidth += nMenuArrowWidth;
		}

		// the button always has a height equivalent to the bar's height
		m_rectButton.top = rectClient.top;
		m_rectButton.bottom = rectClient.bottom;

		if (m_bIsMessageBarMode)
		{
			m_rectButton.DeflateRect(0, nButtonVertMargin);
		}

		switch (m_btnAlignnment)
		{
		case ALIGN_LEFT:
			if (!m_bIsMessageBarMode || m_textAlignment != ALIGN_LEFT)
			{
				m_rectButton.left = nNextXOffsetLeft;

				if (nNextXOffsetLeft == rectClient.left + m_nMargin)
				{
					m_rectButton.left = rectClient.left + m_nMargin;
				}

				m_rectButton.right = m_rectButton.left + nButtonWidth;
				nNextXOffsetLeft = m_rectButton.right + m_nHorzElementOffset;
			}
			else
			{
				bButtonAfterText = TRUE;
			}
			break;

		case ALIGN_RIGHT:
			m_rectButton.left = nNextXOffsetRight - nButtonWidth;

			if (nNextXOffsetRight == rectClient.right - m_nMargin)
			{
				m_rectButton.left = rectClient.right - nButtonWidth - m_nMargin;
			}

			m_rectButton.right = m_rectButton.left + nButtonWidth;
			nNextXOffsetRight = m_rectButton.left - m_nHorzElementOffset;
			// only in this case text at the left side of the button
			bTextLeftOfButton = TRUE;
			break;

		case ALIGN_CENTER:
			bButtonCenter = TRUE;
			nNextXOffsetCenter -= nButtonWidth / 2;

			if (m_textAlignment == ALIGN_LEFT)
			{
				bTextLeftOfButton = TRUE;
			}
			break;

		default:
			ASSERT(FALSE);
			return;
		}
	}

	CSize sizeText(0, 0);

	if (!m_strText.IsEmpty())
	{
		sizeText = GetTextSize(&dc, m_strText);

		m_rectText.top = nBaseLine - sizeText.cy / 2;
		m_rectText.bottom = m_rectText.top + sizeText.cy;

		switch (m_textAlignment)
		{
		case ALIGN_LEFT:
			m_rectText.left = nNextXOffsetLeft;
			nNextXOffsetLeft += sizeText.cx + 2 * m_nMargin;
			break;

		case ALIGN_RIGHT:
			m_rectText.left = nNextXOffsetRight - sizeText.cx;
			break;

		case ALIGN_CENTER:
			bTextCenter = TRUE;
			nNextXOffsetCenter -= sizeText.cx / 2;
			break;

		default:
			ASSERT(FALSE);
			return;
		}

		m_rectText.right = m_rectText.left + sizeText.cx;
		AdjustRectToMargin(m_rectText, rectClient, m_nMargin);
		m_rectDrawText = m_rectText;
	}

	if (bIconCenter)
	{
		m_rectImage.left = nNextXOffsetCenter;
		m_rectImage.right = m_rectImage.left + sizeImage.cx;
		nNextXOffsetCenter = m_rectImage.right + m_nHorzElementOffset;
	}

	if (bButtonAfterText)
	{
		m_rectButton.left = nNextXOffsetLeft;
		m_rectButton.right = m_rectButton.left + nButtonWidth;

		if (m_rectButton.right + m_nMargin > rectClient.right)
		{
			m_rectButton.right = rectClient.right - m_nMargin;
			m_rectButton.left = m_rectButton.right - nButtonWidth;
		}
	}
	else if (bButtonCenter)
	{
		m_rectButton.left = nNextXOffsetCenter;
		m_rectButton.right = m_rectButton.left + nButtonWidth;
		nNextXOffsetCenter = m_rectButton.right + m_nHorzElementOffset;
	}

	if (bTextCenter)
	{
		m_rectText.left = nNextXOffsetCenter;
		m_rectText.right = m_rectText.left + sizeText.cx;
		AdjustRectToMargin(m_rectText, rectClient, m_nMargin);
		m_rectDrawText = m_rectText;
	}

	if (IsImageSet())
	{
		// do not retain image size if it should be stretched
		AdjustRectToMargin(m_rectImage, rectClient, m_nMargin, !m_bStretchImage);

		if (m_rectImage.left < rectClient.left || m_rectImage.right > rectClient.right)
		{
			m_rectImage.SetRectEmpty();
		}
	}

	CRect rectButtonTemp = m_rectButton;
	if (!m_strBtnText.IsEmpty() && IsImageSet())
	{
		CheckRectangle(rectButtonTemp, m_rectImage, bButtonLeftOfIcon);
	}

	if (!m_strBtnText.IsEmpty())
	{
		AdjustRectToMargin(rectButtonTemp, rectClient, m_nMargin);

		if (m_rectButton.Width() + m_rectImage.Width() + 2 * m_nMargin > rectClient.Width())
		{
			m_rectButton.SetRectEmpty();
		}
	}

	if (!m_strText.IsEmpty())
	{
		CheckRectangle(m_rectDrawText, m_rectImage, bTextLeftOfIcon);
		CheckRectangle(m_rectDrawText, rectButtonTemp, bTextLeftOfButton || bButtonAfterText);
	}

	if (pOldFont != NULL)
	{
		dc.SelectObject(pOldFont);
	}

	m_bTextIsTruncated = m_rectDrawText.Width() < sizeText.cx;

	UpdateTooltips();
}

BOOL CMFCCaptionBar::CheckRectangle(CRect& rectSrc, const CRect& rectOther, BOOL bLeftOf)
{
	if (rectSrc.IsRectEmpty() || rectOther.IsRectEmpty())
	{
		return FALSE;
	}

	CRect rectOtherWithOffset = rectOther;
	rectOtherWithOffset.InflateRect(m_nHorzElementOffset, m_nHorzElementOffset);

	if (rectSrc.left <= rectOtherWithOffset.right && rectSrc.left >= rectOtherWithOffset.left)
	{
		rectSrc.left = rectOtherWithOffset.right;
	}

	if (rectSrc.right >= rectOtherWithOffset.left && rectSrc.right <= rectOtherWithOffset.right)
	{
		rectSrc.right = rectOtherWithOffset.left;
	}

	if (rectSrc.left >= rectOtherWithOffset.left && rectSrc.right <= rectOtherWithOffset.right)
	{
		rectSrc.right = rectSrc.left;
	}

	if (rectSrc.left <= rectOtherWithOffset.left && rectSrc.right >= rectOtherWithOffset.right)
	{
		if (bLeftOf)
		{
			rectSrc.right = rectOtherWithOffset.left;
		}
		else
		{
			rectSrc.left = rectOtherWithOffset.right;
		}
	}

	if (bLeftOf && rectSrc.left >= rectOtherWithOffset.right || !bLeftOf && rectSrc.right <= rectOtherWithOffset.left)
	{
		rectSrc.left = rectSrc.right;
	}

	return FALSE;
}

void CMFCCaptionBar::AdjustRectToMargin(CRect& rectSrc, const CRect& rectClient, int nMargin, BOOL bRetainSize)
{
	BOOL bLeftChanged = FALSE;
	BOOL bRightChanged = FALSE;
	int nWidth = rectSrc.Width();
	if (rectSrc.left < rectClient.left + nMargin)
	{
		rectSrc.left = rectClient.left + nMargin;
		bLeftChanged = TRUE;
	}

	if (rectSrc.right > rectClient.right - nMargin)
	{
		rectSrc.right = rectClient.right - nMargin;
		bRightChanged = TRUE;
	}

	if (bRetainSize)
	{
		if (bLeftChanged)
		{
			rectSrc.right = rectSrc.left + nWidth;
		}
		else if (bRightChanged)
		{
			rectSrc.left = 	rectSrc.right - nWidth;
		}
	}
}

CSize CMFCCaptionBar::GetImageSize() const
{
	if (m_Bitmap.GetCount() > 0)
	{
		ENSURE(m_hIcon == NULL);
		return m_Bitmap.GetImageSize();
	}

	if (m_hIcon == NULL)
	{
		return CSize(0, 0);
	}

	ICONINFO info;
	memset(&info, 0, sizeof(ICONINFO));

	::GetIconInfo(m_hIcon, &info);
	HBITMAP hBmp = info.hbmColor;

	BITMAP bmp;
	::GetObject(hBmp, sizeof(BITMAP), (LPVOID) &bmp);

	::DeleteObject(info.hbmColor);
	::DeleteObject(info.hbmMask);

	return CSize(bmp.bmWidth, bmp.bmHeight);
}

BOOL CMFCCaptionBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMFCCaptionBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPane::OnLButtonDown(nFlags, point);

	if (m_uiBtnID != 0 && m_bBtnEnabled && m_bIsBtnHighlighted)
	{
		m_bIsBtnPressed = TRUE;
		InvalidateRect(m_rectButton);
		UpdateWindow();

		if (m_bBtnHasDropDownArrow)
		{
			ASSERT_VALID(GetOwner());
			GetOwner()->SendMessage(WM_COMMAND, m_uiBtnID);
		}
	}

	if (m_bIsCloseBtnHighlighted)
	{
		m_bIsCloseBtnPressed= TRUE;
		InvalidateRect(m_rectClose);
		UpdateWindow();
	}
}

void CMFCCaptionBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPane::OnLButtonUp(nFlags, point);

	if (m_bIsBtnPressed)
	{
		m_bIsBtnPressed = FALSE;
		InvalidateRect(m_rectButton);
		UpdateWindow();

		if (!m_bBtnHasDropDownArrow && m_uiBtnID != 0)
		{
			ASSERT_VALID(GetOwner());
			GetOwner()->SendMessage(WM_COMMAND, m_uiBtnID);
		}
	}
	else if (m_bIsCloseBtnPressed)
	{
		m_bIsCloseBtnPressed = FALSE;

		InvalidateRect(m_rectClose);
		UpdateWindow();

		ShowPane(FALSE, FALSE, FALSE);
	}
}

void CMFCCaptionBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CPane::OnMouseMove(nFlags, point);

	BOOL bTrack = FALSE;

	if (m_uiBtnID != 0 && m_bBtnEnabled)
	{
		BOOL bIsBtnHighlighted = m_rectButton.PtInRect(point);

		if (m_bIsBtnHighlighted != bIsBtnHighlighted)
		{
			m_bIsBtnHighlighted = bIsBtnHighlighted;
			m_bIsBtnPressed = (nFlags & MK_LBUTTON) && m_bIsBtnHighlighted;

			InvalidateRect(m_rectButton);
			UpdateWindow();

			bTrack = bIsBtnHighlighted;
		}
	}

	if (!m_rectClose.IsRectEmpty())
	{
		BOOL bIsBtnHighlighted = m_rectClose.PtInRect(point);

		if (m_bIsCloseBtnHighlighted != bIsBtnHighlighted)
		{
			m_bIsCloseBtnHighlighted = bIsBtnHighlighted;
			m_bIsCloseBtnPressed = (nFlags & MK_LBUTTON) && m_bIsCloseBtnHighlighted;

			InvalidateRect(m_rectClose);
			UpdateWindow();

			bTrack = bIsBtnHighlighted;
		}
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::AFXTrackMouse(&trackmouseevent);
	}
}

afx_msg LRESULT CMFCCaptionBar::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_bIsBtnPressed || m_bIsBtnHighlighted)
	{
		m_bIsBtnPressed = FALSE;
		m_bIsBtnHighlighted = FALSE;

		InvalidateRect(m_rectButton);
		UpdateWindow();
	}

	if (m_bIsCloseBtnPressed || m_bIsCloseBtnHighlighted)
	{
		m_bIsCloseBtnPressed = FALSE;
		m_bIsCloseBtnHighlighted = FALSE;

		InvalidateRect(m_rectClose);
		UpdateWindow();
	}

	return 0;
}

void CMFCCaptionBar::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CFrameWnd* pParent = AFXGetParentFrame(this);
	if (pParent != NULL && pParent->GetSafeHwnd() != NULL)
	{
		pParent->RecalcLayout();
	}

	RecalcLayout();
}

void CMFCCaptionBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!CMFCToolBar::IsCustomizeMode())
	{
		ASSERT_VALID(GetOwner());

		ClientToScreen(&point);
		OnPaneContextMenu(GetOwner(), point);
		return;
	}

	CPane::OnRButtonUp(nFlags, point);
}

void CMFCCaptionBar::OnDestroy()
{
	CTooltipManager::DeleteToolTip(m_pToolTip);
	CPane::OnDestroy();
}

BOOL CMFCCaptionBar::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	ENSURE(pNMH != NULL);

	if (m_pToolTip->GetSafeHwnd() == NULL || pNMH->hwndFrom != m_pToolTip->GetSafeHwnd())
	{
		return FALSE;
	}

	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return FALSE;
	}

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	CString strDescr;

	switch (pNMH->idFrom)
	{
	case nIdToolTipClose:
		ENSURE(strTipText.LoadString(IDS_AFXBARRES_CLOSEBAR));
		break;

	case nIdToolTipText:
		strTipText = m_strText;
		strTipText.Remove(_T('\b'));
		break;

	case nIdToolTipImage:
		strTipText = m_strImageToolTip;
		strDescr = m_strImageDescription;
		break;

	case nIdToolTipButton:
		strTipText = m_strButtonToolTip;
		strDescr = m_strButtonDescription;
		break;
	}

	if (strTipText.IsEmpty())
	{
		return TRUE;
	}

	CMFCToolTipCtrl* pToolTip = DYNAMIC_DOWNCAST(CMFCToolTipCtrl, m_pToolTip);

	if (pToolTip != NULL && !strDescr.IsEmpty())
	{
		ASSERT_VALID(pToolTip);
		pToolTip->SetDescription(strDescr);
	}

	pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
	return TRUE;
}

BOOL CMFCCaptionBar::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;
	}

	return CPane::PreTranslateMessage(pMsg);
}

LRESULT CMFCCaptionBar::OnUpdateToolTips(WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (nTypes & AFX_TOOLTIP_TYPE_CAPTIONBAR)
	{
		CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_CAPTIONBAR);

		CRect rectDummy(0, 0, 0, 0);

		m_pToolTip->SetMaxTipWidth(640);

		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipClose);
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipText);
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipImage);
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipButton);
	}

	return 0;
}

void CMFCCaptionBar::UpdateTooltips()
{
	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		m_pToolTip->SetToolRect(this, nIdToolTipClose, m_rectClose);

		if (m_bTextIsTruncated)
		{
			m_pToolTip->SetToolRect(this, nIdToolTipText, m_rectDrawText);
		}
		else
		{
			m_pToolTip->SetToolRect(this, nIdToolTipText, CRect(0, 0, 0, 0));
		}

		if (!m_strImageToolTip.IsEmpty())
		{
			m_pToolTip->SetToolRect(this, nIdToolTipImage, m_rectImage);
		}
		else
		{
			m_pToolTip->SetToolRect(this, nIdToolTipImage, CRect(0, 0, 0, 0));
		}

		if (!m_strButtonToolTip.IsEmpty())
		{
			m_pToolTip->SetToolRect(this, nIdToolTipButton, m_rectButton);
		}
		else
		{
			m_pToolTip->SetToolRect(this, nIdToolTipButton, CRect(0, 0, 0, 0));
		}
	}
}

CSize CMFCCaptionBar::GetTextSize(CDC* pDC, const CString& strText)
{
	ASSERT_VALID(pDC);

	if (m_arTextParts.GetSize() == 1)
	{
		return pDC->GetTextExtent(strText);
	}

	CSize sizeText(0, 0);

	BOOL bIsBold = FALSE;

	for (int i = 0; i < m_arTextParts.GetSize(); i++)
	{
		if (!m_arTextParts [i].IsEmpty())
		{
			CFont* pOldFont = NULL;

			if (bIsBold)
			{
				pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
			}

			CSize sizePart = pDC->GetTextExtent(m_arTextParts [i]);

			sizeText.cx += sizePart.cx;
			sizeText.cy = max(sizeText.cy, sizePart.cy);

			if (pOldFont != NULL)
			{
				pDC->SelectObject(pOldFont);
			}
		}

		bIsBold = !bIsBold;
	}

	return sizeText;
}



