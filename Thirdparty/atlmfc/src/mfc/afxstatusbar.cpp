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
#include "afxglobals.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmdichildwndex.h"
#include "afxolecntrframewndex.h"
#include "afxglobalutils.h"

#include "afxvisualmanager.h"
#include "afxdrawmanager.h"
#include "afxstatusbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCStatusBar

inline CMFCStatusBarPaneInfo* CMFCStatusBar::_GetPanePtr(int nIndex) const
{
	if (nIndex == 255 && m_nCount < 255)
	{
		// Special case for the simple pane
		for (int i = 0; i < m_nCount; i++)
		{
			CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(i);
			ENSURE(pSBP != NULL);

			if (pSBP->nStyle & SBPS_STRETCH)
			{
				return pSBP;
			}
		}
	}

	if (nIndex < 0 || nIndex >= m_nCount)
	{
		return NULL;
	}

	if (m_pData == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return((CMFCStatusBarPaneInfo*)m_pData) + nIndex;
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

const int nTextMargin = 4; // Gap between image and text

CMFCStatusBar::CMFCStatusBar()
{
	m_hFont = NULL;

	// setup correct margins
	m_cxRightBorder = m_cxDefaultGap;
	m_cxSizeBox = 0;

	m_cxLeftBorder = 4;
	m_cyTopBorder = 2;
	m_cyBottomBorder = 0;
	m_cxRightBorder = 0;

	m_bPaneDoubleClick = FALSE;
	m_bDrawExtendedArea = FALSE;

	m_rectSizeBox.SetRectEmpty();
}

void CMFCStatusBar::OnSettingChange(UINT /*uFlags*/, LPCTSTR /* lpszSection */)
{
	RecalcLayout();
}

CMFCStatusBar::~CMFCStatusBar()
{
}

void CMFCStatusBar::OnDestroy()
{
	for (int i = 0; i < m_nCount; i++)
	{
		VERIFY(SetPaneText(i, NULL, FALSE));    // no update
		SetTipText(i, NULL);
		SetPaneIcon(i, NULL, FALSE);
	}

	CPane::OnDestroy();
}

BOOL CMFCStatusBar::PreCreateWindow(CREATESTRUCT& cs)
{
	// in Win4, status bars do not have a border at all, since it is
	//  provided by the client area.
	if ((m_dwStyle &(CBRS_ALIGN_ANY|CBRS_BORDER_ANY)) == CBRS_BOTTOM)
	{
		m_dwStyle &= ~(CBRS_BORDER_ANY|CBRS_BORDER_3D);
	}

	return CPane::PreCreateWindow(cs);
}

BOOL CMFCStatusBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	return CreateEx(pParentWnd, 0, dwStyle, nID);
}

BOOL CMFCStatusBar::CreateEx(CWnd* pParentWnd, DWORD /*dwCtrlStyle*/, DWORD dwStyle, UINT nID)
{
	ENSURE( AfxIsExtendedFrameClass(pParentWnd) );

	// save the style
	SetPaneAlignment(dwStyle & CBRS_ALL);

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();

	m_dwControlBarStyle = 0; // can't float, resize, close, slide

	if (pParentWnd->GetStyle() & WS_THICKFRAME)
	{
		dwStyle |= SBARS_SIZEGRIP;
	}

	if (!CWnd::Create(afxGlobalData.RegisterWindowClass(_T("Afx:StatusBar")), NULL, dwStyle | WS_CLIPSIBLINGS, rect, pParentWnd, nID))
	{
		return FALSE;
	}

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
		((COleDocIPFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		((CMDIChildWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		((COleCntrFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(CDialog)))
	{
		afxGlobalUtils.m_bDialogApp = TRUE;
	}

	return TRUE;
}

BOOL CMFCStatusBar::SetIndicators(const UINT* lpIDArray, int nIDCount)
{
	ASSERT_VALID(this);
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ENSURE(lpIDArray == NULL || AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	// free strings before freeing array of elements
	for (int i = 0; i < m_nCount; i++)
	{
		VERIFY(SetPaneText(i, NULL, FALSE));    // no update
		//free Imagelist if any exist
		SetPaneIcon(i, NULL, FALSE);

	}

	// first allocate array for panes and copy initial data
	if (!AllocElements(nIDCount, sizeof(CMFCStatusBarPaneInfo)))
		return FALSE;

	ASSERT(nIDCount == m_nCount);

	HFONT hFont = GetCurrentFont();

	BOOL bOK = TRUE;
	if (lpIDArray != NULL)
	{
		ENSURE(hFont != NULL);        // must have a font !
		CString strText;
		CClientDC dcScreen(NULL);
		HGDIOBJ hOldFont = dcScreen.SelectObject(hFont);

		for (int i = 0; i < nIDCount; i++)
		{
			CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(i);
			if (pSBP == NULL)
			{
				ASSERT(FALSE);
				return FALSE;
			}

			pSBP->nStyle = 0;
			pSBP->lpszText = NULL;
			pSBP->lpszToolTip = NULL;
			pSBP->clrText = (COLORREF)-1;
			pSBP->clrBackground = (COLORREF)-1;
			pSBP->hImage = NULL;
			pSBP->cxIcon = 0;
			pSBP->cyIcon = 0;
			pSBP->rect = CRect(0, 0, 0, 0);
			pSBP->nFrameCount = 0;
			pSBP->nCurrFrame = 0;
			pSBP->nProgressCurr = 0;
			pSBP->nProgressTotal = -1;
			pSBP->clrProgressBar = (COLORREF)-1;
			pSBP->clrProgressBarDest = (COLORREF)-1;
			pSBP->clrProgressText = (COLORREF)-1;
			pSBP->bProgressText = FALSE;

			pSBP->nID = *lpIDArray++;
			if (pSBP->nID != 0)
			{
				if (!strText.LoadString(pSBP->nID))
				{
					TRACE1("Warning: failed to load indicator string 0x%04X.\n", pSBP->nID);
					bOK = FALSE;
					break;
				}

				pSBP->cxText = dcScreen.GetTextExtent(strText, strText.GetLength()).cx;
				ASSERT(pSBP->cxText >= 0);

				if (!SetPaneText(i, strText, FALSE))
				{
					bOK = FALSE;
					break;
				}
			}
			else
			{
				// no indicator(must access via index)
				// default to 1/4 the screen width(first pane is stretchy)
				pSBP->cxText = ::GetSystemMetrics(SM_CXSCREEN) / 4;

				if (i == 0)
				{
					pSBP->nStyle |= (SBPS_STRETCH | SBPS_NOBORDERS);
				}
			}
		}

		dcScreen.SelectObject(hOldFont);
	}

	RecalcLayout();
	return bOK;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCStatusBar attribute access

int CMFCStatusBar::CommandToIndex(UINT nIDFind) const
{
	ASSERT_VALID(this);

	if (m_nCount <= 0)
	{
		return -1;
	}

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(0);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		if (pSBP->nID == nIDFind)
		{
			return i;
		}
	}

	return -1;
}

UINT CMFCStatusBar::GetItemID(int nIndex) const
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	return pSBP->nID;
}

void CMFCStatusBar::GetItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);
	ENSURE(AfxIsValidAddress(lpRect, sizeof(RECT)));

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	*lpRect = pSBP->rect;
}

UINT CMFCStatusBar::GetPaneStyle(int nIndex) const
{
	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	return pSBP->nStyle;
}

void CMFCStatusBar::SetPaneStyle(int nIndex, UINT nStyle)
{
	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pSBP->nStyle != nStyle)
	{
		// just change the style of 1 pane, and invalidate it
		pSBP->nStyle = nStyle;
		InvalidateRect(&pSBP->rect, FALSE);
		UpdateWindow();
	}
}

int CMFCStatusBar::GetPaneWidth(int nIndex) const
{
	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	CRect rect = pSBP->rect;
	return rect.Width();
}

void CMFCStatusBar::SetPaneWidth(int nIndex, int cx)
{
	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CRect rect = pSBP->rect;
	int cxCurr = rect.Width() - AFX_CX_BORDER * 4;

	int cxTextNew = cx - pSBP->cxIcon;
	if (pSBP->cxIcon > 0)
	{
		cxTextNew -= nTextMargin;
	}

	pSBP->cxText = max(0, cxTextNew);

	if (cx != cxCurr)
	{
		RecalcLayout();
		Invalidate();
		UpdateWindow();
	}
}

void CMFCStatusBar::GetPaneInfo(int nIndex, UINT& nID, UINT& nStyle, int& cxWidth) const
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	nID = pSBP->nID;
	nStyle = pSBP->nStyle;

	CRect rect = pSBP->rect;
	cxWidth = rect.Width();
}

void CMFCStatusBar::SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pSBP->nID = nID;
	SetPaneStyle(nIndex, nStyle);
	SetPaneWidth(nIndex, cxWidth);
}

