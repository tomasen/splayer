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
#include "afxglobalutils.h"
#include "afxvisualmanager.h"
#include "afxtoolbarbutton.h"
#include "afxoutlookbarpane.h"
#include "afxoutlookbarpanebutton.h"
#include "afxglobals.h"
#include "afxdockablepane.h"
#include "afxbasepane.h"
#include "afxtoolbar.h"
#include "afxtabctrl.h"
#include "afxdrawmanager.h"
#include "afxshowallbutton.h"
#include "afxbutton.h"
#include "afxpaneframewnd.h"
#include "afxcaptionbar.h"
#include "afxoutlookbarpanebutton.h"
#include "afxtaskspane.h"
#include "afxpanedivider.h"
#include "afxmenuimages.h"
#include "afxheaderctrl.h"
#include "afxspinbuttonctrl.h"
#include "afxdockingmanager.h"
#include "afxtabbedpane.h"
#include "afxautohidebutton.h"
#include "afxdesktopalertwnd.h"
#include "afxpropertygridctrl.h"
#include "afxstatusbar.h"
#include "afxribbonbar.h"
#include "afxribbonpanel.h"
#include "afxribboncategory.h"
#include "afxribbonbutton.h"
#include "afxribbonstatusbarpane.h"
#include "afxribbonlinkctrl.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxribbonedit.h"
#include "afxribbonlabel.h"
#include "afxribbonpalettegallery.h"
#include "afxribbonprogressbar.h"
#include "afxtooltipctrl.h"
#include "afxtooltipmanager.h"
#include "afxribboncolorbutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCVisualManager, CObject)

extern CObList afxAllToolBars;
extern CTooltipManager* afxTooltipManager;

CMFCVisualManager* CMFCVisualManager::m_pVisManager = NULL;
CRuntimeClass* CMFCVisualManager::m_pRTIDefault = NULL;

UINT AFX_WM_CHANGEVISUALMANAGER = ::RegisterWindowMessage(_T("AFX_WM_CHANGEVISUALMANAGER"));

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCVisualManager::CMFCVisualManager(BOOL bIsTemporary)
{
	m_bAutoDestroy = FALSE;
	m_bIsTemporary = bIsTemporary;

	if (!bIsTemporary)
	{
		if (m_pVisManager != NULL)
		{
			ASSERT(FALSE);
		}
		else
		{
			m_pVisManager = this;
		}
	}

	m_bMenuFlatLook = FALSE;
	m_nMenuShadowDepth = 6;
	m_bShadowHighlightedImage = FALSE;
	m_bEmbossDisabledImage = TRUE;
	m_bFadeInactiveImage = FALSE;
	m_bEnableToolbarButtonFill = TRUE;

	m_nVertMargin = 12;
	m_nHorzMargin = 12;
	m_nGroupVertOffset = 15;
	m_nGroupCaptionHeight = 25;
	m_nGroupCaptionHorzOffset = 13;
	m_nGroupCaptionVertOffset = 7;
	m_nTasksHorzOffset = 12;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = TRUE;

	m_bOfficeXPStyleMenus = FALSE;
	m_nMenuBorderSize = 2;

	m_b3DTabWideBorder = TRUE;
	m_bAlwaysFillTab = FALSE;
	m_bFrameMenuCheckedItems = FALSE;
	m_clrMenuShadowBase = (COLORREF)-1; // Used in derived classes

	if (!bIsTemporary)
	{
		CDockingManager::m_bSDParamsModified = TRUE;
		CDockingManager::EnableDockSiteMenu(FALSE);

		CMFCAutoHideButton::m_bOverlappingTabs = TRUE;

		afxGlobalData.UpdateSysColors();
	}

	OnUpdateSystemColors();
}

CMFCVisualManager::~CMFCVisualManager()
{
	if (!m_bIsTemporary)
	{
		m_pVisManager = NULL;
	}
}

void CMFCVisualManager::OnUpdateSystemColors()
{
}

void __stdcall CMFCVisualManager::SetDefaultManager(CRuntimeClass* pRTI)
{
	if (pRTI != NULL && !pRTI->IsDerivedFrom(RUNTIME_CLASS(CMFCVisualManager)))
	{
		ASSERT(FALSE);
		return;
	}

	m_pRTIDefault = pRTI;

	if (m_pVisManager != NULL)
	{
		ASSERT_VALID(m_pVisManager);

		delete m_pVisManager;
		m_pVisManager = NULL;
	}

	afxGlobalData.UpdateSysColors();

	CDockingManager::SetDockingMode(DT_STANDARD);
	CTabbedPane::ResetTabs();

	AdjustFrames();
	AdjustToolbars();

	RedrawAll();

	if (afxTooltipManager != NULL)
	{
		afxTooltipManager->UpdateTooltips();
	}
}

void __stdcall CMFCVisualManager::RedrawAll()
{
	CWnd* pMainWnd = AfxGetMainWnd();
	BOOL bIsMainWndRedrawn = FALSE;

	const CList<CFrameWnd*, CFrameWnd*>& lstFrames = CFrameImpl::GetFrameList();

	for (POSITION pos = lstFrames.GetHeadPosition(); pos != NULL;)
	{
		CFrameWnd* pFrame = lstFrames.GetNext(pos);

		if (CWnd::FromHandlePermanent(pFrame->m_hWnd) != NULL)
		{
			ASSERT_VALID(pFrame);

			pFrame->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME);

			if (pFrame->GetSafeHwnd() == pMainWnd->GetSafeHwnd())
			{
				bIsMainWndRedrawn = FALSE;
			}
		}
	}

	if (!bIsMainWndRedrawn && pMainWnd->GetSafeHwnd() != NULL && CWnd::FromHandlePermanent(pMainWnd->m_hWnd) != NULL)
	{
		pMainWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME);
	}

	for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
	{
		CPane* pToolBar = DYNAMIC_DOWNCAST(CPane, afxAllToolBars.GetNext(posTlb));
		if (pToolBar != NULL)
		{
			if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				pToolBar->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
			}
		}
	}

	CPaneFrameWnd::RedrawAll();
}

void __stdcall CMFCVisualManager::AdjustToolbars()
{
	for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
	{
		CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, afxAllToolBars.GetNext(posTlb));
		if (pToolBar != NULL)
		{
			if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);
				pToolBar->OnChangeVisualManager();
			}
		}
	}
}

void __stdcall CMFCVisualManager::AdjustFrames()
{
	const CList<CFrameWnd*, CFrameWnd*>& lstFrames = CFrameImpl::GetFrameList();

	for (POSITION pos = lstFrames.GetHeadPosition(); pos != NULL;)
	{
		CFrameWnd* pFrame = lstFrames.GetNext(pos);

		if (CWnd::FromHandlePermanent(pFrame->m_hWnd) != NULL)
		{
			ASSERT_VALID(pFrame);
			pFrame->SendMessage(AFX_WM_CHANGEVISUALMANAGER);
		}
	}
}

void CMFCVisualManager::OnDrawBarGripper(CDC* pDC, CRect rectGripper, BOOL bHorz, CBasePane* pBar)
{
	ASSERT_VALID(pDC);

	const COLORREF clrHilite = pBar != NULL && pBar->IsDialogControl() ? afxGlobalData.clrBtnHilite : afxGlobalData.clrBarHilite;
	const COLORREF clrShadow = pBar != NULL && pBar->IsDialogControl() ? afxGlobalData.clrBtnShadow : afxGlobalData.clrBarShadow;

	const int iGripperSize = 3;

	if (bHorz)
	{
		//-----------------
		// Gripper at left:
		//-----------------
		rectGripper.DeflateRect(0, iGripperSize);

		//---------------------
		// Center the grippers:
		//---------------------
		rectGripper.left = rectGripper.CenterPoint().x - iGripperSize / 2;
		rectGripper.right = rectGripper.left + iGripperSize;
	}
	else
	{
		//----------------
		// Gripper at top:
		//----------------
		rectGripper.DeflateRect(iGripperSize, 0);

		//---------------------
		// Center the grippers:
		//---------------------
		rectGripper.top = rectGripper.CenterPoint().y - iGripperSize / 2;
		rectGripper.bottom = rectGripper.top + iGripperSize;
	}

	pDC->Draw3dRect(rectGripper, clrHilite, clrShadow);
}

void CMFCVisualManager::OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL /*bNCArea*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBar);

	if (DYNAMIC_DOWNCAST(CReBar, pBar) != NULL || DYNAMIC_DOWNCAST(CReBar, pBar->GetParent()))
	{
		FillReBarPane(pDC, pBar, rectClient);
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPane)))
	{
		((CMFCOutlookBarPane*) pBar)->OnEraseWorkArea(pDC, rectClient);
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCCaptionBar)))
	{
		CMFCCaptionBar* pCaptionBar = (CMFCCaptionBar*) pBar;

		if (pCaptionBar->IsMessageBarMode())
		{
			pDC->FillRect(rectClip, &afxGlobalData.brBarFace);
		}
		else
		{
			pDC->FillSolidRect(rectClip, pCaptionBar->m_clrBarBackground == -1 ? afxGlobalData.clrBarShadow : pCaptionBar->m_clrBarBackground);
		}
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
	{
		CMFCPopupMenuBar* pMenuBar = (CMFCPopupMenuBar*) pBar;

		if (pMenuBar->IsDropDownListMode())
		{
			pDC->FillRect(rectClip, &afxGlobalData.brWindow);
			return;
		}
	}

	// By default, control bar background is filled by
	// the system 3d background color

	pDC->FillRect(rectClip.IsRectEmpty() ? rectClient : rectClip, pBar->IsDialogControl() ? &afxGlobalData.brBtnFace : &afxGlobalData.brBarFace);
}

void CMFCVisualManager::OnDrawPaneBorder(CDC* pDC, CBasePane* pBar, CRect& rect)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if (pBar->IsFloating())
	{
		return;
	}

	DWORD dwStyle = pBar->GetPaneStyle();
	if (!(dwStyle & CBRS_BORDER_ANY))
		return;

	COLORREF clrBckOld = pDC->GetBkColor(); // FillSolidRect changes it

	const COLORREF clrHilite = pBar->IsDialogControl() ? afxGlobalData.clrBtnHilite : afxGlobalData.clrBarHilite;
	const COLORREF clrShadow = pBar->IsDialogControl() ? afxGlobalData.clrBtnShadow : afxGlobalData.clrBarShadow;

	COLORREF clr = clrHilite;

	if (dwStyle & CBRS_BORDER_LEFT)
		pDC->FillSolidRect(0, 0, 1, rect.Height() - 1, clr);
	if (dwStyle & CBRS_BORDER_TOP)
		pDC->FillSolidRect(0, 0, rect.Width()-1 , 1, clr);
	if (dwStyle & CBRS_BORDER_RIGHT)
		pDC->FillSolidRect(rect.right, 0/*RGL~:1*/, -1,
		rect.Height()/*RGL-: - 1*/, clrShadow);
	if (dwStyle & CBRS_BORDER_BOTTOM)
		pDC->FillSolidRect(0, rect.bottom, rect.Width()-1, -1, clrShadow);

	// if undockable toolbar at top of frame, apply special formatting to mesh
	// properly with frame menu
	if (!pBar->CanFloat())
	{
		pDC->FillSolidRect(0,0,rect.Width(),1,clrShadow);
		pDC->FillSolidRect(0,1,rect.Width(),1,clrHilite);
	}

	if (dwStyle & CBRS_BORDER_LEFT)
		++rect.left;
	if (dwStyle & CBRS_BORDER_TOP)
		++rect.top;
	if (dwStyle & CBRS_BORDER_RIGHT)
		--rect.right;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		--rect.bottom;

	// Restore Bk color:
	pDC->SetBkColor(clrBckOld);
}

void CMFCVisualManager::OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* /*pMenu*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, afxGlobalData.clrBarLight, afxGlobalData.clrBarDkShadow);
	rect.DeflateRect(1, 1);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
}

void CMFCVisualManager::OnDrawMenuShadow(CDC* pPaintDC, const CRect& rectClient, const CRect& /*rectExclude*/,
	int nDepth,  int iMinBrightness, int iMaxBrightness, CBitmap* pBmpSaveBottom,  CBitmap* pBmpSaveRight, BOOL bRTL)
{
	ASSERT_VALID(pPaintDC);
	ASSERT_VALID(pBmpSaveBottom);
	ASSERT_VALID(pBmpSaveRight);

	//------------------------------------------------------
	// Simple draw the shadow, ignore rectExclude parameter:
	//------------------------------------------------------
	CDrawingManager dm(*pPaintDC);
	dm.DrawShadow(rectClient, nDepth, iMinBrightness, iMaxBrightness, pBmpSaveBottom, pBmpSaveRight, (COLORREF)-1, !bRTL);
}

void CMFCVisualManager::OnFillButtonInterior(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCShowAllButton)))
	{
		if (state == ButtonsIsHighlighted)
		{
			CDrawingManager dm(*pDC);
			dm.HighlightRect(rect);
		}

		return;
	}

	if (!m_bEnableToolbarButtonFill)
	{
		BOOL bIsPopupMenu = FALSE;

		CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
		if (pMenuButton != NULL)
		{
			bIsPopupMenu = pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));
		}

		if (!bIsPopupMenu)
		{
			return;
		}
	}

	if (!pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton)) && !CMFCToolBar::IsCustomizeMode() &&
		state != ButtonsIsHighlighted && (pButton->m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE)))
	{
		CRect rectDither = rect;
		rectDither.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);

		CMFCToolBarImages::FillDitheredRect(pDC, rectDither);
	}
}

COLORREF CMFCVisualManager::GetToolbarHighlightColor()
{
	return afxGlobalData.clrBarFace;
}

COLORREF CMFCVisualManager::GetToolbarDisabledTextColor()
{
	return afxGlobalData.clrGrayedText;
}

void CMFCVisualManager::OnHighlightMenuItem(CDC*pDC, CMFCToolBarMenuButton* /*pButton*/, CRect rect, COLORREF& /*clrText*/)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &afxGlobalData.brHilite);
}

COLORREF CMFCVisualManager::GetHighlightedMenuItemTextColor(CMFCToolBarMenuButton* pButton)
{
	ASSERT_VALID(pButton);

	if (pButton->m_nStyle & TBBS_DISABLED)
	{
		return afxGlobalData.clrGrayedText;
	}

	return afxGlobalData.clrTextHilite;
}

void CMFCVisualManager::OnHighlightRarelyUsedMenuItems(CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID(pDC);

	CDrawingManager dm(*pDC);
	dm.HighlightRect(rectRarelyUsed);

	pDC->Draw3dRect(rectRarelyUsed, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
}

void CMFCVisualManager::OnDrawMenuCheck(CDC* pDC, CMFCToolBarMenuButton* pButton, CRect rectCheck, BOOL /*bHighlight*/, BOOL bIsRadio)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	int iImage = bIsRadio ? CMenuImages::IdRadio : CMenuImages::IdCheck;
	CMenuImages::Draw(pDC, (CMenuImages::IMAGES_IDS) iImage, rectCheck, (pButton->m_nStyle & TBBS_DISABLED) ? CMenuImages::ImageGray : CMenuImages::ImageBlack);
}

void CMFCVisualManager::OnDrawMenuItemButton(CDC* pDC, CMFCToolBarMenuButton* /*pButton*/, CRect rectButton, BOOL bHighlight, BOOL /*bDisabled*/)
{
	ASSERT_VALID(pDC);

	CRect rect = rectButton;
	rect.right = rect.left + 1;
	rect.left--;
	rect.DeflateRect(0, bHighlight ? 1 : 4);

	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
}

void CMFCVisualManager::OnDrawButtonBorder(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	BOOL bIsOutlookButton = pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton));
	COLORREF clrDark = bIsOutlookButton ? afxGlobalData.clrBarDkShadow : afxGlobalData.clrBarShadow;

	switch(state)
	{
	case ButtonsIsPressed:
		pDC->Draw3dRect(&rect, clrDark, afxGlobalData.clrBarHilite);
		return;

	case ButtonsIsHighlighted:
		pDC->Draw3dRect(&rect, afxGlobalData.clrBarHilite, clrDark);
		return;
	}
}

void CMFCVisualManager::OnDrawButtonSeparator(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state, BOOL /*bHorz*/)
{
	ASSERT_VALID(pButton);

	if (!m_bMenuFlatLook || !pButton->IsDroppedDown())
	{
		OnDrawButtonBorder(pDC, pButton, rect, state);
	}
}

