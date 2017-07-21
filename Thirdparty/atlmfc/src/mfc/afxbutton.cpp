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
#include "afxcontrolbarutil.h"
#include "afxribbonres.h"
#include "afxbutton.h"
#include "afxglobals.h"
#include "afxtoolbarimages.h"
#include "afxpopupmenu.h"
#include "afxvisualmanager.h"
#include "afxdrawmanager.h"
#include "afxtooltipmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nImageHorzMargin = 10;
static const int nVertMargin = 5;
static const COLORREF clrDefault = (COLORREF) -1;
static const UINT IdAutoCommand = 1;

BOOL CMFCButton::m_bWinXPTheme = FALSE;
BOOL CMFCButton::m_bWinXPThemeWasChecked = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CMFCButton

IMPLEMENT_DYNCREATE(CMFCButton, CButton)

CMFCButton::CMFCButton()
{
	m_bPushed = FALSE;
	m_bClickiedInside = FALSE;
	m_bHighlighted = FALSE;
	m_bCaptured = FALSE;
	m_nFlatStyle = BUTTONSTYLE_3D;
	m_nAlignStyle = ALIGN_CENTER;
	m_sizeImage = CSize(0, 0);
	m_nStdImageId = (CMenuImages::IMAGES_IDS) -1;
	m_nStdImageDisabledId = (CMenuImages::IMAGES_IDS) -1;
	m_StdImageState = CMenuImages::ImageBlack;
	m_bFullTextTooltip = FALSE;
	m_bRightImage = FALSE;
	m_bTopImage = FALSE;
	m_hCursor = NULL;
	m_sizePushOffset = CSize(2, 2);
	m_bHover = FALSE;
	m_clrRegular = clrDefault;
	m_clrHover = clrDefault;
	m_clrFace = (COLORREF)-1;
	m_bDrawFocus = TRUE;
	m_bTransparent = FALSE;
	m_hFont = NULL;
	m_bDelayFullTextTooltipSet = FALSE;
	m_bGrayDisabled = TRUE;
	m_bChecked = FALSE;
	m_bCheckButton = FALSE;
	m_bRadioButton = FALSE;
	m_bAutoCheck = FALSE;
	m_bHighlightChecked = TRUE;
	m_nAutoRepeatTimeDelay = 0;
	m_bResponseOnButtonDown = FALSE;
	m_bDontUseWinXPTheme = FALSE;
	m_bWasDblClk = FALSE;
	m_pToolTip = NULL;
}

CMFCButton::~CMFCButton()
{
	CleanUp();
}

void CMFCButton::CleanUp()
{
	m_nStdImageId = (CMenuImages::IMAGES_IDS) -1;
	m_nStdImageDisabledId = (CMenuImages::IMAGES_IDS) -1;

	m_sizeImage = CSize(0, 0);

	m_Image.Clear();
	m_ImageHot.Clear();
	m_ImageDisabled.Clear();

	m_ImageChecked.Clear();
	m_ImageCheckedHot.Clear();
	m_ImageCheckedDisabled.Clear();
}

//{{AFX_MSG_MAP(CMFCButton)
BEGIN_MESSAGE_MAP(CMFCButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_CANCELMODE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_ENABLE()
	ON_WM_SIZE()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_SETFONT, &CMFCButton::OnSetFont)
	ON_MESSAGE(WM_GETFONT, &CMFCButton::OnGetFont)
	ON_MESSAGE(BM_GETCHECK, &CMFCButton::OnGetCheck)
	ON_MESSAGE(BM_SETCHECK, &CMFCButton::OnSetCheck)
	ON_MESSAGE(BM_SETIMAGE, &CMFCButton::OnSetImage)
	ON_MESSAGE(BM_GETIMAGE, &CMFCButton::OnGetImage)
	ON_REGISTERED_MESSAGE(AFX_WM_UPDATETOOLTIPS, &CMFCButton::OnUpdateToolTips)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCButton message handlers

void CMFCButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	ENSURE(lpDIS != NULL);
	ENSURE(lpDIS->CtlType == ODT_BUTTON);

	CDC* pDCPaint = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID(pDCPaint);

	CMemDC memDC(*pDCPaint, this);
	CDC* pDC = &memDC.GetDC();

	CRect rectClient = lpDIS->rcItem;

	OnFillBackground(pDC, rectClient);
	OnDrawBorder(pDC, rectClient, lpDIS->itemState);

	// Draw button content:
	OnDraw(pDC, rectClient, lpDIS->itemState);

	if ((lpDIS->itemState & ODS_FOCUS) && m_bDrawFocus)
	{
		OnDrawFocusRect(pDC, rectClient);
	}
}

void CMFCButton::PreSubclassWindow()
{
	InitStyle(GetStyle());

	ModifyStyle(BS_DEFPUSHBUTTON, BS_OWNERDRAW);
	CButton::PreSubclassWindow();
}

BOOL CMFCButton::PreCreateWindow(CREATESTRUCT& cs)
{
	InitStyle(cs.style);

	cs.style |= BS_OWNERDRAW;
	cs.style &= ~BS_DEFPUSHBUTTON;

	return CButton::PreCreateWindow(cs);
}

