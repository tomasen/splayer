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
#include "afxautohidebar.h"
#include "afxautohidebutton.h"
#include "afxglobalutils.h"
#include "afxdockingmanager.h"
#include "afxdocksite.h"
#include "afxdockablepane.h"
#include "afxdockingpanesrow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_DISPLAY_AHWND_EVENT	1

int CMFCAutoHideBar::m_nShowAHWndDelay = 400;

CRuntimeClass* CMFCAutoHideBar::m_pAutoHideButtonRTS = RUNTIME_CLASS(CMFCAutoHideButton);

IMPLEMENT_DYNCREATE(CMFCAutoHideBar, CPane)

/////////////////////////////////////////////////////////////////////////////
// CMFCAutoHideBar

CMFCAutoHideBar::CMFCAutoHideBar()
{
	m_pLastActiveButton = NULL;
	m_bReadyToDisplayAHWnd = FALSE;
	m_nDisplayAHWndTimerID = 0;
}

CMFCAutoHideBar::~CMFCAutoHideBar()
{
	if (m_nDisplayAHWndTimerID != 0)
	{
		KillTimer(m_nDisplayAHWndTimerID);
	}
	CleanUpAutoHideButtons();
}

BEGIN_MESSAGE_MAP(CMFCAutoHideBar, CPane)
	//{{AFX_MSG_MAP(CMFCAutoHideBar)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_NCDESTROY()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCAutoHideBar message handlers

int CMFCAutoHideBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

CMFCAutoHideButton* CMFCAutoHideBar::AddAutoHideWindow(CDockablePane* pAutoHideWnd, DWORD dwAlignment)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pAutoHideWnd);
	ASSERT_KINDOF(CDockablePane, pAutoHideWnd);

	CDockablePane* pAutoHideBar = DYNAMIC_DOWNCAST(CDockablePane, pAutoHideWnd);

	if (pAutoHideBar == NULL)
	{
		ASSERT(FALSE);
		TRACE0("Only CDockablePane-derived class may have autohide state!\n");
		return NULL;
	}

	ENSURE(m_pAutoHideButtonRTS != NULL);

	CMFCAutoHideButton* pNewAutoHideButton = DYNAMIC_DOWNCAST(CMFCAutoHideButton, m_pAutoHideButtonRTS->CreateObject());

	if (pNewAutoHideButton == NULL)
	{
		ASSERT(FALSE);
		TRACE0("Wrong runtime class was specified for the autohide button class.\n");
		return NULL;
	}

	if (!pNewAutoHideButton->Create(this, pAutoHideBar, dwAlignment))
	{
		delete pNewAutoHideButton;
		TRACE0("Failed to create new AutoHide button.\n");
		return NULL;
	}

	CRect rect;
	GetWindowRect(rect);
	CSize sizeBtn = pNewAutoHideButton->GetSize();

	if (m_lstAutoHideButtons.IsEmpty())
	{
		rect.right += sizeBtn.cx;
		rect.bottom += sizeBtn.cy;
	}

	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);

	m_lstAutoHideButtons.AddTail(pNewAutoHideButton);
	return pNewAutoHideButton;
}

BOOL CMFCAutoHideBar::RemoveAutoHideWindow(CDockablePane* pAutoHideWnd)
{
	if (m_nDisplayAHWndTimerID != 0)
	{
		KillTimer(m_nDisplayAHWndTimerID);
		m_nDisplayAHWndTimerID = 0;
	}

	POSITION posSave = NULL;
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		posSave = pos;
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		if (pBtn->GetAutoHideWindow() == pAutoHideWnd)
		{
			if (m_pLastActiveButton == pBtn)
			{
				m_pLastActiveButton = NULL;
			}

			m_lstAutoHideButtons.RemoveAt(posSave);
			delete pBtn;

			if (m_lstAutoHideButtons.IsEmpty())
			{
				ASSERT_VALID(m_pParentDockBar);
				m_pParentDockBar->RemovePane(this, DM_UNKNOWN);
				CRect rectClient;
				m_pParentDockBar->GetClientRect(rectClient);
				m_pParentDockBar->RepositionPanes(rectClient);
			}
			DestroyWindow();
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CMFCAutoHideBar::ShowAutoHideWindow(CDockablePane* pAutoHideWnd, BOOL bShow, BOOL /*bDelay*/)
{
	ASSERT_VALID(this);

	CMFCAutoHideButton* pBtn = ButtonFromAutoHideWindow(pAutoHideWnd);
	if (pBtn == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pBtn);
	pBtn->ShowButton(bShow);

	UpdateVisibleState();
	return TRUE;
}

void CMFCAutoHideBar::UpdateVisibleState()
{
	BOOL bVisible = IsWindowVisible();
	int nCount = GetVisibleCount();

	ENSURE(m_pParentDockBar != NULL);

	if (/*bVisible &&*/ nCount == 0)
	{
		m_pParentDockBar->ShowPane(this, FALSE, FALSE, FALSE);
	}
	else if (!bVisible && nCount > 0)
	{
		m_pParentDockBar->ShowPane(this, TRUE, FALSE, TRUE);
	}
}

void CMFCAutoHideBar::UnSetAutoHideMode(CDockablePane* pFirstBarInGroup)
{
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		pBtn->UnSetAutoHideMode(pFirstBarInGroup);
	}
}

int CMFCAutoHideBar::GetVisibleCount()
{
	int nCount = 0;
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);
		if (pBtn->IsVisible())
		{
			nCount++;
		}
	}
	return nCount;
}

CMFCAutoHideButton* CMFCAutoHideBar::ButtonFromAutoHideWindow(CDockablePane* pAutoHideWnd)
{
	ASSERT_VALID(this);
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		if (pBtn->GetAutoHideWindow() == pAutoHideWnd)
		{
			return pBtn;
		}
	}

	return NULL;
}

void CMFCAutoHideBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(pDC);

	CMemDC memDC(*pDC, this);
	CPane::DoPaint(&memDC.GetDC());

	CMFCAutoHideButton* pBtnTop = NULL;
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);
		if (pBtn->IsTop())
		{
			pBtnTop = pBtn;
		}
		else if (pBtn->IsVisible())
		{
			pBtn->OnDraw(&memDC.GetDC());
		}
	}

	if (pBtnTop != NULL && pBtnTop->IsVisible())
	{
		pBtnTop->OnDraw(&memDC.GetDC());
	}
}

void CMFCAutoHideBar::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	CMFCAutoHideButton* pBtn = ButtonFromPoint(pt);

	if (pBtn != NULL && !m_bReadyToDisplayAHWnd)
	{
		CDockablePane* pAttachedBar = pBtn->GetAutoHideWindow();
		ASSERT_VALID(pAttachedBar);

		if (!pAttachedBar->IsWindowVisible())
		{
			m_bReadyToDisplayAHWnd = TRUE;

			if (m_nDisplayAHWndTimerID != 0)
			{
				KillTimer(m_nDisplayAHWndTimerID);
			}
			m_nDisplayAHWndTimerID = SetTimer(AFX_DISPLAY_AHWND_EVENT, m_nShowAHWndDelay, NULL);
		}
	}
}

CMFCAutoHideButton* CMFCAutoHideBar::ButtonFromPoint(CPoint pt)
{
	POSITION pos = NULL;
	for (pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		if (!pBtn->IsTop())
		{
			continue;
		}

		if (pBtn->IsVisible())
		{
			CRect rect = pBtn->GetRect();
			if (rect.PtInRect(pt))
			{
				return pBtn;
			}
		}
	}

	for (pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		if (pBtn->IsVisible())
		{
			CRect rect = pBtn->GetRect();
			if (rect.PtInRect(pt))
			{
				return pBtn;
			}
		}
	}

	return NULL;
}

CSize CMFCAutoHideBar::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	CRect rect;
	GetWindowRect(&rect);
	return rect.Size();
}

void CMFCAutoHideBar::CleanUpAutoHideButtons()
{
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		delete pBtn;
	}

	m_lstAutoHideButtons.RemoveAll();
}

void CMFCAutoHideBar::OnNcDestroy()
{
	if (m_nDisplayAHWndTimerID != 0)
	{
		KillTimer(m_nDisplayAHWndTimerID);
		m_nDisplayAHWndTimerID = 0;
	}

	CWnd::OnNcDestroy();
	delete this;
}

void CMFCAutoHideBar::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == AFX_DISPLAY_AHWND_EVENT)
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		CMFCAutoHideButton* pBtn = ButtonFromPoint(pt);

		KillTimer(m_nDisplayAHWndTimerID);
		m_nDisplayAHWndTimerID = 0;

		if (pBtn != NULL && m_bReadyToDisplayAHWnd)
		{
			m_bReadyToDisplayAHWnd = FALSE;

			CDockablePane* pAttachedBar = pBtn->GetAutoHideWindow();
			ASSERT_VALID(pAttachedBar);

			if (!pAttachedBar->IsWindowVisible())
			{
				pBtn->ShowAttachedWindow(TRUE);
			}
		}
		else
		{
			m_bReadyToDisplayAHWnd = FALSE;
		}
	}

	CPane::OnTimer(nIDEvent);
}

CSize CMFCAutoHideBar::StretchPane(int /*nLength*/, BOOL /*bVert*/)
{
	CRect rect;
	GetWindowRect(rect);
	CSize size(0, 0);

	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		size = pBtn->GetSize();
	}

	SetWindowPos(NULL, 0, 0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER);

	return size;
}

void CMFCAutoHideBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CMFCAutoHideButton* pBtn = CMFCAutoHideBar::ButtonFromPoint(point);

	if (pBtn != NULL)
	{
		CDockablePane* pAttachedBar = pBtn->GetAutoHideWindow();
		if (pAttachedBar != NULL)
		{
			pAttachedBar->SetFocus();
		}
	}

	CPane::OnLButtonDown(nFlags, point);
}

void CMFCAutoHideBar::SetActiveInGroup(BOOL bActive)
{
	CPane::SetActiveInGroup(bActive);
	if (bActive)
	{
		CObList lst;
		m_pDockBarRow->GetGroupFromPane(this, lst);

		for (POSITION pos = lst.GetHeadPosition(); pos != NULL;)
		{
			CPane* pBar = DYNAMIC_DOWNCAST(CPane, lst.GetNext(pos));
			ASSERT_VALID(pBar);

			if (pBar != this)
			{
				pBar->SetActiveInGroup(FALSE);
			}
		}

		CRect rect; rect.SetRectEmpty();
		m_pParentDockBar->RepositionPanes(rect);

	}
}

CDockablePane* CMFCAutoHideBar::GetFirstAHWindow()
{
	if (m_lstAutoHideButtons.IsEmpty())
	{
		return NULL;
	}

	CMFCAutoHideButton* pBtn = (CMFCAutoHideButton*) m_lstAutoHideButtons.GetHead();
	if (pBtn != NULL)
	{
		return pBtn->GetAutoHideWindow();
	}
	return NULL;
}


BOOL CMFCAutoHideBar::Create(LPCTSTR lpszClassName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	ENSURE( AfxIsExtendedFrameClass(pParentWnd) );

	return CPane::Create(lpszClassName, dwStyle, rect, pParentWnd, nID, dwControlBarStyle, pContext);
}

