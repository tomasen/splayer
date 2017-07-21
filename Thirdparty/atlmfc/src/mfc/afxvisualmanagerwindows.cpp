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
#include "afxvisualmanagerwindows.h"
#include "afxpopupmenubar.h"
#include "afxbutton.h"
#include "afxdrawmanager.h"
#include "afxbasetabctrl.h"
#include "afxdockablepane.h"
#include "afxtaskspane.h"
#include "afxtoolbarmenubutton.h"
#include "afxoutlookbarpanebutton.h"
#include "afxstatusbar.h"
#include "afxcaptionbar.h"
#include "afxdesktopalertwnd.h"
#include "afxdocksite.h"
#include "afxribbonstatusbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCVisualManagerWindows, CMFCVisualManagerOfficeXP)

BOOL CMFCVisualManagerWindows::m_b3DTabsXPTheme = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCVisualManagerWindows::CMFCVisualManagerWindows(BOOL bIsTemporary) : CMFCVisualManagerOfficeXP(bIsTemporary)
{
	m_bShadowHighlightedImage = FALSE;

	m_bOfficeStyleMenus = FALSE;

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

	afxGlobalData.UpdateSysColors();
	OnUpdateSystemColors();
}

CMFCVisualManagerWindows::~CMFCVisualManagerWindows()
{
}

BOOL __stdcall CMFCVisualManagerWindows::IsWinXPThemeAvailable()
{
	CMFCVisualManagerWindows* pWinXPManager = DYNAMIC_DOWNCAST(CMFCVisualManagerWindows, m_pVisManager);
	if (pWinXPManager != NULL)
	{
		return pWinXPManager->m_hThemeWindow != NULL;
	}

	// Create a temporary manager and check it:
	CMFCVisualManagerWindows winXPManager(TRUE /* Temporary */);
	return winXPManager.m_hThemeWindow != NULL;
}

void CMFCVisualManagerWindows::SetOfficeStyleMenus(BOOL bOn)
{
	m_bOfficeStyleMenus = bOn;
}

void CMFCVisualManagerWindows::OnUpdateSystemColors()
{
	CMFCVisualManagerOfficeXP::OnUpdateSystemColors();

	m_bShadowHighlightedImage = TRUE;

	CMFCBaseVisualManager::UpdateSystemColors();

	if (m_hThemeWindow != NULL)
	{
		m_bShadowHighlightedImage = FALSE;
	}

	if (m_pfGetThemeColor != NULL)
	{
		if (m_hThemeToolBar != NULL)
		{
			(*m_pfGetThemeColor)(m_hThemeToolBar, TP_BUTTON, 0, TMT_TEXTCOLOR, &afxGlobalData.clrBarText);
			(*m_pfGetThemeColor)(m_hThemeToolBar, TP_BUTTON, 0, TMT_FILLCOLOR, &afxGlobalData.clrBarFace);
			(*m_pfGetThemeColor)(m_hThemeToolBar, TP_BUTTON, 0, TMT_EDGELIGHTCOLOR, &afxGlobalData.clrBarLight);
			(*m_pfGetThemeColor)(m_hThemeToolBar, TP_BUTTON, 0, TMT_EDGEHIGHLIGHTCOLOR, &afxGlobalData.clrBarHilite);
			(*m_pfGetThemeColor)(m_hThemeToolBar, TP_BUTTON, 0, TMT_EDGESHADOWCOLOR, &afxGlobalData.clrBarShadow);
			(*m_pfGetThemeColor)(m_hThemeToolBar, TP_BUTTON, 0, TMT_EDGEDKSHADOWCOLOR, &afxGlobalData.clrBarDkShadow);
		}

		if (m_hThemeButton != NULL)
		{
			(*m_pfGetThemeColor)(m_hThemeButton, BP_PUSHBUTTON, 0, TMT_TEXTCOLOR, &afxGlobalData.clrBtnText);
			(*m_pfGetThemeColor)(m_hThemeButton, BP_PUSHBUTTON, 0, TMT_FILLCOLOR, &afxGlobalData.clrBtnFace);
			(*m_pfGetThemeColor)(m_hThemeButton, BP_PUSHBUTTON, 0, TMT_EDGELIGHTCOLOR, &afxGlobalData.clrBtnLight);
			(*m_pfGetThemeColor)(m_hThemeButton, BP_PUSHBUTTON, 0, TMT_EDGEHIGHLIGHTCOLOR, &afxGlobalData.clrBtnHilite);
			(*m_pfGetThemeColor)(m_hThemeButton, BP_PUSHBUTTON, 0, TMT_EDGESHADOWCOLOR, &afxGlobalData.clrBtnShadow);
			(*m_pfGetThemeColor)(m_hThemeButton, BP_PUSHBUTTON, 0, TMT_EDGEDKSHADOWCOLOR, &afxGlobalData.clrBtnDkShadow);
		}
	}
}

