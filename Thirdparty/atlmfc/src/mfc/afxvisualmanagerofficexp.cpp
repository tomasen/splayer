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

#include "afxvisualmanagerofficexp.h"
#include "afxdrawmanager.h"
#include "afxmenubar.h"
#include "afxpopupmenu.h"
#include "afxtoolbarmenubutton.h"
#include "afxoutlookbarpane.h"
#include "afxcolorbar.h"
#include "afxbasetabctrl.h"
#include "afxdockablepane.h"
#include "afxautohidedocksite.h"
#include "afxglobals.h"
#include "afxoutlookbarpanebutton.h"
#include "afxpaneframewnd.h"
#include "afxtaskspaneframewnd.h"
#include "afxtoolbareditboxbutton.h"
#include "afxtaskspane.h"
#include "afxdesktopalertwnd.h"
#include "afxpropertygridctrl.h"
#include "afxcustomizebutton.h"
#include "afxribbonbutton.h"
#include "afxribbonpanelmenu.h"
#include "afxribboncombobox.h"
#include "afxribbonpanel.h"
#include "afxribbonlabel.h"
#include "afxribbonstatusbarpane.h"
#include "afxcaptionbar.h"
#include "afxribboncolorbutton.h"
#include "afxribboncategory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCVisualManagerOfficeXP, CMFCVisualManager)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCVisualManagerOfficeXP::CMFCVisualManagerOfficeXP(BOOL bIsTemporary) : CMFCVisualManager(bIsTemporary)
{
	m_bConnectMenuToParent = TRUE;

	m_nVertMargin = 4;
	m_nHorzMargin = 4;
	m_nGroupVertOffset = 4;
	m_nGroupCaptionHeight = 0;
	m_nGroupCaptionHorzOffset = 0;
	m_nGroupCaptionVertOffset = 0;
	m_nTasksHorzOffset = 12;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = FALSE;

	m_bMenuFlatLook = TRUE;
	m_bShadowHighlightedImage = TRUE;
	m_bEmbossDisabledImage = FALSE;
	m_bFadeInactiveImage = TRUE;
	m_nMenuShadowDepth = 4;
	m_nMenuBorderSize = 1;
	m_bShdowDroppedDownMenuButton = TRUE;

	m_bOfficeXPStyleMenus = TRUE;
	m_bDrawLastTabLine = TRUE;

	afxGlobalData.UpdateSysColors();
	OnUpdateSystemColors();
}

CMFCVisualManagerOfficeXP::~CMFCVisualManagerOfficeXP()
{
}

void CMFCVisualManagerOfficeXP::OnUpdateSystemColors()
{
	CMFCVisualManager::OnUpdateSystemColors();

	m_brBarBkgnd.DeleteObject();
	m_brMenuRarelyUsed.DeleteObject();
	m_brMenuLight.DeleteObject();
	m_brHighlight.DeleteObject();
	m_brHighlightDn.DeleteObject();
	m_brHighlightChecked.DeleteObject();

	m_brFloatToolBarBorder.DeleteObject();

	m_penSeparator.DeleteObject();
	m_brTabBack.DeleteObject();

	COLORREF clrTabBack;
	COLORREF clrFloatToolBarBorder;

	if (afxGlobalData.m_nBitsPerPixel > 8 && !afxGlobalData.IsHighContrastMode())
	{
		COLORREF clrWindow = GetWindowColor();
		COLORREF clrFace = afxGlobalData.clrBarFace;

		m_clrMenuLight = RGB( (219 * GetRValue(clrWindow) + 36 * GetRValue(clrFace)) / 255,
			(219 * GetGValue(clrWindow) + 36 * GetGValue(clrFace)) / 255, (219 * GetBValue(clrWindow) + 36 * GetBValue(clrFace)) / 255);

		double H, S, L;
		CDrawingManager::RGBtoHSL(clrFace, &H, &S, &L);

		double S1;
		double L1;

		if (S < 0.1)
		{
			L1 = min(1., L +(1. - L) * .5);
			S1 = S == 0 ? 0 : min(1., S + .1);
		}
		else
		{
			L1 = min(1., 0.5 * L + 0.5);
			S1 = min(1., S * 2);
		}

		clrTabBack = CDrawingManager::HLStoRGB_ONE(H, L1, S1);

		m_clrBarBkgnd = RGB((40 * GetRValue(clrWindow) + 215 * GetRValue(clrFace)) / 255,
			(40 * GetGValue(clrWindow) + 215 * GetGValue(clrFace)) / 255, (40 * GetBValue(clrWindow) + 215 * GetBValue(clrFace)) / 255);

		m_clrMenuRarelyUsed = CDrawingManager::PixelAlpha(
			m_clrBarBkgnd, 94);

		m_clrInactiveTabText = CDrawingManager::PixelAlpha(clrFace, 55);

		COLORREF clrHL = afxGlobalData.clrHilite;
		CDrawingManager::RGBtoHSL(clrHL, &H, &S, &L);

		COLORREF clrMix = RGB((77 * GetRValue(clrHL) + 178 * GetRValue(m_clrMenuLight)) / 255,
			(77 * GetGValue(clrHL) + 178 * GetGValue(m_clrMenuLight)) / 255, (77 * GetBValue(clrHL) + 178 * GetBValue(m_clrMenuLight)) / 255);

		if (L > .8) // The highlight color is very light
		{
			m_clrHighlight = CDrawingManager::PixelAlpha(clrMix, 91);
			m_clrHighlightDn = CDrawingManager::PixelAlpha(clrMix, 98);
			m_clrMenuItemBorder = CDrawingManager::PixelAlpha(afxGlobalData.clrHilite, 84);
		}
		else
		{
			m_clrHighlight = CDrawingManager::PixelAlpha(clrMix, 102);
			m_clrHighlightDn = CDrawingManager::PixelAlpha(m_clrHighlight, 87);
			m_clrMenuItemBorder = afxGlobalData.clrHilite;
		}

		m_clrHighlightChecked = CDrawingManager::PixelAlpha(RGB((GetRValue(clrHL) + 5 * GetRValue(m_clrMenuLight)) / 6,
			(GetGValue(clrHL) + 5 * GetGValue(m_clrMenuLight)) / 6, (GetBValue(clrHL) + 5 * GetBValue(m_clrMenuLight)) / 6), 100);

		m_clrSeparator = CDrawingManager::PixelAlpha(afxGlobalData.clrBarFace, .86, .86, .86);

		m_clrPaneBorder = afxGlobalData.clrBarShadow;

		m_clrMenuBorder = CDrawingManager::PixelAlpha(clrFace, 55);

		clrFloatToolBarBorder = CDrawingManager::PixelAlpha(afxGlobalData.clrBarShadow, .85, .85, .85);

		m_clrGripper = CDrawingManager::PixelAlpha(afxGlobalData.clrBarShadow, 110);
	}
	else
	{
		m_clrMenuLight = afxGlobalData.clrWindow;

		m_clrBarBkgnd = afxGlobalData.clrBtnFace;

		if (afxGlobalData.m_bIsBlackHighContrast)
		{
			m_clrHighlightChecked = m_clrHighlightDn = m_clrHighlight = afxGlobalData.clrHilite;
			m_clrMenuRarelyUsed = afxGlobalData.clrBtnFace;
		}
		else
		{
			m_clrHighlightDn = m_clrHighlight = afxGlobalData.clrBtnFace;
			m_clrHighlightChecked = afxGlobalData.clrWindow;
			m_clrMenuRarelyUsed = afxGlobalData.clrBarLight;
		}

		clrTabBack = afxGlobalData.clrBtnFace;
		m_clrInactiveTabText = afxGlobalData.clrBtnDkShadow;
		m_clrSeparator = afxGlobalData.clrBtnShadow;
		m_clrGripper = afxGlobalData.clrBtnShadow;
		m_clrPaneBorder = afxGlobalData.clrBtnShadow;
		m_clrMenuBorder = afxGlobalData.clrBtnDkShadow;
		clrFloatToolBarBorder = afxGlobalData.clrBtnShadow;

		m_clrMenuItemBorder = afxGlobalData.IsHighContrastMode() ? afxGlobalData.clrBtnDkShadow : afxGlobalData.clrHilite;
	}

	m_brBarBkgnd.CreateSolidBrush(m_clrBarBkgnd);
	m_brMenuRarelyUsed.CreateSolidBrush(m_clrMenuRarelyUsed);
	m_brMenuLight.CreateSolidBrush(m_clrMenuLight);

	m_brHighlight.CreateSolidBrush(m_clrHighlight);
	m_brHighlightDn.CreateSolidBrush(m_clrHighlightDn);
	m_brHighlightChecked.CreateSolidBrush(m_clrHighlightChecked);
	m_brTabBack.CreateSolidBrush(clrTabBack);
	m_penSeparator.CreatePen(PS_SOLID, 1, m_clrSeparator);

	m_brFloatToolBarBorder.CreateSolidBrush(clrFloatToolBarBorder);

	m_clrPressedButtonBorder = (COLORREF)-1; // Used in derived classes

	m_penMenuItemBorder.DeleteObject();
	m_penMenuItemBorder.CreatePen(PS_SOLID, 1, m_clrMenuItemBorder);
}

void CMFCVisualManagerOfficeXP::OnDrawBarGripper(CDC* pDC, CRect rectGripper, BOOL bHorz, CBasePane* pBar)
{
	ASSERT_VALID(pDC);

	if (pBar != NULL && pBar->IsDialogControl())
	{
		CMFCVisualManager::OnDrawBarGripper(pDC, rectGripper, bHorz, pBar);
		return;
	}

	if (m_brGripperHorz.GetSafeHandle() == NULL)
	{
		CreateGripperBrush();
	}

	BOOL bSideBar = pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CDockablePane));
	BOOL bMenuBar = pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CMFCMenuBar));

	CRect rectFill = rectGripper;

	if (!bSideBar)
	{
		if (bHorz)
		{
			int xCenter = rectFill.CenterPoint().x;
			rectFill.left = xCenter - 1;
			rectFill.right = xCenter + 2;
			rectFill.DeflateRect(0, 5);
		}
		else
		{
			int yCenter = rectFill.CenterPoint().y;
			rectFill.top = yCenter - 1;
			rectFill.bottom = yCenter + 2;
			rectFill.DeflateRect(5, 0);
		}
	}
	else
	{
		if (bHorz)
		{
			rectFill.DeflateRect(4, 0);
		}
		else
		{
			rectFill.DeflateRect(4, 0);
		}

		bHorz = !bHorz;
	}

	COLORREF clrTextOld = pDC->SetTextColor(m_clrGripper);
	COLORREF clrBkOld = pDC->SetBkColor(bSideBar || bMenuBar ? afxGlobalData.clrBarFace : m_clrBarBkgnd);

	pDC->FillRect(rectFill, bHorz ? &m_brGripperHorz : &m_brGripperVert);

	if (bSideBar)
	{
		//------------------
		// Draw bar caption:
		//------------------
		int nOldBkMode = pDC->SetBkMode(OPAQUE);
		pDC->SetTextColor(afxGlobalData.clrBarText);

		const CFont& font = CMFCMenuBar::GetMenuFont(bHorz);

		CFont* pOldFont = pDC->SelectObject((CFont*) &font);

		CString strCaption;
		pBar->GetWindowText(strCaption);
		strCaption = _T(" ") + strCaption + _T(" ");

		CRect rectText = rectGripper;
		UINT uiTextFormat = 0;

		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);

		CPoint ptTextOffset(0, 0);
		if (bHorz)
		{
			ptTextOffset.y = (rectGripper.Height() - tm.tmHeight - 1) / 2;
		}
		else
		{
			ptTextOffset.x = (rectGripper.Width() - tm.tmHeight + 1) / 2;
		}

		if (bHorz)
		{
			rectText.top += ptTextOffset.y;
			pDC->DrawText(strCaption, &rectText, uiTextFormat);
		}
		else
		{
			rectText.left = rectText.right - ptTextOffset.x;
			rectText.top = rectGripper.top + ptTextOffset.y;
			rectText.bottom = rectGripper.top + 3 * ptTextOffset.y;

			uiTextFormat |= DT_NOCLIP;

			pDC->DrawText(strCaption, &rectText, uiTextFormat);
		}

		pDC->SelectObject(pOldFont);
		pDC->SetBkMode(nOldBkMode);
	}

	pDC->SetTextColor(clrTextOld);
	pDC->SetBkColor(clrBkOld);
}

