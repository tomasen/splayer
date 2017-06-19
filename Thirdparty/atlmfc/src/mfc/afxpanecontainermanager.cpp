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
#include "afxpane.h"
#include "afxdockablepane.h"
#include "afxbasetabbedpane.h"
#include "afxpanedivider.h"
#include "afxpanecontainermanager.h"
#include "afxpanecontainer.h"
#include "afxdockingmanager.h"
#include "afxglobalutils.h"
#include "afxpaneframewnd.h"
#include "afxmultipaneframewnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static int g_nSliderSpacingForMove = 4;

static int g_nSliderID = 1;

IMPLEMENT_DYNCREATE(CPaneContainerManager, CObject)

// Construction/Destruction
CPaneContainerManager::CPaneContainerManager() :
m_pRootContainer(NULL), m_pDockSite(NULL), m_pContainerRTC(NULL), m_pDefaultSlider(NULL), m_bDestroyRootContainer(TRUE)
{
}

CPaneContainerManager::~CPaneContainerManager()
{
	// should not be destroyed when the root container is "exported" to another
	// manager (when one multi miniframe is docked to another multi miniframe)
	if (m_bDestroyRootContainer)
	{
		for (POSITION pos = m_lstSliders.GetHeadPosition(); pos != NULL;)
		{
			CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, m_lstSliders.GetNext(pos));
			if (pSlider != NULL)
			{
				pSlider->SetPaneContainerManager(NULL);
			}
		}
		if (m_pRootContainer != NULL)
		{
			delete m_pRootContainer;
		}
	}
}

BOOL CPaneContainerManager::Create(CWnd* pParentWnd, CPaneDivider* pDefaultSlider, CRuntimeClass* pContainerRTC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParentWnd);
	m_pDockSite = pParentWnd;
	m_pContainerRTC = pContainerRTC;

	ENSURE(m_pRootContainer == NULL);

	if (m_pContainerRTC != NULL)
	{
		m_pRootContainer = (CPaneContainer*) m_pContainerRTC->CreateObject();
		m_pRootContainer->SetPaneContainerManager(this);
	}
	else
	{
		m_pRootContainer = new CPaneContainer(this);
	}

	m_pDefaultSlider = pDefaultSlider;
	return TRUE;
}

void CPaneContainerManager::AddPane(CDockablePane* pControlBarToAdd)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBarToAdd);
	ASSERT_KINDOF(CDockablePane, pControlBarToAdd);

	m_pRootContainer->SetPane(pControlBarToAdd, TRUE);
	m_lstControlBars.AddTail(pControlBarToAdd);
}

BOOL CPaneContainerManager::AddPaneContainerManager(CPaneContainerManager& srcManager, BOOL bOuterEdge)
{
	ASSERT_VALID(this);
	ENSURE(m_pRootContainer != NULL);

	if (!m_pRootContainer->IsEmpty())
	{
		return FALSE;
	}

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(m_pDockSite);
	if (pDockManager == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_lstControlBars.RemoveAll();
	m_lstSliders.RemoveAll();

	srcManager.AddPanesToList(&m_lstControlBars, &m_lstSliders);
	srcManager.RemoveAllPanesAndPaneDividers();

	// we must copy containers before SetParent, because Copy sets recent dock info
	// internally and we need the "old" parent for that
	CPaneContainer* pNewContainer = srcManager.m_pRootContainer->Copy(m_pRootContainer);
	m_pRootContainer->SetPaneContainer(pNewContainer, TRUE);
	pNewContainer->SetPaneContainerManager(this, TRUE);

	afxGlobalUtils.SetNewParent(m_lstControlBars, m_pDockSite);
	afxGlobalUtils.SetNewParent(m_lstSliders, m_pDockSite);

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane,  m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pBar);
		// move them out of screen
		CRect rect;
		pBar->GetWindowRect(rect);
		pBar->GetParent()->ScreenToClient(rect);
	}

	// set new container manager for each slider
	for (pos = m_lstSliders.GetHeadPosition(); pos != NULL;)
	{
		CPaneDivider* pSlider = (CPaneDivider*) m_lstSliders.GetNext(pos);
		ASSERT_VALID(pSlider);

		pSlider->SetPaneContainerManager(this);
	}

	// finally, enable caption for each control bar in the container manager
	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane,  m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pBar);

		pBar->SetDefaultPaneDivider(m_pDefaultSlider->m_hWnd);
		pBar->SetPaneAlignment(m_pDefaultSlider->GetCurrentAlignment());
		pDockManager->AddPane(pBar, !bOuterEdge, FALSE, bOuterEdge);

		pBar->EnableGripper(TRUE);
	}

	m_pRootContainer->CheckPaneDividerVisibility();
	m_pRootContainer->CalculateRecentSize();

	return TRUE;
}

BOOL CPaneContainerManager::AddPaneContainerManager(CDockablePane* pTargetControlBar, DWORD dwAlignment, CPaneContainerManager& srcManager, BOOL bCopy)
{
	CObList lstControlBars;
	CObList lstSliders;

	srcManager.AddPanesToList(&lstControlBars, &lstSliders);

	BOOL bLeftBar = FALSE;
	CPaneContainer* pContainer = FindPaneContainer(pTargetControlBar, bLeftBar);

	if (pContainer == NULL)
	{
		return FALSE;
	}

	POSITION posTargetBar = m_lstControlBars.Find(pTargetControlBar);

	if (posTargetBar == NULL)
	{
		return FALSE;
	}

	CPaneContainer* pNewContainer = NULL;

	if (!bCopy)
	{
		pNewContainer = srcManager.m_pRootContainer;
	}
	else
	{
		// we must copy containers before SetParent, because Copy sets recent dock info
		// internally and we need the "old" parent for that
		pNewContainer = srcManager.m_pRootContainer->Copy(m_pRootContainer);
		pNewContainer->SetPaneContainerManager(this, TRUE);
		srcManager.RemoveAllPanesAndPaneDividers();
	}

	CWnd* pOldParent = srcManager.GetDockSiteFrameWnd();

	afxGlobalUtils.SetNewParent(lstControlBars, m_pDockSite);
	afxGlobalUtils.SetNewParent(lstSliders, m_pDockSite);

	if (!AddPaneAndPaneContainer(pTargetControlBar, pNewContainer, dwAlignment))
	{
		// reparent back
		afxGlobalUtils.SetNewParent(lstControlBars, pOldParent);
		afxGlobalUtils.SetNewParent(lstSliders, pOldParent);
		return FALSE;
	}

	BOOL bInsertBefore = (dwAlignment & CBRS_ALIGN_TOP) || (dwAlignment & CBRS_ALIGN_LEFT);

	// add/insert control bars and sliders from the manager is being added
	if (bInsertBefore)
	{
		for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CWnd* pWnd = (CWnd*) lstControlBars.GetNext(pos);
			m_lstControlBars.InsertBefore(posTargetBar, pWnd);
		}
	}
	else
	{
		for (POSITION pos = lstControlBars.GetTailPosition(); pos != NULL;)
		{
			CWnd* pWnd = (CWnd*) lstControlBars.GetPrev(pos);
			m_lstControlBars.InsertAfter(posTargetBar, pWnd);
		}
	}

	m_lstSliders.AddTail(&lstSliders);

	POSITION pos = NULL;

	// set new container manager for each slider
	for (pos = lstSliders.GetHeadPosition(); pos != NULL;)
	{
		CPaneDivider* pSlider = (CPaneDivider*) lstSliders.GetNext(pos);
		ASSERT_VALID(pSlider);

		pSlider->SetPaneContainerManager(this);
	}

	if (!bCopy)
	{
		srcManager.m_bDestroyRootContainer = FALSE;
		srcManager.m_pRootContainer->SetPaneContainerManager(this, TRUE);
	}

	// finally, enable caption for each control bar in the container manager
	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane,  m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pBar);

		pBar->EnableGripper(TRUE);
		pBar->RedrawWindow();
	}

	m_pRootContainer->CheckPaneDividerVisibility();
	m_pRootContainer->CalculateRecentSize();

	return TRUE;
}

