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
#include "afxribbonres.h"
#include "afxcontrolbarutil.h"
#include "afxcustomizemenubutton.h"
#include "afxcustomizebutton.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxkeyboardmanager.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCCustomizeMenuButton, CMFCToolBarMenuButton)

CMap<UINT, UINT, int, int>	 CMFCCustomizeMenuButton::m_mapPresentIDs;
CMFCToolBar* CMFCCustomizeMenuButton::m_pWndToolBar = NULL;

BOOL CMFCCustomizeMenuButton::m_bRecentlyUsedOld = FALSE;

// Construction/Destruction
CMFCCustomizeMenuButton::CMFCCustomizeMenuButton()
{
}

CMFCCustomizeMenuButton::~CMFCCustomizeMenuButton()
{
}

CMFCCustomizeMenuButton::CMFCCustomizeMenuButton(UINT uiID,HMENU hMenu,int iImage,LPCTSTR lpszText,BOOL bUserButton):
	CMFCToolBarMenuButton(uiID, hMenu/* HMENU */, iImage /*iImage*/, lpszText, bUserButton)
{
	m_uiIndex = (UINT)-1;
	bSeparator = FALSE;
	m_bAddSpr = FALSE;
	m_bIsEnabled = TRUE;
	m_bBrothersBtn = FALSE;
}

void CMFCCustomizeMenuButton::SetItemIndex(UINT uiIndex, BOOL bExist, BOOL bAddSpr)
{
	m_uiIndex = uiIndex;
	m_bExist = bExist;
	m_bAddSpr = bAddSpr;

	if ((uiIndex != ID_AFXBARRES_TOOLBAR_RESET_PROMT) && !bSeparator && bExist)
	{
		CMFCToolBarButton* pBtn = m_pWndToolBar->GetButton(uiIndex);
		m_bShow = pBtn->IsVisible();

	}
	else
	{
		m_bShow = FALSE;

		if (m_uiIndex == ID_AFXBARRES_TOOLBAR_RESET_PROMT && m_pWndToolBar->IsUserDefined())
		{
			m_bIsEnabled = FALSE;
		}
	}
}

void CMFCCustomizeMenuButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarButton::CopyFrom(s);
	const CMFCCustomizeMenuButton& src = (const CMFCCustomizeMenuButton&) s;

	m_uiIndex      =   src.m_uiIndex;
	m_bShow        =   src.m_bShow;
	m_pWndToolBar  =   src.m_pWndToolBar;
	bSeparator     =   src.bSeparator;
	m_bExist       =   src.m_bExist;
	m_bAddSpr      =   src.m_bAddSpr;
	m_bIsEnabled   =   src.m_bIsEnabled;
	m_bBrothersBtn =   src.m_bBrothersBtn;
}