void CMFCVisualManagerOfficeXP::OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* pMenu, CRect rect)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pMenu);

	const BOOL bRTL = pMenu->GetExStyle() & WS_EX_LAYOUTRTL;

	pDC->Draw3dRect(rect, m_clrMenuBorder, m_clrMenuBorder);
	rect.DeflateRect(1, 1);
	pDC->Draw3dRect(rect, m_clrMenuLight, m_clrMenuLight);

	CRect rectLeft(1, 1, 2, rect.bottom - 1);
	pDC->FillRect(rectLeft, &m_brBarBkgnd);

	//------------------------------------------------
	// Quick Customize Office XP like draw popup menu
	//------------------------------------------------
	CMFCPopupMenu* pParentPopup = pMenu->GetParentPopupMenu();
	if (pParentPopup != NULL)
	{
		if (pParentPopup->IsQuickCustomize() && !bRTL)
		{
			CMFCToolBarMenuButton* pParentBtn = pMenu->GetParentButton();
			if ((pParentBtn != NULL) &&(pParentBtn->IsQuickMode()))
			{
				CMFCPopupMenu* pParent = (CMFCPopupMenu*)pMenu->GetParentPopupMenu();

				CRect rcParent;
				pParent->GetWindowRect(rcParent);

				CRect rcCurrent;
				pMenu->GetWindowRect(rcCurrent);

				CMFCToolBarMenuButton* pBtn = pMenu->GetMenuItem(0);
				CRect rcButton = pBtn->Rect();

				CRect rectBorder;
				rectBorder.SetRectEmpty();

				if (rcParent.left > rcCurrent.left)
				{
					if (rcParent.top <= rcCurrent.top)
					{
						rectBorder.SetRect(rect.right - 1, rect.top, rect.right + 1, rcButton.bottom);
					}
					else
					{
						// up
						rectBorder.SetRect(rect.right - 1, rect.bottom - rcButton.Height(), rect.right + 1, rect.bottom);
					}
				}
				else
				{
					if (rcParent.top <= rcCurrent.top)
					{
						rectBorder.SetRect(rect.left - 1, rect.top, rect.left + 1, rcButton.bottom);
					}
					else
					{
						// up
						rectBorder.SetRect(rect.left - 1, rect.bottom - rcButton.Height(), rect.left + 1, rect.bottom);
					}
				}

				if (!rectBorder.IsRectEmpty())
				{
					pDC->FillRect(rectBorder, &m_brBarBkgnd);
				}
			}
		}
	}

	if (!CMFCToolBar::IsCustomizeMode())
	{
		//-------------------------------------
		// "Connect" menu to the parent button:
		//-------------------------------------
		CMFCToolBarMenuButton* pParentMenuBtn = pMenu->GetParentButton();
		if (m_bConnectMenuToParent && pParentMenuBtn != NULL && pMenu->GetParentPopupMenu() == NULL && pParentMenuBtn->IsBorder())
		{
			CRect rectConnect;
			rectConnect.SetRectEmpty();

			CRect rectParent = pParentMenuBtn->Rect();
			CWnd* pWnd = pParentMenuBtn->GetParentWnd();
			pWnd->ClientToScreen(rectParent);
			pMenu->ScreenToClient(&rectParent);

			switch(pMenu->GetDropDirection())
			{
			case CMFCPopupMenu::DROP_DIRECTION_BOTTOM:
				rectConnect = CRect(rectParent.left + 1, rect.top - 1, rectParent.right - 1, rect.top);

				if (rectConnect.Width() > rect.Width() + 2)
				{
					return;
				}

				break;

			case CMFCPopupMenu::DROP_DIRECTION_TOP:
				rectConnect = CRect(rectParent.left + 1, rect.bottom, rectParent.right - 1, rect.bottom + 1);

				if (rectConnect.Width() > rect.Width() + 2)
				{
					return;
				}

				break;

			case CMFCPopupMenu::DROP_DIRECTION_RIGHT:
				rectConnect = CRect(rect.left - 1, rectParent.top + 1, rect.left, rectParent.bottom - 1);

				if (rectConnect.Height() > rect.Height() + 2)
				{
					return;
				}

				break;

			case CMFCPopupMenu::DROP_DIRECTION_LEFT:
				rectConnect = CRect(rect.right, rectParent.top + 1, rect.right + 1, rectParent.bottom - 1);

				if (rectConnect.Height() > rect.Height() + 2)
				{
					return;
				}

				break;
			}

			CRect rectBorder = rect;
			rectBorder.InflateRect(1, 1);
			rectConnect.IntersectRect(&rectConnect, &rectBorder);
			rectParent.InflateRect(1, 1);
			rectConnect.IntersectRect(&rectConnect, &rectParent);

			pDC->FillRect(rectConnect, &m_brBarBkgnd);
		}
	}
}

