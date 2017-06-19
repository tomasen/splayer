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
#include "afxfullscreenimpl.h"
#include "afxmdiframewndex.h"
#include "afxmdichildwndex.h"
#include "afxframewndex.h"
#include "afxwinappex.h"
#include "afxtoolbar.h"
#include "afxpaneframewnd.h"
#include "afxdockablepane.h"
#include "afxdropdowntoolbar.h"
#include "afxbasetabbedpane.h"
#include "afxdocksite.h"
#include "afxribbonres.h"
#include "afxstatusbar.h"
#include "afxrebar.h"
#include "multimon.h"
#include "afxribbonbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CFullScreenToolbar : public CMFCToolBar
{
	virtual BOOL CanBeClosed() const
	{
		return FALSE;
	}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFullScreenImpl::CFullScreenImpl(CFrameImpl* pFrameImpl)
{
	m_pImpl = pFrameImpl;
	m_pwndFullScreenBar = NULL;
	m_bFullScreen = FALSE;
	m_bShowMenu = TRUE;
	m_bTabsArea = TRUE;
	m_uiFullScreenID = (UINT)-1;
	m_strRegSection = _T("");
}

CFullScreenImpl::~CFullScreenImpl()
{
}

void CFullScreenImpl::ShowFullScreen(CFrameWnd* pFrame)
{
	ASSERT(m_uiFullScreenID != -1);
	if (m_uiFullScreenID == -1)
	{
		return;
	}

	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp == NULL)
	{
		// Your application class should be derived from CWinAppEx
		ASSERT(FALSE);
		return;
	}

	CMDIFrameWndEx* pTabbedMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pFrame);
	if (pTabbedMDIFrame != NULL)
	{
		CMDIChildWndEx* pChildWnd = DYNAMIC_DOWNCAST(CMDIChildWndEx, pTabbedMDIFrame->MDIGetActive());
		if (pChildWnd != NULL)
		{
			CWnd* pViewWnd = pChildWnd->GetActiveView();
			if (pViewWnd == NULL && !pChildWnd->IsTabbedPane())
			{
				return;
			}
		}
	}
	else
	{
		CFrameWndEx* pFrameEx = DYNAMIC_DOWNCAST(CFrameWndEx, pFrame);
		if (pFrameEx == NULL)
		{
			return;
		}
	}

	CRect rectFrame, rectView, rectChild, rcScreen;

	pFrame->GetWindowRect(&rectFrame);
	m_rectFramePrev = rectFrame;

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromPoint(rectFrame.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rcScreen = mi.rcMonitor;

	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
	}

	m_strRegSection = pApp->GetRegistryBase();

	if (pTabbedMDIFrame != NULL)
	{
		pApp->SaveState(pTabbedMDIFrame, m_strRegSection);
		pApp->CleanState(_T("FullScreeenState"));
	}
	else
	{
		CFrameWndEx* pFrameEx = DYNAMIC_DOWNCAST(CFrameWndEx, pFrame);
		if (pFrameEx != NULL)
		{
			pApp->SaveState(pFrameEx, m_strRegSection);
			pApp->CleanState(_T("FullScreeenState"));
		}
		else
		{
			return;
		}
	}

	pFrame->SetRedraw(FALSE);
	UndockAndHidePanes(pFrame);

	CMDIChildWndEx* pChildWnd = NULL;
	if (pTabbedMDIFrame != NULL)
	{
		pChildWnd = DYNAMIC_DOWNCAST(CMDIChildWndEx, pTabbedMDIFrame->MDIGetActive());
	}

	if (pTabbedMDIFrame != NULL && pChildWnd != NULL)
	{
		CWnd* pViewWnd = NULL;
		if (pChildWnd->IsTabbedPane())
		{
			pViewWnd = pChildWnd->GetTabbedPane();
		}
		else
		{
			pViewWnd = pChildWnd->GetActiveView();
		}

		if (pViewWnd == NULL)
		{
			return;
		}

		pChildWnd->GetWindowRect(&rectView);
		pViewWnd->GetWindowRect(&rectChild);

		if (pTabbedMDIFrame->AreMDITabs())
		{
			if (m_bTabsArea)
			{
				CDockingManager* pDockMgr = m_pImpl->m_pDockManager;
				pDockMgr->AdjustDockingLayout();
				CRect rectClient;
				pTabbedMDIFrame->GetClientRect(&rectClient);
				pTabbedMDIFrame->ClientToScreen(&rectClient);

				rectFrame.InflateRect((rectClient.left - rcScreen.left), (rectClient.top - rcScreen.top) , (rcScreen.right - rectClient.right), rcScreen.bottom - rectClient.bottom);
			}
			else
			{
				rectFrame.InflateRect(rectChild.left - rcScreen.left, rectChild.top - rcScreen.top, rcScreen.right - rectChild.right, rcScreen.bottom - rectChild.bottom);
			}
		}
		else
		{
			rectFrame.InflateRect(rectChild.left - rcScreen.left,
				(rectChild.top - rcScreen.top) ,
				(rcScreen.right - rectChild.right), rcScreen.bottom - rectChild.bottom);
		}
	}
	else // Maybe SDI
	{
		CFrameWndEx* pFrameEx = DYNAMIC_DOWNCAST(CFrameWndEx, pFrame);

		if (pFrameEx != NULL)
		{
			CWnd* pViewWnd = pFrame->GetActiveView();
			if (pViewWnd == NULL)
			{
				CRect rectFrmClient;
				pFrameEx->GetClientRect(&rectFrmClient);
				pFrameEx->ClientToScreen(&rectFrmClient);

				rectFrame.InflateRect(rectFrmClient.left - rcScreen.left, rectFrmClient.top - rcScreen.top, rcScreen.right - rectFrmClient.right, rcScreen.bottom - rectFrmClient.bottom);
			}
			else
			{
				pViewWnd->GetWindowRect(&rectView);
				pFrameEx->GetWindowRect(&rectFrame);
				rectFrame.InflateRect(rectView.left - rcScreen.left, rectView.top - rcScreen.top, rcScreen.right - rectView.right, rcScreen.bottom - rectView.bottom);
			}
		}
	}

	if (pTabbedMDIFrame != NULL && pChildWnd == NULL)
	{
		CRect rectFrmClient;
		pTabbedMDIFrame->GetClientRect(&rectFrmClient);
		pTabbedMDIFrame->ClientToScreen(&rectFrmClient);

		rectFrame.InflateRect(rectFrmClient.left - rcScreen.left, rectFrmClient.top - rcScreen.top, rcScreen.right - rectFrmClient.right, rcScreen.bottom - rectFrmClient.bottom);
	}

	// Remember this for OnGetMinMaxInfo()
	m_rectFullScreenWindow = rectFrame;

	m_pwndFullScreenBar = new CFullScreenToolbar;

	if (!m_pwndFullScreenBar->Create(pFrame))
	{
		TRACE0("Failed to create toolbar\n");
		return;      // fail to create
	}

	CString strCaption;
	ENSURE(strCaption.LoadString(IDS_AFXBARRES_FULLSCREEN));

	CString strLabel;
	ENSURE(strLabel.LoadString(IDS_AFXBARRES_FULLSCREEN_CLOSE));

	CMFCToolBarButton button(m_uiFullScreenID, -1, strLabel, FALSE, TRUE);
	m_pwndFullScreenBar->InsertButton(button);
	m_pwndFullScreenBar->EnableDocking(0);
	m_pwndFullScreenBar->SetWindowPos(0, 100, 100, 100, 100, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	m_pwndFullScreenBar->SetWindowText(strCaption);
	m_pwndFullScreenBar->FloatPane(CRect(100, 100, 200, 200));
	m_bFullScreen = true;

	pFrame->SetWindowPos(NULL, rectFrame.left, rectFrame.top, rectFrame.Width(), rectFrame.Height(), SWP_NOZORDER);

	if (m_bShowMenu)
	{
		if (pTabbedMDIFrame != NULL)
		{
			const CMFCMenuBar* pMenuBar = pTabbedMDIFrame->GetMenuBar();

			if (pMenuBar != NULL)
			{
				if (m_bTabsArea)
				{
					if (pMenuBar->CanFloat())
					{
						pTabbedMDIFrame->DockPane((CBasePane*)pMenuBar);
					}
					pTabbedMDIFrame->ShowPane((CBasePane*)pMenuBar, TRUE,FALSE, FALSE);
				}
				else
				{
					if (pMenuBar->CanFloat())
					{
						pTabbedMDIFrame->DockPane((CBasePane*)pMenuBar);
						((CBasePane*)pMenuBar)->FloatPane(CRect(300, 200, 500, 500));
					}
					pTabbedMDIFrame->ShowPane((CBasePane*)pMenuBar, TRUE,FALSE, FALSE);
				}
			}
		}
		else
		{
			CFrameWndEx* pFrameEx = DYNAMIC_DOWNCAST(CFrameWndEx, pFrame);
			if (pFrameEx != NULL)
			{
				const CMFCMenuBar* pMenuBar = pFrameEx->GetMenuBar();

				if (pMenuBar != NULL)
				{
					if (pMenuBar->CanFloat())
					{
						pFrameEx->DockPane((CBasePane*)pMenuBar);
					}
					pFrameEx->ShowPane((CBasePane*)pMenuBar, TRUE,FALSE, FALSE);
				}
			}
		}
	}

	pFrame->SetRedraw(TRUE);
	pFrame->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	if (pTabbedMDIFrame != NULL && pTabbedMDIFrame->IsMDITabbedGroup())
	{
		pTabbedMDIFrame->AdjustClientArea();
	}
}