SIZE CMFCCustomizeMenuButton::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (bSeparator)
	{
		return CSize(0,  4);
	}

	if (m_bBrothersBtn)
	{
		return CMFCToolBarMenuButton::OnCalculateSize(pDC, sizeDefault, bHorz);
	}

	//  Try to Find Buttons Text
	if (m_strText.IsEmpty())
	{
		// Try to find the command name in resources:
		CString strMessage;
		int iOffset;
		if (strMessage.LoadString(m_nID) && (iOffset = strMessage.Find(_T('\n'))) != -1)
		{
			m_strText = strMessage.Mid(iOffset + 1);
		}
	}
	else
	{
		// m_strText.Remove(_T('&'));

		// Remove trailing label(ex.:"\tCtrl+S"):
		int iOffset = m_strText.Find(_T('\t'));
		if (iOffset != -1)
		{
			m_strText = m_strText.Left(iOffset);
		}
	}

	// Change accelerator:
	if (afxKeyboardManager != NULL && m_bMenuMode && (m_nID < 0xF000 || m_nID >= 0xF1F0)) // Not system.
	{
		// Remove standard aceleration label:
		int iTabOffset = m_strText.Find(_T('\t'));
		if (iTabOffset >= 0)
		{
			m_strText = m_strText.Left(iTabOffset);
		}

		// Add an actual accelartion label:
		CString strAccel;
		CFrameWnd* pParent = m_pWndParent == NULL ? DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd()) : AFXGetTopLevelFrame(m_pWndParent);

		if (pParent != NULL &&
			(CKeyboardManager::FindDefaultAccelerator(m_nID, strAccel, pParent, TRUE) || CKeyboardManager::FindDefaultAccelerator(m_nID, strAccel, pParent->GetActiveFrame(), FALSE)))
		{
			m_strText += _T('\t');
			m_strText += strAccel;
		}
	}

	int nTolalWidth = m_strText.GetLength();
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	nTolalWidth *= tm.tmAveCharWidth;
	CSize sizeImage = CMFCToolBar::GetMenuButtonSize();
	nTolalWidth += 2*sizeImage.cx;
	nTolalWidth += 3*CMFCVisualManager::GetInstance()->GetMenuImageMargin() + 50;

	CSize sizeStandard = CMFCToolBarMenuButton::OnCalculateSize(pDC, sizeDefault, bHorz);

	int nTotalHeight = sizeStandard.cy + 2;

	if (!m_bMenuMode)
	{
		nTotalHeight += CMFCVisualManager::GetInstance()->GetMenuImageMargin();
	}

	return CSize(nTolalWidth,  nTotalHeight);
}

BOOL CMFCCustomizeMenuButton::OnClickMenuItem()
{
	if (bSeparator || !m_bIsEnabled)
	{
		return TRUE;
	}

	CMFCPopupMenuBar* pMenuBar = (CMFCPopupMenuBar*)m_pWndParent;
	ENSURE(pMenuBar != NULL);
	ASSERT_VALID(pMenuBar);

	int nIndex = pMenuBar->ButtonToIndex(this);
	if (nIndex !=-1)
	{
		if (pMenuBar->m_iHighlighted != nIndex)
		{
			pMenuBar->m_iHighlighted = nIndex;
			pMenuBar->InvalidateRect(this->Rect());
		}
	}

	if (m_bBrothersBtn)
	{
		if (m_pWndToolBar->IsOneRowWithSibling())
		{
			m_pWndToolBar->SetTwoRowsWithSibling();
		}
		else
		{
			m_pWndToolBar->SetOneRowWithSibling();
		}

		return FALSE;
	}

	if (m_uiIndex == ID_AFXBARRES_TOOLBAR_RESET_PROMT) // reset pressed
	{
		//load default toolbar
		m_pWndToolBar->PostMessage(AFX_WM_RESETRPROMPT);
		return FALSE;
	}

	if (!m_bExist)
	{
		const CObList& lstOrignButtons = m_pWndToolBar->GetOrigResetButtons();

		POSITION pos = lstOrignButtons.FindIndex(m_uiIndex);
		CMFCToolBarButton* pButton = (CMFCToolBarButton*)lstOrignButtons.GetAt(pos);
		if (pButton == NULL)
		{
			return TRUE;
		}

		UINT nNewIndex = m_pWndToolBar->InsertButton(*pButton, m_uiIndex);

		if (nNewIndex == -1)
		{
			nNewIndex = m_pWndToolBar->InsertButton(*pButton);
		}
		else
		{
			int nCount = pMenuBar->GetCount();
			for (int i = 0; i < nCount; i++)
			{
				CMFCCustomizeMenuButton* pBtn = (CMFCCustomizeMenuButton*)pMenuBar->GetButton(i);
				if ((pBtn->m_uiIndex >= nNewIndex) && (pBtn->m_uiIndex != ID_AFXBARRES_TOOLBAR_RESET_PROMT))
				{
					if (pBtn->m_bExist)
					{
						pBtn->m_uiIndex += 1;
					}
				}
			}
		}

		m_uiIndex = nNewIndex;

		if (m_bAddSpr)
		{
			if (nNewIndex <(UINT)m_pWndToolBar->GetCount())
			{
				CMFCToolBarButton* pBtn = m_pWndToolBar->GetButton(nNewIndex+1);
				if (!(pBtn->m_nStyle & TBBS_SEPARATOR))
				{
					m_pWndToolBar->InsertSeparator();
				}
			}
			else
			{
				m_pWndToolBar->InsertSeparator();
			}
		}

		m_pWndToolBar->AdjustLayout();
		m_pWndToolBar->AdjustSizeImmediate();
		UpdateCustomizeButton();

		m_bExist = TRUE;
		m_bShow = TRUE;
		pMenuBar->Invalidate();

		return TRUE;
	}

	CMFCToolBarButton* pBtn = m_pWndToolBar->GetButton(m_uiIndex);
	BOOL bVisible = pBtn->IsVisible();
	pBtn->SetVisible(!bVisible);
	m_bShow = !bVisible;

	//  Make next Separator the same state
	int nNext = m_uiIndex + 1;
	if (nNext < m_pWndToolBar->GetCount())
	{
		CMFCToolBarButton* pBtnNext = m_pWndToolBar->GetButton(nNext);
		if (pBtnNext->m_nStyle & TBBS_SEPARATOR)
		{
			pBtnNext->SetVisible(!bVisible);
		}
	}

	CMFCPopupMenu* pCustomizeMenu = NULL;

	for (CMFCPopupMenu* pMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pMenuBar->GetParent()); pMenu != NULL; pMenu = pMenu->GetParentPopupMenu())
	{
		pCustomizeMenu = pMenu;
	}

	if (pCustomizeMenu != NULL)
	{
		pCustomizeMenu->ShowWindow(SW_HIDE);
	}

	m_pWndToolBar->AdjustLayout();
	m_pWndToolBar->AdjustSizeImmediate();
	UpdateCustomizeButton();
	pMenuBar->Invalidate();

	if (pCustomizeMenu != NULL)
	{
		pCustomizeMenu->ShowWindow(SW_SHOWNOACTIVATE);

		CRect rectScreen;
		pCustomizeMenu->GetWindowRect(&rectScreen);
		CMFCPopupMenu::UpdateAllShadows(rectScreen);
	}

	return TRUE;
}

void CMFCCustomizeMenuButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	// Draw separator:
	if (bSeparator)
	{
		CRect rcSeparator(rect);
		rcSeparator.left = 2*CMFCToolBar::GetMenuImageSize().cx + CMFCVisualManager::GetInstance()->GetMenuImageMargin();

		CMFCPopupMenuBar* pMenuBar = (CMFCPopupMenuBar*)m_pWndParent;
		ENSURE(pMenuBar != NULL);
		ASSERT_VALID(pMenuBar);

		CMFCVisualManager::GetInstance()->OnDrawSeparator(pDC, pMenuBar, rcSeparator, FALSE);
		return;
	}

	if (m_bBrothersBtn)
	{
		CMFCToolBarMenuButton::OnDraw(pDC, rect, NULL, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);
		return;
	}

	CRect rectItem = rect;
	rectItem.bottom--;

	if (m_bIsEnabled)
	{
		if (m_bShow && bHighlight)
		{
			SetStyle(TBBS_BUTTON|TBBS_CHECKED);
		}
		else
		{
			SetStyle(TBBS_BUTTON);
		}
	}
	else
	{
		SetStyle(TBBS_DISABLED);
		bGrayDisabledButtons = TRUE;
		bHighlight = FALSE;
	}

	BOOL bIsResetItem = m_uiIndex == ID_AFXBARRES_TOOLBAR_RESET_PROMT;

	if (bIsResetItem)
	{
		m_bImage = FALSE;
		m_iImage = -1;
	}

	// Highlight item:
	if (bHighlight && m_bIsEnabled)
	{
		CRect rcHighlight = rectItem;
		rcHighlight.left += 2;
		rcHighlight.right--;

		if (!CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem() && !bIsResetItem)
		{
			rcHighlight.left += 2 * CMFCToolBar::GetMenuImageSize().cx + 5 * CMFCVisualManager::GetInstance()->GetMenuImageMargin();
		}

		COLORREF clrText;
		CMFCVisualManager::GetInstance()->OnHighlightMenuItem(pDC, this, rcHighlight, clrText);
	}

	// Draw checkbox:
	CSize sizeMenuImage = CMFCToolBar::GetMenuImageSize();

	CRect rectCheck = rectItem;
	rectCheck.left += CMFCVisualManager::GetInstance()->GetMenuImageMargin() + 1;
	rectCheck.right = rectCheck.left + sizeMenuImage.cx + CMFCVisualManager::GetInstance()->GetMenuImageMargin() + 2;
	rectCheck.bottom--;

	DrawCheckBox(pDC, rectCheck, bHighlight);

	if (bHighlight && !(m_nStyle & TBBS_DISABLED) && !bIsResetItem)
	{
		SetStyle(TBBS_BUTTON);
	}

	// Draw icon + text:
	CRect rectStdMenu = rectItem;
	rectStdMenu.left = rectCheck.right;

	DrawMenuItem(pDC, rectStdMenu, pImages, bCustomizeMode, bHighlight, bGrayDisabledButtons, TRUE);
}