void CMFCVisualManager::OnDrawSeparator(CDC* pDC, CBasePane* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	CRect rectSeparator = rect;

	if (bHorz)
	{
		rectSeparator.left += rectSeparator.Width() / 2 - 1;
		rectSeparator.right = rectSeparator.left + 2;
	}
	else
	{
		rectSeparator.top += rectSeparator.Height() / 2 - 1;
		rectSeparator.bottom = rectSeparator.top + 2;
	}

	const COLORREF clrHilite = pBar->IsDialogControl() ? afxGlobalData.clrBtnHilite : afxGlobalData.clrBarHilite;
	const COLORREF clrShadow = pBar->IsDialogControl() ? afxGlobalData.clrBtnShadow : afxGlobalData.clrBarShadow;

	pDC->Draw3dRect(rectSeparator, clrShadow, clrHilite);
}

COLORREF CMFCVisualManager::OnDrawMenuLabel(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &afxGlobalData.brBtnFace);

	CRect rectSeparator = rect;
	rectSeparator.top = rectSeparator.bottom - 2;

	pDC->Draw3dRect(rectSeparator, afxGlobalData.clrBtnShadow, afxGlobalData.clrBtnHilite);

	return afxGlobalData.clrBtnText;
}

COLORREF CMFCVisualManager::OnDrawPaneCaption(CDC* pDC, CDockablePane* /*pBar*/, BOOL bActive, CRect rectCaption, CRect /*rectButtons*/)
{
	ASSERT_VALID(pDC);

	CBrush br(bActive ? afxGlobalData.clrActiveCaption : afxGlobalData.clrInactiveCaption);
	pDC->FillRect(rectCaption, &br);

	// get the text color
	return bActive ? afxGlobalData.clrCaptionText : afxGlobalData.clrInactiveCaptionText;
}

void CMFCVisualManager::OnDrawCaptionButton(CDC* pDC, CMFCCaptionButton* pButton, BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled, int nImageID /*= -1*/)
{
	ASSERT_VALID(pDC);
	CRect rc = pButton->GetRect();

	CMenuImages::IMAGES_IDS id = (CMenuImages::IMAGES_IDS)-1;

	if (nImageID != -1)
	{
		id = (CMenuImages::IMAGES_IDS)nImageID;
	}
	else
	{
		id = pButton->GetIconID(bHorz, bMaximized);
	}

	CRect rectImage = rc;

	if (pButton->m_bPushed &&(pButton->m_bFocused || pButton->m_bDroppedDown))
	{
		rectImage.OffsetRect(1, 1);
	}

	CMenuImages::IMAGE_STATE imageState;

	if (bDisabled)
	{
		imageState = CMenuImages::ImageGray;
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

	CMenuImages::Draw(pDC, id, rectImage, imageState);

	if (!bDisabled)
	{
		if (pButton->m_bPushed &&(pButton->m_bFocused || pButton->m_bDroppedDown))
		{
			pDC->Draw3dRect(rc, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarLight);
			rc.DeflateRect(1, 1);
			pDC->Draw3dRect(rc, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarHilite);
		}
		else if (/*!m_bLook2000*/FALSE)
		{
			pDC->Draw3dRect(rc, afxGlobalData.clrBarLight, afxGlobalData.clrBarDkShadow);
			rc.DeflateRect(1, 1);
			pDC->Draw3dRect(rc, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
		}
		else if (pButton->m_bFocused || pButton->m_bPushed || pButton->m_bDroppedDown)
		{
			pDC->Draw3dRect(rc, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
		}
	}
}

void CMFCVisualManager::OnEraseTabsArea(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* /*pTabWnd*/)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &afxGlobalData.brBarFace);
}

void CMFCVisualManager::OnDrawTab(CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);
	ASSERT_VALID(pDC);

	COLORREF clrTab = pTabWnd->GetTabBkColor(iTab);

	CRect rectClip;
	pDC->GetClipBox(rectClip);

	if (pTabWnd->IsFlatTab())
	{
		//----------------
		// Draw tab edges:
		//----------------
#define AFX_FLAT_POINTS_NUM 4
		POINT pts [AFX_FLAT_POINTS_NUM];

		const int nHalfHeight = pTabWnd->GetTabsHeight() / 2;

		if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
		{
			rectTab.bottom --;

			pts [0].x = rectTab.left;
			pts [0].y = rectTab.top;

			pts [1].x = rectTab.left + nHalfHeight;
			pts [1].y = rectTab.bottom;

			pts [2].x = rectTab.right - nHalfHeight;
			pts [2].y = rectTab.bottom;

			pts [3].x = rectTab.right;
			pts [3].y = rectTab.top;
		}
		else
		{
			rectTab.top ++;

			pts [0].x = rectTab.left + nHalfHeight;
			pts [0].y = rectTab.top;

			pts [1].x = rectTab.left;
			pts [1].y = rectTab.bottom;

			pts [2].x = rectTab.right;
			pts [2].y = rectTab.bottom;

			pts [3].x = rectTab.right - nHalfHeight;
			pts [3].y = rectTab.top;

			rectTab.left += 2;
		}

		CBrush* pOldBrush = NULL;
		CBrush br(clrTab);

		if (!bIsActive && clrTab != (COLORREF)-1)
		{
			pOldBrush = pDC->SelectObject(&br);
		}

		pDC->Polygon(pts, AFX_FLAT_POINTS_NUM);

		if (pOldBrush != NULL)
		{
			pDC->SelectObject(pOldBrush);
		}
	}
	else if (pTabWnd->IsLeftRightRounded())
	{
		CList<POINT, POINT> pts;

		POSITION posLeft = pts.AddHead(CPoint(rectTab.left, rectTab.top));
		posLeft = pts.InsertAfter(posLeft, CPoint(rectTab.left, rectTab.top + 2));

		POSITION posRight = pts.AddTail(CPoint(rectTab.right, rectTab.top));
		posRight = pts.InsertBefore(posRight, CPoint(rectTab.right, rectTab.top + 2));

		int xLeft = rectTab.left + 1;
		int xRight = rectTab.right - 1;

		int y = 0;

		for (y = rectTab.top + 2; y < rectTab.bottom - 4; y += 2)
		{
			posLeft = pts.InsertAfter(posLeft, CPoint(xLeft, y));
			posLeft = pts.InsertAfter(posLeft, CPoint(xLeft, y + 2));

			posRight = pts.InsertBefore(posRight, CPoint(xRight, y));
			posRight = pts.InsertBefore(posRight, CPoint(xRight, y + 2));

			xLeft++;
			xRight--;
		}

		if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_TOP)
		{
			xLeft--;
			xRight++;
		}

		const int nTabLeft = xLeft - 1;
		const int nTabRight = xRight + 1;

		for (;y < rectTab.bottom - 1; y++)
		{
			posLeft = pts.InsertAfter(posLeft, CPoint(xLeft, y));
			posLeft = pts.InsertAfter(posLeft, CPoint(xLeft + 1, y + 1));

			posRight = pts.InsertBefore(posRight, CPoint(xRight, y));
			posRight = pts.InsertBefore(posRight, CPoint(xRight - 1, y + 1));

			if (y == rectTab.bottom - 2)
			{
				posLeft = pts.InsertAfter(posLeft, CPoint(xLeft + 1, y + 1));
				posLeft = pts.InsertAfter(posLeft, CPoint(xLeft + 3, y + 1));

				posRight = pts.InsertBefore(posRight, CPoint(xRight, y + 1));
				posRight = pts.InsertBefore(posRight, CPoint(xRight - 2, y + 1));
			}

			xLeft++;
			xRight--;
		}

		posLeft = pts.InsertAfter(posLeft, CPoint(xLeft + 2, rectTab.bottom));
		posRight = pts.InsertBefore(posRight, CPoint(xRight - 2, rectTab.bottom));

		LPPOINT points = new POINT [pts.GetCount()];

		int i = 0;

		for (POSITION pos = pts.GetHeadPosition(); pos != NULL; i++)
		{
			points [i] = pts.GetNext(pos);

			if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_TOP)
			{
				points [i].y = rectTab.bottom -(points [i].y - rectTab.top);
			}
		}

		CRgn rgnClip;
		rgnClip.CreatePolygonRgn(points, (int) pts.GetCount(), WINDING);

		pDC->SelectClipRgn(&rgnClip);

		CBrush br(clrTab == (COLORREF)-1 ? afxGlobalData.clrBtnFace : clrTab);
		OnFillTab(pDC, rectTab, &br, iTab, bIsActive, pTabWnd);

		pDC->SelectClipRgn(NULL);

		CPen pen(PS_SOLID, 1, afxGlobalData.clrBarShadow);
		CPen* pOLdPen = pDC->SelectObject(&pen);

		for (i = 0; i < pts.GetCount(); i++)
		{
			if ((i % 2) != 0)
			{
				int x1 = points [i - 1].x;
				int y1 = points [i - 1].y;

				int x2 = points [i].x;
				int y2 = points [i].y;

				if (x1 > rectTab.CenterPoint().x && x2 > rectTab.CenterPoint().x)
				{
					x1--;
					x2--;
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

		delete [] points;
		pDC->SelectObject(pOLdPen);

		rectTab.left = nTabLeft;
		rectTab.right = nTabRight;
	}
	else // 3D Tab
	{
		CRgn rgnClip;

		CRect rectClipTab;
		pTabWnd->GetTabsRect(rectClipTab);

		BOOL bIsCutted = FALSE;

		const BOOL bIsOneNote = pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style();
		const int nExtra = bIsOneNote ? ((iTab == 0 || bIsActive || pTabWnd->IsVS2005Style()) ? 0 : rectTab.Height()) : 0;

		if (rectTab.left + nExtra + 10 > rectClipTab.right || rectTab.right - 10 <= rectClipTab.left)
		{
			return;
		}

		const int iVertOffset = 2;
		const int iHorzOffset = 2;
		const BOOL bIs2005 = pTabWnd->IsVS2005Style();

#define AFX_POINTS_NUM 8
		POINT pts [AFX_POINTS_NUM];

		if (!bIsActive || bIsOneNote || clrTab != (COLORREF)-1 || m_bAlwaysFillTab)
		{
			if (clrTab != (COLORREF)-1 || bIsOneNote || m_bAlwaysFillTab)
			{
				CRgn rgn;
				CBrush br(clrTab == (COLORREF)-1 ? afxGlobalData.clrBtnFace : clrTab);

				CRect rectFill = rectTab;

				if (bIsOneNote)
				{
					CRect rectFillTab = rectTab;

					const int nHeight = rectFillTab.Height();

					pts [0].x = rectFillTab.left;
					pts [0].y = rectFillTab.bottom;

					pts [1].x = rectFillTab.left;
					pts [1].y = rectFillTab.bottom;

					pts [2].x = rectFillTab.left + 2;
					pts [2].y = rectFillTab.bottom;

					pts [3].x = rectFillTab.left + nHeight;
					pts [3].y = rectFillTab.top + 2;

					pts [4].x = rectFillTab.left + nHeight + 4;
					pts [4].y = rectFillTab.top;

					pts [5].x = rectFillTab.right - 2;
					pts [5].y = rectFillTab.top;

					pts [6].x = rectFillTab.right;
					pts [6].y = rectFillTab.top + 2;

					pts [7].x = rectFillTab.right;
					pts [7].y = rectFillTab.bottom;

					for (int i = 0; i < AFX_POINTS_NUM; i++)
					{
						if (pts [i].x > rectClipTab.right)
						{
							pts [i].x = rectClipTab.right;
							bIsCutted = TRUE;
						}

						if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
						{
							pts [i].y = rectFillTab.bottom - pts [i].y + rectFillTab.top - 1;
						}
					}

					rgn.CreatePolygonRgn(pts, AFX_POINTS_NUM, WINDING);
					pDC->SelectClipRgn(&rgn);
				}
				else
				{
					rectFill.DeflateRect(1, 0);

					if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
					{
						rectFill.bottom--;
					}
					else
					{
						rectFill.top++;
					}

					rectFill.right = min(rectFill.right, rectClipTab.right);
				}

				OnFillTab(pDC, rectFill, &br, iTab, bIsActive, pTabWnd);
				pDC->SelectClipRgn(NULL);

				if (bIsOneNote)
				{
					CRect rectLeft;
					pTabWnd->GetClientRect(rectLeft);
					rectLeft.right = rectClipTab.left - 1;

					pDC->ExcludeClipRect(rectLeft);

					if (iTab > 0 && !bIsActive && iTab != pTabWnd->GetFirstVisibleTabNum())
					{
						CRect rectLeftTab = rectClipTab;
						rectLeftTab.right = rectFill.left + rectFill.Height() - 10;

						const int nVertOffset = bIs2005 ? 2 : 1;

						if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
						{
							rectLeftTab.top -= nVertOffset;
						}
						else
						{
							rectLeftTab.bottom += nVertOffset;
						}

						pDC->ExcludeClipRect(rectLeftTab);
					}

					pDC->Polyline(pts, AFX_POINTS_NUM);

					if (bIsCutted)
					{
						pDC->MoveTo(rectClipTab.right, rectTab.top);
						pDC->LineTo(rectClipTab.right, rectTab.bottom);
					}

					CRect rectRight = rectClipTab;
					rectRight.left = rectFill.right;

					pDC->ExcludeClipRect(rectRight);
				}
			}
		}

		CPen penLight(PS_SOLID, 1, afxGlobalData.clrBarHilite);
		CPen penShadow(PS_SOLID, 1, afxGlobalData.clrBarShadow);
		CPen penDark(PS_SOLID, 1, afxGlobalData.clrBarDkShadow);

		CPen* pOldPen = NULL;

		if (bIsOneNote)
		{
			pOldPen = (CPen*) pDC->SelectObject(&penLight);
			ENSURE(pOldPen != NULL);

			if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
			{
				if (!bIsCutted)
				{
					int yTop = bIsActive ? pts [7].y - 1 : pts [7].y;

					pDC->MoveTo(pts [6].x - 1, pts [6].y);
					pDC->LineTo(pts [7].x - 1, yTop);
				}
			}
			else
			{
				pDC->MoveTo(pts [2].x + 1, pts [2].y);
				pDC->LineTo(pts [3].x + 1, pts [3].y);

				pDC->MoveTo(pts [3].x + 1, pts [3].y);
				pDC->LineTo(pts [3].x + 2, pts [3].y);

				pDC->MoveTo(pts [3].x + 2, pts [3].y);
				pDC->LineTo(pts [3].x + 3, pts [3].y);

				pDC->MoveTo(pts [4].x - 1, pts [4].y + 1);
				pDC->LineTo(pts [5].x + 1, pts [5].y + 1);

				if (!bIsActive && !bIsCutted && m_b3DTabWideBorder)
				{
					pDC->SelectObject(&penShadow);

					pDC->MoveTo(pts [6].x - 2, pts [6].y - 1);
					pDC->LineTo(pts [6].x - 1, pts [6].y - 1);
				}

				pDC->MoveTo(pts [6].x - 1, pts [6].y);
				pDC->LineTo(pts [7].x - 1, pts [7].y);
			}
		}
		else
		{
			if (rectTab.right > rectClipTab.right)
			{
				CRect rectTabClip = rectTab;
				rectTabClip.right = rectClipTab.right;

				rgnClip.CreateRectRgnIndirect(&rectTabClip);
				pDC->SelectClipRgn(&rgnClip);
			}

			if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
			{
				pOldPen = (CPen*) pDC->SelectObject(&penLight);
				ENSURE(pOldPen != NULL);

				if (!m_b3DTabWideBorder)
				{
					pDC->SelectObject(&penShadow);
				}

				pDC->MoveTo(rectTab.left, rectTab.top);
				pDC->LineTo(rectTab.left, rectTab.bottom - iVertOffset);

				if (m_b3DTabWideBorder)
				{
					pDC->SelectObject(&penDark);
				}

				pDC->LineTo(rectTab.left + iHorzOffset, rectTab.bottom);
				pDC->LineTo(rectTab.right - iHorzOffset, rectTab.bottom);
				pDC->LineTo(rectTab.right, rectTab.bottom - iVertOffset);
				pDC->LineTo(rectTab.right, rectTab.top - 1);

				pDC->SelectObject(&penShadow);

				if (m_b3DTabWideBorder)
				{
					pDC->MoveTo(rectTab.left + iHorzOffset + 1, rectTab.bottom - 1);
					pDC->LineTo(rectTab.right - iHorzOffset, rectTab.bottom - 1);
					pDC->LineTo(rectTab.right - 1, rectTab.bottom - iVertOffset);
					pDC->LineTo(rectTab.right - 1, rectTab.top - 1);
				}
			}
			else
			{
				pOldPen = pDC->SelectObject(m_b3DTabWideBorder ? &penDark : &penShadow);

				ENSURE(pOldPen != NULL);

				pDC->MoveTo(rectTab.right, bIsActive ? rectTab.bottom : rectTab.bottom - 1);
				pDC->LineTo(rectTab.right, rectTab.top + iVertOffset);
				pDC->LineTo(rectTab.right - iHorzOffset, rectTab.top);

				if (m_b3DTabWideBorder)
				{
					pDC->SelectObject(&penLight);
				}

				pDC->LineTo(rectTab.left + iHorzOffset, rectTab.top);
				pDC->LineTo(rectTab.left, rectTab.top + iVertOffset);

				pDC->LineTo(rectTab.left, rectTab.bottom);

				if (m_b3DTabWideBorder)
				{
					pDC->SelectObject(&penShadow);

					pDC->MoveTo(rectTab.right - 1, bIsActive ? rectTab.bottom : rectTab.bottom - 1);
					pDC->LineTo(rectTab.right - 1, rectTab.top + iVertOffset - 1);
				}
			}
		}

		if (bIsActive)
		{
			const int iBarHeight = 1;
			const int y = (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM) ? (rectTab.top - iBarHeight - 1) :(rectTab.bottom);

			CRect rectFill(CPoint(rectTab.left, y), CSize(rectTab.Width(), iBarHeight + 1));

			COLORREF clrActiveTab = pTabWnd->GetTabBkColor(iTab);

			if (bIsOneNote)
			{
				if (bIs2005)
				{
					rectFill.left += 3;
				}
				else
				{
					rectFill.OffsetRect(1, 0);
					rectFill.left++;
				}

				if (clrActiveTab == (COLORREF)-1)
				{
					clrActiveTab = afxGlobalData.clrWindow;
				}
			}

			if (clrActiveTab != (COLORREF)-1)
			{
				CBrush br(clrActiveTab);
				pDC->FillRect(rectFill, &br);
			}
			else
			{
				pDC->FillRect(rectFill, &afxGlobalData.brBarFace);
			}
		}

		pDC->SelectObject(pOldPen);

		if (bIsOneNote)
		{
			const int nLeftMargin = pTabWnd->IsVS2005Style() && bIsActive ? rectTab.Height() * 3 / 4 : rectTab.Height();
			const int nRightMargin = pTabWnd->IsVS2005Style() && bIsActive ? CMFCBaseTabCtrl::AFX_TAB_IMAGE_MARGIN * 3 / 4 : CMFCBaseTabCtrl::AFX_TAB_IMAGE_MARGIN;

			rectTab.left += nLeftMargin;
			rectTab.right -= nRightMargin;

			if (pTabWnd->IsVS2005Style() && bIsActive && pTabWnd->HasImage(iTab))
			{
				rectTab.OffsetRect(CMFCBaseTabCtrl::AFX_TAB_IMAGE_MARGIN, 0);
			}
		}

		pDC->SelectClipRgn(NULL);
	}

	COLORREF clrText = pTabWnd->GetTabTextColor(iTab);

	COLORREF clrTextOld = (COLORREF)-1;
	if (!bIsActive && clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor(clrText);
	}

	if (pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style())
	{
		CRect rectClipTab;
		pTabWnd->GetTabsRect(rectClipTab);

		rectTab.right = min(rectTab.right, rectClipTab.right - 2);
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect(rectClip);

	pDC->SelectClipRgn(&rgn);

	OnDrawTabContent(pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}

	pDC->SelectClipRgn(NULL);
}

void CMFCVisualManager::OnFillTab(CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pbrFill);
	ASSERT_VALID(pTabWnd);

	if (bIsActive && !afxGlobalData.IsHighContrastMode() && (pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded()) &&
		pTabWnd->GetTabBkColor(iTab) == (COLORREF)-1)
	{
		pDC->FillRect(rectFill, &afxGlobalData.brWindow);
	}
	else
	{
		pDC->FillRect(rectFill, pbrFill);
	}
}

BOOL CMFCVisualManager::OnEraseTabsFrame(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);
	ASSERT_VALID(pDC);

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor(pTabWnd->GetActiveTab());

	if (clrActiveTab == (COLORREF)-1)
	{
		return FALSE;
	}

	pDC->FillSolidRect(rect, clrActiveTab);
	return TRUE;
}

