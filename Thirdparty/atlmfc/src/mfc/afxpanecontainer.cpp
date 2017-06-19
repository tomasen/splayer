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
#include "afxpanecontainer.h"

#include "afxdockablepane.h"
#include "afxbasetabbedpane.h"
#include "afxpanedivider.h"
#include "afxpaneframewnd.h"

#include "afxpanecontainermanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPaneContainer, CObject)

BOOL CPaneContainer::m_bMaintainPercentage = FALSE;
BOOL CPaneContainer::m_bRetainInternalSliderPosition = FALSE;

CPaneContainerGC gc;

// Construction/Destruction
CPaneContainer::CPaneContainer(CPaneContainerManager* pManager, CDockablePane* pLeftBar, CDockablePane* pRightBar, CPaneDivider* pSlider) :
m_pContainerManager(pManager), m_pBarLeftTop(pLeftBar), m_pBarRightBottom(pRightBar), m_pSlider(pSlider), m_pLeftContainer(NULL),
m_pRightContainer(NULL), m_pParentContainer(NULL), m_dwRefCount(0)
{
	m_nSavedLeftBarID = (UINT)-1;
	m_nSavedRightBarID = (UINT)-1;
	m_nSavedSliderID = (UINT)-1;
	m_bSavedSliderVisibility = FALSE;
	m_rectSavedSliderRect.SetRectEmpty();

	m_dwRecentSliderStyle = 0;
	m_rectRecentSlider.SetRectEmpty();

	m_nRecentPercent = 50;
	m_bIsRecentSliderHorz = FALSE;
	m_bDisposed = FALSE;
}

CPaneContainer::~CPaneContainer()
{
	CleanUp();
	m_bDisposed = TRUE;
}

void CPaneContainer::CleanUp()
{
	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->CleanUp();
		delete m_pLeftContainer;
		m_pLeftContainer = NULL;
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->CleanUp();
		delete m_pRightContainer;
		m_pRightContainer = NULL;
	}

	if (m_pSlider != NULL && !m_pSlider->IsDefault() && m_pSlider->GetSafeHwnd() != NULL)
	{
		m_pSlider->DestroyWindow();
		m_pSlider = NULL;
	}
}

int CPaneContainer::GetResizeStep() const
{
	ASSERT_VALID(this);

	int nStep = -1;

	if (m_pBarLeftTop != NULL)
	{
		nStep = m_pBarLeftTop->GetResizeStep();
	}

	if (m_pBarRightBottom != NULL)
	{
		nStep = max(nStep, m_pBarRightBottom->GetResizeStep());
	}

	if (m_pLeftContainer != NULL)
	{
		nStep = m_pLeftContainer->GetResizeStep();
	}

	if (m_pRightContainer != NULL)
	{
		nStep = max(nStep, m_pRightContainer->GetResizeStep());
	}

	return nStep;
}

void CPaneContainer::GetWindowRect(CRect& rect, BOOL bIgnoreVisibility) const
{
	ASSERT_VALID(this);
	CRect rectLeft;
	CRect rectRight;
	CRect rectContainer;

	rect.SetRectEmpty();
	rectLeft.SetRectEmpty();
	rectRight.SetRectEmpty();

	// VCheck
	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode();

	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsPaneVisible() || bIgnoreVisibility || bAutoHideMode))
	{
		m_pBarLeftTop->GetWindowRect(rectLeft);
		if (rectLeft.IsRectEmpty())
		{
			CSize sz;
			m_pBarLeftTop->GetMinSize(sz);

			if (rectLeft.Width() == 0)
			{
				rectLeft.InflateRect(0, 0, sz.cx, 0);
			}

			if (rectLeft.Height() == 0)
			{
				rectLeft.InflateRect(0, 0, 0, sz.cy);
			}
		}
	}

	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsPaneVisible() || bIgnoreVisibility || bAutoHideMode))
	{
		m_pBarRightBottom->GetWindowRect(rectRight);
		if (rectRight.IsRectEmpty())
		{
			CSize sz;
			m_pBarRightBottom->GetMinSize(sz);

			if (rectRight.Width() == 0)
			{
				rectRight.InflateRect(0, 0, sz.cx, 0);
			}

			if (rectRight.Height() == 0)
			{
				rectRight.InflateRect(0, 0, 0, sz.cy);
			}
		}
	}

	rect.UnionRect(rectLeft, rectRight);

	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsVisible() || bIgnoreVisibility || bAutoHideMode))
	{
		m_pLeftContainer->GetWindowRect(rectContainer);
		rect.UnionRect(rect, rectContainer);
	}

	if (m_pRightContainer != NULL && (m_pRightContainer->IsVisible() || bIgnoreVisibility || bAutoHideMode))
	{
		m_pRightContainer->GetWindowRect(rectContainer);
		rect.UnionRect(rect, rectContainer);
	}
}

void CPaneContainer::GetMinSize(CSize& size) const
{
	ASSERT_VALID(this);
	ENSURE(m_pContainerManager != NULL);

	CSize minSizeLeft(0, 0);
	CSize minSizeRight(0, 0);
	size.cx = size.cy = 0;

	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode();

	// VCheck
	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsPaneVisible() || bAutoHideMode))
	{
		m_pBarLeftTop->GetMinSize(minSizeLeft);
	}

	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsPaneVisible() || bAutoHideMode))
	{
		m_pBarRightBottom->GetMinSize(minSizeRight);
	}

	CSize sizeLeftContainer(0, 0);
	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsVisible() || bAutoHideMode))
	{
		m_pLeftContainer->GetMinSize(sizeLeftContainer);
	}

	CSize sizeRightContainer(0, 0);
	if (m_pRightContainer != NULL && (m_pRightContainer->IsVisible() || bAutoHideMode))
	{
		m_pRightContainer->GetMinSize(sizeRightContainer);
	}

	if (m_pSlider != NULL && (m_pSlider->IsPaneVisible() || bAutoHideMode))
	{
		if (IsPaneDividerHorz())
		{
			size.cx = max(minSizeLeft.cx, minSizeRight.cx);
			size.cx = max(sizeLeftContainer.cx, size.cx);
			size.cx = max(sizeRightContainer.cx, size.cx);
			size.cy = minSizeLeft.cy + minSizeRight.cy + sizeLeftContainer.cy + sizeRightContainer.cy + m_pSlider->GetWidth();
		}
		else
		{
			size.cy = max(minSizeLeft.cy, minSizeRight.cy);
			size.cy = max(sizeLeftContainer.cy, size.cy);
			size.cy = max(sizeRightContainer.cy, size.cy);
			size.cx = minSizeLeft.cx + minSizeRight.cx + sizeLeftContainer.cx + sizeRightContainer.cx + m_pSlider->GetWidth();
		}
	}
	else
	{
		size.cx = max(minSizeLeft.cx, minSizeRight.cx);
		size.cy = max(minSizeLeft.cy, minSizeRight.cy);
		if (m_pLeftContainer != NULL && m_pLeftContainer->IsVisible())
		{
			size = sizeLeftContainer;
		}
		if (m_pRightContainer != NULL && m_pRightContainer->IsVisible())
		{
			size = sizeRightContainer;
		}
	}
}

void CPaneContainer::GetMinSizeLeft(CSize& size) const
{
	ASSERT_VALID(this);

	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode();

	// VCheck
	CSize minSizeLeft(0, 0);
	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsPaneVisible() || bAutoHideMode))
	{
		m_pBarLeftTop->GetMinSize(minSizeLeft);
	}

	CSize sizeLeftContainer(0, 0);
	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsVisible() || bAutoHideMode))
	{
		m_pLeftContainer->GetMinSize(sizeLeftContainer);
	}

	size.cx = max(minSizeLeft.cx, sizeLeftContainer.cx);
	size.cy = max(minSizeLeft.cy, sizeLeftContainer.cy);

}

void CPaneContainer::GetMinSizeRight(CSize& size) const
{
	ASSERT_VALID(this);
	// VCheck
	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode();
	CSize minSizeRight(0, 0);
	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsPaneVisible() || bAutoHideMode))
	{
		m_pBarRightBottom->GetMinSize(minSizeRight);
	}

	CSize sizeRightContainer(0, 0);
	if (m_pRightContainer != NULL && (m_pRightContainer->IsVisible() || bAutoHideMode))
	{
		m_pRightContainer->GetMinSize(sizeRightContainer);
	}

	size.cx = max(minSizeRight.cx, sizeRightContainer.cx);
	size.cy = max(minSizeRight.cy, sizeRightContainer.cy);
}

