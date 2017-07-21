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

#include "afxglobals.h"
#include "afxpanedivider.h"
#include "afxdockablepane.h"
#include "afxpanecontainermanager.h"
#include "afxpanecontainer.h"
#include "afxglobalutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CPaneDivider,CBasePane)

int CPaneDivider::m_nDefaultWidth = 4;
CRuntimeClass* CPaneDivider::m_pContainerManagerRTC = RUNTIME_CLASS(CPaneContainerManager);
CRuntimeClass* CPaneDivider::m_pSliderRTC = RUNTIME_CLASS(CPaneDivider);

/////////////////////////////////////////////////////////////////////////////
// CPaneDivider
CPaneDivider::CPaneDivider()
{
	Init();
}

CPaneDivider::CPaneDivider(BOOL bDefaultSlider, CWnd* pParentWnd)
{
	Init(bDefaultSlider, pParentWnd);
}

void CPaneDivider::Init(BOOL bDefaultSlider, CWnd* pParentWnd)
{
	m_nID = (UINT) -1;
	m_dwDividerStyle = 0;
	m_nWidth = 0;
	m_bCaptured = false;
	m_pContainerManager = NULL;
	m_bDefaultDivider = bDefaultSlider;

	m_rectLastDragRect.SetRectEmpty();
	m_rectDragBounds.SetRectEmpty();
	m_nMinOffset = 0;
	m_nMaxOffset = 0;
	m_nStep		 = -1;
	m_bAutoHideMode = FALSE;
	m_pParentWndForSerialize = pParentWnd;
}

CPaneDivider::~CPaneDivider()
{
}

BEGIN_MESSAGE_MAP(CPaneDivider, CBasePane)
	//{{AFX_MSG_MAP(CPaneDivider)
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_NCDESTROY()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaneDivider message handlers
BOOL CPaneDivider::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	ASSERT_VALID(this);

	return CPaneDivider::CreateEx(0L, dwStyle, rect, pParentWnd, nID, pContext);
}

BOOL CPaneDivider::CreateEx(DWORD dwStyleEx, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	ASSERT_VALID(this);

	m_nID = nID;
	m_dwDividerStyle = dwStyle;

	if (m_dwDividerStyle & SS_VERT)
	{
		m_nWidth = rect.right - rect.left;
	}
	else if (m_dwDividerStyle & SS_HORZ)
	{
		m_nWidth = rect.bottom - rect.top;
	}

	DWORD dwSliderStyle = m_dwDividerStyle | WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (m_bDefaultDivider)
	{
		ENSURE(m_pContainerManagerRTC != NULL);
		m_pContainerManager = DYNAMIC_DOWNCAST(CPaneContainerManager, m_pContainerManagerRTC->CreateObject());

		ENSURE(m_pContainerManager != NULL);
		m_pContainerManager->Create(pParentWnd, this);
	}

	m_pDockSite = DYNAMIC_DOWNCAST(CFrameWnd, pParentWnd);

	if (m_pDockSite == NULL)
	{
		m_pDockSite = AFXGetParentFrame(pParentWnd);
	}

	if (m_pDockSite == NULL)
	{
		ASSERT(FALSE);
	}

	return CWnd::CreateEx(dwStyleEx, afxGlobalData.RegisterWindowClass(_T("Afx:Slider")), NULL, dwSliderStyle, rect, pParentWnd, nID, pContext);
}

void CPaneDivider::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);
	CBasePane::Serialize(ar);

	CRect rect;

	if (ar.IsStoring())
	{
		GetWindowRect(rect);
		GetParent()->ScreenToClient(rect);

		ar << m_nID;
		ar << m_nStep;
		ar << rect;
		ar << IsWindowVisible();
		ar << m_dwDividerStyle;
		ar << m_nWidth;
		ar << m_bDefaultDivider;
		ar << m_nMinOffset;
		ar << m_nMaxOffset;
	}
	else
	{
		BOOL bVisible = FALSE;

		ar >> m_nID;
		ar >> m_nStep;
		ar >> rect;
		ar >> bVisible;
		ar >> m_dwDividerStyle;
		ar >> m_nWidth;
		ar >> m_bDefaultDivider;
		ar >> m_nMinOffset;
		ar >> m_nMaxOffset;

		if (bVisible)
		{
			m_dwDividerStyle |= WS_VISIBLE;
		}
		else
		{
			m_dwDividerStyle &= ~WS_VISIBLE;
		}

		if (!CreateEx(0, m_dwDividerStyle, rect, m_pParentWndForSerialize, m_nID, NULL))
		{
			TRACE0("Unable to create slider from archive");
		}
	}

	if (m_pContainerManager != NULL && m_bDefaultDivider)
	{
		m_pContainerManager->Serialize(ar);
	}

}