void CMFCButton::InitStyle(DWORD dwStyle)
{
	switch (dwStyle & 0x0F)
	{
	case BS_CHECKBOX:
		m_bCheckButton = TRUE;
		break ;
	case BS_AUTOCHECKBOX:
		m_bCheckButton = m_bAutoCheck = TRUE;
		break ;
	case BS_RADIOBUTTON:
		m_bRadioButton = TRUE;
		break ;
	case BS_AUTORADIOBUTTON:
		m_bRadioButton = m_bAutoCheck = TRUE;
		break ;
	}

	if (m_bCheckButton || m_bRadioButton)
	{
		switch (dwStyle & BS_CENTER)
		{
		case BS_LEFT:
			m_nAlignStyle = CMFCButton::ALIGN_LEFT;
			break;
		case BS_RIGHT:
			m_nAlignStyle = CMFCButton::ALIGN_RIGHT;
			break;
		case BS_CENTER:
			m_nAlignStyle = CMFCButton::ALIGN_CENTER;
			break;
		}
	}

	if (!m_bWinXPThemeWasChecked)
	{
		if (!m_bWinXPTheme)
		{
			EnableWindowsTheming(AfxFindResourceHandle(MAKEINTRESOURCE(1), _T("24")) != NULL);
		}

		m_bWinXPThemeWasChecked = TRUE;
	}
}

BOOL CMFCButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMFCButton::OnFillBackground(CDC* pDC, const CRect& rectClient)
{
	if (m_bTransparent)
	{
		// Copy background from the parent window
		afxGlobalData.DrawParentBackground(this, pDC);
	}
	else
	{
		if (m_clrFace == (COLORREF)-1)
		{
			pDC->FillRect(rectClient, &afxGlobalData.brBtnFace);
		}
		else
		{
			pDC->FillSolidRect(rectClient, m_clrFace);
		}
	}

	if (m_bChecked && m_bHighlightChecked && !(m_bPushed && m_bHighlighted))
	{
		CDrawingManager dm(*pDC);
		dm.HighlightRect(rectClient);
	}
}

void CMFCButton::OnDraw(CDC* pDC, const CRect& rect, UINT uiState)
{
	CRect rectText = rect;
	CRect rectImage = rect;

	CString strText;
	GetWindowText(strText);

	if (m_sizeImage.cx != 0)
	{
		if (!strText.IsEmpty())
		{
			if (m_bTopImage)
			{
				rectImage.bottom = rectImage.top + m_sizeImage.cy + GetVertMargin();
				rectText.top = rectImage.bottom;
				rectText.bottom -= GetVertMargin();
			}
			else if (m_bRightImage)
			{
				rectText.right -= m_sizeImage.cx + GetImageHorzMargin() / 2;
				rectImage.left = rectText.right;
				rectImage.right -= GetImageHorzMargin() / 2;
			}
			else
			{
				rectText.left +=  m_sizeImage.cx + GetImageHorzMargin() / 2;
				rectImage.left += GetImageHorzMargin() / 2;
				rectImage.right = rectText.left;
			}
		}

		// Center image:
		rectImage.DeflateRect((rectImage.Width() - m_sizeImage.cx) / 2, max(0, (rectImage.Height() - m_sizeImage.cy) / 2));
	}
	else
	{
		rectImage.SetRectEmpty();
	}

	// Draw text:
	CFont* pOldFont = SelectFont(pDC);
	ENSURE(pOldFont != NULL);

	pDC->SetBkMode(TRANSPARENT);
	COLORREF clrText = m_clrRegular == clrDefault ? afxGlobalData.clrBtnText : m_clrRegular;

	if (m_bHighlighted && m_clrHover != clrDefault)
	{
		clrText = m_clrHover;
	}

	UINT uiDTFlags = DT_END_ELLIPSIS;
	BOOL bIsSingleLine = FALSE;

	if (strText.Find(_T('\n')) < 0)
	{
		uiDTFlags |= DT_VCENTER | DT_SINGLELINE;
		bIsSingleLine = TRUE;
	}
	else
	{
		rectText.DeflateRect(0, GetVertMargin() / 2);
	}

	switch (m_nAlignStyle)
	{
	case ALIGN_LEFT:
		uiDTFlags |= DT_LEFT;
		rectText.left += GetImageHorzMargin() / 2;
		break;

	case ALIGN_RIGHT:
		uiDTFlags |= DT_RIGHT;
		rectText.right -= GetImageHorzMargin() / 2;
		break;

	case ALIGN_CENTER:
		uiDTFlags |= DT_CENTER;
	}

	if (GetExStyle() & WS_EX_LAYOUTRTL)
	{
		uiDTFlags |= DT_RTLREADING;
	}

	if ((uiState & ODS_DISABLED) && m_bGrayDisabled)
	{
		pDC->SetTextColor(afxGlobalData.clrBtnHilite);

		CRect rectShft = rectText;
		rectShft.OffsetRect(1, 1);
		OnDrawText(pDC, rectShft, strText, uiDTFlags, uiState);

		clrText = afxGlobalData.clrGrayedText;
	}

	pDC->SetTextColor(clrText);

	if (m_bDelayFullTextTooltipSet)
	{
		BOOL bIsFullText = pDC->GetTextExtent(strText).cx <= rectText.Width();
		SetTooltip(bIsFullText || !bIsSingleLine ? NULL :(LPCTSTR) strText);
		m_bDelayFullTextTooltipSet = FALSE;
	}

	OnDrawText(pDC, rectText, strText, uiDTFlags, uiState);

	// Draw image:
	if (!rectImage.IsRectEmpty())
	{
		if (m_nStdImageId != (CMenuImages::IMAGES_IDS) -1)
		{
			CMenuImages::IMAGES_IDS id = m_nStdImageId;

			if ((uiState & ODS_DISABLED) && m_bGrayDisabled && m_nStdImageDisabledId != 0)
			{
				id = m_nStdImageDisabledId;
			}

			CMenuImages::Draw(pDC, id, rectImage.TopLeft(), m_StdImageState);
		}
		else
		{
			BOOL bIsDisabled = (uiState & ODS_DISABLED) && m_bGrayDisabled;

			CMFCToolBarImages& imageChecked = (bIsDisabled && m_ImageCheckedDisabled.GetCount() != 0) ? m_ImageCheckedDisabled :
				(m_bHighlighted && m_ImageCheckedHot.GetCount() != 0) ? m_ImageCheckedHot : m_ImageChecked;

			CMFCToolBarImages& image = (bIsDisabled && m_ImageDisabled.GetCount() != 0) ? m_ImageDisabled :
				(m_bHighlighted && m_ImageHot.GetCount() != 0) ? m_ImageHot : m_Image;

			if (m_bChecked && imageChecked.GetCount() != 0)
			{
				CAfxDrawState ds;

				imageChecked.PrepareDrawImage(ds);
				imageChecked.Draw(pDC, rectImage.left, rectImage.top, 0, FALSE, bIsDisabled && m_ImageCheckedDisabled.GetCount() == 0);
				imageChecked.EndDrawImage(ds);
			}
			else if (image.GetCount() != 0)
			{
				CAfxDrawState ds;

				image.PrepareDrawImage(ds);
				image.Draw(pDC, rectImage.left, rectImage.top, 0, FALSE, bIsDisabled && m_ImageDisabled.GetCount() == 0);
				image.EndDrawImage(ds);
			}
		}
	}

	pDC->SelectObject(pOldFont);
}

