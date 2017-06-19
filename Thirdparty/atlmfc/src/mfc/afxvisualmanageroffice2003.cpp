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
#include "afxcontrolbarutil.h"
#include "afxvisualmanageroffice2003.h"
#include "afxdrawmanager.h"
#include "afxpopupmenubar.h"
#include "afxmenubar.h"
#include "afxglobals.h"
#include "afxtoolbarmenubutton.h"
#include "afxcustomizebutton.h"
#include "afxmenuimages.h"
#include "afxcaptionbar.h"
#include "afxbasetabctrl.h"
#include "afxcolorbar.h"
#include "afxtabctrl.h"
#include "afxtaskspane.h"
#include "afxstatusbar.h"
#include "afxautohidebutton.h"
#include "afxheaderctrl.h"
#include "afxrebar.h"
#include "afxdesktopalertwnd.h"
#include "afxdropdowntoolbar.h"
#include "afxribboncategory.h"
#include "afxribbonquickaccesstoolbar.h"
#include "afxribbonbar.h"
#include "afxribbonstatusbar.h"
#include "afxribbonstatusbarpane.h"
#include "afxtooltipctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCVisualManagerOffice2003, CMFCVisualManagerOfficeXP)

BOOL CMFCVisualManagerOffice2003::m_bUseGlobalTheme = TRUE;
BOOL CMFCVisualManagerOffice2003::m_bStatusBarOfficeXPLook = TRUE;
BOOL CMFCVisualManagerOffice2003::m_bDefaultWinXPColors = TRUE;

// Construction/Destruction
CMFCVisualManagerOffice2003::CMFCVisualManagerOffice2003()
{
	m_WinXPTheme = WinXpTheme_None;

	m_bShadowHighlightedImage = FALSE;
	m_bFadeInactiveImage = FALSE;
	m_nMenuShadowDepth = 3;

	m_nVertMargin = 8;
	m_nHorzMargin = 8;
	m_nGroupVertOffset = 8;
	m_nGroupCaptionHeight = 18;
	m_nGroupCaptionHorzOffset = 3;
	m_nGroupCaptionVertOffset = 3;
	m_nTasksHorzOffset = 8;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = TRUE;

	OnUpdateSystemColors();
}

CMFCVisualManagerOffice2003::~CMFCVisualManagerOffice2003()
{
}

void CMFCVisualManagerOffice2003::DrawCustomizeButton(CDC* pDC, CRect rect, BOOL bIsHorz, CMFCVisualManager::AFX_BUTTON_STATE state, BOOL bIsCustomize, BOOL bIsMoreButtons)
{
	ASSERT_VALID(pDC);

	COLORREF clrDark = state == ButtonsIsRegular ? m_clrCustomizeButtonGradientDark : m_clrHighlightGradientDark;
	COLORREF clrLight = state == ButtonsIsRegular ? m_clrCustomizeButtonGradientLight : m_clrHighlightGradientLight;

#define AFX_PTS_NUM 6
	POINT pts [AFX_PTS_NUM];

	if (bIsHorz)
	{
		pts [0] = CPoint(rect.left, rect.top);
		pts [1] = CPoint(rect.left + 2, rect.top + 1);
		pts [2] = CPoint(rect.left + 3, rect.bottom - 3);
		pts [3] = CPoint(rect.left, rect.bottom);
		pts [4] = CPoint(rect.right, rect.bottom);
		pts [5] = CPoint(rect.right, rect.top);
	}
	else
	{
		pts [0] = CPoint(rect.left, rect.top);
		pts [1] = CPoint(rect.left + 3, rect.top + 2);
		pts [2] = CPoint(rect.right - 3, rect.top + 3);
		pts [3] = CPoint(rect.right, rect.top);
		pts [4] = CPoint(rect.right, rect.bottom);
		pts [5] = CPoint(rect.left, rect.bottom);
	}

	CRgn rgnClip;
	rgnClip.CreatePolygonRgn(pts, AFX_PTS_NUM, WINDING);

	pDC->SelectClipRgn(&rgnClip);

	CDrawingManager dm(*pDC);
	dm.FillGradient(rect, clrDark, clrLight, bIsHorz);

	//---------------------
	// Draw button content:
	//---------------------
	const int nEllipse = 2;

	if (bIsHorz)
	{
		rect.DeflateRect(0, nEllipse);
		rect.left += nEllipse;
	}
	else
	{
		rect.DeflateRect(nEllipse, 0);
		rect.top += nEllipse;
	}

	const int nMargin = GetToolBarCustomizeButtonMargin();

	CSize sizeImage = CMenuImages::Size();
	if (CMFCToolBar::IsLargeIcons())
	{
		sizeImage.cx *= 2;
		sizeImage.cy *= 2;
	}

	if (bIsCustomize)
	{
		//-----------------
		// Draw menu image:
		//-----------------
		CRect rectMenu = rect;
		if (bIsHorz)
		{
			rectMenu.top = rectMenu.bottom - sizeImage.cy - 2 * nMargin;
		}
		else
		{
			rectMenu.top++;
			rectMenu.left = rectMenu.right - sizeImage.cx - 2 * nMargin;
		}

		rectMenu.DeflateRect((rectMenu.Width() - sizeImage.cx) / 2, (rectMenu.Height() - sizeImage.cy) / 2);
		rectMenu.OffsetRect(1, 1);

		CMenuImages::IMAGES_IDS id = bIsHorz ? CMenuImages::IdCustomizeArrowDown : CMenuImages::IdCustomizeArrowLeft;

		CMenuImages::Draw( pDC, id, rectMenu.TopLeft(), CMenuImages::ImageWhite, sizeImage);

		rectMenu.OffsetRect(-1, -1);

		CMenuImages::Draw( pDC, id, rectMenu.TopLeft(), CMenuImages::ImageBlack, sizeImage);
	}

	if (bIsMoreButtons)
	{
		//-------------------
		// Draw "more" image:
		//-------------------
		CRect rectMore = rect;
		if (bIsHorz)
		{
			rectMore.bottom = rectMore.top + sizeImage.cy + 2 * nMargin;
		}
		else
		{
			rectMore.right = rectMore.left + sizeImage.cx + 2 * nMargin;
			rectMore.top++;
		}

		rectMore.DeflateRect((rectMore.Width() - sizeImage.cx) / 2, (rectMore.Height() - sizeImage.cy) / 2);

		CMenuImages::IMAGES_IDS id = bIsHorz ? CMenuImages::IdCustomizeMoreButtonsHorz : CMenuImages::IdCustomizeMoreButtonsVert;

		rectMore.OffsetRect(1, 1);
		CMenuImages::Draw(pDC, id, rectMore.TopLeft(), CMenuImages::ImageWhite, sizeImage);

		rectMore.OffsetRect(-1, -1);
		CMenuImages::Draw(pDC, id, rectMore.TopLeft(), CMenuImages::ImageBlack, sizeImage);
	}

	pDC->SelectClipRgn(NULL);
}

BOOL CMFCVisualManagerOffice2003::IsToolbarRoundShape(CMFCToolBar* pToolBar)
{
	ASSERT_VALID(pToolBar);
	return !pToolBar->IsKindOf(RUNTIME_CLASS(CMFCMenuBar));
}

void CMFCVisualManagerOffice2003::OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL bNCArea)
{
	ENSURE(pBar != NULL);
	ENSURE(pDC != NULL);

	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if (DYNAMIC_DOWNCAST(CMFCReBar, pBar) != NULL || DYNAMIC_DOWNCAST(CMFCReBar, pBar->GetParent()))
	{
		FillReBarPane(pDC, pBar, rectClient);
		return;
	}

	CRuntimeClass* pBarClass = pBar->GetRuntimeClass();

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode() || pBar->IsDialogControl() || pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCColorBar)))
	{
		CMFCVisualManagerOfficeXP::OnFillBarBackground(pDC, pBar, rectClient, rectClip);
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCStatusBar)))
	{
		if (m_bStatusBarOfficeXPLook && m_hThemeStatusBar != NULL && m_pfDrawThemeBackground != NULL)
		{
			(*m_pfDrawThemeBackground)(m_hThemeStatusBar, pDC->GetSafeHdc(), 0, 0, &rectClient, 0);
			return;
		}
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonStatusBar)))
	{
		if (m_hThemeStatusBar != NULL && m_pfDrawThemeBackground != NULL)
		{
			(*m_pfDrawThemeBackground)(m_hThemeStatusBar, pDC->GetSafeHdc(), 0, 0, &rectClient, 0);
			return;
		}
	}

	if (rectClip.IsRectEmpty())
	{
		rectClip = rectClient;
	}

	CDrawingManager dm(*pDC);

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCCaptionBar)))
	{
		CMFCCaptionBar* pCaptionBar = (CMFCCaptionBar*) pBar;

		if (pCaptionBar->IsMessageBarMode())
		{
			dm.FillGradient(rectClient, m_clrBarGradientDark, m_clrBarGradientLight, FALSE);
		}
		else
		{
			dm.FillGradient(rectClient, m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);
		}
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
	{
		pDC->FillRect(rectClip, &m_brMenuLight);

		BOOL bQuickMode = FALSE;

		CMFCPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, pBar);
		if (!pMenuBar->m_bDisableSideBarInXPMode)
		{
			CWnd* pWnd = pMenuBar->GetParent();

			if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CMFCPopupMenu)))
			{
				CMFCPopupMenu* pMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pWnd);

				if (pMenu->IsCustomizePane())
				{
					bQuickMode = TRUE;
				}
			}

			CRect rectImages = rectClient;
			rectImages.DeflateRect(0, 1);

			if (bQuickMode)
			{
				rectImages.right = rectImages.left + 2*CMFCToolBar::GetMenuImageSize().cx + 4 * GetMenuImageMargin() + 4;
			}
			else
			{
				rectImages.right = rectImages.left + CMFCToolBar::GetMenuImageSize().cx + 2 * GetMenuImageMargin() + 2;
			}

			dm.FillGradient(rectImages, m_clrToolBarGradientLight, m_clrToolBarGradientDark, FALSE, 35);
		}

		return;
	}

	BOOL bIsHorz = (pBar->GetPaneStyle() & CBRS_ORIENT_HORZ);
	BOOL bIsToolBar = pBar->IsKindOf(RUNTIME_CLASS(CMFCToolBar)) && !pBar->IsKindOf(RUNTIME_CLASS(CMFCMenuBar));

	COLORREF clr1 = bIsHorz ? m_clrToolBarGradientDark : m_clrToolBarGradientVertLight;
	COLORREF clr2 = bIsHorz ? m_clrToolBarGradientLight : m_clrToolBarGradientVertDark;

	if (!bIsToolBar)
	{
		bIsHorz = FALSE;

		clr1 = m_clrBarGradientDark;
		clr2 = m_clrBarGradientLight;

		rectClient.right = rectClient.left + afxGlobalData.m_rectVirtual.Width() + 10;
	}

	const int nStartFlatPercentage = bIsToolBar ? 25 : 0;

	BOOL bRoundedCorners = FALSE;

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCDropDownToolBar)))
	{
		bNCArea = FALSE;
	}

	CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, pBar);
	if (bNCArea && pToolBar != NULL && pToolBar->IsDocked() && pToolBar->GetParentDockSite() != NULL && !pToolBar->IsKindOf(RUNTIME_CLASS(CMFCMenuBar)))
	{
		bRoundedCorners = TRUE;

		CBasePane* pParentBar = DYNAMIC_DOWNCAST(CBasePane, pBar->GetParent());

		if (pParentBar != NULL)
		{
			CPoint pt(0, 0);
			pBar->MapWindowPoints(pParentBar, &pt, 1);
			pt = pDC->OffsetWindowOrg(pt.x, pt.y);

			CRect rectParent;
			pParentBar->GetClientRect(rectParent);

			OnFillBarBackground(pDC, pParentBar, rectParent, rectParent);

			pDC->SetWindowOrg(pt.x, pt.y);
		}

		CRect rectFill = rectClient;
		rectFill.DeflateRect(1, 0);

		dm.FillGradient(rectFill, clr1, clr2, bIsHorz, nStartFlatPercentage);

		CRect rectLeft = rectClient;
		rectLeft.top ++;
		rectLeft.right = rectLeft.left + 1;

		dm.FillGradient(rectLeft, clr1, clr2, bIsHorz);

		CRect rectRight = rectClient;
		rectLeft.top ++;
		rectRight.left = rectRight.right - 1;

		dm.FillGradient(rectRight, clr1, clr2, bIsHorz);
	}
	else
	{
		CRect rectFill = rectClient;

		if (bIsToolBar && pToolBar != NULL && pBar->IsDocked() && pToolBar->GetParentDockSite() != NULL)
		{
			ASSERT_VALID(pToolBar);

			rectFill.left -= pToolBar->m_cxLeftBorder;
			rectFill.right += pToolBar->m_cxRightBorder;
			rectFill.top -= pToolBar->m_cyTopBorder;
			rectFill.bottom += pToolBar->m_cyBottomBorder;
		}

		dm.FillGradient(rectFill, clr1, clr2, bIsHorz, nStartFlatPercentage);
	}

	if (bNCArea)
	{
		CMFCCustomizeButton* pCustomizeButton = NULL;

		CRect rectCustomizeBtn;
		rectCustomizeBtn.SetRectEmpty();

		if (pToolBar != NULL && pToolBar->GetCount() > 0)
		{
			pCustomizeButton = DYNAMIC_DOWNCAST(CMFCCustomizeButton,
				pToolBar->GetButton(pToolBar->GetCount() - 1));

			if (pCustomizeButton != NULL)
			{
				rectCustomizeBtn = pCustomizeButton->Rect();
			}
		}

		if (bRoundedCorners)
		{
			//------------------------
			// Draw bottom/right edge:
			//------------------------
			CPen* pOldPen = pDC->SelectObject(&m_penBottomLine);
			ENSURE(pOldPen != NULL);

			if (bIsHorz)
			{
				pDC->MoveTo(rectClient.left + 2, rectClient.bottom - 1);
				pDC->LineTo(rectClient.right - rectCustomizeBtn.Width(), rectClient.bottom - 1);
			}
			else
			{
				pDC->MoveTo(rectClient.right - 1, rectClient.top + 2);
				pDC->LineTo(rectClient.right - 1, rectClient.bottom - 2 - rectCustomizeBtn.Height());
			}

			pDC->SelectObject(pOldPen);
		}

		if (pToolBar != NULL && pToolBar->GetCount() > 0)
		{
			if (pCustomizeButton != NULL && !rectCustomizeBtn.IsRectEmpty() && pCustomizeButton->IsPipeStyle())
			{
				BOOL bIsRTL = pBar->GetExStyle() & WS_EX_LAYOUTRTL;

				//----------------------------------------
				// Special drawing for "Customize" button:
				//----------------------------------------
				CRect rectWindow;
				pBar->GetWindowRect(rectWindow);

				pBar->ClientToScreen(&rectCustomizeBtn);

				CRect rectButton = rectClient;

				if (pToolBar->IsHorizontal())
				{
					if (bIsRTL)
					{
						int nButtonWidth = rectCustomizeBtn.Width();

						nButtonWidth -= rectWindow.left - rectCustomizeBtn.left;
						rectButton.left = rectButton.right - nButtonWidth;
					}
					else
					{
						rectButton.left = rectButton.right - rectCustomizeBtn.Width() - rectWindow.right + rectCustomizeBtn.right;
					}

					pCustomizeButton->SetExtraSize(0, rectWindow.bottom - rectCustomizeBtn.bottom);
				}
				else
				{
					rectButton.top = rectButton.bottom - rectCustomizeBtn.Height() - rectWindow.bottom + rectCustomizeBtn.bottom;
					pCustomizeButton->SetExtraSize(rectWindow.right - rectCustomizeBtn.right, 0);
				}

				AFX_BUTTON_STATE state = ButtonsIsRegular;

				if (pToolBar->IsButtonHighlighted(pToolBar->GetCount() - 1) || pCustomizeButton->IsDroppedDown())
				{
					state = ButtonsIsHighlighted;
				}
				else if (pCustomizeButton->m_nStyle &(TBBS_PRESSED | TBBS_CHECKED))
				{
					//-----------------------
					// Pressed in or checked:
					//-----------------------
					state = ButtonsIsPressed;
				}

				DrawCustomizeButton(pDC, rectButton, pToolBar->IsHorizontal(), state,
					(int) pCustomizeButton->GetCustomizeCmdId() > 0, !pCustomizeButton->GetInvisibleButtons().IsEmpty());
			}
		}
	}
}