void CMFCVisualManager::OnDrawTabContent(CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd, COLORREF clrText)
{
	ASSERT_VALID(pTabWnd);
	ASSERT_VALID(pDC);

	if (pTabWnd->IsActiveTabCloseButton() && bIsActive)
	{
		CRect rectClose = pTabWnd->GetTabCloseButton();
		rectTab.right = rectClose.left;

		OnDrawTabCloseButton(pDC, rectClose, pTabWnd, pTabWnd->IsTabCloseButtonHighlighted(), pTabWnd->IsTabCloseButtonPressed(), FALSE /* Disabled */);
	}

	CString strText;
	pTabWnd->GetTabLabel(iTab, strText);

	if (pTabWnd->IsFlatTab())
	{
		//---------------
		// Draw tab text:
		//---------------
		UINT nFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
		if (pTabWnd->IsDrawNoPrefix())
		{
			nFormat |= DT_NOPREFIX;
		}

		pDC->DrawText(strText, rectTab, nFormat);
	}
	else
	{
		CSize sizeImage = pTabWnd->GetImageSize();
		UINT uiIcon = pTabWnd->GetTabIcon(iTab);
		HICON hIcon = pTabWnd->GetTabHicon(iTab);

		if (uiIcon == (UINT)-1 && hIcon == NULL)
		{
			sizeImage.cx = 0;
		}

		if (sizeImage.cx + 2 * CMFCBaseTabCtrl::AFX_TAB_IMAGE_MARGIN <= rectTab.Width())
		{
			if (hIcon != NULL)
			{
				//---------------------
				// Draw the tab's icon:
				//---------------------
				CRect rectImage = rectTab;

				rectImage.top += (rectTab.Height() - sizeImage.cy) / 2;
				rectImage.bottom = rectImage.top + sizeImage.cy;

				rectImage.left += AFX_IMAGE_MARGIN;
				rectImage.right = rectImage.left + sizeImage.cx;

				pDC->DrawState(rectImage.TopLeft(), rectImage.Size(), hIcon, DSS_NORMAL, (HBRUSH) NULL);
			}
			else
			{
				const CImageList* pImageList = pTabWnd->GetImageList();
				if (pImageList != NULL && uiIcon != (UINT)-1)
				{
					//----------------------
					// Draw the tab's image:
					//----------------------
					CRect rectImage = rectTab;

					rectImage.top += (rectTab.Height() - sizeImage.cy) / 2;
					rectImage.bottom = rectImage.top + sizeImage.cy;

					rectImage.left += AFX_IMAGE_MARGIN;
					rectImage.right = rectImage.left + sizeImage.cx;

					ASSERT_VALID(pImageList);
					((CImageList*) pImageList)->Draw(pDC, uiIcon, rectImage.TopLeft(), ILD_TRANSPARENT);
				}
			}

			//------------------------------
			// Finally, draw the tab's text:
			//------------------------------
			CRect rcText = rectTab;
			rcText.left += sizeImage.cx + 2 * AFX_TEXT_MARGIN;

			if (rcText.Width() < sizeImage.cx * 2 && !pTabWnd->IsLeftRightRounded())
			{
				rcText.right -= AFX_TEXT_MARGIN;
			}

			if (clrText == (COLORREF)-1)
			{
				clrText = GetTabTextColor(pTabWnd, iTab, bIsActive);
			}

			if (clrText != (COLORREF)-1)
			{
				pDC->SetTextColor(clrText);
			}

			UINT nFormat = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
			if (pTabWnd->IsDrawNoPrefix())
			{
				nFormat |= DT_NOPREFIX;
			}

			if (pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style())
			{
				nFormat |= DT_CENTER;
			}
			else
			{
				nFormat |= DT_LEFT;
			}

			pDC->DrawText(strText, rcText, nFormat);
		}
	}
}

void CMFCVisualManager::OnDrawTabCloseButton(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* /*pTabWnd*/, BOOL bIsHighlighted, BOOL bIsPressed, BOOL /*bIsDisabled*/)
{
	if (bIsHighlighted)
	{
		pDC->FillRect(rect, &afxGlobalData.brBarFace);
	}

	CMenuImages::Draw(pDC, CMenuImages::IdClose, rect, CMenuImages::ImageBlack);

	if (bIsHighlighted)
	{
		if (bIsPressed)
		{
			pDC->Draw3dRect(rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarHilite);
		}
		else
		{
			pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarDkShadow);
		}
	}
}

void CMFCVisualManager::OnEraseTabsButton(CDC* pDC, CRect rect, CMFCButton* /*pButton*/, CMFCBaseTabCtrl* /*pWndTab*/)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &afxGlobalData.brBarFace);
}

void CMFCVisualManager::OnDrawTabsButtonBorder(CDC* pDC, CRect& rect, CMFCButton* pButton, UINT uiState, CMFCBaseTabCtrl* /*pWndTab*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsPressed() ||(uiState & ODS_SELECTED))
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarHilite);

		rect.left += 2;
		rect.top += 2;
	}
	else
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarDkShadow);
	}

	rect.DeflateRect(2, 2);
}

COLORREF CMFCVisualManager::OnFillCommandsListBackground(CDC* pDC, CRect rect, BOOL bIsSelected)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (bIsSelected)
	{
		pDC->FillRect(rect, &afxGlobalData.brHilite);

		const int nFrameSize = 1;

		rect.DeflateRect(1, 1);
		rect.right--;
		rect.bottom--;

		pDC->PatBlt(rect.left, rect.top + nFrameSize, nFrameSize, rect.Height(), PATINVERT);
		pDC->PatBlt(rect.left, rect.top, rect.Width(), nFrameSize, PATINVERT);
		pDC->PatBlt(rect.right, rect.top, nFrameSize, rect.Height(), PATINVERT);
		pDC->PatBlt(rect.left + nFrameSize, rect.bottom, rect.Width(), nFrameSize, PATINVERT);

		return afxGlobalData.clrTextHilite;
	}

	pDC->FillRect(rect, &afxGlobalData.brBarFace);

	return afxGlobalData.clrBarText;
}

void CMFCVisualManager::OnDrawMenuArrowOnCustomizeList(CDC* pDC, CRect rectCommand, BOOL bSelected)
{
	CRect rectTriangle = rectCommand;
	rectTriangle.left = rectTriangle.right - CMenuImages::Size().cx;

	CMenuImages::Draw(pDC, CMenuImages::IdArrowRightLarge, rectTriangle, bSelected ? CMenuImages::ImageWhite : CMenuImages::ImageBlack);

	CRect rectLine = rectCommand;
	rectLine.right = rectTriangle.left - 1;
	rectLine.left = rectLine.right - 2;
	rectLine.DeflateRect(0, 2);

	pDC->Draw3dRect(&rectLine, afxGlobalData.clrBtnShadow, afxGlobalData.clrBtnHilite);
}

CMFCVisualManager* __stdcall CMFCVisualManager::CreateVisualManager(CRuntimeClass* pVisualManager)
{
	if (pVisualManager == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CMFCVisualManager* pVisManagerOld = m_pVisManager;

	CObject* pObj = pVisualManager->CreateObject();
	if (pObj == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	ASSERT_VALID(pObj);

	if (pVisManagerOld != NULL)
	{
		ASSERT_VALID(pVisManagerOld);
		delete pVisManagerOld;
	}

	m_pVisManager = (CMFCVisualManager*) pObj;
	m_pVisManager->m_bAutoDestroy = TRUE;

	return m_pVisManager;
}

void __stdcall CMFCVisualManager::DestroyInstance(BOOL bAutoDestroyOnly)
{
	if (m_pVisManager == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pVisManager);

	if (bAutoDestroyOnly && !m_pVisManager->m_bAutoDestroy)
	{
		return;
	}

	delete m_pVisManager;
	m_pVisManager = NULL;
}

void CMFCVisualManager::OnDrawTearOffCaption(CDC* pDC, CRect rect, BOOL bIsActive)
{
	const int nBorderSize = 2;

	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &afxGlobalData.brBarFace);

	rect.DeflateRect(nBorderSize, 1);

	pDC->FillSolidRect(rect, bIsActive ? afxGlobalData.clrActiveCaption : afxGlobalData.clrInactiveCaption);
}

void CMFCVisualManager::OnDrawMenuResizeBar(CDC* pDC, CRect rect, int /*nResizeFlags*/)
{
	ASSERT_VALID(pDC);

	pDC->FillSolidRect(rect, afxGlobalData.clrInactiveCaption);
}

void CMFCVisualManager::OnDrawMenuScrollButton(CDC* pDC, CRect rect, BOOL bIsScrollDown, BOOL bIsHighlited, BOOL /*bIsPressed*/, BOOL /*bIsDisabled*/)
{
	ASSERT_VALID(pDC);

	CRect rectFill = rect;
	rectFill.top -= 2;

	pDC->FillRect(rectFill, &afxGlobalData.brBarFace);

	CMenuImages::Draw(pDC, bIsScrollDown ? CMenuImages::IdArrowDown : CMenuImages::IdArrowUp, rect);

	if (bIsHighlited)
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
	}
}

void CMFCVisualManager::OnDrawMenuSystemButton(CDC* pDC, CRect rect, UINT uiSystemCommand, UINT nStyle, BOOL /*bHighlight*/)
{
	ASSERT_VALID(pDC);

	UINT uiState = 0;

	switch(uiSystemCommand)
	{
	case SC_CLOSE:
		uiState |= DFCS_CAPTIONCLOSE;
		break;

	case SC_MINIMIZE:
		uiState |= DFCS_CAPTIONMIN;
		break;

	case SC_RESTORE:
		uiState |= DFCS_CAPTIONRESTORE;
		break;

	default:
		return;
	}

	if (nStyle & TBBS_PRESSED)
	{
		uiState |= DFCS_PUSHED;
	}

	if (nStyle & TBBS_DISABLED) // Jan Vasina: Add support for disabled buttons
	{
		uiState |= DFCS_INACTIVE;
	}

	pDC->DrawFrameControl(rect, DFC_CAPTION, uiState);
}

void CMFCVisualManager::OnDrawComboDropButton(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* /*pButton*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	COLORREF clrText = pDC->GetTextColor();

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawRect(rect, afxGlobalData.clrBarFace, afxGlobalData.clrBarHilite);

		if (bIsDropped)
		{
			rect.OffsetRect(1, 1);
			dm.DrawRect(rect, (COLORREF)-1, afxGlobalData.clrBarShadow);
		}
		else if (bIsHighlighted)
		{
			dm.DrawRect(rect, (COLORREF)-1, afxGlobalData.clrBarShadow);
		}
	}
	else
	{
		pDC->FillRect(rect, &afxGlobalData.brBarFace);
		pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarHilite);

		if (bIsDropped)
		{
			rect.OffsetRect(1, 1);
			pDC->Draw3dRect(&rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
		}
		else if (bIsHighlighted)
		{
			pDC->Draw3dRect(&rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
		}
	}

	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rect, bDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack);

	pDC->SetTextColor(clrText);
}

void CMFCVisualManager::OnDrawComboBorder(CDC* pDC, CRect rect, BOOL /*bDisabled*/, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* /*pButton*/)
{
	ASSERT_VALID(pDC);

	if (bIsHighlighted || bIsDropped)
	{
		if (m_bMenuFlatLook)
		{
			CRect rectBorder = rect;
			rectBorder.DeflateRect(1, 1);

			pDC->Draw3dRect(&rectBorder, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarDkShadow);
		}
		else
		{
			pDC->Draw3dRect(&rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
		}
	}
}

void CMFCVisualManager::OnDrawStatusBarPaneBorder(CDC* pDC, CMFCStatusBar* /*pBar*/, CRect rectPane, UINT /*uiID*/, UINT nStyle)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	if (!(nStyle & SBPS_NOBORDERS))
	{
		// draw the borders
		COLORREF clrHilite;
		COLORREF clrShadow;

		if (nStyle & SBPS_POPOUT)
		{
			// reverse colors
			clrHilite = afxGlobalData.clrBarShadow;
			clrShadow = afxGlobalData.clrBarHilite;
		}
		else
		{
			// normal colors
			clrHilite = afxGlobalData.clrBarHilite;
			clrShadow = afxGlobalData.clrBarShadow;
		}

		pDC->Draw3dRect(rectPane, clrShadow, clrHilite);
	}
}