CDockablePane* CPaneContainer::AddPane(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_KINDOF(CDockablePane, pBar);

	CWnd* pDockSite = m_pContainerManager->GetDockSiteFrameWnd();
	ASSERT_VALID(pDockSite);

	BOOL bAddToMiniFrame = pDockSite->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd));

	CRect rectNew = pBar->m_recentDockInfo.GetRecentDockedRect(!bAddToMiniFrame);

	CRect rectContainer;
	rectContainer.SetRectEmpty();
	GetWindowRect(rectContainer);

	pDockSite->ScreenToClient(rectContainer);

	// if the container was empty we'll need to expand its parent container
	BOOL bExpandParentContainer = IsEmpty();
	// find first non-empty parent container
	CPaneContainer* pNextContainer = m_pParentContainer;
	while (pNextContainer != NULL)
	{
		if (!pNextContainer->IsEmpty())
		{
			break;
		}
		pNextContainer = pNextContainer->GetParentPaneContainer();
	}

	CRect rectParentContainer;
	rectParentContainer.SetRectEmpty();

	if (pNextContainer != NULL)
	{
		pNextContainer->GetWindowRect(rectParentContainer);
		pDockSite->ScreenToClient(rectParentContainer);
	}

	int nNewWidth  = rectContainer.Width() > 0 ? rectContainer.Width() : rectParentContainer.Width();
	int nNewHeight = rectContainer.Height() > 0 ? rectContainer.Height() : rectParentContainer.Height();

	if (nNewWidth == 0)
	{
		nNewWidth = rectNew.Width();
	}

	if (nNewHeight == 0)
	{
		nNewHeight = rectNew.Height();
	}

	if (!rectContainer.IsRectEmpty())
	{
		rectNew.left = rectContainer.left;
		rectNew.top = rectContainer.top;
	}
	else if (!rectParentContainer.IsRectEmpty())
	{
		rectNew.left = rectParentContainer.left;
		rectNew.top = rectParentContainer.top;
	}

	CSize sizeMin;
	pBar->GetMinSize(sizeMin);

	if (nNewWidth < sizeMin.cx)
	{
		nNewWidth = sizeMin.cx;
	}

	if (nNewHeight < sizeMin.cy)
	{
		nNewHeight = sizeMin.cy;
	}

	int nRecentPercent = pBar->m_recentDockInfo.GetRecentDockedPercent(!bAddToMiniFrame);

	if (nRecentPercent == 100 || nRecentPercent == 0)
	{
		nRecentPercent = 50;
	}

	if (!IsEmpty() && m_pSlider != NULL)
	{
		if (IsPaneDividerHorz())
		{
			if (pBar->m_recentDockInfo.IsRecentLeftPane(!bAddToMiniFrame))
			{
				nNewHeight = rectContainer.Height() * nRecentPercent / 100;
				rectNew.top = rectContainer.top;
			}
			else
			{
				nNewHeight = rectContainer.Height() - (rectContainer.Height() * (100 - nRecentPercent) / 100) - m_pSlider->GetWidth();
				rectNew.top = rectContainer.bottom - nNewHeight;
			}
		}
		else
		{
			if (pBar->m_recentDockInfo.IsRecentLeftPane(!bAddToMiniFrame))
			{
				nNewWidth = rectContainer.Width() * nRecentPercent / 100;
				rectNew.left = rectContainer.left;
			}
			else
			{
				nNewWidth = rectContainer.Width() - (rectContainer.Width() * (100 - nRecentPercent) / 100) - m_pSlider->GetWidth();
				rectNew.left = rectContainer.right - nNewWidth;
			}
		}
	}

	rectNew.bottom = rectNew.top + nNewHeight;
	rectNew.right = rectNew.left + nNewWidth;

	BOOL bShowSlider = FALSE;

	HDWP hdwp = BeginDeferWindowPos(10);

	hdwp = pBar->MoveWindow(rectNew, FALSE, hdwp);

	CRect rectSlider = rectNew;
	CRect rectSecondBar;

	BOOL bIsRecentLeftBar = pBar->m_recentDockInfo.IsRecentLeftPane(!bAddToMiniFrame);

	if (bIsRecentLeftBar && m_pLeftContainer != NULL)
	{
		return m_pLeftContainer->AddPane(pBar);
	}

	if (!bIsRecentLeftBar && m_pRightContainer != NULL)
	{
		return m_pRightContainer->AddPane(pBar);
	}

	if (bIsRecentLeftBar)
	{
		ENSURE(m_pLeftContainer == NULL);

		if (m_pBarLeftTop != NULL)
		{
			CDockablePane* pTabbedControlBar = NULL;
			pBar->AttachToTabWnd(m_pBarLeftTop, DM_DBL_CLICK, TRUE, &pTabbedControlBar);
			if (pTabbedControlBar != NULL && m_pBarLeftTop != NULL)
			{
				m_pContainerManager->ReplacePane(m_pBarLeftTop, pTabbedControlBar);
			}
			else if (pTabbedControlBar != NULL)
			{
				m_pContainerManager->AddPaneToList(pTabbedControlBar);
				m_pBarLeftTop = pTabbedControlBar;
			}
			return pTabbedControlBar;
		}

		m_pBarLeftTop = pBar;

		bShowSlider = (m_pBarRightBottom != NULL) || (m_pRightContainer != NULL);

		if (m_pBarRightBottom != NULL)
		{
			m_pBarRightBottom->GetWindowRect(rectSecondBar);
		}
		else if (m_pRightContainer != NULL)
		{
			m_pRightContainer->GetWindowRect(rectSecondBar);
		}

		pDockSite->ScreenToClient(rectSecondBar);

		if (m_pSlider != NULL)
		{
			if (IsPaneDividerHorz())
			{
				rectSlider.top = rectNew.bottom;
				rectSlider.bottom = rectSlider.top + m_pSlider->GetWidth();
				rectSecondBar.top = rectSlider.bottom;
			}
			else
			{
				rectSlider.left = rectNew.right;
				rectSlider.right = rectSlider.left + m_pSlider->GetWidth();
				rectSecondBar.left  = rectSlider.right;
			}
		}

		if (m_pBarRightBottom != NULL)
		{
			hdwp = m_pBarRightBottom->MoveWindow(rectSecondBar, FALSE, hdwp);
		}
		else if (m_pRightContainer != NULL)
		{
			m_pRightContainer->Resize(rectSecondBar, hdwp);
		}
	}
	else
	{
		ENSURE(m_pRightContainer == NULL);
		if (m_pBarRightBottom != NULL)
		{
			CDockablePane* pTabbedControlBar = NULL;
			pBar->AttachToTabWnd(m_pBarRightBottom, DM_DBL_CLICK, TRUE, &pTabbedControlBar);
			if (pTabbedControlBar != NULL && m_pBarRightBottom != NULL)
			{
				m_pContainerManager->ReplacePane(m_pBarRightBottom, pTabbedControlBar);
			}
			else if (pTabbedControlBar != NULL)
			{
				m_pContainerManager->AddPaneToList(pTabbedControlBar);
				m_pBarRightBottom = pTabbedControlBar;
			}
			return pTabbedControlBar;
		}

		m_pBarRightBottom = pBar;

		bShowSlider = (m_pBarLeftTop != NULL) || (m_pLeftContainer != NULL);

		if (m_pBarLeftTop != NULL)
		{
			m_pBarLeftTop->GetWindowRect(rectSecondBar);
		}
		else if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->GetWindowRect(rectSecondBar);
		}

		pDockSite->ScreenToClient(rectSecondBar);

		if (m_pSlider != NULL)
		{
			if (IsPaneDividerHorz())
			{
				rectSlider.bottom = rectNew.top;
				rectSlider.top = rectSlider.bottom - m_pSlider->GetWidth();
				rectSecondBar.bottom = rectSlider.top;
			}
			else
			{
				rectSlider.right = rectNew.left;
				rectSlider.left = rectSlider.right - m_pSlider->GetWidth();
				rectSecondBar.right = rectSlider.left;
			}
		}

		if (m_pBarLeftTop != NULL)
		{
			hdwp = m_pBarLeftTop->MoveWindow(rectSecondBar, FALSE, hdwp);
		}
		else if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->Resize(rectSecondBar, hdwp);
		}
	}

	if (m_pSlider != NULL)
	{
		if (bShowSlider)
		{
			hdwp = m_pSlider->MoveWindow(rectSlider, FALSE, hdwp);
		}
		else
		{
			m_pSlider->ShowWindow(SW_HIDE);
		}
	}

	rectContainer.UnionRect(rectNew, rectSecondBar);
	pDockSite->ClientToScreen(rectContainer);

	if (bExpandParentContainer)
	{
		// find the first parent container that has non-empty rectangle and
		// whose left/right bar/container should be expanded

		if (pNextContainer != NULL)
		{
			const CPaneDivider* pParentSlider = pNextContainer->GetPaneDivider();

			if (pParentSlider != NULL)
			{
				ASSERT_VALID(pParentSlider);

				CPaneContainer* pLeftContainer = (CPaneContainer*) pNextContainer->GetLeftPaneContainer();
				CPaneContainer* pRightContainer = (CPaneContainer*) pNextContainer->GetRightPaneContainer();

				BOOL bIsLeftContainer = FALSE;

				if (pLeftContainer != NULL &&
					pLeftContainer-> FindSubPaneContainer(this, BC_FIND_BY_CONTAINER))
				{
					bIsLeftContainer = TRUE;
				}
				else if (pRightContainer != NULL &&
					pRightContainer-> FindSubPaneContainer(this, BC_FIND_BY_CONTAINER))
				{
					bIsLeftContainer = FALSE;
				}
				else
				{
					return pBar;
				}

				pParentSlider->GetWindowRect(rectSlider);

				int nOffset = pParentSlider->GetWidth();

				if (bIsLeftContainer)
				{
					if (pParentSlider->IsHorizontal())
					{
						nOffset += nNewHeight;
						rectSlider.top = rectContainer.bottom;
						rectSlider.bottom = rectSlider.top + pParentSlider->GetWidth();
					}
					else
					{
						nOffset += nNewWidth;
						rectSlider.left = rectContainer.right;
						rectSlider.right = rectSlider.left + pParentSlider->GetWidth();
					}
				}
				else
				{
					if (pParentSlider->IsHorizontal())
					{
						nOffset = -(nNewHeight + pParentSlider->GetWidth());
						rectSlider.bottom = rectContainer.top;
						rectSlider.top = rectSlider.bottom - pParentSlider->GetWidth();
					}
					else
					{
						nOffset = -(nNewWidth + pParentSlider->GetWidth());;
						rectSlider.right = rectContainer.left;
						rectSlider.left = rectSlider.right - pParentSlider->GetWidth();
					}
				}

				pDockSite->ScreenToClient(rectSlider);
				if (m_pSlider != NULL)
				{
					hdwp = m_pSlider->MoveWindow(rectSlider, FALSE, hdwp);
				}
				pNextContainer->ResizePartOfPaneContainer(nOffset, !bIsLeftContainer, hdwp);
			}
		}
	}

	EndDeferWindowPos(hdwp);
	return pBar;
}