BOOL CPaneContainerManager::AddPaneContainerManagerToDockablePane(CDockablePane* pTargetControlBar, CPaneContainerManager& srcManager)
{
	CObList lstControlBars;

	srcManager.AddPanesToList(&lstControlBars, NULL);

	BOOL bLeftBar = FALSE;
	CPaneContainer* pContainer = FindPaneContainer(pTargetControlBar, bLeftBar);

	if (pContainer == NULL)
	{
		return FALSE;
	}

	POSITION posTargetBar = m_lstControlBars.Find(pTargetControlBar);

	if (posTargetBar == NULL)
	{
		return FALSE;
	}

	CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pTargetControlBar);
	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		if (pBar != NULL)
		{
			if (pTabbedBar == NULL)
			{
				pBar->AttachToTabWnd(pTargetControlBar, DM_MOUSE, TRUE, (CDockablePane**) &pTabbedBar);
			}
			else
			{
				pBar->AttachToTabWnd(pTabbedBar, DM_MOUSE, TRUE, (CDockablePane**) &pTabbedBar);
			}
		}
	}

	return TRUE;
}

BOOL CPaneContainerManager::InsertPane(CDockablePane* pControlBarToInsert,
											CDockablePane* pTargetControlBar, DWORD dwAlignment, LPCRECT /*lpRect*/, AFX_DOCK_METHOD /*dockMethod*/)
{
	ASSERT_VALID(this);
	ENSURE(m_pRootContainer != NULL);
	ASSERT_VALID(pControlBarToInsert);
	ASSERT_KINDOF(CDockablePane, pControlBarToInsert);

	BOOL bResult = FALSE;
	if (pTargetControlBar != NULL)
	{
		POSITION pos = m_lstControlBars.Find(pTargetControlBar);
		if (pos != NULL)
		{
			bResult = AddPaneAndPaneDivider(pTargetControlBar, pControlBarToInsert, pos, dwAlignment);
		}
		else
		{
			TRACE0("TargetControlBar does not belong to the container. Docking failed\n");
			ASSERT(FALSE);
		}
	}
	return bResult;
}

CDockablePane* CPaneContainerManager::AddPaneToRecentPaneContainer(CDockablePane* pBarToAdd, CPaneContainer* pRecentContainer)
{
	ASSERT_VALID(this);
	ASSERT_KINDOF(CDockablePane, pBarToAdd);
	ASSERT_VALID(pRecentContainer);

	CPaneContainer::BC_FIND_CRITERIA searchType = CPaneContainer::BC_FIND_BY_CONTAINER;
	CPaneContainer* pContainer = m_pRootContainer->FindSubPaneContainer(pRecentContainer, searchType);

	if (pContainer == NULL)
	{
		return NULL;
	}

	if (!pContainer->IsEmpty() && pContainer->GetPaneDivider() == NULL)
	{
		CPaneDivider* pSlider = CreatePaneDivider(pContainer->GetRecentPaneDividerRect(), pContainer->GetRecentPaneDividerStyle());
		pContainer->SetPaneDivider(pSlider);
	}

	if (pContainer->IsEmpty())
	{
		// this container becomes non-empty
		// we need to ckeck parent containers in order to ensure their
		// slider existance
		CPaneContainer* pParentContainer = pContainer->GetParentPaneContainer();
		while (pParentContainer != m_pRootContainer && pParentContainer != NULL)
		{
			if (pParentContainer->GetPaneDivider() == NULL && pParentContainer->GetRecentPaneDividerStyle() != 0)
			{
				CPaneDivider* pSlider = CreatePaneDivider(pParentContainer->GetRecentPaneDividerRect(), pParentContainer->GetRecentPaneDividerStyle());
				pParentContainer->SetPaneDivider(pSlider);
			}
			pParentContainer = pParentContainer->GetParentPaneContainer();
		}
	}

	BOOL bDockSiteIsMiniFrame = m_pDockSite->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd));
	CObList lstRecentListOfBars;
	lstRecentListOfBars.AddTail(&pBarToAdd->m_recentDockInfo.GetRecentListOfPanes(!bDockSiteIsMiniFrame));
	// scan recent list of bars and look for the bar to insert after
	POSITION posRecent = lstRecentListOfBars.Find(pBarToAdd);

	// it may be different from pBarToAdd in case of tabbed window
	CDockablePane* pAddedBar = pContainer->AddPane(pBarToAdd);
	if (pAddedBar == pBarToAdd)
	{
		m_pRootContainer->CheckPaneDividerVisibility();

		while (posRecent != NULL)
		{
			CDockablePane* p = (CDockablePane*) lstRecentListOfBars.GetPrev(posRecent);

			ASSERT_VALID(p);

			POSITION posCurrent = m_lstControlBars.Find(p);
			if (posCurrent != NULL)
			{
				m_lstControlBars.InsertAfter(posCurrent, pAddedBar);
				return pAddedBar;
			}
		}

		m_lstControlBars.AddHead(pAddedBar);
		return pAddedBar;
	}

	return pAddedBar;
}

void CPaneContainerManager::StoreRecentDockSiteInfo(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	BOOL bLeftBar = TRUE;
	CPaneContainer* pContainer = FindPaneContainer(pBar, bLeftBar);

	if (pContainer != NULL)
	{
		pContainer->StoreRecentDockSiteInfo(pBar);
	}
}

CBasePane* CPaneContainerManager::GetFirstPane() const
{
	ASSERT_VALID(this);
	if (!m_lstControlBars.IsEmpty())
	{
		return (CBasePane*) m_lstControlBars.GetHead();
	}

	return NULL;
}