void CMFCVisualManagerWindows::OnDrawBarGripper(CDC* pDC, CRect rectGripper, BOOL bHorz, CBasePane* pBar)
{
	ASSERT_VALID(pDC);

	if (m_hThemeRebar == NULL || pBar->GetSafeHwnd() == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawBarGripper(pDC, rectGripper, bHorz, pBar);
		return;
	}

	BOOL bSideBar = pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CDockablePane));

	CRect rectFill = rectGripper;

	if (bSideBar)
	{
		bHorz = !bHorz;
	}

	COLORREF clrTextOld = pDC->SetTextColor(afxGlobalData.clrBtnShadow);
	COLORREF clrBkOld = pDC->SetBkColor(afxGlobalData.clrBtnFace);

	CRect rectGripperTheme = rectGripper;
	const int nGripperOffset = 2;

	if (bHorz)
	{
		rectGripperTheme.DeflateRect(0, nGripperOffset);
		rectGripperTheme.OffsetRect(nGripperOffset, 0);
		rectGripperTheme.right = rectGripperTheme.left + 3 * nGripperOffset;
	}
	else
	{
		rectGripperTheme.DeflateRect(nGripperOffset, 0);
		rectGripperTheme.OffsetRect(0, nGripperOffset);
		rectGripperTheme.bottom = rectGripperTheme.top + 3 * nGripperOffset;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeRebar, pDC->GetSafeHdc(), bHorz ? RP_GRIPPER : RP_GRIPPERVERT, 0, &rectGripperTheme, 0);
	}

	if (bSideBar)
	{
		//------------------
		// Draw bar caption:
		//------------------
		int nOldBkMode = pDC->SetBkMode(OPAQUE);
		pDC->SetTextColor(afxGlobalData.clrBtnText);

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

void CMFCVisualManagerWindows::OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL bNCArea)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCStatusBar)) && m_hThemeStatusBar != NULL && m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeStatusBar, pDC->GetSafeHdc(), 0, 0, &rectClient, 0);
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonStatusBar)))
	{
		if (m_hThemeStatusBar != NULL && m_pfDrawThemeBackground != NULL)
		{
			(*m_pfDrawThemeBackground)(m_hThemeStatusBar, pDC->GetSafeHdc(), 0, 0, &rectClient, 0);
			return;
		}
	}

	if (m_pfDrawThemeBackground == NULL || m_hThemeRebar == NULL || pBar->IsDialogControl() || pBar->IsKindOf(RUNTIME_CLASS(CMFCCaptionBar)))
	{
		CMFCVisualManagerOfficeXP::OnFillBarBackground(pDC, pBar,rectClient, rectClip, bNCArea);
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
	{
		if (m_bOfficeStyleMenus)
		{
			CMFCVisualManagerOfficeXP::OnFillBarBackground(pDC, pBar,rectClient, rectClip, bNCArea);
		}
		else
		{
			::FillRect(pDC->GetSafeHdc(), rectClient, ::GetSysColorBrush(COLOR_MENU));
		}

		return;
	}

	FillReBarPane(pDC, pBar, rectClient);
}

void CMFCVisualManagerWindows::OnDrawPaneBorder(CDC* pDC, CBasePane* pBar, CRect& rect)
{
	CMFCVisualManager::OnDrawPaneBorder(pDC, pBar, rect);
}

void CMFCVisualManagerWindows::OnFillButtonInterior(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	if (m_hThemeToolBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnFillButtonInterior(pDC, pButton, rect, state);
		return;
	}

	BOOL bIsMenuBar = FALSE;
	BOOL bIsPopupMenu = FALSE;

	CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
	if (pMenuButton != NULL)
	{
		bIsMenuBar = pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCMenuBar));
		bIsPopupMenu = pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));
	}

	if (m_bOfficeStyleMenus &&(bIsPopupMenu || bIsMenuBar))
	{
		CMFCVisualManagerOfficeXP::OnFillButtonInterior(pDC, pButton, rect, state);
		return;
	}

	if (bIsPopupMenu && state != ButtonsIsHighlighted && state != ButtonsIsPressed)
	{
		return;
	}

	int nState = TS_NORMAL;

	if (pButton->m_nStyle & TBBS_DISABLED)
	{
		nState = TS_DISABLED;
	}
	else if ((pButton->m_nStyle & TBBS_PRESSED) && state == ButtonsIsHighlighted)
	{
		nState = TS_PRESSED;
	}
	else if (pButton->m_nStyle & TBBS_CHECKED)
	{
		nState = (state == ButtonsIsHighlighted) ? TS_HOTCHECKED : TS_CHECKED;
	}
	else if (state == ButtonsIsHighlighted)
	{
		nState = TS_HOT;

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown())
		{
			nState = TS_PRESSED;
			rect.bottom--;
		}
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);
	}
}

COLORREF CMFCVisualManagerWindows::GetToolbarButtonTextColor(CMFCToolBarButton* pButton, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	ASSERT_VALID(pButton);

	if (m_hThemeToolBar == NULL || pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton)))
	{
		return CMFCVisualManagerOfficeXP::GetToolbarButtonTextColor(pButton, state);
	}

	return CMFCVisualManager::GetToolbarButtonTextColor(pButton, state);
}

void CMFCVisualManagerWindows::OnHighlightMenuItem(CDC*pDC, CMFCToolBarMenuButton* pButton, CRect rect, COLORREF& clrText)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		CMFCVisualManagerOfficeXP::OnHighlightMenuItem(pDC, pButton, rect, clrText);
	}
	else
	{
		CMFCVisualManager::OnHighlightMenuItem(pDC, pButton, rect, clrText);
	}
}

COLORREF CMFCVisualManagerWindows::GetHighlightedMenuItemTextColor(CMFCToolBarMenuButton* pButton)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		return CMFCVisualManagerOfficeXP::GetHighlightedMenuItemTextColor(pButton);
	}
	else
	{
		return CMFCVisualManager::GetHighlightedMenuItemTextColor(pButton);
	}
}