CDockablePane* CPaneDivider::FindTabbedPane(UINT nID)
{
	ASSERT_VALID(this);
	if (m_pContainerManager != NULL)
	{
		return m_pContainerManager->FindTabbedPane(nID);
	}
	return NULL;
}

void CPaneDivider::OnSize(UINT nType, int cx, int cy)
{
	ASSERT_VALID(this);
	CWnd::OnSize(nType, cx, cy);
}

BOOL CPaneDivider::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	ASSERT_VALID(this);

	switch (nHitTest)
	{
	case HTCLIENT:
		if (m_dwDividerStyle & SS_HORZ)
		{
			SetCursor(afxGlobalData.m_hcurStretchVert);
		}
		else if (m_dwDividerStyle & SS_VERT)
		{
			SetCursor(afxGlobalData.m_hcurStretch);
		}
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CPaneDivider::OnLButtonDown(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);
	if (!m_bCaptured)
	{
		m_bCaptured = true;
		SetCapture();

		CRect rectSlider;
		GetWindowRect(rectSlider);
		CSize size(m_nWidth / 2, m_nWidth / 2);
		CWindowDC dc(GetDesktopWindow());
		dc.DrawDragRect(&rectSlider, size, NULL, size);

		m_rectLastDragRect = rectSlider;
		m_rectDragBounds = rectSlider;

		if (m_pContainerManager != NULL)
		{
			m_pContainerManager->GetMinMaxOffset(this, m_nMinOffset, m_nMaxOffset, m_nStep);

			if (IsHorizontal())
			{
				m_rectDragBounds.top = rectSlider.top + m_nMinOffset;
				m_rectDragBounds.bottom = rectSlider.bottom + m_nMaxOffset;
			}
			else
			{
				m_rectDragBounds.left = rectSlider.left + m_nMinOffset;
				m_rectDragBounds.right = rectSlider.right + m_nMaxOffset;
			}

			m_pContainerManager->SetResizeMode(TRUE);
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CPaneDivider::OnLButtonUp(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);

	StopTracking(TRUE);
	CWnd::OnLButtonUp(nFlags, point);
}

void CPaneDivider::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);
	if (m_bCaptured)
	{
		CRect rectNew = m_rectLastDragRect;

		CPoint ptNew;
		GetCursorPos(&ptNew);

		if (m_dwDividerStyle & SS_VERT)
		{
			rectNew.left = ptNew.x - m_nWidth / 2;
			rectNew.right = rectNew.left + m_nWidth;
			if (rectNew.left < m_rectDragBounds.left)
			{
				rectNew.left = m_rectDragBounds.left;
				rectNew.right = rectNew.left + m_rectLastDragRect.Width();
			}

			if (rectNew.right > m_rectDragBounds.right)
			{
				rectNew.right = m_rectDragBounds.right;
				rectNew.left = rectNew.right - m_rectLastDragRect.Width();
			}
		}
		else
		{
			rectNew.top = ptNew.y - m_nWidth / 2;
			rectNew.bottom = rectNew.top + m_nWidth;
			if (rectNew.top < m_rectDragBounds.top)
			{
				rectNew.top = m_rectDragBounds.top;
				rectNew.bottom = rectNew.top + m_nWidth;
			}

			if (rectNew.bottom > m_rectDragBounds.bottom)
			{
				rectNew.bottom = m_rectDragBounds.bottom;
				rectNew.top = rectNew.bottom - m_nWidth;
			}
		}

		CSize size(m_nWidth / 2, m_nWidth / 2);
		CWindowDC dc(GetDesktopWindow());
		dc.DrawDragRect(&rectNew, size, &m_rectLastDragRect, size);
		m_rectLastDragRect = rectNew;
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CPaneDivider::OnCancelMode()
{
	StopTracking(FALSE);
	CBasePane::OnCancelMode();
}

void CPaneDivider::StopTracking(BOOL bMoveSlider)
{
	if (m_bCaptured)
	{
		CRect rectSlider;
		GetWindowRect(rectSlider);

		CPoint ptOffset = m_rectLastDragRect.TopLeft() - rectSlider.TopLeft();

		CRect rectEmpty;
		rectEmpty.SetRectEmpty();

		CWindowDC dc(NULL);
		CSize size(m_nWidth / 2, m_nWidth / 2);
		dc.DrawDragRect(&rectEmpty, size, &m_rectLastDragRect, size);

		if (bMoveSlider)
		{
			Move(ptOffset);
		}

		m_rectLastDragRect.SetRectEmpty();

		ReleaseCapture();
		m_bCaptured = false;

		if (m_pContainerManager != NULL)
		{
			m_pContainerManager->SetResizeMode(FALSE);
		}
	}
}

void CPaneDivider::OnPaint()
{
	ASSERT_VALID(this);
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(dc, this);

	CRect rectClient;
	GetClientRect(rectClient);
	CMFCVisualManager::GetInstance()->OnDrawPaneDivider(&memDC.GetDC(), this, rectClient, m_bAutoHideMode);
}

void CPaneDivider::Move(CPoint& ptOffset, BOOL /*bAdjustLayout*/)
{
	ASSERT_VALID(this);

	BOOL bIsRTL = GetParent()->GetExStyle() & WS_EX_LAYOUTRTL;

	CRect rectSlider;
	CRect rectSliderWnd;
	GetWindowRect(rectSlider);
	GetParent()->ScreenToClient(rectSlider);
	rectSliderWnd = rectSlider;

	int nOffset = 0;
	if (m_dwDividerStyle & SS_VERT)
	{
		nOffset = bIsRTL ? -ptOffset.x : ptOffset.x;
		//nOffset = ptOffset.x;
		rectSlider.OffsetRect(nOffset, 0);
	}
	else if (m_dwDividerStyle & SS_HORZ)
	{
		nOffset = ptOffset.y;
		rectSlider.OffsetRect(0, nOffset);
	}
	else
	{
		return;
	}

	HDWP hdwp = BeginDeferWindowPos(50);
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->OnPaneDividerMove(this, 0, nOffset, hdwp);
	}
	EndDeferWindowPos(hdwp);

	// it moves the slider
	AdjustDockingLayout();

	// CGlobalUtils::ScreenToClientUnmapped(GetParent(), rectSlider);
	/*
	int nLeftBound = rectSlider.left;
	if (bIsRTL && m_dwDividerStyle & SS_VERT)
	{
	GetParent()->ScreenToClient(rectSliderWnd);
	nLeftBound = rectSliderWnd.left - ptOffset.x;
	}
	*/
	// move the slider by ourself
	SetWindowPos(NULL, rectSlider.left, rectSlider.top, rectSlider.Width(), rectSlider.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);

}

void CPaneDivider::AddPane(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT_KINDOF(CDockablePane, pBar);

	m_pContainerManager->AddPane(pBar);
	CheckVisibility();
}

CDockablePane*  CPaneDivider::AddRecentPane(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT_KINDOF(CDockablePane, pBar);

	CDockablePane* pAddedBar = NULL;
	CPaneContainer* pRecentContainer = pBar->m_recentDockInfo.GetRecentPaneContainer(TRUE);
	CPaneContainer* pRecentTabContainer = pBar->m_recentDockInfo.GetRecentTabContainer(TRUE);
	if (pRecentContainer != NULL)
	{
		pAddedBar = m_pContainerManager->AddPaneToRecentPaneContainer(pBar, pRecentContainer);
		CheckVisibility();
	}
	else if (pRecentTabContainer != NULL)
	{
		pAddedBar = m_pContainerManager->AddPaneToRecentPaneContainer(pBar, pRecentTabContainer);
		CheckVisibility();
	}
	else
	{
		ASSERT(FALSE);
	}
	return pAddedBar;
}

BOOL CPaneDivider::InsertPane(CDockablePane* pBarToInsert, CDockablePane* pTargetBar, DWORD dwAlignment, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	ASSERT_KINDOF(CDockablePane, pBarToInsert);
	ASSERT_KINDOF(CDockablePane, pTargetBar);

	BOOL bResult = FALSE;
	if (m_pContainerManager != NULL)
	{
		bResult = m_pContainerManager->InsertPane(pBarToInsert, pTargetBar, dwAlignment, lpRect);
		CheckVisibility();
	}
	return bResult;
}

const CBasePane* CPaneDivider::GetFirstPane() const
{
	ASSERT_VALID(this);
	if (m_pContainerManager != NULL)
	{
		return m_pContainerManager->GetFirstPane();
	}
	return NULL;
}

void CPaneDivider::RemovePane(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT_KINDOF(CDockablePane, pBar);

	if (m_pContainerManager != NULL)
	{
		// remove the bar from container
		m_pContainerManager->RemovePaneFromPaneContainer(pBar);
		// remove the bar from docksite(do not destroy!!!)
		CBasePane::RemovePaneFromDockManager(pBar, FALSE, FALSE, m_bAutoHideMode);
		if (m_pContainerManager->IsEmpty() && m_pContainerManager->GetTotalRefCount() == 0 && pBar->m_recentDockInfo.GetRecentDefaultPaneDivider() != this)
		{
			// it was the last control bar in the container -
			// remove and DESTROY  the slider as well
			CBasePane::RemovePaneFromDockManager(this, TRUE, FALSE, m_bAutoHideMode);
		}
		else
		{
			if (!CheckVisibility())
			{
				ShowWindow(SW_HIDE);
			}
		}
	}
}

BOOL CPaneDivider::ReplacePane(CDockablePane* pBarToReplace, CDockablePane* pBarToReplaceWith)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBarToReplace);
	ASSERT_VALID(pBarToReplaceWith);
	ASSERT_KINDOF(CDockablePane, pBarToReplaceWith);

	if (m_pContainerManager == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pBarToReplaceWith->SetDefaultPaneDivider(m_hWnd);

	BOOL bResult = m_pContainerManager->ReplacePane(pBarToReplace, pBarToReplaceWith);
	CheckVisibility();
	return bResult;
}