void CMFCButton::OnDrawText(CDC* pDC, const CRect& rect, const CString& strText, UINT uiDTFlags, UINT /*uiState*/)
{
	ASSERT_VALID(pDC);

	CRect rectText = rect;
	pDC->DrawText(strText, rectText, uiDTFlags);
}

void CMFCButton::SetImage(HICON hIconCold, BOOL bAutoDestroy, HICON hIconHot, HICON hIconDisabled, BOOL bAlphaBlend)
{
	SetImageInternal(hIconCold, bAutoDestroy, hIconHot, FALSE /* Not checked */, hIconDisabled, bAlphaBlend);
}

void CMFCButton::SetImage(HBITMAP hBitmapCold, BOOL bAutoDestroy, HBITMAP hBitmapHot, BOOL bMap3dColors, HBITMAP hBitmapDisabled)
{
	SetImageInternal(hBitmapCold, bAutoDestroy, hBitmapHot, bMap3dColors, FALSE /* Not checked */, hBitmapDisabled);
}

void CMFCButton::SetImage(UINT uiBmpResId, UINT uiBmpHotResId, UINT uiBmpDsblResID)
{
	SetImageInternal(uiBmpResId, uiBmpHotResId, FALSE /* Not checked */, uiBmpDsblResID);
}

void CMFCButton::SetCheckedImage(HICON hIconCold, BOOL bAutoDestroy, HICON hIconHot, HICON hIconDisabled, BOOL bAlphaBlend)
{
	SetImageInternal(hIconCold, bAutoDestroy, hIconHot, TRUE /* Checked */, hIconDisabled, bAlphaBlend);
}

void CMFCButton::SetCheckedImage(HBITMAP hBitmapCold, BOOL bAutoDestroy, HBITMAP hBitmapHot, BOOL bMap3dColors, HBITMAP hBitmapDisabled)
{
	SetImageInternal(hBitmapCold, bAutoDestroy, hBitmapHot, bMap3dColors, TRUE /* Checked */, hBitmapDisabled);
}

void CMFCButton::SetCheckedImage(UINT uiBmpResId, UINT uiBmpHotResId, UINT uiBmpDsblResID)
{
	SetImageInternal(uiBmpResId, uiBmpHotResId, TRUE /* Checked */, uiBmpDsblResID);
}