void CPaneContainer::ResizePartOfPaneContainer(int nOffset, BOOL bLeftPart, HDWP& hdwp)
{
	ASSERT_VALID(this);

	if (m_pSlider == NULL)
	{
		return;
	}

	CRect rectPart; rectPart.SetRectEmpty();
	CSize sizeMin(0, 0);

	if (bLeftPart && m_pLeftContainer != NULL)
	{
		m_pLeftContainer->GetWindowRect(rectPart);
		m_pLeftContainer->GetMinSize(sizeMin);
	}
	else if (bLeftPart && m_pBarLeftTop != NULL)
	{
		m_pBarLeftTop->GetWindowRect(rectPart);
		m_pBarLeftTop->GetMinSize(sizeMin);
	}
	else if (!bLeftPart && m_pRightContainer != NULL)
	{
		m_pRightContainer->GetWindowRect(rectPart);
		m_pRightContainer->GetMinSize(sizeMin);
	}
	else if (!bLeftPart && m_pBarRightBottom != NULL)
	{
		m_pBarRightBottom->GetWindowRect(rectPart);
		m_pBarRightBottom->GetMinSize(sizeMin);
	}
	else
	{
		return;
	}

	if (bLeftPart && IsPaneDividerHorz())
	{
		rectPart.bottom += nOffset;
		if (rectPart.Height() < sizeMin.cy)
		{
			rectPart.bottom = rectPart.top + sizeMin.cy;
		}
	}
	else if (bLeftPart && !IsPaneDividerHorz())
	{
		rectPart.right += nOffset;
		if (rectPart.Width() < sizeMin.cx)
		{
			rectPart.right = rectPart.left + sizeMin.cx;
		}
	}
	else if (!bLeftPart && IsPaneDividerHorz())
	{
		rectPart.top += nOffset;
		if (rectPart.Height() < sizeMin.cy)
		{
			rectPart.top = rectPart.bottom - sizeMin.cy;
		}
	}
	else
	{
		rectPart.left += nOffset;
		if (rectPart.Width() < sizeMin.cx)
		{
			rectPart.left = rectPart.right - sizeMin.cx;
		}
	}

	CWnd* pDockSite = m_pContainerManager->GetDockSiteFrameWnd();
	ASSERT_VALID(pDockSite);
	pDockSite->ScreenToClient(rectPart);

	if (bLeftPart && m_pLeftContainer != NULL)
	{
		m_pLeftContainer->Resize(rectPart, hdwp);
	}
	else if (bLeftPart && m_pBarLeftTop != NULL)
	{
		hdwp = m_pBarLeftTop->MoveWindow(rectPart, FALSE, hdwp);
	}
	else if (!bLeftPart && m_pRightContainer != NULL)
	{
		m_pRightContainer->Resize(rectPart, hdwp);
	}
	else if (!bLeftPart && m_pBarRightBottom != NULL)
	{
		hdwp = m_pBarRightBottom->MoveWindow(rectPart, FALSE, hdwp);
	}
}

BOOL CPaneContainer::AddSubPaneContainer(CPaneContainer* pContainer, BOOL bRightNodeNew)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pContainer);

	// slider must be unique
	ENSURE(m_pSlider != pContainer->GetPaneDivider());

	ENSURE(pContainer->GetLeftPane() != NULL || pContainer->GetRightPane() != NULL);

	CPaneContainer* pExistingContainer = NULL;
	// one of the nodes (control bars) is always new, e.g is being docked.
	// find a container that contains a node with an exisisting control bar
	// the incoming control bar is being docked to.
	const CPane* pBarToFind = bRightNodeNew ? pContainer->GetLeftPane() : pContainer->GetRightPane();
	ASSERT_VALID(pBarToFind);

	pExistingContainer = FindSubPaneContainer(pBarToFind, BC_FIND_BY_LEFT_BAR);

	if (pExistingContainer == NULL)
	{
		pExistingContainer = FindSubPaneContainer(pBarToFind, BC_FIND_BY_RIGHT_BAR);
	}

	// a node with the left or right bar must exist in the tree
	if (pExistingContainer == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pExistingContainer->AddNode(pContainer);
	return TRUE;
}

void CPaneContainer::AddNode(CPaneContainer* pContainer)
{
	ASSERT_VALID(this);
	// onr of the bars must be the same
	ENSURE(m_pBarLeftTop == pContainer->GetLeftPane() || m_pBarLeftTop == pContainer->GetRightPane() ||
		m_pBarRightBottom == pContainer->GetLeftPane() || m_pBarRightBottom == pContainer->GetRightPane());

	if (m_pBarLeftTop != NULL && (m_pBarLeftTop == pContainer->GetLeftPane() || m_pBarLeftTop == pContainer->GetRightPane()))
	{
		m_pBarLeftTop = NULL;
		m_pLeftContainer = pContainer;
	}
	else
	{
		m_pBarRightBottom = NULL;
		m_pRightContainer = pContainer;
	}

	pContainer->SetParentPaneContainer(this);
}

void CPaneContainer::RemovePane(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	BC_FIND_CRITERIA barType = BC_FIND_BY_LEFT_BAR;
	CPaneContainer* pContainer = FindSubPaneContainer(pBar, barType);
	if (pContainer == NULL)
	{
		barType = BC_FIND_BY_RIGHT_BAR;
		pContainer = FindSubPaneContainer(pBar, barType);
	}

	if (pContainer != NULL)
	{
		pContainer->DeletePane(pBar, barType);
	}
}

void CPaneContainer::AddRef()
{
	m_dwRefCount++;
}

DWORD CPaneContainer::Release()
{
	m_dwRefCount--;
	if (m_dwRefCount <= 0)
	{
		FreeReleasedPaneContainer();
		return 0;
	}

	return m_dwRefCount;
}

void CPaneContainer::FreeReleasedPaneContainer()
{
	int nCountNode = 0;

	if (m_pBarLeftTop != NULL)
	{
		nCountNode++;
	}
	if (m_pBarRightBottom != NULL)
	{
		nCountNode++;
	}
	if (m_pLeftContainer != NULL)
	{
		nCountNode++;
	}
	if (m_pRightContainer != NULL)
	{
		nCountNode++;
	}

	if (nCountNode > 1)
	{
		return;
	}

	if (m_dwRefCount <= 0)
	{
		if ((m_pSlider != NULL && !m_pSlider->IsDefault() || m_pSlider == NULL) && m_pParentContainer != NULL &&
			m_pParentContainer != m_pContainerManager->m_pRootContainer)
		{
			ENSURE(m_pParentContainer->GetLeftPaneContainer() != NULL || m_pParentContainer->GetRightPaneContainer() != NULL);

			BOOL bLeft = (m_pParentContainer->GetLeftPaneContainer() == this);
			m_pParentContainer->SetPaneContainer(NULL, bLeft);

			if (m_pBarLeftTop != NULL)
			{
				m_pParentContainer->SetPane(m_pBarLeftTop, bLeft);
				m_pBarLeftTop = NULL;
			}
			else if (m_pBarRightBottom != NULL)
			{
				m_pParentContainer->SetPane(m_pBarRightBottom, bLeft);
				m_pBarRightBottom = NULL;
			}
			else if (m_pLeftContainer != NULL)
			{
				m_pParentContainer->SetPaneContainer(m_pLeftContainer, bLeft);
				m_pLeftContainer = NULL;
			}
			else if (m_pRightContainer != NULL)
			{
				m_pParentContainer->SetPaneContainer(m_pRightContainer, bLeft);
				m_pRightContainer = NULL;
			}

			if (m_pSlider != NULL)
			{
				m_pSlider->DestroyWindow();
				m_pSlider = NULL;
			}

			//delete this;
			m_bDisposed = TRUE;
			gc.AddPaneContainer(this);
		}
		else
		{
			m_pContainerManager->NotifyPaneDivider();
		}
	}
}

void CPaneContainer::ReleaseEmptyPaneContainer()
{
	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->ReleaseEmptyPaneContainer();
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->ReleaseEmptyPaneContainer();
	}

	if (m_pParentContainer != m_pContainerManager->m_pRootContainer)
	{
		FreeReleasedPaneContainer();
	}
}

void CPaneContainer::DeletePane(CDockablePane* pBar, BC_FIND_CRITERIA barType)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	//-------------- set recent dock info
	//StoreRecentDockSiteInfo(pBar);
	//--------------

	CRect rectContainer;
	GetWindowRect(rectContainer);

	CRect rectBar;
	pBar->GetWindowRect(rectBar);

	// it's required to expand remaining container - take the width of slider first
	int nExpandOffset = 0;
	if (m_pSlider != NULL)
	{
		nExpandOffset = m_pSlider->GetWidth();

		nExpandOffset += IsPaneDividerHorz() ? rectBar.Height() : rectBar.Width();
	}

	HDWP hdwp = BeginDeferWindowPos(10);

	BOOL bNeedToExpandParentContainer = FALSE;

	if (barType == BC_FIND_BY_LEFT_BAR && pBar == m_pBarLeftTop)
	{
		m_pBarLeftTop = NULL;

		if (m_pBarRightBottom != NULL)
		{
			m_pBarRightBottom->MovePane(rectContainer, FALSE, hdwp);
		}
		else if (m_pRightContainer != NULL && !m_pRightContainer->IsEmpty())
		{
			// expanding right container - in the left direction
			m_pContainerManager->GetDockSiteFrameWnd()->ScreenToClient(rectContainer);
			Resize(rectContainer, hdwp);
		}
		else if (m_pParentContainer != NULL)
		{
			bNeedToExpandParentContainer = TRUE;
		}
	}
	else if (barType == BC_FIND_BY_RIGHT_BAR && pBar == m_pBarRightBottom)
	{
		m_pBarRightBottom = NULL;
		if (m_pBarLeftTop)
		{
			m_pBarLeftTop->MovePane(rectContainer, FALSE, hdwp);
		}
		else if (m_pLeftContainer != NULL && !m_pLeftContainer->IsEmpty())
		{
			// expanding left container - in the right direction
			m_pContainerManager->GetDockSiteFrameWnd()->ScreenToClient(rectContainer);
			Resize(rectContainer, hdwp);
		}
		else if (m_pParentContainer != NULL)
		{
			bNeedToExpandParentContainer = TRUE;
		}
	}
	else
	{
		ASSERT(FALSE);
	}

	if (bNeedToExpandParentContainer)
	{
		// find the first parent container that has non-empty rectangly and
		// whose left/right bar/containre should be expanded
		CPaneContainer* pNextContainer = m_pParentContainer;
		while (pNextContainer != NULL)
		{
			if (!pNextContainer->IsEmpty())
			{
				break;
			}
			pNextContainer = pNextContainer->GetParentPaneContainer();
		}

		if (pNextContainer != NULL)
		{
			CPaneDivider* pParentSlider = (CPaneDivider*) pNextContainer->GetPaneDivider();

			if (pParentSlider != NULL)
			{
				int nExpandParentContainerOffset = pParentSlider->IsHorizontal() ? rectBar.Height() : rectBar.Width();
				nExpandParentContainerOffset *= 2;
				nExpandParentContainerOffset += pParentSlider->GetWidth() +2;

				if (pNextContainer->IsLeftPartEmpty())
				{
					pNextContainer->StretchPaneContainer(-nExpandParentContainerOffset, !pParentSlider->IsHorizontal(), FALSE, TRUE, hdwp);
				}
				else if (pNextContainer->IsRightPartEmpty())
				{
					pNextContainer->StretchPaneContainer(nExpandParentContainerOffset, !pParentSlider->IsHorizontal(), TRUE, TRUE, hdwp);
				}
			}
		}
	}
	EndDeferWindowPos(hdwp);

	if (m_pSlider == NULL)
	{
		// it was last bar/container here
		m_pBarLeftTop = m_pBarRightBottom = NULL;
		m_pLeftContainer = m_pRightContainer = NULL;
	}
}