COLORREF CMFCVisualManager::OnFillMiniFrameCaption(CDC* pDC, CRect rectCaption, CPaneFrameWnd* pFrameWnd, BOOL bActive)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pFrameWnd);

	if (DYNAMIC_DOWNCAST(CMFCBaseToolBar, pFrameWnd->GetPane()) != NULL)
	{
		bActive = TRUE;
	}

	CBrush br(bActive ? afxGlobalData.clrActiveCaption : afxGlobalData.clrInactiveCaption);
	pDC->FillRect(rectCaption, &br);

	// get the text color
	return bActive ? afxGlobalData.clrCaptionText : afxGlobalData.clrInactiveCaptionText;
}

void CMFCVisualManager::OnDrawMiniFrameBorder(CDC* pDC, CPaneFrameWnd* pFrameWnd, CRect rectBorder, CRect rectBorderSize)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pFrameWnd);

	AFX_PREDOCK_STATE preDockState = pFrameWnd->GetPreDockState();

	if (preDockState == PDS_DOCK_REGULAR)
	{
		// draw outer edge;
		pDC->Draw3dRect(rectBorder, RGB(127, 0, 0), afxGlobalData.clrBarDkShadow);
		rectBorder.DeflateRect(1, 1);
		pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarHilite, RGB(127, 0, 0));
	}
	else if (preDockState == PDS_DOCK_TO_TAB)
	{
		// draw outer edge;
		pDC->Draw3dRect(rectBorder, RGB(0, 0, 127), afxGlobalData.clrBarDkShadow);
		rectBorder.DeflateRect(1, 1);
		pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarHilite, RGB(0, 0, 127));
	}
	else
	{
		// draw outer edge;
		pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarFace, afxGlobalData.clrBarDkShadow);
		rectBorder.DeflateRect(1, 1);
		pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
	}

	// draw the inner egde
	rectBorder.DeflateRect(rectBorderSize.right - 2, rectBorderSize.top - 2);
	pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarFace, afxGlobalData.clrBarFace);
	rectBorder.InflateRect(1, 1);
	pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarFace, afxGlobalData.clrBarFace);
}

void CMFCVisualManager::OnDrawFloatingToolbarBorder(CDC* pDC, CMFCBaseToolBar* /*pToolBar*/, CRect rectBorder, CRect /*rectBorderSize*/)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarFace, afxGlobalData.clrBarDkShadow);
	rectBorder.DeflateRect(1, 1);
	pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
	rectBorder.DeflateRect(1, 1);
	pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarFace, afxGlobalData.clrBarFace);
}

COLORREF CMFCVisualManager::GetToolbarButtonTextColor(CMFCToolBarButton* pButton, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pButton);

	BOOL bDisabled = (CMFCToolBar::IsCustomizeMode() && !pButton->IsEditable()) || (!CMFCToolBar::IsCustomizeMode() &&(pButton->m_nStyle & TBBS_DISABLED));

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton)))
	{
		if (afxGlobalData.IsHighContrastMode())
		{
			return bDisabled ? afxGlobalData.clrGrayedText : afxGlobalData.clrWindowText;
		}

		return bDisabled ? afxGlobalData.clrBtnFace : afxGlobalData.clrWindow;
	}

	return(bDisabled ? afxGlobalData.clrGrayedText : (state == ButtonsIsHighlighted) ? CMFCToolBar::GetHotTextColor() : afxGlobalData.clrBarText);
}

void CMFCVisualManager::OnFillOutlookPageButton(CDC* pDC, const CRect& rect, BOOL /*bIsHighlighted*/, BOOL /*bIsPressed*/, COLORREF& clrText)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &afxGlobalData.brBarFace);
	clrText = afxGlobalData.clrBarText;
}

void CMFCVisualManager::OnDrawOutlookPageButtonBorder(CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	ASSERT_VALID(pDC);

	if (bIsHighlighted && bIsPressed)
	{
		pDC->Draw3dRect(rectBtn, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarFace);
		rectBtn.DeflateRect(1, 1);
		pDC->Draw3dRect(rectBtn, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
	}
	else
	{
		if (bIsHighlighted || bIsPressed)
		{
			pDC->Draw3dRect(rectBtn, afxGlobalData.clrBarFace, afxGlobalData.clrBarDkShadow);
			rectBtn.DeflateRect(1, 1);
		}

		pDC->Draw3dRect(rectBtn, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
	}

	rectBtn.DeflateRect(1, 1);
}

COLORREF CMFCVisualManager::GetCaptionBarTextColor(CMFCCaptionBar* pBar)
{
	ASSERT_VALID(pBar);

	return pBar->IsMessageBarMode() ? ::GetSysColor(COLOR_INFOTEXT) : afxGlobalData.clrWindow;
}

void CMFCVisualManager::OnDrawCaptionBarBorder(CDC* pDC, CMFCCaptionBar* /*pBar*/, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder)
{
	ASSERT_VALID(pDC);

	if (clrBarBorder == (COLORREF) -1)
	{
		pDC->FillRect(rect, &afxGlobalData.brBarFace);
	}
	else
	{
		CBrush brBorder;
		brBorder.CreateSolidBrush(clrBarBorder);
		pDC->FillRect(rect, &brBorder);
	}

	if (!bFlatBorder)
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
	}
}

void CMFCVisualManager::OnDrawCaptionBarInfoArea(CDC* pDC, CMFCCaptionBar* /*pBar*/, CRect rect)
{
	ASSERT_VALID(pDC);

	::FillRect(pDC->GetSafeHdc(), rect, ::GetSysColorBrush(COLOR_INFOBK));

	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
	rect.DeflateRect(1, 1);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
}

COLORREF CMFCVisualManager::OnFillCaptionBarButton(CDC* pDC, CMFCCaptionBar* pBar, CRect rect,
	BOOL bIsPressed, BOOL bIsHighlighted, BOOL bIsDisabled, BOOL bHasDropDownArrow, BOOL bIsSysButton)
{
	UNREFERENCED_PARAMETER(bIsPressed);
	UNREFERENCED_PARAMETER(bIsHighlighted);
	UNREFERENCED_PARAMETER(bIsDisabled);
	UNREFERENCED_PARAMETER(bHasDropDownArrow);
	UNREFERENCED_PARAMETER(bIsSysButton);

	ASSERT_VALID(pBar);

	if (!pBar->IsMessageBarMode())
	{
		return(COLORREF)-1;
	}

	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &afxGlobalData.brBarFace);
	return bIsDisabled ? afxGlobalData.clrGrayedText : afxGlobalData.clrBarText;
}

void CMFCVisualManager::OnDrawCaptionBarButtonBorder(CDC* pDC, CMFCCaptionBar* pBar, CRect rect,
	BOOL bIsPressed, BOOL bIsHighlighted, BOOL bIsDisabled, BOOL bHasDropDownArrow, BOOL bIsSysButton)
{
	UNREFERENCED_PARAMETER(bIsDisabled);
	UNREFERENCED_PARAMETER(bHasDropDownArrow);
	UNREFERENCED_PARAMETER(bIsSysButton);

	ASSERT_VALID(pDC);
	ASSERT_VALID(pBar);

	if (bIsPressed)
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarHilite);
	}
	else if (bIsHighlighted || pBar->IsMessageBarMode())
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarDkShadow);
	}
}

void CMFCVisualManager::OnDrawStatusBarProgress(CDC* pDC, CMFCStatusBar* /*pStatusBar*/, CRect rectProgress,
	int nProgressTotal, int nProgressCurr, COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText, BOOL bProgressText)
{
	ASSERT_VALID(pDC);

	if (nProgressTotal == 0)
	{
		return;
	}

	CRect rectComplete = rectProgress;
	rectComplete.right = rectComplete.left + nProgressCurr * rectComplete.Width() / nProgressTotal;

	if (clrProgressBarDest == (COLORREF)-1)
	{
		// one-color bar
		CBrush br(clrBar);
		pDC->FillRect(rectComplete, &br);
	}
	else
	{
		// gradient bar:
		CDrawingManager dm(*pDC);
		dm.FillGradient(rectComplete, clrBar, clrProgressBarDest, FALSE);
	}

	if (bProgressText)
	{
		CString strText;
		strText.Format(_T("%d%%"), nProgressCurr * 100 / nProgressTotal);

		COLORREF clrText = pDC->SetTextColor(afxGlobalData.clrBarText);

		pDC->DrawText(strText, rectProgress, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

		CRgn rgn;
		rgn.CreateRectRgnIndirect(rectComplete);
		pDC->SelectClipRgn(&rgn);
		pDC->SetTextColor(clrProgressText == (COLORREF)-1 ? afxGlobalData.clrTextHilite : clrProgressText);
		pDC->DrawText(strText, rectProgress, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		pDC->SelectClipRgn(NULL);
		pDC->SetTextColor(clrText);
	}
}

void CMFCVisualManager::OnFillHeaderCtrlBackground(CMFCHeaderCtrl* pCtrl, CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, pCtrl->IsDialogControl() ? &afxGlobalData.brBtnFace : &afxGlobalData.brBarFace);
}

void CMFCVisualManager::OnDrawHeaderCtrlBorder(CMFCHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsPressed, BOOL /*bIsHighlighted*/)
{
	ASSERT_VALID(pDC);

	if (bIsPressed)
	{
		if (pCtrl->IsDialogControl())
		{
			pDC->Draw3dRect(rect, afxGlobalData.clrBtnShadow, afxGlobalData.clrBtnShadow);
		}
		else
		{
			pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
		}

		rect.left++;
		rect.top++;
	}
	else
	{
		if (pCtrl->IsDialogControl())
		{
			pDC->Draw3dRect(rect, afxGlobalData.clrBtnHilite, afxGlobalData.clrBtnShadow);
		}
		else
		{
			pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
		}
	}
}

void CMFCVisualManager::OnDrawHeaderCtrlSortArrow(CMFCHeaderCtrl* pCtrl, CDC* pDC, CRect& rectArrow, BOOL bIsUp)
{
	DoDrawHeaderSortArrow(pDC, rectArrow, bIsUp, pCtrl->IsDialogControl());
}

void CMFCVisualManager::OnDrawStatusBarSizeBox(CDC* pDC, CMFCStatusBar* /*pStatBar*/, CRect rectSizeBox)
{
	ASSERT_VALID(pDC);

	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontMarlett);
	ENSURE(pOldFont != NULL);

	const CString strSizeBox(_T("o")); // Char of the sizing box in "Marlett" font

	UINT nTextAlign = pDC->SetTextAlign(TA_RIGHT | TA_BOTTOM);
	COLORREF clrText = pDC->SetTextColor(afxGlobalData.clrBarShadow);

	pDC->ExtTextOut(rectSizeBox.right, rectSizeBox.bottom,
		ETO_CLIPPED, &rectSizeBox, strSizeBox, NULL);

	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(clrText);
	pDC->SetTextAlign(nTextAlign);
}

void CMFCVisualManager::OnDrawEditBorder(CDC* pDC, CRect rect, BOOL /*bDisabled*/, BOOL bIsHighlighted, CMFCToolBarEditBoxButton* /*pButton*/)
{
	ASSERT_VALID(pDC);

	if (bIsHighlighted)
	{
		pDC->DrawEdge(rect, EDGE_SUNKEN, BF_RECT);
	}
}

void CMFCVisualManager::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rectWorkArea, &afxGlobalData.brWindow);
}

void CMFCVisualManager::OnDrawTasksGroupCaption(CDC* pDC, CMFCTasksPaneTaskGroup* pGroup, BOOL bIsHighlighted, BOOL bIsSelected, BOOL bCanCollapse)
{
	ENSURE(pGroup != NULL);
	ENSURE(pGroup->m_pPage != NULL);

	ASSERT_VALID(pDC);
	ASSERT_VALID(pGroup);
	ASSERT_VALID(pGroup->m_pPage);

	CRect rectGroup = pGroup->m_rect;

	// ---------------------------------
	// Draw caption background(Windows)
	// ---------------------------------
	COLORREF clrBckOld = pDC->GetBkColor();
	pDC->FillSolidRect(rectGroup, (pGroup->m_bIsSpecial ? afxGlobalData.clrHilite : afxGlobalData.clrBarFace));
	pDC->SetBkColor(clrBckOld);

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
		clrTextOld = pDC->SetTextColor(pGroup->m_clrTextHot == (COLORREF)-1 ? (pGroup->m_bIsSpecial ? afxGlobalData.clrWindow : afxGlobalData.clrWindowText) : pGroup->m_clrTextHot);
	}
	else
	{
		clrTextOld = pDC->SetTextColor(pGroup->m_clrText == (COLORREF)-1 ? (pGroup->m_bIsSpecial ? afxGlobalData.clrWindow : afxGlobalData.clrWindowText) : pGroup->m_clrText);
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
				COLORREF clrBckOldFrame = pDC->GetBkColor();

				pDC->Draw3dRect(&rectButton, afxGlobalData.clrWindow, afxGlobalData.clrBarShadow);

				pDC->SetBkColor(clrBckOldFrame);
				pDC->SelectObject(pBrushOld);
			}

			if (pGroup->m_bIsSpecial)
			{
				if (!pGroup->m_bIsCollapsed)
				{
					CMenuImages::Draw(pDC, CMenuImages::IdArrowUp, rectButton.TopLeft());
				}
				else
				{
					CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rectButton.TopLeft());
				}
			}
			else
			{
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
}

void CMFCVisualManager::OnDrawTasksGroupIcon(CDC* pDC, CMFCTasksPaneTaskGroup* pGroup, int nIconHOffset, BOOL /*bIsHighlighted*/, BOOL /*bIsSelected*/, BOOL /*bCanCollapse*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pGroup);

	if (pGroup->m_hIcon == NULL)
	{
		return;
	}

	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();

	CRect rectImage = pGroup->m_rect;
	rectImage.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectImage.right = rectImage.left + pGroup->m_sizeIcon.cx + nIconHOffset;

	int x = max(0, (rectImage.Width() - pGroup->m_sizeIcon.cx) / 2);
	int y = max(0, (rectImage.Height() - pGroup->m_sizeIcon.cy) / 2);

	::DrawIconEx(pDC->GetSafeHdc(), rectImage.left + x, rectImage.bottom - y - pGroup->m_sizeIcon.cy,
		pGroup->m_hIcon, pGroup->m_sizeIcon.cx, pGroup->m_sizeIcon.cy, 0, NULL, DI_NORMAL);
}

void CMFCVisualManager::OnFillTasksGroupInterior(CDC* /*pDC*/, CRect /*rect*/, BOOL /*bSpecial*/)
{
}

void CMFCVisualManager::OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial, BOOL bNoTitle)
{
	ASSERT_VALID(pDC);

	// Draw caption background:
	CPen* pPenOld = (CPen*) pDC->SelectObject(bSpecial ? &afxGlobalData.penHilite : &afxGlobalData.penBarFace);

	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.left, rect.bottom-1);
	pDC->LineTo(rect.right-1, rect.bottom-1);
	pDC->LineTo(rect.right-1, rect.top);
	if (bNoTitle)
	{
		pDC->LineTo(rect.left, rect.top);
	}
	else
	{
		pDC->LineTo(rect.right-1, rect.top-1);
	}
	pDC->SelectObject(pPenOld);
}

void CMFCVisualManager::OnDrawTask(CDC* pDC, CMFCTasksPaneTask* pTask, CImageList* pIcons, BOOL bIsHighlighted, BOOL /*bIsSelected*/)
{
	ENSURE(pTask != NULL);
	ENSURE(pIcons != NULL);

	ASSERT_VALID(pDC);
	ASSERT_VALID(pIcons);
	ASSERT_VALID(pTask);

	CRect rectText = pTask->m_rect;

	if (pTask->m_bIsSeparator)
	{
		CPen* pPenOld = (CPen*) pDC->SelectObject(&afxGlobalData.penBarFace);

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
		pFontOld = pDC->SelectObject(&afxGlobalData.fontUnderline);
		pDC->SetTextColor(pTask->m_clrTextHot == (COLORREF)-1 ? afxGlobalData.clrWindowText : pTask->m_clrTextHot);
	}
	else
	{
		pFontOld = pDC->SelectObject(&afxGlobalData.fontRegular);
		pDC->SetTextColor(pTask->m_clrText == (COLORREF)-1 ? afxGlobalData.clrWindowText : pTask->m_clrText);
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

void CMFCVisualManager::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize, int iImage, BOOL bHilited)
{
	ASSERT_VALID(pDC);

	CRect rectImage(CPoint(0, 0), CMenuImages::Size());

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect(rectFill, &afxGlobalData.brBarFace);

	if (bHilited)
	{
		CDrawingManager dm(*pDC);
		dm.HighlightRect(rect);

		pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarDkShadow);
	}

	CMenuImages::Draw(pDC, (CMenuImages::IMAGES_IDS) iImage, rect);
}