void CMFCStatusBar::GetPaneText(int nIndex, CString& s) const
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	s = pSBP->lpszText == NULL ? _T("") : pSBP->lpszText;
}

CString CMFCStatusBar::GetPaneText(int nIndex) const
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return _T("");
	}

	CString s = pSBP->lpszText == NULL ? _T("") : pSBP->lpszText;
	return s;
}

BOOL CMFCStatusBar::SetPaneText(int nIndex, LPCTSTR lpszNewText, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		return FALSE;
	}

	if (pSBP->lpszText != NULL)
	{
		if (lpszNewText != NULL && lstrcmp(pSBP->lpszText, lpszNewText) == 0)
		{
			return TRUE;        // nothing to change
		}

		free((LPVOID)pSBP->lpszText);
	}
	else if (lpszNewText == NULL || *lpszNewText == '\0')
	{
		return TRUE; // nothing to change
	}

	BOOL bOK = TRUE;
	if (lpszNewText == NULL || *lpszNewText == '\0')
	{
		pSBP->lpszText = NULL;
	}
	else
	{
		pSBP->lpszText = _tcsdup(lpszNewText);
		if (pSBP->lpszText == NULL)
			bOK = FALSE; // old text is lost and replaced by NULL
	}

	if (bUpdate)
	{
		InvalidatePaneContent(nIndex);
	}

	return bOK;
}