void CMFCVisualManagerOfficeXP::OnDrawMenuShadow(CDC* pPaintDC, const CRect& rectClient, const CRect& rectExclude,
	int nDepth, int iMinBrightness, int iMaxBrightness, CBitmap* pBmpSaveBottom,  CBitmap* pBmpSaveRight, BOOL bRTL)
{
	ASSERT_VALID(pPaintDC);
	ASSERT_VALID(pBmpSaveBottom);
	ASSERT_VALID(pBmpSaveRight);

	if (rectExclude.IsRectNull())
	{
		//------------------------
		// Simple draw the shadow:
		//------------------------
		CDrawingManager dm(*pPaintDC);
		dm.DrawShadow(rectClient, nDepth, iMinBrightness, iMaxBrightness, pBmpSaveBottom, pBmpSaveRight, m_clrMenuShadowBase, !bRTL);
	}
	else
	{
		//--------------------------------------------
		// Copy screen content into the memory bitmap:
		//--------------------------------------------
		CDC dcMem;
		if (!dcMem.CreateCompatibleDC(pPaintDC))
		{
			ASSERT(FALSE);
			return;
		}

		//--------------------------------------------
		// Gets the whole menu and changes the shadow.
		//--------------------------------------------
		CRect rectBmp(0, 0, rectClient.Width(), rectClient.Height());
		int cx = rectBmp.Width() + nDepth;
		int cy = rectBmp.Height() + nDepth;
		CBitmap bmpMem;
		if (!bmpMem.CreateCompatibleBitmap(pPaintDC, cx, cy))
		{
			ASSERT(FALSE);
			return;
		}

		CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
		ENSURE(pOldBmp != NULL);

		dcMem.BitBlt(0, 0, cx, cy, pPaintDC, rectClient.left, rectClient.top, SRCCOPY);

		//-----------------
		// Draw the shadow:
		//-----------------
		CDrawingManager dm(dcMem);
		dm.DrawShadow(rectBmp, nDepth, iMinBrightness, iMaxBrightness, pBmpSaveBottom, pBmpSaveRight, m_clrMenuShadowBase, !bRTL);

		//------------------------------------------
		// Do not cover rectExclude with the shadow:
		//------------------------------------------
		dcMem.BitBlt(rectExclude.left - rectClient.left, rectExclude.top - rectClient.top, rectExclude.Width(), rectExclude.Height(),
			pPaintDC, rectExclude.left, rectExclude.top, SRCCOPY);

		//-----------------------------------------
		// Copy shadowed bitmap back to the screen:
		//-----------------------------------------
		pPaintDC->BitBlt(rectClient.left, rectClient.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawPaneBorder(CDC* pDC, CBasePane* pBar, CRect& rect)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if (pBar->IsDialogControl())
	{
		CMFCVisualManager::OnDrawPaneBorder(pDC, pBar, rect);
		return;
	}

	DWORD dwBarStyle = pBar->GetPaneStyle();
	if (!(dwBarStyle & CBRS_BORDER_ANY))
	{
		return;
	}

	COLORREF clrBckOld = pDC->GetBkColor(); // FillSolidRect changes it

	if (dwBarStyle & CBRS_BORDER_LEFT)
		pDC->FillSolidRect(0, 0, 1, rect.Height() - 1, afxGlobalData.clrBarFace);
	if (dwBarStyle & CBRS_BORDER_TOP)
		pDC->FillSolidRect(0, 0, rect.Width()-1 , 1, afxGlobalData.clrBarFace);
	if (dwBarStyle & CBRS_BORDER_RIGHT)
		pDC->FillSolidRect(rect.right, 0/*RGL~:1*/, -1,
		rect.Height()/*RGL-: - 1*/, afxGlobalData.clrBarFace);
	if (dwBarStyle & CBRS_BORDER_BOTTOM)
		pDC->FillSolidRect(0, rect.bottom, rect.Width()-1, -1, afxGlobalData.clrBarFace);

	if (dwBarStyle & CBRS_BORDER_LEFT)
		++rect.left;
	if (dwBarStyle & CBRS_BORDER_TOP)
		++rect.top;
	if (dwBarStyle & CBRS_BORDER_RIGHT)
		--rect.right;
	if (dwBarStyle & CBRS_BORDER_BOTTOM)
		--rect.bottom;

	// Restore Bk color:
	pDC->SetBkColor(clrBckOld);
}

void CMFCVisualManagerOfficeXP::OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL /*bNCArea*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBar);

	if (DYNAMIC_DOWNCAST(CReBar, pBar) != NULL || DYNAMIC_DOWNCAST(CReBar, pBar->GetParent()))
	{
		FillReBarPane(pDC, pBar, rectClient);
		return;
	}

	if (rectClip.IsRectEmpty())
	{
		rectClip = rectClient;
	}

	CRuntimeClass* pBarClass = pBar->GetRuntimeClass();

	if (pBarClass == NULL || pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCMenuBar)))
	{
		CMFCVisualManager::OnFillBarBackground(pDC, pBar, rectClient, rectClip);
		return;
	}

	if (pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCOutlookBarPane)))
	{
		CMFCOutlookBarPane* pOlBar = DYNAMIC_DOWNCAST(CMFCOutlookBarPane, pBar);
		ASSERT_VALID(pOlBar);

		if (pOlBar->IsBackgroundTexture())
		{
			CMFCVisualManager::OnFillBarBackground(pDC, pBar, rectClient, rectClip);
			return;
		}
	}

	if (pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCColorBar)))
	{
		if (pBar->IsDialogControl())
		{
			CMFCVisualManager::OnFillBarBackground(pDC, pBar, rectClient, rectClip);
		}
		else
		{
			pDC->FillRect(rectClip, ((CMFCColorBar*) pBar)->IsTearOff() ? &m_brBarBkgnd : &m_brMenuLight);
		}

		return;
	}

	if (pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCPopupMenuBar)))
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

			if (bQuickMode)
			{
				rectImages.right = rectImages.left + 2*CMFCToolBar::GetMenuImageSize().cx + 4 * GetMenuImageMargin() + 4;

			}
			else
			{
				rectImages.right = rectImages.left + CMFCToolBar::GetMenuImageSize().cx + 2 * GetMenuImageMargin() + 2;
			}

			rectImages.DeflateRect(0, 1);
			pDC->FillRect(rectImages, &m_brBarBkgnd);
		}

		return;
	}

	if (pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCToolBar)))
	{
		if (pBar->IsDialogControl())
		{
			CMFCVisualManager::OnFillBarBackground(pDC, pBar, rectClient, rectClip);
		}
		else
		{
			pDC->FillRect(rectClip, &m_brBarBkgnd);
		}

		return;
	}

	if (pBarClass->IsDerivedFrom(RUNTIME_CLASS(CAutoHideDockSite)))
	{
		pDC->FillRect(rectClip, &m_brTabBack);
		return;
	}

	CMFCVisualManager::OnFillBarBackground(pDC, pBar, rectClient, rectClip);
}

void CMFCVisualManagerOfficeXP::OnDrawSeparator(CDC* pDC, CBasePane* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBar);

	if (pBar->IsDialogControl())
	{
		CMFCVisualManager::OnDrawSeparator(pDC, pBar, rect, bHorz);
		return;
	}

	CRect rectSeparator = rect;

	CPen* pOldPen = pDC->SelectObject(&m_penSeparator);
	ENSURE(pOldPen != NULL);

	int x1, x2;
	int y1, y2;

	if (bHorz)
	{
		x1 = x2 = (rect.left + rect.right) / 2;
		y1 = rect.top;
		y2 = rect.bottom - 1;
	}
	else
	{
		y1 = y2 = (rect.top + rect.bottom) / 2;
		x1 = rect.left;
		x2 = rect.right;

		BOOL bIsRibbon = FALSE;

		bIsRibbon = pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonPanelMenuBar));

		if (bIsRibbon &&((CMFCRibbonPanelMenuBar*) pBar)->IsDefaultMenuLook())
		{
			bIsRibbon = FALSE;
		}

		if (pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)) && !bIsRibbon && !pBar->IsKindOf(RUNTIME_CLASS(CMFCColorBar)))
		{

			x1 = rect.left + CMFCToolBar::GetMenuImageSize().cx + GetMenuImageMargin() + 1; 
			CRect rectBar;
			pBar->GetClientRect(rectBar);

			if (rectBar.right - x2 < 50) // Last item in row
			{
				x2 = rectBar.right;
			}

			if (((CMFCPopupMenuBar*) pBar)->m_bDisableSideBarInXPMode)
			{
				x1 = 0;
			}

			//---------------------------------
			// Maybe Quick Customize separator
			//---------------------------------
			if (pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
			{
				CWnd* pWnd = pBar->GetParent();
				if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CMFCPopupMenu)))
				{
					CMFCPopupMenu* pMenu = (CMFCPopupMenu*)pWnd;
					if (pMenu->IsCustomizePane())
					{
						x1 = rect.left + 2*CMFCToolBar::GetMenuImageSize().cx + 3*GetMenuImageMargin() + 2;
					}
				}
			}
		}
	}

	pDC->MoveTo(x1, y1);
	pDC->LineTo(x2, y2);

	pDC->SelectObject(pOldPen);
}

void CMFCVisualManagerOfficeXP::OnDrawButtonBorder(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (state != ButtonsIsPressed && state != ButtonsIsHighlighted)
	{
		ASSERT((pButton->m_nStyle & TBBS_CHECKED) == 0);
		return;
	}

	COLORREF clrBorder = m_clrMenuItemBorder;

	CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
	BOOL bIsMenuButton = pMenuButton != NULL;

	BOOL bIsPopupMenu = bIsMenuButton && pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));

	BOOL bIsPressedBorder = !bIsPopupMenu;

	if (bIsMenuButton && !bIsPopupMenu && pMenuButton->IsDroppedDown())
	{
		bIsPressedBorder = FALSE;

		CMFCPopupMenu* pPopupMenu= pMenuButton->GetPopupMenu();
		if (pPopupMenu != NULL && (pPopupMenu->IsWindowVisible() || pPopupMenu->IsShown()))
		{
			clrBorder = m_clrMenuBorder;
			ExtendMenuButton(pMenuButton, rect);

			BOOL bRTL = pPopupMenu->GetExStyle() & WS_EX_LAYOUTRTL;

			if (m_bShdowDroppedDownMenuButton && !bRTL && CMFCMenuBar::IsMenuShadows() && !CMFCToolBar::IsCustomizeMode() &&
				afxGlobalData.m_nBitsPerPixel > 8 && !afxGlobalData.IsHighContrastMode() && !pPopupMenu->IsRightAlign())
			{
				CDrawingManager dm(*pDC);

				dm.DrawShadow(rect, m_nMenuShadowDepth, 100, 75, NULL, NULL, m_clrMenuShadowBase);
			}
		}
	}

	const BOOL bIsChecked = (pButton->m_nStyle & TBBS_CHECKED);

	switch(state)
	{
	case ButtonsIsPressed:
		if (bIsPressedBorder && m_clrPressedButtonBorder != (COLORREF)-1 && !bIsChecked && rect.Width() > 5 && rect.Height() > 5)
		{
			clrBorder = m_clrPressedButtonBorder;
		}

	case ButtonsIsHighlighted:
		if (bIsPopupMenu && bIsChecked)
		{
			if (pButton->m_nStyle & TBBS_MARKED)
			{
				clrBorder = m_clrPressedButtonBorder;
			}

			rect.bottom ++;
		}

		pDC->Draw3dRect(rect, clrBorder, clrBorder);
	}
}

void CMFCVisualManagerOfficeXP::OnFillButtonInterior(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (state != ButtonsIsPressed && state != ButtonsIsHighlighted)
	{
		return;
	}

	if (CMFCToolBar::IsCustomizeMode() && !CMFCToolBar::IsAltCustomizeMode() && !pButton->IsLocked())
	{
		return;
	}

	CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
	BOOL bIsMenuButton = pMenuButton != NULL;
	BOOL bIsPopupMenu = bIsMenuButton && pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));
	if (!bIsPopupMenu && !m_bEnableToolbarButtonFill)
	{
		return;
	}

	CBrush* pBrush = ((pButton->m_nStyle & TBBS_PRESSED) && !bIsPopupMenu) ? &m_brHighlightDn : &m_brHighlight;

	if (bIsMenuButton && !bIsPopupMenu && pMenuButton->IsDroppedDown())
	{
		ExtendMenuButton(pMenuButton, rect);
		pBrush = &m_brBarBkgnd;
	}

	if (pButton->m_nStyle & TBBS_CHECKED)
	{
		pBrush = (state == ButtonsIsHighlighted) ? &m_brHighlightDn : &m_brHighlightChecked;
	}

	if (bIsMenuButton &&(pButton->m_nStyle & TBBS_DISABLED))
	{
		pBrush = &m_brMenuLight;
	}

	switch(state)
	{
	case ButtonsIsPressed:
	case ButtonsIsHighlighted:
		if ((pButton->m_nStyle & TBBS_CHECKED) == 0)
		{
			rect.DeflateRect(1, 1);
		}

		OnFillHighlightedArea(pDC, rect, pBrush, pButton);
	}
}

void CMFCVisualManagerOfficeXP::OnHighlightMenuItem(CDC* pDC, CMFCToolBarMenuButton* pButton, CRect rect, COLORREF& clrText)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	CBrush* pBrush = (pButton->m_nStyle & TBBS_DISABLED) ? &m_brMenuLight : &m_brHighlight;

	rect.DeflateRect(1, 0);

	OnFillHighlightedArea(pDC, rect, pBrush, pButton);
	pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);

	clrText = GetHighlightedMenuItemTextColor(pButton);
}

COLORREF CMFCVisualManagerOfficeXP::GetHighlightedMenuItemTextColor(CMFCToolBarMenuButton* pButton)
{
	ASSERT_VALID(pButton);

	if (pButton->m_nStyle & TBBS_DISABLED)
	{
		return afxGlobalData.clrGrayedText;
	}

	if (GetRValue(m_clrHighlight) > 128 && GetGValue(m_clrHighlight) > 128 && GetBValue(m_clrHighlight) > 128)
	{
		return RGB(0, 0, 0);
	}
	else
	{
		return RGB(255, 255, 255);
	}
}

void CMFCVisualManagerOfficeXP::OnHighlightQuickCustomizeMenuButton(CDC* pDC, CMFCToolBarMenuButton* /*pButton*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &m_brBarBkgnd);
	pDC->Draw3dRect(rect, m_clrMenuBorder, m_clrMenuBorder);
}

void CMFCVisualManagerOfficeXP::OnHighlightRarelyUsedMenuItems(CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID(pDC);

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CMFCToolBar::GetMenuImageSize().cx + 2 * GetMenuImageMargin() + 2;

	pDC->FillRect(rectRarelyUsed, &m_brMenuRarelyUsed);
}

void CMFCVisualManagerOfficeXP::OnDrawTab(CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
#define AFX_TEXT_MARGIN 4
#define AFX_IMAGE_MARGIN 4

	ASSERT_VALID(pTabWnd);
	ASSERT_VALID(pDC);

	if (pTabWnd->IsFlatTab() || pTabWnd->IsOneNoteStyle() || pTabWnd->IsColored() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded())
	{
		CMFCVisualManager::OnDrawTab(pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	COLORREF clrDark;
	COLORREF clrBlack;
	COLORREF clrHighlight;
	COLORREF clrFace;
	COLORREF clrDarkShadow;
	COLORREF clrLight;
	CBrush* pbrFace = NULL;
	CBrush* pbrBlack = NULL;

	GetTabFrameColors(pTabWnd, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack);

	CPen penGray(PS_SOLID, 1, clrDark);
	CPen penDkGray(PS_SOLID, 1, clrBlack);
	CPen penHiLight(PS_SOLID, 1, clrHighlight);

	CPen* pOldPen = pDC->SelectObject(&penGray);
	ENSURE(pOldPen != NULL);

	if (iTab != pTabWnd->GetActiveTab() - 1)
	{
		if (iTab < pTabWnd->GetVisibleTabsNum() - 1 || m_bDrawLastTabLine)
		{
			pDC->MoveTo(rectTab.right, rectTab.top + 3);
			pDC->LineTo(rectTab.right, rectTab.bottom - 3);
		}
	}

	if (bIsActive)
	{
		if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
		{
			CRect rectFace = rectTab;
			rectFace.top--;

			OnFillTab(pDC, rectFace, pbrFace, iTab, bIsActive, pTabWnd);

			pDC->SelectObject(&penDkGray);

			pDC->MoveTo(rectTab.right, rectTab.top);
			pDC->LineTo(rectTab.right, rectTab.bottom);
			pDC->LineTo(rectTab.left, rectTab.bottom);

			pDC->SelectObject(&penHiLight);
			pDC->LineTo(rectTab.left, rectTab.top - 2);
		}
		else
		{
			CPen penLight(PS_SOLID, 1, m_clrMenuLight);

			CRect rectFace = rectTab;
			rectFace.bottom++;
			rectFace.left++;

			OnFillTab(pDC, rectFace, pbrFace, iTab, bIsActive, pTabWnd);

			pDC->SelectObject(&penDkGray);
			pDC->MoveTo(rectTab.right, rectTab.bottom);
			pDC->LineTo(rectTab.right, rectTab.top);

			pDC->SelectObject(&penHiLight);

			pDC->LineTo(rectTab.right, rectTab.top);
			pDC->LineTo(rectTab.left, rectTab.top);
			pDC->LineTo(rectTab.left, rectTab.bottom);
		}
	}

	pDC->SelectObject(pOldPen);

	COLORREF clrText;

	if (pTabWnd->IsDialogControl())
	{
		clrText = afxGlobalData.clrBtnText;
	}
	else
	{
		clrText = bIsActive ? afxGlobalData.clrBarText : m_clrInactiveTabText;
	}

	OnDrawTabContent(pDC, rectTab, iTab, bIsActive, pTabWnd, clrText);
}

void CMFCVisualManagerOfficeXP::OnFillTab(CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pbrFill);
	ASSERT_VALID(pTabWnd);

	if (pTabWnd->GetTabBkColor(iTab) != (COLORREF)-1 && !bIsActive)
	{
		CBrush br(pTabWnd->GetTabBkColor(iTab));
		pDC->FillRect(rectFill, &br);
		return;
	}

	if (pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded())
	{
		CMFCVisualManager::OnFillTab(pDC, rectFill, pbrFill, iTab, bIsActive, pTabWnd);
	}
	else if (bIsActive)
	{
		pDC->FillRect(rectFill, pbrFill);
	}
}

void CMFCVisualManagerOfficeXP::OnEraseTabsArea(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTabWnd);

	if (pTabWnd->IsFlatTab())
	{
		CMFCVisualManager::OnEraseTabsArea(pDC, rect, pTabWnd);
		return;
	}

	if (pTabWnd->IsDialogControl())
	{
		pDC->FillRect(rect, &afxGlobalData.brBtnFace);
		return;
	}

	pDC->FillRect(rect, &m_brTabBack);
}

COLORREF CMFCVisualManagerOfficeXP::OnDrawPaneCaption(CDC* pDC, CDockablePane* /*pBar*/, BOOL bActive, CRect rectCaption, CRect /*rectButtons*/)
{
	ASSERT_VALID(pDC);

	CPen pen(PS_SOLID, 1, bActive ? afxGlobalData.clrBarLight : afxGlobalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject(&pen);

	CBrush* pOldBrush = (CBrush*) pDC->SelectObject(bActive ? &afxGlobalData.brActiveCaption : &afxGlobalData.brBarFace);

	if (bActive)
	{
		rectCaption.InflateRect(1, 1);
	}

	pDC->RoundRect(rectCaption, CPoint(2, 2));

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	// get the text color
	COLORREF clrCptnText = bActive ? afxGlobalData.clrCaptionText : afxGlobalData.clrBarText;

	return clrCptnText;
}

void CMFCVisualManagerOfficeXP::OnDrawCaptionButton(CDC* pDC, CMFCCaptionButton* pButton, BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled, int nImageID /*= -1*/)
{
	ASSERT_VALID(pDC);
	ENSURE(pButton != NULL);

	CRect rc = pButton->GetRect();

	if (pButton->m_bPushed &&(pButton->m_bFocused || pButton->m_bDroppedDown) && !bDisabled)
	{
		OnFillHighlightedArea(pDC, rc, &m_brHighlightDn, NULL);
		bActive = TRUE;
	}
	else if (pButton->m_bPushed || pButton->m_bFocused || pButton->m_bDroppedDown)
	{
		if (!bDisabled)
		{
			OnFillHighlightedArea(pDC, rc, &m_brHighlight, NULL);
		}

		bActive = FALSE;
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

	if ((pButton->m_bPushed || pButton->m_bFocused || pButton->m_bDroppedDown) && !bDisabled)
	{
		COLORREF clrDark = afxGlobalData.clrBarDkShadow;
		pDC->Draw3dRect(rc, clrDark, clrDark);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawCaptionButtonIcon(CDC* pDC, CMFCCaptionButton* pButton, CMenuImages::IMAGES_IDS id, BOOL bActive, BOOL bDisabled, CPoint ptImage)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	CMenuImages::IMAGE_STATE imageState;

	if (bDisabled)
	{
		imageState = CMenuImages::ImageGray;
	}
	else if (pButton->m_bFocused || pButton->m_bPushed)
	{
		COLORREF clrBack = pButton->m_bPushed ? m_clrHighlightDn : m_clrHighlight;

		if (GetRValue(clrBack) <= 192 && GetGValue(clrBack) <= 192 && GetBValue(clrBack) <= 192)
		{
			imageState = CMenuImages::ImageWhite;
		}
		else
		{
			imageState = CMenuImages::ImageBlack;
		}
	}
	else if (pButton->m_clrForeground == (COLORREF)-1)
	{
		imageState = bActive ? CMenuImages::ImageWhite : CMenuImages::ImageBlack;
	}
	else
	{
		if (GetRValue(pButton->m_clrForeground) > 192 && GetGValue(pButton->m_clrForeground) > 192 && GetBValue(pButton->m_clrForeground) > 192)
		{
			imageState = CMenuImages::ImageWhite;
		}
		else
		{
			imageState = CMenuImages::ImageBlack;
		}
	}

	CMenuImages::Draw(pDC, id, ptImage, imageState);
}

COLORREF CMFCVisualManagerOfficeXP::OnFillCommandsListBackground(CDC* pDC, CRect rect, BOOL bIsSelected)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	COLORREF clrText = afxGlobalData.clrBarText;

	int nImageWidth = CMFCToolBar::GetMenuImageSize().cx + GetMenuImageMargin();

	if (bIsSelected)
	{
		if (m_bEnableToolbarButtonFill)
		{
			rect.left = 0;
		}

		OnFillHighlightedArea(pDC, rect, &m_brHighlight, NULL);

		pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);

		// Now, we should define a menu text color...
		if (GetRValue(m_clrHighlight) > 128 && GetGValue(m_clrHighlight) > 128 && GetBValue(m_clrHighlight) > 128)
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

		pDC->FillRect(rectImages, &m_brBarBkgnd);

		clrText = afxGlobalData.clrBarText;
	}

	return clrText;
}

void CMFCVisualManagerOfficeXP::OnDrawMenuArrowOnCustomizeList(CDC* pDC, CRect rectCommand, BOOL /*bSelected*/)
{
	ASSERT_VALID(pDC);

	CRect rectTriangle = rectCommand;
	rectTriangle.left = rectTriangle.right - CMenuImages::Size().cx;

	CMenuImages::IMAGE_STATE state = CMenuImages::ImageBlack;
	COLORREF clrPen = RGB(0, 0, 0);

	if (GetRValue(m_clrHighlight) < 128 || GetGValue(m_clrHighlight) < 128 || GetBValue(m_clrHighlight) < 128)
	{
		state = CMenuImages::ImageWhite;
		clrPen = RGB(255, 255, 255);
	}

	CMenuImages::Draw(pDC, CMenuImages::IdArrowRightLarge, rectTriangle, state);

	CPen penLine(PS_SOLID, 1, clrPen);
	CPen* pOldPen = pDC->SelectObject(&penLine);
	ENSURE(pOldPen != NULL);

	pDC->MoveTo(rectTriangle.left - 1, rectCommand.top + 2);
	pDC->LineTo(rectTriangle.left - 1, rectCommand.bottom - 2);

	pDC->SelectObject(pOldPen);
}

void CMFCVisualManagerOfficeXP::OnDrawTearOffCaption(CDC* pDC, CRect rect, BOOL bIsActive)
{
	const int nBorderSize = 1;
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &m_brMenuLight);

	rect.DeflateRect(nBorderSize, nBorderSize);
	OnFillHighlightedArea(pDC, rect, bIsActive ? &m_brHighlight : &m_brBarBkgnd, NULL);

	// Draw gripper:
	int nGripperWidth = max(20, CMFCToolBar::GetMenuImageSize().cx * 2);

	CRect rectGripper = rect;
	rectGripper.DeflateRect((rectGripper.Width() - nGripperWidth) / 2, 1);

	if (m_brGripperHorz.GetSafeHandle() == NULL)
	{
		CreateGripperBrush();
	}

	COLORREF clrTextOld = pDC->SetTextColor(bIsActive ? afxGlobalData.clrBarDkShadow : afxGlobalData.clrBarShadow);
	COLORREF clrBkOld = pDC->SetBkColor(bIsActive ? m_clrHighlight : m_clrBarBkgnd);

	if (bIsActive)
	{
		rectGripper.DeflateRect(0, 1);
	}

	pDC->FillRect(rectGripper, &m_brGripperHorz);

	pDC->SetTextColor(clrTextOld);
	pDC->SetBkColor(clrBkOld);

	if (bIsActive)
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarDkShadow);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawMenuResizeBar(CDC* pDC, CRect rect, int nResizeFlags)
{
	ASSERT_VALID(pDC);

	const int nBorderSize = 1;
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &m_brMenuLight);
	rect.DeflateRect(nBorderSize, nBorderSize);
	OnFillHighlightedArea(pDC, rect, &m_brBarBkgnd, NULL);

	CRect rectGripper = rect;

	if (nResizeFlags == (int) CMFCPopupMenu::MENU_RESIZE_BOTTOM_RIGHT || nResizeFlags == (int) CMFCPopupMenu::MENU_RESIZE_TOP_RIGHT)
	{
		rectGripper.left = rectGripper.right - rectGripper.Height();
	}
	else
	{
		rectGripper.left = rectGripper.CenterPoint().x - rectGripper.Height() / 2;
		rectGripper.right = rectGripper.left + rectGripper.Height();
	}

	rectGripper.DeflateRect(2, 2);

	if (m_brGripperHorz.GetSafeHandle() == NULL)
	{
		CreateGripperBrush();
	}

	COLORREF clrTextOld = pDC->SetTextColor(afxGlobalData.clrBarShadow);
	COLORREF clrBkOld = pDC->SetBkColor(m_clrBarBkgnd);

	pDC->FillRect(rectGripper, &m_brGripperHorz);

	pDC->SetTextColor(clrTextOld);
	pDC->SetBkColor(clrBkOld);
}

