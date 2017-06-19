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
#include "multimon.h"
#include "afxcontrolbarutil.h"
#include "afxglobals.h"
#include "afxdrawmanager.h"
#include "afxcaptionbutton.h"
#include "afxtabctrl.h"
#include "afxvisualmanagervs2005.h"
#include "afxautohidebutton.h"
#include "afxtoolbar.h"
#include "afxtoolbarmenubutton.h"
#include "afxstatusbar.h"
#include "afxdockingmanager.h"
#include "afxtabbedpane.h"
#include "afxpropertygridctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CMFCVisualManagerVS2005::m_bRoundedAutohideButtons = FALSE;

IMPLEMENT_DYNCREATE(CMFCVisualManagerVS2005, CMFCVisualManagerOffice2003)

// Construction/Destruction
CMFCVisualManagerVS2005::CMFCVisualManagerVS2005()
{
	m_bAlwaysFillTab = TRUE;
	m_b3DTabWideBorder = FALSE;
	m_bShdowDroppedDownMenuButton = TRUE;
	m_bDrawLastTabLine = FALSE;
	m_colorActiveTabBorder = (COLORREF)-1;
	m_bFrameMenuCheckedItems = TRUE;

	CDockingManager::EnableDockSiteMenu();
	CDockingManager::SetDockingMode(DT_SMART);
	CMFCAutoHideButton::m_bOverlappingTabs = FALSE;
}

CMFCVisualManagerVS2005::~CMFCVisualManagerVS2005()
{
}