void CMFCButton::SetImageInternal(HICON hIconCold, BOOL bAutoDestroy, HICON hIconHot, BOOL bChecked, HICON hIconDisabled, BOOL bAlphaBlend)
{
	ClearImages(bChecked);

	if (hIconCold == NULL)
	{
		return;
	}

	const int nCount = hIconDisabled == NULL ? 2 : 3;

	for (int i = 0; i < nCount; i++)
	{
		HICON hIcon = (i == 0) ? hIconCold :(i == 1) ? hIconHot : hIconDisabled;
		CMFCToolBarImages& image = bChecked ?((i == 0) ? m_ImageChecked :(i == 1) ? m_ImageCheckedHot : m_ImageCheckedDisabled) :
			((i == 0) ? m_Image :(i == 1) ? m_ImageHot : m_ImageDisabled);

		if (hIcon == NULL)
		{
			continue;
		}

		ICONINFO info;
		::GetIconInfo(hIcon, &info);

		BITMAP bmp;
		::GetObject(info.hbmColor, sizeof(BITMAP), (LPVOID) &bmp);

		m_sizeImage.cx = bmp.bmWidth;
		m_sizeImage.cy = bmp.bmHeight;

		if (i == 0)
		{
			// Create disabled image from the "cold" icon:
			CDC dcMem;
			dcMem.CreateCompatibleDC(NULL);

			HBITMAP hBmp = (HBITMAP) ::CopyImage(info.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
			if (hBmp == NULL)
			{
				ASSERT(FALSE);
			}
			else
			{
				HBITMAP hOldBmp = (HBITMAP) dcMem.SelectObject(hBmp);

				dcMem.FillRect(CRect(0, 0, m_sizeImage.cx, m_sizeImage.cy), &afxGlobalData.brBtnFace);

				::DrawIconEx(dcMem.GetSafeHdc(), 0, 0, hIcon, m_sizeImage.cx, m_sizeImage.cy, 0, NULL, DI_NORMAL);

				dcMem.SelectObject(hOldBmp);
				::DeleteObject(hBmp);
			}
		}

		::DeleteObject(info.hbmColor);
		::DeleteObject(info.hbmMask);

		image.SetImageSize(CSize(bmp.bmWidth, bmp.bmHeight));

		if (!bAlphaBlend)
		{
			image.SetTransparentColor(afxGlobalData.clrBtnFace);
		}

		image.AddIcon(hIcon, bAlphaBlend);
	}

	if (bAutoDestroy)
	{
		if (hIconCold != NULL)
		{
			::DestroyIcon(hIconCold);
		}

		if (hIconHot != NULL)
		{
			::DestroyIcon(hIconHot);
		}

		if (hIconDisabled != NULL)
		{
			::DestroyIcon(hIconDisabled);
		}
	}
}

void CMFCButton::SetImageInternal(HBITMAP hBitmapCold, BOOL bAutoDestroy, HBITMAP hBitmapHot, BOOL bMap3dColors, BOOL bChecked, HBITMAP hBitmapDisabled)
{
	ClearImages(bChecked);

	if (hBitmapCold == NULL)
	{
		return;
	}

	const int nCount = hBitmapDisabled == NULL ? 2 : 3;

	for (int i = 0; i < nCount; i++)
	{
		HBITMAP hBitmap = (i == 0) ? hBitmapCold :(i == 1) ? hBitmapHot : hBitmapDisabled;
		CMFCToolBarImages& image = bChecked ?((i == 0) ? m_ImageChecked :(i == 1) ? m_ImageCheckedHot : m_ImageCheckedDisabled) :
			((i == 0) ? m_Image :(i == 1) ? m_ImageHot : m_ImageDisabled);

		if (hBitmap == NULL)
		{
			break;
		}

		BITMAP bmp;
		::GetObject(hBitmap, sizeof(BITMAP), (LPVOID) &bmp);

		BOOL bMap3dColorsCurr = bMap3dColors ||(bmp.bmBitsPixel > 8 && bmp.bmBitsPixel < 32);
		BOOL bAlpha = !bMap3dColors && bmp.bmBitsPixel == 32 && CMFCToolBarImages::Is32BitTransparencySupported();

		if (i == 0)
		{
			m_sizeImage.cx = bmp.bmWidth;
			m_sizeImage.cy = bmp.bmHeight;
		}
		else
		{
			// Hot and cold bitmaps should have the same size!
			ASSERT(m_sizeImage.cx == bmp.bmWidth);
			ASSERT(m_sizeImage.cy == bmp.bmHeight);
		}

		if (bAlpha)
		{
			CMFCToolBarImages::PreMultiplyAlpha(hBitmap, TRUE);
		}

		image.SetImageSize(CSize(bmp.bmWidth, bmp.bmHeight));
		image.SetTransparentColor(bMap3dColorsCurr ? RGB(192, 192, 192) : bAlpha ?(COLORREF) -1 : afxGlobalData.clrBtnFace);
		image.AddImage(hBitmap, TRUE);
	}

	if (bAutoDestroy)
	{
		if (hBitmapHot != NULL)
		{
			::DeleteObject(hBitmapHot);
		}

		if (hBitmapCold != NULL)
		{
			::DeleteObject(hBitmapCold);
		}

		if (hBitmapDisabled != NULL)
		{
			::DeleteObject(hBitmapDisabled);
		}
	}
}

static HBITMAP __stdcall ButtonLoadBitmap(UINT uiBmpResId)
{
	if (uiBmpResId == 0)
	{
		return NULL;
	}

	LPCTSTR lpszResourceName = MAKEINTRESOURCE(uiBmpResId);
	ENSURE(lpszResourceName != NULL);

	HBITMAP hbmp = NULL;

	// Try to load PNG image first:
	CPngImage pngImage;
	if (pngImage.Load(lpszResourceName))
	{
		hbmp = (HBITMAP) pngImage.Detach();
	}
	else
	{
		HINSTANCE hinstRes = AfxFindResourceHandle(lpszResourceName, RT_BITMAP);
		if (hinstRes == NULL)
		{
			return NULL;
		}

		UINT uiLoadImageFlags = LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS;

		hbmp = (HBITMAP) ::LoadImage(hinstRes, lpszResourceName, IMAGE_BITMAP, 0, 0, uiLoadImageFlags);
	}

	return hbmp;
}

void CMFCButton::SetImageInternal(UINT uiBmpResId, UINT uiBmpHotResId, BOOL bChecked, UINT uiBmpDsblResID)
{
	ClearImages(bChecked);

	if (uiBmpResId == 0)
	{
		return;
	}

	HBITMAP hbmp = ButtonLoadBitmap(uiBmpResId);
	HBITMAP hbmpHot = ButtonLoadBitmap(uiBmpHotResId);
	HBITMAP hbmpDisabled = ButtonLoadBitmap(uiBmpDsblResID);

	SetImageInternal(hbmp, TRUE /* AutoDestroy */, hbmpHot, FALSE, bChecked, hbmpDisabled);
}

void CMFCButton::SetStdImage(CMenuImages::IMAGES_IDS id, CMenuImages::IMAGE_STATE state, CMenuImages::IMAGES_IDS idDisabled)
{
	CleanUp();

	m_sizeImage = CMenuImages::Size();
	m_nStdImageId = id;
	m_nStdImageDisabledId = idDisabled;
	m_StdImageState = state;
}

void CMFCButton::OnCancelMode()
{
	CButton::OnCancelMode();

	if (m_bCaptured)
	{
		ReleaseCapture();

		m_bCaptured = FALSE;
		m_bPushed = FALSE;
		m_bClickiedInside = FALSE;
		m_bHighlighted = FALSE;
		m_bHover = FALSE;

		Invalidate();
		UpdateWindow();
	}

	if (m_nAutoRepeatTimeDelay >= 0)
	{
		KillTimer(IdAutoCommand);
	}
}

void CMFCButton::OnMouseMove(UINT nFlags, CPoint point)
{
	m_bHover = FALSE;

	if ((nFlags & MK_LBUTTON) || m_nFlatStyle != BUTTONSTYLE_3D ||
		(m_bWinXPTheme && !m_bDontUseWinXPTheme))
	{
		BOOL bRedraw = FALSE;

		CRect rectClient;
		GetClientRect(rectClient);

		if (rectClient.PtInRect(point))
		{
			m_bHover = TRUE;

			if (!m_bHighlighted)
			{
				m_bHighlighted = TRUE;
				bRedraw = TRUE;
			}

			if ((nFlags & MK_LBUTTON) && !m_bPushed && m_bClickiedInside)
			{
				m_bPushed = TRUE;
				bRedraw = TRUE;
			}

			if (!m_bCaptured)
			{
				SetCapture();
				m_bCaptured = TRUE;
				bRedraw = TRUE;
			}
		}
		else
		{
			if (nFlags & MK_LBUTTON)
			{
				if (m_bPushed)
				{
					m_bPushed = FALSE;
					bRedraw = TRUE;
				}
			}
			else if (m_bHighlighted)
			{
				m_bHighlighted = FALSE;
				bRedraw = TRUE;
			}

			if (m_bCaptured && !(nFlags & MK_LBUTTON))
			{
				ReleaseCapture();
				m_bCaptured = FALSE;

				bRedraw = TRUE;
			}
		}

		if (bRedraw)
		{
			Invalidate();
			UpdateWindow();
		}
	}

	CButton::OnMouseMove(nFlags, point);
}

void CMFCButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bResponseOnButtonDown)
	{
		CWnd* pParent = GetParent();
		if (pParent != NULL)
		{
			pParent->SendMessage( WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM) m_hWnd);
		}
	}
	else
	{
		m_bPushed = TRUE;
		m_bClickiedInside = TRUE;
		m_bHighlighted = TRUE;

		if (!m_bCaptured)
		{
			SetCapture();
			m_bCaptured = TRUE;
		}

		Invalidate();
		UpdateWindow();

		if (m_nAutoRepeatTimeDelay > 0)
		{
			SetTimer(IdAutoCommand, m_nAutoRepeatTimeDelay, NULL);
		}
	}

	CButton::OnLButtonDown(nFlags, point);
}

void CMFCButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	BOOL bClicked = m_bPushed && m_bClickiedInside && m_bHighlighted;

	m_bPushed = FALSE;
	m_bClickiedInside = FALSE;
	m_bHighlighted = FALSE;

	if (bClicked && m_bAutoCheck)
	{
		if (m_bCheckButton)
		{
			m_bChecked = !m_bChecked;
		}
		else if (m_bRadioButton && !m_bChecked)
		{
			m_bChecked = TRUE;
			UncheckRadioButtonsInGroup();
		}
	}

	HWND hWnd = GetSafeHwnd();

	if (m_bWasDblClk)
	{
		m_bWasDblClk = FALSE;
		CWnd* pParent = GetParent();
		if (pParent != NULL)
		{
			pParent->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM) m_hWnd);
		}
	}

	if (!::IsWindow(hWnd))
	{
		// The button was destroyed after send message
		return;
	}

	RedrawWindow();

	CButton::OnLButtonUp(nFlags, point);

	if (!::IsWindow(hWnd))
	{
		// The button was destroyed after the OnLButtonUp handler
		return;
	}

	if (m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured = FALSE;
	}

	if (m_nAutoRepeatTimeDelay > 0)
	{
		KillTimer(IdAutoCommand);
	}

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		m_pToolTip->Pop();
	}
}