void CMFCVisualManagerOfficeXP::OnDrawMenuScrollButton(CDC* pDC, CRect rect, BOOL bIsScrollDown, BOOL bIsHighlited, BOOL /*bIsPressed*/, BOOL /*bIsDisabled*/)
{
	ASSERT_VALID(pDC);

	rect.top --;
	pDC->FillRect(rect, &afxGlobalData.brBarFace);

	CMenuImages::Draw(pDC, bIsScrollDown ? CMenuImages::IdArrowDown : CMenuImages::IdArrowUp, rect);

	if (bIsHighlited)
	{
		CPen pen(PS_SOLID, 1, afxGlobalData.clrBarShadow);
		CPen* pOldPen = pDC->SelectObject(&pen);

		CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject(NULL_BRUSH);

		rect.DeflateRect(1, 1);
		pDC->RoundRect(rect, CPoint(2, 2));

		pDC->SelectObject(pOldBrush);
		pDC->SelectObject(pOldPen);
	}
}

void CMFCVisualManagerOfficeXP::CreateGripperBrush()
{
	ASSERT(m_brGripperHorz.GetSafeHandle() == NULL);
	ASSERT(m_brGripperVert.GetSafeHandle() == NULL);

	WORD horzHatchBits [8] = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 };

	CBitmap bmpGripperHorz;
	bmpGripperHorz.CreateBitmap(8, 8, 1, 1, horzHatchBits);

	m_brGripperHorz.CreatePatternBrush(&bmpGripperHorz);

	WORD vertHatchBits[8] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };

	CBitmap bmpGripperVert;
	bmpGripperVert.CreateBitmap(8, 8, 1, 1, vertHatchBits);

	m_brGripperVert.CreatePatternBrush(&bmpGripperVert);
}