void CFullScreenImpl::RestoreState(CFrameWnd* pFrame)
{
	ASSERT(m_uiFullScreenID != -1);
	if (m_uiFullScreenID == -1)
	{
		return;
	}

	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp == NULL)
	{
		// Your application class should be derived from CWinAppEx
		ASSERT(FALSE);
		return;
	}

	// Destroy the toolbar
	CWnd* pWnd = m_pwndFullScreenBar->GetParentMiniFrame();
	if (pWnd == NULL)
		pWnd = m_pwndFullScreenBar;
	VERIFY(pWnd->DestroyWindow());

	delete m_pwndFullScreenBar;

	m_pwndFullScreenBar = NULL;
	m_bFullScreen = false;

	CMDIFrameWndEx* pTabbedMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pFrame);

	//Restore window pos
	pFrame->SetWindowPos(NULL, m_rectFramePrev.left, m_rectFramePrev.top, m_rectFramePrev.Width(), m_rectFramePrev.Height(), SWP_NOZORDER);

	//restore layout
	pFrame->SetRedraw(FALSE);

	pApp->m_bLoadUserToolbars = FALSE;

	if (pTabbedMDIFrame != NULL)
	{
		pApp->LoadState(pTabbedMDIFrame, m_strRegSection);
	}
	else
	{
		CFrameWndEx* pFrameEx = DYNAMIC_DOWNCAST(CFrameWndEx, pFrame);
		if (pFrameEx != NULL)
		{
			pApp->LoadState(pFrameEx, m_strRegSection);
		}
	}

	pFrame->SetRedraw(TRUE);
	pFrame->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	if (!m_bShowMenu)
	{
		if (pTabbedMDIFrame != NULL)
		{
			const CMFCMenuBar* pMenuBar = pTabbedMDIFrame->GetMenuBar();

			if (pMenuBar != NULL)
			{
				pTabbedMDIFrame->ShowPane((CBasePane*)pMenuBar, TRUE,FALSE, FALSE);
				if (pMenuBar->IsFloating())
				{
					pTabbedMDIFrame->SetFocus();
				}
			}
		}
		else
		{
			CFrameWndEx* pFrameEx = DYNAMIC_DOWNCAST(CFrameWndEx, pFrame);
			if (pFrameEx != NULL)
			{
				const CMFCMenuBar* pMenuBar = pFrameEx->GetMenuBar();
				if (pMenuBar != NULL)
				{
					pFrameEx->ShowPane((CBasePane*)pMenuBar, TRUE,FALSE, FALSE);
					if (pMenuBar->IsFloating())
					{
						pFrameEx->SetFocus();
					}
				}
			}
		}
	}

	if (pTabbedMDIFrame != NULL && pTabbedMDIFrame->IsMDITabbedGroup())
	{
		pTabbedMDIFrame->AdjustClientArea();
	}

	if (m_pImpl != NULL && m_pImpl->m_pRibbonBar != NULL && m_pImpl->m_pRibbonBar->IsWindowVisible() && m_pImpl->m_pRibbonBar->IsReplaceFrameCaption())
	{
		m_pImpl->OnChangeVisualManager();
	}
}