BOOL CPaneDivider::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

CSize CPaneDivider::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	ASSERT_VALID(this);
	CRect rectWnd;
	GetWindowRect(&rectWnd);
	CSize size = rectWnd.Size();

	CRect rectContainer;
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->GetWindowRect(rectContainer);
		size += rectContainer.Size();
	}
	return size;
}

void CPaneDivider::RepositionPanes(CRect& rectNew, HDWP& hdwp)
{
	CRect rectContainer = rectNew;
	CRect rectNewSlider = rectNew;

	DWORD dwAlignment = GetCurrentAlignment();

	switch (dwAlignment)
	{
	case CBRS_ALIGN_LEFT:
		rectNewSlider.left = rectNew.right - m_nWidth;
		rectContainer.right = rectNewSlider.left;
		rectContainer.top = rectNewSlider.top;
		rectContainer.bottom = rectNewSlider.bottom;
		break;
	case CBRS_ALIGN_RIGHT:
		rectNewSlider.right = rectNew.left + m_nWidth;
		rectContainer.left = rectNewSlider.right;
		rectContainer.top = rectNewSlider.top;
		rectContainer.bottom = rectNewSlider.bottom;
		break;
	case CBRS_ALIGN_TOP:
		rectNewSlider.top = rectNew.bottom - m_nWidth;
		rectContainer.bottom = rectNewSlider.top;
		rectContainer.left = rectNewSlider.left;
		rectContainer.right = rectNewSlider.right;
		break;
	case CBRS_ALIGN_BOTTOM:
		rectNewSlider.bottom = rectNew.top + m_nWidth;
		rectContainer.top = rectNewSlider.bottom;
		rectContainer.left = rectNewSlider.left;
		rectContainer.right = rectNewSlider.right;
		break;
	}

	CWnd* pParentWnd = GetParent();
	ASSERT_VALID(pParentWnd);

	pParentWnd->ScreenToClient(rectNew);
	pParentWnd->ScreenToClient(rectNewSlider);
	pParentWnd->ScreenToClient(rectContainer);
	hdwp = MoveWindow(rectNewSlider, TRUE, hdwp);

	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->ResizePaneContainers(rectContainer, hdwp);
		CSize sizeMin;
		m_pContainerManager->GetMinSize(sizeMin);

		if (CPane::m_bHandleMinSize)
		{
			CObList lstBars;
			CObList lstSliders;
			m_pContainerManager->AddPanesToList(&lstBars, &lstSliders);

			if (rectContainer.Width() < sizeMin.cx || rectContainer.Height() < sizeMin.cy)
			{

				POSITION pos = NULL;
				for (pos = lstBars.GetHeadPosition(); pos != NULL;)
				{
					CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, lstBars.GetNext(pos));
					ASSERT_VALID(pBar);

					CRect rectWnd;
					pBar->GetWindowRect(rectWnd);
					pParentWnd->ScreenToClient(rectWnd);
					if (rectWnd.right > rectContainer.right)
					{
						rectWnd.right = rectContainer.right;
					}
					if (rectWnd.bottom > rectContainer.bottom)
					{
						rectWnd.bottom = rectContainer.bottom;
					}
					rectWnd.OffsetRect(-rectWnd.left, -rectWnd.top);
					CRgn rgn;
					rgn.CreateRectRgn(rectWnd.left, rectWnd.top, rectWnd.right, rectWnd.bottom);
					pBar->SetWindowRgn(rgn, TRUE);
				}

				for (pos = lstSliders.GetHeadPosition(); pos != NULL;)
				{
					CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, lstBars.GetNext(pos));
					ASSERT_VALID(pSlider);

					pSlider->SetWindowPos(&CWnd::wndBottom, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
			else
			{
				for (POSITION pos = lstBars.GetHeadPosition(); pos != NULL;)
				{
					CDockablePane* pBar = (CDockablePane*) lstBars.GetNext(pos);
					pBar->SetWindowRgn(NULL, TRUE);
				}
			}
		}
	}
}