CSize CMFCButton::SizeToContent(BOOL bCalcOnly)
{
	ENSURE(GetSafeHwnd() != NULL);

	CClientDC dc(this);

	CFont* pOldFont = SelectFont(&dc);
	ENSURE(pOldFont != NULL);

	CString strText;
	GetWindowText(strText);

	CSize sizeText(0, 0);

	if (strText.Find(_T('\n')) < 0)
	{
		sizeText = dc.GetTextExtent(strText);
	}
	else
	{
		CRect rectText;
		GetClientRect(rectText);

		dc.DrawText(strText, rectText, DT_CALCRECT);
		sizeText = rectText.Size();
	}

	int cx = 0;
	int cy = 0;

	if (m_bTopImage)
	{
		cx = max(sizeText.cx, m_sizeImage.cx) + GetImageHorzMargin();
		if (sizeText.cx > 0)
		{
			cx += GetImageHorzMargin();
		}

		cy = sizeText.cy + m_sizeImage.cy + GetVertMargin();
		if (m_sizeImage.cy > 0)
		{
			cy += GetVertMargin();
		}
		if (sizeText.cy > 0)
		{
			cy += GetVertMargin();
		}
	}
	else
	{
		cx = sizeText.cx + m_sizeImage.cx + GetImageHorzMargin();
		if (sizeText.cx > 0)
		{
			cx += GetImageHorzMargin();
		}

		cy = max(sizeText.cy, m_sizeImage.cy) + GetVertMargin() * 2;
	}

	if (!bCalcOnly)
	{
		SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	dc.SelectObject(pOldFont);

	return CSize(cx, cy);
}

BOOL CMFCButton::PreTranslateMessage(MSG* pMsg)
{
	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP || pMsg->message == WM_MOUSEMOVE)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && CMFCPopupMenu::GetActiveMenu() == NULL)
	{
		CWnd* pParent = GetParent();
		if (pParent != NULL)
		{
			pParent->SendMessage( WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM) m_hWnd);
			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN && m_bAutoCheck && GetParent() != NULL)
	{
		switch (pMsg->wParam)
		{
		case VK_SPACE:
			if (m_bCheckButton)
			{
				m_bChecked = !m_bChecked;

				RedrawWindow();

				CWnd* pWndParent = GetParent();
				ASSERT_VALID(pWndParent);

				::SendMessage(pWndParent->GetSafeHwnd(), WM_COMMAND, MAKELONG(::GetWindowLong(m_hWnd, GWL_ID), BN_CLICKED), (LPARAM) m_hWnd);

				return TRUE ;
			}
			break;

		case VK_UP:
		case VK_LEFT:
			if (CheckNextPrevRadioButton(FALSE))
			{
				return TRUE;
			}
			break;

		case VK_DOWN:
		case VK_RIGHT:
			if (CheckNextPrevRadioButton(TRUE))
			{
				return TRUE;
			}
			break ;
		}
	}

	return CButton::PreTranslateMessage(pMsg);
}

void CMFCButton::SetTooltip(LPCTSTR lpszToolTipText)
{
	ENSURE(GetSafeHwnd() != NULL);

	if (lpszToolTipText == NULL)
	{
		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->Activate(FALSE);
		}
	}
	else
	{
		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->UpdateTipText(lpszToolTipText, this);
		}
		else
		{
			if (CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_BUTTON))
			{
				m_pToolTip->AddTool(this, lpszToolTipText);
			}
		}

		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->Activate(TRUE);
		}
	}
}

void CMFCButton::SetMouseCursor(HCURSOR hcursor)
{
	m_hCursor = hcursor;
}

void CMFCButton::SetMouseCursorHand()
{
	SetMouseCursor(afxGlobalData.GetHandCursor());
}

BOOL CMFCButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_hCursor != NULL)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (rectClient.PtInRect(ptCursor))
		{
			::SetCursor(m_hCursor);
			return TRUE;
		}
	}

	return CButton::OnSetCursor(pWnd, nHitTest, message);
}

void CMFCButton::OnDrawFocusRect(CDC* pDC, const CRect& rectClient)
{
	ASSERT_VALID(pDC);

	CRect rectFocus = rectClient;
	rectFocus.DeflateRect(1, 1);

	COLORREF clrBckgr = (m_clrFace == (COLORREF)-1) ? afxGlobalData.clrBtnFace : m_clrFace;

	if (!m_bWinXPTheme || m_bDontUseWinXPTheme)
	{
		rectFocus.DeflateRect(1, 1);
		pDC->Draw3dRect(rectFocus, clrBckgr, clrBckgr);
	}

	pDC->DrawFocusRect(rectFocus);
}

void CMFCButton::OnDrawParentBackground(CDC* pDC, CRect rectClient)
{
	ASSERT_VALID(pDC);
	afxGlobalData.DrawParentBackground(this, pDC, rectClient);
}

void CMFCButton::OnEnable(BOOL bEnable)
{
	if (!bEnable)
	{
		// control disabled
		m_bPushed = FALSE;
		m_bClickiedInside = FALSE;
		m_bHighlighted = FALSE;

		if (m_bCaptured)
		{
			ReleaseCapture();
			m_bCaptured = FALSE;
		}
	}

	RedrawWindow();
	CButton::OnEnable(bEnable);
}