void CMFCVisualManagerOffice2003::OnDrawPaneBorder(CDC* pDC, CBasePane* pBar, CRect& rect)
{
	ASSERT_VALID(pBar);

	if (pBar->IsDialogControl() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawPaneBorder(pDC, pBar, rect);
	}
}

void CMFCVisualManagerOffice2003::OnDrawBarGripper(CDC* pDC, CRect rectGripper, BOOL bHorz, CBasePane* pBar)
{
	ASSERT_VALID(pDC);

	if (pBar != NULL && pBar->IsDialogControl() || afxGlobalData.m_nBitsPerPixel <= 8)
	{
		CMFCVisualManagerOfficeXP::OnDrawBarGripper(pDC, rectGripper, bHorz, pBar);
		return;
	}

	const int nBoxSize = 4;

	if (bHorz)
	{
		rectGripper.left = rectGripper.right - nBoxSize;
	}
	else
	{
		rectGripper.top = rectGripper.bottom - nBoxSize;
	}

	CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, pBar);
	if (pToolBar != NULL)
	{
		if (bHorz)
		{
			const int nHeight = CMFCToolBar::IsLargeIcons() ? pToolBar->GetRowHeight() : pToolBar->GetButtonSize().cy;
			const int nDelta = max(0, (nHeight - pToolBar->GetImageSize().cy) / 2);
			rectGripper.DeflateRect(0, nDelta);
		}
		else
		{
			const int nWidth = CMFCToolBar::IsLargeIcons() ? pToolBar->GetColumnWidth() : pToolBar->GetButtonSize().cx;
			const int nDelta = max(0, (nWidth - pToolBar->GetImageSize().cx) / 2);
			rectGripper.DeflateRect(nDelta, 0);
		}
	}

	const int nBoxesNumber = bHorz ? (rectGripper.Height() - nBoxSize) / nBoxSize : (rectGripper.Width() - nBoxSize) / nBoxSize;
	int nOffset = bHorz ? (rectGripper.Height() - nBoxesNumber * nBoxSize) / 2 : (rectGripper.Width() - nBoxesNumber * nBoxSize) / 2;

	for (int nBox = 0; nBox < nBoxesNumber; nBox++)
	{
		int x = bHorz ? rectGripper.left : rectGripper.left + nOffset;
		int y = bHorz ? rectGripper.top + nOffset : rectGripper.top;

		pDC->FillSolidRect(x + 1, y + 1, nBoxSize / 2, nBoxSize / 2, afxGlobalData.clrBtnHilite);
		pDC->FillSolidRect(x, y, nBoxSize / 2, nBoxSize / 2, m_clrGripper);

		nOffset += nBoxSize;
	}
}

void CMFCVisualManagerOffice2003::OnDrawComboBorder(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* pButton)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawComboBorder(pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	if (bIsHighlighted || bIsDropped || bDisabled)
	{
		rect.DeflateRect(1, 1);

		COLORREF colorBorder = bDisabled ? afxGlobalData.clrBtnShadow : m_clrMenuItemBorder;
		pDC->Draw3dRect(&rect, colorBorder, colorBorder);
	}
}

void CMFCVisualManagerOffice2003::OnFillOutlookPageButton(CDC* pDC, const CRect& rect, BOOL bIsHighlighted, BOOL bIsPressed, COLORREF& clrText)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnFillOutlookPageButton(pDC, rect, bIsHighlighted, bIsPressed, clrText);
		return;
	}

	ASSERT_VALID(pDC);

	COLORREF clr1 = m_clrBarGradientDark;
	COLORREF clr2 = m_clrBarGradientLight;

	if (bIsPressed)
	{
		if (bIsHighlighted)
		{
			clr1 = m_clrHighlightDnGradientDark;
			clr2 = m_clrHighlightDnGradientLight;
		}
		else
		{
			clr1 = m_clrHighlightDnGradientLight;
			clr2 = m_clrHighlightDnGradientDark;
		}
	}
	else if (bIsHighlighted)
	{
		clr1 = m_clrHighlightGradientDark;
		clr2 = m_clrHighlightGradientLight;
	}

	CDrawingManager dm(*pDC);
	dm.FillGradient(rect, clr1, clr2, TRUE);

	clrText = afxGlobalData.clrBtnText;
}

void CMFCVisualManagerOffice2003::OnDrawOutlookPageButtonBorder(CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawOutlookPageButtonBorder(pDC, rectBtn, bIsHighlighted, bIsPressed);
		return;
	}

	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rectBtn, afxGlobalData.clrBtnHilite, m_clrGripper);
}

void CMFCVisualManagerOffice2003::OnFillButtonInterior(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	CMFCCustomizeButton* pCustButton = DYNAMIC_DOWNCAST(CMFCCustomizeButton, pButton);
	if (pCustButton == NULL || !pCustButton->IsPipeStyle() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnFillButtonInterior(pDC, pButton, rect, state);
		return;
	}

	CMFCToolBar* pToolBar = pCustButton->GetParentToolbar();
	if (pToolBar != NULL)
	{
		ASSERT_VALID(pToolBar);

		CRect rectToolbar;
		pToolBar->GetWindowRect(rectToolbar);
		pToolBar->ScreenToClient(rectToolbar);

		if (pToolBar->IsHorizontal())
		{
			rect.right = rectToolbar.right;
		}
		else
		{
			rect.bottom = rectToolbar.bottom;
		}

		CSize sizeExtra = pCustButton->GetExtraSize();

		rect.InflateRect(sizeExtra);
		DrawCustomizeButton(pDC, rect, pToolBar->IsHorizontal(), state, (int) pCustButton->GetCustomizeCmdId() > 0, !pCustButton->GetInvisibleButtons().IsEmpty());
	}

	pCustButton->SetDefaultDraw(FALSE);
}

void CMFCVisualManagerOffice2003::OnDrawButtonBorder(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, AFX_BUTTON_STATE state)
{
	CMFCCustomizeButton* pCustButton = DYNAMIC_DOWNCAST(CMFCCustomizeButton, pButton);
	if (pCustButton == NULL || !pCustButton->IsPipeStyle() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawButtonBorder(pDC, pButton, rect, state);
	}

	// Do nothing - the border is already painted in OnFillButtonInterior
}

void CMFCVisualManagerOffice2003::OnDrawSeparator(CDC* pDC, CBasePane* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID(pBar);

	if (pBar->IsDialogControl() || pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)) || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawSeparator(pDC, pBar, rect, bHorz);
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonStatusBar)))
	{
		if (m_hThemeStatusBar == NULL)
		{
			CMFCVisualManagerOfficeXP::OnDrawSeparator(pDC, pBar, rect, bHorz);
			return;
		}

		rect.InflateRect(1, 5);

		if (m_pfDrawThemeBackground)
		{
			(*m_pfDrawThemeBackground)(m_hThemeStatusBar, pDC->GetSafeHdc(), SP_PANE, 0, &rect, 0);
		}
		return;
	}

	CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, pBar);
	if (pToolBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawSeparator(pDC, pBar, rect, bHorz);
		return;
	}

	CPen* pOldPen = pDC->SelectObject(&m_penSeparator);
	ENSURE(pOldPen != NULL);

	if (bHorz)
	{
		const int nDelta = max(0, (pToolBar->GetButtonSize().cy - pToolBar->GetImageSize().cy) / 2);
		rect.DeflateRect(0, nDelta);

		int x = rect.left += rect.Width() / 2 - 1;

		pDC->MoveTo(x, rect.top);
		pDC->LineTo(x, rect.bottom - 1);

		pDC->SelectObject(&m_penSeparatorLight);

		pDC->MoveTo(x + 1, rect.top + 1);
		pDC->LineTo(x + 1, rect.bottom);
	}
	else
	{
		const int nDelta = max(0, (pToolBar->GetButtonSize().cx - pToolBar->GetImageSize().cx) / 2);
		rect.DeflateRect(nDelta, 0);

		int y = rect.top += rect.Height() / 2 - 1;

		pDC->MoveTo(rect.left, y);
		pDC->LineTo(rect.right - 1, y);

		pDC->SelectObject(&m_penSeparatorLight);

		pDC->MoveTo(rect.left + 1, y + 1);
		pDC->LineTo(rect.right, y + 1);
	}

	pDC->SelectObject(pOldPen);
}