void CMFCVisualManagerOfficeXP::ExtendMenuButton(CMFCToolBarMenuButton* pMenuButton, CRect& rect)
{
	ASSERT_VALID(pMenuButton);

	CMFCPopupMenu* pPopupMenu= pMenuButton->GetPopupMenu();
	if (pPopupMenu == NULL || pPopupMenu->GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectMenu;
	pPopupMenu->GetWindowRect(rectMenu);

	if (DYNAMIC_DOWNCAST(CMFCCustomizeButton, pMenuButton) != NULL)
	{
		CBasePane* pParentBar = DYNAMIC_DOWNCAST(CBasePane, pMenuButton->GetParentWnd());

		if (pParentBar != NULL)
		{
			CRect rectScreen = rect;
			pParentBar->ClientToScreen(&rectScreen);

			if (pParentBar->IsHorizontal())
			{
				rectScreen.top = rectMenu.top;
				rectScreen.bottom = rectMenu.bottom;
			}
			else
			{
				rectScreen.left = rectMenu.left;
				rectScreen.right = rectMenu.right;
				rectScreen.bottom++;
			}

			CRect rectInter;
			if (!rectInter.IntersectRect(rectScreen, rectMenu))
			{
				return;
			}
		}
	}

	int nGrow = 4;

	switch(pPopupMenu->GetDropDirection())
	{
	case CMFCPopupMenu::DROP_DIRECTION_BOTTOM:
		if (rectMenu.Width() < rect.Width())
		{
			nGrow = 1;
		}

		rect.bottom += nGrow;
		break;

	case CMFCPopupMenu::DROP_DIRECTION_TOP:
		if (rectMenu.Width() < rect.Width())
		{
			nGrow = 1;
		}

		rect.top -= nGrow;
		break;

	case CMFCPopupMenu::DROP_DIRECTION_RIGHT:
		if (rectMenu.Height() < rect.Height())
		{
			nGrow = 1;
		}

		rect.right += nGrow;
		break;

	case CMFCPopupMenu::DROP_DIRECTION_LEFT:
		if (rectMenu.Height() < rect.Height())
		{
			nGrow = 1;
		}

		rect.left -= nGrow;
		break;
	}
}

void CMFCVisualManagerOfficeXP::OnDrawMenuSystemButton(CDC* pDC, CRect rect, UINT uiSystemCommand, UINT nStyle, BOOL bHighlight)
{
	ASSERT_VALID(pDC);

	BOOL bIsDisabled = (nStyle & TBBS_DISABLED);
	BOOL bIsPressed = (nStyle & TBBS_PRESSED);

	CMenuImages::IMAGES_IDS imageID;

	switch(uiSystemCommand)
	{
	case SC_CLOSE:
		imageID = CMenuImages::IdClose;
		break;

	case SC_MINIMIZE:
		imageID = CMenuImages::IdMinimize;
		break;

	case SC_RESTORE:
		imageID = CMenuImages::IdRestore;
		break;

	default:
		return;
	}

	if (bHighlight && !bIsDisabled)
	{
		OnFillHighlightedArea(pDC, rect, bIsPressed ? &m_brHighlightDn : &m_brHighlight, NULL);

		COLORREF clrBorder = m_clrMenuItemBorder;
		pDC->Draw3dRect(rect, clrBorder, clrBorder);
	}

	CMenuImages::Draw(pDC, imageID, rect, bIsDisabled ? CMenuImages::ImageGray : bHighlight ? CMenuImages::ImageWhite : CMenuImages::ImageBlack);
}

void CMFCVisualManagerOfficeXP::OnDrawStatusBarPaneBorder(CDC* pDC, CMFCStatusBar* /*pBar*/, CRect rectPane, UINT /*uiID*/, UINT nStyle)
{
	if (!(nStyle & SBPS_NOBORDERS))
	{
		if (nStyle & SBPS_POPOUT)
		{
			CDrawingManager dm(*pDC);
			dm.HighlightRect(rectPane);
		}

		// Draw pane border:
		pDC->Draw3dRect(rectPane, m_clrPaneBorder, m_clrPaneBorder);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawComboDropButton(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* /*pButton*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	COLORREF clrText = pDC->GetTextColor();

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
			CPen* pOldPen = pDC->SelectObject(&m_penMenuItemBorder);
			ENSURE(pOldPen != NULL);

			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.left, rect.bottom);

			pDC->SelectObject(pOldPen);
		}
	}
	else
	{
		pDC->FillRect(rect, &afxGlobalData.brBarFace);

		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);
			dm.DrawRect(rect, (COLORREF)-1, afxGlobalData.clrWindow);
		}
		else
		{
			pDC->Draw3dRect(rect, afxGlobalData.clrBarWindow, afxGlobalData.clrBarWindow);
		}
	}

	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rect, bDisabled ? CMenuImages::ImageGray : (bIsDropped && bIsHighlighted) ? CMenuImages::ImageWhite : CMenuImages::ImageBlack);

	pDC->SetTextColor(clrText);
}

void CMFCVisualManagerOfficeXP::OnDrawComboBorder(CDC* pDC, CRect rect, BOOL /*bDisabled*/, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* /*pButton*/)
{
	if (bIsHighlighted || bIsDropped)
	{
		rect.DeflateRect(1, 1);
		pDC->Draw3dRect(&rect,  m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawTabCloseButton(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* /*pTabWnd*/, BOOL bIsHighlighted, BOOL bIsPressed, BOOL /*bIsDisabled*/)
{
	if (bIsHighlighted)
	{
		OnFillHighlightedArea(pDC, rect, bIsPressed ? &m_brHighlightDn : &m_brHighlight, NULL);
	}

	CMenuImages::Draw(pDC, CMenuImages::IdClose, rect, CMenuImages::ImageBlack);

	if (bIsHighlighted)
	{
		pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}

void CMFCVisualManagerOfficeXP::OnEraseTabsButton(CDC* pDC, CRect rect, CMFCButton* pButton, CMFCBaseTabCtrl* pWndTab)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);
	ASSERT_VALID(pWndTab);

	if (pWndTab->IsFlatTab())
	{
		CBrush* pBrush = pButton->IsPressed() ? &m_brHighlightDn : pButton->IsHighlighted() ? &m_brHighlight : &afxGlobalData.brBarFace;

		pDC->FillRect(rect, pBrush);
		OnFillHighlightedArea(pDC, rect, pBrush, NULL);
	}
	else if (pWndTab->IsDialogControl())
	{
		pDC->FillRect(rect, &afxGlobalData.brBtnFace);
	}
	else
	{
		pDC->FillRect(rect, &m_brTabBack);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawTabsButtonBorder(CDC* pDC, CRect& rect, CMFCButton* pButton, UINT /*uiState*/, CMFCBaseTabCtrl* pWndTab)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);
	ASSERT_VALID(pWndTab);

	if (pWndTab->IsFlatTab())
	{
		if (pButton->IsPushed() || pButton->IsHighlighted())
		{
			COLORREF clrDark = afxGlobalData.clrBarDkShadow;
			pDC->Draw3dRect(rect, clrDark, clrDark);
		}
	}
	else
	{
		if (pButton->IsPushed() || pButton->IsHighlighted())
		{
			if (pButton->IsPressed())
			{
				pDC->Draw3dRect(rect, afxGlobalData.clrBarDkShadow, m_clrGripper);
			}
			else
			{
				pDC->Draw3dRect(rect, m_clrGripper, afxGlobalData.clrBarDkShadow);
			}
		}
	}
}

COLORREF CMFCVisualManagerOfficeXP::OnFillMiniFrameCaption(CDC* pDC, CRect rectCaption, CPaneFrameWnd* pFrameWnd, BOOL bActive)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pFrameWnd);

	BOOL bIsToolBar = FALSE;
	BOOL bIsTasksPane = pFrameWnd->IsKindOf( RUNTIME_CLASS( CMFCTasksPaneFrameWnd ) );

	if (DYNAMIC_DOWNCAST(CMFCBaseToolBar, pFrameWnd->GetPane()) != NULL)
	{
		bActive = FALSE;
		bIsToolBar = TRUE;
	}

	if (bIsToolBar)
	{
		pDC->FillRect(rectCaption, &m_brFloatToolBarBorder);
		return afxGlobalData.clrCaptionText;
	}
	else if (bIsTasksPane)
	{
		pDC->FillRect(rectCaption, &afxGlobalData.brBarFace);
		return afxGlobalData.clrBarText;
	}

	pDC->FillRect(rectCaption, bActive ? &afxGlobalData.brActiveCaption : &afxGlobalData.brInactiveCaption);

	// get the text color
	return afxGlobalData.clrCaptionText;
}

void CMFCVisualManagerOfficeXP::OnDrawMiniFrameBorder(CDC* pDC, CPaneFrameWnd* pFrameWnd, CRect rectBorder, CRect rectBorderSize)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pFrameWnd);

	BOOL bIsTasksPane = pFrameWnd->IsKindOf( RUNTIME_CLASS( CMFCTasksPaneFrameWnd ) );

	if (bIsTasksPane)
	{
		CBrush* pOldBrush = pDC->SelectObject(&m_brFloatToolBarBorder);
		ENSURE(pOldBrush != NULL);

		pDC->PatBlt(rectBorder.left, rectBorder.top, rectBorderSize.left, rectBorder.Height(), PATCOPY);
		pDC->PatBlt(rectBorder.left, rectBorder.top, rectBorder.Width(), rectBorderSize.top, PATCOPY);
		pDC->PatBlt(rectBorder.right - rectBorderSize.right, rectBorder.top, rectBorderSize.right, rectBorder.Height(), PATCOPY);
		pDC->PatBlt(rectBorder.left, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width(), rectBorderSize.bottom, PATCOPY);

		rectBorderSize.DeflateRect(2, 2);
		rectBorder.DeflateRect(2, 2);

		pDC->SelectObject(bIsTasksPane ? &afxGlobalData.brLight : &afxGlobalData.brBarFace);

		pDC->PatBlt(rectBorder.left, rectBorder.top + 1, rectBorderSize.left, rectBorder.Height() - 2, PATCOPY);
		pDC->PatBlt(rectBorder.left + 1, rectBorder.top, rectBorder.Width() - 2, rectBorderSize.top, PATCOPY);
		pDC->PatBlt(rectBorder.right - rectBorderSize.right, rectBorder.top + 1, rectBorderSize.right, rectBorder.Height() - 2, PATCOPY);
		pDC->PatBlt(rectBorder.left + 1, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width() - 2, rectBorderSize.bottom, PATCOPY);

		pDC->SelectObject(pOldBrush);
	}
	else
	{
		CMFCVisualManager::OnDrawMiniFrameBorder(pDC, pFrameWnd, rectBorder, rectBorderSize);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawFloatingToolbarBorder(CDC* pDC, CMFCBaseToolBar* /*pToolBar*/, CRect rectBorder, CRect rectBorderSize)
{
	ASSERT_VALID(pDC);

	CBrush* pOldBrush = pDC->SelectObject(&m_brFloatToolBarBorder);
	ENSURE(pOldBrush != NULL);

	pDC->PatBlt(rectBorder.left, rectBorder.top, rectBorderSize.left, rectBorder.Height(), PATCOPY);
	pDC->PatBlt(rectBorder.left, rectBorder.top, rectBorder.Width(), rectBorderSize.top, PATCOPY);
	pDC->PatBlt(rectBorder.right - rectBorderSize.right, rectBorder.top, rectBorderSize.right, rectBorder.Height(), PATCOPY);
	pDC->PatBlt(rectBorder.left, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width(), rectBorderSize.bottom, PATCOPY);

	rectBorderSize.DeflateRect(2, 2);
	rectBorder.DeflateRect(2, 2);

	pDC->SelectObject(&afxGlobalData.brBarFace);

	pDC->PatBlt(rectBorder.left, rectBorder.top + 1, rectBorderSize.left, rectBorder.Height() - 2, PATCOPY);
	pDC->PatBlt(rectBorder.left + 1, rectBorder.top, rectBorder.Width() - 2, rectBorderSize.top, PATCOPY);
	pDC->PatBlt(rectBorder.right - rectBorderSize.right, rectBorder.top + 1, rectBorderSize.right, rectBorder.Height() - 2, PATCOPY);
	pDC->PatBlt(rectBorder.left + 1, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width() - 2, rectBorderSize.bottom, PATCOPY);

	pDC->SelectObject(pOldBrush);
}

COLORREF CMFCVisualManagerOfficeXP::GetToolbarButtonTextColor(CMFCToolBarButton* pButton, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pButton);

	if (!afxGlobalData.IsHighContrastMode())
	{
		BOOL bDisabled = (CMFCToolBar::IsCustomizeMode() && !pButton->IsEditable()) || (!CMFCToolBar::IsCustomizeMode() &&(pButton->m_nStyle & TBBS_DISABLED));

		if (pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton)))
		{
			if (bDisabled)
			{
				return afxGlobalData.clrGrayedText;
			}

			return afxGlobalData.IsHighContrastMode() ? afxGlobalData.clrWindowText : afxGlobalData.clrBarText;
		}

		if (state == ButtonsIsHighlighted && (pButton->m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)))
		{
			return afxGlobalData.clrTextHilite;
		}
	}

	return CMFCVisualManager::GetToolbarButtonTextColor(pButton, state);
}