BOOL CPaneContainerManager::RemovePaneFromPaneContainer(CDockablePane* pControlBar)
{
	ASSERT_VALID(this);
	if (m_pRootContainer == NULL)
	{
		return FALSE;
	}

	BOOL bLeftBar = FALSE;
	CPaneContainer* pContainer = FindPaneContainer(pControlBar, bLeftBar);

	if (pContainer == NULL)
	{
		return FALSE;
	}

	pContainer->DeletePane(pControlBar, bLeftBar ?  CPaneContainer::BC_FIND_BY_LEFT_BAR : CPaneContainer::BC_FIND_BY_RIGHT_BAR);

	m_pRootContainer->CheckPaneDividerVisibility();

	CPaneDivider* pSlider = (CPaneDivider*) pContainer->GetPaneDivider();

	if (pSlider != NULL)
	{
		POSITION pos = m_lstSliders.Find(pSlider);
		ENSURE(pos != NULL);

		pSlider->ShowWindow(SW_HIDE);
	}

	POSITION pos = m_lstControlBars.Find(pControlBar);

	if (pos != NULL)
	{
		CList<HWND,HWND> lstRecentBarHandles;
		for (POSITION posBar = m_lstControlBars.GetHeadPosition(); posBar != NULL;)
		{
			CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_lstControlBars.GetNext(posBar));
			ASSERT_VALID(pWnd);

			lstRecentBarHandles.AddTail(pWnd->GetSafeHwnd());
		}

		BOOL bDockSiteIsMiniFrame = m_pDockSite->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd));
		pControlBar->m_recentDockInfo.SaveListOfRecentPanes(lstRecentBarHandles, !bDockSiteIsMiniFrame);
		m_lstControlBars.RemoveAt(pos);
	}

	return TRUE;
}

BOOL CPaneContainerManager::OnShowPane(CDockablePane* /*pBar*/, BOOL /*bShow*/)
{
	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->CheckPaneDividerVisibility();
	}

	return IsRootPaneContainerVisible();
}

int CPaneContainerManager::OnPaneDividerMove(CPaneDivider* pSlider, UINT /*uFlags*/, int nOffset, HDWP& hdwp)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSlider);

	CSize sizeMinContainer;
	CRect rectContainer;
	m_pRootContainer->GetWindowRect(rectContainer);
	m_pRootContainer->GetMinSize(sizeMinContainer);

	// check whether it is the default slider
	if (pSlider == m_pDefaultSlider)
	{
		DWORD dwSliderAlignment = pSlider->GetCurrentAlignment();
		m_pDockSite->ScreenToClient(rectContainer);
		BOOL bIsRTL = m_pDockSite->GetExStyle() & WS_EX_LAYOUTRTL;
		switch (dwSliderAlignment)
		{
		case CBRS_ALIGN_LEFT:
			if (bIsRTL)
			{
				rectContainer.left += nOffset;
			}
			else
			{
				rectContainer.right += nOffset;
			}
			if (rectContainer.Width() < sizeMinContainer.cx)
			{
				rectContainer.right = rectContainer.left + sizeMinContainer.cx;
			}
			break;
		case CBRS_ALIGN_RIGHT:
			if (bIsRTL)
			{
				rectContainer.right += nOffset;
			}
			else
			{
				rectContainer.left += nOffset;
			}
			if (rectContainer.Width() < sizeMinContainer.cx)
			{
				rectContainer.left = rectContainer.right - sizeMinContainer.cx;
			}
			break;
		case CBRS_ALIGN_TOP:
			rectContainer.bottom += nOffset;
			if (rectContainer.Height() < sizeMinContainer.cy)
			{
				rectContainer.bottom = rectContainer.top + sizeMinContainer.cy;
			}
			break;
		case CBRS_ALIGN_BOTTOM:
			rectContainer.top += nOffset;
			if (rectContainer.Height() < sizeMinContainer.cy)
			{
				rectContainer.top = rectContainer.bottom - sizeMinContainer.cy;
			}
			break;
		}

		ResizePaneContainers(rectContainer, hdwp);

		return 0;
	}

	CRect rectSlider;
	pSlider->GetWindowRect(&rectSlider);
	CPaneContainer* pContainer = m_pRootContainer->FindSubPaneContainer(pSlider, CPaneContainer::BC_FIND_BY_SLIDER);
	if (pContainer != NULL)
	{
		return pContainer->OnMoveInternalPaneDivider(nOffset, hdwp);
	}

	return 0;
}

void CPaneContainerManager::GetMinMaxOffset(CPaneDivider* pSlider, int& nMinOffset, int& nMaxOffset, int& nStep)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSlider);
	ASSERT_VALID(m_pRootContainer);

	nMinOffset = nMaxOffset = 0;
	nStep = -1;

	CRect rectContainer;
	CRect rectSlider;

	pSlider->GetWindowRect(rectSlider);

	if (pSlider->IsDefault())
	{
		CRect rectMainClientArea;
		ASSERT_VALID(pSlider->GetDockSiteFrameWnd());

		CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(pSlider->GetDockSiteFrameWnd());

		ASSERT_VALID(pDockManager);

		m_pRootContainer->GetWindowRect(rectContainer);

		CSize sizeMin;
		m_pRootContainer->GetMinSize(sizeMin);
		rectContainer.DeflateRect(sizeMin);

		rectMainClientArea = pDockManager->GetClientAreaBounds();
		pSlider->GetDockSiteFrameWnd()->ClientToScreen(rectMainClientArea);
		rectMainClientArea.DeflateRect(g_nSliderSpacingForMove, g_nSliderSpacingForMove);

		DWORD dwAlignment = pSlider->GetCurrentAlignment();
		if (dwAlignment & CBRS_ALIGN_LEFT)
		{
			nMinOffset = rectContainer.left - rectSlider.left + 1;
			nMaxOffset = rectMainClientArea.right - rectSlider.right - 1;
		}
		else if (dwAlignment & CBRS_ALIGN_TOP)
		{
			nMinOffset = rectContainer.top - rectSlider.top + 1;
			nMaxOffset = rectMainClientArea.bottom - rectSlider.bottom - 1;
		}
		else if (dwAlignment & CBRS_ALIGN_RIGHT)
		{
			nMinOffset = rectMainClientArea.left - rectSlider.left + 1;
			nMaxOffset = rectContainer.right - rectSlider.right - 1;
		}
		else if (dwAlignment & CBRS_ALIGN_BOTTOM)
		{
			nMinOffset = rectMainClientArea.top - rectSlider.top + 1;
			nMaxOffset = rectContainer.bottom - rectSlider.bottom - 1;
		}

		nStep = m_pRootContainer->GetResizeStep();
	}
	else
	{
		CPaneContainer* pContainer = m_pRootContainer->FindSubPaneContainer(pSlider, CPaneContainer::BC_FIND_BY_SLIDER);

		if (pContainer == NULL)
		{
			return;
		}

		pContainer->GetWindowRect(rectContainer);

		CSize sizeMinLeft;
		CSize sizeMinRight;
		pContainer->GetMinSizeLeft(sizeMinLeft);
		pContainer->GetMinSizeRight(sizeMinRight);

		if (pSlider->IsHorizontal())
		{
			nMinOffset = rectContainer.top - rectSlider.top + sizeMinLeft.cy + 1;
			nMaxOffset = rectContainer.bottom - rectSlider.bottom - sizeMinRight.cy - 1;
		}
		else
		{
			nMinOffset = rectContainer.left - rectSlider.left + sizeMinLeft.cx;
			nMaxOffset = rectContainer.right - rectSlider.right - sizeMinRight.cx - 1;
		}

		nStep = pContainer->GetResizeStep();
	}
}