void CMFCVisualManagerOffice2003::OnUpdateSystemColors()
{
	CMFCBaseVisualManager::UpdateSystemColors();

	BOOL bIsAppThemed = m_bUseGlobalTheme ||(m_pfGetWindowTheme != NULL && (*m_pfGetWindowTheme)(AfxGetMainWnd()->GetSafeHwnd()) != NULL);

	m_WinXPTheme = bIsAppThemed ? GetStandardWindowsTheme() : WinXpTheme_None;

	if (!m_bDefaultWinXPColors && m_WinXPTheme != WinXpTheme_None)
	{
		m_WinXPTheme = WinXpTheme_NonStandard;
	}

	m_bIsStandardWinXPTheme = m_WinXPTheme == WinXpTheme_Blue || m_WinXPTheme == WinXpTheme_Olive || m_WinXPTheme == WinXpTheme_Silver;

	//----------------------
	// Modify global colors:
	//----------------------
	ModifyGlobalColors();

	CMFCVisualManagerOfficeXP::OnUpdateSystemColors();

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		m_clrTaskPaneGradientDark  = afxGlobalData.clrWindow;
		m_clrTaskPaneGradientLight  = afxGlobalData.clrWindow;
		m_clrTaskPaneGroupCaptionDark  = afxGlobalData.clrBarFace;
		m_clrTaskPaneGroupCaptionLight  = afxGlobalData.clrBarFace;
		m_clrTaskPaneGroupCaptionSpecDark  = afxGlobalData.clrBarFace;
		m_clrTaskPaneGroupCaptionSpecLight  = afxGlobalData.clrBarFace;
		m_clrTaskPaneGroupAreaLight  = afxGlobalData.clrWindow;
		m_clrTaskPaneGroupAreaDark  = afxGlobalData.clrWindow;
		m_clrTaskPaneGroupAreaSpecLight  = afxGlobalData.clrWindow;
		m_clrTaskPaneGroupAreaSpecDark  = afxGlobalData.clrWindow;
		m_clrTaskPaneGroupBorder  = afxGlobalData.clrBtnShadow;

		m_clrBarGradientLight = m_clrToolBarGradientLight = afxGlobalData.clrBarLight;

		m_penTaskPaneGroupBorder.DeleteObject();
		m_penTaskPaneGroupBorder.CreatePen(PS_SOLID, 1, m_clrTaskPaneGroupBorder);

		m_clrToolbarDisabled = afxGlobalData.clrBtnHilite;
		return;
	}

	//--------------------------------------------------
	// Calculate control bars bakground gradient colors:
	//--------------------------------------------------
	COLORREF clrBase = GetBaseThemeColor();

	if (m_WinXPTheme == WinXpTheme_Olive)
	{
		m_clrToolBarGradientDark = CDrawingManager::PixelAlpha(clrBase, 120);
		m_clrBarGradientDark = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_3DFACE), .87, 1, 3);
		m_clrToolBarGradientLight = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_WINDOW), 1., 2, 1);
		m_clrBarGradientLight = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_WINDOW), 1.03);
	}
	else if (m_WinXPTheme == WinXpTheme_Silver)
	{
		m_clrToolBarGradientDark = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_3DFACE), 0.75, 2);
		m_clrBarGradientDark = CDrawingManager::PixelAlpha(clrBase, 120);
		m_clrToolBarGradientLight = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_3DHIGHLIGHT), .98);
		m_clrBarGradientLight = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_WINDOW), 1.03);
	}
	else if (m_WinXPTheme == WinXpTheme_Blue)
	{
		m_clrToolBarGradientDark = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_3DFACE), 0.93, 2);
		m_clrBarGradientDark = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_3DLIGHT), .99, 2, 1);
		m_clrToolBarGradientLight = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_WINDOW), 1.03);
		m_clrBarGradientLight = m_clrToolBarGradientLight;
	}
	else
	{
		m_clrToolBarGradientDark = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_3DFACE), 0.93, 2);
		m_clrBarGradientDark = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_3DLIGHT), .99, 2, 1);
		m_clrToolBarGradientLight = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_WINDOW), 1., 1, 4);
		m_clrBarGradientLight = m_clrToolBarGradientLight;
	}

	m_clrToolBarGradientVertLight = m_clrToolBarGradientLight;
	m_clrToolBarGradientVertDark = CDrawingManager::PixelAlpha(m_clrToolBarGradientDark, 98);

	COLORREF clrSeparatorDark;

	//-------------------------------------
	// Calculate highlight gradient colors:
	//-------------------------------------
	if (m_bIsStandardWinXPTheme && m_pfGetThemeColor != NULL)
	{
		COLORREF clr1, clr2, clr3;

		if (m_WinXPTheme == WinXpTheme_Blue && afxGlobalData.bIsWindowsVista)
		{
			clr1 = RGB (250, 196, 88);
			clr2 = RGB (250, 196, 88);
			clr3 = RGB (228, 93, 61);
		}
		else
		{
			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, 0, TMT_ACCENTCOLORHINT, &clr1);
			(*m_pfGetThemeColor) (m_hThemeButton, BP_RADIOBUTTON, 0, TMT_ACCENTCOLORHINT, &clr2);
			(*m_pfGetThemeColor) (m_hThemeWindow, WP_CLOSEBUTTON, 0, TMT_FILLCOLORHINT, &clr3);
		}

		m_clrHighlightMenuItem = CDrawingManager::SmartMixColors(clr1, clr2, 1.3, 1, 1);
		m_clrHighlightGradientLight = CDrawingManager::SmartMixColors(clr1, clr3, 1.55, 2, 1);
		m_clrHighlightGradientDark = CDrawingManager::SmartMixColors(clr1, clr2, 1.03, 2, 1);
		m_clrHighlightDnGradientLight = CDrawingManager::SmartMixColors(clr1, clr3, 1.03, 1, 2);

		m_brFloatToolBarBorder.DeleteObject();

		COLORREF clrCustom;
		(*m_pfGetThemeColor)(m_hThemeButton, 2, 0, 3822, &clrCustom);

		COLORREF clrToolbarBorder = CDrawingManager::SmartMixColors(clrCustom, clrBase, 0.84, 1, 4);
		m_brFloatToolBarBorder.CreateSolidBrush(clrToolbarBorder);

		if (m_WinXPTheme == WinXpTheme_Blue || m_WinXPTheme == WinXpTheme_Silver)
		{
			m_clrCustomizeButtonGradientDark = afxGlobalData.bIsWindowsVista ? RGB (7, 57, 142) : 
												CDrawingManager::PixelAlpha (afxGlobalData.clrActiveCaption, 60);

			const double k = (m_WinXPTheme == WinXpTheme_Blue) ? 1.61 : 1;

			m_clrCustomizeButtonGradientLight = CDrawingManager::SmartMixColors(m_clrCustomizeButtonGradientDark, clrBase, k, 3, 1);

			(*m_pfGetThemeColor)(m_hThemeButton, 1, 5, 3823, &clrCustom);
		}
		else
		{
			m_clrCustomizeButtonGradientDark = CDrawingManager::SmartMixColors(clrCustom, clrBase, 0.63, 1, 3);

			(*m_pfGetThemeColor)(m_hThemeButton, 1, 5, 3823, &clrCustom);

			m_clrCustomizeButtonGradientLight = CDrawingManager::SmartMixColors(clrCustom, clrBase, 1.2, 1, 3);
		}

		afxGlobalData.clrBarDkShadow = m_clrCustomizeButtonGradientDark;

		if (m_WinXPTheme != WinXpTheme_Silver)
		{
			afxGlobalData.clrBarShadow = CDrawingManager::SmartMixColors(clrCustom, clrBase, 1.4, 1, 3);
		}

		m_clrToolBarBottomLine = m_WinXPTheme == WinXpTheme_Silver ? CDrawingManager::PixelAlpha(m_clrToolBarGradientDark, 80) :
		CDrawingManager::PixelAlpha(m_clrToolBarGradientDark, 50);

		m_colorToolBarCornerTop = CDrawingManager::PixelAlpha(m_clrToolBarGradientLight, 92);
		m_colorToolBarCornerBottom = CDrawingManager::PixelAlpha(m_clrToolBarGradientDark, 97);

		m_clrGripper = CDrawingManager::PixelAlpha(m_clrToolBarGradientVertDark, 40);

		clrSeparatorDark = CDrawingManager::PixelAlpha(m_clrToolBarGradientVertDark, 81);

		m_clrMenuItemBorder = m_clrGripper;

		m_clrMenuBorder = CDrawingManager::PixelAlpha(clrBase, 80);

		m_clrCaptionBarGradientDark = m_clrCustomizeButtonGradientDark;
		m_clrCaptionBarGradientLight = m_clrCustomizeButtonGradientLight;

		m_clrMenuLight = CDrawingManager::PixelAlpha(afxGlobalData.clrWindow, .96, .96, .96);

		m_brMenuLight.DeleteObject();
		m_brMenuLight.CreateSolidBrush(m_clrMenuLight);
	}
	else
	{
		m_clrHighlightMenuItem = (COLORREF)-1;

		m_clrHighlightGradientLight = m_clrHighlight;
		m_clrHighlightGradientDark = m_clrHighlightDn;
		m_clrHighlightDnGradientLight = CDrawingManager::PixelAlpha(m_clrHighlightGradientLight, 120);

		m_clrCustomizeButtonGradientDark = afxGlobalData.clrBarShadow;
		m_clrCustomizeButtonGradientLight = CDrawingManager::SmartMixColors(m_clrCustomizeButtonGradientDark, afxGlobalData.clrBarFace, 1, 1, 1);

		m_clrToolBarBottomLine = CDrawingManager::PixelAlpha(m_clrToolBarGradientDark, 75);
		m_colorToolBarCornerTop = afxGlobalData.clrBarLight;
		m_colorToolBarCornerBottom = m_clrToolBarGradientDark;

		m_clrGripper = afxGlobalData.clrBarShadow;
		clrSeparatorDark = afxGlobalData.clrBarShadow;

		m_clrCaptionBarGradientLight = afxGlobalData.clrBarShadow;
		m_clrCaptionBarGradientDark = afxGlobalData.clrBarDkShadow;
	}

	m_clrHighlightDnGradientDark = m_clrHighlightGradientDark;

	m_clrHighlightCheckedGradientLight = m_clrHighlightDnGradientDark;

	m_clrHighlightCheckedGradientDark = CDrawingManager::PixelAlpha(m_clrHighlightDnGradientLight, 120);

	m_brTabBack.DeleteObject();
	m_brTabBack.CreateSolidBrush(m_clrToolBarGradientLight);

	m_penSeparatorLight.DeleteObject();
	m_penSeparatorLight.CreatePen(PS_SOLID, 1, afxGlobalData.clrBarHilite);

	m_brTearOffCaption.DeleteObject();
	m_brTearOffCaption.CreateSolidBrush(afxGlobalData.clrBarFace);

	m_brFace.DeleteObject();
	m_brFace.CreateSolidBrush(m_clrToolBarGradientLight);

	m_penSeparator.DeleteObject();
	m_penSeparator.CreatePen(PS_SOLID, 1, clrSeparatorDark);

	m_clrMenuShadowBase = afxGlobalData.clrBarFace;

	m_clrToolbarDisabled = CDrawingManager::SmartMixColors(m_clrToolBarGradientDark, m_clrToolBarGradientLight, .92);

	m_penBottomLine.DeleteObject();
	m_penBottomLine.CreatePen(PS_SOLID, 1, m_clrToolBarBottomLine);

	// --------------------------
	// Calculate TaskPane colors:
	// --------------------------
	if (m_bIsStandardWinXPTheme && m_hThemeExplorerBar != NULL && m_pfGetThemeColor != NULL)
	{
		(*m_pfGetThemeColor)(m_hThemeExplorerBar, 0, 0, 3810, &m_clrTaskPaneGradientLight);// GRADIENTCOLOR1
		(*m_pfGetThemeColor)(m_hThemeExplorerBar, 0, 0, 3811, &m_clrTaskPaneGradientDark); // GRADIENTCOLOR2

		(*m_pfGetThemeColor)(m_hThemeExplorerBar, 5, 0, 3802, &m_clrTaskPaneGroupCaptionDark); // FILLCOLOR
		(*m_pfGetThemeColor)(m_hThemeExplorerBar, 12, 0, 3802, &m_clrTaskPaneGroupCaptionSpecDark);// FILLCOLOR
		m_clrTaskPaneGroupCaptionSpecLight = m_clrTaskPaneGroupCaptionDark;

		(*m_pfGetThemeColor)(m_hThemeExplorerBar, 5, 0, 3802, &m_clrTaskPaneGroupAreaLight); // FILLCOLOR
		m_clrTaskPaneGroupAreaDark = m_clrTaskPaneGroupAreaLight;
		(*m_pfGetThemeColor)(m_hThemeExplorerBar, 9, 0, 3821, &m_clrTaskPaneGroupAreaSpecLight); // FILLCOLORHINT
		m_clrTaskPaneGroupAreaSpecDark = m_clrTaskPaneGroupAreaSpecLight;
		(*m_pfGetThemeColor)(m_hThemeExplorerBar, 5, 0, 3801, &m_clrTaskPaneGroupBorder); // BORDERCOLOR
		m_clrTaskPaneGroupCaptionLight = m_clrTaskPaneGroupBorder;
	}
	else
	{
		m_clrTaskPaneGradientDark  = m_clrBarGradientDark;
		m_clrTaskPaneGradientLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupCaptionDark  = m_clrBarGradientDark;
		m_clrTaskPaneGroupCaptionLight  = m_clrToolBarGradientLight;

		COLORREF clrLight = CDrawingManager::PixelAlpha(afxGlobalData.clrBarShadow, 125);

		m_clrTaskPaneGroupCaptionSpecDark = CDrawingManager::SmartMixColors(m_clrCustomizeButtonGradientDark, clrLight);

		m_clrTaskPaneGroupCaptionSpecLight  = m_clrCustomizeButtonGradientLight;
		m_clrTaskPaneGroupAreaLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaDark  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaSpecLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaSpecDark  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupBorder  = m_clrToolBarGradientLight;
	}

	m_penTaskPaneGroupBorder.DeleteObject();
	m_penTaskPaneGroupBorder.CreatePen(PS_SOLID, 1, m_clrTaskPaneGroupBorder);
}