void CMFCVisualManager::OnDrawToolBoxFrame(CDC* pDC, const CRect& rect)
{
	ASSERT_VALID(pDC);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarFace, afxGlobalData.clrBarFace);
}

void CMFCVisualManager::OnDrawPaneDivider(CDC* pDC, CPaneDivider* pSlider, CRect rect, BOOL bAutoHideMode)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pSlider);

	CRect rectScreen = afxGlobalData.m_rectVirtual;
	pSlider->ScreenToClient(&rectScreen);

	CRect rectFill = rect;
	rectFill.left = min(rectFill.left, rectScreen.left);

	OnFillBarBackground(pDC, pSlider, rectFill, rect);

	if (bAutoHideMode)
	{
		// draw outer edge;

		DWORD dwAlgn = pSlider->GetCurrentAlignment();
		CRect rectBorder = rect;

		COLORREF clrBorder = afxGlobalData.clrBarDkShadow;

		if (dwAlgn & CBRS_ALIGN_LEFT)
		{
			rectBorder.left = rectBorder.right;
		}
		else if (dwAlgn & CBRS_ALIGN_RIGHT)
		{
			rectBorder.right = rectBorder.left;
			clrBorder = afxGlobalData.clrBarHilite;
		}
		else if (dwAlgn & CBRS_ALIGN_TOP)
		{
			rectBorder.top = rectBorder.bottom;
		}
		else if (dwAlgn & CBRS_ALIGN_BOTTOM)
		{
			rectBorder.bottom = rectBorder.top;
			clrBorder = afxGlobalData.clrBarHilite;
		}
		else
		{
			ASSERT(FALSE);
			return;
		}

		pDC->Draw3dRect(rectBorder, clrBorder, clrBorder);
	}
}

void CMFCVisualManager::OnDrawSplitterBorder(CDC* pDC, CSplitterWndEx* /*pSplitterWnd*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
	rect.InflateRect(-AFX_CX_BORDER, -AFX_CY_BORDER);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarFace, afxGlobalData.clrBarFace);
}

void CMFCVisualManager::OnDrawSplitterBox(CDC* pDC, CSplitterWndEx* /*pSplitterWnd*/, CRect& rect)
{
	ASSERT_VALID(pDC);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarFace, afxGlobalData.clrBarShadow);
}

void CMFCVisualManager::OnFillSplitterBackground(CDC* pDC, CSplitterWndEx* /*pSplitterWnd*/, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillSolidRect(rect, afxGlobalData.clrBarFace);
}

void CMFCVisualManager::OnDrawCheckBox(CDC *pDC, CRect rect, BOOL bHighlighted, BOOL bChecked, BOOL bEnabled)
{
	OnDrawCheckBoxEx(pDC, rect, bChecked ? 1 : 0, bHighlighted, FALSE, bEnabled);
}

void CMFCVisualManager::OnDrawCheckBoxEx(CDC *pDC, CRect rect, int nState, BOOL bHighlighted, BOOL /*bPressed*/, BOOL bEnabled)
{
	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);

		rect.DeflateRect(1, 1);

		dm.DrawRect(rect, bEnabled ? afxGlobalData.clrWindow : afxGlobalData.clrBarFace, afxGlobalData.clrBarShadow);

		if (nState == 1)
		{
			CMenuImages::Draw(pDC, CMenuImages::IdCheck, rect, CMenuImages::ImageBlack);
		}

		return;
	}

	if (bHighlighted)
	{
		pDC->DrawFocusRect(rect);
	}

	rect.DeflateRect(1, 1);
	pDC->FillSolidRect(&rect, bEnabled ? afxGlobalData.clrWindow : afxGlobalData.clrBarFace);
	pDC->Draw3dRect(&rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarHilite);

	rect.DeflateRect(1, 1);
	pDC->Draw3dRect(&rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarLight);

	if (nState == 1)
	{
		CMenuImages::Draw(pDC, CMenuImages::IdCheck, rect, CMenuImages::ImageBlack);
	}
	else if (nState == 2)
	{
		rect.DeflateRect(1, 1);

		CBrush br;
		br.CreateHatchBrush(HS_DIAGCROSS, afxGlobalData.clrBtnText);

		pDC->FillRect(rect, &br);
	}
}

void CMFCVisualManager::OnDrawSpinButtons(CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CMFCSpinButtonCtrl* /*pSpinCtrl*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	rectSpin.DeflateRect(1, 1);

	CRect rect[2];

	rect[0] = rect[1] = rectSpin;

	if (!bOrientation)
	{
		rect[0].DeflateRect(0, 0, 0, rect[0].Height() / 2);
		rect[1].top = rect[0].bottom + 1;
	}
	else
	{
		rect[0].DeflateRect(0, 0, rect[0].Width() / 2, 0);
		rect[1].left = rect[0].right + 1;
	}

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawRect(rectSpin, afxGlobalData.clrBarFace, afxGlobalData.clrBarHilite);
	}
	else
	{
		pDC->FillRect(rectSpin, &afxGlobalData.brBarFace);
		pDC->Draw3dRect(rectSpin, afxGlobalData.clrBarHilite, afxGlobalData.clrBarHilite);
	}

	CMenuImages::IMAGES_IDS id[2][2] = {{CMenuImages::IdArrowUp, CMenuImages::IdArrowDown}, {CMenuImages::IdArrowLeft, CMenuImages::IdArrowRight}};

	int idxPressed = (nState &(AFX_SPIN_PRESSEDUP | AFX_SPIN_PRESSEDDOWN)) - 1;
	BOOL bDisabled = nState & AFX_SPIN_DISABLED;

	for (int i = 0; i < 2; i ++)
	{
		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);

			if (idxPressed == i)
			{
				dm.DrawRect(rect[i], (COLORREF)-1, afxGlobalData.clrBarShadow);
			}
			else
			{
				dm.DrawRect(rect[i], (COLORREF)-1, afxGlobalData.clrBarHilite);
			}
		}
		else
		{
			if (idxPressed == i)
			{
				pDC->Draw3dRect(&rect[i], afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
			}
			else
			{
				pDC->Draw3dRect(&rect[i], afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
			}
		}

		CMenuImages::Draw(pDC, id[bOrientation ? 1 : 0][i], rect[i], bDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack);
	}
}

void CMFCVisualManager::OnDrawExpandingBox(CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, colorBox, colorBox);

	rect.DeflateRect(2, 2);

	CPen penLine(PS_SOLID, 1, afxGlobalData.clrBarText);
	CPen* pOldPen = pDC->SelectObject(&penLine);

	CPoint ptCenter = rect.CenterPoint();

	pDC->MoveTo(rect.left, ptCenter.y);
	pDC->LineTo(rect.right, ptCenter.y);

	if (!bIsOpened)
	{
		pDC->MoveTo(ptCenter.x, rect.top);
		pDC->LineTo(ptCenter.x, rect.bottom);
	}

	pDC->SelectObject(pOldPen);
}

void CMFCVisualManager::OnDrawControlBorder(CWnd* pWndCtrl)
{
	ASSERT_VALID(pWndCtrl);

	CWindowDC dc(pWndCtrl);

	CRect rect;
	pWndCtrl->GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	if (pWndCtrl->GetStyle() & WS_POPUP)
	{
		dc.Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
	}
	else
	{
		dc.Draw3dRect(rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarHilite);
	}

	rect.DeflateRect(1, 1);
	dc.Draw3dRect(rect, afxGlobalData.clrWindow, afxGlobalData.clrWindow);
}

void CMFCVisualManager::OnDrawShowAllMenuItems(CDC* pDC, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE /*state*/)
{
	ASSERT_VALID(pDC);
	CMenuImages::Draw(pDC, CMenuImages::IdArrowShowAll, rect);
}

int CMFCVisualManager::GetShowAllMenuItemsHeight(CDC* /*pDC*/, const CSize& /*sizeDefault*/)
{
	return CMenuImages::Size().cy + 2 * AFX_TEXT_MARGIN;
}

void CMFCVisualManager::GetTabFrameColors(const CMFCBaseTabCtrl* pTabWnd, COLORREF& clrDark, COLORREF& clrBlack,
	COLORREF& clrHighlight, COLORREF& clrFace, COLORREF& clrDarkShadow, COLORREF& clrLight, CBrush*& pbrFace, CBrush*& pbrBlack)
{
	ASSERT_VALID(pTabWnd);

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor(pTabWnd->GetActiveTab());

	if (pTabWnd->IsOneNoteStyle() && clrActiveTab != (COLORREF)-1)
	{
		clrFace = clrActiveTab;
	}
	else if (pTabWnd->IsDialogControl())
	{
		clrFace = afxGlobalData.clrBtnFace;
	}
	else
	{
		clrFace = afxGlobalData.clrBarFace;
	}

	if (pTabWnd->IsDialogControl())
	{
		clrDark = afxGlobalData.clrBtnShadow;
		clrBlack = afxGlobalData.clrBtnText;
		clrHighlight = pTabWnd->IsVS2005Style() ? afxGlobalData.clrBtnShadow : afxGlobalData.clrBtnHilite;
		clrDarkShadow = afxGlobalData.clrBtnDkShadow;
		clrLight = afxGlobalData.clrBtnLight;

		pbrFace = &afxGlobalData.brBtnFace;
	}
	else
	{
		clrDark = afxGlobalData.clrBarShadow;
		clrBlack = afxGlobalData.clrBarText;
		clrHighlight = pTabWnd->IsVS2005Style() ? afxGlobalData.clrBarShadow : afxGlobalData.clrBarHilite;
		clrDarkShadow = afxGlobalData.clrBarDkShadow;
		clrLight = afxGlobalData.clrBarLight;

		pbrFace = &afxGlobalData.brBarFace;
	}

	pbrBlack = &afxGlobalData.brBlack;
}

void CMFCVisualManager::OnFillAutoHideButtonBackground(CDC* pDC, CRect rect, CMFCAutoHideButton* /*pButton*/)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &afxGlobalData.brBarFace);
}

void CMFCVisualManager::OnDrawAutoHideButtonBorder(CDC* pDC, CRect rectBounds, CRect rectBorderSize, CMFCAutoHideButton* /*pButton*/)
{
	ASSERT_VALID(pDC);

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

COLORREF CMFCVisualManager::GetAutoHideButtonTextColor(CMFCAutoHideButton* /*pButton*/)
{
	return afxGlobalData.clrBarText;
}

void CMFCVisualManager::OnDrawOutlookBarSplitter(CDC* pDC, CRect rectSplitter)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rectSplitter, &afxGlobalData.brBarFace);
	pDC->Draw3dRect(rectSplitter, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
}

void CMFCVisualManager::OnFillOutlookBarCaption(CDC* pDC, CRect rectCaption, COLORREF& clrText)
{

	pDC->FillSolidRect(rectCaption, afxGlobalData.clrBarShadow);
	clrText = afxGlobalData.clrBarHilite;
}

BOOL CMFCVisualManager::OnDrawBrowseButton(CDC* pDC, CRect rect, CMFCEditBrowseCtrl* /*pEdit*/, CMFCVisualManager::AFX_BUTTON_STATE state, COLORREF& /*clrText*/)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(&rect, &afxGlobalData.brBtnFace);

	CRect rectFrame = rect;
	rectFrame.InflateRect(0, 1, 1, 1);

	pDC->Draw3dRect(rectFrame, afxGlobalData.clrBtnDkShadow, afxGlobalData.clrBtnDkShadow);

	rectFrame.DeflateRect(1, 1);
	pDC->DrawEdge(rectFrame, state == ButtonsIsPressed ? BDR_SUNKENINNER : BDR_RAISEDINNER, BF_RECT);

	return TRUE;
}

void CMFCVisualManager::GetSmartDockingBaseGuideColors(COLORREF& clrBaseGroupBackground, COLORREF& clrBaseGroupBorder)
{
	clrBaseGroupBackground = afxGlobalData.clrBarFace;
	clrBaseGroupBorder = afxGlobalData.clrBarShadow;
}

COLORREF CMFCVisualManager::GetSmartDockingHighlightToneColor()
{
	return afxGlobalData.clrActiveCaption;
}

void CMFCVisualManager::OnFillPopupWindowBackground(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &afxGlobalData.brBarFace);
}

void CMFCVisualManager::OnDrawPopupWindowBorder(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, afxGlobalData.clrBarLight, afxGlobalData.clrBarDkShadow);
	rect.DeflateRect(1, 1);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
}

COLORREF  CMFCVisualManager::OnDrawPopupWindowCaption(CDC* pDC, CRect rectCaption, CMFCDesktopAlertWnd* /*pPopupWnd*/)
{
	ASSERT_VALID(pDC);

	CBrush br(afxGlobalData.clrActiveCaption);
	pDC->FillRect(rectCaption, &br);

	// get the text color
	return afxGlobalData.clrCaptionText;
}

void CMFCVisualManager::OnErasePopupWindowButton(CDC* pDC, CRect rect, CMFCDesktopAlertWndButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsCaptionButton())
	{
		pDC->FillRect(rect, &afxGlobalData.brBtnFace);
		return;
	}

	CRect rectParent;
	pButton->GetParent()->GetClientRect(rectParent);

	pButton->GetParent()->MapWindowPoints(pButton, rectParent);
	OnFillPopupWindowBackground(pDC, rectParent);
}

void CMFCVisualManager::OnDrawPopupWindowButtonBorder(CDC* pDC, CRect rect, CMFCDesktopAlertWndButton* pButton)
{
	if (pButton->IsPressed())
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarLight);
		rect.DeflateRect(1, 1);
		pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
	}
	else
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarLight, afxGlobalData.clrBarDkShadow);
		rect.DeflateRect(1, 1);
		pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
	}
}

void CMFCVisualManager::DoDrawHeaderSortArrow(CDC* pDC, CRect rectArrow, BOOL bIsUp, BOOL bDlgCtrl)
{
	CPen penLight(PS_SOLID, 1, bDlgCtrl ? afxGlobalData.clrBtnHilite : afxGlobalData.clrBarHilite);
	CPen penDark(PS_SOLID, 1, bDlgCtrl ? afxGlobalData.clrBtnDkShadow : afxGlobalData.clrBarDkShadow);

	CPen* pPenOld = pDC->SelectObject(&penLight);
	ASSERT_VALID(pPenOld);

	if (!bIsUp)
	{
		pDC->MoveTo(rectArrow.right, rectArrow.top);
		pDC->LineTo(rectArrow.CenterPoint().x, rectArrow.bottom);

		pDC->SelectObject(&penDark);
		pDC->LineTo(rectArrow.left, rectArrow.top);
		pDC->LineTo(rectArrow.right, rectArrow.top);
	}
	else
	{
		pDC->MoveTo(rectArrow.left, rectArrow.bottom);
		pDC->LineTo(rectArrow.right, rectArrow.bottom);
		pDC->LineTo(rectArrow.CenterPoint().x, rectArrow.top);

		pDC->SelectObject(&penDark);
		pDC->LineTo(rectArrow.left, rectArrow.bottom);
	}

	pDC->SelectObject(pPenOld);
}

COLORREF CMFCVisualManager::GetPropertyGridGroupColor(CMFCPropertyGridCtrl* pPropList)
{
	ASSERT_VALID(pPropList);

	return pPropList->DrawControlBarColors() ? afxGlobalData.clrBarFace : afxGlobalData.clrBtnFace;
}

COLORREF CMFCVisualManager::GetPropertyGridGroupTextColor(CMFCPropertyGridCtrl* pPropList)
{
	ASSERT_VALID(pPropList);

	return pPropList->DrawControlBarColors() ? afxGlobalData.clrBarDkShadow : afxGlobalData.clrBtnDkShadow;
}

COLORREF CMFCVisualManager::GetMenuItemTextColor(CMFCToolBarMenuButton* /*pButton*/, BOOL bHighlighted, BOOL bDisabled)
{
	if (bHighlighted)
	{
		return bDisabled ? afxGlobalData.clrBtnFace : afxGlobalData.clrTextHilite;
	}

	return bDisabled ? afxGlobalData.clrGrayedText : afxGlobalData.clrWindowText;
}

COLORREF CMFCVisualManager::GetStatusBarPaneTextColor(CMFCStatusBar* /*pStatusBar*/, CMFCStatusBarPaneInfo* pPane)
{
	ENSURE(pPane != NULL);

	return(pPane->nStyle & SBPS_DISABLED) ? afxGlobalData.clrGrayedText : pPane->clrText == (COLORREF)-1 ? afxGlobalData.clrBtnText : pPane->clrText;
}