void CMFCStatusBar::SetPaneIcon(int nIndex, HICON hIcon, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	// Disable animation(if exist):
	SetPaneAnimation(nIndex, NULL, 0, FALSE);

	if (hIcon == NULL)
	{
		if (pSBP->hImage != NULL)
		{
			::ImageList_Destroy(pSBP->hImage);
		}

		pSBP->hImage = NULL;

		if (bUpdate)
		{
			InvalidatePaneContent(nIndex);
		}

		return;
	}

	ICONINFO iconInfo;
	::GetIconInfo(hIcon, &iconInfo);

	BITMAP bitmap;
	::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bitmap);

	::DeleteObject(iconInfo.hbmColor);
	::DeleteObject(iconInfo.hbmMask);

	if (pSBP->hImage == NULL)
	{
		pSBP->cxIcon = bitmap.bmWidth;
		pSBP->cyIcon = bitmap.bmHeight;

		pSBP->hImage = ::ImageList_Create(pSBP->cxIcon, pSBP->cyIcon, ILC_MASK | ILC_COLORDDB, 1, 0);
		::ImageList_AddIcon(pSBP->hImage, hIcon);

		RecalcLayout();
	}
	else
	{
		ASSERT(pSBP->cxIcon == bitmap.bmWidth);
		ASSERT(pSBP->cyIcon == bitmap.bmHeight);

		::ImageList_ReplaceIcon(pSBP->hImage, 0, hIcon);
	}

	if (bUpdate)
	{
		InvalidatePaneContent(nIndex);
	}
}

void CMFCStatusBar::SetPaneIcon(int nIndex, HBITMAP hBmp, COLORREF clrTransparent, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	// Disable animation(if exist):
	SetPaneAnimation(nIndex, NULL, 0, FALSE);

	if (hBmp == NULL)
	{
		if (pSBP->hImage != NULL)
		{
			::ImageList_Destroy(pSBP->hImage);
		}

		pSBP->hImage = NULL;

		if (bUpdate)
		{
			InvalidatePaneContent(nIndex);
		}

		return;
	}

	BITMAP bitmap;
	::GetObject(hBmp, sizeof(BITMAP), &bitmap);

	if (pSBP->hImage == NULL)
	{
		pSBP->cxIcon = bitmap.bmWidth;
		pSBP->cyIcon = bitmap.bmHeight;

		pSBP->hImage = ::ImageList_Create(pSBP->cxIcon, pSBP->cyIcon, ILC_MASK | ILC_COLORDDB, 1, 0);
		RecalcLayout();
	}
	else
	{
		ASSERT(pSBP->cxIcon == bitmap.bmWidth);
		ASSERT(pSBP->cyIcon == bitmap.bmHeight);

		::ImageList_Remove(pSBP->hImage, 0);
	}

	// Because ImageList_AddMasked changes the original bitmap,
	// we need to create a copy:
	HBITMAP hbmpCopy = (HBITMAP) ::CopyImage(hBmp, IMAGE_BITMAP, 0, 0, 0);
	::ImageList_AddMasked(pSBP->hImage, hbmpCopy, clrTransparent);
	::DeleteObject(hbmpCopy);

	if (bUpdate)
	{
		InvalidatePaneContent(nIndex);
	}
}

void CMFCStatusBar::SetPaneAnimation(int nIndex, HIMAGELIST hImageList, UINT nFrameRate, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pSBP->nFrameCount > 0)
	{
		KillTimer(pSBP->nID);
	}

	if (pSBP->hImage != NULL)
	{
		::ImageList_Destroy(pSBP->hImage);
		pSBP->hImage = NULL;
	}

	pSBP->nCurrFrame = 0;
	pSBP->nFrameCount = 0;

	if (hImageList == NULL)
	{
		if (bUpdate)
		{
			InvalidatePaneContent(nIndex);
		}

		return;
	}

	pSBP->nFrameCount = ::ImageList_GetImageCount(hImageList);
	if (pSBP->nFrameCount == 0)
	{
		if (bUpdate)
		{
			InvalidatePaneContent(nIndex);
		}

		return;
	}

	::ImageList_GetIconSize(hImageList, &pSBP->cxIcon, &pSBP->cyIcon);

	pSBP->hImage = ::ImageList_Create(pSBP->cxIcon, pSBP->cyIcon, ILC_MASK | ILC_COLORDDB, 1, 1);

	for (int i =0; i < pSBP->nFrameCount; i++)
	{
		HICON hIcon = ::ImageList_GetIcon(hImageList, i, ILD_TRANSPARENT);
		::ImageList_AddIcon(pSBP->hImage, hIcon);
		::DestroyIcon(hIcon);
	}

	RecalcLayout();
	if (bUpdate)
	{
		InvalidatePaneContent(nIndex);
	}

	SetTimer(pSBP->nID, nFrameRate, NULL);
}