void CPaneContainer::StoreRecentDockSiteInfo(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	//-------------- set recent dock info
	CPaneDivider* pSlider = pBar->GetDefaultPaneDivider();

	// default slider is NULL when the bar is float_multi on miniframe
	if (pSlider == NULL || pBar->GetParentMiniFrame() != NULL)
	{
		pBar->m_recentDockInfo.StoreDockInfo(this);
		return;
	}

	// DO NOT SAVE recent dock info during transition from autohide mode
	// to the regular dock mode! (because it's transition from dock to dock state)
	if (!pSlider->IsAutoHideMode())
	{
		pBar->m_recentDockInfo.StoreDockInfo(this);
	}
}

void CPaneContainer::SetPane(CDockablePane* pBar, BOOL bLeft)
{
	ASSERT_VALID(this);
	// pBar can be NULL
	if (bLeft)
	{
		m_pBarLeftTop = pBar;
	}
	else
	{
		m_pBarRightBottom = pBar;
	}
}

void CPaneContainer::SetPaneContainer(CPaneContainer* pContainer, BOOL bLeft)
{
	ASSERT_VALID(this);

	if (bLeft)
	{
		m_pLeftContainer = pContainer;
	}
	else
	{
		m_pRightContainer = pContainer;
	}

	if (pContainer != NULL)
	{
		pContainer->SetParentPaneContainer(this);
	}
}

CPaneContainer* CPaneContainer::FindSubPaneContainer(const CObject* pObject, BC_FIND_CRITERIA findCriteria)
{
	ASSERT_VALID(this);
	ENSURE(pObject != NULL);

	switch (findCriteria)
	{
	case BC_FIND_BY_LEFT_BAR:
		if (m_pBarLeftTop == pObject)
		{
			return this;
		}
		break;
	case BC_FIND_BY_RIGHT_BAR:
		if (m_pBarRightBottom == pObject)
		{
			return this;
		}
		break;
	case BC_FIND_BY_SLIDER:
		if (m_pSlider == pObject)
		{
			return this;
		}
		break;
	case BC_FIND_BY_CONTAINER:
		if (this == pObject)
		{
			return this;
		}
		break;
	}

	CPaneContainer* pSubContainer = NULL;

	if (m_pLeftContainer != NULL)
	{
		pSubContainer = m_pLeftContainer->FindSubPaneContainer(pObject, findCriteria);
	}

	if (pSubContainer == NULL && m_pRightContainer != NULL)
	{
		pSubContainer = m_pRightContainer->FindSubPaneContainer(pObject, findCriteria);
	}

	return pSubContainer;
}