void CMFCButton::SetFaceColor(COLORREF crFace, BOOL bRedraw)
{
	m_clrFace = crFace;

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
}

CFont* CMFCButton::SelectFont(CDC* pDC)
{
	ASSERT_VALID(pDC);

	if (m_hFont != NULL && ::GetObjectType(m_hFont) != OBJ_FONT)
	{
		m_hFont = NULL;
	}

	CFont* pOldFont = m_hFont == NULL ? (CFont*) pDC->SelectStockObject(DEFAULT_GUI_FONT) : pDC->SelectObject(CFont::FromHandle(m_hFont));

	ENSURE(pOldFont != NULL);
	return pOldFont;
}

afx_msg LRESULT CMFCButton::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	BOOL bRedraw = (BOOL) LOWORD(lParam);

	m_hFont = (HFONT) wParam;

	if (bRedraw)
	{
		Invalidate();
		UpdateWindow();
	}

	return 0;
}

afx_msg LRESULT CMFCButton::OnGetFont(WPARAM, LPARAM)
{
	return(LRESULT) m_hFont;
}

void CMFCButton::EnableMenuFont(BOOL bOn, BOOL bRedraw)
{
	m_hFont = bOn ?(HFONT) afxGlobalData.fontRegular.GetSafeHandle() : NULL;

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
}

void CMFCButton::EnableFullTextTooltip(BOOL bOn)
{
	m_bFullTextTooltip = bOn;
	m_bDelayFullTextTooltipSet = bOn;
}

void CMFCButton::OnSize(UINT nType, int cx, int cy)
{
	m_bDelayFullTextTooltipSet = m_bFullTextTooltip;
	CButton::OnSize(nType, cx, cy);
}

void CMFCButton::OnDrawBorder(CDC* pDC, CRect& rectClient, UINT uiState)
{
	DrawBorder(pDC, rectClient, uiState);
}

void CMFCButton::OnKillFocus(CWnd* pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);

	if (m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured = FALSE;
	}

	m_bPushed = FALSE;
	m_bClickiedInside = FALSE;
	m_bHighlighted = FALSE;
	m_bHover = FALSE;

	Invalidate();
	UpdateWindow();
}

void CMFCButton::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_bPushed = TRUE;
	m_bClickiedInside = TRUE;
	m_bHighlighted = TRUE;

	Invalidate();
	UpdateWindow();

	CButton::OnLButtonDblClk(nFlags, point);
	m_bWasDblClk = TRUE;
}

void __stdcall CMFCButton::EnableWindowsTheming(BOOL bEnable/* = TRUE*/)
{
	m_bWinXPTheme = bEnable;
}

LRESULT CMFCButton::OnGetCheck(WPARAM, LPARAM)
{
	if (m_bCheckButton || m_bRadioButton)
	{
		return m_bChecked ? BST_CHECKED : BST_UNCHECKED;
	}

	return 0;
}

LRESULT CMFCButton::OnSetCheck(WPARAM fCheck, LPARAM)
{
	ASSERT(fCheck != BST_INDETERMINATE);

	if ((m_bCheckButton || m_bRadioButton) &&(!m_bChecked) != (fCheck == BST_UNCHECKED))
	{
		m_bChecked = fCheck != BST_UNCHECKED;

		if (m_bRadioButton)
		{
			UncheckRadioButtonsInGroup();
		}

		Invalidate();
		UpdateWindow();
	}

	return 0 ;
}

void CMFCButton::ClearImages(BOOL bChecked)
{
	m_nStdImageId = (CMenuImages::IMAGES_IDS) -1;
	m_nStdImageDisabledId = (CMenuImages::IMAGES_IDS) -1;
	m_sizeImage = CSize(0, 0);

	if (bChecked)
	{
		m_ImageChecked.Clear();
		m_ImageCheckedHot.Clear();
		m_ImageCheckedDisabled.Clear();
	}
	else
	{
		m_Image.Clear();
		m_ImageHot.Clear();
		m_ImageDisabled.Clear();
	}
}

BOOL CMFCButton::CheckNextPrevRadioButton(BOOL bNext)
{
	ASSERT_VALID(this);

	if (!m_bRadioButton)
	{
		return FALSE;
	}

	CWnd* pWndParent = GetParent();
	ASSERT_VALID(pWndParent);

	CMFCButton* pBtn = NULL;

	for (CWnd* pWnd = pWndParent->GetNextDlgGroupItem(this, !bNext); pWnd != this; pWnd = pWndParent->GetNextDlgGroupItem(pWnd, !bNext))
	{
		if ((pBtn = DYNAMIC_DOWNCAST(CMFCButton, pWnd)) != NULL && pBtn->m_bRadioButton && (pBtn->GetStyle() &(WS_DISABLED | WS_VISIBLE)) == WS_VISIBLE)
		{
			break;
		}
	}

	if (pBtn != NULL && pBtn != this && !pBtn->m_bChecked)
	{
		pBtn->SetCheck(TRUE);
		pBtn->SetFocus();

		::SendMessage(pBtn->GetParent()->GetSafeHwnd(), WM_COMMAND, MAKELONG(::GetWindowLong(pBtn->m_hWnd, GWL_ID), BN_CLICKED), (LPARAM) pBtn->m_hWnd);
		return TRUE;
	}

	return FALSE;
}