void CPaneDivider::OnDestroy()
{

	CBasePane::OnDestroy();
}

void CPaneDivider::OnNcDestroy()
{
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->RemovePaneDivider(this);
	}

	if (m_pContainerManager != NULL && m_bDefaultDivider)
	{
		delete m_pContainerManager;
		m_pContainerManager = NULL;
	}

	CBasePane::OnNcDestroy();
	delete this;
}

void CPaneDivider::ShowWindow(int nCmdShow)
{
	CWnd::ShowWindow(nCmdShow);
}

int CPaneDivider::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBasePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (afxGlobalData.m_hcurStretch == NULL)
	{
		afxGlobalData.m_hcurStretch = AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR);
	}

	if (afxGlobalData.m_hcurStretchVert == NULL)
	{
		afxGlobalData.m_hcurStretchVert = AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR);
	}

	return 0;
}

void CPaneDivider::StoreRecentDockSiteInfo(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->StoreRecentDockSiteInfo(pBar);
	}
}

void CPaneDivider::StoreRecentTabRelatedInfo(CDockablePane* pDockingBar, CDockablePane* pTabbedBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDockingBar);
	ASSERT_VALID(pTabbedBar);

	if (m_pContainerManager != NULL)
	{
		BOOL bLeftBar = FALSE;
		CPaneContainer* pTabbedContainer = m_pContainerManager->FindPaneContainer(pTabbedBar, bLeftBar);
		if (pTabbedContainer != NULL)
		{
			pDockingBar->
				m_recentDockInfo.StoreDockInfo(pTabbedContainer, pTabbedBar);
		}
		else
		{
			ASSERT(FALSE);
		}
	}
}

