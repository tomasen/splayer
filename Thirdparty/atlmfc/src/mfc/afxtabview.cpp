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
#include "afxtabview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabView

IMPLEMENT_DYNCREATE(CTabView, CView)

CTabView::CTabView()
{
	m_bIsReady = FALSE;
	m_nFirstActiveTab = -1;
}

CTabView::~CTabView()
{
}

BEGIN_MESSAGE_MAP(CTabView, CView)
	//{{AFX_MSG_MAP(CTabView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOUSEACTIVATE()
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, &CTabView::OnChangeActiveTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabView drawing

void CTabView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CTabView diagnostics

#ifdef _DEBUG
void CTabView::AssertValid() const
{
	CView::AssertValid();
}

void CTabView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTabView message handlers

int CTabView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
	if (!m_wndTabs.Create(IsScrollBar () ? CMFCTabCtrl::STYLE_FLAT_SHARED_HORZ_SCROLL : CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("Failed to create tab window\n");
		return -1;      // fail to create
	}

	m_wndTabs.SetFlatFrame();
	m_wndTabs.SetTabBorderSize(0);
	m_wndTabs.AutoDestroyWindow(FALSE);
	return 0;
}

void CTabView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// Tab control should cover a whole client area:
	m_wndTabs.SetWindowPos(NULL, -1, -1, cx + 1, cy + 3, SWP_NOACTIVATE | SWP_NOZORDER);
}

int CTabView::AddView(CRuntimeClass* pViewClass, const CString& strViewLabel, int iIndex /*= -1*/, CCreateContext* pContext/* = NULL*/)
{
	ASSERT_VALID(this);
	ENSURE(pViewClass != NULL);
	ENSURE(pViewClass->IsDerivedFrom(RUNTIME_CLASS(CView)));

	CView* pView = DYNAMIC_DOWNCAST(CView, pViewClass->CreateObject());
	ASSERT_VALID(pView);

	if (!pView->Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), &m_wndTabs, (UINT) -1, pContext))
	{
		TRACE1("CTabView:Failed to create view '%s'\n", pViewClass->m_lpszClassName);
		return -1;
	}

	CDocument* pDoc = GetDocument();
	if (pDoc != NULL)
	{
		ASSERT_VALID(pDoc);

		BOOL bFound = FALSE;
		for (POSITION pos = pDoc->GetFirstViewPosition(); !bFound && pos != NULL;)
		{
			if (pDoc->GetNextView(pos) == pView)
			{
				bFound = TRUE;
			}
		}

		if (!bFound)
		{
			pDoc->AddView(pView);
		}
	}

	m_wndTabs.InsertTab(pView, strViewLabel, iIndex);

	int nTabs = m_wndTabs.GetTabsNum();
	return nTabs - 1;
}

LRESULT CTabView::OnChangeActiveTab(WPARAM wp, LPARAM)
{
	if (!m_bIsReady)
	{
		m_nFirstActiveTab = (int) wp;
		return 0;
	}

	CFrameWnd* pFrame = AFXGetParentFrame(this);
	ASSERT_VALID(pFrame);

	int iTabNum = (int) wp;
	if (iTabNum >= 0)
	{
		CView* pView = DYNAMIC_DOWNCAST(CView, m_wndTabs.GetTabWnd(iTabNum));
		ASSERT_VALID(pView);

		pFrame->SetActiveView(pView);

		OnActivateView(pView);
	}
	else
	{
		pFrame->SetActiveView(NULL);

		OnActivateView(NULL);
	}

	return 0;
}

int CTabView::FindTab(HWND hWndView) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_wndTabs.GetTabsNum(); i++)
	{
		if (m_wndTabs.GetTabWnd(i)->GetSafeHwnd() == hWndView)
		{
			return i;
		}
	}

	return -1;
}

BOOL CTabView::RemoveView(int iTabNum)
{
	ASSERT_VALID(this);
	return m_wndTabs.RemoveTab(iTabNum);
}

BOOL CTabView::SetActiveView(int iTabNum)
{
	ASSERT_VALID(this);
	return m_wndTabs.SetActiveTab(iTabNum);
}

CView* CTabView::GetActiveView() const
{
	ASSERT_VALID(this);

	int iActiveTab = m_wndTabs.GetActiveTab();
	if (iActiveTab < 0)
	{
		return NULL;
	}

	return DYNAMIC_DOWNCAST(CView, m_wndTabs.GetTabWnd(iActiveTab));
}

class CInternalTabView : public CView
{
	friend class CTabView;
};

int CTabView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	CView* pCurrView = GetActiveView();
	if (pCurrView == NULL)
	{
		return CView::OnMouseActivate(pDesktopWnd, nHitTest, message);
	}

	int nResult = CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
	if (nResult == MA_NOACTIVATE || nResult == MA_NOACTIVATEANDEAT)
		return nResult;   // frame does not want to activate

	CFrameWnd* pParentFrame = AFXGetParentFrame(this);
	if (pParentFrame != NULL)
	{
		// eat it if this will cause activation
		ASSERT(pParentFrame == pDesktopWnd || pDesktopWnd->IsChild(pParentFrame));

		// either re-activate the current view, or set this view to be active
		CView* pView = pParentFrame->GetActiveView();
		HWND hWndFocus = ::GetFocus();
		if (pView == pCurrView && pCurrView->m_hWnd != hWndFocus && !::IsChild(pCurrView->m_hWnd, hWndFocus))
		{
			// re-activate this view
			((CInternalTabView*)pCurrView)->OnActivateView(TRUE, pCurrView, pCurrView);
		}
		else
		{
			// activate this view
			pParentFrame->SetActiveView(pCurrView);
		}
	}

	return nResult;
}

void CTabView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_bIsReady = TRUE;
	OnChangeActiveTab(m_nFirstActiveTab, 0);
}