void CMFCStatusBar::EnablePaneProgressBar(int nIndex, long nTotal, BOOL bDisplayText, COLORREF clrBar, COLORREF clrBarDest, COLORREF clrProgressText)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pSBP->bProgressText = bDisplayText;
	pSBP->clrProgressBar = clrBar;
	pSBP->clrProgressBarDest = clrBarDest;
	pSBP->nProgressTotal = nTotal;
	pSBP->nProgressCurr = 0;
	pSBP->clrProgressText = clrProgressText;

	if (clrBarDest != (COLORREF)-1 && pSBP->bProgressText)
	{
		// Progress text is not available when the gradient is ON
		ASSERT(FALSE);
		pSBP->bProgressText = FALSE;
	}

	InvalidatePaneContent(nIndex);
}

void CMFCStatusBar::SetPaneProgress(int nIndex, long nCurr, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT(nCurr >= 0);
	ASSERT(nCurr <= pSBP->nProgressTotal);

	long lPos = min(max(0, nCurr), pSBP->nProgressTotal);
	if (pSBP->nProgressCurr != lPos)
	{
		pSBP->nProgressCurr = lPos;

		if (bUpdate)
		{
			InvalidatePaneContent(nIndex);
		}
	}
}

void CMFCStatusBar::SetPaneTextColor(int nIndex, COLORREF clrText, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pSBP->clrText != clrText)
	{
		pSBP->clrText = clrText;

		if (bUpdate)
		{
			InvalidatePaneContent(nIndex);
		}
	}
}

void CMFCStatusBar::SetPaneBackgroundColor(int nIndex, COLORREF clrBackground, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pSBP->clrBackground != clrBackground)
	{
		pSBP->clrBackground = clrBackground;

		if (bUpdate)
		{
			InvalidatePaneContent(nIndex);
		}
	}
}

CString CMFCStatusBar::GetTipText(int nIndex) const
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return _T("");
	}

	CString s = pSBP->lpszToolTip == NULL ? _T("") : pSBP->lpszToolTip;
	return s;
}

void CMFCStatusBar::SetTipText(int nIndex, LPCTSTR pszTipText)
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pSBP->lpszToolTip != NULL)
	{
		if (pszTipText != NULL && lstrcmp(pSBP->lpszToolTip, pszTipText) == 0)
		{
			return;        // nothing to change
		}

		free((LPVOID)pSBP->lpszToolTip);
	}
	else if (pszTipText == NULL || *pszTipText == '\0')
	{
		return; // nothing to change
	}

	if (pszTipText == NULL || *pszTipText == '\0')
	{
		pSBP->lpszToolTip = NULL;
	}
	else
	{
		pSBP->lpszToolTip = _tcsdup(pszTipText);
	}

	CBasePane::SetPaneStyle(CBasePane::GetPaneStyle() | CBRS_TOOLTIPS);
}

void CMFCStatusBar::InvalidatePaneContent(int nIndex)
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	// invalidate the text of the pane - not including the border

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CRect rect = pSBP->rect;

	if (!(pSBP->nStyle & SBPS_NOBORDERS))
		rect.InflateRect(-AFX_CX_BORDER, -AFX_CY_BORDER);
	else
		rect.top -= AFX_CY_BORDER;  // base line adjustment

	InvalidateRect(rect, FALSE);
	UpdateWindow();
}

void CMFCStatusBar::EnablePaneDoubleClick(BOOL bEnable)
{
	m_bPaneDoubleClick = bEnable;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCStatusBar implementation

CSize CMFCStatusBar::CalcFixedLayout(BOOL, BOOL bHorz)
{
	ASSERT_VALID(this);

	// recalculate based on font height + icon height + borders
	TEXTMETRIC tm;
	{
		CClientDC dcScreen(NULL);
		HFONT hFont = GetCurrentFont();

		HGDIOBJ hOldFont = dcScreen.SelectObject(hFont);
		VERIFY(dcScreen.GetTextMetrics(&tm));
		dcScreen.SelectObject(hOldFont);
	}

	int cyIconMax = 0;
	CMFCStatusBarPaneInfo* pSBP = (CMFCStatusBarPaneInfo*)m_pData;
	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		cyIconMax = max(cyIconMax, pSBP->cyIcon);
	}

	CRect rectSize;
	rectSize.SetRectEmpty();
	CalcInsideRect(rectSize, bHorz);    // will be negative size

	// sizeof text + 1 or 2 extra on top, 2 on bottom + borders
	return CSize(32767, max(cyIconMax, tm.tmHeight) + AFX_CY_BORDER * 4 - rectSize.Height());
}