void CMFCButton::UncheckRadioButtonsInGroup()
{
	CWnd* pWndParent = GetParent();
	if (pWndParent == NULL)
	{
		return;
	}

	ASSERT_VALID(pWndParent);

	if (!IsWindowEnabled())
	{
		return;
	}

	// Walk through group and clear radio buttons check state
	for (CWnd * pCtl = pWndParent->GetNextDlgGroupItem(this); pCtl != this && pCtl != NULL; pCtl = pWndParent->GetNextDlgGroupItem(pCtl))
	{
		CMFCButton* pBtn = DYNAMIC_DOWNCAST(CMFCButton, pCtl);

		if (pBtn != NULL && pBtn->m_bRadioButton && pBtn->m_bChecked)
		{
			pBtn->m_bChecked = FALSE;
			pBtn->RedrawWindow();
		}
	}
}

void CMFCButton::SetAutorepeatMode(int nTimeDelay)
{
	ASSERT(nTimeDelay >= 0);
	m_nAutoRepeatTimeDelay = nTimeDelay;
}

void CMFCButton::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IdAutoCommand)
	{
		if (m_bPushed && m_bHighlighted)
		{
			CWnd* pParent = GetParent();
			if (pParent != NULL)
			{
				pParent->SendMessage( WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM) m_hWnd);
			}
		}
	}

	CButton::OnTimer(nIDEvent);
}

void CMFCButton::DrawBorder(CDC* pDC, CRect& rectClient, UINT uiState)
{
	ASSERT_VALID(pDC);

	BOOL bBorderIsReady = FALSE;

	// Draw 3d border:
	if (m_nFlatStyle != BUTTONSTYLE_NOBORDERS)
	{
		if (m_bWinXPTheme && !m_bDontUseWinXPTheme && CMFCVisualManager::GetInstance()->DrawPushButtonWinXP(pDC, rectClient, this, uiState))
		{
			bBorderIsReady = TRUE;
		}

		if (m_bPushed && m_bHighlighted ||(uiState & ODS_SELECTED) || m_bChecked)
		{
			if (!bBorderIsReady)
			{
				pDC->Draw3dRect(rectClient, afxGlobalData.clrBtnDkShadow, afxGlobalData.clrBtnHilite);

				rectClient.DeflateRect(1, 1);

				if (m_nFlatStyle != BUTTONSTYLE_FLAT)
				{
					pDC->Draw3dRect(rectClient, afxGlobalData.clrBtnShadow, afxGlobalData.clrBtnLight);
				}

				rectClient.DeflateRect(1, 1);
			}
			else
			{
				rectClient.DeflateRect(2, 2);
			}

			if (!m_bWinXPTheme || m_bDontUseWinXPTheme)
			{
				rectClient.left += m_sizePushOffset.cx;
				rectClient.top += m_sizePushOffset.cy;
			}
		}
		else if (!bBorderIsReady &&(m_nFlatStyle != BUTTONSTYLE_FLAT || m_bHighlighted))
		{
			pDC->Draw3dRect(rectClient, afxGlobalData.clrBtnHilite, afxGlobalData.clrBtnDkShadow);
			rectClient.DeflateRect(1, 1);

			if (m_nFlatStyle == BUTTONSTYLE_3D || (m_nFlatStyle == BUTTONSTYLE_SEMIFLAT && m_bHighlighted))
			{
				pDC->Draw3dRect(rectClient, afxGlobalData.clrBtnLight, afxGlobalData.clrBtnShadow);
			}

			rectClient.DeflateRect(1, 1);
		}
		else
		{
			rectClient.DeflateRect(2, 2);
		}
	}
	else
	{
		rectClient.DeflateRect(2, 2);
	}
}

LRESULT CMFCButton::OnSetImage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IMAGE_BITMAP:
		SetImage((HBITMAP) lParam, FALSE);
		break;

	case IMAGE_ICON:
		SetImage((HICON) lParam, FALSE);
		break;

	default:
		TRACE1("Error: unknown image type '%u'\n", (unsigned) wParam);
	}

	return 0;
}

LRESULT CMFCButton::OnGetImage(WPARAM wParam, LPARAM)
{
	switch (wParam)
	{
	case IMAGE_BITMAP:
		return(LRESULT) m_Image.GetImageWell();

	case IMAGE_ICON:
		return(LRESULT) m_Image.ExtractIcon(0);

	default:
		TRACE1("Error: unknown image type '%u'\n", (unsigned) wParam);
	}

	return 0;
}

int CMFCButton::GetImageHorzMargin() const
{
	return nImageHorzMargin;
}

int CMFCButton::GetVertMargin() const
{
	return nVertMargin;
}

void CMFCButton::OnDestroy()
{
	CTooltipManager::DeleteToolTip(m_pToolTip);
	CButton::OnDestroy();
}

LRESULT CMFCButton::OnUpdateToolTips(WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (m_pToolTip->GetSafeHwnd() == NULL)
	{
		return 0;
	}

	if (nTypes & AFX_TOOLTIP_TYPE_BUTTON)
	{
		CString str;
		m_pToolTip->GetText(str, this);

		CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_BUTTON);

		if (m_pToolTip != NULL)
		{
			m_pToolTip->AddTool(this, str);
		}
	}

	return 0;
}