CPaneDivider* CPaneContainerManager::CreatePaneDivider(CRect rectSlider, DWORD dwSliderStyle, int nSliderID)
{
	ASSERT_VALID(this);

	CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, CPaneDivider::m_pSliderRTC->CreateObject());
	ASSERT_VALID(pSlider);

	pSlider->Init();

	if (nSliderID == -1)
	{
		nSliderID = g_nSliderID;
		g_nSliderID ++;
	}

	if (nSliderID >= g_nSliderID)
	{
		g_nSliderID = nSliderID;
		g_nSliderID++;
	}

	for (POSITION pos = m_lstSliders.GetHeadPosition(); pos != NULL;)
	{
		CPaneDivider* pNextSlider = (CPaneDivider*) m_lstSliders.GetNext(pos);
		if (pNextSlider->GetDlgCtrlID() == nSliderID)
		{
			nSliderID = g_nSliderID;
			g_nSliderID++;
		}
	}

	if (!pSlider->CreateEx(0, dwSliderStyle, rectSlider, m_pDockSite, nSliderID, NULL))
	{
		TRACE0("CPaneContainerManager: Failed to create slider");
		delete pSlider;
		return NULL;
	}
	pSlider->ShowWindow(SW_SHOW);
	pSlider->SetPaneContainerManager(this);

	m_lstSliders.AddTail(pSlider);
	return pSlider;
}

void CPaneContainerManager::RemovePaneDivider(CPaneDivider* pSlider)
{
	POSITION pos = m_lstSliders.Find(pSlider);
	if (pos != NULL)
	{
		m_lstSliders.RemoveAt(pos);
		pSlider->SetPaneContainerManager(NULL);
	}

	if (m_pRootContainer != NULL)
	{
		CPaneContainer* pContainer = m_pRootContainer->FindSubPaneContainer(pSlider, CPaneContainer::BC_FIND_BY_SLIDER);
		if (pContainer != NULL)
		{
			pContainer->SetPaneDivider(NULL);
		}
	}
}

UINT CPaneContainerManager::FindPane(CPoint /*pt*/, CPane** /*ppBar*/, POSITION& /*posRet*/)
{
	return (UINT) HTERROR;
}

UINT CPaneContainerManager::FindPane(CRect /*rect*/, CPane** /*ppBar*/, POSITION& /*posRet*/)
{
	return (UINT) HTERROR;
}

void CPaneContainerManager::GetWindowRect(CRect& rect) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRootContainer);

	m_pRootContainer->GetWindowRect(rect);
}

void CPaneContainerManager::GetAvailableSpace(CRect& rect) const
{
	ASSERT_VALID(this);
	CRect rectUnited;
	rectUnited.SetRectEmpty();
	CRect rectBar;
	rectBar.SetRectEmpty();

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CWnd* pWnd = (CWnd*) m_lstControlBars.GetNext(pos);
		pWnd->GetWindowRect(rectBar);
		rectUnited.UnionRect(&rectUnited, &rectBar);
	}

	for (pos = m_lstSliders.GetHeadPosition(); pos != NULL;)
	{
		CWnd* pWnd = (CWnd*) m_lstSliders.GetNext(pos);
		pWnd->GetWindowRect(rectBar);
		rectUnited.UnionRect(&rectUnited, &rectBar);
	}

	GetWindowRect(rect);
	rect.SubtractRect(&rect, &rectUnited);
}

void CPaneContainerManager::CalcRects(CRect& rectOriginal, CRect& rectInserted, CRect& rectSlider,
									 DWORD& dwSliderStyle, DWORD dwAlignment, CSize /*sizeMinOriginal*/, CSize sizeMinInserted)
{
	if (rectInserted.Width() < sizeMinInserted.cx)
	{
		rectInserted.right = rectInserted.left + sizeMinInserted.cx;
	}

	if (rectInserted.Height() < sizeMinInserted.cy)
	{
		rectInserted.bottom = rectInserted.top + sizeMinInserted.cy;
	}

	// calculate the width/height (size) of both rectangles, slider's boundaries and orientation
	int nNewSize = 0;

	if (dwAlignment & CBRS_ORIENT_HORZ)
	{
		// align the rectangle of the bar to insert by the width of the sell
		rectSlider.left = rectInserted.left = rectOriginal.left;
		rectSlider.right = rectInserted.right = rectOriginal.right;

		if (rectInserted.Height() > rectOriginal.Height() / 2)
		{
			nNewSize = rectOriginal.Height() / 2;
		}
		else
		{
			nNewSize = rectInserted.Height();
		}

		dwSliderStyle = CPaneDivider::SS_HORZ;
	}
	else
	{
		// align the rectangle of the bar to insert by the height of the sell
		rectSlider.top = rectInserted.top = rectOriginal.top;
		rectSlider.bottom = rectInserted.bottom = rectOriginal.bottom;

		if (rectInserted.Width() > rectOriginal.Width() / 2)
		{
			nNewSize = rectOriginal.Width() / 2;
		}
		else
		{
			nNewSize = rectInserted.Width();
		}

		dwSliderStyle = CPaneDivider::SS_VERT;
	}

	// set rects for both rectangles and slider
	switch (dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_TOP:
		rectInserted.top = rectOriginal.top;
		rectInserted.bottom = rectInserted.top + nNewSize;
		rectOriginal.top = rectInserted.bottom + CPaneDivider::GetDefaultWidth();
		rectSlider.top = rectInserted.bottom;
		rectSlider.bottom = rectOriginal.top;
		break;

	case CBRS_ALIGN_BOTTOM:
		rectInserted.top = rectOriginal.bottom - nNewSize;
		rectInserted.bottom = rectOriginal.bottom;
		rectOriginal.bottom = rectInserted.top - CPaneDivider::GetDefaultWidth();
		rectSlider.top = rectOriginal.bottom;
		rectSlider.bottom = rectInserted.top;
		break;

	case CBRS_ALIGN_LEFT:
		rectInserted.left = rectOriginal.left;
		rectInserted.right = rectInserted.left + nNewSize;
		rectOriginal.left = rectInserted.right + CPaneDivider::GetDefaultWidth();
		rectSlider.left = rectInserted.right;
		rectSlider.right = rectOriginal.left;
		break;

	case CBRS_ALIGN_RIGHT:
		rectInserted.right = rectOriginal.right;
		rectInserted.left = rectInserted.right - nNewSize;
		rectOriginal.right = rectInserted.left - CPaneDivider::GetDefaultWidth();
		rectSlider.left = rectOriginal.right;
		rectSlider.right = rectInserted.left;
		break;
	}
}