void CFullScreenImpl::ShowFullScreen()
{
	if (!m_bFullScreen)
	{
		ShowFullScreen(m_pImpl->m_pFrame);
	}
	else
	{
		RestoreState(m_pImpl->m_pFrame);
	}
}

void CFullScreenImpl::UndockAndHidePanes(CFrameWnd* pFrame)
{
	CMDIFrameWndEx* pTabbedMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pFrame);
	CFrameWndEx* pFrameEx = DYNAMIC_DOWNCAST(CFrameWndEx, pFrame);

	CDockingManager* pDockMgr = m_pImpl->m_pDockManager;

	if (pDockMgr == NULL)
	{
		return;
	}

	CObList list;
	pDockMgr->GetPaneList(list, TRUE);

	// UnDock and hide DockingControlBars

	POSITION pos;
	for (pos = list.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBarNext = (CBasePane*) list.GetNext(pos);

		if (!::IsWindow(pBarNext->m_hWnd))
		{
			continue;
		}

		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pBarNext);
		if (pBar != NULL)
		{
			if (pBar->IsAutoHideMode())
			{
				pBar->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
			}

			if (pBar->IsMDITabbed ())
			{
				continue;
			}

			if (pBar->IsTabbed())
			{
				CMFCBaseTabCtrl* pTabWnd = (CMFCBaseTabCtrl*) pBar->GetParent();
				CBaseTabbedPane* pTabBar = (CBaseTabbedPane*) pTabWnd->GetParent();
				pTabBar->DetachPane(pBar);
			}

			if (pBar->CanFloat())
			{
				pBar->FloatPane(CRect(300, 200, 500, 500));
			}

			if (pTabbedMDIFrame != NULL)
			{
				pTabbedMDIFrame->ShowPane(pBar, FALSE,FALSE, FALSE);

			}
			else if (pFrameEx != NULL)
			{
				pFrameEx->ShowPane(pBar, FALSE,FALSE, FALSE);
			}

		}
		else
		{
			CPane* pControlBar =  DYNAMIC_DOWNCAST(CPane, pBarNext);

			if (pControlBar != NULL)
			{
				if (pTabbedMDIFrame != NULL)
				{
					pTabbedMDIFrame->ShowPane(pControlBar, FALSE,FALSE, FALSE);
				}
				else if (pFrameEx != NULL)
				{
					pFrameEx->ShowPane(pControlBar, FALSE,FALSE, FALSE);
				}

				continue;
			}
		}
	}

	// UnDock and hide all Toolbars
	const CObList& afxAllToolBars = CMFCToolBar::GetAllToolbars();

	for (pos = afxAllToolBars.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(pos);
		ENSURE(pToolBar != NULL);

		if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			// Don't touch dropdown toolbars!
			if (!pToolBar->IsKindOf(RUNTIME_CLASS(CMFCDropDownToolBar)))
			{
				if (pToolBar->IsKindOf(RUNTIME_CLASS(CMFCMenuBar)))
				{
					if (pToolBar->CanFloat())
					{
						pToolBar->FloatPane(CRect(0, -1024, 0, -1024));

						if (pTabbedMDIFrame != NULL)
						{
							pTabbedMDIFrame->ShowPane(pToolBar, FALSE,FALSE, FALSE);
						}
						else if (pFrameEx != NULL)
						{
							pFrameEx->ShowPane(pToolBar, FALSE,FALSE, FALSE);
						}
					}
					continue;
				}

				// Don't touch toolbars resids on the DockingControlBars

				CWnd* pWnd = pToolBar->GetParent();
				if (pWnd->IsKindOf(RUNTIME_CLASS(CDockSite)) || pWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
				{
					if (pTabbedMDIFrame != NULL)
					{
						if (pToolBar->CanFloat())
						{
							pToolBar->FloatPane(CRect(300, 200, 500, 500));
						}

						pTabbedMDIFrame->ShowPane(pToolBar, FALSE,FALSE, FALSE);
					}
					else if (pFrameEx != NULL)
					{

						if (pToolBar->CanFloat())
						{
							pToolBar->FloatPane(CRect(300, 200, 500, 500));
						}
						pFrameEx->ShowPane(pToolBar, FALSE,FALSE, FALSE);
					}
				}
			}
		}
	}
}

void CFullScreenImpl::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (m_bFullScreen)
	{
		lpMMI->ptMaxSize.x = lpMMI->ptMaxTrackSize.x = m_rectFullScreenWindow.Width();
		lpMMI->ptMaxSize.y = lpMMI->ptMaxTrackSize.y = m_rectFullScreenWindow.Height();
	}
}