void CMFCVisualManagerVS2005::OnUpdateSystemColors()
{
	BOOL bDefaultWinXPColors = m_bDefaultWinXPColors;
	m_clrPressedButtonBorder = (COLORREF)-1;
	m_CurrAppTheme = GetStandardWindowsTheme();

	if (m_CurrAppTheme != WinXpTheme_Silver)
	{
		m_bDefaultWinXPColors = FALSE;
	}

	CMFCVisualManagerOffice2003::OnUpdateSystemColors();

	if (!bDefaultWinXPColors)
	{
		return;
	}

	COLORREF clrMenuButtonDroppedDown = m_clrBarBkgnd;
	COLORREF clrMenuItemCheckedHighlight = m_clrHighlightDn;

	if (m_hThemeComboBox == NULL || m_pfGetThemeColor == NULL || (*m_pfGetThemeColor)(m_hThemeComboBox, 5, 0, 3801, &m_colorActiveTabBorder) != S_OK)
	{
		m_colorActiveTabBorder = (COLORREF)-1;
	}

	if (afxGlobalData.m_nBitsPerPixel > 8 && !afxGlobalData.IsHighContrastMode())
	{
		m_clrCustomizeButtonGradientLight = CDrawingManager::SmartMixColors(m_clrCustomizeButtonGradientDark, afxGlobalData.clrBarFace, 1.5, 1, 1);

		if (m_CurrAppTheme == WinXpTheme_Blue || m_CurrAppTheme == WinXpTheme_Olive)
		{
			m_clrToolBarGradientDark = CDrawingManager::PixelAlpha(m_clrToolBarGradientDark, 83);

			m_clrToolBarGradientLight = CDrawingManager::SmartMixColors(GetBaseThemeColor(), GetThemeColor(m_hThemeWindow, COLOR_WINDOW), 1., 3, 2);
		}

		if (m_CurrAppTheme == WinXpTheme_Blue)
		{
			m_clrCustomizeButtonGradientDark = CDrawingManager::PixelAlpha(m_clrCustomizeButtonGradientDark, 90);
			m_clrCustomizeButtonGradientLight = CDrawingManager::PixelAlpha(m_clrCustomizeButtonGradientLight, 115);

			m_clrToolBarBottomLine = CDrawingManager::PixelAlpha(m_clrToolBarBottomLine, 85);
		}
		else if (m_CurrAppTheme == WinXpTheme_Olive)
		{
			m_clrToolBarBottomLine = CDrawingManager::PixelAlpha(m_clrToolBarBottomLine, 110);

			m_clrCustomizeButtonGradientDark = m_clrToolBarBottomLine;

			m_clrCustomizeButtonGradientLight = CDrawingManager::PixelAlpha(m_clrCustomizeButtonGradientLight, 120);

			m_clrHighlightDn = afxGlobalData.clrHilite;

			m_clrHighlight = CDrawingManager::PixelAlpha(m_clrHighlightDn, 124);
			m_clrHighlightChecked = CDrawingManager::PixelAlpha(GetThemeColor(m_hThemeWindow, 27 /*COLOR_GRADIENTACTIVECAPTION*/), 98);

			m_brHighlight.DeleteObject();
			m_brHighlightDn.DeleteObject();

			m_brHighlight.CreateSolidBrush(m_clrHighlight);
			m_brHighlightDn.CreateSolidBrush(m_clrHighlightDn);

			m_brHighlightChecked.DeleteObject();
			m_brHighlightChecked.CreateSolidBrush(m_clrHighlightChecked);
		}
		else if (m_CurrAppTheme != WinXpTheme_Silver)
		{
			m_clrToolBarBottomLine = m_clrToolBarGradientDark;
		}

		clrMenuButtonDroppedDown = CDrawingManager::PixelAlpha(m_clrBarBkgnd, 107);

		clrMenuItemCheckedHighlight = GetThemeColor(m_hThemeWindow, COLOR_HIGHLIGHT);

		if (m_CurrAppTheme == WinXpTheme_Blue || m_CurrAppTheme == WinXpTheme_Olive)
		{
			m_clrBarGradientLight = CDrawingManager::PixelAlpha(m_clrToolBarGradientLight, 95);
			m_clrBarGradientDark = CDrawingManager::PixelAlpha(m_clrBarGradientDark, 97);
		}

		m_clrToolbarDisabled = CDrawingManager::SmartMixColors(m_clrToolBarGradientDark, m_clrToolBarGradientLight, .92, 1, 2);
		m_clrPressedButtonBorder = CDrawingManager::SmartMixColors(m_clrMenuItemBorder, afxGlobalData.clrBarDkShadow, .8, 1, 2);
	}

	m_brMenuButtonDroppedDown.DeleteObject();
	m_brMenuButtonDroppedDown.CreateSolidBrush(clrMenuButtonDroppedDown);

	m_brMenuItemCheckedHighlight.DeleteObject();
	m_brMenuItemCheckedHighlight.CreateSolidBrush(clrMenuItemCheckedHighlight);

	m_penActiveTabBorder.DeleteObject();

	if (m_colorActiveTabBorder != (COLORREF)-1)
	{
		m_penActiveTabBorder.CreatePen(PS_SOLID, 1, m_colorActiveTabBorder);
	}

	m_bDefaultWinXPColors = bDefaultWinXPColors;

	m_clrInactiveTabText = afxGlobalData.clrBtnDkShadow;

	if (afxGlobalData.m_nBitsPerPixel > 8 && !afxGlobalData.IsHighContrastMode())
	{
		m_penSeparator.DeleteObject();

		COLORREF clrSeparator = CDrawingManager::PixelAlpha(afxGlobalData.clrBarFace, 84);

		m_penSeparator.CreatePen(PS_SOLID, 1, clrSeparator);
	}
}

COLORREF CMFCVisualManagerVS2005::OnDrawPaneCaption(CDC* pDC, CDockablePane* pBar, BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		return CMFCVisualManagerOfficeXP::OnDrawPaneCaption(pDC, pBar, bActive, rectCaption, rectButtons);
	}

	rectCaption.bottom++;

	COLORREF clrFill;

	if (!bActive)
	{
		clrFill = CDrawingManager::PixelAlpha(m_clrBarGradientDark, 87);

		CBrush brFill(clrFill);
		pDC->FillRect(rectCaption, &brFill);

		pDC->Draw3dRect(rectCaption, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
	}
	else
	{
		if (m_CurrAppTheme == WinXpTheme_Blue || m_CurrAppTheme == WinXpTheme_Olive || m_CurrAppTheme == WinXpTheme_Silver)
		{
			COLORREF clrLight = CDrawingManager::PixelAlpha(afxGlobalData.clrHilite, 130);

			CDrawingManager dm(*pDC);
			dm.FillGradient(rectCaption, afxGlobalData.clrHilite, clrLight, TRUE);

			return afxGlobalData.clrTextHilite;
		}
		else
		{
			pDC->FillRect(rectCaption, &afxGlobalData.brActiveCaption);
			return afxGlobalData.clrCaptionText;
		}
	}

	if (GetRValue(clrFill) <= 192 && GetGValue(clrFill) <= 192 && GetBValue(clrFill) <= 192)
	{
		return RGB(255, 255, 255);
	}
	else
	{
		return RGB(0, 0, 0);
	}
}