void CPaneContainer::CalculateRecentSize()
{
	CRect rectContainer; rectContainer.SetRectEmpty();

	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode();

	GetWindowRect(rectContainer);

	CRect rectLeft; rectLeft.SetRectEmpty();
	CRect rectRight; rectRight.SetRectEmpty();

	CSize sizeMinLeft;
	CSize sizeMinRight;

	double dLeftPercent = 0.;

	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsPaneVisible() || bAutoHideMode))
	{
		m_pBarLeftTop->GetWindowRect(rectLeft);
		m_pBarLeftTop->GetMinSize(sizeMinLeft);
	}

	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsVisible() || bAutoHideMode))
	{
		m_pLeftContainer->GetWindowRect(rectLeft);
		m_pLeftContainer->GetMinSize(sizeMinLeft);
	}

	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsPaneVisible() || bAutoHideMode))
	{
		m_pBarRightBottom->GetWindowRect(rectRight);
		m_pBarRightBottom->GetMinSize(sizeMinRight);
	}

	if (m_pRightContainer != NULL && (m_pRightContainer->IsVisible() || bAutoHideMode))
	{
		m_pRightContainer->GetWindowRect(rectRight);
		m_pRightContainer->GetMinSize(sizeMinRight);
	}

	BOOL bCheckVisibility = !bAutoHideMode;

	if (!IsLeftPartEmpty(bCheckVisibility) && IsRightPartEmpty(bCheckVisibility))
	{
		if (m_pBarLeftTop != NULL)
		{
			m_pBarLeftTop->SetLastPercentInPaneContainer(100);
		}

		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->CalculateRecentSize();
			m_pLeftContainer->SetRecentPercent(100);
		}
	}
	else if (IsLeftPartEmpty(bCheckVisibility) && !IsRightPartEmpty(bCheckVisibility))
	{

		if (m_pBarRightBottom != NULL)
		{
			m_pBarRightBottom->SetLastPercentInPaneContainer(100);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->CalculateRecentSize();
			m_pRightContainer->SetRecentPercent(100);
		}
	}
	else if (!IsLeftPartEmpty(bCheckVisibility) && !IsRightPartEmpty(bCheckVisibility))
	{
		ENSURE(m_pSlider != NULL);

		if (IsPaneDividerHorz())
		{
			int nPercent = -1;
			if ((rectLeft.Height() + rectRight.Height()) > rectContainer.Height())
			{
				nPercent = 50;
				if (rectLeft.Height() == rectContainer.Height())
				{
					if (m_pBarRightBottom != NULL)
					{
						nPercent = m_pBarRightBottom->GetLastPercentInPaneContainer();
					}
					else if (m_pRightContainer != NULL)
					{
						nPercent = m_pRightContainer->GetRecentPercent();
					}

					rectLeft.bottom = rectLeft.top + rectContainer.Height() - ((rectContainer.Height() * nPercent) / 100);
					nPercent = 100 - nPercent;
				}
				else if (rectRight.Height() == rectContainer.Height())
				{
					if (m_pBarLeftTop != NULL)
					{
						nPercent = m_pBarLeftTop->GetLastPercentInPaneContainer();
					}
					else if (m_pLeftContainer != NULL)
					{
						nPercent = m_pLeftContainer->GetRecentPercent();
					}

					rectLeft.bottom = rectLeft.top + ((rectContainer.Height() * nPercent) / 100);
				}
			}

			dLeftPercent = ((double) rectLeft.Height()) / rectContainer.Height() * 100;

			if (nPercent != -1)
			{
				dLeftPercent = nPercent;
			}
		}
		else
		{
			int nPercent = -1;
			if ((rectLeft.Width() + rectRight.Width()) > rectContainer.Width())
			{
				if (rectLeft.Width() == rectContainer.Width())
				{
					nPercent = 50;
					if (m_pBarRightBottom != NULL)
					{
						nPercent = m_pBarRightBottom->GetLastPercentInPaneContainer();
					}
					else if (m_pRightContainer != NULL)
					{
						nPercent = m_pRightContainer->GetRecentPercent();
					}

					rectLeft.right = rectLeft.left + rectContainer.Width() - ((rectContainer.Width() * nPercent) / 100);
					nPercent = 100 - nPercent;
				}
				else if (rectRight.Width() == rectContainer.Width())
				{
					nPercent = 50;
					if (m_pBarLeftTop != NULL)
					{
						nPercent = m_pBarLeftTop->GetLastPercentInPaneContainer();
					}
					else if (m_pLeftContainer != NULL)
					{
						nPercent = m_pLeftContainer->GetRecentPercent();
					}

					rectLeft.right = rectLeft.left + ((rectContainer.Width() * nPercent) / 100);
				}
			}

			dLeftPercent = ((double) rectLeft.Width()) / rectContainer.Width() * 100;

			if (nPercent != -1)
			{
				dLeftPercent = nPercent;
			}
		}

		if (m_pBarLeftTop != NULL)
		{
			m_pBarLeftTop->SetLastPercentInPaneContainer((int) dLeftPercent);
		}

		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->CalculateRecentSize();
			m_pLeftContainer->SetRecentPercent((int) dLeftPercent);
		}

		if (m_pBarRightBottom != NULL)
		{
			m_pBarRightBottom->SetLastPercentInPaneContainer(100 - (int) dLeftPercent);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->CalculateRecentSize();
			m_pRightContainer->SetRecentPercent(100 - (int) dLeftPercent);
		}
	}
}
//-----------------------------------------------------------------------------------//
void CPaneContainer::Resize(CRect rect, HDWP& hdwp, BOOL bRedraw)
{
	CRect rectContainer; rectContainer.SetRectEmpty();
	CRect rectSlider; rectSlider.SetRectEmpty();

	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode();
	// VCheck
	if (m_pSlider != NULL && (m_pSlider->IsPaneVisible() || bAutoHideMode))
	{
		m_pSlider->GetWindowRect(rectSlider);
	}

	GetWindowRect(rectContainer);

	CRect rectLeft; rectLeft.SetRectEmpty();
	CRect rectRight; rectRight.SetRectEmpty();

	CSize sizeMinLeft;
	CSize sizeMinRight;

	double dLeftPercent = 0.;

	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsPaneVisible() || bAutoHideMode))
	{
		m_pBarLeftTop->GetWindowRect(rectLeft);
		m_pBarLeftTop->GetMinSize(sizeMinLeft);
	}

	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsVisible() || bAutoHideMode))
	{
		m_pLeftContainer->GetWindowRect(rectLeft);
		m_pLeftContainer->GetMinSize(sizeMinLeft);
	}

	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsPaneVisible() || bAutoHideMode))
	{
		m_pBarRightBottom->GetWindowRect(rectRight);
		m_pBarRightBottom->GetMinSize(sizeMinRight);
	}

	if (m_pRightContainer != NULL && (m_pRightContainer->IsVisible() || bAutoHideMode))
	{
		m_pRightContainer->GetWindowRect(rectRight);
		m_pRightContainer->GetMinSize(sizeMinRight);
	}

	BOOL bCheckVisibility = !bAutoHideMode;

	if (!IsLeftPartEmpty(bCheckVisibility) && IsRightPartEmpty(bCheckVisibility))
	{
		if (m_pBarLeftTop != NULL)
		{
			if (rect.Width() < sizeMinLeft.cx && CPane::m_bHandleMinSize)
			{
				rect.right = rect.left + sizeMinLeft.cx;
			}
			if (rect.Height() < sizeMinLeft.cy && CPane::m_bHandleMinSize)
			{
				rect.bottom = rect.top + sizeMinLeft.cy;
			}
			hdwp = m_pBarLeftTop->MoveWindow(rect, bRedraw, hdwp);
		}

		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->Resize(rect, hdwp, bRedraw);
		}
	}
	else if (IsLeftPartEmpty(bCheckVisibility) && !IsRightPartEmpty(bCheckVisibility))
	{
		if (m_pBarRightBottom != NULL)
		{
			if (rect.Width() < sizeMinRight.cx && CPane::m_bHandleMinSize)
			{
				rect.right = rect.left + sizeMinRight.cx;
			}
			if (rect.Height() < sizeMinRight.cy && CPane::m_bHandleMinSize)
			{
				rect.bottom = rect.top + sizeMinRight.cy;
			}

			hdwp = m_pBarRightBottom->MoveWindow(rect, bRedraw, hdwp);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->Resize(rect, hdwp, bRedraw);
		}
	}
	else if (!IsLeftPartEmpty(bCheckVisibility) && !IsRightPartEmpty(bCheckVisibility))
	{
		CRect rectFinalLeft = rect;
		CRect rectFinalRight = rect;
		CRect rectFinalSlider = rect;

		ENSURE(m_pSlider != NULL);
		ASSERT_VALID(m_pSlider );

		if (IsPaneDividerHorz())
		{
			int nPercent = -1;
			if ((rectLeft.Height() + rectRight.Height()) > rectContainer.Height() || rectLeft.IsRectEmpty() || rectRight.IsRectEmpty())
			{
				nPercent = 50;
				if (rectLeft.Height() == rectContainer.Height())
				{
					if (m_pBarRightBottom != NULL)
					{
						nPercent = m_pBarRightBottom->GetLastPercentInPaneContainer();
					}
					else if (m_pRightContainer != NULL)
					{
						nPercent = m_pRightContainer->GetRecentPercent();
					}

					if (nPercent == 100 || nPercent == 0)
					{
						nPercent = 50;
					}

					rectLeft.bottom = rectLeft.top + rectContainer.Height() - ((rectContainer.Height() * nPercent) / 100);
					nPercent = 100 - nPercent;
				}
				else if (rectRight.Height() == rectContainer.Height())
				{
					if (m_pBarLeftTop != NULL)
					{
						nPercent = m_pBarLeftTop->GetLastPercentInPaneContainer();
					}
					else if (m_pLeftContainer != NULL)
					{
						nPercent = m_pLeftContainer->GetRecentPercent();
					}

					if (nPercent == 100 || nPercent == 0)
					{
						nPercent = 50;
					}

					rectLeft.bottom = rectLeft.top + ((rectContainer.Height() * nPercent) / 100);
				}
			}

			int nDelta = rect.Height() - rectContainer.Height();

			dLeftPercent = ((double) rectLeft.Height()) / rectContainer.Height() * 100;

			if (dLeftPercent == 100 || dLeftPercent == 0)
			{
				dLeftPercent = 50;
			}
			if (m_bMaintainPercentage)
			{
				if (nDelta != 0)
				{
					rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height() + (int)((double)nDelta * (dLeftPercent) / 100.);
				}
				else
				{
					rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height();
					if (nPercent != -1)
					{
						dLeftPercent = nPercent;
					}
				}
			}
			else
			{
				if (m_bRetainInternalSliderPosition)
				{
					if (nDelta > 0)
					{
						rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height();
					}

					else if (nDelta < 0)
					{
						rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height();
					}
					else
					{
						rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height();
						if (nPercent != -1)
						{
							dLeftPercent = nPercent;
						}
					}
					if (CWnd::GetCapture() != m_pSlider)
					{
						CRect rc(rectSlider);
						m_pSlider->GetParent()->ScreenToClient(rc);
						rectFinalLeft.bottom = rc.top;
					}
					dLeftPercent =   rectFinalLeft.Height() / (static_cast<double>(rectContainer.Height()));
				}
				else
				{
					if (nDelta > 0)
					{
						rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height() + (int)((double)(nDelta * (100 - dLeftPercent)) / 100.) ;
					}

					else if (nDelta < 0)
					{
						rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height() + (int)((double) nDelta * (dLeftPercent) / 100.);
					}
					else
					{
						rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height();
						if (nPercent != -1)
						{
							dLeftPercent = nPercent;
						}
					}
				}
			}

			rectFinalSlider.top = rectFinalLeft.bottom;
			rectFinalSlider.bottom = rectFinalSlider.top + m_pSlider->GetWidth();
			rectFinalRight.top = rectFinalSlider.bottom;

			if (CPane::m_bHandleMinSize)
			{
				int deltaLeft = sizeMinLeft.cy - rectFinalLeft.Height();
				int deltaRight = sizeMinRight.cy - rectFinalRight.Height();
				int nSliderWidth = m_pSlider->GetWidth();

				if (deltaLeft <= 0 && deltaRight <= 0)
				{
				}
				else if (deltaLeft > 0 && deltaRight <= 0)
				{
					rectFinalLeft.bottom += deltaLeft;
					rectFinalRight.top = rectFinalLeft.bottom + nSliderWidth;
					if (rectFinalRight.Height() < sizeMinRight.cy)
					{
						rectFinalRight.bottom = rectFinalRight.top + sizeMinRight.cy;
					}
				}
				else if (deltaLeft <= 0 && deltaRight > 0)
				{
					rectFinalLeft.bottom -= deltaRight;
					if (rectFinalLeft.Height() < sizeMinLeft.cy)
					{
						rectFinalLeft.bottom = rectFinalLeft.top + sizeMinLeft.cy;
					}

					rectFinalRight.top = rectFinalLeft.bottom + nSliderWidth;
					rectFinalRight.bottom = rectFinalRight.top + sizeMinRight.cy;
				}
				else if (deltaLeft > 0 && deltaRight > 0)
				{
					rectFinalLeft.bottom = rectFinalLeft.top + sizeMinLeft.cy;
					rectFinalRight.top = rectFinalLeft.bottom + nSliderWidth;
					rectFinalRight.bottom = rectFinalRight.top + sizeMinRight.cy;
				}

				rectFinalSlider.top = rectFinalLeft.bottom;
				rectFinalSlider.bottom = rectFinalSlider.top + nSliderWidth;

				dLeftPercent = ((double) rectFinalLeft.Height()) / rectContainer.Height() * 100;

				if (rectFinalLeft.Width() < sizeMinLeft.cx)
				{
					rectFinalLeft.right = rectFinalLeft.left + sizeMinLeft.cx;
					rectFinalRight.right = rectFinalRight.left + sizeMinLeft.cx;
				}
			}
		}
		else
		{
			int nPercent = -1;
			if ((rectLeft.Width() + rectRight.Width()) > rectContainer.Width() || rectLeft.IsRectEmpty() || rectRight.IsRectEmpty())
			{
				if (rectLeft.Width() == rectContainer.Width())
				{
					nPercent = 50;
					if (m_pBarRightBottom != NULL)
					{
						nPercent = m_pBarRightBottom->GetLastPercentInPaneContainer();
					}
					else if (m_pRightContainer != NULL)
					{
						nPercent = m_pRightContainer->GetRecentPercent();
					}

					if (nPercent == 100 || nPercent == 0)
					{
						nPercent = 50;
					}

					rectLeft.right = rectLeft.left + rectContainer.Width() - ((rectContainer.Width() * nPercent) / 100);
					nPercent = 100 - nPercent;
				}
				else if (rectRight.Width() == rectContainer.Width())
				{
					nPercent = 50;
					if (m_pBarLeftTop != NULL)
					{
						nPercent = m_pBarLeftTop->GetLastPercentInPaneContainer();
					}
					else if (m_pLeftContainer != NULL)
					{
						nPercent = m_pLeftContainer->GetRecentPercent();
					}

					if (nPercent == 100 || nPercent == 0)
					{
						nPercent = 50;
					}

					rectLeft.right = rectLeft.left + ((rectContainer.Width() * nPercent) / 100);
				}
			}

			int nDelta = rect.Width() - rectContainer.Width();
			dLeftPercent = ((double) rectLeft.Width()) / rectContainer.Width() * 100;

			if (dLeftPercent == 100 || dLeftPercent == 0)
			{
				dLeftPercent = 50;
			}

			if (m_bMaintainPercentage)
			{
				if (nDelta != 0)
				{
					rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width() + (int)(((double) nDelta * dLeftPercent) / 100.);
				}
				else
				{
					rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width();
					if (nPercent != -1)
					{
						dLeftPercent = nPercent;
					}
				}
			}
			else
			{
				if (m_bRetainInternalSliderPosition)
				{
					if (nDelta > 0)
					{
						rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width();
					}
					else if (nDelta < 0)
					{
						rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width();
					}
					else
					{
						rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width();
						if (nPercent != -1)
						{
							dLeftPercent = nPercent;
						}
					}
					if (CWnd::GetCapture() != m_pSlider)
					{
						CRect rc(rectSlider);
						m_pSlider->GetParent()->ScreenToClient(rc);
						rectFinalLeft.right = rc.left;
					}
					dLeftPercent =   rectFinalLeft.Width() / (static_cast<double>(rectContainer.Width()));
				}
				else
				{
					if (nDelta > 0)
					{
						rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width() + (int)(((double) nDelta * (100 - dLeftPercent)) / 100.);
					}
					else if (nDelta < 0)
					{
						rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width() + (int)(((double) nDelta * dLeftPercent) / 100.);
					}
					else
					{
						rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width();
						if (nPercent != -1)
						{
							dLeftPercent = nPercent;
						}
					}
				}
			}

			rectFinalSlider.left = rectFinalLeft.right;
			rectFinalSlider.right = rectFinalSlider.left + m_pSlider->GetWidth();
			rectFinalRight.left = rectFinalSlider.right;

			if (CPane::m_bHandleMinSize)
			{
				int deltaLeft = sizeMinLeft.cx - rectFinalLeft.Width();
				int deltaRight = sizeMinRight.cx - rectFinalRight.Width();
				int nSliderWidth = m_pSlider->GetWidth();

				if (deltaLeft <= 0 && deltaRight <= 0)
				{
				}
				else if (deltaLeft > 0 && deltaRight <= 0)
				{
					rectFinalLeft.right += deltaLeft;
					rectFinalRight.left = rectFinalLeft.right + nSliderWidth;
					if (rectFinalRight.Width() < sizeMinRight.cx)
					{
						rectFinalRight.right = rectFinalRight.left + sizeMinRight.cx;
					}
				}
				else if (deltaLeft <= 0 && deltaRight > 0)
				{
					rectFinalLeft.right -= deltaRight;
					if (rectFinalLeft.Width() < sizeMinLeft.cx)
					{
						rectFinalLeft.right = rectFinalLeft.left + sizeMinLeft.cx;
					}

					rectFinalRight.left = rectFinalLeft.right + nSliderWidth;
					rectFinalRight.right = rectFinalRight.left + sizeMinRight.cx;
				}
				else if (deltaLeft > 0 && deltaRight > 0)
				{
					rectFinalLeft.right = rectFinalLeft.left + sizeMinLeft.cx;
					rectFinalRight.left = rectFinalLeft.right + nSliderWidth;
					rectFinalRight.right = rectFinalRight.left + sizeMinRight.cx;
				}

				rectFinalSlider.left = rectFinalLeft.right;
				rectFinalSlider.right = rectFinalSlider.left + nSliderWidth;

				dLeftPercent = ((double) rectFinalLeft.Width()) / rectContainer.Width() * 100;

				if (rectFinalLeft.Height() < sizeMinLeft.cy)
				{
					rectFinalLeft.bottom = rectFinalLeft.top + sizeMinLeft.cy;
					rectFinalRight.bottom = rectFinalRight.top + sizeMinLeft.cy;
				}
			}
		}

		if (m_pBarLeftTop != NULL)
		{
			hdwp = m_pBarLeftTop->MoveWindow(rectFinalLeft, bRedraw, hdwp);
			m_pBarLeftTop->SetLastPercentInPaneContainer((int) dLeftPercent);
		}

		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->Resize(rectFinalLeft, hdwp, bRedraw);
			m_pLeftContainer->SetRecentPercent((int) dLeftPercent);
		}

		if (m_pBarRightBottom != NULL)
		{
			hdwp = m_pBarRightBottom->MoveWindow(rectFinalRight, bRedraw, hdwp);
			m_pBarRightBottom->SetLastPercentInPaneContainer(100 - (int) dLeftPercent);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->Resize(rectFinalRight, hdwp, bRedraw);
			m_pRightContainer->SetRecentPercent(100 - (int) dLeftPercent);
		}

		if (m_pSlider->IsPaneVisible())
		{
			hdwp = m_pSlider->MoveWindow(rectFinalSlider, bRedraw, hdwp);
		}

	}
}