void CMFCVisualManagerWindows::OnHighlightRarelyUsedMenuItems(CDC* pDC, CRect rectRarelyUsed)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		CMFCVisualManagerOfficeXP::OnHighlightRarelyUsedMenuItems(pDC, rectRarelyUsed);
		return;
	}

	ASSERT_VALID(pDC);

	CDrawingManager dm(*pDC);

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CMFCToolBar::GetMenuImageSize().cx + 2 * GetMenuImageMargin() + 2;
	dm.HighlightRect(rectRarelyUsed, 94);
}

void CMFCVisualManagerWindows::OnDrawButtonBorder(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, AFX_BUTTON_STATE state)
{
	if (m_hThemeToolBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawButtonBorder(pDC, pButton, rect, state);
		return;
	}

	if (m_bOfficeStyleMenus)
	{
		CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
		if (pMenuButton != NULL)
		{
			BOOL bIsMenuBar = pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCMenuBar));
			BOOL bIsPopupMenu = pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));

			if (bIsPopupMenu || bIsMenuBar)
			{
				CMFCVisualManagerOfficeXP::OnDrawButtonBorder(pDC, pButton, rect, state);
				return;
			}
		}
	}
}

void CMFCVisualManagerWindows::OnDrawButtonSeparator(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state, BOOL bHorz)
{
	if (m_hThemeToolBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawButtonSeparator(pDC, pButton, rect, state, bHorz);
		return;
	}

	rect.InflateRect(2, 2);

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeToolBar, pDC->GetSafeHdc(), bHorz ? TP_SEPARATOR : TP_SEPARATORVERT, 0, &rect, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawSeparator(CDC* pDC, CBasePane* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID(pBar);

	if (m_hThemeToolBar == NULL || pBar->IsDialogControl())
	{
		CMFCVisualManagerOfficeXP::OnDrawSeparator(pDC, pBar, rect, bHorz);
		return;
	}

	if (m_bOfficeStyleMenus && pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
	{
		CMFCVisualManagerOfficeXP::OnDrawSeparator(pDC, pBar, rect, bHorz);
		return;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeToolBar, pDC->GetSafeHdc(), bHorz ? TP_SEPARATOR : TP_SEPARATORVERT, 0, &rect, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawCaptionButton(CDC* pDC, CMFCCaptionButton* pButton, BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled, int nImageID /*= -1*/)
{
	if (m_hThemeToolBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawCaptionButton(pDC, pButton, bActive, bHorz, bMaximized, bDisabled, nImageID);
		return;
	}

	ENSURE(pButton != NULL);
	ASSERT_VALID(pButton);
	ASSERT_VALID(pDC);

	BOOL bPushed = pButton->m_bPushed;
	BOOL bFocused = pButton->m_bFocused || pButton->m_bDroppedDown;

	int nState = TS_NORMAL;
	if (bDisabled)
	{
		nState = TS_DISABLED;
	}
	else if (bPushed && bFocused)
	{
		nState = TS_PRESSED;
	}
	else if (bFocused)
	{
		nState = TS_HOT;
	}

	CRect rect = pButton->GetRect();

	if (pButton->IsMiniFrameButton())
	{
		rect.bottom -= 3;
		rect.right -= 2;
	}

	int nPart = 0;
	HTHEME hTheme = NULL;

	if (nImageID == -1)
	{
		switch(pButton->m_nHit)
		{
		case HTCLOSE:
		case AFX_HTCLOSE:

			if (pButton->IsMiniFrameButton())
			{
				nPart = WP_SMALLCLOSEBUTTON;
				hTheme = m_hThemeWindow;
			}
			else
			{
				nPart = EBP_HEADERCLOSE;
				hTheme = m_hThemeExplorerBar;
			}

			break;

		case HTMAXBUTTON:
			nPart = EBP_HEADERPIN;
			hTheme = m_hThemeExplorerBar;

			if (!bMaximized)
			{
				nState += 3;
			}
			break;
		}
	}

	if (nPart == 0 || hTheme == NULL)
	{
		if (!pButton->IsMiniFrameButton())
		{
			rect.DeflateRect(1, 2);
		}

		if (m_pfDrawThemeBackground != NULL)
		{
			(*m_pfDrawThemeBackground)(m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);
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
			CPoint ptImage(rect.left +(rect.Width() - sizeImage.cx) / 2, rect.top +(rect.Height() - sizeImage.cy) / 2);

			OnDrawCaptionButtonIcon(pDC, pButton, id, bActive, bDisabled, ptImage);
		}
	}
	else if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(hTheme, pDC->GetSafeHdc(), nPart, nState, &rect, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawCaptionButtonIcon(CDC* pDC, CMFCCaptionButton* pButton, CMenuImages::IMAGES_IDS id, BOOL bActive, BOOL bDisabled, CPoint ptImage)
{
	BOOL bFocused = pButton->m_bFocused || pButton->m_bDroppedDown;

	if (m_hThemeButton == NULL || !bFocused)
	{
		CMFCVisualManagerOfficeXP::OnDrawCaptionButtonIcon(pDC, pButton, id, bActive,
			bDisabled, ptImage);
		return;
	}

	ASSERT_VALID(pDC);
	CMenuImages::Draw(pDC, id, ptImage, CMenuImages::ImageBlack);
}

COLORREF CMFCVisualManagerWindows::OnFillCommandsListBackground(CDC* pDC, CRect rect, BOOL bIsSelected)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		return CMFCVisualManagerOfficeXP::OnFillCommandsListBackground(pDC, rect, bIsSelected);
	}

	::FillRect(pDC->GetSafeHdc(), rect, ::GetSysColorBrush(COLOR_MENU));

	if (bIsSelected)
	{
		pDC->FillRect(rect, &afxGlobalData.brHilite);
		pDC->Draw3dRect(rect, afxGlobalData.clrMenuText, afxGlobalData.clrMenuText);

		return afxGlobalData.clrTextHilite;
	}

	return afxGlobalData.clrMenuText;
}

void CMFCVisualManagerWindows::OnDrawTearOffCaption(CDC* pDC, CRect rect, BOOL bIsActive)
{
	CMFCVisualManagerOfficeXP::OnDrawTearOffCaption(pDC, rect, bIsActive);
}

void CMFCVisualManagerWindows::OnDrawMenuSystemButton(CDC* pDC, CRect rect, UINT uiSystemCommand, UINT nStyle, BOOL bHighlight)
{
	if (m_hThemeWindow == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawMenuSystemButton(pDC, rect, uiSystemCommand, nStyle, bHighlight);
		return;
	}

	int nPart;
	switch(uiSystemCommand)
	{
	case SC_CLOSE:
		nPart = WP_MDICLOSEBUTTON;
		break;

	case SC_RESTORE:
		nPart = WP_MDIRESTOREBUTTON;
		break;

	case SC_MINIMIZE:
		nPart = WP_MDIMINBUTTON;
		break;

	default:
		return;
	}

	BOOL bIsDisabled = (nStyle & TBBS_DISABLED);
	BOOL bIsPressed = (nStyle & TBBS_PRESSED);

	int nState = CBS_NORMAL;
	if (bIsDisabled)
	{
		nState = CBS_DISABLED;
	}
	else if (bIsPressed && bHighlight)
	{
		nState = CBS_PUSHED;
	}
	else if (bHighlight)
	{
		nState = CBS_HOT;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeWindow, pDC->GetSafeHdc(), nPart, nState, &rect, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawStatusBarPaneBorder(CDC* pDC, CMFCStatusBar* pBar, CRect rectPane, UINT uiID, UINT nStyle)
{
	if (m_hThemeStatusBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawStatusBarPaneBorder(pDC, pBar, rectPane, uiID, nStyle);
		return;
	}

	if (!(nStyle & SBPS_NOBORDERS) && m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeStatusBar, pDC->GetSafeHdc(), SP_PANE, 0, &rectPane, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawStatusBarSizeBox(CDC* pDC, CMFCStatusBar* pStatBar, CRect rectSizeBox)
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

void CMFCVisualManagerWindows::OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* pMenu, CRect rect)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		ASSERT_VALID(pMenu);

		BOOL bConnectMenuToParent = m_bConnectMenuToParent;
		m_bConnectMenuToParent = FALSE;

		if (m_hThemeWindow == NULL)
		{
			m_bConnectMenuToParent = TRUE;
		}
		else if (!CMFCToolBar::IsCustomizeMode())
		{
			CMFCToolBarMenuButton* pMenuButton = pMenu->GetParentButton();

			if (pMenuButton != NULL)
			{
				BOOL bIsMenuBar = pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCMenuBar));
				if (bIsMenuBar)
				{
					m_bConnectMenuToParent = TRUE;
				}
			}
		}

		CMFCVisualManagerOfficeXP::OnDrawMenuBorder(pDC, pMenu, rect);

		m_bConnectMenuToParent = bConnectMenuToParent;
	}
	else
	{
		CMFCVisualManager::OnDrawMenuBorder(pDC, pMenu, rect);
	}
}

void CMFCVisualManagerWindows::OnDrawComboDropButton(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* pButton)
{
	if (m_hThemeComboBox == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawComboDropButton(pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	int nState = bDisabled ? CBXS_DISABLED : bIsDropped ? CBXS_PRESSED : bIsHighlighted ? CBXS_HOT : CBXS_NORMAL;

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeComboBox, pDC->GetSafeHdc(), CP_DROPDOWNBUTTON, nState, &rect, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawComboBorder(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* pButton)
{
	if (m_hThemeWindow == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawComboBorder(pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	if (bIsHighlighted || bIsDropped)
	{
		rect.DeflateRect(1, 1);
		pDC->Draw3dRect(&rect,  afxGlobalData.clrHilite, afxGlobalData.clrHilite);
	}
}

void CMFCVisualManagerWindows::OnEraseTabsButton(CDC* pDC, CRect rect, CMFCButton* pButton, CMFCBaseTabCtrl* pWndTab)
{
	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pWndTab->IsFlatTab() || pWndTab->IsOneNoteStyle() || pWndTab->IsVS2005Style())
	{
		CMFCVisualManagerOfficeXP::OnEraseTabsButton(pDC, rect, pButton, pWndTab);
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
		rectTabs.bottom += 2;
	}
	else
	{
		rectTabs.bottom = rectTabArea.bottom;
	}

	pWndTab->MapWindowPoints(pButton, rectTabs);

	OnEraseTabsArea(pDC, rectTabs, pWndTab);

	pDC->SelectClipRgn(NULL);
}

void CMFCVisualManagerWindows::OnDrawTabsButtonBorder(CDC* pDC, CRect& rect, CMFCButton* pButton, UINT uiState, CMFCBaseTabCtrl* pWndTab)
{
	if (m_hThemeToolBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawTabsButtonBorder(pDC, rect, pButton, uiState, pWndTab);
		return;
	}

	int nState = TS_NORMAL;

	if (!pButton->IsWindowEnabled())
	{
		nState = TS_DISABLED;
	}
	else if (pButton->IsPressed() || pButton->GetCheck())
	{
		nState = TS_PRESSED;
	}
	else if (pButton->IsHighlighted())
	{
		nState = TS_HOT;
	}

	afxGlobalData.DrawParentBackground(pButton, pDC, rect);

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);
	}
}

COLORREF CMFCVisualManagerWindows::OnFillMiniFrameCaption( CDC* pDC, CRect rectCaption, CPaneFrameWnd* pFrameWnd, BOOL bActive)
{
	if (m_hThemeWindow == NULL)
	{
		return CMFCVisualManager::OnFillMiniFrameCaption(pDC, rectCaption, pFrameWnd, bActive);
	}

	return afxGlobalData.clrCaptionText;
}

void CMFCVisualManagerWindows::OnDrawMiniFrameBorder( CDC* pDC, CPaneFrameWnd* pFrameWnd, CRect rectBorder, CRect rectBorderSize)
{
	if (m_hThemeWindow == NULL)
	{
		CMFCVisualManager::OnDrawMiniFrameBorder(pDC, pFrameWnd, rectBorder, rectBorderSize);
		return;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeWindow, pDC->GetSafeHdc(), WP_SMALLCAPTION, 0, &rectBorder, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawFloatingToolbarBorder( CDC* pDC, CMFCBaseToolBar* pToolBar, CRect rectBorder, CRect rectBorderSize)
{
	if (m_hThemeWindow == NULL)
	{
		CMFCVisualManager::OnDrawFloatingToolbarBorder(pDC, pToolBar, rectBorder, rectBorderSize);
		return;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeWindow, pDC->GetSafeHdc(), WP_SMALLCAPTION, 0, &rectBorder, 0);
	}
}

void CMFCVisualManagerWindows::OnFillOutlookPageButton(CDC* pDC, const CRect& rectClient, BOOL bIsHighlighted, BOOL bIsPressed, COLORREF& clrText)
{
	if (m_hThemeButton == NULL)
	{
		CMFCVisualManagerOfficeXP::OnFillOutlookPageButton(pDC, rectClient, bIsHighlighted, bIsPressed, clrText);
		return;
	}

	int nState = PBS_NORMAL;
	if (bIsHighlighted)
	{
		nState = PBS_PRESSED;
	}
	else if (bIsPressed)
	{
		nState = PBS_HOT;
	}

	CRect rect = rectClient;
	rect.InflateRect(1, 1);

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, nState, &rect, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawOutlookPageButtonBorder(CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (m_hThemeButton == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawOutlookPageButtonBorder(pDC, rectBtn, bIsHighlighted, bIsPressed);
	}
}

void CMFCVisualManagerWindows::OnDrawStatusBarProgress(CDC* pDC, CMFCStatusBar* pStatusBar, CRect rectProgress, int nProgressTotal, int nProgressCurr,
	COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText, BOOL bProgressText)
{
	if (!DrawStatusBarProgress(pDC, pStatusBar, rectProgress, nProgressTotal, nProgressCurr, clrBar, clrProgressBarDest, clrProgressText, bProgressText))
	{
		CMFCVisualManagerOfficeXP::OnDrawStatusBarProgress(pDC, pStatusBar, rectProgress, nProgressTotal, nProgressCurr, clrBar, clrProgressBarDest, clrProgressText, bProgressText);
	}
}

CSize CMFCVisualManagerWindows::GetButtonExtraBorder() const
{
	if (m_hThemeWindow == NULL)
	{
		return CMFCVisualManagerOfficeXP::GetButtonExtraBorder();
	}

	return CSize(2, 2);
}

CSize CMFCVisualManagerWindows::GetCaptionButtonExtraBorder() const
{
	if (m_hThemeWindow == NULL)
	{
		return CMFCVisualManagerOfficeXP::GetCaptionButtonExtraBorder();
	}

	return CSize(1, 1);
}

void CMFCVisualManagerWindows::OnDrawHeaderCtrlBorder(CMFCHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted)
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

void CMFCVisualManagerWindows::OnDrawHeaderCtrlSortArrow(CMFCHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsUp)
{
	if (m_hThemeHeader == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawHeaderCtrlSortArrow(pCtrl, pDC, rect, bIsUp);
		return;
	}

#define AFX_POINTS_NUM 3
	POINT pts [AFX_POINTS_NUM];

	if (bIsUp)
	{
		pts [0].x = rect.left;
		pts [0].y = rect.bottom;

		pts [1].x = rect.CenterPoint().x;
		pts [1].y = rect.top;

		pts [2].x = rect.right;
		pts [2].y = rect.bottom;
	}
	else
	{
		pts [0].x = rect.left;
		pts [0].y = rect.top;

		pts [1].x = rect.CenterPoint().x;
		pts [1].y = rect.bottom;

		pts [2].x = rect.right;
		pts [2].y = rect.top;
	}

	CBrush br(afxGlobalData.clrBtnShadow);
	CBrush* pOldBrush = pDC->SelectObject(&br);

	CPen* pOldPen = (CPen*) pDC->SelectStockObject(NULL_PEN);

	pDC->Polygon(pts, AFX_POINTS_NUM);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}


void CMFCVisualManagerWindows::OnDrawEditBorder(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsHighlighted, CMFCToolBarEditBoxButton* pButton)
{
	if (m_hThemeWindow == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawEditBorder(pDC, rect, bDisabled, bIsHighlighted, pButton);
		return;
	}

	if (bIsHighlighted)
	{
		pDC->Draw3dRect(&rect,  afxGlobalData.clrBtnHilite, afxGlobalData.clrBtnHilite);
	}
}

void CMFCVisualManagerWindows::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID(pDC);

	if (m_hThemeExplorerBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnFillTasksPaneBackground(pDC, rectWorkArea);
		return;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_HEADERBACKGROUND, 0, &rectWorkArea, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawTasksGroupCaption(CDC* pDC, CMFCTasksPaneTaskGroup* pGroup, BOOL bIsHighlighted, BOOL bIsSelected, BOOL bCanCollapse)
{
	ENSURE(pGroup != NULL);
	ENSURE(pGroup->m_pPage != NULL);

	ASSERT_VALID(pDC);
	ASSERT_VALID(pGroup);
	ASSERT_VALID(pGroup->m_pPage);

	if (m_hThemeExplorerBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawTasksGroupCaption(pDC, pGroup, bIsHighlighted, bIsSelected, bCanCollapse);
		return;
	}

	if (pGroup->m_strName.IsEmpty())
	{
		return;
	}

	// -------------------------------
	// Draw group caption(Windows XP)
	// -------------------------------
	if (m_pfDrawThemeBackground != NULL)
	{
		if (pGroup->m_bIsSpecial)
		{
			(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPHEAD, 0, &pGroup->m_rect, 0);
		}
		else
		{
			(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD, 0, &pGroup->m_rect, 0);
		}
	}

	// ---------------------------
	// Draw an icon if it presents
	// ---------------------------
	BOOL bShowIcon = (pGroup->m_hIcon != NULL && pGroup->m_sizeIcon.cx < pGroup->m_rect.Width() - pGroup->m_rect.Height());
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
		if (pGroup->m_bIsSpecial)
		{
			pDC->SetTextColor(pGroup->m_clrTextHot == (COLORREF)-1 ? afxGlobalData.clrWindow : pGroup->m_clrTextHot);
		}
		else
		{
			pDC->SetTextColor(pGroup->m_clrTextHot == (COLORREF)-1 ? afxGlobalData.clrHilite : pGroup->m_clrTextHot);
		}
	}
	else
	{
		if (pGroup->m_bIsSpecial)
		{
			pDC->SetTextColor(pGroup->m_clrText == (COLORREF)-1 ? afxGlobalData.clrWindow : pGroup->m_clrText);
		}
		else
		{
			pDC->SetTextColor(pGroup->m_clrText == (COLORREF)-1 ? afxGlobalData.clrHilite : pGroup->m_clrText);
		}
	}
	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();
	int nCaptionHOffset = (nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset);

	CRect rectText = pGroup->m_rect;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx + 5: nCaptionHOffset);
	rectText.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectText.right = max(rectText.left, rectText.right -(bCanCollapse ? pGroup->m_rect.Height() : nCaptionHOffset));

	pDC->DrawText(pGroup->m_strName, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject(pFontOld);
	pDC->SetTextColor(clrTextOld);

	// -------------------------
	// Draw group caption button
	// -------------------------
	if (bCanCollapse)
	{
		CRect rectButton = pGroup->m_rect;
		rectButton.left = max(rectButton.left, rectButton.right - rectButton.Height());

		if (m_pfDrawThemeBackground != NULL)
		{
			if (pGroup->m_bIsSpecial)
			{
				if (!pGroup->m_bIsCollapsed)
				{
					if (bIsHighlighted)
					{
						(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPCOLLAPSE, EBSGC_HOT, &rectButton, 0);
					}
					else
					{
						(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPCOLLAPSE, EBSGC_NORMAL, &rectButton, 0);
					}
				}
				else
				{
					if (bIsHighlighted)
					{
						(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPEXPAND, EBSGE_HOT, &rectButton, 0);
					}
					else
					{
						(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPEXPAND, EBSGE_NORMAL, &rectButton, 0);
					}
				}
			}
			else
			{
				if (!pGroup->m_bIsCollapsed)
				{
					if (bIsHighlighted)
					{
						(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPCOLLAPSE, EBNGC_HOT, &rectButton, 0);
					}
					else
					{
						(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPCOLLAPSE, EBNGC_NORMAL, &rectButton, 0);
					}
				}
				else
				{
					if (bIsHighlighted)
					{
						(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPEXPAND, EBNGE_HOT, &rectButton, 0);
					}
					else
					{
						(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPEXPAND, EBNGE_NORMAL, &rectButton, 0);
					}
				}
			}
		}
	}
}

void CMFCVisualManagerWindows::OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial)
{
	ASSERT_VALID(pDC);

	if (m_hThemeExplorerBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnFillTasksGroupInterior(pDC, rect);
		return;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		if (!bSpecial)
		{
			(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPBACKGROUND, 0, &rect, 0);
		}
		else
		{
			(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPBACKGROUND, 0, &rect, 0);
		}
	}
}

void CMFCVisualManagerWindows::OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial, BOOL bNoTitle)
{
	if (m_hThemeExplorerBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawTasksGroupAreaBorder(pDC, rect, bSpecial, bNoTitle);
		return;
	}

	ASSERT_VALID(pDC);

	// Draw underline
	if (bNoTitle)
	{
		CRect rectDraw = rect;
		rectDraw.bottom = rectDraw.top + 1;

		if (m_pfDrawThemeBackground != NULL)
		{
			if (bSpecial)
			{
				(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPHEAD, 0, &rectDraw, 0);
			}
			else
			{
				(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD, 0, &rectDraw, 0);
			}
		}
	}

	return;
}

void CMFCVisualManagerWindows::OnDrawTask(CDC* pDC, CMFCTasksPaneTask* pTask, CImageList* pIcons, BOOL bIsHighlighted, BOOL bIsSelected)
{
	ENSURE(pTask != NULL);
	ENSURE(pIcons != NULL);

	ASSERT_VALID(pDC);
	ASSERT_VALID(pIcons);
	ASSERT_VALID(pTask);

	if (m_hThemeExplorerBar == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawTask(pDC, pTask, pIcons, bIsHighlighted, bIsSelected);
		return;
	}

	if (pTask->m_bIsSeparator)
	{
		// --------------
		// Draw separator
		// --------------
		CRect rectDraw = pTask->m_rect;
		rectDraw.top = pTask->m_rect.CenterPoint().y;
		rectDraw.bottom = rectDraw.top + 1;

		// draw same as group caption
		if (m_pfDrawThemeBackground != NULL)
		{
			(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD, 0, &rectDraw, 0);
		}
		return;
	}

	// ---------
	// Draw icon
	// ---------
	CRect rectText = pTask->m_rect;
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
		pFontOld = pDC->SelectObject( pTask->m_bIsBold ? &afxGlobalData.fontBold : &afxGlobalData.fontRegular);
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
		pDC->SetTextColor(pTask->m_clrTextHot == (COLORREF)-1 ? afxGlobalData.clrHilite : pTask->m_clrTextHot);
	}
	else
	{
		pFontOld = pDC->SelectObject(&afxGlobalData.fontRegular);
		pDC->SetTextColor(pTask->m_clrText == (COLORREF)-1 ? afxGlobalData.clrHilite : pTask->m_clrText);
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

void CMFCVisualManagerWindows::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize, int iImage, BOOL bHilited)
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

void CMFCVisualManagerWindows::OnDrawExpandingBox(CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
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

void CMFCVisualManagerWindows::OnDrawCheckBoxEx(CDC *pDC, CRect rect, int nState, BOOL bHighlighted, BOOL bPressed, BOOL bEnabled)
{
	if (!DrawCheckBox(pDC, rect, bHighlighted, nState, bEnabled, bPressed))
	{
		CMFCVisualManagerOfficeXP::OnDrawCheckBoxEx(pDC, rect, nState, bHighlighted, bPressed, bEnabled);
	}
}

void CMFCVisualManagerWindows::OnDrawControlBorder(CWnd* pWndCtrl)
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


BOOL CMFCVisualManagerWindows::OnDrawBrowseButton(CDC* pDC, CRect rect, CMFCEditBrowseCtrl* pEdit, CMFCVisualManager::AFX_BUTTON_STATE state, COLORREF& clrText)
{
	if (m_hThemeButton == NULL)
	{
		return CMFCVisualManagerOfficeXP::OnDrawBrowseButton(pDC, rect, pEdit, state, clrText);
	}

	ASSERT_VALID(pDC);
	pDC->FillRect(rect, &afxGlobalData.brWindow);

	int nState = PBS_NORMAL;

	switch(state)
	{
	case ButtonsIsPressed:
		nState = PBS_PRESSED;
		break;

	case ButtonsIsHighlighted:
		nState = PBS_HOT;
		break;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, nState, &rect, 0);
	}

	return TRUE;
}

void CMFCVisualManagerWindows::OnDrawSpinButtons(CDC* pDC, CRect rect, int nState, BOOL bOrientation, CMFCSpinButtonCtrl* pSpinCtrl)
{
	if (m_hThemeSpin == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawSpinButtons(pDC, rect, nState, bOrientation, pSpinCtrl);
		return;
	}

	// Draw up part:
	CRect rectUp = rect;
	rectUp.bottom = rect.CenterPoint().y - 1;

	int nDrawState = UPS_NORMAL;

	if (nState & AFX_SPIN_DISABLED)
	{
		nDrawState = UPS_DISABLED;
	}
	else if (nState & AFX_SPIN_PRESSEDUP)
	{
		nDrawState = UPS_PRESSED;
	}
	else if (nState & AFX_SPIN_HIGHLIGHTEDUP)
	{
		nDrawState = UPS_HOT;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeSpin, pDC->GetSafeHdc(), bOrientation ? SPNP_UPHORZ : SPNP_UP, nDrawState, &rectUp, 0);
	}

	// Draw up part:
	CRect rectDown = rect;
	rectDown.top = rect.CenterPoint().y;

	nDrawState = UPS_NORMAL;

	if (nState & AFX_SPIN_DISABLED)
	{
		nDrawState = UPS_DISABLED;
	}
	else if (nState & AFX_SPIN_PRESSEDDOWN)
	{
		nDrawState = UPS_PRESSED;
	}
	else if (nState & AFX_SPIN_HIGHLIGHTEDDOWN)
	{
		nDrawState = UPS_HOT;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeSpin, pDC->GetSafeHdc(), bOrientation ? SPNP_DOWNHORZ : SPNP_DOWN, nDrawState, &rectDown, 0);
	}
}

void CMFCVisualManagerWindows::OnDrawTab(CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);
	ASSERT_VALID(pDC);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab() || pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded())
	{
		CMFCVisualManagerOfficeXP::OnDrawTab(pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	int nState = TIS_NORMAL;
	if (bIsActive)
	{
		nState = TIS_SELECTED;
	}
	else if (iTab == pTabWnd->GetHighlightedTab())
	{
		nState = TIS_HOT;
	}

	rectTab.right++;

	if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_TOP && !bIsActive)
	{
		rectTab.bottom--;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeTab, pDC->GetSafeHdc(), TABP_TABITEM, nState, &rectTab, 0);
	}

	if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
	{
		CDrawingManager dm(*pDC);
		dm.MirrorRect(rectTab, FALSE);
	}

	COLORREF clrTabText = afxGlobalData.clrWindowText;
	if (m_pfGetThemeColor != NULL)
	{
		(*m_pfGetThemeColor)(m_hThemeTab, TABP_TABITEM, nState, TMT_TEXTCOLOR, &clrTabText);
	}

	COLORREF cltTextOld = pDC->SetTextColor(clrTabText);

	OnDrawTabContent(pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	pDC->SetTextColor(cltTextOld);
}

void CMFCVisualManagerWindows::OnDrawTabCloseButton(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	if (m_hThemeWindow == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawTabCloseButton(pDC, rect, pTabWnd, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID(pDC);

	int nState = TS_NORMAL;
	if (bIsDisabled)
	{
		nState = TS_DISABLED;
	}
	else if (bIsPressed && bIsHighlighted)
	{
		nState = TS_PRESSED;
	}
	else if (bIsHighlighted)
	{
		nState = TS_HOT;
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeWindow, pDC->GetSafeHdc(), WP_SMALLCLOSEBUTTON, nState, &rect, 0);
	}
}

void CMFCVisualManagerWindows::OnEraseTabsArea(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTabWnd);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab() || pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style() || pTabWnd->IsDialogControl())
	{
		CMFCVisualManagerOfficeXP::OnEraseTabsArea(pDC, rect, pTabWnd);
		return;
	}

	rect.right += 10;

	if (pTabWnd->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
	{
		rect.top -= 3;
		CMemDC memDC(*pDC, (CWnd*) pTabWnd);

		if (m_pfDrawThemeBackground != NULL)
		{
			(*m_pfDrawThemeBackground)(m_hThemeTab, memDC.GetDC().GetSafeHdc(), TABP_PANE, 0, &rect, NULL);
		}
		CDrawingManager dm(memDC.GetDC());
		dm.MirrorRect(rect, FALSE);
	}
	else
	{
		rect.bottom += 2;
		if (m_pfDrawThemeBackground != NULL)
		{
			(*m_pfDrawThemeBackground)(m_hThemeTab, pDC->GetSafeHdc(), TABP_PANE, 0, &rect, 0);
		}
	}
}

BOOL CMFCVisualManagerWindows::OnEraseTabsFrame(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTabWnd);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab() || pTabWnd->IsOneNoteStyle() || pTabWnd->IsVS2005Style())
	{
		return CMFCVisualManagerOfficeXP::OnEraseTabsFrame(pDC, rect, pTabWnd);
	}

	return FALSE;
}