BOOL CPaneContainerManager::AddPaneAndPaneContainer(CDockablePane* pBarOriginal, CPaneContainer* pContainerToInsert, DWORD dwAlignment)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBarOriginal);
	ASSERT_VALID(pContainerToInsert);
	ASSERT(dwAlignment & CBRS_ALIGN_ANY);

	if (m_pRootContainer == NULL)
	{
		TRACE0("The root container must be created first (call Create) \r\n");
		return FALSE;
	}

	CRect rectBarOriginal;
	CRect rectContainerToInsert;
	CRect rectSlider; rectSlider.SetRectEmpty();

	CSize sizeMinOriginal;
	pBarOriginal->GetMinSize(sizeMinOriginal);

	CSize sizeMinToInsert;
	pContainerToInsert->GetMinSize(sizeMinToInsert);

	pBarOriginal->GetWindowRect(rectBarOriginal);
	pContainerToInsert->GetWindowRect(rectContainerToInsert);

	DWORD dwSliderStyle = CPaneDivider::SS_HORZ;

	m_pDockSite->ScreenToClient(rectBarOriginal);
	m_pDockSite->ScreenToClient(rectContainerToInsert);
	m_pDockSite->ScreenToClient(rectSlider);

	BOOL bIsRTL = m_pDockSite->GetExStyle() & WS_EX_LAYOUTRTL;

	CalcRects(rectBarOriginal, rectContainerToInsert, rectSlider, dwSliderStyle, dwAlignment, sizeMinOriginal, sizeMinToInsert);

	pBarOriginal->MoveWindow(rectBarOriginal);

	HDWP hdwp = NULL;
	pContainerToInsert->Resize(rectContainerToInsert, hdwp);
	pContainerToInsert->Move(rectContainerToInsert.TopLeft());

	// it's not a default slider
	CPaneDivider* pSlider = CreatePaneDivider(rectSlider, dwSliderStyle);
	if (pSlider == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CPaneContainer* pContainer = NULL;
	if (m_pContainerRTC == NULL)
	{
		pContainer = new CPaneContainer();
	}
	else
	{
		pContainer = (CPaneContainer*) m_pContainerRTC->CreateObject();
	}

	pContainer->SetPaneContainerManager(this);
	pContainer->SetPaneDivider(pSlider);

	BOOL bRightNode = (dwAlignment & CBRS_ALIGN_BOTTOM) || (dwAlignment & CBRS_ALIGN_RIGHT);

	if (bIsRTL)
	{
		bRightNode = dwAlignment & CBRS_ALIGN_LEFT;
	}

	pContainer->SetPane(pBarOriginal, bRightNode);
	pContainer->SetPaneContainer(pContainerToInsert, !bRightNode);

	pSlider->BringWindowToTop();

	return m_pRootContainer->AddSubPaneContainer(pContainer, bRightNode);
}