void CMFCVisualManagerOffice2003::OnFillHighlightedArea(CDC* pDC, CRect rect, CBrush* pBrush, CMFCToolBarButton* pButton)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnFillHighlightedArea(pDC, rect, pBrush, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pBrush);

	BOOL bIsHorz = TRUE;
	BOOL bIsPopupMenu = FALSE;

	COLORREF clr1 = (COLORREF)-1;
	COLORREF clr2 = (COLORREF)-1;

	if (pButton != NULL)
	{
		ASSERT_VALID(pButton);

		bIsHorz = pButton->IsHorizontal();

		CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);

		bIsPopupMenu = pMenuButton != NULL && pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));

		if (bIsPopupMenu && pBrush == &m_brHighlight)
		{
			if (m_clrHighlightMenuItem != (COLORREF)-1)
			{
				CBrush br(m_clrHighlightMenuItem);
				pDC->FillRect(&rect, &br);
				return;
			}
		}

		if (pMenuButton != NULL && !bIsPopupMenu && pMenuButton->IsDroppedDown())
		{
			clr1 = CDrawingManager::PixelAlpha(m_clrToolBarGradientDark, m_bIsStandardWinXPTheme ? 101 : 120);
			clr2 = CDrawingManager::PixelAlpha(m_clrToolBarGradientLight, 110);
		}
	}

	if (pBrush == &m_brHighlight && m_bIsStandardWinXPTheme)
	{
		clr1 = m_clrHighlightGradientDark;
		clr2 = bIsPopupMenu ? clr1 : m_clrHighlightGradientLight;
	}
	else if (pBrush == &m_brHighlightDn && m_bIsStandardWinXPTheme)
	{
		clr1 = bIsPopupMenu ? m_clrHighlightDnGradientLight : m_clrHighlightDnGradientDark;
		clr2 = m_clrHighlightDnGradientLight;
	}
	else if (pBrush == &m_brHighlightChecked && m_bIsStandardWinXPTheme)
	{
		clr1 = bIsPopupMenu ? m_clrHighlightCheckedGradientLight : m_clrHighlightCheckedGradientDark;
		clr2 = m_clrHighlightCheckedGradientLight;
	}

	if (clr1 == (COLORREF)-1 || clr2 == (COLORREF)-1)
	{
		CMFCVisualManagerOfficeXP::OnFillHighlightedArea(pDC, rect, pBrush, pButton);
		return;
	}

	CDrawingManager dm(*pDC);
	dm.FillGradient(rect, clr1, clr2, bIsHorz);
}

void CMFCVisualManagerOffice2003::OnDrawShowAllMenuItems(CDC* pDC, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel > 8 && !afxGlobalData.IsHighContrastMode())
	{
		const int nRadius = 8;

		rect = CRect(rect.CenterPoint() - CSize(nRadius - 1, nRadius - 1), CSize(nRadius * 2, nRadius * 2));

		CDrawingManager dm(*pDC);
		dm.DrawGradientRing(rect, m_clrToolBarGradientDark, m_clrMenuLight, (COLORREF)-1, 45, nRadius);
	}

	CMFCVisualManager::OnDrawShowAllMenuItems(pDC, rect, state);
}

int CMFCVisualManagerOffice2003::GetShowAllMenuItemsHeight(CDC* pDC, const CSize& sizeDefault)
{
	int nHeight = CMFCVisualManager::GetShowAllMenuItemsHeight(pDC, sizeDefault);
	return nHeight + 4;
}

void CMFCVisualManagerOffice2003::OnDrawCaptionBarBorder(CDC* pDC, CMFCCaptionBar* pBar, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder)
{
	ASSERT_VALID(pDC);

	if (clrBarBorder == (COLORREF) -1)
	{
		pDC->FillRect(rect, (pBar != NULL && pBar->IsDialogControl()) ? &afxGlobalData.brBtnFace : &afxGlobalData.brBarFace);
	}
	else
	{
		CBrush brBorder(clrBarBorder);
		pDC->FillRect(rect, &brBorder);
	}

	if (!bFlatBorder)
	{
		pDC->Draw3dRect(rect, m_clrBarGradientLight, m_clrToolBarBottomLine);
	}
}

void CMFCVisualManagerOffice2003::OnDrawComboDropButton(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* pButton)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawComboDropButton(pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	if (!bDisabled)
	{
		if (bIsDropped || bIsHighlighted)
		{
			OnFillHighlightedArea(pDC, rect, bIsDropped ? &m_brHighlightDn : &m_brHighlight, NULL);

			if (CMFCToolBarImages::m_bIsDrawOnGlass)
			{
				CDrawingManager dm(*pDC);
				dm.DrawLine(rect.left, rect.top, rect.left, rect.bottom, m_clrMenuItemBorder);
			}
			else
			{
				CPen pen(PS_SOLID, 1, m_clrMenuItemBorder);
				CPen* pOldPen = pDC->SelectObject(&pen);
				ENSURE(pOldPen != NULL);

				pDC->MoveTo(rect.left, rect.top);
				pDC->LineTo(rect.left, rect.bottom);

				pDC->SelectObject(pOldPen);
			}
		}
		else
		{
			CDrawingManager dm(*pDC);
			dm.FillGradient(rect, m_clrToolBarGradientDark, m_clrToolBarGradientLight, TRUE);

			if (CMFCToolBarImages::m_bIsDrawOnGlass)
			{
				dm.DrawRect(rect, (COLORREF)-1, afxGlobalData.clrWindow);
			}
			else
			{
				pDC->Draw3dRect(rect, afxGlobalData.clrWindow, afxGlobalData.clrWindow);
			}
		}
	}

	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rect, bDisabled ? CMenuImages::ImageGray :(bIsDropped && bIsHighlighted) ? CMenuImages::ImageWhite : CMenuImages::ImageBlack);
}

void CMFCVisualManagerOffice2003::OnDrawTearOffCaption(CDC* pDC, CRect rect, BOOL bIsActive)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawTearOffCaption(pDC, rect, bIsActive);
		return;
	}

	const int nBorderSize = 1;
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &m_brMenuLight);

	rect.DeflateRect(nBorderSize, 1);

	if (bIsActive)
	{
		OnFillHighlightedArea(pDC, rect, bIsActive ? &m_brHighlight : &m_brBarBkgnd, NULL);
	}
	else
	{
		pDC->FillRect(rect, &m_brTearOffCaption);
	}

	// Draw gripper:
	OnDrawBarGripper(pDC, rect, FALSE, NULL);

	if (bIsActive)
	{
		pDC->Draw3dRect(rect, m_clrMenuBorder, m_clrMenuBorder);
	}
}

void CMFCVisualManagerOffice2003::OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* pMenu, CRect rect)
{
	BOOL bConnectMenuToParent = m_bConnectMenuToParent;

	if (DYNAMIC_DOWNCAST(CMFCCustomizeButton, pMenu->GetParentButton()) != NULL)
	{
		m_bConnectMenuToParent = FALSE;
	}

	CMFCVisualManagerOfficeXP::OnDrawMenuBorder(pDC, pMenu, rect);
	m_bConnectMenuToParent = bConnectMenuToParent;
}

COLORREF CMFCVisualManagerOffice2003::GetThemeColor(HTHEME hTheme, int nIndex) const
{
	if (hTheme != NULL && m_pfGetThemeSysColor != NULL)
	{
		return(*m_pfGetThemeSysColor)(hTheme, nIndex);
	}

	return ::GetSysColor(nIndex);
}

void CMFCVisualManagerOffice2003::OnEraseTabsArea(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTabWnd);

	if (pTabWnd->IsDialogControl())
	{
		pDC->FillRect(rect, &afxGlobalData.brBtnFace);
		return;
	}

	if (pTabWnd->IsFlatTab() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnEraseTabsArea(pDC, rect, pTabWnd);
		return;
	}

	CDrawingManager dm(*pDC);

	COLORREF clr1 = m_clrToolBarGradientDark;
	COLORREF clr2 = m_clrToolBarGradientLight;

	if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
	{
		dm.FillGradient(rect, clr1, clr2, TRUE);
	}
	else
	{
		dm.FillGradient(rect, clr2, clr1, TRUE);
	}
}