void CMFCVisualManager::OnDrawRibbonCaption(CDC* pDC, CMFCRibbonBar* pBar, CRect rect, CRect rectText)
{
	ASSERT_VALID(pBar);

	CWnd* pWnd = pBar->GetParent();
	ASSERT_VALID(pWnd);

	const BOOL bGlass = pBar->IsTransparentCaption();
	const DWORD dwStyleEx  = pWnd->GetExStyle();
	const BOOL bIsRTL = (dwStyleEx & WS_EX_LAYOUTRTL) == WS_EX_LAYOUTRTL;
	BOOL bTextCenter = TRUE;

	ASSERT_VALID(pDC);

	if ((pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ALL) == AFX_RIBBONBAR_HIDE_ALL)
	{
		HICON hIcon = afxGlobalUtils.GetWndIcon(pWnd);

		if (hIcon != NULL)
		{
			CSize szIcon(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
			CRect rectIcon(rect.TopLeft(), CSize(min(::GetSystemMetrics(SM_CYCAPTION), rect.Height()), rect.Height()));

			long x = rect.left + max(0, (rectIcon.Width()  - szIcon.cx) / 2);
			long y = rect.top  + max(0, (rectIcon.Height() - szIcon.cy) / 2);

			::DrawIconEx(pDC->GetSafeHdc(), x, y, hIcon, szIcon.cx, szIcon.cy, 0, NULL, DI_NORMAL);

			if (rectText.left < rectIcon.right)
			{
				rectText.left = rectIcon.right;
			}
		}

		bTextCenter = TRUE;
	}

	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
	ENSURE(pOldFont != NULL);

	int nOldMode = pDC->SetBkMode(TRANSPARENT);

	CString strCaption;
	pWnd->GetWindowText(strCaption);

	DWORD dwTextStyle = DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | (bIsRTL ? DT_RTLREADING | DT_RIGHT : 0);

	COLORREF clrText = RGB(0, 0, 0);

	int widthFull = rectText.Width();
	int width = pDC->GetTextExtent(strCaption).cx;

	if (bTextCenter && width < widthFull)
	{
		rectText.left += (widthFull - width) / 2;
	}

	rectText.right = min(rectText.left + width, rectText.right);

	if (rectText.right > rectText.left)
	{
		if (bGlass)
		{
			DrawTextOnGlass(pDC, strCaption, rectText, dwTextStyle, 10);
		}
		else
		{
			COLORREF clrOldText = pDC->SetTextColor(clrText);
			pDC->DrawText(strCaption, rectText, dwTextStyle);
			pDC->SetTextColor(clrOldText);
		}
	}

	pDC->SetBkMode(nOldMode);
	pDC->SelectObject(pOldFont);
}

void CMFCVisualManager::OnDrawRibbonCaptionButton(CDC* pDC, CMFCRibbonCaptionButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	OnFillRibbonButton(pDC, pButton);

	CMenuImages::IMAGES_IDS imageID;

	switch(pButton->GetID())
	{
	case SC_CLOSE:
		imageID = CMenuImages::IdClose;
		break;

	case SC_MINIMIZE:
		imageID = CMenuImages::IdMinimize;
		break;

	case SC_MAXIMIZE:
		imageID = CMenuImages::IdMaximize;
		break;

	case SC_RESTORE:
		imageID = CMenuImages::IdRestore;
		break;

	default:
		return;
	}

	CMenuImages::Draw(pDC, imageID, pButton->GetRect(), pButton->IsDisabled() ? CMenuImages::ImageGray : CMenuImages::ImageBlack);

	OnDrawRibbonButtonBorder(pDC, pButton);
}

COLORREF CMFCVisualManager::OnDrawRibbonButtonsGroup(CDC* /*pDC*/, CMFCRibbonButtonsGroup* /*pGroup*/, CRect /*rectGroup*/)
{
	return(COLORREF)-1;
}

void CMFCVisualManager::OnDrawDefaultRibbonImage(CDC* pDC, CRect rectImage, BOOL bIsDisabled, BOOL /*bIsPressed*/, BOOL /*bIsHighlighted*/)
{
	ASSERT_VALID(pDC);

	CRect rectBullet(rectImage.CenterPoint(), CSize(1, 1));
	rectBullet.InflateRect(5, 5);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		CBrush br(bIsDisabled ? afxGlobalData.clrGrayedText : RGB(0, 127, 0));

		CBrush* pOldBrush = (CBrush*) pDC->SelectObject(&br);
		CPen* pOldPen = (CPen*) pDC->SelectStockObject(NULL_PEN);

		pDC->Ellipse(rectBullet);

		pDC->SelectObject(pOldBrush);
		pDC->SelectObject(pOldPen);
	}
	else
	{
		CDrawingManager dm(*pDC);

		dm.DrawEllipse(rectBullet, bIsDisabled ? afxGlobalData.clrGrayedText : RGB(160, 208, 128), bIsDisabled ? afxGlobalData.clrBtnShadow : RGB(71, 117, 44));
	}
}

void CMFCVisualManager::OnDrawRibbonApplicationButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	const BOOL bIsHighlighted = pButton->IsHighlighted();
	const BOOL bIsPressed = pButton->IsPressed() || pButton->IsDroppedDown();

	CRect rect = pButton->GetRect();
	rect.DeflateRect(2, 2);

	CDrawingManager dm(*pDC);

	dm.DrawEllipse(rect, bIsPressed ? afxGlobalData.clrBarLight : afxGlobalData.clrBarFace, bIsHighlighted ? afxGlobalData.clrBarDkShadow : afxGlobalData.clrBarShadow);
}

COLORREF CMFCVisualManager::OnDrawRibbonTabsFrame(CDC* pDC, CMFCRibbonBar* /*pWndRibbonBar*/, CRect rectTab)
{
	ASSERT_VALID(pDC);

	CPen pen(PS_SOLID, 1, afxGlobalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject(&pen);
	ENSURE(pOldPen != NULL);

	pDC->MoveTo(rectTab.left, rectTab.top);
	pDC->LineTo(rectTab.right, rectTab.top);

	pDC->SelectObject(pOldPen);

	return(COLORREF)-1;
}

void CMFCVisualManager::OnDrawRibbonCategory(CDC* pDC, CMFCRibbonCategory* pCategory, CRect rectCategory)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pCategory);

	const int nShadowSize = 2;

	rectCategory.right -= nShadowSize;
	rectCategory.bottom -= nShadowSize;

	pDC->FillRect(rectCategory, &afxGlobalData.brBarFace);

	CRect rectActiveTab = pCategory->GetTabRect();

	CPen pen(PS_SOLID, 1, afxGlobalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject(&pen);
	ENSURE(pOldPen != NULL);

	pDC->MoveTo(rectCategory.left, rectCategory.top);
	pDC->LineTo(rectActiveTab.left + 1, rectCategory.top);

	pDC->MoveTo(rectActiveTab.right - 2, rectCategory.top);
	pDC->LineTo(rectCategory.right, rectCategory.top);
	pDC->LineTo(rectCategory.right, rectCategory.bottom);
	pDC->LineTo(rectCategory.left, rectCategory.bottom);
	pDC->LineTo(rectCategory.left, rectCategory.top);

	pDC->SelectObject(pOldPen);

	CDrawingManager dm(*pDC);
	dm.DrawShadow(rectCategory, nShadowSize, 100, 75, NULL, NULL, m_clrMenuShadowBase);
}

void CMFCVisualManager::OnDrawRibbonCategoryScroll(CDC* pDC, CRibbonCategoryScroll* pScroll)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pScroll);

	CRect rect = pScroll->GetRect();
	rect.bottom--;

	pDC->FillRect(rect, &afxGlobalData.brBarFace);
	if (pScroll->IsHighlighted())
	{
		CDrawingManager dm(*pDC);
		dm.HighlightRect(rect);
	}

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

COLORREF CMFCVisualManager::OnDrawRibbonCategoryTab(CDC* pDC, CMFCRibbonTab* pTab, BOOL bIsActive)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTab);

	CMFCRibbonCategory* pCategory = pTab->GetParentCategory();
	ASSERT_VALID(pCategory);
	CMFCRibbonBar* pBar = pCategory->GetParentRibbonBar();
	ASSERT_VALID(pBar);

	bIsActive = bIsActive && ((pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ELEMENTS) == 0 || pTab->GetDroppedDown() != NULL);

	const BOOL bIsHighlighted = pTab->IsHighlighted() && !pTab->IsDroppedDown();

	CPen pen(PS_SOLID, 1, afxGlobalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject(&pen);
	ENSURE(pOldPen != NULL);

	CRect rectTab = pTab->GetRect();

	rectTab.top += 3;

	const int nTrancateRatio = pTab->GetParentCategory()->GetParentRibbonBar()->GetTabTrancateRatio();

	if (nTrancateRatio > 0)
	{
		const int nPercent = max(10, 100 - nTrancateRatio / 2);

		COLORREF color = CDrawingManager::PixelAlpha(afxGlobalData.clrBarFace, nPercent);

		CPen penColor(PS_SOLID, 1, color);
		pDC->SelectObject(&penColor);

		pDC->MoveTo(rectTab.right - 1, rectTab.top);
		pDC->LineTo(rectTab.right - 1, rectTab.bottom);
	}

	if (!bIsActive && !bIsHighlighted)
	{
		pDC->SelectObject(pOldPen);
		return afxGlobalData.clrBarText;
	}

	rectTab.right -= 2;

#define AFX_POINTS_NUM 8
	POINT pts [AFX_POINTS_NUM];

	pts [0] = CPoint(rectTab.left, rectTab.bottom);
	pts [1] = CPoint(rectTab.left + 1, rectTab.bottom - 1);
	pts [2] = CPoint(rectTab.left + 1, rectTab.top + 2);
	pts [3] = CPoint(rectTab.left + 3, rectTab.top);
	pts [4] = CPoint(rectTab.right - 3, rectTab.top);
	pts [5] = CPoint(rectTab.right - 1, rectTab.top + 2);
	pts [6] = CPoint(rectTab.right - 1, rectTab.bottom - 1);
	pts [7] = CPoint(rectTab.right, rectTab.bottom);

	CRgn rgnClip;
	rgnClip.CreatePolygonRgn(pts, AFX_POINTS_NUM, WINDING);

	if (bIsActive)
	{
		pDC->SelectClipRgn(&rgnClip);

		COLORREF clrFill = pTab->IsSelected() ? afxGlobalData.clrBarHilite : RibbonCategoryColorToRGB(pTab->GetParentCategory()->GetTabColor());

		if (clrFill != (COLORREF)-1)
		{
			CBrush br(clrFill);
			pDC->FillRect(rectTab, &br);
		}
		else
		{
			pDC->FillRect(rectTab, bIsHighlighted ? &afxGlobalData.brWindow : &afxGlobalData.brBarFace);
		}

		pDC->SelectClipRgn(NULL);
	}

	pDC->Polyline(pts, AFX_POINTS_NUM);
	pDC->SelectObject(pOldPen);

	return afxGlobalData.clrBarText;
}

COLORREF CMFCVisualManager::OnDrawRibbonPanel(CDC* pDC, CMFCRibbonPanel* pPanel, CRect rectPanel, CRect /*rectCaption*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pPanel);

	if (pPanel->IsHighlighted())
	{
		CDrawingManager dm(*pDC);
		dm.HighlightRect(rectPanel);
	}

	pDC->Draw3dRect(rectPanel, afxGlobalData.clrBarHilite, afxGlobalData.clrBarHilite);
	rectPanel.OffsetRect(-1, -1);
	pDC->Draw3dRect(rectPanel, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);

	return afxGlobalData.clrBarText;
}

void CMFCVisualManager::OnDrawRibbonPanelCaption(CDC* pDC, CMFCRibbonPanel* pPanel, CRect rectCaption)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pPanel);

	COLORREF clrTextOld = pDC->SetTextColor(pPanel->IsHighlighted() ? afxGlobalData.clrCaptionText : afxGlobalData.clrInactiveCaptionText);

	rectCaption.DeflateRect(1, 1);
	rectCaption.right -= 2;

	CBrush br(pPanel->IsHighlighted() ? afxGlobalData.clrActiveCaption : afxGlobalData.clrInactiveCaption);
	pDC->FillRect(rectCaption, &br);

	CString str = pPanel->GetName();

	if (pPanel->GetLaunchButton().GetID() > 0)
	{
		rectCaption.right = pPanel->GetLaunchButton().GetRect().left;
	}

	pDC->DrawText( str, rectCaption, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

	pDC->SetTextColor(clrTextOld);
}

void CMFCVisualManager::OnDrawRibbonLaunchButton(CDC* pDC, CMFCRibbonLaunchButton* pButton, CMFCRibbonPanel* pPanel)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);
	ASSERT_VALID(pPanel);

	OnFillRibbonButton(pDC, pButton);

	COLORREF clrText = pPanel->IsHighlighted() ? afxGlobalData.clrCaptionText : afxGlobalData.clrInactiveCaptionText;

	CMenuImages::IMAGE_STATE imageState = CMenuImages::ImageBlack;

	if (pButton->IsDisabled())
	{
		imageState = CMenuImages::ImageGray;
	}
	else if (!pButton->IsHighlighted())
	{
		if (GetRValue(clrText) > 192 && GetGValue(clrText) > 192 && GetBValue(clrText) > 192)
		{
			imageState = CMenuImages::ImageWhite;
		}
		else
		{
			imageState = CMenuImages::ImageBlack;
		}
	}

	CMenuImages::Draw(pDC, CMenuImages::IdLaunchArrow, pButton->GetRect(), imageState);

	OnDrawRibbonButtonBorder(pDC, pButton);
}

void CMFCVisualManager::OnDrawRibbonDefaultPaneButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsQATMode())
	{
		OnFillRibbonButton(pDC, pButton);
		OnDrawRibbonDefaultPaneButtonContext(pDC, pButton);
		OnDrawRibbonButtonBorder(pDC, pButton);
	}
	else
	{
		OnDrawRibbonDefaultPaneButtonContext(pDC, pButton);
	}
}

void CMFCVisualManager::OnDrawRibbonDefaultPaneButtonContext(CDC* pDC, CMFCRibbonButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	CRect rectMenuArrow = pButton->GetRect();;

	if (pButton->IsQATMode())
	{
		pButton->DrawImage(pDC, CMFCRibbonButton::RibbonImageSmall, pButton->GetRect());
		return;
	}

	CRect rectImage = pButton->GetRect();
	rectImage.top += 10;
	rectImage.bottom = rectImage.top + pButton->GetImageSize(CMFCRibbonButton::RibbonImageSmall).cy;

	pButton->DrawImage(pDC, CMFCRibbonButton::RibbonImageSmall, rectImage);

	// Draw text:
	pButton->DrawBottomText(pDC, FALSE);
}

void CMFCVisualManager::OnDrawRibbonDefaultPaneButtonIndicator(CDC* pDC, CMFCRibbonButton* /*pButton*/, CRect rect, BOOL /*bIsSelected*/, BOOL /*bHighlighted*/)
{
	ASSERT_VALID(pDC);

	rect.left = rect.right - rect.Height();
	rect.DeflateRect(1, 1);

	pDC->FillRect(rect, &afxGlobalData.brBarFace);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);

	CRect rectWhite = rect;
	rectWhite.OffsetRect(0, 1);

	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rectWhite, CMenuImages::ImageWhite);
	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rect, CMenuImages::ImageBlack);
}

COLORREF CMFCVisualManager::OnFillRibbonButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonEdit)))
	{
		COLORREF clrBorder = afxGlobalData.clrBarShadow;
		CRect rectCommand = pButton->GetCommandRect();

		CRect rect = pButton->GetRect();
		rect.left = rectCommand.left;

		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);
			dm.DrawRect(rect, afxGlobalData.clrWindow, clrBorder);
		}
		else
		{
			if (pButton->IsDroppedDown() || pButton->IsHighlighted())
			{
				pDC->FillRect(rectCommand, &afxGlobalData.brWindow);
			}
			else
			{
				CDrawingManager dm(*pDC);
				dm.HighlightRect(rectCommand);
			}

			pDC->Draw3dRect(rect, clrBorder, clrBorder);
		}

		return(COLORREF)-1;
	}

	if (pButton->IsMenuMode() && !pButton->IsGalleryIcon())
	{
		if (pButton->IsHighlighted())
		{
			pDC->FillRect(pButton->GetRect(), &afxGlobalData.brHilite);
			return afxGlobalData.clrTextHilite;
		}
	}
	else
	{
		if (pButton->IsChecked() && !pButton->IsHighlighted())
		{
			if (CMFCToolBarImages::m_bIsDrawOnGlass)
			{
				CDrawingManager dm(*pDC);
				dm.DrawRect(pButton->GetRect(), afxGlobalData.clrWindow, (COLORREF)-1);
			}
			else
			{
				CMFCToolBarImages::FillDitheredRect(pDC, pButton->GetRect());
			}
		}
	}

	return(COLORREF)-1;
}