CPaneContainer* CPaneDivider::FindPaneContainer(CDockablePane* pBar, BOOL& bLeftBar)
{
	ASSERT_VALID(this);

	return m_pContainerManager->FindPaneContainer(pBar, bLeftBar);
}

BOOL CPaneDivider::AddPaneContainer(CPaneContainerManager& barContainerManager, BOOL bOuterEdge)
{
	BOOL bResult = FALSE;
	if (m_pContainerManager != NULL)
	{
		bResult = m_pContainerManager->AddPaneContainerManager(barContainerManager, bOuterEdge);
		CheckVisibility();
	}
	return bResult;
}

BOOL CPaneDivider::AddPaneContainer(CDockablePane* pTargetBar, CPaneContainerManager& barContainerManager, DWORD dwAlignment)
{
	BOOL bResult = FALSE;
	if (m_pContainerManager != NULL)
	{
		bResult = m_pContainerManager->AddPaneContainerManager(pTargetBar, dwAlignment, barContainerManager, TRUE);
		CheckVisibility();
	}
	return bResult;
}

void CPaneDivider::OnShowPane(CDockablePane* pBar, BOOL bShow)
{
	if (m_pContainerManager != NULL && !IsAutoHideMode())
	{
		BOOL bNewVisibleState = m_pContainerManager->OnShowPane(pBar, bShow);
		if (!bShow)
		{
			// actual only for hide, because when bShow is TRUE slider must be always
			// visible
			ShowWindow(bNewVisibleState ? SW_SHOW : SW_HIDE);
			BOOL bLeftBar = FALSE;
			CPaneContainer* pContainer = m_pContainerManager->FindPaneContainer(pBar, bLeftBar);
			if (pContainer != NULL)
			{
				pContainer->OnShowPane(pBar, bShow);
			}
		}
		else
		{
			ShowWindow(SW_SHOW);
		}
	}
}