void CMFCVisualManagerOffice2003::OnDrawTab(CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);
	ASSERT_VALID(pDC);

	if (!pTabWnd->IsOneNoteStyle() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode() || pTabWnd->IsLeftRightRounded())
	{
		CMFCVisualManagerOfficeXP::OnDrawTab(pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	CRect rectClip;
	pTabWnd->GetTabsRect(rectClip);

	const int nExtra = (iTab == 0 || bIsActive) ? 0 : rectTab.Height();

	if (rectTab.left + nExtra + 10 > rectClip.right || rectTab.right - 10 <= rectClip.left)
	{
		return;
	}

	const BOOL bIsHighlight = iTab == pTabWnd->GetHighlightedTab();

	COLORREF clrTab = pTabWnd->GetTabBkColor(iTab);
	if (clrTab == (COLORREF)-1 && bIsActive)
	{
		clrTab = afxGlobalData.clrWindow;
	}

	if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
	{
		rectTab.OffsetRect(0, -1);
	}

	CRect rectFill = rectTab;

#define AFX_POINTS_NUM 5
	POINT pts [AFX_POINTS_NUM];

	const int nHeight = rectFill.Height();

	pts [0].x = rectFill.left;
	pts [0].y = rectFill.bottom;

	pts [1].x = rectFill.left + nHeight;
	pts [1].y = rectFill.top;

	pts [2].x = rectFill.right - 2;
	pts [2].y = rectFill.top;

	pts [3].x = rectFill.right;
	pts [3].y = rectFill.top + 2;

	pts [4].x = rectFill.right;
	pts [4].y = rectFill.bottom;

	BOOL bIsCutted = FALSE;

	for (int i = 0; i < AFX_POINTS_NUM; i++)
	{
		if (pts [i].x > rectClip.right)
		{
			pts [i].x = rectClip.right;
			bIsCutted = TRUE;
		}

		if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
		{
			pts [i].y = rectFill.bottom - pts [i].y + rectFill.top;
		}
	}

	CRgn rgn;
	rgn.CreatePolygonRgn(pts, AFX_POINTS_NUM, WINDING);

	pDC->SelectClipRgn(&rgn);

	CRect rectLeft;
	pTabWnd->GetClientRect(rectLeft);
	rectLeft.right = rectClip.left - 1;

	pDC->ExcludeClipRect(rectLeft);

	CDrawingManager dm(*pDC);

	COLORREF clrFill = bIsHighlight ? m_clrHighlightMenuItem : clrTab;
	COLORREF clr2;

	if (clrFill != (COLORREF)-1)
	{
		clr2 = CDrawingManager::PixelAlpha(clrFill, 1.22, 1.22, 1.22);
	}
	else
	{
		clrFill = m_clrToolBarGradientDark;
		clr2 = m_clrToolBarGradientLight;
	}

	if (pTabWnd->GetLocation() == CMFCTabCtrl::LOCATION_BOTTOM)
	{
		rectFill.top++;
	}

	CRect rectTop = rectFill;
	rectTop.bottom = rectTop.CenterPoint().y - 1;

	CBrush brTop(clr2);
	pDC->FillRect(rectTop, &brTop);

	CRect rectMiddle = rectFill;
	rectMiddle.top = rectTop.bottom;
	rectMiddle.bottom = rectMiddle.top + 3;

	dm.FillGradient(rectMiddle, clrFill, clr2);

	CRect rectBottom = rectFill;
	rectBottom.top = rectMiddle.bottom;

	CBrush brBottom(clrFill);
	pDC->FillRect(rectBottom, &brBottom);

	pDC->SelectClipRgn(NULL);

	pDC->ExcludeClipRect(rectLeft);

	if (iTab > 0 && !bIsActive && iTab != pTabWnd->GetFirstVisibleTabNum())
	{
		CRect rectLeftTab = rectClip;
		rectLeftTab.right = rectFill.left + rectFill.Height() - 10;

		if (pTabWnd->GetLocation() == CMFCTabCtrl::LOCATION_BOTTOM)
		{
			rectLeftTab.top -= 2;
		}
		else
		{
			rectLeftTab.bottom++;
		}

		pDC->ExcludeClipRect(rectLeftTab);
	}

	CPen penGray(PS_SOLID, 1, afxGlobalData.clrBarDkShadow);
	CPen penShadow(PS_SOLID, 1, afxGlobalData.clrBarShadow);

	CPen* pOldPen = pDC->SelectObject(&penGray);
	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject(NULL_BRUSH);

	pDC->Polyline(pts, AFX_POINTS_NUM);

	if (bIsCutted)
	{
		pDC->MoveTo(rectClip.right, rectTab.top);
		pDC->LineTo(rectClip.right, rectTab.bottom);
	}

	CRect rectRight = rectClip;
	rectRight.left = rectFill.right;

	pDC->ExcludeClipRect(rectRight);

	CPen penLight(PS_SOLID, 1, afxGlobalData.clrBarHilite);

	pDC->SelectObject(&penLight);

	if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
	{
	}
	else
	{
		pDC->MoveTo(rectFill.left + 1, rectFill.bottom);
		pDC->LineTo(rectFill.left + nHeight, rectFill.top + 1);
		pDC->LineTo(rectFill.right - 1, rectFill.top + 1);
	}

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	if (bIsActive)
	{
		const int iBarHeight = 1;
		const int y = (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM) ? (rectTab.top - iBarHeight) :(rectTab.bottom);

		CRect rectFillTab(CPoint(rectTab.left + 2, y), CSize(rectTab.Width() - 1, iBarHeight));

		if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
		{
			rectFillTab.OffsetRect(-1, 1);
		}

		rectFillTab.right = min(rectFillTab.right, rectClip.right);

		CBrush br(clrTab);
		pDC->FillRect(rectFillTab, &br);
	}

	if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
	{
		rectTab.left += rectTab.Height() + CMFCBaseTabCtrl::AFX_TAB_IMAGE_MARGIN;
	}
	else
	{
		rectTab.left += rectTab.Height();
		rectTab.right -= CMFCBaseTabCtrl::AFX_TAB_IMAGE_MARGIN;
	}

	COLORREF clrText = pTabWnd->GetTabTextColor(iTab);

	COLORREF cltTextOld = (COLORREF)-1;
	if (!bIsActive && clrText != (COLORREF)-1)
	{
		cltTextOld = pDC->SetTextColor(clrText);
	}

	rectTab.right = min(rectTab.right, rectClip.right - 2);

	OnDrawTabContent(pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	if (cltTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(cltTextOld);
	}

	pDC->SelectClipRgn(NULL);
}

void CMFCVisualManagerOffice2003::OnFillTab(CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);

	if (pTabWnd->IsFlatTab() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode() || pTabWnd->IsDialogControl())
	{
		CMFCVisualManagerOfficeXP::OnFillTab(pDC, rectFill, pbrFill, iTab, bIsActive, pTabWnd);
		return;
	}

	ASSERT_VALID(pDC);

	COLORREF clr1 = CDrawingManager::PixelAlpha(m_clrBarGradientDark, 105);

	if (pTabWnd->GetTabBkColor(iTab) != (COLORREF)-1)
	{
		clr1 = pTabWnd->GetTabBkColor(iTab);

		if (clr1 == afxGlobalData.clrWindow && bIsActive)
		{
			pDC->FillRect(rectFill, &afxGlobalData.brWindow);
			return;
		}
	}
	else
	{
		if (m_bAlwaysFillTab)
		{
			if (bIsActive)
			{
				pDC->FillRect(rectFill, &afxGlobalData.brWindow);
				return;
			}
		}
		else
		{
			if (pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded())
			{
				if (bIsActive)
				{
					pDC->FillRect(rectFill, &afxGlobalData.brWindow);
					return;
				}
			}
			else if (!bIsActive)
			{
				return;
			}
		}
	}

	COLORREF clr2 = CDrawingManager::PixelAlpha(clr1, 120);

	CDrawingManager dm(*pDC);

	if (pTabWnd->GetLocation() == CMFCTabCtrl::LOCATION_TOP)
	{
		dm.FillGradient(rectFill, clr1, clr2, TRUE);
	}
	else
	{
		dm.FillGradient(rectFill, clr2, clr1, TRUE);
	}
}

BOOL CMFCVisualManagerOffice2003::OnEraseTabsFrame(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTabWnd);

	if (pTabWnd->IsFlatTab() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode() || pTabWnd->IsDialogControl())
	{
		return CMFCVisualManagerOfficeXP::OnEraseTabsFrame(pDC, rect, pTabWnd);
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor(pTabWnd->GetActiveTab());
	if (clrActiveTab == (COLORREF)-1 && (pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style()))
	{
		pDC->FillRect(rect, &afxGlobalData.brWindow);
		return TRUE;
	}

	CDrawingManager dm(*pDC);

	COLORREF clr1 = m_clrBarGradientDark;

	if (clrActiveTab != (COLORREF)-1)
	{
		clr1 = clrActiveTab;
	}

	COLORREF clr2 = CDrawingManager::PixelAlpha(clr1, 130);

	if (pTabWnd->GetLocation() == CMFCTabCtrl::LOCATION_BOTTOM)
	{
		COLORREF clr = clr1;
		clr1 = clr2;
		clr2 = clr;
	}

	dm.FillGradient2(rect, clr1, clr2, 45);
	return TRUE;
}

void CMFCVisualManagerOffice2003::OnEraseTabsButton(CDC* pDC, CRect rect, CMFCButton* pButton, CMFCBaseTabCtrl* pBaseTab)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);
	ASSERT_VALID(pBaseTab);

	CMFCTabCtrl* pWndTab = DYNAMIC_DOWNCAST(CMFCTabCtrl, pBaseTab);

	if (pWndTab == NULL || pBaseTab->IsFlatTab() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode() || pBaseTab->IsDialogControl())
	{
		CMFCVisualManagerOfficeXP::OnEraseTabsButton(pDC, rect, pButton, pBaseTab);
		return;
	}

	if ((pBaseTab->IsOneNoteStyle() || pBaseTab->IsVS2005Style()) && (pButton->IsPressed() || pButton->IsHighlighted()))
	{
		CDrawingManager dm(*pDC);

		if (pButton->IsPressed())
		{
			dm.FillGradient(rect, m_clrHighlightDnGradientDark, m_clrHighlightDnGradientLight);
		}
		else
		{
			dm.FillGradient(rect, m_clrHighlightGradientDark, m_clrHighlightGradientLight);
		}

		return;
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect(rect);

	pDC->SelectClipRgn(&rgn);

	CRect rectTabs;
	pWndTab->GetClientRect(&rectTabs);

	CRect rectTabArea;
	pWndTab->GetTabsRect(rectTabArea);

	if (pWndTab->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
	{
		rectTabs.top = rectTabArea.top;
	}
	else
	{
		rectTabs.bottom = rectTabArea.bottom;
	}

	pWndTab->MapWindowPoints(pButton, rectTabs);
	OnEraseTabsArea(pDC, rectTabs, pWndTab);

	pDC->SelectClipRgn(NULL);
}

void CMFCVisualManagerOffice2003::OnDrawTabsButtonBorder(CDC* pDC, CRect& rect, CMFCButton* pButton, UINT /*uiState*/, CMFCBaseTabCtrl* /*pWndTab*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsPushed() || pButton->IsHighlighted())
	{
		pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}

void CMFCVisualManagerOffice2003::ModifyGlobalColors()
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || !m_bIsStandardWinXPTheme || afxGlobalData.IsHighContrastMode())
	{
		//----------------------------------------------
		// Theme color may differ from the system color:
		//----------------------------------------------
		afxGlobalData.clrBarFace = GetThemeColor(m_hThemeButton, COLOR_3DFACE);
		afxGlobalData.clrBarShadow = GetThemeColor(m_hThemeButton, COLOR_3DSHADOW);
		afxGlobalData.clrBarHilite = GetThemeColor(m_hThemeButton, COLOR_3DHIGHLIGHT);
		afxGlobalData.clrBarDkShadow = GetThemeColor(m_hThemeButton, COLOR_3DDKSHADOW);
		afxGlobalData.clrBarLight = GetThemeColor(m_hThemeButton, COLOR_3DLIGHT);
	}
	else
	{
		COLORREF clrBase = GetBaseThemeColor();

		if (m_WinXPTheme == WinXpTheme_Olive)
		{
			COLORREF clrToolBarGradientDark = CDrawingManager::PixelAlpha(clrBase, 120);

			COLORREF clrToolBarGradientLight = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_WINDOW), 1., 2, 1);

			afxGlobalData.clrBarFace = CDrawingManager::SmartMixColors(clrToolBarGradientDark, clrToolBarGradientLight, 1., 2, 1);
		}
		else if (m_WinXPTheme == WinXpTheme_Silver)
		{
			COLORREF clrToolBarGradientDark = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_3DFACE), 0.75, 2);
			COLORREF clrToolBarGradientLight = CDrawingManager::SmartMixColors(clrBase, GetThemeColor(m_hThemeWindow, COLOR_WINDOW), 1.03);

			afxGlobalData.clrBarFace = CDrawingManager::PixelAlpha(CDrawingManager::SmartMixColors(clrToolBarGradientDark, clrToolBarGradientLight), 95);
		}
		else
		{
			afxGlobalData.clrBarFace = CDrawingManager::SmartMixColors(GetThemeColor(m_hThemeWindow, /*COLOR_HIGHLIGHT*/29), GetThemeColor(m_hThemeWindow, COLOR_WINDOW));
		}

		afxGlobalData.clrBarShadow = CDrawingManager::PixelAlpha(afxGlobalData.clrBarFace, 70);
		afxGlobalData.clrBarHilite = CDrawingManager::PixelAlpha(afxGlobalData.clrBarFace, 130);
		afxGlobalData.clrBarDkShadow = CDrawingManager::PixelAlpha(afxGlobalData.clrBarFace, 50);
		afxGlobalData.clrBarLight = CDrawingManager::PixelAlpha(afxGlobalData.clrBarFace, 110);
	}

	afxGlobalData.brBarFace.DeleteObject();
	afxGlobalData.brBarFace.CreateSolidBrush(afxGlobalData.clrBarFace);
}

void __stdcall CMFCVisualManagerOffice2003::SetUseGlobalTheme(BOOL bUseGlobalTheme/* = TRUE*/)
{
	m_bUseGlobalTheme = bUseGlobalTheme;

	CMFCVisualManager::GetInstance()->OnUpdateSystemColors();
	CMFCVisualManager::GetInstance()->RedrawAll();
}

void __stdcall CMFCVisualManagerOffice2003::SetStatusBarOfficeXPLook(BOOL bStatusBarOfficeXPLook/* = TRUE*/)
{
	m_bStatusBarOfficeXPLook = bStatusBarOfficeXPLook;

	CMFCVisualManager::GetInstance()->RedrawAll();
}

void __stdcall CMFCVisualManagerOffice2003::SetDefaultWinXPColors(BOOL bDefaultWinXPColors/* = TRUE*/)
{
	m_bDefaultWinXPColors = bDefaultWinXPColors;

	CMFCVisualManager::GetInstance()->OnUpdateSystemColors();
	CMFCVisualManager::GetInstance()->RedrawAll();
}

void CMFCVisualManagerOffice2003::GetTabFrameColors(const CMFCBaseTabCtrl* pTabWnd, COLORREF& clrDark, COLORREF& clrBlack,
	COLORREF& clrHighlight, COLORREF& clrFace, COLORREF& clrDarkShadow, COLORREF& clrLight, CBrush*& pbrFace, CBrush*& pbrBlack)
{
	ASSERT_VALID(pTabWnd);

	CMFCVisualManagerOfficeXP::GetTabFrameColors(pTabWnd, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack);

	if (pTabWnd->IsOneNoteStyle() || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode() || pTabWnd->IsDialogControl() || !m_bIsStandardWinXPTheme)
	{
		return;
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor(pTabWnd->GetActiveTab());

	if (clrActiveTab == (COLORREF)-1)
	{
		clrFace = afxGlobalData.clrWindow;
	}

	clrDark = afxGlobalData.clrBarShadow;
	clrBlack = afxGlobalData.clrBarDkShadow;
	clrHighlight = pTabWnd->IsVS2005Style() ? afxGlobalData.clrBarShadow : afxGlobalData.clrBarLight;
	clrDarkShadow = afxGlobalData.clrBarShadow;
	clrLight = afxGlobalData.clrBarFace;
}

void CMFCVisualManagerOffice2003::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnFillTasksPaneBackground(pDC, rectWorkArea);
		return;
	}

	ASSERT_VALID(pDC);

	CDrawingManager dm(*pDC);
	dm.FillGradient(rectWorkArea, m_clrTaskPaneGradientDark, m_clrTaskPaneGradientLight, TRUE);
}