void CMFCStatusBar::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CRect rectClip;
	pDCPaint->GetClipBox(rectClip);

	CRect rect;
	GetClientRect(rect);

	CMemDC memDC(*pDCPaint, this);
	CDC* pDC = &memDC.GetDC();

	CPane::DoPaint(pDC);      // draw border

	HFONT hFont = GetCurrentFont();
	HGDIOBJ hOldFont = pDC->SelectObject(hFont);

	int nOldMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF crTextColor = pDC->SetTextColor(afxGlobalData.clrBtnText);
	COLORREF crBkColor = pDC->SetBkColor(afxGlobalData.clrBtnFace);

	CMFCStatusBarPaneInfo* pSBP = (CMFCStatusBarPaneInfo*)m_pData;
	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		OnDrawPane(pDC, pSBP);
	}

	pDC->SelectObject(hOldFont);

	// draw the size box in the bottom right corner
	if (!m_rectSizeBox.IsRectEmpty())
	{
		CMFCVisualManager::GetInstance()->OnDrawStatusBarSizeBox(pDC, this, m_rectSizeBox);
	}

	pDC->SetTextColor(crTextColor);
	pDC->SetBkColor(crBkColor);
	pDC->SetBkMode(nOldMode);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCStatusBar message handlers

//{{AFX_MSG_MAP(CMFCStatusBar)
BEGIN_MESSAGE_MAP(CMFCStatusBar, CPane)
	ON_WM_NCHITTEST()
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
	ON_WM_SETTINGCHANGE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_MESSAGE(WM_SETFONT, &CMFCStatusBar::OnSetFont)
	ON_MESSAGE(WM_GETFONT, &CMFCStatusBar::OnGetFont)
	ON_MESSAGE(WM_SETTEXT, &CMFCStatusBar::OnSetText)
	ON_MESSAGE(WM_GETTEXT, &CMFCStatusBar::OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, &CMFCStatusBar::OnGetTextLength)
	ON_MESSAGE(WM_STYLECHANGED, &CMFCStatusBar::OnStyleChanged)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

int CMFCStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableToolTips();
	return 0;
}

void CMFCStatusBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_bPaneDoubleClick)
	{
		CMFCStatusBarPaneInfo* pSBP = HitTest(point);
		if (pSBP != NULL)
		{
			GetOwner()->PostMessage(WM_COMMAND, pSBP->nID);
		}
	}

	CPane::OnLButtonDblClk(nFlags, point);
}

void CMFCStatusBar::OnTimer(UINT_PTR nIDEvent)
{
	CPane::OnTimer(nIDEvent);

	int nIndex = CommandToIndex((UINT)nIDEvent);
	if (nIndex < 0)
	{
		return;
	}

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (++pSBP->nCurrFrame >= pSBP->nFrameCount)
	{
		pSBP->nCurrFrame = 0;
	}

	CRect rect = pSBP->rect;

	if (!(pSBP->nStyle & SBPS_NOBORDERS))
		rect.InflateRect(-AFX_CX_BORDER, -AFX_CY_BORDER);
	else
		rect.top -= AFX_CY_BORDER;  // base line adjustment

	rect.right = rect.left + pSBP->cxIcon;
	InvalidateRect(rect, FALSE);
	UpdateWindow();

	CWnd* pMenu = CMFCPopupMenu::GetActiveMenu();

	if (pMenu != NULL && CWnd::FromHandlePermanent(pMenu->GetSafeHwnd()) != NULL)
	{
		ClientToScreen(&rect);
		CMFCPopupMenu::UpdateAllShadows(rect);
	}
}

LRESULT CMFCStatusBar::OnNcHitTest(CPoint point)
{
	BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	// hit test the size box - convert to HTCAPTION if so
	if (m_cxSizeBox != 0)
	{
		CRect rect;
		GetClientRect(rect);
		CalcInsideRect(rect, TRUE);
		int cxMax = min(m_cxSizeBox-1, rect.Height());
		rect.left = rect.right - cxMax;
		ClientToScreen(&rect);

		if (rect.PtInRect(point))
		{
			return bRTL ? HTBOTTOMLEFT : HTBOTTOMRIGHT;
		}
	}
	return CPane::OnNcHitTest(point);
}

void CMFCStatusBar::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (!m_cxSizeBox != 0 &&(nID & 0xFFF0) == SC_SIZE)
	{
		CFrameWnd* pFrameWnd = AFXGetParentFrame(this);
		if (pFrameWnd != NULL)
		{
			pFrameWnd->SendMessage(WM_SYSCOMMAND, (WPARAM)nID, lParam);
			return;
		}
	}

	CPane::OnSysCommand(nID, lParam);
}

