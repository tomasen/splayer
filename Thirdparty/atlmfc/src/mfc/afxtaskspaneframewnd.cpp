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

#include "afxcaptionmenubutton.h"
#include "afxtaskspane.h"
#include "afxtaskspaneframewnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMFCTasksPaneFrameWnd,CPaneFrameWnd,VERSIONABLE_SCHEMA | 2)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCTasksPaneFrameWnd::CMFCTasksPaneFrameWnd()
{
	m_bMenuBtnPressed = FALSE;
}

CMFCTasksPaneFrameWnd::~CMFCTasksPaneFrameWnd()
{
}

BEGIN_MESSAGE_MAP(CMFCTasksPaneFrameWnd, CPaneFrameWnd)
	//{{AFX_MSG_MAP(CMFCTasksPaneFrameWnd)
	ON_WM_NCPAINT()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CMFCTasksPaneFrameWnd::OnNeedTipText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMFCTasksPaneFrameWnd::SetCaptionButtons(DWORD dwButtons)
{
	ASSERT_VALID(this);
	RemoveAllCaptionButtons();

	if (dwButtons & AFX_CAPTION_BTN_CLOSE)
	{
		CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, GetPane());
		if (pBar != NULL && pBar->CanBeClosed())
		{
			AddButton(HTCLOSE);
		}
	}

	if (dwButtons & AFX_CAPTION_BTN_PIN)
	{
		AddButton(HTMAXBUTTON);
	}

	if (dwButtons & AFX_CAPTION_BTN_MENU)
	{
		AddButton(HTMINBUTTON);
	}

	AddButton(AFX_HTLEFTBUTTON);
	AddButton(AFX_HTRIGHTBUTTON);
	AddButton(AFX_HTMENU);

	m_dwCaptionButtons = dwButtons | AFX_CAPTION_BTN_LEFT | AFX_CAPTION_BTN_RIGHT | AFX_CAPTION_BTN_TPMENU;
	SetCaptionButtonsToolTips();

	ArrangeCaptionButtons();
	SendMessage(WM_NCPAINT);
}

void CMFCTasksPaneFrameWnd::AddButton(UINT nHit)
{
	ASSERT_VALID(this);

	CMFCCaptionButton* pBtn = FindButton(nHit);

	if (pBtn == NULL)
	{
		switch (nHit)
		{
		case AFX_HTLEFTBUTTON:
			m_lstCaptionButtons.AddHead(new CMFCCaptionButton(AFX_HTLEFTBUTTON, TRUE));
			break;
		case AFX_HTRIGHTBUTTON:
			m_lstCaptionButtons.AddHead(new CMFCCaptionButton(AFX_HTRIGHTBUTTON, TRUE));
			break;

		case AFX_HTMENU:
			{
				CMFCCaptionMenuButton *pMenuBtn = new CMFCCaptionMenuButton;
				pMenuBtn->m_bOSMenu = FALSE;
				pMenuBtn->m_nHit = AFX_HTMENU;
				m_lstCaptionButtons.AddHead(pMenuBtn);
			}
			break;

		default:
			CPaneFrameWnd::AddButton(nHit);
		}
	}
}

void CMFCTasksPaneFrameWnd::SetTaskPaneCaptionButtons()
{
	ASSERT_VALID(this);

	if (TRUE)
	{
		SetCaptionButtons(m_dwCaptionButtons | AFX_CAPTION_BTN_LEFT | AFX_CAPTION_BTN_RIGHT | AFX_CAPTION_BTN_TPMENU);
	}
}

void CMFCTasksPaneFrameWnd::OnNcPaint()
{
	// Enable or disable Taskpane specific caption buttons:
	CMFCTasksPane* pTaskPane = DYNAMIC_DOWNCAST(CMFCTasksPane, GetPane());
	BOOL bMultiPages = (pTaskPane != NULL && pTaskPane->GetPagesCount() > 1);
	BOOL bUseNavigationToolbar = (pTaskPane != NULL && pTaskPane->IsNavigationToolbarEnabled());

	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		switch (pBtn->GetHit())
		{
			case AFX_HTLEFTBUTTON:
			case AFX_HTRIGHTBUTTON:
			case AFX_HTMENU:
				pBtn->m_bHidden = !bMultiPages || bUseNavigationToolbar;
		}

		if (pBtn->GetHit() == AFX_HTLEFTBUTTON)
		{
			pBtn->m_bEnabled = (pTaskPane != NULL && pTaskPane->IsBackButtonEnabled());
		}
		if (pBtn->GetHit() == AFX_HTRIGHTBUTTON)
		{
			pBtn->m_bEnabled = (pTaskPane != NULL && pTaskPane->IsForwardButtonEnabled());
		}
	}

	UpdateTooltips();

	CPaneFrameWnd::OnNcPaint();
}