void CMFCVisualManagerOffice2003::OnDrawTasksGroupCaption(CDC* pDC, CMFCTasksPaneTaskGroup* pGroup, BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/, BOOL bCanCollapse /*= FALSE*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pGroup);
	ASSERT_VALID(pGroup->m_pPage);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawTasksGroupCaption(pDC, pGroup, bIsHighlighted, bIsSelected, bCanCollapse);
		return;
	}

	CRect rectGroup = pGroup->m_rect;

	// -----------------------
	// Draw caption background
	// -----------------------
	POINT pts [7];

	const int nLeft = pGroup->m_rect.left;
	const int nTop = pGroup->m_rect.top;

	pts [0].x = nLeft;
	pts [0].y = pGroup->m_rect.bottom;

	pts [1].x = nLeft;
	pts [1].y = nTop + 4;

	pts [2].x = nLeft + 1;
	pts [2].y = nTop + 2;

	pts [3].x = nLeft + 2;
	pts [3].y = nTop + 1;

	pts [4].x = nLeft + 4;
	pts [4].y = nTop;

	pts [5].x = pGroup->m_rect.right;
	pts [5].y = nTop;

	pts [6].x = pGroup->m_rect.right;
	pts [6].y = pGroup->m_rect.bottom;

	CRgn rgn;
	rgn.CreatePolygonRgn(pts, 7, WINDING);

	pDC->SelectClipRgn(&rgn);

	CDrawingManager dm(*pDC);

	if (pGroup->m_bIsSpecial)
	{
		dm.FillGradient(pGroup->m_rect, m_clrTaskPaneGroupCaptionSpecDark, m_clrTaskPaneGroupCaptionSpecLight, FALSE);
	}
	else
	{
		dm.FillGradient(pGroup->m_rect, m_clrTaskPaneGroupCaptionLight, m_clrTaskPaneGroupCaptionDark, FALSE);
	}

	pDC->SelectClipRgn(NULL);

	// ---------------------------
	// Draw an icon if it presents
	// ---------------------------
	BOOL bShowIcon = (pGroup->m_hIcon != NULL && pGroup->m_sizeIcon.cx < rectGroup.Width() - rectGroup.Height());
	if (bShowIcon)
	{
		OnDrawTasksGroupIcon(pDC, pGroup, 5, bIsHighlighted, bIsSelected, bCanCollapse);
	}

	// -----------------------
	// Draw group caption text
	// -----------------------
	CFont* pFontOld = pDC->SelectObject(&afxGlobalData.fontBold);
	COLORREF clrTextOld = pDC->GetTextColor();

	if (bCanCollapse && bIsHighlighted)
	{
		pDC->SetTextColor(pGroup->m_clrTextHot == (COLORREF)-1 ? (pGroup->m_bIsSpecial ? m_clrTaskPaneGroupBorder : afxGlobalData.clrHilite) : pGroup->m_clrTextHot);
	}
	else
	{
		pDC->SetTextColor(pGroup->m_clrText == (COLORREF)-1 ? (pGroup->m_bIsSpecial ? m_clrTaskPaneGroupBorder : afxGlobalData.clrHilite) : pGroup->m_clrText);
	}

	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();
	int nCaptionHOffset = (nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset);

	CRect rectText = rectGroup;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx + 5: nCaptionHOffset);
	rectText.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectText.right = max(rectText.left, rectText.right -(bCanCollapse ? rectGroup.Height() : nCaptionHOffset));

	pDC->DrawText(pGroup->m_strName, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject(pFontOld);
	pDC->SetTextColor(clrTextOld);

	// -------------------------
	// Draw group caption button
	// -------------------------
	if (bCanCollapse && !pGroup->m_strName.IsEmpty())
	{
		CSize sizeButton = CMenuImages::Size();
		CRect rectButton = rectGroup;
		rectButton.left = max(rectButton.left, rectButton.right -(rectButton.Height()+1)/2 -(sizeButton.cx+1)/2);
		rectButton.top = max(rectButton.top, rectButton.bottom -(rectButton.Height()+1)/2 -(sizeButton.cy+1)/2);
		rectButton.right = rectButton.left + sizeButton.cx;
		rectButton.bottom = rectButton.top + sizeButton.cy;

		if (rectButton.right <= rectGroup.right && rectButton.bottom <= rectGroup.bottom)
		{
			if (bIsHighlighted)
			{
				// Draw button frame
				CBrush* pBrushOld = (CBrush*) pDC->SelectObject(&afxGlobalData.brBarFace);
				COLORREF clrBckOld = pDC->GetBkColor();

				pDC->Draw3dRect(&rectButton, afxGlobalData.clrWindow, afxGlobalData.clrBarShadow);

				pDC->SetBkColor(clrBckOld);
				pDC->SelectObject(pBrushOld);
			}

			if (!pGroup->m_bIsCollapsed)
			{
				CMenuImages::Draw(pDC, CMenuImages::IdArrowUp, rectButton.TopLeft());
			}
			else
			{
				CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rectButton.TopLeft());
			}
		}
	}
}

void CMFCVisualManagerOffice2003::OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial /*= FALSE*/)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnFillTasksGroupInterior(pDC, rect, bSpecial);
		return;
	}

	ASSERT_VALID(pDC);

	CDrawingManager dm(*pDC);
	if (bSpecial)
	{
		dm.FillGradient(rect, m_clrTaskPaneGroupCaptionSpecDark, m_clrTaskPaneGroupCaptionSpecLight, TRUE);
	}
	else
	{
		dm.FillGradient(rect, m_clrTaskPaneGroupAreaDark, m_clrTaskPaneGroupAreaLight, TRUE);
	}
}

void CMFCVisualManagerOffice2003::OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL /*bSpecial = FALSE*/, BOOL /*bNoTitle = FALSE*/)
{
	ASSERT_VALID(pDC);

	// Draw underline
	CPen* pPenOld = (CPen*) pDC->SelectObject(&m_penTaskPaneGroupBorder);

	rect.right -= 1;
	rect.bottom -= 1;
	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.right, rect.top);
	pDC->LineTo(rect.right, rect.bottom);
	pDC->LineTo(rect.left, rect.bottom);
	pDC->LineTo(rect.left, rect.top);

	pDC->SelectObject(pPenOld);
}

void CMFCVisualManagerOffice2003::OnDrawTask(CDC* pDC, CMFCTasksPaneTask* pTask, CImageList* pIcons, BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/)
{
	ASSERT_VALID(pTask);
	ASSERT_VALID(pDC);

	if (pTask->m_bIsSeparator)
	{
		CRect rectText = pTask->m_rect;

		CPen* pPenOld = (CPen*) pDC->SelectObject(&m_penSeparator);

		pDC->MoveTo(rectText.left, rectText.CenterPoint().y);
		pDC->LineTo(rectText.right, rectText.CenterPoint().y);

		pDC->SelectObject(pPenOld);
		return;
	}

	COLORREF clrOld = afxGlobalData.clrHotLinkNormalText;
	afxGlobalData.clrHotLinkNormalText = afxGlobalData.clrHilite;

	CMFCVisualManagerOfficeXP::OnDrawTask(pDC, pTask, pIcons, bIsHighlighted, bIsSelected);

	afxGlobalData.clrHotLinkNormalText = clrOld;
}

void CMFCVisualManagerOffice2003::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize, int iImage, BOOL bHilited)
{
	ASSERT_VALID(pDC);

	CRect rectImage(CPoint(0, 0), CMenuImages::Size());

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect(rectFill, &afxGlobalData.brBarFace);

	if (bHilited)
	{
		CBrush br(afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode() ?
			afxGlobalData.clrWindow : m_clrHighlightMenuItem == (COLORREF)-1 ? m_clrHighlight : m_clrHighlightMenuItem);

		pDC->FillRect(rect, &br);
		pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
	else
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
	}

	CMenuImages::Draw(pDC, (CMenuImages::IMAGES_IDS) iImage, rect);
}

COLORREF CMFCVisualManagerOffice2003::OnFillCommandsListBackground(CDC* pDC, CRect rect, BOOL bIsSelected)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		return CMFCVisualManagerOfficeXP::OnFillCommandsListBackground(pDC, rect, bIsSelected);
	}

	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	COLORREF clrText = afxGlobalData.clrBarText;

	int nImageWidth = CMFCToolBar::GetMenuImageSize().cx + GetMenuImageMargin();

	if (bIsSelected)
	{
		rect.left = 0;

		COLORREF color = m_clrHighlightMenuItem == (COLORREF)-1 ? m_clrHighlight : m_clrHighlightMenuItem;

		CBrush br(color);
		pDC->FillRect(&rect, &br);

		pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);

		// Now, we should define a menu text color...
		if (GetRValue(color) > 128 && GetGValue(color) > 128 && GetBValue(color) > 128)
		{
			clrText = RGB(0, 0, 0);
		}
		else
		{
			clrText = RGB(255, 255, 255);
		}
	}
	else
	{
		pDC->FillRect(rect, &m_brMenuLight);

		CRect rectImages = rect;
		rectImages.right = rectImages.left + nImageWidth + AFX_MENU_IMAGE_MARGIN;

		CDrawingManager dm(*pDC);
		dm.FillGradient(rectImages, m_clrToolBarGradientLight, m_clrToolBarGradientDark, FALSE);

		clrText = afxGlobalData.clrBarText;
	}

	return clrText;
}

void CMFCVisualManagerOffice2003::OnDrawStatusBarProgress(CDC* pDC, CMFCStatusBar* pStatusBar, CRect rectProgress,
	int nProgressTotal, int nProgressCurr, COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText, BOOL bProgressText)
{
	if (!DrawStatusBarProgress(pDC, pStatusBar, rectProgress, nProgressTotal, nProgressCurr, clrBar, clrProgressBarDest, clrProgressText, bProgressText))
	{
		CMFCVisualManagerOfficeXP::OnDrawStatusBarProgress(pDC, pStatusBar, rectProgress, nProgressTotal, nProgressCurr, clrBar, clrProgressBarDest, clrProgressText, bProgressText);
	}
}

void CMFCVisualManagerOffice2003::OnDrawStatusBarPaneBorder(CDC* pDC, CMFCStatusBar* pBar, CRect rectPane, UINT uiID, UINT nStyle)
{
	if (!m_bStatusBarOfficeXPLook || m_hThemeStatusBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawStatusBarPaneBorder(pDC, pBar, rectPane, uiID, nStyle);
	}

	if (m_hThemeStatusBar != NULL && m_pfDrawThemeBackground != NULL && !(nStyle & SBPS_NOBORDERS))
	{
		(*m_pfDrawThemeBackground)(m_hThemeStatusBar, pDC->GetSafeHdc(), SP_PANE, 0, &rectPane, 0);
	}
}

void CMFCVisualManagerOffice2003::OnFillHeaderCtrlBackground(CMFCHeaderCtrl* pCtrl, CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);
	CMFCVisualManagerOfficeXP::OnFillHeaderCtrlBackground(pCtrl, pDC, rect);
}

COLORREF CMFCVisualManagerOffice2003::OnDrawPaneCaption(CDC* pDC, CDockablePane* pBar, BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		return CMFCVisualManagerOfficeXP::OnDrawPaneCaption(pDC, pBar, bActive, rectCaption, rectButtons);
	}

	CDrawingManager dm(*pDC);

	if (!bActive)
	{
		dm.FillGradient(rectCaption, m_clrToolBarGradientDark, m_clrToolBarGradientLight, TRUE);
	}
	else
	{
		dm.FillGradient(rectCaption,  m_clrHighlightGradientDark, m_clrHighlightGradientLight, TRUE);
	}

	return afxGlobalData.clrBarText;
}

void CMFCVisualManagerOffice2003::OnFillAutoHideButtonBackground(CDC* pDC, CRect rect, CMFCAutoHideButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnFillAutoHideButtonBackground(pDC, rect, pButton);
		return;
	}

	CDrawingManager dm(*pDC);

	if (pButton->IsActive())
	{
		dm.FillGradient(rect, m_clrHighlightGradientLight, m_clrHighlightGradientDark, pButton->IsHorizontal());
	}
	else
	{
		dm.FillGradient(rect, m_clrBarGradientLight, m_clrBarGradientDark, pButton->IsHorizontal());
	}
}