COLORREF CMFCVisualManager::OnFillRibbonMainPanelButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	return OnFillRibbonButton(pDC, pButton);
}

void CMFCVisualManager::OnDrawRibbonMainPanelButtonBorder(CDC* pDC, CMFCRibbonButton* pButton)
{
	OnDrawRibbonButtonBorder(pDC, pButton);
}

void CMFCVisualManager::OnFillRibbonEdit( CDC* pDC, CMFCRibbonRichEditCtrl* /*pEdit*/, CRect rect, BOOL bIsHighlighted,
	BOOL /*bIsPaneHighlighted*/, BOOL bIsDisabled, COLORREF& /*clrText*/, COLORREF& /*clrSelBackground*/, COLORREF& /*clrSelText*/)
{
	ASSERT_VALID(pDC);

	if (bIsHighlighted && !bIsDisabled)
	{
		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);
			dm.DrawRect(rect, afxGlobalData.clrWindow, (COLORREF)-1);
		}
		else
		{
			pDC->FillRect(rect, &afxGlobalData.brWindow);
		}
	}
	else
	{
		CDrawingManager dm(*pDC);

		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			dm.DrawRect(rect, afxGlobalData.clrBarFace, (COLORREF)-1);
		}
		else
		{
			pDC->FillRect(rect, &afxGlobalData.brBarFace);
			dm.HighlightRect(rect);
		}
	}
}

void CMFCVisualManager::OnDrawRibbonButtonBorder(CDC* pDC, CMFCRibbonButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonEdit)))
	{
		return;
	}

	CRect rect = pButton->GetRect();

	if (pButton->IsMenuMode() && pButton->IsChecked() && !pButton->IsHighlighted())
	{
		return;
	}

	if (pButton->IsHighlighted() || pButton->IsChecked() || pButton->IsDroppedDown())
	{
		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);
			dm.DrawRect(rect, (COLORREF)-1, afxGlobalData.clrBarShadow);
		}
		else
		{
			if (pButton->IsPressed() || pButton->IsChecked() || pButton->IsDroppedDown())
			{
				pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
			}
			else
			{
				pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
			}
		}

		CRect rectMenu = pButton->GetMenuRect();

		if (!rectMenu.IsRectEmpty())
		{
			if (CMFCToolBarImages::m_bIsDrawOnGlass)
			{
				CDrawingManager dm(*pDC);

				if (pButton->IsMenuOnBottom())
				{
					dm.DrawLine(rectMenu.left, rectMenu.top, rectMenu.right, rectMenu.top, afxGlobalData.clrBarShadow);
				}
				else
				{
					dm.DrawLine(rectMenu.left, rectMenu.top, rectMenu.left, rectMenu.bottom, afxGlobalData.clrBarShadow);
				}
			}
			else
			{
				CPen* pOldPen = pDC->SelectObject(&afxGlobalData.penBarShadow);
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

void CMFCVisualManager::OnDrawRibbonMenuCheckFrame(CDC* pDC, CMFCRibbonButton* /*pButton*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &afxGlobalData.brBtnFace);

	pDC->Draw3dRect(rect, afxGlobalData.clrBtnShadow, afxGlobalData.clrBtnHilite);
}

void CMFCVisualManager::OnDrawRibbonMainPanelFrame(CDC* pDC, CMFCRibbonMainPanel* /*pPanel*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
	rect.InflateRect(1, 1);
	pDC->Draw3dRect(rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarHilite);
}

void CMFCVisualManager::OnFillRibbonMenuFrame(CDC* pDC, CMFCRibbonMainPanel* /*pPanel*/, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &afxGlobalData.brWindow);
}

void CMFCVisualManager::OnDrawRibbonRecentFilesFrame(CDC* pDC, CMFCRibbonMainPanel* /*pPanel*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &afxGlobalData.brBtnFace);

	CRect rectSeparator = rect;
	rectSeparator.right = rectSeparator.left + 2;

	pDC->Draw3dRect(rectSeparator, afxGlobalData.clrBtnShadow, afxGlobalData.clrBtnHilite);
}

void CMFCVisualManager::OnDrawRibbonLabel(CDC* /*pDC*/, CMFCRibbonLabel* /*pLabel*/, CRect /*rect*/)
{
}

void CMFCVisualManager::OnDrawRibbonGalleryButton(CDC* pDC, CMFCRibbonGalleryIcon* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	OnFillRibbonButton(pDC, pButton);
	OnDrawRibbonButtonBorder(pDC, pButton);
}

void CMFCVisualManager::OnDrawRibbonGalleryBorder(CDC* pDC, CMFCRibbonGallery* /*pButton*/, CRect rectBorder)
{
	ASSERT_VALID(pDC);
	pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
}

COLORREF CMFCVisualManager::RibbonCategoryColorToRGB(AFX_RibbonCategoryColor color)
{
	if (afxGlobalData.m_nBitsPerPixel <= 8 || afxGlobalData.IsHighContrastMode())
	{
		switch(color)
		{
		case AFX_CategoryColor_Red:
			return RGB(255, 0, 0);

		case AFX_CategoryColor_Orange:
			return RGB(255, 128, 0);

		case AFX_CategoryColor_Yellow:
			return RGB(255, 255, 0);

		case AFX_CategoryColor_Green:
			return RGB(0, 255, 0);

		case AFX_CategoryColor_Blue:
			return RGB(0, 0, 255);

		case AFX_CategoryColor_Indigo:
			return RGB(0, 0, 128);

		case AFX_CategoryColor_Violet:
			return RGB(255, 0, 255);
		}

		return(COLORREF)-1;
	}

	switch(color)
	{
	case AFX_CategoryColor_Red:
		return RGB(255, 160, 160);

	case AFX_CategoryColor_Orange:
		return RGB(239, 189, 55);

	case AFX_CategoryColor_Yellow:
		return RGB(253, 229, 27);

	case AFX_CategoryColor_Green:
		return RGB(113, 190, 89);

	case AFX_CategoryColor_Blue:
		return RGB(128, 181, 196);

	case AFX_CategoryColor_Indigo:
		return RGB(114, 163, 224);

	case AFX_CategoryColor_Violet:
		return RGB(214, 178, 209);
	}

	return(COLORREF)-1;
}

COLORREF CMFCVisualManager::OnDrawRibbonCategoryCaption(CDC* pDC, CMFCRibbonContextCaption* pContextCaption)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pContextCaption);

	COLORREF clrFill = RibbonCategoryColorToRGB(pContextCaption->GetColor());
	CRect rect = pContextCaption->GetRect();

	if (clrFill != (COLORREF)-1)
	{
		CBrush br(clrFill);
		pDC->FillRect(rect, &br);
	}

	return afxGlobalData.clrBarText;
}

COLORREF CMFCVisualManager::OnDrawRibbonStatusBarPane(CDC* pDC, CMFCRibbonStatusBar* /*pBar*/, CMFCRibbonStatusBarPane* pPane)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pPane);

	CRect rect = pPane->GetRect();

	if (pPane->IsHighlighted())
	{
		CRect rectButton = rect;
		rectButton.DeflateRect(1, 1);

		pDC->Draw3dRect(rectButton, pPane->IsPressed() ? afxGlobalData.clrBarShadow : afxGlobalData.clrBarHilite, pPane->IsPressed() ? afxGlobalData.clrBarHilite : afxGlobalData.clrBarShadow);
	}

	return(COLORREF)-1;
}

void CMFCVisualManager::GetRibbonSliderColors(CMFCRibbonSlider* /*pSlider*/, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled, COLORREF& clrLine, COLORREF& clrFill)
{
	clrLine = bIsDisabled ? afxGlobalData.clrBarShadow : afxGlobalData.clrBarDkShadow;
	clrFill = bIsPressed && bIsHighlighted ? afxGlobalData.clrBarShadow :
	bIsHighlighted ? afxGlobalData.clrBarHilite : afxGlobalData.clrBarFace;
}

void CMFCVisualManager::OnDrawRibbonSliderZoomButton(CDC* pDC, CMFCRibbonSlider* pSlider, CRect rect, BOOL bIsZoomOut, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	ASSERT_VALID(pDC);

	COLORREF clrLine;
	COLORREF clrFill;

	GetRibbonSliderColors(pSlider, bIsHighlighted, bIsPressed, bIsDisabled, clrLine, clrFill);

	CPoint ptCenter = rect.CenterPoint();
	CRect rectCircle(CPoint(ptCenter.x - 7, ptCenter.y - 7), CSize(15, 15));
	CDrawingManager dm(*pDC);

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

void CMFCVisualManager::OnDrawRibbonSliderChannel(CDC* pDC, CMFCRibbonSlider* /*pSlider*/, CRect rect)
{
	ASSERT_VALID(pDC);

	rect.InflateRect(0, 1);

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawRect(rect, (COLORREF)-1, afxGlobalData.clrBarShadow);
	}
	else
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
	}
}

void CMFCVisualManager::OnDrawRibbonSliderThumb(CDC* pDC, CMFCRibbonSlider* pSlider, CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	ASSERT_VALID(pDC);

	COLORREF clrLine;
	COLORREF clrFill;

	GetRibbonSliderColors(pSlider, bIsHighlighted, bIsPressed, bIsDisabled, clrLine, clrFill);

	rect.DeflateRect(1, 2);

	rect.top = rect.CenterPoint().y - rect.Width();
	rect.bottom = rect.top + 2 * rect.Width();

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawRect(rect, clrFill, clrLine);
	}
	else
	{
		CPen penLine(PS_SOLID, 1, clrLine);
		CPen* pOldPen = pDC->SelectObject(&penLine);

		CBrush br(clrFill);
		CBrush* pOldBrush = pDC->SelectObject(&br);

		POINT pts [5] =
		{
			{ rect.left, rect.top },
			{ rect.left, rect.bottom - rect.Width() / 2 },
			{ rect.left + rect.Width() / 2, rect.bottom },
			{ rect.right, rect.bottom - rect.Width() / 2 },
			{ rect.right, rect.top },
		};

		pDC->Polygon(pts, 5);

		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
	}
}

void CMFCVisualManager::OnDrawRibbonProgressBar(CDC* pDC, CMFCRibbonProgressBar* /*pProgress*/, CRect rectProgress, CRect rectChunk, BOOL /*bInfiniteMode*/)
{
	ASSERT_VALID(pDC);

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);

		if (!rectChunk.IsRectEmpty())
		{
			dm.DrawRect(rectChunk, afxGlobalData.clrHilite, (COLORREF)-1);
		}

		dm.DrawRect(rectProgress, (COLORREF)-1, afxGlobalData.clrBarShadow);
	}
	else
	{
		if (!rectChunk.IsRectEmpty())
		{
			pDC->FillRect(rectChunk, &afxGlobalData.brHilite);
		}

		pDC->Draw3dRect(rectProgress, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
	}
}

void CMFCVisualManager::OnFillRibbonQuickAccessToolBarPopup(CDC* pDC, CMFCRibbonPanelMenuBar* /*pMenuBar*/, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &afxGlobalData.brBarFace);
}

void CMFCVisualManager::OnDrawRibbonQuickAccessToolBarSeparator(CDC* pDC, CMFCRibbonSeparator* /*pSeparator*/, CRect rect)
{
	ASSERT_VALID(pDC);

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawRect(rect, (COLORREF)-1, afxGlobalData.clrBtnShadow);
	}
	else
	{
		pDC->Draw3dRect(rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
	}
}

void CMFCVisualManager::OnDrawRibbonKeyTip(CDC* pDC, CMFCRibbonBaseElement* pElement, CRect rect, CString str)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pElement);

	::FillRect(pDC->GetSafeHdc(), rect, ::GetSysColorBrush(COLOR_INFOBK));

	str.MakeUpper();

	COLORREF clrTextOld = pDC->SetTextColor( pElement->IsDisabled() ? afxGlobalData.clrGrayedText : ::GetSysColor(COLOR_INFOTEXT));

	pDC->DrawText(str, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	pDC->SetTextColor(clrTextOld);

	pDC->Draw3dRect(rect, ::GetSysColor(COLOR_INFOTEXT), ::GetSysColor(COLOR_INFOTEXT));
}

void CMFCVisualManager::OnDrawRibbonCheckBoxOnList(CDC* pDC, CMFCRibbonCheckBox* /*pCheckBox*/, CRect rect, BOOL /*bIsSelected*/, BOOL /*bHighlighted*/)
{
	ASSERT_VALID(pDC);

	rect.OffsetRect(1, 1);
	CMenuImages::Draw(pDC, CMenuImages::IdCheck, rect, CMenuImages::ImageWhite);

	rect.OffsetRect(-1, -1);
	CMenuImages::Draw(pDC, CMenuImages::IdCheck, rect, CMenuImages::ImageBlack);
}

COLORREF CMFCVisualManager::GetRibbonHyperlinkTextColor(CMFCRibbonLinkCtrl* pHyperLink)
{
	ASSERT_VALID(pHyperLink);

	if (pHyperLink->IsDisabled())
	{
		return GetToolbarDisabledTextColor();
	}

	return pHyperLink->IsHighlighted() ? afxGlobalData.clrHotLinkHoveredText : afxGlobalData.clrHotLinkNormalText;
}

COLORREF CMFCVisualManager::GetRibbonStatusBarTextColor(CMFCRibbonStatusBar* /*pStatusBar*/)
{
	return afxGlobalData.clrBarText;
}