void CMFCTasksPaneFrameWnd::OnDrawBorder(CDC* pDC)
{
	CPaneFrameWnd::OnDrawBorder(pDC);
}

void CMFCTasksPaneFrameWnd::OnDrawCaptionButtons(CDC* pDC)
{
	ASSERT_VALID(pDC);

	// Paint caption buttons:
	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		BOOL bMaximized = TRUE;
		if (pBtn->GetHit() == HTMAXBUTTON && m_bPinned)
		{
			bMaximized = FALSE;
		}
		pBtn->OnDraw(pDC, FALSE, TRUE, bMaximized);
	}
}

void CMFCTasksPaneFrameWnd::OnPressButtons(UINT nHit)
{
	CMFCTasksPane* pTaskPane = DYNAMIC_DOWNCAST(CMFCTasksPane, GetPane());
	if (pTaskPane != NULL)
	{
		ASSERT_VALID(pTaskPane);

		switch (nHit)
		{
		case AFX_HTLEFTBUTTON:
			// Handle Back caption button
			pTaskPane->OnPressBackButton();
			break;

		case AFX_HTRIGHTBUTTON:
			// Handle Forward caption button
			pTaskPane->OnPressForwardButton();
			break;

		case AFX_HTMENU:
			// Handle Other caption button
			{
				CMFCCaptionMenuButton* pbtn = (CMFCCaptionMenuButton*)FindButton(AFX_HTMENU);
				if (pbtn != NULL)
				{
					m_bMenuBtnPressed = TRUE;
					pTaskPane->OnPressOtherButton(pbtn, this);
					m_bMenuBtnPressed = FALSE;
				}
			}
			break;
		}
	}

	CPaneFrameWnd::OnPressButtons(nHit);
}

void CMFCTasksPaneFrameWnd::CalcBorderSize(CRect& rectBorderSize) const
{
	rectBorderSize.SetRect(m_nToolbarBorderSize, m_nToolbarBorderSize, m_nToolbarBorderSize, m_nToolbarBorderSize);
}

void CMFCTasksPaneFrameWnd::OnTrackCaptionButtons(CPoint point)
{
	if (!m_bMenuBtnPressed)
	{
		CPaneFrameWnd::OnTrackCaptionButtons(point);
	}
}

void CMFCTasksPaneFrameWnd::StopCaptionButtonsTracking()
{
	if (!m_bMenuBtnPressed)
	{
		CPaneFrameWnd::StopCaptionButtonsTracking();
	}
}

BOOL CMFCTasksPaneFrameWnd::OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult)
{
	static CString strTipText;

	ENSURE(pNMH != NULL);

	if (m_pToolTip->GetSafeHwnd() == NULL || pNMH->hwndFrom != m_pToolTip->GetSafeHwnd())
	{
		return FALSE;
	}

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (pNMH->idFrom > 0 &&(int)pNMH->idFrom <= m_lstCaptionButtons.GetCount())
	{
		POSITION pos = m_lstCaptionButtons.FindIndex(pNMH->idFrom - 1);
		if (pos != NULL)
		{
			CMFCCaptionButton* pBtn = (CMFCCaptionButton*)m_lstCaptionButtons.GetAt(pos);
			ASSERT_VALID(pBtn);

			switch (pBtn->GetHit())
			{
			case AFX_HTLEFTBUTTON:
				strTipText = _T("Back");
				pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
				return TRUE;
			case AFX_HTRIGHTBUTTON:
				strTipText = _T("Forward");
				pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
				return TRUE;
			case AFX_HTMENU:
				strTipText = _T("Other Tasks Pane");
				pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
				return TRUE;
			}
		}
	}

	return CPaneFrameWnd::OnNeedTipText(id, pNMH, pResult);
}