void CMFCVisualManagerWindows::OnErasePopupWindowButton(CDC* pDC, CRect rect, CMFCDesktopAlertWndButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	if (m_hThemeButton == NULL || pButton->IsCaptionButton())
	{
		CMFCVisualManagerOfficeXP::OnErasePopupWindowButton(pDC, rect, pButton);
		return;
	}

	CRect rectParent;
	pButton->GetParent()->GetClientRect(rectParent);

	pButton->GetParent()->MapWindowPoints(pButton, rectParent);
	OnFillPopupWindowBackground(pDC, rectParent);
}

void CMFCVisualManagerWindows::OnDrawPopupWindowButtonBorder(CDC* pDC, CRect rect, CMFCDesktopAlertWndButton* pButton)
{
	ASSERT_VALID(pButton);

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
	else
	{
		nState = PBS_NORMAL;
	}

	if (m_hThemeWindow != NULL && m_pfDrawThemeBackground != NULL && pButton->IsCloseButton() && pButton->IsCaptionButton())
	{
		(*m_pfDrawThemeBackground)(m_hThemeWindow, pDC->GetSafeHdc(), WP_SMALLCLOSEBUTTON, nState, &rect, 0);
		return;
	}

	if (m_hThemeButton == NULL)
	{
		CMFCVisualManagerOfficeXP::OnDrawPopupWindowButtonBorder(pDC, rect, pButton);
		return;
	}

	afxGlobalData.DrawParentBackground(pButton, pDC, rect);
	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, nState, &rect, 0);
	}
}