void CMFCVisualManagerVS2005::OnDrawCaptionButton(CDC* pDC, CMFCCaptionButton* pButton, BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled, int nImageID /*= -1*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (bActive || pButton->IsMiniFrameButton())
	{
		CMFCVisualManagerOfficeXP::OnDrawCaptionButton(pDC, pButton, bActive, bHorz, bMaximized, bDisabled, nImageID);
		return;
	}

	CRect rc = pButton->GetRect();

	const BOOL bHighlight = (pButton->m_bPushed || pButton->m_bFocused || pButton->m_bDroppedDown) && !bDisabled;

	if (bHighlight)
	{
		pDC->FillRect(rc, &afxGlobalData.brBarFace);
	}

	CMenuImages::IMAGES_IDS id = (CMenuImages::IMAGES_IDS)-1;

	if (nImageID != -1)
	{
		id = (CMenuImages::IMAGES_IDS)nImageID;
	}
	else
	{
		id = pButton->GetIconID(bHorz, bMaximized);
	}

	if (id != (CMenuImages::IMAGES_IDS)-1)
	{
		CSize sizeImage = CMenuImages::Size();
		CPoint ptImage(rc.left +(rc.Width() - sizeImage.cx) / 2, rc.top +(rc.Height() - sizeImage.cy) / 2);

		OnDrawCaptionButtonIcon(pDC, pButton, id, bActive, bDisabled, ptImage);
	}

	if (bHighlight)
	{
		pDC->Draw3dRect(rc, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarDkShadow);
	}
}

void CMFCVisualManagerVS2005::OnEraseTabsArea(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTabWnd);

	if (pTabWnd->IsFlatTab() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnEraseTabsArea(pDC, rect, pTabWnd);
		return;
	}

	if (pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style())
	{
		if (pTabWnd->IsDialogControl())
		{
			pDC->FillRect(rect, &afxGlobalData.brBtnFace);
		}
		else
		{
			pDC->FillRect(rect, &afxGlobalData.brBarFace);
		}
	}
	else
	{
		CBasePane* pParentBar = DYNAMIC_DOWNCAST(CBasePane, pTabWnd->GetParent());
		if (pParentBar == NULL)
		{
			pDC->FillRect(rect, &afxGlobalData.brBtnFace);
		}
		else
		{
			CRect rectScreen = afxGlobalData.m_rectVirtual;
			pTabWnd->ScreenToClient(&rectScreen);

			CRect rectFill = rect;
			rectFill.left = min(rectFill.left, rectScreen.left);

			OnFillBarBackground(pDC, pParentBar, rectFill, rect);
		}
	}
}

void CMFCVisualManagerVS2005::OnDrawTab(CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);
	ASSERT_VALID(pDC);

	if (pTabWnd->IsFlatTab() || pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style())
	{
		CPen* pOldPen = NULL;

		if (bIsActive && pTabWnd->IsVS2005Style() && m_penActiveTabBorder.GetSafeHandle() != NULL)
		{
			pOldPen = pDC->SelectObject(&m_penActiveTabBorder);
		}

		CMFCVisualManagerOffice2003::OnDrawTab(pDC, rectTab, iTab, bIsActive, pTabWnd);

		if (pOldPen != NULL)
		{
			pDC->SelectObject(pOldPen);
		}

		return;
	}

	COLORREF clrTab = pTabWnd->GetTabBkColor(iTab);
	COLORREF clrTextOld = (COLORREF)-1;

	if (bIsActive && clrTab == (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor(afxGlobalData.clrWindowText);
		((CMFCBaseTabCtrl*)pTabWnd)->SetTabBkColor(iTab, afxGlobalData.clrWindow);
	}

	CMFCVisualManagerOfficeXP::OnDrawTab(pDC, rectTab, iTab, bIsActive, pTabWnd);

	((CMFCBaseTabCtrl*)pTabWnd)->SetTabBkColor(iTab, clrTab);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}
}

int CMFCVisualManagerVS2005::CreateAutoHideButtonRegion(CRect rect, DWORD dwAlignment, LPPOINT& points)
{
	switch(dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_LEFT:
		rect.right--;
		break;

	case CBRS_ALIGN_TOP:
		rect.bottom--;
		break;
	}

	CRect rectOrign = rect;
	DWORD dwAlignmentOrign = dwAlignment;

	if ((dwAlignment & CBRS_ALIGN_ANY) == CBRS_ALIGN_LEFT || (dwAlignment & CBRS_ALIGN_ANY) == CBRS_ALIGN_RIGHT)
	{
		rect = CRect(0, 0, rectOrign.Height(), rectOrign.Width());
		dwAlignment = (dwAlignment == CBRS_ALIGN_LEFT) ? CBRS_ALIGN_TOP : CBRS_ALIGN_BOTTOM;
	}

	CList<POINT, POINT> pts;

	if (!m_bRoundedAutohideButtons)
	{
		rect.right--;

		pts.AddHead(CPoint(rect.left, rect.top));
		pts.AddHead(CPoint(rect.left, rect.bottom - 2));
		pts.AddHead(CPoint(rect.left + 2, rect.bottom));
		pts.AddHead(CPoint(rect.right - 2, rect.bottom));
		pts.AddHead(CPoint(rect.right, rect.bottom - 2));
		pts.AddHead(CPoint(rect.right, rect.top));
	}
	else
	{
		POSITION posLeft = pts.AddHead(CPoint(rect.left, rect.top));
		posLeft = pts.InsertAfter(posLeft, CPoint(rect.left, rect.top + 2));

		POSITION posRight = pts.AddTail(CPoint(rect.right, rect.top));
		posRight = pts.InsertBefore(posRight, CPoint(rect.right, rect.top + 2));

		int xLeft = rect.left + 1;
		int xRight = rect.right - 1;

		int y = 0;

		BOOL bIsHorz = (dwAlignmentOrign & CBRS_ALIGN_ANY) == CBRS_ALIGN_LEFT || (dwAlignmentOrign & CBRS_ALIGN_ANY) == CBRS_ALIGN_RIGHT;

		for (y = rect.top + 2; y < rect.bottom - 4; y += 2)
		{
			posLeft = pts.InsertAfter(posLeft, CPoint(xLeft, y));
			posLeft = pts.InsertAfter(posLeft, CPoint(xLeft, y + 2));

			posRight = pts.InsertBefore(posRight, CPoint(xRight, y));
			posRight = pts.InsertBefore(posRight, CPoint(xRight, y + 2));

			xLeft++;
			xRight--;
		}

		if ((dwAlignmentOrign & CBRS_ALIGN_ANY) == CBRS_ALIGN_BOTTOM && !bIsHorz)
		{
			xLeft--;
			xRight++;
		}

		if (bIsHorz)
		{
			xRight++;
		}

		for (;y < rect.bottom - 1; y++)
		{
			posLeft = pts.InsertAfter(posLeft, CPoint(xLeft, y));
			posLeft = pts.InsertAfter(posLeft, CPoint(xLeft + 1, y + 1));

			posRight = pts.InsertBefore(posRight, CPoint(xRight, y));
			posRight = pts.InsertBefore(posRight, CPoint(xRight - 1, y + 1));

			if (y == rect.bottom - 2)
			{
				posLeft = pts.InsertAfter(posLeft, CPoint(xLeft + 1, y + 1));
				posLeft = pts.InsertAfter(posLeft, CPoint(xLeft + 3, y + 1));

				posRight = pts.InsertBefore(posRight, CPoint(xRight, y + 1));
				posRight = pts.InsertBefore(posRight, CPoint(xRight - 2, y + 1));
			}

			xLeft++;
			xRight--;
		}

		posLeft = pts.InsertAfter(posLeft, CPoint(xLeft + 2, rect.bottom));
		posRight = pts.InsertBefore(posRight, CPoint(xRight - 2, rect.bottom));
	}

	points = new POINT [pts.GetCount()];

	int i = 0;

	for (POSITION pos = pts.GetHeadPosition(); pos != NULL; i++)
	{
		points [i] = pts.GetNext(pos);

		switch(dwAlignmentOrign & CBRS_ALIGN_ANY)
		{
		case CBRS_ALIGN_BOTTOM:
			points [i].y = rect.bottom -(points [i].y - rect.top);
			break;

		case CBRS_ALIGN_RIGHT:
			{
				int x = rectOrign.right - points [i].y;
				int y = rectOrign.top + points [i].x;

				points [i] = CPoint(x, y);
			}
			break;

		case CBRS_ALIGN_LEFT:
			{
				int x = rectOrign.left + points [i].y;
				int y = rectOrign.top + points [i].x;

				points [i] = CPoint(x, y);
			}
			break;
		}
	}

	return(int) pts.GetCount();
}

void CMFCVisualManagerVS2005::OnFillAutoHideButtonBackground(CDC* pDC, CRect rect, CMFCAutoHideButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (!m_bRoundedAutohideButtons)
	{
		return;
	}

	LPPOINT points;
	int nPoints = CreateAutoHideButtonRegion(rect, pButton->GetAlignment(), points);

	CRgn rgnClip;
	rgnClip.CreatePolygonRgn(points, nPoints, WINDING);

	pDC->SelectClipRgn(&rgnClip);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOffice2003::OnFillAutoHideButtonBackground(pDC, rect, pButton);
	}
	else
	{
		BOOL bIsHorz = ((pButton->GetAlignment() & CBRS_ALIGN_ANY) == CBRS_ALIGN_LEFT || (pButton->GetAlignment() & CBRS_ALIGN_ANY) == CBRS_ALIGN_RIGHT);

		CDrawingManager dm(*pDC);

		dm.FillGradient(rect, m_clrBarGradientDark, m_clrBarGradientLight, !bIsHorz);
	}

	pDC->SelectClipRgn(NULL);
	delete [] points;
}

void CMFCVisualManagerVS2005::OnDrawAutoHideButtonBorder(CDC* pDC, CRect rect, CRect /*rectBorderSize*/, CMFCAutoHideButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	CPen pen(PS_SOLID, 1, afxGlobalData.clrBarShadow);

	CPen* pOldPen = pDC->SelectObject(&pen);
	ENSURE(pOldPen != NULL);

	LPPOINT points;
	int nPoints = CreateAutoHideButtonRegion(rect, pButton->GetAlignment(), points);

	if (!m_bRoundedAutohideButtons)
	{
		pDC->Polyline(points, nPoints);
	}
	else
	{
		BOOL bIsHorz ((pButton->GetAlignment() & CBRS_ALIGN_ANY) == CBRS_ALIGN_LEFT || (pButton->GetAlignment() & CBRS_ALIGN_ANY) == CBRS_ALIGN_RIGHT);

		for (int i = 0; i < nPoints; i++)
		{
			if ((i % 2) != 0)
			{
				int x1 = points [i - 1].x;
				int y1 = points [i - 1].y;

				int x2 = points [i].x;
				int y2 = points [i].y;

				if (bIsHorz)
				{
					if (y1 > rect.CenterPoint().y && y2 > rect.CenterPoint().y)
					{
						y1--;
						y2--;
					}
				}
				else
				{
					if (x1 > rect.CenterPoint().x && x2 > rect.CenterPoint().x)
					{
						x1--;
						x2--;
					}
				}

				if (y2 >= y1)
				{
					pDC->MoveTo(x1, y1);
					pDC->LineTo(x2, y2);
				}
				else
				{
					pDC->MoveTo(x2, y2);
					pDC->LineTo(x1, y1);
				}
			}
		}
	}

	pDC->SelectObject(pOldPen);
	delete [] points;
}