CString CMFCCustomizeMenuButton::SearchCommandText(CMenu* pMenu, UINT in_uiCmd)
{
	ENSURE(pMenu != NULL);

	int iCount = (int) pMenu->GetMenuItemCount();

	for (int i = 0; i < iCount; i ++)
	{
		UINT uiCmd = pMenu->GetMenuItemID(i);
		if (uiCmd == in_uiCmd)
		{
			CString strText;
			pMenu->GetMenuString(i, strText, MF_BYPOSITION);
			return strText;
		}

		switch (uiCmd)
		{
		case 0: // Separator, ignore it.
			break;

		case -1: // Submenu
			{
				CMenu* pSubMenu = pMenu->GetSubMenu(i);

				CString strText = SearchCommandText(pSubMenu, in_uiCmd);
				if (strText != _T("")) return strText;
			}
			break;

		}//end switch
	}//end for

	return _T("");
}

void CMFCCustomizeMenuButton::DrawCheckBox(CDC* pDC, const CRect& rect, BOOL bHighlight)
{
	if (!m_bShow)
	{
		return;
	}

	CRect rectCheck = rect;
	rectCheck.DeflateRect(0, 1, 1, 1);

	if (!CMFCVisualManager::GetInstance()->IsOwnerDrawMenuCheck())
	{
		UINT nStyle = m_nStyle;
		m_nStyle |= TBBS_CHECKED;

		FillInterior(pDC, rectCheck, bHighlight);

		if (bHighlight && CMFCVisualManager::GetInstance()->IsFrameMenuCheckedItems())
		{
			m_nStyle |= TBBS_MARKED;
		}

		CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectCheck, CMFCVisualManager::ButtonsIsPressed);

		m_nStyle = nStyle;
	}

	CMFCVisualManager::GetInstance()->OnDrawMenuCheck(pDC, this, rectCheck, bHighlight, FALSE);
}

void CMFCCustomizeMenuButton::UpdateCustomizeButton()
{
	ASSERT_VALID(m_pWndToolBar);

	if (m_pWndToolBar->GetParent()->GetSafeHwnd() != NULL)
	{
		m_pWndToolBar->GetParent()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
	}

	m_pWndToolBar->RedrawCustomizeButton();
}

BOOL __stdcall CMFCCustomizeMenuButton::IsCommandExist(UINT uiCmdId)
{
	int nTmp = 0;
	return m_mapPresentIDs.Lookup(uiCmdId, nTmp);
}