void CMFCVisualManagerOfficeXP::OnDrawEditBorder(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsHighlighted, CMFCToolBarEditBoxButton* pButton)
{
	if (!CMFCToolBarEditBoxButton::IsFlatMode())
	{
		CMFCVisualManager::OnDrawEditBorder(pDC, rect, bDisabled, bIsHighlighted, pButton);
		return;
	}

	if (bIsHighlighted)
	{
		pDC->Draw3dRect(&rect,  m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawTasksGroupCaption(CDC* pDC, CMFCTasksPaneTaskGroup* pGroup, BOOL bIsHighlighted, BOOL bIsSelected, BOOL bCanCollapse)
{
	ENSURE(pGroup != NULL);
	ENSURE(pGroup->m_pPage != NULL);

	ASSERT_VALID(pDC);
	ASSERT_VALID(pGroup);
	ASSERT_VALID(pGroup->m_pPage);

	CRect rectGroup = pGroup->m_rect;

	// ------------------------------
	// Draw group caption(Office XP)
	// ------------------------------

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
		clrTextOld = pDC->SetTextColor(pGroup->m_clrTextHot == (COLORREF)-1 ? afxGlobalData.clrWindowText : pGroup->m_clrTextHot);
	}
	else
	{
		clrTextOld = pDC->SetTextColor(pGroup->m_clrText == (COLORREF)-1 ? afxGlobalData.clrWindowText : pGroup->m_clrText);
	}

	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();
	int nCaptionHOffset = (nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset);

	CRect rectText = rectGroup;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx + 5: nCaptionHOffset);
	rectText.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectText.right = max(rectText.left, rectText.right -(bCanCollapse ? rectGroup.Height() : nCaptionHOffset));

	pDC->DrawText(pGroup->m_strName, rectText, DT_SINGLELINE | DT_VCENTER);

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
		rectButton.left = max(rectButton.left, rectButton.right - sizeButton.cx);
		rectButton.top = max(rectButton.top, rectButton.bottom - sizeButton.cy);

		if (rectButton.Width() >= sizeButton.cx && rectButton.Height() >= sizeButton.cy)
		{
			if (bIsHighlighted)
			{
				// Draw button frame
				CPen* pPenOld = (CPen*) pDC->SelectObject(&afxGlobalData.penHilite);
				CBrush* pBrushOld = (CBrush*) pDC->SelectObject(&m_brHighlight);
				COLORREF clrBckOld = pDC->GetBkColor();

				pDC->Rectangle(&rectButton);

				pDC->SetBkColor(clrBckOld);
				pDC->SelectObject(pPenOld);
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

void CMFCVisualManagerOfficeXP::OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL /*bSpecial*/)
{
	ASSERT_VALID(pDC);

	// Draw underline
	CPen* pPenOld = (CPen*) pDC->SelectObject(&afxGlobalData.penBarShadow);
	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.right, rect.top);
	pDC->SelectObject(pPenOld);
}

void CMFCVisualManagerOfficeXP::OnDrawTasksGroupAreaBorder(CDC* /*pDC*/, CRect /*rect*/, BOOL /*bSpecial*/, BOOL /*bNoTitle*/)
{
}

void CMFCVisualManagerOfficeXP::OnDrawTask(CDC* pDC, CMFCTasksPaneTask* pTask, CImageList* pIcons, BOOL bIsHighlighted, BOOL /*bIsSelected*/)
{
	ENSURE(pTask != NULL);
	ENSURE(pIcons != NULL);

	ASSERT_VALID(pDC);
	ASSERT_VALID(pTask);
	ASSERT_VALID(pIcons);

	CRect rectText = pTask->m_rect;

	if (pTask->m_bIsSeparator)
	{
		CPen* pPenOld = (CPen*) pDC->SelectObject(&afxGlobalData.penBarShadow);

		pDC->MoveTo(rectText.left, rectText.CenterPoint().y);
		pDC->LineTo(rectText.right, rectText.CenterPoint().y);

		pDC->SelectObject(pPenOld);
		return;
	}

	// ---------
	// Draw icon
	// ---------
	CSize sizeIcon(0, 0);
	::ImageList_GetIconSize(pIcons->m_hImageList, (int*) &sizeIcon.cx, (int*) &sizeIcon.cy);
	if (pTask->m_nIcon >= 0 && sizeIcon.cx > 0)
	{
		pIcons->Draw(pDC, pTask->m_nIcon, rectText.TopLeft(), ILD_TRANSPARENT);
	}
	int nTaskPaneOffset = pTask->m_pGroup->m_pPage->m_pTaskPane->GetTasksIconHorzOffset();
	rectText.left += sizeIcon.cx +(nTaskPaneOffset != -1 ? nTaskPaneOffset : m_nTasksIconHorzOffset);

	// ---------
	// Draw text
	// ---------
	BOOL bIsLabel = (pTask->m_uiCommandID == 0);

	CFont* pFontOld = NULL;
	COLORREF clrTextOld = pDC->GetTextColor();
	if (bIsLabel)
	{
		pFontOld = pDC->SelectObject(pTask->m_bIsBold ? &afxGlobalData.fontBold : &afxGlobalData.fontRegular);
		pDC->SetTextColor(pTask->m_clrText == (COLORREF)-1 ? afxGlobalData.clrWindowText : pTask->m_clrText);
	}
	else if (!pTask->m_bEnabled)
	{
		pDC->SetTextColor(afxGlobalData.clrGrayedText);
		pFontOld = pDC->SelectObject(&afxGlobalData.fontRegular);
	}
	else if (bIsHighlighted)
	{
		pDC->SetTextColor(pTask->m_clrTextHot == (COLORREF)-1 ? afxGlobalData.clrHotLinkNormalText : pTask->m_clrTextHot);
		pFontOld = pDC->SelectObject(&afxGlobalData.fontUnderline);
	}
	else
	{
		pDC->SetTextColor(pTask->m_clrText == (COLORREF)-1 ? afxGlobalData.clrWindowText : pTask->m_clrText);
		pFontOld = pDC->SelectObject(&afxGlobalData.fontRegular);
	}
	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	CMFCTasksPane* pTaskPane = pTask->m_pGroup->m_pPage->m_pTaskPane;
	ASSERT_VALID(pTaskPane);

	BOOL bMultiline = bIsLabel ? pTaskPane->IsWrapLabelsEnabled() : pTaskPane->IsWrapTasksEnabled();

	if (bMultiline)
	{
		pDC->DrawText(pTask->m_strName, rectText, DT_WORDBREAK);
	}
	else
	{
		pDC->DrawText(pTask->m_strName, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
	}

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject(pFontOld);
	pDC->SetTextColor(clrTextOld);
}

void CMFCVisualManagerOfficeXP::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize, int iImage, BOOL bHilited)
{
	ASSERT_VALID(pDC);

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect(rectFill, &afxGlobalData.brWindow);

	if (bHilited)
	{
		pDC->FillRect(rect, &m_brHighlight);
		pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}

	CMenuImages::Draw(pDC, (CMenuImages::IMAGES_IDS) iImage, rect);
}

void CMFCVisualManagerOfficeXP::OnDrawSpinButtons(CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CMFCSpinButtonCtrl* /*pSpinCtrl*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	CRect rect [2];
	rect[0] = rect[1] = rectSpin;

	if (!bOrientation)
	{
		rect[0].DeflateRect(0, 0, 0, rect[0].Height() / 2);
		rect[1].top = rect[0].bottom ;
	}
	else
	{
		rect[0].DeflateRect(0, 0, rect[0].Width() / 2, 0);
		rect[1].left = rect[0].right ;
	}

	CMenuImages::IMAGES_IDS id[2][2] = {{CMenuImages::IdArrowUp, CMenuImages::IdArrowDown}, {CMenuImages::IdArrowLeft, CMenuImages::IdArrowRight}};

	int idxPressed = (nState &(AFX_SPIN_PRESSEDUP | AFX_SPIN_PRESSEDDOWN)) - 1;
	int idxHighlighted = -1;
	if (nState & AFX_SPIN_HIGHLIGHTEDUP)
	{
		idxHighlighted = 0;
	}
	else if (nState & AFX_SPIN_HIGHLIGHTEDDOWN)
	{
		idxHighlighted = 1;
	}

	BOOL bDisabled = nState & AFX_SPIN_DISABLED;

	for (int i = 0; i < 2; i ++)
	{
		if (idxPressed == i || idxHighlighted == i)
		{
			OnFillHighlightedArea(pDC, rect [i], (idxPressed == i) ? &m_brHighlightDn : &m_brHighlight, NULL);
		}
		else
		{
			if (CMFCToolBarImages::m_bIsDrawOnGlass)
			{
				CDrawingManager dm(*pDC);
				dm.DrawRect(rect[i], afxGlobalData.clrBarFace, afxGlobalData.clrBarHilite);
			}
			else
			{
				pDC->FillRect(rect[i], &afxGlobalData.brBarFace);
				pDC->Draw3dRect(rect[i], afxGlobalData.clrBarHilite, afxGlobalData.clrBarHilite);
			}
		}

		CMenuImages::Draw(pDC, id [bOrientation ? 1 : 0][i], rect[i], bDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack);
	}

	if (idxHighlighted >= 0)
	{
		CRect rectHot = rect [idxHighlighted];

		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);
			dm.DrawRect(rectHot, (COLORREF)-1, m_clrMenuItemBorder);
		}
		else
		{
			pDC->Draw3dRect(rectHot, m_clrMenuItemBorder, m_clrMenuItemBorder);
		}
	}
}

void CMFCVisualManagerOfficeXP::OnFillHighlightedArea(CDC* pDC, CRect rect, CBrush* pBrush, CMFCToolBarButton* /*pButton*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBrush);

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		LOGBRUSH br;
		pBrush->GetLogBrush(&br);

		CDrawingManager dm(*pDC);
		dm.DrawRect(rect, br.lbColor, (COLORREF)-1);
	}
	else
	{
		pDC->FillRect(rect, pBrush);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawSplitterBorder(CDC* pDC, CSplitterWndEx* /*pSplitterWnd*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
	rect.InflateRect(-AFX_CX_BORDER, -AFX_CY_BORDER);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarFace, afxGlobalData.clrBarFace);
}

void CMFCVisualManagerOfficeXP::OnDrawSplitterBox(CDC* pDC, CSplitterWndEx* /*pSplitterWnd*/, CRect& rect)
{
	ASSERT_VALID(pDC);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarFace, afxGlobalData.clrBarFace);
}

BOOL CMFCVisualManagerOfficeXP::OnDrawBrowseButton(CDC* pDC, CRect rect, CMFCEditBrowseCtrl* /*pEdit*/, CMFCVisualManager::AFX_BUTTON_STATE state, COLORREF& /*clrText*/)
{
	ASSERT_VALID(pDC);

	CRect rectFrame = rect;
	rectFrame.InflateRect(0, 1, 1, 1);

	switch(state)
	{
	case ButtonsIsPressed:
		pDC->FillRect(rect, &m_brHighlightDn);
		pDC->Draw3dRect(&rectFrame, m_clrMenuItemBorder, m_clrMenuItemBorder);
		pDC->SetTextColor(afxGlobalData.clrWindow);
		break;

	case ButtonsIsHighlighted:
		pDC->FillRect(rect, &m_brHighlight);
		pDC->Draw3dRect(&rectFrame, m_clrMenuItemBorder, m_clrMenuItemBorder);
		break;

	default:
		pDC->FillRect(rect, &afxGlobalData.brBtnFace);
		pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarHilite);
		break;
	}

	return TRUE;
}