void CMFCVisualManager::OnDrawRibbonColorPaletteBox(CDC* pDC, CMFCRibbonColorButton* /*pColorButton*/, CMFCRibbonGalleryIcon* /*pIcon*/,
	COLORREF color, CRect rect, BOOL bDrawTopEdge, BOOL bDrawBottomEdge, BOOL bIsHighlighted, BOOL bIsChecked, BOOL /*bIsDisabled*/)
{
	ASSERT_VALID(pDC);

	CRect rectFill = rect;
	rectFill.DeflateRect(1, 0);

	if (bIsHighlighted || bIsChecked)
	{
		CMFCToolBarImages::FillDitheredRect(pDC, rect);
		rectFill.DeflateRect(1, 2);
	}

	if (color != (COLORREF)-1)
	{
		CBrush br(color);
		pDC->FillRect(rectFill, &br);
	}

	COLORREF clrBorder = afxGlobalData.clrBtnShadow;

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

	if (bIsHighlighted)
	{
		pDC->Draw3dRect(&rect, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
	}
	else if (bIsChecked)
	{
		pDC->Draw3dRect(&rect, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
	}
}

BOOL CMFCVisualManager::OnSetWindowRegion(CWnd* pWnd, CSize sizeWindow)
{
	if (afxGlobalData.DwmIsCompositionEnabled())
	{
		return FALSE;
	}

	ASSERT_VALID(pWnd);

	CMFCRibbonBar* pRibbonBar = NULL;

	if (pWnd->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		pRibbonBar = ((CFrameWndEx*) pWnd)->GetRibbonBar();
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		pRibbonBar = ((CMDIFrameWndEx*) pWnd)->GetRibbonBar();
	}

	if (pRibbonBar == NULL || !pRibbonBar->IsWindowVisible() || !pRibbonBar->IsReplaceFrameCaption())
	{
		return FALSE;
	}

	const int nLeftRadius  = 11;
	const int nRightRadius = 11;

	CRgn rgnWnd;
	rgnWnd.CreateRectRgn(0, 0, sizeWindow.cx, sizeWindow.cy);

	CRgn rgnTemp;

	rgnTemp.CreateRectRgn(0, 0, nLeftRadius / 2, nLeftRadius / 2);
	rgnWnd.CombineRgn(&rgnTemp, &rgnWnd, RGN_XOR);

	rgnTemp.DeleteObject();
	rgnTemp.CreateEllipticRgn(0, 0, nLeftRadius, nLeftRadius);
	rgnWnd.CombineRgn(&rgnTemp, &rgnWnd, RGN_OR);

	rgnTemp.DeleteObject();
	rgnTemp.CreateRectRgn(sizeWindow.cx - nRightRadius / 2, 0, sizeWindow.cx, nRightRadius / 2);
	rgnWnd.CombineRgn(&rgnTemp, &rgnWnd, RGN_XOR);

	rgnTemp.DeleteObject();
	rgnTemp.CreateEllipticRgn(sizeWindow.cx - nRightRadius + 1, 0, sizeWindow.cx + 1, nRightRadius);
	rgnWnd.CombineRgn(&rgnTemp, &rgnWnd, RGN_OR);

	pWnd->SetWindowRgn((HRGN)rgnWnd.Detach(), TRUE);
	return TRUE;
}

BOOL CMFCVisualManager::OnNcPaint(CWnd* /*pWnd*/, const CObList& /*lstSysButtons*/, CRect /*rectRedraw*/)
{
	return FALSE;
}

void CMFCVisualManager::OnActivateApp(CWnd* /*pWnd*/, BOOL /*bActive*/)
{
}

BOOL CMFCVisualManager::OnNcActivate(CWnd* /*pWnd*/, BOOL /*bActive*/)
{
	return FALSE;
}

CSize CMFCVisualManager::GetNcBtnSize(BOOL /*bSmall*/) const
{
	return CSize(0, 0);
}

BOOL CMFCVisualManager::OnEraseMDIClientArea(CDC* /*pDC*/, CRect /*rectClient*/)
{
	return FALSE;
}

BOOL CMFCVisualManager::GetToolTipInfo(CMFCToolTipInfo& params, UINT /*nType*/ /*= (UINT)(-1)*/)
{
	CMFCToolTipInfo dummy;
	params = dummy;

	return TRUE;
}

BOOL CMFCVisualManager::DrawTextOnGlass(CDC* pDC, CString strText, CRect rect, DWORD dwFlags, int nGlowSize, COLORREF clrText)
{
	ASSERT_VALID(pDC);

	COLORREF clrOldText = pDC->GetTextColor();
	pDC->SetTextColor(RGB(0, 0, 0));

	BOOL bRes = afxGlobalData.DrawTextOnGlass(m_hThemeButton, pDC, 0, 0, strText, rect, dwFlags, nGlowSize, clrText);

	pDC->SetTextColor(clrOldText);

	return bRes;
}

COLORREF CMFCVisualManager::OnDrawPropertySheetListItem(CDC* pDC, CMFCPropertySheet* /*pParent*/, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	ASSERT_VALID(pDC);

	COLORREF clrText = (COLORREF)-1;

	if (bIsSelected)
	{
		pDC->FillRect(rect, &afxGlobalData.brHilite);
		clrText = afxGlobalData.clrTextHilite;
	}

	if (bIsHighlihted)
	{
		pDC->DrawFocusRect(rect);
	}

	return clrText;
}

/////////////////////////////////////////////////////////////////////////////////////
// CMFCBaseVisualManager

CMFCBaseVisualManager::CMFCBaseVisualManager()
{
	m_hThemeWindow = NULL;
	m_hThemeToolBar = NULL;
	m_hThemeButton = NULL;
	m_hThemeStatusBar = NULL;
	m_hThemeRebar = NULL;
	m_hThemeComboBox = NULL;
	m_hThemeProgress = NULL;
	m_hThemeHeader = NULL;
	m_hThemeScrollBar = NULL;
	m_hThemeExplorerBar = NULL;
	m_hThemeTree = NULL;
	m_hThemeStartPanel = NULL;
	m_hThemeTaskBand = NULL;
	m_hThemeTaskBar = NULL;
	m_hThemeSpin = NULL;
	m_hThemeTab = NULL;
	m_hThemeTrack = NULL;

	m_hinstUXDLL = ::AfxCtxLoadLibrary(_T("UxTheme.dll"));

	if (m_hinstUXDLL != NULL)
	{
		m_pfOpenThemeData = (OPENTHEMEDATA)::GetProcAddress(m_hinstUXDLL, "OpenThemeData");
		m_pfCloseThemeData = (CLOSETHEMEDATA)::GetProcAddress(m_hinstUXDLL, "CloseThemeData");
		m_pfDrawThemeBackground = (DRAWTHEMEBACKGROUND)::GetProcAddress(m_hinstUXDLL, "DrawThemeBackground");
		m_pfGetThemeColor = (GETTHEMECOLOR)::GetProcAddress(m_hinstUXDLL, "GetThemeColor");
		m_pfGetThemeSysColor = (GETTHEMESYSCOLOR)::GetProcAddress(m_hinstUXDLL, "GetThemeSysColor");
		m_pfGetCurrentThemeName = (GETCURRENTTHEMENAME)::GetProcAddress(m_hinstUXDLL, "GetCurrentThemeName");
		m_pfGetWindowTheme = (GETWINDOWTHEME)::GetProcAddress(m_hinstUXDLL, "GetWindowTheme");

		UpdateSystemColors();
	}
	else
	{
		m_pfOpenThemeData = NULL;
		m_pfCloseThemeData = NULL;
		m_pfDrawThemeBackground = NULL;
		m_pfGetThemeColor = NULL;
		m_pfGetThemeSysColor = NULL;
		m_pfGetCurrentThemeName = NULL;
		m_pfGetWindowTheme = NULL;
	}
}

CMFCBaseVisualManager::~CMFCBaseVisualManager()
{
	if (m_hinstUXDLL != NULL)
	{
		CleanUpThemes();
		::FreeLibrary(m_hinstUXDLL);
	}
}

void CMFCBaseVisualManager::UpdateSystemColors()
{
	if (m_hinstUXDLL != NULL)
	{
		CleanUpThemes();

		if (m_pfOpenThemeData == NULL ||
			m_pfCloseThemeData == NULL ||
			m_pfDrawThemeBackground == NULL)
		{
			ASSERT(FALSE);
		}
		else
		{
			m_hThemeWindow = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"WINDOW");
			m_hThemeToolBar = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"TOOLBAR");
			m_hThemeButton = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"BUTTON");
			m_hThemeStatusBar = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"STATUS");
			m_hThemeRebar = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"REBAR");
			m_hThemeComboBox = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"COMBOBOX");
			m_hThemeProgress = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"PROGRESS");
			m_hThemeHeader = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"HEADER");
			m_hThemeScrollBar = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"SCROLLBAR");
			m_hThemeExplorerBar = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"EXPLORERBAR");
			m_hThemeTree = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"TREEVIEW");
			m_hThemeStartPanel = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"STARTPANEL");
			m_hThemeTaskBand = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"TASKBAND");
			m_hThemeTaskBar = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"TASKBAR");
			m_hThemeSpin = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"SPIN");
			m_hThemeTab = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"TAB");
			m_hThemeTrack = (*m_pfOpenThemeData)(AfxGetMainWnd()->GetSafeHwnd(), L"TRACKBAR");
		}
	}
}

void CMFCBaseVisualManager::CleanUpThemes()
{
	if (m_pfCloseThemeData == NULL)
	{
		return;
	}

	if (m_hThemeWindow != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeWindow);
	}

	if (m_hThemeToolBar != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeToolBar);
	}

	if (m_hThemeRebar != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeRebar);
	}

	if (m_hThemeStatusBar != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeStatusBar);
	}

	if (m_hThemeButton != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeButton);
	}

	if (m_hThemeComboBox != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeComboBox);
	}

	if (m_hThemeProgress != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeProgress);
	}

	if (m_hThemeHeader != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeHeader);
	}

	if (m_hThemeScrollBar != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeScrollBar);
	}

	if (m_hThemeExplorerBar != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeExplorerBar);
	}

	if (m_hThemeTree != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeTree);
	}

	if (m_hThemeStartPanel != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeStartPanel);
	}

	if (m_hThemeTaskBand != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeTaskBand);
	}

	if (m_hThemeTaskBar != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeTaskBar);
	}

	if (m_hThemeSpin != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeSpin);
	}

	if (m_hThemeTab != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeTab);
	}

	if (m_hThemeTrack != NULL)
	{
		(*m_pfCloseThemeData)(m_hThemeTrack);
	}
}

BOOL CMFCBaseVisualManager::DrawPushButton(CDC* pDC, CRect rect, CMFCButton* pButton, UINT /*uiState*/)
{
	if (m_hThemeButton == NULL)
	{
		return FALSE;
	}

	int nState = PBS_NORMAL;

	if (!pButton->IsWindowEnabled())
	{
		nState = PBS_DISABLED;
	}
	else if (pButton->IsPressed() || pButton->GetCheck())
	{
		nState = PBS_PRESSED;
	}
	else if (pButton->IsHighlighted())
	{
		nState = PBS_HOT;
	}
	else if (CWnd::GetFocus() == pButton)
	{
		nState = PBS_DEFAULTED;
	}

	pButton->OnDrawParentBackground(pDC, rect);

	if (m_pfDrawThemeBackground)
	{
		(*m_pfDrawThemeBackground)(m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, nState, &rect, 0);
	}

	return TRUE;
}

BOOL CMFCBaseVisualManager::DrawStatusBarProgress(CDC* pDC, CMFCStatusBar* /*pStatusBar*/, CRect rectProgress, int nProgressTotal,
	int nProgressCurr, COLORREF /*clrBar*/, COLORREF /*clrProgressBarDest*/, COLORREF /*clrProgressText*/, BOOL bProgressText)
{
	if (m_hThemeProgress == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pDC);

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeProgress, pDC->GetSafeHdc(), PP_BAR, 0, &rectProgress, 0);
	}

	if (nProgressTotal == 0)
	{
		return TRUE;
	}

	CRect rectComplete = rectProgress;
	rectComplete.DeflateRect(3, 3);

	rectComplete.right = rectComplete.left + nProgressCurr * rectComplete.Width() / nProgressTotal;

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeProgress, pDC->GetSafeHdc(), PP_CHUNK, 0, &rectComplete, 0);
	}

	if (bProgressText)
	{
		CString strText;
		strText.Format(_T("%d%%"), nProgressCurr * 100 / nProgressTotal);

		COLORREF clrText = pDC->SetTextColor(afxGlobalData.clrBtnText);
		pDC->DrawText(strText, rectProgress, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		pDC->SetTextColor(clrText);
	}

	return TRUE;
}

BOOL CMFCBaseVisualManager::DrawComboDropButton(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted)
{
	if (m_hThemeComboBox == NULL)
	{
		return FALSE;
	}

	int nState = bDisabled ? CBXS_DISABLED : bIsDropped ? CBXS_PRESSED : bIsHighlighted ? CBXS_HOT : CBXS_NORMAL;
	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeComboBox, pDC->GetSafeHdc(), CP_DROPDOWNBUTTON, nState, &rect, 0);
	}
	return TRUE;
}

BOOL CMFCBaseVisualManager::DrawComboBorder(CDC* pDC, CRect rect, BOOL /*bDisabled*/, BOOL bIsDropped, BOOL bIsHighlighted)
{
	ASSERT_VALID(pDC);

	if (m_hThemeWindow == NULL)
	{
		return FALSE;
	}

	if (bIsHighlighted || bIsDropped)
	{
		rect.DeflateRect(1, 1);
		pDC->Draw3dRect(&rect,  afxGlobalData.clrHilite, afxGlobalData.clrHilite);
	}

	return TRUE;
}

void CMFCBaseVisualManager::FillReBarPane(CDC* pDC, CBasePane* pBar, CRect rectClient)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBar);

	if (m_pfDrawThemeBackground == NULL || m_hThemeRebar == NULL)
	{
		pDC->FillRect(rectClient, &afxGlobalData.brBarFace);
		return;
	}

	CWnd* pWndParent = AFXGetParentFrame(pBar);
	if (pWndParent->GetSafeHwnd() == NULL)
	{
		pWndParent = pBar->GetParent();
	}

	ASSERT_VALID(pWndParent);

	CRect rectParent;
	pWndParent->GetWindowRect(rectParent);

	pBar->ScreenToClient(&rectParent);

	rectClient.right = max(rectClient.right, rectParent.right);
	rectClient.bottom = max(rectClient.bottom, rectParent.bottom);

	if (!pBar->IsFloating() && pBar->GetParentMiniFrame() == NULL)
	{
		rectClient.left = rectParent.left;
		rectClient.top = rectParent.top;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeRebar, pDC->GetSafeHdc(), 0, 0, &rectClient, 0);
	}
}

BOOL CMFCBaseVisualManager::DrawCheckBox(CDC *pDC, CRect rect, BOOL bHighlighted, int nState, BOOL bEnabled, BOOL bPressed)
{
	if (m_hThemeButton == NULL)
	{
		return FALSE;
	}

	nState = max(0, nState);
	nState = min(2, nState);

	ASSERT_VALID(pDC);

	int nDrawState = nState == 1 ? CBS_CHECKEDNORMAL : nState == 2 ? CBS_MIXEDNORMAL : CBS_UNCHECKEDNORMAL;

	if (!bEnabled)
	{
		nDrawState = nState == 1 ? CBS_CHECKEDDISABLED : nState == 2 ? CBS_MIXEDDISABLED : CBS_UNCHECKEDDISABLED;
	}
	else if (bPressed)
	{
		nDrawState = nState == 1 ? CBS_CHECKEDPRESSED : nState == 2 ? CBS_MIXEDPRESSED : CBS_UNCHECKEDPRESSED;
	}
	else if (bHighlighted)
	{
		nDrawState = nState == 1 ? CBS_CHECKEDHOT : nState == 2 ? CBS_MIXEDHOT : CBS_UNCHECKEDHOT;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeButton, pDC->GetSafeHdc(), BP_CHECKBOX, nDrawState, &rect, 0);
	}

	return TRUE;
}

BOOL CMFCBaseVisualManager::DrawRadioButton(CDC *pDC, CRect rect, BOOL bHighlighted, BOOL bChecked, BOOL bEnabled, BOOL bPressed)
{
/*	#define RBS_UNCHECKEDNORMAL 1
	#define RBS_UNCHECKEDHOT 2
	#define RBS_UNCHECKEDPRESSED 3
	#define RBS_UNCHECKEDDISABLED 4
	#define RBS_CHECKEDNORMAL 5
	#define RBS_CHECKEDHOT 6
	#define RBS_CHECKEDPRESSED 7
	#define RBS_CHECKEDDISABLED 8

	#define BP_RADIOBUTTON 2	*/

	if (m_hThemeButton == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pDC);

	int nDrawState = bChecked ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;

	if (!bEnabled)
	{
		nDrawState = bChecked ? RBS_CHECKEDDISABLED : RBS_UNCHECKEDDISABLED;
	}
	else if (bPressed)
	{
		nDrawState = bChecked ? RBS_CHECKEDPRESSED : RBS_UNCHECKEDPRESSED;
	}
	else if (bHighlighted)
	{
		nDrawState = bChecked ? RBS_CHECKEDHOT : RBS_UNCHECKEDHOT;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeButton, pDC->GetSafeHdc(), BP_RADIOBUTTON, nDrawState, &rect, 0);
	}

	return TRUE;
}

CMFCBaseVisualManager::WinXpTheme CMFCBaseVisualManager::GetStandardWindowsTheme()
{
	WCHAR szName [256] = L"";
	WCHAR szColor [256] = L"";

	if (m_pfGetCurrentThemeName == NULL || (*m_pfGetCurrentThemeName)(szName, 255, szColor, 255, NULL, 0) != S_OK)
	{
		return WinXpTheme_None;
	}

	CString strThemeName = szName;
	CString strWinXPThemeColor = szColor;

	TCHAR fname[_MAX_FNAME];
	_tsplitpath_s(strThemeName, NULL, 0, NULL, 0, fname, _MAX_FNAME, NULL, 0);

	strThemeName = fname;

	if (strThemeName.CompareNoCase(_T("Luna")) != 0 && 
		strThemeName.CompareNoCase (_T("Aero")) != 0)
	{
		return WinXpTheme_NonStandard;
	}

	// Check for 3-d party visual managers:
	if (m_pfGetThemeColor != NULL && m_hThemeButton != NULL)
	{
		COLORREF clrTest = 0;
		if ((*m_pfGetThemeColor)(m_hThemeButton, 1, 0, 3823, &clrTest) != S_OK || clrTest == 1)
		{
			return WinXpTheme_NonStandard;
		}
	}

	if (strWinXPThemeColor.CompareNoCase(_T("normalcolor")) == 0)
	{
		return WinXpTheme_Blue;
	}

	if (strWinXPThemeColor.CompareNoCase(_T("homestead")) == 0)
	{
		return WinXpTheme_Olive;
	}

	if (strWinXPThemeColor.CompareNoCase(_T("metallic")) == 0)
	{
		// Check for Royale theme:
		CString strThemeLower = szName;
		strThemeLower.MakeLower();

		if (strThemeLower.Find(_T("royale")) >= 0)
		{
			return WinXpTheme_NonStandard;
		}

		return WinXpTheme_Silver;
	}

	return WinXpTheme_NonStandard;
}