int CPaneContainer::StretchPaneContainer(int nOffset, BOOL bStretchHorz, BOOL bLeftBar, BOOL bMoveSlider, HDWP& hdwp)
{
	ASSERT_VALID(this);

	if ((AfxGetMainWnd()->GetExStyle() & WS_EX_LAYOUTRTL) && bStretchHorz)
	{
		nOffset = -nOffset;
	}

	int nDirection = nOffset < 0 ? -1 : 1;

	CSize sizeStretch(0, 0);
	bStretchHorz ? sizeStretch.cx = nOffset : sizeStretch.cy = nOffset;

	int nAvailSpace = bStretchHorz ? CalcAvailableSpace(sizeStretch, bLeftBar).cx : CalcAvailableSpace(sizeStretch, bLeftBar).cy;
	// set the sign here
	int nActualSize = nDirection * min(abs(nOffset), abs(nAvailSpace));

	if (abs(nActualSize) == 0)
	{
		return 0;
	}

	// check whether the container's native slider has the same
	// orientation as stretch direction
	if (m_pSlider == NULL || (m_pSlider->IsHorizontal() && bStretchHorz || !m_pSlider->IsHorizontal() && !bStretchHorz))
	{
		// just use minimum of the avail. and req to stretch both bars and the
		// slider
		ResizePane(nActualSize, m_pBarLeftTop, m_pLeftContainer, bStretchHorz, bLeftBar, hdwp);
		ResizePane(nActualSize, m_pBarRightBottom, m_pRightContainer, bStretchHorz, bLeftBar, hdwp);
		// resize the slider
		if (bMoveSlider && m_pSlider != NULL)
		{
			CRect rectSlider;
			m_pSlider->GetWindowRect(rectSlider);
			if (m_pSlider->IsHorizontal())
			{
				bLeftBar ? rectSlider.right += nActualSize : rectSlider.left += nActualSize;
			}
			else
			{
				bLeftBar ? rectSlider.bottom += nActualSize : rectSlider.top += nActualSize;
			}
			if (m_pSlider->IsPaneVisible())
			{
				m_pSlider->GetParent()->ScreenToClient(rectSlider);
				m_pSlider->MoveWindow(rectSlider, FALSE, hdwp);
			}
		}
	}
	else
	{
		// treat bar's available space individually
		int nLeftAvailOffset  = CalcAvailablePaneSpace(nOffset, m_pBarLeftTop, m_pLeftContainer, bLeftBar);
		int nRigthAvailOffset = CalcAvailablePaneSpace(nOffset, m_pBarRightBottom, m_pRightContainer, bLeftBar);

		int nSliderOffset = 0;
		int nBarOffset = nActualSize;
		if (abs(nLeftAvailOffset) == abs(nRigthAvailOffset))
		{
			nSliderOffset = (abs(nLeftAvailOffset) / 2 + 1) * nDirection;
		}
		else
		{
			nSliderOffset = nActualSize;
		}

		CPoint pt(0, 0);
		bStretchHorz ? pt.x = nSliderOffset : pt.y = nSliderOffset;

		if (bMoveSlider)
		{
			m_pSlider->Move(pt);
		}

		if (bLeftBar)
		{
			ResizePane(nBarOffset, m_pBarRightBottom, m_pRightContainer, bStretchHorz, bLeftBar, hdwp);
		}
		else
		{
			ResizePane(nBarOffset, m_pBarLeftTop, m_pLeftContainer, bStretchHorz, bLeftBar, hdwp);
		}
	}

	return nActualSize;
}

int CPaneContainer::OnMoveInternalPaneDivider(int nOffset, HDWP& hdwp)
{

	ASSERT_VALID(this);
	ASSERT_VALID(m_pSlider);

	CRect rectLeft; rectLeft.SetRectEmpty();
	CRect rectRight; rectRight.SetRectEmpty();

	CSize sizeMinLeft;
	CSize sizeMinRight;

	if (m_pBarLeftTop != NULL)
	{
		m_pBarLeftTop->GetWindowRect(rectLeft);
		m_pBarLeftTop->GetMinSize(sizeMinLeft);
	}

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->GetWindowRect(rectLeft);
		m_pLeftContainer->GetMinSize(sizeMinLeft);
	}

	if (m_pBarRightBottom != NULL)
	{
		m_pBarRightBottom->GetWindowRect(rectRight);
		m_pBarRightBottom->GetMinSize(sizeMinRight);
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->GetWindowRect(rectRight);
		m_pRightContainer->GetMinSize(sizeMinRight);
	}

	m_pSlider->GetParent()->ScreenToClient(rectLeft);
	m_pSlider->GetParent()->ScreenToClient(rectRight);

	if (!rectLeft.IsRectEmpty())
	{
		if (IsPaneDividerHorz())
		{
			rectLeft.bottom += nOffset;
			if (rectLeft.Height() < sizeMinLeft.cy)
			{
				rectLeft.bottom = rectLeft.top + sizeMinLeft.cy;
			}
		}
		else
		{
			rectLeft.right += nOffset;
			if (rectLeft.Width() < sizeMinLeft.cx)
			{
				rectLeft.right = rectLeft.left + sizeMinLeft.cx;
			}
		}
	}

	if (!rectRight.IsRectEmpty())
	{
		if (IsPaneDividerHorz())
		{
			rectRight.top += nOffset;
			if (rectRight.Height() < sizeMinRight.cy)
			{
				rectRight.top = rectRight.bottom - sizeMinRight.cy;
			}
		}
		else
		{
			rectRight.left += nOffset;
			if (rectRight.Width() < sizeMinRight.cx)
			{
				rectRight.left = rectRight.right - sizeMinRight.cx;
			}
		}
	}

	if (m_pBarLeftTop != NULL)
	{
		hdwp = m_pBarLeftTop->MoveWindow(rectLeft, TRUE, hdwp);
	}

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->Resize(rectLeft, hdwp);
	}

	if (m_pBarRightBottom != NULL)
	{
		hdwp = m_pBarRightBottom->MoveWindow(rectRight, TRUE, hdwp);
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->Resize(rectRight, hdwp);
	}

	return nOffset;
}

void CPaneContainer::ResizePane(int nOffset, CPane* pBar, CPaneContainer* pContainer, BOOL bHorz, BOOL bLeftBar, HDWP& hdwp)
{
	ASSERT_VALID(this);
	if (pBar != NULL)
	{
		CRect rectBar;
		pBar->GetWindowRect(rectBar);
		if (bHorz)
		{
			bLeftBar ? rectBar.bottom += nOffset : rectBar.top -= nOffset;
		}
		else
		{
			bLeftBar ? rectBar.right += nOffset : rectBar.left += nOffset;
		}
		pBar->MovePane(rectBar, FALSE, hdwp);
	}
	else if (pContainer != NULL)
	{
		// the container will be stretched by "foregn" slider, threfore
		// if the native bar's slider is horizontal, a container
		// will be stretched vertically
		pContainer->StretchPaneContainer(nOffset, bHorz, bLeftBar, TRUE, hdwp);
	}
}