void CMFCStatusBar::OnSize(UINT nType, int cx, int cy)
{
	CPane::OnSize(nType, cx, cy);

	RecalcLayout();

	// force repaint on resize(recalculate stretchy)
	Invalidate();
	UpdateWindow();
}

LRESULT CMFCStatusBar::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	m_hFont = (HFONT)wParam;
	ASSERT(m_hFont != NULL);

	RecalcLayout();

	if ((BOOL)lParam)
	{
		Invalidate();
		UpdateWindow();
	}

	return 0L;      // does not re-draw or invalidate - resize parent instead
}

LRESULT CMFCStatusBar::OnGetFont(WPARAM, LPARAM)
{
	HFONT hFont = GetCurrentFont();
	return(LRESULT)(UINT_PTR)hFont;
}

LRESULT CMFCStatusBar::OnSetText(WPARAM, LPARAM lParam)
{
	int nIndex = CommandToIndex(0);
	if (nIndex < 0)
		return -1;
	return SetPaneText(nIndex, (LPCTSTR)lParam) ? 0 : -1;
}

LRESULT CMFCStatusBar::OnGetText(WPARAM wParam, LPARAM lParam)
{
	int nMaxLen = (int)wParam;
	if (nMaxLen == 0)
		return 0;       // nothing copied
	LPTSTR lpszDest = (LPTSTR)lParam;

	int nLen = 0;
	int nIndex = CommandToIndex(0); // use pane with ID zero
	if (nIndex >= 0)
	{
		CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
		if (pSBP == NULL)
		{
			ASSERT(FALSE);
			return 0;
		}

		nLen = pSBP->lpszText != NULL ? lstrlen(pSBP->lpszText) : 0;
		if (nLen > nMaxLen)
			nLen = nMaxLen - 1; // number of characters to copy(less term.)
		memcpy(lpszDest, pSBP->lpszText, nLen*sizeof(TCHAR));
	}
	lpszDest[nLen] = '\0';
	return nLen+1;      // number of bytes copied
}

LRESULT CMFCStatusBar::OnGetTextLength(WPARAM, LPARAM)
{
	int nLen = 0;
	int nIndex = CommandToIndex(0); // use pane with ID zero
	if (nIndex >= 0)
	{
		CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
		if (pSBP == NULL)
		{
			ASSERT(FALSE);
			return 0;
		}

		if (pSBP->lpszText != NULL)
		{
			nLen = lstrlen(pSBP->lpszText);
		}
	}

	return nLen;
}

