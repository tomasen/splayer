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
#include "afxrecentdocksiteinfo.h"

#include "afxpanecontainermanager.h"
#include "afxpanecontainer.h"
#include "afxdocksite.h"
#include "afxdockingpanesrow.h"
#include "afxpanedivider.h"
#include "afxpaneframewnd.h"
#include "afxdockablepane.h"
#include "afxbasetabctrl.h"
#include "afxbasetabbedpane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRecentPaneContainerInfo::CRecentPaneContainerInfo()
{
	Init();
}

CRecentPaneContainerInfo::~CRecentPaneContainerInfo()
{
}

void CRecentPaneContainerInfo::Init()
{
	m_pRecentBarContainer = NULL;
	m_rectDockedRect.SetRect(0, 0, 30, 30);
	m_nRecentPercent = 50;
	m_bIsRecentLeftBar = TRUE;
	m_pRecentContainerOfTabWnd = NULL;
}

void CRecentPaneContainerInfo::StoreDockInfo(CPaneContainer* pRecentContainer, CDockablePane* pBar, CDockablePane* pTabbedBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pRecentContainer);

	try
	{
		if (pRecentContainer == m_pRecentBarContainer && m_pRecentBarContainer->m_dwRefCount == 0 || pRecentContainer->m_dwRefCount < 0)
		{
			//ASSERT(FALSE);
		}

		if (pRecentContainer != NULL)
		{
			pRecentContainer->AddRef();
			m_bIsRecentLeftBar = (pTabbedBar != NULL) ? pRecentContainer->IsLeftPane(pTabbedBar) : pRecentContainer->IsLeftPane(pBar);
		}

		if (m_pRecentBarContainer != NULL && !m_pRecentBarContainer->IsDisposed())
		{
			CPaneContainerManager* pManager = m_pRecentBarContainer->m_pContainerManager;
			m_pRecentBarContainer->m_dwRefCount--;

			//m_pRecentBarContainer->Release();
			if (m_pRecentBarContainer->m_dwRefCount <= 0)
			{
				pManager->m_pRootContainer->ReleaseEmptyPaneContainer();
			}
			m_pRecentBarContainer = NULL;
		}

		if (m_pRecentContainerOfTabWnd != NULL && !m_pRecentContainerOfTabWnd->IsDisposed())
		{
			CPaneContainerManager* pManager = m_pRecentContainerOfTabWnd->m_pContainerManager;
			m_pRecentContainerOfTabWnd->m_dwRefCount--;
			if (m_pRecentContainerOfTabWnd->m_dwRefCount <= 0)
			{
				pManager->m_pRootContainer->ReleaseEmptyPaneContainer();
			}

			m_pRecentContainerOfTabWnd = NULL;
		}

		pBar->GetWindowRect(m_rectDockedRect);
		if (pTabbedBar == NULL)
		{
			m_pRecentBarContainer = pRecentContainer;
		}
		else
		{
			m_pRecentContainerOfTabWnd = pRecentContainer;
		}

		m_nRecentPercent = (pTabbedBar != NULL) ? pTabbedBar->GetLastPercentInPaneContainer() : pBar->GetLastPercentInPaneContainer();

		if (m_pRecentBarContainer != NULL && m_pRecentBarContainer->GetRefCount() == 0 || m_pRecentContainerOfTabWnd != NULL && m_pRecentContainerOfTabWnd->GetRefCount() == 0)
		{
			//ASSERT(FALSE);
		}
	}
	catch(...)
	{
	}
}