void CMFCVisualManagerVS2005::GetTabFrameColors(const CMFCBaseTabCtrl* pTabWnd, COLORREF& clrDark, COLORREF& clrBlack, COLORREF& clrHighlight,
	COLORREF& clrFace, COLORREF& clrDarkShadow, COLORREF& clrLight, CBrush*& pbrFace, CBrush*& pbrBlack)
{
	ASSERT_VALID(pTabWnd);

	CMFCVisualManagerOffice2003::GetTabFrameColors(pTabWnd, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack);

	if (pTabWnd->IsVS2005Style() && m_colorActiveTabBorder != (COLORREF)-1)
	{
		clrHighlight = m_colorActiveTabBorder;
	}

	clrBlack = clrDarkShadow;
}

void CMFCVisualManagerVS2005::OnDrawSeparator(CDC* pDC, CBasePane* pBar, CRect rect, BOOL bHorz)
{
	CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, pBar);
	if (pToolBar != NULL)
	{
		ASSERT_VALID(pToolBar);

		if (bHorz)
		{
			const int nDelta = max(0, (pToolBar->GetButtonSize().cy - pToolBar->GetImageSize().cy) / 2);
			rect.top += nDelta;
		}
		else
		{
			const int nDelta = max(0, (pToolBar->GetButtonSize().cx - pToolBar->GetImageSize().cx) / 2);
			rect.left += nDelta;
		}
	}

	CMFCVisualManagerOfficeXP::OnDrawSeparator(pDC, pBar, rect, bHorz);
}