BOOL CPaneContainerManager::AddPaneAndPaneDivider(CDockablePane* pBarOriginal,
												  CDockablePane* pBarToInsert, POSITION posNearestBar, DWORD dwAlignment)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBarOriginal);
	ASSERT_VALID(pBarToInsert);
	ASSERT_VALID(m_pRootContainer);
	ASSERT_KINDOF(CDockablePane, pBarOriginal);
	ASSERT_KINDOF(CDockablePane, pBarToInsert);
	ASSERT(dwAlignment & CBRS_ALIGN_ANY);

	if (m_pRootContainer == NULL)
	{
		TRACE0("The root container must be created first (call Create) \r\n");
		return FALSE;
	}

	// insert the new bar into the list of control bars accordig to its
	// hit test (side) relatively to the nearest bar
	switch (dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_TOP:
	case CBRS_ALIGN_LEFT:
		m_lstControlBars.InsertBefore(posNearestBar, pBarToInsert);
		break;

	case CBRS_ALIGN_BOTTOM:
	case CBRS_ALIGN_RIGHT:
		m_lstControlBars.InsertAfter(posNearestBar, pBarToInsert);
		break;

	default:
		ASSERT(FALSE);
		return FALSE;
	}

	CRect rectBarOriginal;
	CRect rectBarToInsert;
	CRect rectSlider;

	CSize sizeMinOriginal;
	pBarOriginal->GetMinSize(sizeMinOriginal);

	CSize sizeMinToInsert;
	pBarToInsert->GetMinSize(sizeMinToInsert);

	pBarOriginal->GetWindowRect(rectBarOriginal);
	pBarToInsert->GetWindowRect(rectBarToInsert);

	if (rectBarToInsert.Width() < sizeMinToInsert.cx)
	{
		rectBarToInsert.right = rectBarToInsert.left + sizeMinToInsert.cx;
	}

	if (rectBarToInsert.Height() < sizeMinToInsert.cy)
	{
		rectBarToInsert.bottom = rectBarToInsert.top + sizeMinToInsert.cy;
	}

	// calculate the width/height (size) of both rectangles, slider's boundaries and orientation
	DWORD dwSliderStyle = 0;
	int nNewSize = 0;

	if (dwAlignment & CBRS_ORIENT_HORZ)
	{
		// align the rectangle of the bar to insert by the width of the sell
		rectSlider.left = rectBarToInsert.left = rectBarOriginal.left;
		rectSlider.right = rectBarToInsert.right = rectBarOriginal.right;

		if (rectBarToInsert.Height() > rectBarOriginal.Height() / 2) //- sizeMinOriginal.cy * 2- CPaneDivider::GetDefaultWidth())
		{
			nNewSize = rectBarOriginal.Height() / 2; //  - sizeMinOriginal.cy * 4 - CPaneDivider::GetDefaultWidth();
		}
		else
		{
			nNewSize = rectBarToInsert.Height();
		}
		dwSliderStyle = CPaneDivider::SS_HORZ;
	}
	else
	{
		// align the rectangle of the bar to insert by the height of the sell
		rectSlider.top = rectBarToInsert.top = rectBarOriginal.top;
		rectSlider.bottom = rectBarToInsert.bottom = rectBarOriginal.bottom;

		if (rectBarToInsert.Width() > rectBarOriginal.Width() / 2) //- sizeMinOriginal.cx * 2 - CPaneDivider::GetDefaultWidth())
		{
			nNewSize = rectBarOriginal.Width() / 2; //- sizeMinOriginal.cx * 4;
		}
		else
		{
			nNewSize = rectBarToInsert.Width();
		}
		dwSliderStyle = CPaneDivider::SS_VERT;
	}

	BOOL bRightNode = FALSE;
	CDockablePane* pLeftBar = NULL;
	CDockablePane* pRightBar = NULL;

	m_pDockSite->ScreenToClient(rectBarOriginal);
	m_pDockSite->ScreenToClient(rectBarToInsert);
	m_pDockSite->ScreenToClient(rectSlider);

	BOOL bIsRTL = m_pDockSite->GetExStyle() & WS_EX_LAYOUTRTL;

	// set rects for both rectangles and slider
	switch (dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_TOP:
		rectBarToInsert.top = rectBarOriginal.top;
		rectBarToInsert.bottom = rectBarToInsert.top + nNewSize;
		rectBarOriginal.top = rectBarToInsert.bottom + CPaneDivider::GetDefaultWidth();
		rectSlider.top = rectBarToInsert.bottom;
		rectSlider.bottom = rectBarOriginal.top;
		pLeftBar = pBarToInsert;
		pRightBar = pBarOriginal;
		break;

	case CBRS_ALIGN_BOTTOM:
		rectBarToInsert.top = rectBarOriginal.bottom - nNewSize;
		rectBarToInsert.bottom = rectBarOriginal.bottom;
		rectBarOriginal.bottom = rectBarToInsert.top - CPaneDivider::GetDefaultWidth();
		rectSlider.top = rectBarOriginal.bottom;
		rectSlider.bottom = rectBarToInsert.top;
		dwSliderStyle = CPaneDivider::SS_HORZ;
		pLeftBar = pBarOriginal;
		pRightBar = pBarToInsert;
		bRightNode = TRUE;
		break;

	case CBRS_ALIGN_LEFT:
		if (bIsRTL)
		{
			rectBarToInsert.right = rectBarOriginal.right;
			rectBarToInsert.left = rectBarToInsert.right - nNewSize;
			rectBarOriginal.right = rectBarToInsert.left - CPaneDivider::GetDefaultWidth();
			rectSlider.left = rectBarOriginal.right;
			rectSlider.right = rectBarToInsert.left;
			pLeftBar = pBarOriginal;
			pRightBar = pBarToInsert;
			bRightNode = TRUE;
		}
		else
		{
			rectBarToInsert.left = rectBarOriginal.left;
			rectBarToInsert.right = rectBarToInsert.left + nNewSize;
			rectBarOriginal.left = rectBarToInsert.right + CPaneDivider::GetDefaultWidth();
			rectSlider.left = rectBarToInsert.right;
			rectSlider.right = rectBarOriginal.left;
			pLeftBar = pBarToInsert;
			pRightBar = pBarOriginal;
		}
		break;

	case CBRS_ALIGN_RIGHT:
		if (bIsRTL)
		{
			rectBarToInsert.left = rectBarOriginal.left;
			rectBarToInsert.right = rectBarToInsert.left + nNewSize;
			rectBarOriginal.left = rectBarToInsert.right + CPaneDivider::GetDefaultWidth();
			rectSlider.left = rectBarToInsert.right;
			rectSlider.right = rectBarOriginal.left;
			pLeftBar = pBarToInsert;
			pRightBar = pBarOriginal;

		}
		else
		{
			rectBarToInsert.right = rectBarOriginal.right;
			rectBarToInsert.left = rectBarToInsert.right - nNewSize;
			rectBarOriginal.right = rectBarToInsert.left - CPaneDivider::GetDefaultWidth();
			rectSlider.left = rectBarOriginal.right;
			rectSlider.right = rectBarToInsert.left;
			pLeftBar = pBarOriginal;
			pRightBar = pBarToInsert;
			bRightNode = TRUE;
		}
		break;
	}

	pBarOriginal->SetWindowPos(NULL, rectBarOriginal.left, rectBarOriginal.top, rectBarOriginal.Width(), rectBarOriginal.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	pBarToInsert->SetWindowPos(NULL, rectBarToInsert.left, rectBarToInsert.top, rectBarToInsert.Width(), rectBarToInsert.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	// it's not a default slider
	CPaneDivider* pSlider = CreatePaneDivider(rectSlider, dwSliderStyle);
	if (pSlider == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CPaneContainer* pContainer = NULL;
	if (m_pContainerRTC == NULL)
	{
		pContainer = new CPaneContainer(this, pLeftBar, pRightBar, pSlider);
	}
	else
	{
		pContainer = (CPaneContainer*) m_pContainerRTC->CreateObject();
		pContainer->SetPaneContainerManager(this);
		pContainer->SetPane(pLeftBar, TRUE);
		pContainer->SetPane(pRightBar, FALSE);
		pContainer->SetPaneDivider(pSlider);
	}

	return m_pRootContainer->AddSubPaneContainer(pContainer, bRightNode);
}

void CPaneContainerManager::ResizePaneContainers(UINT nSide, BOOL bExpand, int nOffset, HDWP& hdwp)
{
	ASSERT_VALID(this);

	if (m_pRootContainer != NULL)
	{
		ASSERT_VALID(m_pRootContainer);

		bool bStretchHorz = (nSide == WMSZ_RIGHT || nSide  == WMSZ_LEFT);
		bool bLeftBar = true;
		nOffset *= bExpand ? 1 : (-1);

		m_pRootContainer->StretchPaneContainer(nOffset, bStretchHorz, bLeftBar, TRUE, hdwp);
	}
}

void CPaneContainerManager::ResizePaneContainers(CRect rect, HDWP& hdwp)
{
	ASSERT_VALID(this);

	if (m_pRootContainer != NULL)
	{
		ASSERT_VALID(m_pRootContainer);
		m_pRootContainer->Resize(rect, hdwp);
	}
}

BOOL CPaneContainerManager::ReplacePane(CDockablePane* pBarOld, CDockablePane* pBarNew)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBarOld);
	ASSERT_VALID(pBarNew);
	ASSERT_KINDOF(CDockablePane, pBarOld);
	ASSERT_KINDOF(CDockablePane, pBarNew);

	POSITION pos = m_lstControlBars.Find(pBarOld);

	if (pos != NULL)
	{
		BOOL bLeftBar = FALSE;
		CPaneContainer* pContainer = FindPaneContainer(pBarOld, bLeftBar);

		if (pContainer != NULL)
		{
			pContainer->SetPane(pBarNew, bLeftBar);

			m_lstControlBars.InsertAfter(pos, pBarNew);
			m_lstControlBars.RemoveAt(pos);
		}
	}
	else
	{
		m_lstControlBars.AddTail(pBarNew);
	}

	return TRUE;
}