void CPaneContainer::Move(CPoint ptNewLeftTop)
{
	ASSERT_VALID(this);

	CRect rectLeft; rectLeft.SetRectEmpty();
	CRect rectRight; rectRight.SetRectEmpty();

	int nLeftOffset = 0;
	int nTopOffset = 0;

	if (m_pBarLeftTop != NULL)
	{
		m_pBarLeftTop->GetWindowRect(rectLeft);
		m_pBarLeftTop->SetWindowPos(NULL, ptNewLeftTop.x, ptNewLeftTop.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->GetWindowRect(rectLeft);
		m_pLeftContainer->Move(ptNewLeftTop);
	}

	nLeftOffset = rectLeft.Width();
	nTopOffset = rectLeft.Height();

	if (m_pSlider != NULL)
	{
		if (m_pSlider->IsHorizontal())
		{
			m_pSlider->SetWindowPos(NULL, ptNewLeftTop.x, ptNewLeftTop.y + nTopOffset, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			nTopOffset += m_pSlider->GetWidth();
			nLeftOffset = 0;
		}
		else
		{
			m_pSlider->SetWindowPos(NULL, ptNewLeftTop.x + nLeftOffset, ptNewLeftTop.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			nLeftOffset += m_pSlider->GetWidth();
			nTopOffset = 0;
		}
	}

	if (m_pBarRightBottom != NULL)
	{
		m_pBarRightBottom->SetWindowPos(NULL, ptNewLeftTop.x + nLeftOffset, ptNewLeftTop.y + nTopOffset,
			0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	if (m_pRightContainer != NULL)
	{
		CPoint pt(ptNewLeftTop.x + nLeftOffset, ptNewLeftTop.y + nTopOffset);
		m_pRightContainer->Move(pt);
	}
}

void CPaneContainer::MoveWnd(CWnd* pWnd, int nOffset, BOOL bHorz)
{
	ASSERT_VALID(this);

	if (pWnd != NULL)
	{
		CWnd* pParent = pWnd->GetParent();
		ASSERT_VALID(pParent);

		CRect rectWnd;
		CRect rectParent;

		pParent->GetClientRect(rectParent);
		pWnd->GetWindowRect(rectWnd);
		pParent->ScreenToClient(rectWnd);

		int nActualOffset = bHorz ? rectWnd.left - rectParent.left : rectWnd.top - rectParent.top;

		bHorz ? rectWnd.OffsetRect(CPoint(nOffset - nActualOffset, 0)) : rectWnd.OffsetRect(CPoint(0, nOffset - nActualOffset));

		pWnd->MoveWindow(rectWnd, TRUE);
	}
}

int CPaneContainer::CalcAvailablePaneSpace(int nRequiredOffset, CPane* pBar, CPaneContainer* pContainer, BOOL bLeftBar)
{
	ASSERT_VALID(this);

	CRect rectBar;
	int nAvailableSpace = nRequiredOffset;

	if (pBar != NULL)
	{
		ASSERT_VALID(pBar);
		pBar->GetWindowRect(rectBar);
		if (IsPaneDividerHorz())
		{
			bLeftBar ? rectBar.bottom += nRequiredOffset : rectBar.top += nRequiredOffset;
			nAvailableSpace = pBar->CalcAvailableSize(rectBar).cy;
		}
		else
		{
			bLeftBar ? rectBar.right += nRequiredOffset : rectBar.left += nRequiredOffset;
			nAvailableSpace = pBar->CalcAvailableSize(rectBar).cx;
		}
	}
	else if (pContainer != NULL)
	{
		ASSERT_VALID(pContainer);
		nAvailableSpace = IsPaneDividerHorz() ? pContainer->CalcAvailableSpace(CSize(0, nRequiredOffset), bLeftBar).cy :
		pContainer->CalcAvailableSpace(CSize(nRequiredOffset, 0), bLeftBar).cx;
	}

	return nAvailableSpace;
}

CSize CPaneContainer::CalcAvailableSpace(CSize sizeStretch, BOOL bLeftBar)
{
	ASSERT_VALID(this);

	CRect rectWndOrg;
	GetWindowRect(rectWndOrg);
	CRect rectWndNew = rectWndOrg;

	if (bLeftBar)
	{
		rectWndNew.right += sizeStretch.cx;
		rectWndNew.bottom += sizeStretch.cy;
	}
	else
	{
		rectWndNew.left += sizeStretch.cx;
		rectWndNew.top += sizeStretch.cy;
	}

	CSize sizeMin;
	GetMinSize(sizeMin);

	CSize sizeAvailable(sizeStretch.cx, sizeStretch.cy);

	if (rectWndNew.Width() < sizeMin.cx)
	{
		sizeAvailable.cx = rectWndOrg.Width() - sizeMin.cx;
		// if already less or eq. to minimum
		if (sizeAvailable.cx < 0)
		{
			sizeAvailable.cx = 0;
		}

		// preserve direction
		if (sizeStretch.cx < 0)
		{
			sizeAvailable.cx = -sizeAvailable.cx;
		}
	}

	if (rectWndNew.Height() < sizeMin.cy)
	{
		sizeAvailable.cy = rectWndNew.Height() - sizeMin.cy;
		if (sizeAvailable.cy < 0)
		{
			sizeAvailable.cy = 0;
		}

		// preserve direction
		if (sizeStretch.cy < 0)
		{
			sizeAvailable.cy = -sizeAvailable.cy;
		}
	}

	return sizeAvailable;
}

BOOL CPaneContainer::IsLeftPaneContainer() const
{
	ASSERT_VALID(this);

	if (m_pParentContainer == NULL)
	{
		return TRUE;
	}

	if (m_pParentContainer->GetLeftPaneContainer() == this)
	{
		return TRUE;
	}

	if (m_pParentContainer->GetRightPaneContainer() == this)
	{
		return FALSE;
	}

	return FALSE;
}

BOOL CPaneContainer::IsLeftPane(CDockablePane* pBar) const
{
	if (pBar == m_pBarLeftTop)
	{
		return TRUE;
	}

	if (pBar == m_pBarRightBottom)
	{
		return FALSE;
	}

	return FALSE;
}

BOOL CPaneContainer::IsEmpty() const
{
	ASSERT_VALID(this);

	return(m_pBarLeftTop == NULL && m_pBarRightBottom == NULL && (m_pLeftContainer == NULL || m_pLeftContainer->IsEmpty()) &&
		(m_pRightContainer == NULL || m_pRightContainer->IsEmpty()));
}

BOOL CPaneContainer::IsLeftPartEmpty(BOOL bCheckVisibility) const
{
	ASSERT_VALID(this);
	return((m_pBarLeftTop == NULL || bCheckVisibility && m_pBarLeftTop != NULL  && !m_pBarLeftTop->IsPaneVisible()) &&
		(m_pLeftContainer == NULL || m_pLeftContainer->IsEmpty() || bCheckVisibility && m_pLeftContainer != NULL && !m_pLeftContainer->IsVisible()));
}

BOOL CPaneContainer::IsRightPartEmpty(BOOL bCheckVisibility) const
{
	ASSERT_VALID(this);
	return((m_pBarRightBottom == NULL || bCheckVisibility && m_pBarRightBottom != NULL && !m_pBarRightBottom->IsPaneVisible()) &&
		(m_pRightContainer == NULL || m_pRightContainer->IsEmpty() || bCheckVisibility && m_pRightContainer != NULL && !m_pRightContainer->IsVisible()));
}

BOOL CPaneContainer::IsVisible() const
{
	ASSERT_VALID(this);

	return(m_pBarLeftTop != NULL && m_pBarLeftTop->IsPaneVisible() || m_pBarRightBottom != NULL && m_pBarRightBottom->IsPaneVisible() ||
		m_pLeftContainer != NULL && m_pLeftContainer->IsVisible() || m_pRightContainer != NULL && m_pRightContainer->IsVisible());
}

void CPaneContainer::CheckPaneDividerVisibility()
{
	ASSERT_VALID(this);

	BOOL bLeftContainerVisible = FALSE;
	BOOL bRightContainerVisible = FALSE;
	BOOL bLeftBarVisible = m_pBarLeftTop != NULL && m_pBarLeftTop->IsPaneVisible();
	BOOL bRightBarVisible = m_pBarRightBottom != NULL && m_pBarRightBottom->IsPaneVisible();

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->CheckPaneDividerVisibility();
		bLeftContainerVisible = m_pLeftContainer->IsVisible();
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->CheckPaneDividerVisibility();
		bRightContainerVisible = m_pRightContainer->IsVisible();
	}

	if (m_pSlider == NULL)
	{
		return;
	}

	BOOL bShow = FALSE;
	if (bLeftBarVisible && bRightBarVisible || bLeftBarVisible && bRightContainerVisible ||
		bRightBarVisible && bLeftContainerVisible || bLeftContainerVisible && bRightContainerVisible)
	{
		bShow = TRUE;
	}

	m_pSlider->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
}

void CPaneContainer::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		if (m_pBarLeftTop != NULL)
		{
			int nBarID = m_pBarLeftTop->GetDlgCtrlID();
			if (nBarID != -1)
			{
				ar << nBarID;
			}
			else
			{
				SaveTabbedPane(ar, m_pBarLeftTop);
			}
		}
		else
		{
			ar << (int) 0;
		}

		if (m_pBarRightBottom != NULL)
		{
			int nBarID = m_pBarRightBottom->GetDlgCtrlID();
			if (nBarID != -1)
			{
				ar << nBarID;
			}
			else
			{
				SaveTabbedPane(ar, m_pBarRightBottom);
			}
		}
		else
		{
			ar << (int) 0;
		}

		if (m_pSlider != NULL)
		{
			ar << m_pSlider->GetDlgCtrlID();
			m_pSlider->Serialize(ar);
		}
		else
		{
			ar << (int) 0;
		}

		ar << (BOOL)(m_pLeftContainer != NULL);
		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->Serialize(ar);
		}

		ar << (BOOL)(m_pRightContainer != NULL);
		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->Serialize(ar);
		}
	}
	else
	{
		ar >> m_nSavedLeftBarID;

		if (m_nSavedLeftBarID == -1)
		{
			m_pBarLeftTop = LoadTabbedPane(ar, m_lstSavedSiblingBarIDsLeft);
		}

		ar >> m_nSavedRightBarID;

		if (m_nSavedRightBarID == -1)
		{
			m_pBarRightBottom = LoadTabbedPane(ar, m_lstSavedSiblingBarIDsRight);
		}

		ar >> m_nSavedSliderID;

		if (m_nSavedSliderID != NULL)
		{
			m_pSlider = DYNAMIC_DOWNCAST(CPaneDivider, CPaneDivider::m_pSliderRTC->CreateObject());
			ASSERT_VALID(m_pSlider);

			m_pSlider->Init(FALSE, m_pContainerManager->m_pDockSite);
			m_pSlider->Serialize(ar);
			m_pSlider->SetPaneContainerManager(m_pContainerManager);
			m_pContainerManager->m_lstSliders.AddTail(m_pSlider);
		}

		BOOL bLeftContainerPresent = FALSE;
		ar >> bLeftContainerPresent;

		CRuntimeClass* pContainerRTC = m_pContainerManager->GetPaneContainerRTC();

		if (bLeftContainerPresent)
		{
			if (pContainerRTC == NULL)
			{
				m_pLeftContainer = new CPaneContainer(m_pContainerManager);
			}
			else
			{
				m_pLeftContainer = (CPaneContainer*) pContainerRTC->CreateObject();
				m_pLeftContainer->SetPaneContainerManager(m_pContainerManager);
			}
			m_pLeftContainer->Serialize(ar);
			m_pLeftContainer->SetParentPaneContainer(this);
		}

		BOOL bRightContainerPresent = FALSE;
		ar >> bRightContainerPresent;

		if (bRightContainerPresent)
		{
			if (pContainerRTC == NULL)
			{
				m_pRightContainer = new CPaneContainer(m_pContainerManager);
			}
			else
			{
				m_pRightContainer = (CPaneContainer*) pContainerRTC->CreateObject();
				m_pRightContainer->SetPaneContainerManager(m_pContainerManager);
			}

			m_pRightContainer->Serialize(ar);
			m_pRightContainer->SetParentPaneContainer(this);
		}
	}
}