COLORREF CMFCVisualManagerOfficeXP::GetWindowColor() const
{
	return afxGlobalData.clrWindow;
}

COLORREF CMFCVisualManagerOfficeXP::GetAutoHideButtonTextColor(CMFCAutoHideButton* /*pButton*/)
{
	return afxGlobalData.clrBtnDkShadow;
}

void CMFCVisualManagerOfficeXP::GetSmartDockingBaseGuideColors(COLORREF& clrBaseGroupBackground, COLORREF& clrBaseGroupBorder)
{
	clrBaseGroupBackground = m_clrBarBkgnd;
	clrBaseGroupBorder = m_clrMenuBorder;
}

void CMFCVisualManagerOfficeXP::OnDrawButtonSeparator(CDC* pDC, CMFCToolBarButton* /*pButton*/, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE /*state*/, BOOL bHorz)
{
	CPen* pOldPen = pDC->SelectObject(&m_penMenuItemBorder);
	ENSURE(pOldPen != NULL);

	if (bHorz)
	{
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.left, rect.bottom);
	}
	else
	{
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.right, rect.top);
	}

	pDC->SelectObject(pOldPen);
}

void CMFCVisualManagerOfficeXP::OnDrawPopupWindowBorder(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, m_clrMenuBorder, m_clrMenuBorder);
	rect.DeflateRect(1, 1);
	pDC->Draw3dRect(rect, m_clrMenuLight, m_clrMenuLight);
}

COLORREF  CMFCVisualManagerOfficeXP::OnDrawPopupWindowCaption(CDC* pDC, CRect rectCaption, CMFCDesktopAlertWnd* /*pPopupWnd*/)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rectCaption, &m_brHighlight);

	// get the text color
	return afxGlobalData.clrBarText;
}

void CMFCVisualManagerOfficeXP::OnErasePopupWindowButton(CDC* pDC, CRect rc, CMFCDesktopAlertWndButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsPressed())
	{
		CBrush br(m_clrHighlightDn);
		pDC->FillRect(&rc, &br);
		return;
	}
	else if (pButton->IsHighlighted() || pButton->IsPushed())
	{
		CBrush br(m_clrHighlight);
		pDC->FillRect(&rc, &br);
		return;
	}

	CRect rectParent;
	pButton->GetParent()->GetClientRect(rectParent);

	pButton->GetParent()->MapWindowPoints(pButton, rectParent);
	OnFillPopupWindowBackground(pDC, rectParent);
}

void CMFCVisualManagerOfficeXP::OnDrawPopupWindowButtonBorder(CDC* pDC, CRect rc, CMFCDesktopAlertWndButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsHighlighted() || pButton->IsPushed() || pButton->IsCaptionButton())
	{
		pDC->Draw3dRect(rc, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}

void CMFCVisualManagerOfficeXP::OnFillPopupWindowBackground(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &m_brMenuLight);
}

COLORREF CMFCVisualManagerOfficeXP::GetPropertyGridGroupColor(CMFCPropertyGridCtrl* pPropList)
{
	ASSERT_VALID(pPropList);

	if (afxGlobalData.m_nBitsPerPixel <= 8)
	{
		return CMFCVisualManager::GetPropertyGridGroupColor(pPropList);
	}

	return CDrawingManager::PixelAlpha(pPropList->DrawControlBarColors() ? afxGlobalData.clrBarFace : afxGlobalData.clrBtnFace, 94);
}

COLORREF CMFCVisualManagerOfficeXP::GetPropertyGridGroupTextColor(CMFCPropertyGridCtrl* pPropList)
{
	ASSERT_VALID(pPropList);

	return pPropList->DrawControlBarColors() ? afxGlobalData.clrBarShadow : afxGlobalData.clrBtnShadow;
}

COLORREF CMFCVisualManagerOfficeXP::OnFillRibbonButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsDefaultPanelButton() && !pButton->IsQATMode())
	{
		return(COLORREF)-1;
	}

	CRect rect = pButton->GetRect();

	const BOOL bIsMenuMode = pButton->IsMenuMode() && !pButton->IsGalleryIcon();

	const BOOL bIsHighlighted = (pButton->IsHighlighted() || pButton->IsDroppedDown()) && !pButton->IsDisabled();

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonEdit)))
	{
		COLORREF clrBorder = afxGlobalData.clrBarShadow;
		CRect rectCommand = pButton->GetCommandRect();

		if (pButton->GetLocationInGroup() != CMFCRibbonBaseElement::RibbonElementNotInGroup)
		{
			rectCommand.right++;
		}

		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);
			if (bIsHighlighted)
			{
				dm.DrawRect(rect, afxGlobalData.clrWindow, clrBorder);
			}
			else
			{
				dm.DrawRect(rect, afxGlobalData.clrBarFace, clrBorder);
			}
		}
		else
		{
			if (bIsHighlighted)
			{
				pDC->FillRect(rectCommand, &afxGlobalData.brWindow);
			}
			else
			{
				pDC->FillRect(rectCommand, &afxGlobalData.brBarFace);

				CDrawingManager dm(*pDC);
				dm.HighlightRect(rectCommand);
			}

			pDC->Draw3dRect(rect, clrBorder, clrBorder);
		}

		return(COLORREF)-1;
	}

	if (!pButton->IsChecked() && !bIsHighlighted)
	{
		return(COLORREF)-1;
	}

	if (pButton->IsChecked() && bIsMenuMode && !bIsHighlighted)
	{
		return(COLORREF)-1;
	}

	CRect rectMenu = pButton->GetMenuRect();

	if (pButton->GetLocationInGroup() != CMFCRibbonBaseElement::RibbonElementNotInGroup)
	{
		rect.DeflateRect(1, 1);
	}

	CRect rectCommand(0, 0, 0, 0);
	if (!rectMenu.IsRectEmpty())
	{
		rectCommand = pButton->GetCommandRect();

		if (pButton->GetLocationInGroup() != CMFCRibbonBaseElement::RibbonElementNotInGroup)
		{
			rectMenu.DeflateRect(0, 1, 1, 1);
			rectCommand.DeflateRect(1, 1, 0, 1);
		}
	}

	if (!rectMenu.IsRectEmpty() && bIsHighlighted)
	{
		if (pButton->IsCommandAreaHighlighted())
		{
			OnFillHighlightedArea(pDC, rectCommand, (pButton->IsPressed() || pButton->IsDroppedDown()) && !bIsMenuMode ? &m_brHighlightDn : &m_brHighlight, NULL);
		}
		else
		{
			OnFillHighlightedArea(pDC, rectCommand, &m_brHighlight, NULL);

			CDrawingManager dm(*pDC);
			dm.HighlightRect(rectCommand);
		}

		if (pButton->IsMenuAreaHighlighted())
		{
			OnFillHighlightedArea(pDC, rectMenu, (pButton->IsPressed() || pButton->IsDroppedDown()) && !bIsMenuMode ? &m_brHighlightDn : &m_brHighlight, NULL);
		}
		else
		{
			OnFillHighlightedArea(pDC, rectMenu, &m_brHighlight, NULL);

			CDrawingManager dm(*pDC);
			dm.HighlightRect(rectMenu);
		}
	}
	else
	{
		CBrush* pBrush = (pButton->IsPressed() || pButton->IsDroppedDown()) && !bIsMenuMode ? &m_brHighlightDn : &m_brHighlight;

		CRect rectFill = rect;

		if (pButton->IsChecked() && !bIsMenuMode)
		{
			pBrush = bIsHighlighted ? &m_brHighlightDn : &m_brHighlightChecked;

			if (!bIsHighlighted && !rectCommand.IsRectEmpty())
			{
				rectFill = rectCommand;
			}
		}

		OnFillHighlightedArea(pDC, rectFill, pBrush, NULL);
	}

	return(COLORREF)-1;
}