void CMFCStatusBar::OnDrawPane(CDC* pDC, CMFCStatusBarPaneInfo* pPane)
{
	ASSERT_VALID(pDC);
	ENSURE(pPane != NULL);

	CRect rectPane = pPane->rect;
	if (rectPane.IsRectEmpty() || !pDC->RectVisible(rectPane))
	{
		return;
	}

	// Fill pane background:
	if (pPane->clrBackground != (COLORREF)-1)
	{
		CBrush brush(pPane->clrBackground);
		CBrush* pOldBrush = pDC->SelectObject(&brush);

		pDC->PatBlt(rectPane.left, rectPane.top, rectPane.Width(), rectPane.Height(), PATCOPY);

		pDC->SelectObject(pOldBrush);
	}

	// Draw pane border:
	CMFCVisualManager::GetInstance()->OnDrawStatusBarPaneBorder(pDC, this, rectPane, pPane->nID, pPane->nStyle);

	if (!(pPane->nStyle & SBPS_NOBORDERS)) // only adjust if there are borders
	{
		rectPane.DeflateRect(2 * AFX_CX_BORDER, AFX_CY_BORDER);
	}

	// Draw icon
	if (pPane->hImage != NULL && pPane->cxIcon > 0)
	{
		CRect rectIcon = rectPane;
		rectIcon.right = rectIcon.left + pPane->cxIcon;

		int x = max(0, (rectIcon.Width() - pPane->cxIcon) / 2);
		int y = max(0, (rectIcon.Height() - pPane->cyIcon) / 2);

		::ImageList_DrawEx(pPane->hImage, pPane->nCurrFrame, pDC->GetSafeHdc(), rectIcon.left + x, rectIcon.top + y, pPane->cxIcon, pPane->cyIcon, CLR_NONE, 0, ILD_NORMAL);
	}

	CRect rectText = rectPane;
	rectText.left += pPane->cxIcon;

	if (pPane->cxIcon > 0)
	{
		rectText.left += nTextMargin;
	}

	if (pPane->nProgressTotal > 0)
	{
		// Draw progress bar:
		CRect rectProgress = rectText;
		rectProgress.DeflateRect(1, 1);

		COLORREF clrBar = (pPane->clrProgressBar == (COLORREF)-1) ? afxGlobalData.clrHilite : pPane->clrProgressBar;

		CMFCVisualManager::GetInstance()->OnDrawStatusBarProgress(pDC, this, rectProgress,
			pPane->nProgressTotal, pPane->nProgressCurr, clrBar, pPane->clrProgressBarDest, pPane->clrProgressText, pPane->bProgressText);
	}
	else
	{
		// Draw text
		if (pPane->lpszText != NULL && pPane->cxText > 0)
		{
			COLORREF clrText = pDC->SetTextColor(CMFCVisualManager::GetInstance()->GetStatusBarPaneTextColor(this, pPane));

			pDC->DrawText(pPane->lpszText, lstrlen(pPane->lpszText), rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
			pDC->SetTextColor(clrText);
		}
	}
}

void CMFCStatusBar::RecalcLayout()
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() != NULL);

	// get the drawing area for the status bar
	CRect rect;
	GetClientRect(rect);
	CalcInsideRect(rect, TRUE);

	// the size box is based off the size of a scrollbar
	m_cxSizeBox = min(GetSystemMetrics(SM_CXVSCROLL)+1, rect.Height());

	CFrameWnd* pFrameWnd = AFXGetParentFrame(this);
	if (pFrameWnd != NULL && pFrameWnd->IsZoomed())
	{
		m_cxSizeBox = 0;
	}

	if ((GetStyle() & SBARS_SIZEGRIP) == 0)
	{
		m_cxSizeBox = 0;
	}

	CClientDC dcScreen(NULL);

	int xMax = (rect.right -= m_cxSizeBox);
	if (m_cxSizeBox == 0)
		xMax += m_cxRightBorder + 1;

	// walk through to calculate extra space
	int cxExtra = rect.Width() + m_cxDefaultGap;
	CMFCStatusBarPaneInfo* pSBP = (CMFCStatusBarPaneInfo*)m_pData;
	int i = 0;

	for (i = 0; i < m_nCount; i++, pSBP++)
	{
		cxExtra -= (pSBP->cxText + pSBP->cxIcon + AFX_CX_BORDER * 4 + m_cxDefaultGap);

		if (pSBP->cxText > 0 && pSBP->cxIcon > 0)
		{
			cxExtra -= nTextMargin;
		}
	}
	// if cxExtra <= 0 then we will not stretch but just clip

	for (i = 0, pSBP = (CMFCStatusBarPaneInfo*)m_pData; i < m_nCount; i++, pSBP++)
	{
		ASSERT(pSBP->cxText >= 0);
		ASSERT(pSBP->cxIcon >= 0);

		if (rect.left >= xMax)
		{
			pSBP->rect = CRect(0, 0, 0, 0);
		}
		else
		{
			int cxPane = pSBP->cxText + pSBP->cxIcon;
			if (pSBP->cxText > 0 && pSBP->cxIcon > 0)
			{
				cxPane += nTextMargin;
			}

			if ((pSBP->nStyle & SBPS_STRETCH) && cxExtra > 0)
			{
				cxPane += cxExtra;
				cxExtra = 0;
			}

			rect.right = rect.left + cxPane + AFX_CX_BORDER * 4;
			rect.right = min(rect.right, xMax);

			pSBP->rect = rect;

			rect.left = rect.right + m_cxDefaultGap;
		}
	}

	if (m_cxSizeBox != 0)
	{
		int cxMax = min(m_cxSizeBox, rect.Height()+m_cyTopBorder);

		m_rectSizeBox = rect;
		m_rectSizeBox.left = rect.right;
		m_rectSizeBox.right = m_rectSizeBox.left + cxMax;
	}
	else
	{
		m_rectSizeBox.SetRectEmpty();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCStatusBar idle update through CMFCStatusBarCmdUI class

class CMFCStatusBarCmdUI : public CCmdUI      // class private to this file!
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

void CMFCStatusBarCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CMFCStatusBar* pStatusBar = (CMFCStatusBar*)m_pOther;
	ENSURE(pStatusBar != NULL);
	ASSERT_KINDOF(CMFCStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->GetPaneStyle(m_nIndex) & ~SBPS_DISABLED;
	if (!bOn)
		nNewStyle |= SBPS_DISABLED;
	pStatusBar->SetPaneStyle(m_nIndex, nNewStyle);
}

void CMFCStatusBarCmdUI::SetCheck(int nCheck) // "checking" will pop out the text
{
	CMFCStatusBar* pStatusBar = (CMFCStatusBar*)m_pOther;
	ENSURE(pStatusBar != NULL);
	ASSERT_KINDOF(CMFCStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->GetPaneStyle(m_nIndex) & ~SBPS_POPOUT;
	if (nCheck != 0)
		nNewStyle |= SBPS_POPOUT;
	pStatusBar->SetPaneStyle(m_nIndex, nNewStyle);
}

void CMFCStatusBarCmdUI::SetText(LPCTSTR lpszText)
{
	ENSURE(m_pOther != NULL);
	ASSERT_KINDOF(CMFCStatusBar, m_pOther);
	ASSERT(m_nIndex < m_nIndexMax);

	((CMFCStatusBar*)m_pOther)->SetPaneText(m_nIndex, lpszText);
}

void CMFCStatusBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CMFCStatusBarCmdUI state;
	state.m_pOther = this;
	state.m_nIndexMax = (UINT)m_nCount;
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = _GetPanePtr(state.m_nIndex)->nID;
		state.DoUpdate(pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the status bar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

INT_PTR CMFCStatusBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	// check child windows first by calling CPane
	INT_PTR nHit = (INT_PTR) CPane::OnToolHitTest(point, pTI);
	if (nHit != -1)
		return nHit;

	CMFCStatusBarPaneInfo* pSBP = HitTest(point);
	if (pSBP != NULL && pSBP->lpszToolTip != NULL)
	{
		nHit = pSBP->nID;

		if (pTI != NULL)
		{
			CString strTipText = pSBP->lpszToolTip;

			pTI->lpszText = (LPTSTR) ::calloc((strTipText.GetLength() + 1), sizeof(TCHAR));
			lstrcpy(pTI->lpszText, strTipText);

			pTI->rect = pSBP->rect;
			pTI->uId = 0;
			pTI->hwnd = m_hWnd;
		}
	}

	CToolTipCtrl* pToolTip = AfxGetModuleState()->m_thread.GetDataNA()->m_pToolTip;
	if (pToolTip != NULL && pToolTip->GetSafeHwnd() != NULL)
	{
		pToolTip->SetFont(&afxGlobalData.fontTooltip, FALSE);
	}

	return nHit;
}

CMFCStatusBarPaneInfo* CMFCStatusBar::HitTest(CPoint pt) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_nCount; i++)
	{
		CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(i);
		ENSURE(pSBP != NULL);

		CRect rect = pSBP->rect;
		if (rect.PtInRect(pt))
		{
			return pSBP;
		}
	}

	return NULL;
}