BOOL CPaneContainer::SetUpByID(UINT nID, CDockablePane* pBar)
{
	ASSERT_KINDOF(CDockablePane, pBar);
	if (m_nSavedLeftBarID == nID)
	{
		m_pBarLeftTop = pBar;
		return TRUE;
	}

	if (m_nSavedRightBarID == nID)
	{
		m_pBarRightBottom = pBar;
		return TRUE;
	}

	if (m_pLeftContainer != NULL &&
		m_pLeftContainer->SetUpByID(nID, pBar))
	{
		return TRUE;
	}

	if (m_pRightContainer != NULL)
	{
		return m_pRightContainer->SetUpByID(nID, pBar);
	}

	return FALSE;
}

void CPaneContainer::SaveTabbedPane(CArchive& ar, CDockablePane* pBar)
{
	ASSERT_KINDOF(CBaseTabbedPane, pBar);
	CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pBar);
	ASSERT(ar.IsStoring());

	if (pTabbedBar->GetTabsNum() > 0)
	{
		ar << (int) -1;
		pTabbedBar->SaveSiblingBarIDs(ar);
		ar << pTabbedBar;
		ar << pTabbedBar->GetStyle();

		pTabbedBar->SerializeTabWindow(ar);
	}
}

CDockablePane* CPaneContainer::LoadTabbedPane(CArchive& ar, CList<UINT, UINT>& lstBarIDs)
{
	ASSERT(ar.IsLoading());

	CDockablePane* pBar = NULL;
	DWORD dwStyle = 0;

	CBaseTabbedPane::LoadSiblingPaneIDs(ar, lstBarIDs);
	ar >> pBar;
	ar >> dwStyle;

	if (!pBar->Create(_T(""), m_pContainerManager->m_pDockSite, pBar->m_rectSavedDockedRect, TRUE, (UINT) -1, dwStyle, pBar->GetControlBarStyle()))
	{
		TRACE0("Failed to create tab docking bar");
		ASSERT(FALSE);
		lstBarIDs.RemoveAll();
		delete pBar;
		return NULL;
	}

	ASSERT_KINDOF(CBaseTabbedPane, pBar);
	((CBaseTabbedPane*)pBar)->SerializeTabWindow(ar);
	((CBaseTabbedPane*)pBar)->SetAutoDestroy(TRUE);

	return pBar;
}

CDockablePane* CPaneContainer::FindTabbedPane(UINT nID)
{
	ASSERT_VALID(this);

	if (m_lstSavedSiblingBarIDsLeft.Find(nID) != NULL)
	{
		return m_pBarLeftTop;
	}

	if (m_lstSavedSiblingBarIDsRight.Find(nID) != NULL)
	{
		return m_pBarRightBottom;
	}

	if (m_pLeftContainer != NULL)
	{
		CDockablePane* pBar = m_pLeftContainer->FindTabbedPane(nID);
		if (pBar != NULL)
		{
			return pBar;
		}
	}

	if (m_pRightContainer != NULL)
	{
		return m_pRightContainer->FindTabbedPane(nID);
	}

	return NULL;
}

CList<UINT, UINT>* CPaneContainer::GetAssociatedSiblingPaneIDs(CDockablePane* pBar)
{
	if (pBar == m_pBarLeftTop)
	{
		return &m_lstSavedSiblingBarIDsLeft;
	}

	if (pBar == m_pBarRightBottom)
	{
		return &m_lstSavedSiblingBarIDsRight;
	}
	return NULL;
}

void CPaneContainer::SetPaneContainerManager(CPaneContainerManager* p, BOOL bDeep)
{
	m_pContainerManager = p;

	if (bDeep)
	{
		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->SetPaneContainerManager(p, bDeep);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->SetPaneContainerManager(p, bDeep);
		}
	}
}

int  CPaneContainer::GetNodeCount() const
{
	int nCount = 1;

	if (m_pLeftContainer != NULL)
	{
		nCount += m_pLeftContainer->GetNodeCount();
	}

	if (m_pRightContainer != NULL)
	{
		nCount += m_pRightContainer->GetNodeCount();
	}

	return nCount;
}

CPaneContainer* CPaneContainer::Copy(CPaneContainer* pParentContainer)
{
	// we should copy container and pointers to contained bars
	// only if these bars are visible;
	// unvisible parts of the new container shold be cleared
	CRuntimeClass* pContainerRTC = m_pContainerManager->GetPaneContainerRTC();
	CPaneContainer* pNewContainer = NULL;

	if (pContainerRTC == NULL)
	{
		pNewContainer = new CPaneContainer(m_pContainerManager, m_pBarLeftTop, m_pBarRightBottom, m_pSlider);
	}
	else
	{
		pNewContainer = (CPaneContainer*) pContainerRTC->CreateObject();
		pNewContainer->SetPaneContainerManager(m_pContainerManager);
		pNewContainer->SetPane(m_pBarLeftTop, TRUE);
		pNewContainer->SetPane(m_pBarRightBottom, FALSE);
		pNewContainer->SetPaneDivider(m_pSlider);
	}

	if (m_pBarLeftTop != NULL)
	{
		if (m_pBarLeftTop->GetStyle() & WS_VISIBLE)
		{
			m_pBarLeftTop = NULL;
		}
		else
		{
			pNewContainer->SetPane(NULL, TRUE);
		}
	}
	if (m_pBarRightBottom != NULL)
	{
		if (m_pBarRightBottom->GetStyle() & WS_VISIBLE)
		{
			m_pBarRightBottom = NULL;
		}
		else
		{
			pNewContainer->SetPane(NULL, FALSE);
		}
	}

	pNewContainer->SetParentPaneContainer(pParentContainer);

	if (m_pLeftContainer != NULL)
	{
		CPaneContainer* pNewLeftContainer = m_pLeftContainer->Copy(pNewContainer);
		pNewContainer->SetPaneContainer(pNewLeftContainer, TRUE);
	}

	if (m_pRightContainer != NULL)
	{

		CPaneContainer* pNewRightContainer = m_pRightContainer->Copy(pNewContainer);
		pNewContainer->SetPaneContainer(pNewRightContainer, FALSE);
	}

	if (m_pSlider != NULL)
	{
		if (m_pSlider->GetStyle() & WS_VISIBLE)
		{
			m_dwRecentSliderStyle = m_pSlider->GetPaneDividerStyle();
			m_pSlider->GetClientRect(m_rectRecentSlider);
			m_bIsRecentSliderHorz = m_pSlider->IsHorizontal();
			m_pSlider = NULL;
		}
		else
		{
			pNewContainer->SetPaneDivider(NULL);
		}
	}

	return pNewContainer;
}

BOOL CPaneContainer::IsPaneDividerHorz() const
{
	return m_pSlider != NULL ? m_pSlider->IsHorizontal() : m_bIsRecentSliderHorz;
}

int  CPaneContainer::GetTotalReferenceCount() const
{
	int nRefCount = m_dwRefCount;

	if (m_pRightContainer != NULL)
	{
		nRefCount += m_pRightContainer->GetTotalReferenceCount();
	}

	if (m_pLeftContainer != NULL)
	{
		nRefCount += m_pLeftContainer->GetTotalReferenceCount();
	}

	return nRefCount;
}

void CPaneContainer::RemoveNonValidPanes()
{
	if (m_pContainerManager == NULL)
	{
		return;
	}

	if (m_pBarLeftTop != NULL)
	{
		if (!m_pContainerManager->CheckAndRemoveNonValidPane(m_pBarLeftTop))
		{
			m_pBarLeftTop = NULL;
		}
	}

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->RemoveNonValidPanes();
	}

	if (m_pBarRightBottom != NULL)
	{
		if (!m_pContainerManager->CheckAndRemoveNonValidPane(m_pBarRightBottom))
		{
			m_pBarRightBottom = NULL;
		}
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->RemoveNonValidPanes();
	}
}

void CPaneContainer::OnShowPane(CDockablePane* pBar, BOOL bShow)
{
	if (bShow)
	{
		return;
	}

	CWnd* pDockSite = m_pContainerManager->GetDockSiteFrameWnd();
	ASSERT_VALID(pDockSite);

	CRect rectContainer;
	GetWindowRect(rectContainer, TRUE);
	pDockSite->ScreenToClient(rectContainer);

	if (m_pBarLeftTop != NULL && m_pBarLeftTop != pBar)
	{
		m_pBarLeftTop->SetWindowPos(NULL, rectContainer.left, rectContainer.top,
			rectContainer.Width(), rectContainer.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else if (m_pBarRightBottom != NULL && m_pBarRightBottom != pBar)
	{
		m_pBarRightBottom->SetWindowPos(NULL, rectContainer.left, rectContainer.top,
			rectContainer.Width(), rectContainer.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else if (m_pLeftContainer != NULL)
	{
		HDWP hdwp = NULL;
		m_pLeftContainer->Resize(rectContainer, hdwp, TRUE);
	}
	else if (m_pRightContainer != NULL)
	{
		HDWP hdwp = NULL;
		m_pRightContainer->Resize(rectContainer, hdwp, TRUE);
	}
}