void CMFCVisualManagerOfficeXP::OnDrawRibbonButtonBorder(CDC* pDC, CMFCRibbonButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsDefaultPanelButton() && !pButton->IsQATMode())
	{
		return;
	}

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonEdit)))
	{
		return;
	}

	const BOOL bIsMenuMode = pButton->IsMenuMode();

	const BOOL bIsHighlighted = (pButton->IsHighlighted() || pButton->IsDroppedDown()) && !pButton->IsDisabled();

	if (pButton->IsChecked() && bIsMenuMode && !bIsHighlighted)
	{
		return;
	}

	CRect rect = pButton->GetRect();
	CRect rectMenu = pButton->GetMenuRect();

	if (pButton->GetLocationInGroup() != CMFCRibbonBaseElement::RibbonElementNotInGroup)
	{
		rect.DeflateRect(1, 1);
	}

	if ((bIsHighlighted || pButton->IsChecked()) && (!pButton->IsDisabled() || pButton->IsFocused() || pButton->IsChecked()))
	{
		COLORREF clrLine = ((pButton->IsPressed() || pButton->IsDroppedDown()) && !bIsMenuMode) ? m_clrPressedButtonBorder : m_clrMenuItemBorder;

		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);
			dm.DrawRect(rect, (COLORREF)-1, m_clrMenuItemBorder);
		}
		else
		{
			pDC->Draw3dRect(rect, clrLine, clrLine);
		}

		if (!rectMenu.IsRectEmpty())
		{
			if (pButton->GetLocationInGroup() != CMFCRibbonBaseElement::RibbonElementNotInGroup)
			{
				rectMenu.DeflateRect(0, 1, 1, 1);
			}

			if (CMFCToolBarImages::m_bIsDrawOnGlass)
			{
				CDrawingManager dm(*pDC);

				if (pButton->IsMenuOnBottom())
				{
					dm.DrawLine(rectMenu.left, rectMenu.top, rectMenu.right, rectMenu.top, m_clrMenuItemBorder);
				}
				else
				{
					dm.DrawLine(rectMenu.left, rectMenu.top, rectMenu.left, rectMenu.bottom, m_clrMenuItemBorder);
				}
			}
			else
			{
				CPen* pOldPen = pDC->SelectObject(&m_penMenuItemBorder);
				ENSURE(pOldPen != NULL);

				if (pButton->IsMenuOnBottom())
				{
					pDC->MoveTo(rectMenu.left, rectMenu.top);
					pDC->LineTo(rectMenu.right, rectMenu.top);
				}
				else
				{
					pDC->MoveTo(rectMenu.left, rectMenu.top);
					pDC->LineTo(rectMenu.left, rectMenu.bottom);
				}

				pDC->SelectObject(pOldPen);
			}
		}
	}
}

void CMFCVisualManagerOfficeXP::OnDrawRibbonCategoryScroll (CDC* pDC, CRibbonCategoryScroll* pScroll)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pScroll);

	CRect rect = pScroll->GetRect();
	rect.bottom--;

	OnFillHighlightedArea(pDC, rect, 
		pScroll->IsHighlighted () ? &m_brHighlight : &afxGlobalData.brBarFace, NULL);

	BOOL bIsLeft = pScroll->IsLeftScroll();
	if (afxGlobalData.m_bIsRTL)
	{
		bIsLeft = !bIsLeft;
	}

	CMenuImages::Draw(pDC,
		bIsLeft ? CMenuImages::IdArrowLeftLarge : CMenuImages::IdArrowRightLarge, 
		rect);

	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
}

void CMFCVisualManagerOfficeXP::OnDrawRibbonMenuCheckFrame(CDC* pDC, CMFCRibbonButton* /*pButton*/, CRect rect)
{
	ASSERT_VALID(pDC);

	OnFillHighlightedArea(pDC, rect, &m_brHighlight, NULL);
	pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
}

void CMFCVisualManagerOfficeXP::OnFillRibbonMenuFrame(CDC* pDC, CMFCRibbonMainPanel* /*pPanel*/, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &m_brMenuLight);
}

void CMFCVisualManagerOfficeXP::OnDrawRibbonRecentFilesFrame(CDC* pDC, CMFCRibbonMainPanel* /*pPanel*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &m_brBarBkgnd);

	CRect rectSeparator = rect;
	rectSeparator.right = rectSeparator.left + 2;

	pDC->Draw3dRect(rectSeparator, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
}

COLORREF CMFCVisualManagerOfficeXP::OnDrawRibbonStatusBarPane(CDC* pDC, CMFCRibbonStatusBar* /*pBar*/, CMFCRibbonStatusBarPane* pPane)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pPane);

	CRect rect = pPane->GetRect();

	if (pPane->IsHighlighted())
	{
		CRect rectButton = rect;
		rectButton.DeflateRect(1, 1);

		OnFillHighlightedArea(pDC, rectButton, pPane->IsPressed() ? &m_brHighlightDn : &m_brHighlight, NULL);
		pDC->Draw3dRect(rectButton, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}

	CRect rectSeparator = rect;
	rectSeparator.DeflateRect(0, 2);

	rectSeparator.left = rectSeparator.right - 1;

	pDC->Draw3dRect(rectSeparator, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);

	if (afxGlobalData.IsHighContrastMode())
	{
		return afxGlobalData.clrBarText;
	}

	return(COLORREF)-1;
}

void CMFCVisualManagerOfficeXP::GetRibbonSliderColors(CMFCRibbonSlider* /*pSlider*/, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled, COLORREF& clrLine, COLORREF& clrFill)
{
	clrLine = bIsDisabled ? afxGlobalData.clrBarShadow : (bIsPressed || bIsHighlighted) ? m_clrMenuItemBorder : afxGlobalData.clrBarDkShadow;
	clrFill = bIsPressed && bIsHighlighted ? m_clrHighlightDn : bIsHighlighted ? m_clrHighlight : afxGlobalData.clrBarFace;
}

void CMFCVisualManagerOfficeXP::OnDrawRibbonQuickAccessToolBarSeparator(CDC* pDC, CMFCRibbonSeparator* /*pSeparator*/, CRect rect)
{
	ASSERT_VALID(pDC);

	int x = rect.CenterPoint().x;
	int y1 = rect.top;
	int y2 = rect.bottom - 1;

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawLine(x, y1, x, y2, m_clrSeparator);
	}
	else
	{
		CPen* pOldPen = pDC->SelectObject(&m_penSeparator);
		ENSURE(pOldPen != NULL);

		pDC->MoveTo(x, y1);
		pDC->LineTo(x, y2);

		pDC->SelectObject(pOldPen);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawRibbonColorPaletteBox(CDC* pDC, CMFCRibbonColorButton* /*pColorButton*/, CMFCRibbonGalleryIcon* /*pIcon*/,
	COLORREF color, CRect rect, BOOL bDrawTopEdge, BOOL bDrawBottomEdge, BOOL bIsHighlighted, BOOL bIsChecked, BOOL /*bIsDisabled*/)
{
	ASSERT_VALID(pDC);

	CRect rectFill = rect;
	rectFill.DeflateRect(1, 0);

	if (bIsHighlighted || bIsChecked)
	{
		OnFillHighlightedArea(pDC, rect, &m_brHighlight, NULL);
		rectFill.DeflateRect(1, 2);
	}

	if (color != (COLORREF)-1)
	{
		CBrush br(color);
		pDC->FillRect(rectFill, &br);
	}

	COLORREF clrBorder = RGB(197, 197, 197);

	if (bDrawTopEdge && bDrawBottomEdge)
	{
		pDC->Draw3dRect(rect, clrBorder, clrBorder);
	}
	else
	{
		CPen penBorder(PS_SOLID, 1, clrBorder);

		CPen* pOldPen = pDC->SelectObject(&penBorder);
		ENSURE(pOldPen != NULL);

		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.left, rect.bottom);

		pDC->MoveTo(rect.right - 1, rect.top);
		pDC->LineTo(rect.right - 1, rect.bottom);

		if (bDrawTopEdge)
		{
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right, rect.top);
		}

		if (bDrawBottomEdge)
		{
			pDC->MoveTo(rect.left, rect.bottom - 1);
			pDC->LineTo(rect.right, rect.bottom - 1);
		}

		pDC->SelectObject(pOldPen);
	}

	if (bIsHighlighted || bIsChecked)
	{
		clrBorder = bIsChecked ? m_clrPressedButtonBorder : m_clrMenuItemBorder;
		pDC->Draw3dRect(rect, clrBorder, clrBorder);
	}
}

COLORREF CMFCVisualManagerOfficeXP::OnDrawPropertySheetListItem(CDC* pDC, CMFCPropertySheet* /*pParent*/, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	ASSERT_VALID(pDC);

	CBrush* pBrush = NULL;

	if (bIsSelected)
	{
		pBrush = !bIsHighlihted ? &m_brHighlightChecked : &m_brHighlightDn;
	}
	else if (bIsHighlihted)
	{
		pBrush = &m_brHighlight;
	}

	OnFillHighlightedArea(pDC, rect, pBrush, NULL);
	pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);

	return afxGlobalData.clrBtnText;
}

COLORREF CMFCVisualManagerOfficeXP::OnDrawMenuLabel(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &m_brBarBkgnd);

	CRect rectSeparator = rect;
	rectSeparator.top = rectSeparator.bottom - 2;

	pDC->Draw3dRect(rectSeparator, afxGlobalData.clrBarShadow,
		afxGlobalData.clrBarHilite);

	return afxGlobalData.clrBarText;
}

COLORREF CMFCVisualManagerOfficeXP::OnFillCaptionBarButton(CDC* pDC, CMFCCaptionBar* pBar, CRect rect,
	BOOL bIsPressed, BOOL bIsHighlighted, BOOL bIsDisabled, BOOL bHasDropDownArrow, BOOL bIsSysButton)
{
	ASSERT_VALID(pBar);

	if (!pBar->IsMessageBarMode())
	{
		return CMFCVisualManager::OnFillCaptionBarButton(pDC, pBar, rect, bIsPressed, bIsHighlighted, bIsDisabled, bHasDropDownArrow, bIsSysButton);
	}

	if (bIsDisabled)
	{
		return(COLORREF)-1;
	}

	COLORREF clrText = afxGlobalData.clrBarText;

	if (bIsHighlighted)
	{
		OnFillHighlightedArea(pDC, rect, &m_brHighlight, NULL);

		if (GetRValue(m_clrHighlight) > 128 && GetGValue(m_clrHighlight) > 128 && GetBValue(m_clrHighlight) > 128)
		{
			clrText = RGB(0, 0, 0);
		}
		else
		{
			clrText = RGB(255, 255, 255);
		}
	}
	else if (!bIsSysButton)
	{
		pDC->FillRect(rect, &m_brMenuLight);
	}

	return clrText;
}

void CMFCVisualManagerOfficeXP::OnDrawCaptionBarButtonBorder(CDC* pDC, CMFCCaptionBar* pBar, CRect rect,
	BOOL bIsPressed, BOOL bIsHighlighted, BOOL bIsDisabled, BOOL bHasDropDownArrow, BOOL bIsSysButton)
{
	ASSERT_VALID(pBar);

	if (!pBar->IsMessageBarMode())
	{
		CMFCVisualManager::OnDrawCaptionBarButtonBorder(pDC, pBar, rect, bIsPressed, bIsHighlighted, bIsDisabled, bHasDropDownArrow, bIsSysButton);
		return;
	}

	ASSERT_VALID(pDC);

	if (bIsHighlighted)
	{
		if (bIsSysButton && bIsPressed && m_clrPressedButtonBorder != (COLORREF)-1)
		{
			pDC->Draw3dRect(rect, m_clrPressedButtonBorder, m_clrPressedButtonBorder);
		}
		else
		{
			pDC->Draw3dRect(rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
		}
	}
	else if (!bIsSysButton)
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarDkShadow);
	}
}

void CMFCVisualManagerOfficeXP::OnDrawCaptionBarInfoArea(CDC* pDC, CMFCCaptionBar* /*pBar*/, CRect rect)
{
	ASSERT_VALID(pDC);

	::FillRect(pDC->GetSafeHdc(), rect, ::GetSysColorBrush(COLOR_INFOBK));
	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
}