void CMFCVisualManagerOffice2003::OnDrawAutoHideButtonBorder(CDC* pDC, CRect rectBounds, CRect rectBorderSize, CMFCAutoHideButton* pButton)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawAutoHideButtonBorder(pDC, rectBounds, rectBorderSize, pButton);
		return;
	}

	COLORREF clr = afxGlobalData.clrBarShadow;
	COLORREF clrText = pDC->GetTextColor();

	if (rectBorderSize.left > 0)
	{
		pDC->FillSolidRect(rectBounds.left, rectBounds.top, rectBounds.left + rectBorderSize.left, rectBounds.bottom, clr);
	}
	if (rectBorderSize.top > 0)
	{
		pDC->FillSolidRect(rectBounds.left, rectBounds.top, rectBounds.right, rectBounds.top + rectBorderSize.top, clr);
	}
	if (rectBorderSize.right > 0)
	{
		pDC->FillSolidRect(rectBounds.right - rectBorderSize.right, rectBounds.top, rectBounds.right, rectBounds.bottom, clr);
	}
	if (rectBorderSize.bottom > 0)
	{
		pDC->FillSolidRect(rectBounds.left, rectBounds.bottom - rectBorderSize.bottom, rectBounds.right, rectBounds.bottom, clr);
	}

	pDC->SetTextColor(clrText);
}

void CMFCVisualManagerOffice2003::OnDrawOutlookBarSplitter(CDC* pDC, CRect rectSplitter)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawOutlookBarSplitter(pDC, rectSplitter);
		return;
	}

	CDrawingManager dm(*pDC);

	dm.FillGradient(rectSplitter, m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);

	const int nBoxesNumber = 10;
	const int nBoxSize = rectSplitter.Height() - 3;

	int x = rectSplitter.CenterPoint().x - nBoxSize * nBoxesNumber / 2;
	int y = rectSplitter.top + 2;

	for (int nBox = 0; nBox < nBoxesNumber; nBox++)
	{
		pDC->FillSolidRect(x + 1, y + 1, nBoxSize / 2, nBoxSize / 2, afxGlobalData.clrBtnHilite); pDC->FillSolidRect(x, y, nBoxSize / 2, nBoxSize / 2, m_clrGripper);

		x += nBoxSize;
	}
}

void CMFCVisualManagerOffice2003::OnFillOutlookBarCaption(CDC* pDC, CRect rectCaption, COLORREF& clrText)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnFillOutlookBarCaption(pDC, rectCaption, clrText);
		return;
	}

	CDrawingManager dm(*pDC);

	dm.FillGradient(rectCaption, m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);
	clrText = afxGlobalData.clrBarHilite;
}

BOOL CMFCVisualManagerOffice2003::OnDrawBrowseButton(CDC* pDC, CRect rect, CMFCEditBrowseCtrl* /*pEdit*/, CMFCVisualManager::AFX_BUTTON_STATE state, COLORREF& /*clrText*/)
{
	ASSERT_VALID(pDC);

	CRect rectFrame = rect;
	rectFrame.InflateRect(0, 1, 1, 1);

	switch(state)
	{
	case ButtonsIsPressed:
		OnFillHighlightedArea(pDC, rect, &m_brHighlightDn, NULL);
		pDC->Draw3dRect(&rectFrame, m_clrToolBarGradientDark, m_clrToolBarGradientDark);
		break;

	case ButtonsIsHighlighted:
		OnFillHighlightedArea(pDC, rect, &m_brHighlight, NULL);
		pDC->Draw3dRect(&rectFrame, m_clrToolBarGradientDark, m_clrToolBarGradientDark);
		break;

	default:
		{
			CDrawingManager dm(*pDC);

			dm.FillGradient(rect, afxGlobalData.clrBtnFace, afxGlobalData.clrBtnHilite);
			pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarHilite);
		}
		break;
	}

	return TRUE;
}

COLORREF CMFCVisualManagerOffice2003::GetWindowColor() const
{
	return GetThemeColor(m_hThemeWindow, COLOR_WINDOW);
}

void CMFCVisualManagerOffice2003::OnHighlightRarelyUsedMenuItems(CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID(pDC);

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CMFCToolBar::GetMenuImageSize().cx + 2 * GetMenuImageMargin() + 2;

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnHighlightRarelyUsedMenuItems(pDC, rectRarelyUsed);
		return;
	}

	CDrawingManager dm(*pDC);
	dm.FillGradient(rectRarelyUsed, m_clrMenuRarelyUsed, m_clrToolBarGradientDark, FALSE);
}

void CMFCVisualManagerOffice2003::OnDrawControlBorder(CWnd* pWndCtrl)
{
	if (m_hThemeComboBox == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawControlBorder(pWndCtrl);
		return;
	}

	ASSERT_VALID(pWndCtrl);

	CWindowDC dc(pWndCtrl);

	CRect rect;
	pWndCtrl->GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	COLORREF clrBorder = (COLORREF)-1;

	if ((m_pfGetThemeColor == NULL) || ((*m_pfGetThemeColor)(m_hThemeComboBox, 5, 0, 3801, &clrBorder) != S_OK))
	{
		CMFCVisualManagerOfficeXP::OnDrawControlBorder(pWndCtrl);
		return;
	}

	dc.Draw3dRect(&rect, clrBorder, clrBorder);

	rect.DeflateRect(1, 1);
	dc.Draw3dRect(rect, afxGlobalData.clrWindow, afxGlobalData.clrWindow);
}

void CMFCVisualManagerOffice2003::OnDrawExpandingBox(CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
{
	ASSERT_VALID(pDC);

	if (m_hThemeTree == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawExpandingBox(pDC, rect, bIsOpened, colorBox);
		return;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeTree, pDC->GetSafeHdc(), TVP_GLYPH, bIsOpened ? GLPS_OPENED : GLPS_CLOSED, &rect, 0);
	}
}

void CMFCVisualManagerOffice2003::GetSmartDockingBaseGuideColors(COLORREF& clrBaseGroupBackground, COLORREF& clrBaseGroupBorder)
{
	if (afxGlobalData.m_nBitsPerPixel > 8 && !afxGlobalData.IsHighContrastMode())
	{
		clrBaseGroupBackground = RGB(228, 228, 228);
		clrBaseGroupBorder = RGB(181, 181, 181);
	}
	else
	{
		clrBaseGroupBackground = afxGlobalData.clrBarFace;
		clrBaseGroupBorder = afxGlobalData.clrBarShadow;
	}
}

COLORREF CMFCVisualManagerOffice2003::GetSmartDockingHighlightToneColor()
{
	if (afxGlobalData.m_nBitsPerPixel > 8 && !afxGlobalData.IsHighContrastMode())
	{
		WinXpTheme theme = GetStandardWindowsTheme();

		switch(theme)
		{
		case WinXpTheme_Blue:
			return RGB(61, 123, 241);

		case WinXpTheme_Olive:
			return RGB(190, 146, 109);

		case WinXpTheme_Silver:
			return RGB(134, 130, 169);
		}
	}

	return CMFCVisualManagerOfficeXP::GetSmartDockingHighlightToneColor();
}

void CMFCVisualManagerOffice2003::OnDrawStatusBarSizeBox(CDC* pDC, CMFCStatusBar* pStatBar, CRect rectSizeBox)
{
	if (m_hThemeScrollBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawStatusBarSizeBox(pDC, pStatBar, rectSizeBox);
		return;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeScrollBar, pDC->GetSafeHdc(), SBP_SIZEBOX, SZB_RIGHTALIGN, &rectSizeBox, 0);
	}
}

COLORREF CMFCVisualManagerOffice2003::GetBaseThemeColor()
{
	return m_bIsStandardWinXPTheme && m_hThemeWindow != NULL ? GetThemeColor(m_hThemeWindow, 29) : afxGlobalData.clrBarFace;
}

void CMFCVisualManagerOffice2003::OnHighlightQuickCustomizeMenuButton(CDC* pDC, CMFCToolBarMenuButton* /*pButton*/, CRect rect)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.IsHighContrastMode())
	{
		pDC->FillRect(rect, &m_brBarBkgnd);
	}
	else
	{
		CBrush br(m_clrToolBarGradientLight);
		pDC->FillRect(rect, &br);
	}

	pDC->Draw3dRect(rect, m_clrMenuBorder, m_clrMenuBorder);
}

void CMFCVisualManagerOffice2003::OnDrawHeaderCtrlBorder(CMFCHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	if (m_hThemeHeader == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawHeaderCtrlBorder(pCtrl, pDC, rect, bIsPressed, bIsHighlighted);
		return;
	}

	int nState = HIS_NORMAL;

	if (bIsPressed)
	{
		nState = HIS_PRESSED;
	}
	else if (bIsHighlighted)
	{
		nState = HIS_HOT;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeHeader, pDC->GetSafeHdc(), HP_HEADERITEM, nState, &rect, 0);
	}
}

void CMFCVisualManagerOffice2003::OnFillPopupWindowBackground(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnFillPopupWindowBackground(pDC, rect);
		return;
	}

	CDrawingManager dm(*pDC);
	dm.FillGradient(rect, m_clrBarGradientDark, m_clrBarGradientLight);
}

void CMFCVisualManagerOffice2003::OnDrawPopupWindowBorder(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
}

COLORREF  CMFCVisualManagerOffice2003::OnDrawPopupWindowCaption(CDC* pDC, CRect rectCaption, CMFCDesktopAlertWnd* pPopupWnd)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		return CMFCVisualManagerOfficeXP::OnDrawPopupWindowCaption(pDC, rectCaption, pPopupWnd);
	}

	CDrawingManager dm(*pDC);
	dm.FillGradient(rectCaption, m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);

	if (pPopupWnd->HasSmallCaption())
	{
		CRect rectGripper = rectCaption;

		int xCenter = rectGripper.CenterPoint().x;
		int yCenter = rectGripper.CenterPoint().y;

		rectGripper.left = xCenter - 20;
		rectGripper.right = xCenter + 20;

		rectGripper.top = yCenter - 4;
		rectGripper.bottom = yCenter + 2;

		OnDrawBarGripper(pDC, rectGripper, FALSE, NULL);
	}

	// get the text color
	return afxGlobalData.clrBarHilite;
}

void CMFCVisualManagerOffice2003::OnErasePopupWindowButton(CDC* pDC, CRect rc, CMFCDesktopAlertWndButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnErasePopupWindowButton(pDC, rc, pButton);
		return;
	}

	if (pButton->IsPressed())
	{
		COLORREF color = m_clrHighlightDnGradientLight == (COLORREF)-1 ? m_clrHighlightDn : m_clrHighlightDnGradientLight;
		CBrush br(color);
		pDC->FillRect(&rc, &br);
		return;
	}
	else if (pButton->IsHighlighted() || pButton->IsPushed())
	{
		COLORREF color = m_clrHighlightMenuItem == (COLORREF)-1 ? m_clrHighlight : m_clrHighlightMenuItem;
		CBrush br(color);
		pDC->FillRect(&rc, &br);
		return;
	}

	CRect rectParent;
	pButton->GetParent()->GetClientRect(rectParent);

	pButton->GetParent()->MapWindowPoints(pButton, rectParent);
	OnFillPopupWindowBackground(pDC, rectParent);
}