void CMFCVisualManagerVS2005::OnFillHighlightedArea(CDC* pDC, CRect rect, CBrush* pBrush, CMFCToolBarButton* pButton)
{
	if (pButton != NULL && (m_CurrAppTheme == WinXpTheme_Blue || m_CurrAppTheme == WinXpTheme_Olive))
	{
		ASSERT_VALID(pButton);

		CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);

		BOOL bIsPopupMenu = pMenuButton != NULL && pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));

		if (bIsPopupMenu && (pButton->m_nStyle & TBBS_CHECKED) && pBrush == &m_brHighlightDn)
		{
			pDC->FillRect(rect, &m_brMenuItemCheckedHighlight);
			return;
		}

		if (pMenuButton != NULL && !bIsPopupMenu && pMenuButton->IsDroppedDown())
		{
			pDC->FillRect(rect, &m_brMenuButtonDroppedDown);
			return;
		}
	}

	CMFCVisualManagerOffice2003::OnFillHighlightedArea(pDC, rect, pBrush, pButton);
}

int CMFCVisualManagerVS2005::GetDockingTabsBordersSize()
{
	return CTabbedPane::m_StyleTabWnd == CMFCTabCtrl::STYLE_3D_ROUNDED ? 0 : 3;
}

COLORREF CMFCVisualManagerVS2005::GetPropertyGridGroupColor(CMFCPropertyGridCtrl* pPropList)
{
	ASSERT_VALID(pPropList);

	if (m_bDefaultWinXPColors)
	{
		return CMFCVisualManagerOffice2003::GetPropertyGridGroupColor(pPropList);
	}

	return pPropList->DrawControlBarColors() ? afxGlobalData.clrBarLight : afxGlobalData.clrBtnLight;
}

COLORREF CMFCVisualManagerVS2005::OnFillMiniFrameCaption(CDC* pDC, CRect rectCaption, CPaneFrameWnd* pFrameWnd, BOOL bActive)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pFrameWnd);

	if (DYNAMIC_DOWNCAST(CMFCBaseToolBar, pFrameWnd->GetPane()) == NULL)
	{
		return CMFCVisualManagerOffice2003::OnFillMiniFrameCaption(pDC, rectCaption, pFrameWnd, bActive);
	}

	::FillRect(pDC->GetSafeHdc(), rectCaption, ::GetSysColorBrush(COLOR_3DSHADOW));
	return afxGlobalData.clrCaptionText;
}

void CMFCVisualManagerVS2005::OnDrawToolBoxFrame(CDC* pDC, const CRect& rect)
{
	ASSERT_VALID(pDC);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
}