void CRecentPaneContainerInfo::SetInfo(CRecentPaneContainerInfo& srcInfo)
{
	if (srcInfo.m_pRecentBarContainer != NULL)
	{
		srcInfo.m_pRecentBarContainer->AddRef();
	}

	if (m_pRecentBarContainer != NULL)
	{
		m_pRecentBarContainer->Release();
	}

	m_pRecentBarContainer = srcInfo.m_pRecentBarContainer;
	m_rectDockedRect = srcInfo.m_rectDockedRect;
	m_nRecentPercent = srcInfo.m_nRecentPercent;

	if (srcInfo.m_pRecentContainerOfTabWnd != NULL)
	{
		srcInfo.m_pRecentContainerOfTabWnd->AddRef();
	}

	if (m_pRecentContainerOfTabWnd != NULL)
	{
		m_pRecentContainerOfTabWnd->Release();
	}
	m_pRecentContainerOfTabWnd = srcInfo.m_pRecentContainerOfTabWnd;

	m_lstRecentListOfBars.RemoveAll();
	m_lstRecentListOfBars.AddTail(&srcInfo.m_lstRecentListOfBars);
}

// CRecentDockSiteInfo implementation
CRecentDockSiteInfo::CRecentDockSiteInfo(CPane* pBar)
{
	m_pBar = pBar;
	Init();
}

CRecentDockSiteInfo::~CRecentDockSiteInfo()
{
}

void CRecentDockSiteInfo::Init()
{
	m_rectRecentFloatingRect.SetRect(10, 10, 110, 110);

	m_nRecentRowIndex = 0;
	m_pRecentDockBar = NULL;
	m_pRecentDockBarRow = NULL;
	m_nRecentTabNumber = -1;
	m_hRecentDefaultSlider = NULL;
	m_hRecentMiniFrame = NULL;
	m_dwRecentAlignmentToFrame = CBRS_ALIGN_LEFT;
}

void CRecentDockSiteInfo::CleanUp()
{
	Init();
	m_recentSliderInfo.Init();
	m_recentMiniFrameInfo.Init();
}

void CRecentDockSiteInfo::StoreDockInfo(CPaneContainer* pRecentContainer, CDockablePane* pTabbedBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pBar);
	ASSERT_KINDOF(CDockablePane, m_pBar); // get here only for docking bars

	CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, m_pBar);

	CPaneDivider* pDefaultSlider = (pTabbedBar != NULL) ? pTabbedBar->GetDefaultPaneDivider() : pBar->GetDefaultPaneDivider();
	CPaneFrameWnd* pMiniFrame = pBar->GetParentMiniFrame();

	if (pMiniFrame != NULL)
	{
		CPaneFrameWnd* pRecentMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, CWnd::FromHandlePermanent(m_hRecentMiniFrame));

		m_hRecentMiniFrame = pMiniFrame->GetSafeHwnd();
		m_recentMiniFrameInfo.StoreDockInfo(pRecentContainer, pBar, pTabbedBar);
		pMiniFrame->ScreenToClient(m_recentMiniFrameInfo.m_rectDockedRect);
		pMiniFrame->GetWindowRect(m_rectRecentFloatingRect);

		if (pRecentMiniFrame != NULL)
		{
			pRecentMiniFrame->PostMessage(AFX_WM_CHECKEMPTYMINIFRAME);
		}
	}
	else if (pDefaultSlider != NULL)
	{
		m_recentSliderInfo.StoreDockInfo(pRecentContainer, pBar, pTabbedBar);
		pBar->GetDockSiteFrameWnd()->ScreenToClient(m_recentSliderInfo.m_rectDockedRect);

		m_hRecentDefaultSlider = pDefaultSlider->GetSafeHwnd();
		m_dwRecentAlignmentToFrame = pDefaultSlider->GetCurrentAlignment();
	}
	else
	{
		m_hRecentMiniFrame = NULL;
		m_recentMiniFrameInfo.StoreDockInfo(NULL, pBar);
	}
}

CPaneContainer* CRecentDockSiteInfo::GetRecentPaneContainer(BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_pRecentBarContainer : m_recentMiniFrameInfo.m_pRecentBarContainer;
}

CPaneContainer* CRecentDockSiteInfo::GetRecentTabContainer(BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_pRecentContainerOfTabWnd:
		m_recentMiniFrameInfo.m_pRecentContainerOfTabWnd;
}