void CPaneContainerManager::SetResizeMode(BOOL bResize)
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pBar = (CDockablePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBar);

		pBar->SetResizeMode(bResize);
	}
}

void CPaneContainerManager::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		int nSliderCount = (int) m_lstSliders.GetCount();
		m_pRootContainer->ReleaseEmptyPaneContainer();

		nSliderCount = (int) m_lstSliders.GetCount();

		m_pRootContainer->Serialize(ar);

		ar << (int) m_lstControlBars.GetCount();
		for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CWnd* pNextBar = (CWnd*) m_lstControlBars.GetNext(pos);
			ASSERT_VALID(pNextBar);

			int nBarID = pNextBar->GetDlgCtrlID();
			if (nBarID != -1)
			{
				ar << nBarID;
			}
			else
			{
				// this is tab control bar - identify it by its first tabbed bar
				CBaseTabbedPane* pTabBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pNextBar);
				ASSERT_VALID(pTabBar);
				CWnd* pWnd  = pTabBar->FindBarByTabNumber(0);

				if (pWnd != NULL)
				{
					int nTabbedBarID = pWnd->GetDlgCtrlID();

					ASSERT(nTabbedBarID != -1);
					ar << nBarID;
					ar << nTabbedBarID;
				}
			}
		}
	}
	else
	{
		m_pRootContainer->Serialize(ar);

		// rewrite to conform with miniframe (m_pDefaultSlider is null there) !!!
		CDockingManager* pDockManager = NULL;

		if (m_pDefaultSlider != NULL)
		{
			pDockManager = afxGlobalUtils.GetDockingManager(m_pDefaultSlider->GetDockSiteFrameWnd());
		}
		else if (m_pDockSite->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
		{
			pDockManager = afxGlobalUtils.GetDockingManager(m_pDockSite->GetParent());
		}

		if (pDockManager == NULL)
		{
			TRACE0("Unexpected NULL pointer to dock manager. Serialization failed.\n");
			throw new CArchiveException;
			return;
		}

		int nCount = 0;

		// load control bar id's
		ar >> nCount;

		int nControlBarID = 0;
		for (int i = 0; i < nCount; i++)
		{
			ar >> nControlBarID;

			// -1 means tabbed control bar, these bars are stored and loaded by containers
			if (nControlBarID != -1)
			{
				CDockablePane* pBar =
					DYNAMIC_DOWNCAST(CDockablePane, pDockManager->FindPaneByID(nControlBarID, TRUE));
				if (pBar != NULL)
				{
					ASSERT_VALID(pBar);

					m_lstControlBars.AddTail(pBar);

					m_pRootContainer->SetUpByID(nControlBarID, pBar);
				}
			}
			else
			{
				// load the first tabbed bar id
				ar >> nControlBarID;

				CDockablePane* pBar = m_pRootContainer->FindTabbedPane((UINT) nControlBarID);

				if (pBar != NULL)
				{
					m_lstControlBars.AddTail(pBar);
				}
			}
		}
	}
}

CDockablePane* CPaneContainerManager::FindTabbedPane(UINT nID)
{
	ASSERT_VALID(this);

	if (m_pRootContainer != NULL)
	{
		return m_pRootContainer->FindTabbedPane(nID);
	}

	return NULL;
}

CPaneContainer* CPaneContainerManager::FindPaneContainer(CDockablePane* pBar, BOOL& bLeftBar)
{
	ASSERT_VALID(this);

	if (m_pRootContainer != NULL)
	{
		bLeftBar = TRUE;
		CPaneContainer::BC_FIND_CRITERIA barType = CPaneContainer::BC_FIND_BY_LEFT_BAR;
		CPaneContainer* pContainer = m_pRootContainer->FindSubPaneContainer(pBar, barType);
		if (pContainer == NULL)
		{
			barType = CPaneContainer::BC_FIND_BY_RIGHT_BAR;
			pContainer = m_pRootContainer->FindSubPaneContainer(pBar, barType);
			bLeftBar = FALSE;
		}
		return pContainer;
	}

	return NULL;
}

//-----------------------------------------------------------------------------------//
// Look for control bar that contains point according to sensitivity: if we're looking
// for the exact bar, the point must be in the area between bars' bounds and deflated bars'
// bounds; otherwise the point must be inside inflated bars' window rectangle
//-----------------------------------------------------------------------------------//
CDockablePane* CPaneContainerManager::PaneFromPoint(CPoint point,
															  int nSensitivity, BOOL bExactBar, BOOL& bIsTabArea, BOOL& bCaption)
{
	ASSERT_VALID(this);

	bIsTabArea = FALSE;

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, m_lstControlBars.GetNext(pos));

		CRect rectWnd;
		pBar->GetWindowRect(rectWnd);

		CRect rectTabAreaTop;
		CRect rectTabAreaBottom;
		pBar->GetTabArea(rectTabAreaTop, rectTabAreaBottom);

		if (rectTabAreaTop.PtInRect(point) || rectTabAreaBottom.PtInRect(point))
		{
			bIsTabArea = TRUE;
			return pBar;
		}

		if (pBar->HitTest(point, TRUE) == HTCAPTION)
		{
			bCaption = TRUE;
			return pBar;
		}

		int nCaptionHeight = pBar->GetCaptionHeight();
		rectWnd.top += nCaptionHeight;
		rectWnd.bottom -= rectTabAreaBottom.Height();

		if (rectWnd.PtInRect(point))
		{
			CWnd* pWnd = pBar->GetParent();
			ASSERT_VALID(pWnd);

			CDockingManager* pDockManager = NULL;
			CSmartDockingManager* pSDManager = NULL;

			if ((pDockManager = afxGlobalUtils.GetDockingManager(pWnd)) != NULL && (pSDManager = pDockManager->GetSmartDockingManagerPermanent()) != NULL
				&& pSDManager->IsStarted())
			{
				CSmartDockingStandaloneGuide::SDMarkerPlace m_nHiliteSideNo = pSDManager->GetHighlightedGuideNo();
				if (m_nHiliteSideNo >= CSmartDockingStandaloneGuide::sdCLEFT && m_nHiliteSideNo <= CSmartDockingStandaloneGuide::sdCMIDDLE) // if central group is selected
				{
					bCaption = (m_nHiliteSideNo == CSmartDockingStandaloneGuide::sdCMIDDLE);
				}

				return pBar;
			}

			rectWnd.InflateRect(-nSensitivity, -nSensitivity);
			if (!rectWnd.PtInRect(point) || nSensitivity == 0)
			{
				return pBar;
			}
		}
	}

	if (!bExactBar)
	{
		for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, m_lstControlBars.GetNext(pos));

			CRect rectWnd;
			pBar->GetWindowRect(rectWnd);

			rectWnd.InflateRect(nSensitivity, nSensitivity);
			if (rectWnd.PtInRect(point))
			{
				return pBar;
			}
		}
	}
	return NULL;
}