BOOL CMFCVisualManagerWindows::IsDefaultWinXPPopupButton(CMFCDesktopAlertWndButton* pButton) const
{
	ASSERT_VALID(pButton);
	return m_hThemeWindow != NULL && pButton->IsCloseButton() && pButton->IsCaptionButton();
}

COLORREF CMFCVisualManagerWindows::OnDrawPaneCaption(CDC* pDC, CDockablePane* pBar, BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	if (m_hThemeExplorerBar == NULL || m_pfGetThemeColor == NULL)
	{
		return CMFCVisualManagerOfficeXP::OnDrawPaneCaption(pDC, pBar, bActive, rectCaption, rectButtons);
	}

	if (m_pfDrawThemeBackground != NULL)
	{
		(*m_pfDrawThemeBackground)(m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_HEADERBACKGROUND, 0, &rectCaption, 0);
	}

	COLORREF clrText;
	if ((m_pfGetThemeColor == NULL) || ((*m_pfGetThemeColor)(m_hThemeExplorerBar, EBP_HEADERBACKGROUND, 0, TMT_TEXTCOLOR, &clrText) != S_OK))
	{
		clrText = afxGlobalData.clrInactiveCaptionText;
	}

	if (bActive)
	{
		CDrawingManager dm(*pDC);
		int nPercentage = 110;

		if (GetRValue(clrText) > 128 &&
			GetGValue(clrText) > 128 &&
			GetBValue(clrText) > 128)
		{
			nPercentage = 80;
		}

		dm.HighlightRect(rectCaption, nPercentage);
	}

	return clrText;
}