void CMFCVisualManagerOffice2003::OnDrawPopupWindowButtonBorder(CDC* pDC, CRect rc, CMFCDesktopAlertWndButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsHighlighted() || pButton->IsPushed() || pButton->IsCaptionButton())
	{
		pDC->Draw3dRect(rc, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}

void CMFCVisualManagerOffice2003::OnDrawCheckBoxEx(CDC *pDC, CRect rect, int nState, BOOL bHighlighted, BOOL bPressed, BOOL bEnabled)
{
	if (!DrawCheckBox(pDC, rect, bHighlighted, nState, bEnabled, bPressed))
	{
		CMFCVisualManagerOfficeXP::OnDrawCheckBoxEx(pDC, rect, nState, bHighlighted, bPressed, bEnabled);
	}
}

COLORREF CMFCVisualManagerOffice2003::GetPropertyGridGroupColor(CMFCPropertyGridCtrl* pPropList)
{
	return CMFCVisualManager::GetPropertyGridGroupColor(pPropList);
}

COLORREF CMFCVisualManagerOffice2003::GetPropertyGridGroupTextColor(CMFCPropertyGridCtrl* pPropList)
{
	return CMFCVisualManager::GetPropertyGridGroupTextColor(pPropList);
}

COLORREF CMFCVisualManagerOffice2003::OnDrawRibbonCategoryTab(CDC* pDC, CMFCRibbonTab* pTab, BOOL bIsActive)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		return CMFCVisualManagerOfficeXP::OnDrawRibbonCategoryTab(pDC, pTab, bIsActive);
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pTab);

	CMFCRibbonCategory* pCategory = pTab->GetParentCategory();
	ASSERT_VALID(pCategory);
	CMFCRibbonBar* pBar = pCategory->GetParentRibbonBar();
	ASSERT_VALID(pBar);

	bIsActive = bIsActive && ((pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ELEMENTS) == 0 || pTab->GetDroppedDown() != NULL);

	const BOOL bIsHighlighted = pTab->IsHighlighted() && !pTab->IsDroppedDown();

	CRect rectTab = pTab->GetRect();
	rectTab.top += 3;

	const int nTrancateRatio = pBar->GetTabTrancateRatio();

	if (nTrancateRatio > 0)
	{
		CRect rectRight = rectTab;
		rectRight.left = rectRight.right - 1;

		const int nPercent = max(10, 100 - nTrancateRatio / 2);

		COLORREF color1 = CDrawingManager::PixelAlpha(afxGlobalData.clrBarShadow, nPercent);
		COLORREF color2 = CDrawingManager::PixelAlpha(color1, 120);

		CDrawingManager dm(*pDC);
		dm.FillGradient(rectRight, color1, color2, TRUE);
	}

	if (!bIsActive && !bIsHighlighted)
	{
		return afxGlobalData.clrBarText;
	}

	rectTab.right -= 2;

	CPen pen(PS_SOLID, 1, afxGlobalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject(&pen);
	ENSURE(pOldPen != NULL);

#define AFX_RIBBONTAB_POINTS_NUM 8
	POINT pts [AFX_RIBBONTAB_POINTS_NUM];

	pts [0] = CPoint(rectTab.left, rectTab.bottom);
	pts [1] = CPoint(rectTab.left + 1, rectTab.bottom - 1);
	pts [2] = CPoint(rectTab.left + 1, rectTab.top + 2);
	pts [3] = CPoint(rectTab.left + 3, rectTab.top);
	pts [4] = CPoint(rectTab.right - 3, rectTab.top);
	pts [5] = CPoint(rectTab.right - 1, rectTab.top + 2);
	pts [6] = CPoint(rectTab.right - 1, rectTab.bottom - 1);
	pts [7] = CPoint(rectTab.right, rectTab.bottom);

	CRgn rgnClip;
	rgnClip.CreatePolygonRgn(pts, AFX_RIBBONTAB_POINTS_NUM, WINDING);

	pDC->SelectClipRgn(&rgnClip);

	CDrawingManager dm(*pDC);

	const BOOL bIsSelected = pTab->IsSelected();

	COLORREF clrFill = bIsSelected ? m_clrHighlightGradientDark : RibbonCategoryColorToRGB(pCategory->GetTabColor());

	COLORREF clr1 = afxGlobalData.clrBarFace;
	COLORREF clr2 = (clrFill == (COLORREF)-1) ? CDrawingManager::PixelAlpha(clr1, 120) : clrFill;

	if (bIsHighlighted)
	{
		if (bIsActive)
		{
			clr2 = m_clrHighlightGradientLight;
		}
		else
		{
			if (clrFill == (COLORREF)-1)
			{
				clr1 = m_clrHighlightGradientDark;
				clr2 = m_clrHighlightGradientLight;
			}
			else
			{
				clr1 = clrFill;
				clr2 = CDrawingManager::PixelAlpha(clr1, 120);
			}
		}
	}

	dm.FillGradient(rectTab, clr1, clr2, TRUE);

	pDC->SelectClipRgn(NULL);

	pDC->Polyline(pts, AFX_RIBBONTAB_POINTS_NUM);

	if (bIsHighlighted && bIsActive && !bIsSelected)
	{
		//---------------------
		// Draw internal frame:
		//---------------------
		const CPoint ptCenter = rectTab.CenterPoint();

		for (int i = 0; i < AFX_RIBBONTAB_POINTS_NUM; i++)
		{
			if (pts [i].x < ptCenter.x)
			{
				pts [i].x++;
			}
			else
			{
				pts [i].x--;
			}

			if (pts [i].y < ptCenter.y)
			{
				pts [i].y++;
			}
			else
			{
				pts [i].y--;
			}
		}

		CPen penInternal(PS_SOLID, 1, m_clrHighlightGradientDark);
		pDC->SelectObject(&penInternal);

		pDC->Polyline(pts, AFX_RIBBONTAB_POINTS_NUM);
		pDC->SelectObject(pOldPen);
	}

	pDC->SelectObject(pOldPen);

	return afxGlobalData.clrBarText;
}

COLORREF CMFCVisualManagerOffice2003::OnDrawRibbonButtonsGroup(CDC* pDC, CMFCRibbonButtonsGroup* pGroup, CRect rect)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pGroup);

	if (DYNAMIC_DOWNCAST(CMFCRibbonQuickAccessToolBar, pGroup) != NULL || pGroup->GetCount() == 0)
	{
		return(COLORREF)-1;
	}

	CMFCRibbonBaseElement* pButton = pGroup->GetButton(0);
	ASSERT_VALID(pButton);

	if (!pButton->IsShowGroupBorder())
	{
		return(COLORREF)-1;
	}

	const int dx = 2;
	const int dy = 2;

	CPen pen(PS_SOLID, 1, m_clrToolBarGradientDark);
	CPen* pOldPen = pDC->SelectObject(&pen);
	ENSURE(pOldPen != NULL);

	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject(NULL_BRUSH);
	ENSURE(pOldBrush != NULL);

	rect.DeflateRect(1, 1);
	pDC->RoundRect(rect, CPoint(dx, dy));

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	return(COLORREF)-1;
}

COLORREF CMFCVisualManagerOffice2003::OnDrawRibbonCategoryCaption(CDC* pDC, CMFCRibbonContextCaption* pContextCaption)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pContextCaption);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		return CMFCVisualManagerOfficeXP::OnDrawRibbonCategoryCaption(pDC, pContextCaption);
	}

	COLORREF clrFill = RibbonCategoryColorToRGB(pContextCaption->GetColor());
	CRect rect = pContextCaption->GetRect();

	if (clrFill != (COLORREF)-1)
	{
		CDrawingManager dm(*pDC);
		dm.FillGradient(rect, clrFill, afxGlobalData.clrBarFace, TRUE);
	}

	return afxGlobalData.clrBarText;
}

void CMFCVisualManagerOffice2003::OnDrawRibbonSliderZoomButton(CDC* pDC, CMFCRibbonSlider* pSlider, CRect rect, BOOL bIsZoomOut, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawRibbonSliderZoomButton(pDC, pSlider, rect, bIsZoomOut, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	COLORREF clrLine = (bIsPressed || bIsHighlighted) ? afxGlobalData.clrBarDkShadow : afxGlobalData.clrBtnDkShadow;

	CPoint ptCenter = rect.CenterPoint();
	CRect rectCircle(CPoint(ptCenter.x - 7, ptCenter.y - 7), CSize(15, 15));
	CDrawingManager dm(*pDC);

	COLORREF clrFill = (COLORREF)-1;

	if (bIsPressed || bIsHighlighted)
	{
		clrFill = bIsPressed ? m_clrHighlightDnGradientLight : m_clrHighlightDnGradientDark;
	}

	dm.DrawEllipse(rectCircle, clrFill, clrLine);

	// Draw +/- sign:
	CRect rectSign(CPoint(ptCenter.x - 3, ptCenter.y - 3), CSize(7, 7));

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		dm.DrawLine(rectSign.left, ptCenter.y, rectSign.right, ptCenter.y, clrLine);

		if (!bIsZoomOut)
		{
			dm.DrawLine(ptCenter.x, rectSign.top, ptCenter.x, rectSign.bottom, clrLine);
		}
	}
	else
	{
		CPen penLine(PS_SOLID, 1, clrLine);
		CPen* pOldPen = pDC->SelectObject(&penLine);

		pDC->MoveTo(rectSign.left, ptCenter.y);
		pDC->LineTo(rectSign.right, ptCenter.y);

		if (!bIsZoomOut)
		{
			pDC->MoveTo(ptCenter.x, rectSign.top);
			pDC->LineTo(ptCenter.x, rectSign.bottom);
		}

		pDC->SelectObject(pOldPen);
	}
}

void CMFCVisualManagerOffice2003::OnDrawRibbonSliderChannel(CDC* pDC, CMFCRibbonSlider* pSlider, CRect rect)
{
	ASSERT_VALID(pDC);

	if (m_hThemeTrack == NULL || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawRibbonSliderChannel(pDC, pSlider, rect);
		return;
	}

	rect.InflateRect(0, 1);

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeTrack, pDC->GetSafeHdc(), TKP_TRACK, 1, &rect, 0);
	}
}

void CMFCVisualManagerOffice2003::OnDrawRibbonSliderThumb(CDC* pDC, CMFCRibbonSlider* pSlider, CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	ASSERT_VALID(pDC);

	if (m_hThemeTrack == NULL || afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CMFCVisualManagerOfficeXP::OnDrawRibbonSliderThumb(pDC, pSlider, rect, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeTrack, pDC->GetSafeHdc(), TKP_THUMBBOTTOM, bIsPressed ? 3 : bIsHighlighted ? 2 : 1, &rect, 0);
	}
}

COLORREF CMFCVisualManagerOffice2003::OnDrawRibbonStatusBarPane(CDC* pDC, CMFCRibbonStatusBar* pBar, CMFCRibbonStatusBarPane* pPane)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pPane);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode() || m_hThemeStatusBar == NULL)
	{
		return CMFCVisualManagerOfficeXP::OnDrawRibbonStatusBarPane(pDC, pBar, pPane);
	}

	CRect rect = pPane->GetRect();

	if (pPane->IsHighlighted())
	{
		CRect rectButton = rect;
		rectButton.DeflateRect(1, 1);

		OnFillHighlightedArea(pDC, rectButton, pPane->IsPressed() ? &m_brHighlightDn : &m_brHighlight, NULL);

		pDC->Draw3dRect(rectButton, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}

	return(COLORREF)-1;
}

void CMFCVisualManagerOffice2003::OnDrawRibbonProgressBar(CDC* pDC, CMFCRibbonProgressBar* pProgress, CRect rectProgress, CRect rectChunk, BOOL bInfiniteMode)
{
	ASSERT_VALID(pDC);

#define PP_BAR 1
#define PP_CHUNK 3

	if (m_hThemeProgress != NULL && m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeProgress, pDC->GetSafeHdc(), PP_BAR, 0, &rectProgress, 0);

		if (!rectChunk.IsRectEmpty())
		{
			rectChunk.DeflateRect(2, 2);
			(*m_pfDrawThemeBackground)(m_hThemeProgress, pDC->GetSafeHdc(), PP_CHUNK, 0, &rectChunk, 0);
		}
	}
	else
	{
		CMFCVisualManagerOfficeXP::OnDrawRibbonProgressBar(pDC, pProgress, rectProgress, rectChunk, bInfiniteMode);
	}
}

void CMFCVisualManagerOffice2003::OnDrawRibbonQuickAccessToolBarSeparator(CDC* pDC, CMFCRibbonSeparator* /*pSeparator*/, CRect rect)
{
	ASSERT_VALID(pDC);

	int x = rect.CenterPoint().x;

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawLine(x, rect.top, x, rect.bottom - 1, afxGlobalData.clrBarDkShadow);
		dm.DrawLine(x + 1, rect.top + 1, x + 1, rect.bottom, afxGlobalData.clrBarLight);
	}
	else
	{
		CPen* pOldPen = pDC->SelectObject(&m_penSeparator);
		ENSURE(pOldPen != NULL);

		pDC->MoveTo(x, rect.top);
		pDC->LineTo(x, rect.bottom - 1);

		pDC->SelectObject(&m_penSeparatorLight);

		pDC->MoveTo(x + 1, rect.top + 1);
		pDC->LineTo(x + 1, rect.bottom);

		pDC->SelectObject(pOldPen);
	}
}

BOOL CMFCVisualManagerOffice2003::GetToolTipInfo(CMFCToolTipInfo& params, UINT /*nType*/ /*= (UINT)(-1)*/)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		return CMFCVisualManagerOfficeXP::GetToolTipInfo(params);
	}

	params.m_bBoldLabel = TRUE;
	params.m_bDrawDescription = TRUE;
	params.m_bDrawIcon = TRUE;
	params.m_bRoundedCorners = TRUE;
	params.m_bDrawSeparator = FALSE;

	params.m_clrFill = afxGlobalData.clrBarHilite;
	params.m_clrFillGradient = afxGlobalData.clrBarFace;
	params.m_clrText = afxGlobalData.clrBarText;
	params.m_clrBorder = afxGlobalData.clrBarShadow;

	return TRUE;
}