void CPaneContainerManager::GetMinSize(CSize& size)
{
	ASSERT_VALID(this);
	size.cx = size.cy = 0;

	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->GetMinSize(size);
	}
}

BOOL CPaneContainerManager::IsRootPaneContainerVisible() const
{
	ASSERT_VALID(this);
	if (m_pRootContainer != NULL)
	{
		return m_pRootContainer->IsVisible();
	}

	return FALSE;
}

int CPaneContainerManager::GetVisiblePaneCount() const
{
	ASSERT_VALID(this);
	int nCount = 0;

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pWnd);

		if (pWnd->IsPaneVisible())
		{
			nCount++;
		}
	}

	return nCount;
}

CWnd* CPaneContainerManager::GetFirstVisiblePane() const
{
	ASSERT_VALID(this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pWnd);

		if (pWnd->IsPaneVisible())
		{
			return pWnd;
		}
	}
	return NULL;
}

void CPaneContainerManager::EnableGrippers(BOOL bEnable)
{
	ASSERT_VALID(this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pWnd = DYNAMIC_DOWNCAST(CDockablePane, m_lstControlBars.GetNext(pos));
		if (pWnd != NULL)
		{
			pWnd->EnableGripper(bEnable);
		}
	}
}

void CPaneContainerManager::HideAll()
{
	ASSERT_VALID(this);

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_lstControlBars.GetNext(pos));

		if (pWnd != NULL)
		{
			pWnd->ShowWindow(SW_HIDE);
		}
	}

	for (pos = m_lstSliders.GetHeadPosition(); pos != NULL;)
	{
		CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_lstSliders.GetNext(pos));

		if (pWnd != NULL)
		{
			pWnd->ShowWindow(SW_HIDE);
		}
	}
}

BOOL CPaneContainerManager::DoesContainFloatingPane()
{
	ASSERT_VALID(this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
		if (pWnd->CanFloat())
		{
			return TRUE;
		}
	}

	return FALSE;
}

int	CPaneContainerManager::GetNodeCount() const
{
	ASSERT_VALID(this);

	if (m_pRootContainer == NULL)
	{
		return 0;
	}

	return m_pRootContainer->GetNodeCount();
}

BOOL CPaneContainerManager::IsEmpty() const
{
	return m_lstControlBars.IsEmpty();
}

int CPaneContainerManager::GetTotalRefCount() const
{
	if (m_pRootContainer == NULL)
	{
		return 0;
	}
	return m_pRootContainer->GetTotalReferenceCount();
}

void CPaneContainerManager::SetDefaultPaneDividerForPanes(CPaneDivider* /*pSlider*/)
{
	ASSERT_VALID(this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pWnd = DYNAMIC_DOWNCAST(CDockablePane, m_lstControlBars.GetNext(pos));
		if (pWnd != NULL)
		{
			pWnd->SetDefaultPaneDivider(NULL);
		}
	}
}

void CPaneContainerManager::AddPanesToList(CObList* plstControlBars, CObList* plstSliders)
{
	ASSERT_VALID(this);
	if (plstControlBars != NULL)
	{
		for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_lstControlBars.GetNext(pos));
			ASSERT_VALID(pWnd);

			if (pWnd->GetStyle() & WS_VISIBLE)
			{
				plstControlBars->AddTail(pWnd);
			}
		}
	}

	if (plstSliders != NULL)
	{
		for (POSITION pos = m_lstSliders.GetHeadPosition(); pos != NULL;)
		{
			CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_lstSliders.GetNext(pos));
			ASSERT_VALID(pWnd);

			if (pWnd->GetStyle() & WS_VISIBLE)
			{
				plstSliders->AddTail(pWnd);
			}
		}
	}
}

void CPaneContainerManager::RemoveAllPanesAndPaneDividers()
{
	ASSERT_VALID(this);
	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pWnd);

		if (pWnd->IsPaneVisible())
		{
			m_lstControlBars.RemoveAt(posSave);
		}
	}

	for (pos = m_lstSliders.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pWnd);

		if (pWnd->IsPaneVisible())
		{
			m_lstSliders.RemoveAt(posSave);
		}
	}
}

void CPaneContainerManager::AddPaneToList(CDockablePane* pControlBarToAdd)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBarToAdd);

	m_lstControlBars.AddTail(pControlBarToAdd);
}

BOOL CPaneContainerManager::DoesAllowDynInsertBefore() const
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pWnd);

		if (pWnd->DoesAllowDynInsertBefore())
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CPaneContainerManager::NotifyPaneDivider()
{
	if (m_pDefaultSlider != NULL)
	{
		m_pDefaultSlider->NotifyAboutRelease();
	}
}

BOOL CPaneContainerManager::CheckForMiniFrameAndCaption(CPoint point, CDockablePane** ppTargetControlBar)
{
	CMultiPaneFrameWnd* pMiniFrameWnd = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, m_pDockSite);

	*ppTargetControlBar = NULL;

	if (pMiniFrameWnd == NULL)
	{
		return FALSE;
	}

	if (GetVisiblePaneCount() > 1)
	{
		return FALSE;
	}

	CRect rectCaption;
	pMiniFrameWnd->GetCaptionRect(rectCaption);
	pMiniFrameWnd->ScreenToClient(&point);

	CRect rectBorderSize;
	pMiniFrameWnd->CalcBorderSize(rectBorderSize);

	point.Offset(rectBorderSize.left, rectBorderSize.top + pMiniFrameWnd->GetCaptionHeight());

	if (rectCaption.PtInRect(point))
	{
		*ppTargetControlBar = DYNAMIC_DOWNCAST(CDockablePane, GetFirstVisiblePane());
	}

	return (*ppTargetControlBar != NULL);
}

void CPaneContainerManager::RemoveNonValidPanes()
{
	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->RemoveNonValidPanes();
	}
}

BOOL CPaneContainerManager::CheckAndRemoveNonValidPane(CWnd* pWnd)
{
	if (pWnd != NULL)
	{
		UINT nControlID = pWnd->GetDlgCtrlID();
		if (IsWindow(pWnd->GetSafeHwnd()) && nControlID != -1)
		{
			return TRUE;
		}

		CBaseTabbedPane* pBaseTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pWnd);

		if (pBaseTabbedBar != NULL && pBaseTabbedBar->GetTabsNum() > 0)
		{
			return TRUE;
		}
	}

	POSITION pos = m_lstControlBars.Find(pWnd);
	if (pos != NULL)
	{
		m_lstControlBars.RemoveAt(pos);
	}

	return FALSE;
}

BOOL CPaneContainerManager::CanBeAttached() const
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pWnd);

		if (!pWnd->CanBeAttached())
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CPaneContainerManager::ReleaseEmptyPaneContainers()
{
	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->ReleaseEmptyPaneContainer();
	}
}