long CMFCStatusBar::GetPaneProgress(int nIndex) const
{
	ASSERT_VALID(this);

	CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	return pSBP->nProgressCurr;
}

HFONT CMFCStatusBar::GetCurrentFont() const
{
	return m_hFont == NULL ? (HFONT) afxGlobalData.fontRegular.GetSafeHandle() : m_hFont;
}

LRESULT CMFCStatusBar::OnStyleChanged(WPARAM wp, LPARAM lp)
{
	int nStyleType = (int) wp;
	LPSTYLESTRUCT lpStyleStruct = (LPSTYLESTRUCT) lp;

	CPane::OnStyleChanged(nStyleType, lpStyleStruct);

	if ((lpStyleStruct->styleNew & SBARS_SIZEGRIP) && (lpStyleStruct->styleOld & SBARS_SIZEGRIP) == 0)
	{
		RecalcLayout();
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCStatusBar diagnostics

#ifdef _DEBUG
void CMFCStatusBar::AssertValid() const
{
	CPane::AssertValid();
}

void CMFCStatusBar::Dump(CDumpContext& dc) const
{
	CPane::Dump(dc);

	dc << "\nm_hFont = " <<(UINT_PTR)m_hFont;

	if (dc.GetDepth() > 0)
	{
		for (int i = 0; i < m_nCount; i++)
		{
			dc << "\nstatus pane[" << i << "] = {";
			dc << "\n\tnID = " << _GetPanePtr(i)->nID;
			dc << "\n\tnStyle = " << _GetPanePtr(i)->nStyle;
			dc << "\n\tcxText = " << _GetPanePtr(i)->cxText;
			dc << "\n\tcxIcon = " << _GetPanePtr(i)->cxIcon;
			dc << "\n\tlpszText = " << _GetPanePtr(i)->lpszText;
			dc << "\n\t}";
		}
	}

	dc << "\n";
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CMFCStatusBar, CPane)

BOOL CMFCStatusBar::GetExtendedArea(CRect& rect) const
{
	ASSERT_VALID(this);

	if (!m_bDrawExtendedArea)
	{
		return FALSE;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	for (int i = m_nCount - 1; i >= 0; i--)
	{
		CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(i);
		ENSURE(pSBP != NULL);

		if (pSBP->nStyle & SBPS_STRETCH)
		{
			rect = rectClient;
			rect.left = pSBP->rect.right;

			return TRUE;
		}
	}

	return FALSE;
}

void CMFCStatusBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CPane::OnShowWindow(bShow, nStatus);

	if (GetParentFrame () != NULL)
	{
		GetParentFrame ()->PostMessage (AFX_WM_CHANGEVISUALMANAGER);
	}
}