CRect& CRecentDockSiteInfo::GetRecentDockedRect(BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_rectDockedRect : m_recentMiniFrameInfo.m_rectDockedRect;
}

int CRecentDockSiteInfo::GetRecentDockedPercent(BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_nRecentPercent : m_recentMiniFrameInfo.m_nRecentPercent;
}

BOOL CRecentDockSiteInfo::IsRecentLeftPane(BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_bIsRecentLeftBar : m_recentMiniFrameInfo.m_bIsRecentLeftBar;
}

void CRecentDockSiteInfo::SaveListOfRecentPanes(CList<HWND, HWND>& lstOrg, BOOL bForSlider)
{
	if (bForSlider)
	{
		m_recentSliderInfo.m_lstRecentListOfBars.RemoveAll();
		m_recentSliderInfo.m_lstRecentListOfBars.AddTail(&lstOrg);
	}
	else
	{
		m_recentMiniFrameInfo.m_lstRecentListOfBars.RemoveAll();
		m_recentMiniFrameInfo.m_lstRecentListOfBars.AddTail(&lstOrg);
	}
}

CList<HWND,HWND>& CRecentDockSiteInfo::GetRecentListOfPanes(BOOL bForSlider)
{
	return  bForSlider ? m_recentSliderInfo.m_lstRecentListOfBars : m_recentMiniFrameInfo.m_lstRecentListOfBars;
}

CPaneDivider* CRecentDockSiteInfo::GetRecentDefaultPaneDivider()
{
	return DYNAMIC_DOWNCAST(CPaneDivider, CWnd::FromHandlePermanent(m_hRecentDefaultSlider));
}

void CRecentDockSiteInfo::SetInfo(BOOL bForSlider, CRecentDockSiteInfo& srcInfo)
{
	if (bForSlider)
	{
		m_dwRecentAlignmentToFrame = srcInfo.m_dwRecentAlignmentToFrame;
		m_hRecentDefaultSlider = srcInfo.m_hRecentDefaultSlider;
		m_recentSliderInfo.SetInfo(srcInfo.m_recentSliderInfo);
	}
	else
	{
		m_rectRecentFloatingRect = srcInfo.m_rectRecentFloatingRect;
		m_hRecentMiniFrame = srcInfo.m_hRecentMiniFrame;
		m_recentMiniFrameInfo.SetInfo(srcInfo.m_recentMiniFrameInfo);
	}
}

CRecentDockSiteInfo& CRecentDockSiteInfo::operator= (CRecentDockSiteInfo& src)
{
	m_rectRecentFloatingRect = src.m_rectRecentFloatingRect;
	m_dwRecentAlignmentToFrame = src.m_dwRecentAlignmentToFrame;
	m_nRecentRowIndex = src.m_nRecentRowIndex;
	m_pRecentDockBar = src.m_pRecentDockBar;
	m_pRecentDockBarRow = src.m_pRecentDockBarRow;
	m_nRecentTabNumber = src.m_nRecentTabNumber;
	m_hRecentDefaultSlider = src.m_hRecentDefaultSlider;
	m_hRecentMiniFrame = src.m_hRecentMiniFrame;
	m_recentSliderInfo = src.m_recentSliderInfo;
	m_recentMiniFrameInfo = src.m_recentMiniFrameInfo;

	return *this;
}

CRecentPaneContainerInfo& CRecentPaneContainerInfo::operator= (CRecentPaneContainerInfo& src)
{
	m_pRecentBarContainer = src.m_pRecentBarContainer;
	m_rectDockedRect = src.m_rectDockedRect;
	m_nRecentPercent = src.m_nRecentPercent;
	m_bIsRecentLeftBar = src.m_bIsRecentLeftBar;
	m_pRecentContainerOfTabWnd = src.m_pRecentContainerOfTabWnd;

	m_lstRecentListOfBars.RemoveAll();
	m_lstRecentListOfBars.AddTail(&src.m_lstRecentListOfBars);

	return *this;
}