BOOL CPaneDivider::CheckVisibility()
{
	if (m_bDefaultDivider && !IsAutoHideMode() && m_pContainerManager != NULL)
	{
		BOOL bIsRootContainerVisible = m_pContainerManager->IsRootPaneContainerVisible();
		ShowWindow(bIsRootContainerVisible ? SW_SHOW : SW_HIDE);
		return bIsRootContainerVisible;
	}

	return FALSE;
}

BOOL CPaneDivider::DoesContainFloatingPane()
{
	if (m_pContainerManager != NULL)
	{
		return m_pContainerManager->DoesContainFloatingPane();
	}
	return FALSE;
}

BOOL CPaneDivider::DoesAllowDynInsertBefore() const
{
	if (m_pContainerManager != NULL)
	{
		return m_pContainerManager->DoesAllowDynInsertBefore();
	}
	return TRUE;
}

void CPaneDivider::CalcExpectedDockedRect(CWnd* pWndToDock, CPoint ptMouse, CRect& rectResult, BOOL& bDrawTab, CDockablePane** ppTargetBar)
{
	CGlobalUtils globalUtilsLocal;
	if (m_pContainerManager != NULL)
	{
		globalUtilsLocal.CalcExpectedDockedRect(*m_pContainerManager, pWndToDock, ptMouse, rectResult, bDrawTab, ppTargetBar);
	}
}

void CPaneDivider::NotifyAboutRelease()
{
	if (m_pContainerManager->IsEmpty() && m_pContainerManager->GetTotalRefCount() == 0)
	{
		// it was the last control bar in the container -
		// remove and DESTROY  the slider as well
		CBasePane::RemovePaneFromDockManager(this, TRUE, FALSE, m_bAutoHideMode);
	}
}

void CPaneDivider::GetPanes(CObList& lstBars)
{
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->AddPanesToList(&lstBars, NULL);
	}
}

void CPaneDivider::GetPaneDividers(CObList& lstSliders)
{
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->AddPanesToList(NULL, &lstSliders);
	}
}

void CPaneDivider::ReleaseEmptyPaneContainers()
{
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->ReleaseEmptyPaneContainers();
	}
}

CRect CPaneDivider::GetRootContainerRect()
{
	CRect rect; rect.SetRectEmpty();
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->GetWindowRect(rect);
	}
	return rect;
}



