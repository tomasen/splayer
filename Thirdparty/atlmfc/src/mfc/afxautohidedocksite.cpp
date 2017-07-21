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
#include "afxautohidedocksite.h"
#include "afxpane.h"
#include "afxdockingpanesrow.h"
#include "afxvisualmanager.h"
#include "afxautohidebar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CAutoHideDockSite::m_nExtraSpace = 2;

IMPLEMENT_DYNCREATE(CAutoHideDockSite, CDockSite)

/////////////////////////////////////////////////////////////////////////////
// CAutoHideDockSite

CAutoHideDockSite::CAutoHideDockSite()
{
	m_nOffsetLeft = 0;
	m_nOffsetRight = 0;
}

CAutoHideDockSite::~CAutoHideDockSite()
{
}

BEGIN_MESSAGE_MAP(CAutoHideDockSite, CDockSite)
	//{{AFX_MSG_MAP(CAutoHideDockSite)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAutoHideDockSite::DockPane(CPane* pControlBar, AFX_DOCK_METHOD /*dockMethod*/, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	BOOL bVertDock = !IsHorizontal();
	CSize szBarSize = pControlBar->CalcFixedLayout(FALSE, !bVertDock);

	// the control bar doesn't take up all space of the row
	int nRowHeight = bVertDock ? szBarSize.cx + m_nExtraSpace : szBarSize.cy + m_nExtraSpace;

	if (!m_lstControlBars.Find(pControlBar))
	{
		CDockingPanesRow* pRowToDock = NULL;

		if (m_lstDockBarRows.IsEmpty())
		{
			pRowToDock = AddRow(NULL, nRowHeight);
			if (GetCurrentAlignment() & CBRS_ALIGN_LEFT || GetCurrentAlignment() & CBRS_ALIGN_TOP)
			{
				pRowToDock->SetExtra(m_nExtraSpace, AFX_ROW_ALIGN_TOP);
			}
			else
			{
				pRowToDock->SetExtra(m_nExtraSpace, AFX_ROW_ALIGN_BOTTOM);
			}
		}
		else
		{
			pRowToDock = (CDockingPanesRow*) m_lstDockBarRows.GetHead();
		}

		ASSERT_VALID(pRowToDock);
		// the bar should be placed on the existing row or new row
		pRowToDock->AddPane(pControlBar, DM_RECT, lpRect, TRUE);

		ShowWindow(SW_SHOW);

		m_lstControlBars.AddTail(pControlBar);
		AdjustDockingLayout();
		CRect rectClient;
		GetClientRect(rectClient);
		RepositionPanes(rectClient);

	}
}

void CAutoHideDockSite::RepositionPanes(CRect& /*rectNewClientArea*/)
{
	ASSERT_VALID(this);

	if (!m_lstDockBarRows.IsEmpty())
	{
		CDockingPanesRow* pRow = (CDockingPanesRow*) m_lstDockBarRows.GetHead();
		ASSERT_VALID(pRow);

		pRow->ArrangePanes(m_nOffsetLeft + afxGlobalData.m_nAutoHideToolBarMargin, afxGlobalData.m_nAutoHideToolBarSpacing);

		if (CMFCVisualManager::GetInstance()->HasOverlappedAutoHideButtons())
		{
			pRow->RedrawAll();
		}
	}
}

void CAutoHideDockSite::UnSetAutoHideMode(CMFCAutoHideBar* pAutohideToolbar)
{
	if (pAutohideToolbar == NULL)
	{
		CObList lstBars;
		lstBars.AddTail(&m_lstControlBars);

		POSITION posSave = NULL;
		POSITION pos = NULL;

		for (pos = lstBars.GetHeadPosition(); pos != NULL;)
		{
			posSave = pos;
			CMFCAutoHideBar* pToolBar = (CMFCAutoHideBar*) lstBars.GetNext(pos);
			if (!pToolBar->m_bFirstInGroup)
			{
				lstBars.RemoveAt(posSave);
			}
		}

		for (pos = lstBars.GetHeadPosition(); pos != NULL;)
		{
			CMFCAutoHideBar* pToolBar = (CMFCAutoHideBar*) lstBars.GetNext(pos);
			UnSetAutoHideMode(pToolBar);
		}
		return;
	}

	// find the group;
	CDockingPanesRow* pRow = RowFromPane(pAutohideToolbar);

	CObList lstGroup;
	if (pRow != NULL)
	{
		pRow->GetGroupFromPane(pAutohideToolbar, lstGroup);
	}

	if (lstGroup.IsEmpty())
	{
		pAutohideToolbar->UnSetAutoHideMode(NULL);
	}
	else
	{
		BOOL bFirstBar = TRUE;
		CDockablePane* pFirstBar = NULL;
		for (POSITION pos = lstGroup.GetHeadPosition(); pos != NULL;)
		{
			CMFCAutoHideBar* pNextBar = DYNAMIC_DOWNCAST(CMFCAutoHideBar, lstGroup.GetNext(pos));
			if (pNextBar != NULL)
			{
				if (bFirstBar)
				{
					pFirstBar = pNextBar->GetFirstAHWindow();
					pNextBar->UnSetAutoHideMode(NULL);
					bFirstBar = FALSE;
				}
				else
				{
					pNextBar->UnSetAutoHideMode(pFirstBar);
				}
			}
		}
	}
}

void CAutoHideDockSite::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(dc, this);

	CRect rectClient;
	GetClientRect(rectClient);

	CMFCVisualManager::GetInstance()->OnFillBarBackground(&memDC.GetDC(), this, rectClient, rectClient);
}

void CAutoHideDockSite::GetAlignRect(CRect& rect) const
{
	GetWindowRect(rect);

	if (IsHorizontal())
	{
		rect.left += m_nOffsetLeft;
		rect.right -= m_nOffsetRight;
	}
	else
	{
		rect.top += m_nOffsetLeft;
		rect.bottom -= m_nOffsetRight;
	}
}

BOOL CAutoHideDockSite::CanAcceptPane(const CBasePane* pBar) const
{
	return pBar->IsKindOf(RUNTIME_CLASS(CMFCAutoHideBar));
}



