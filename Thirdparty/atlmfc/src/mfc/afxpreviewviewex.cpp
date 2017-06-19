// This MFC Library source code supports the Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"
#include "afxpreviewviewex.h"
#include "afxstatusbar.h"
#include "afxribbonbar.h"
#include "afxribboncategory.h"
#include "afxribbonres.h"
#include "afxglobalutils.h"
#include "afxdockingmanager.h"
#include "afxmdichildwndex.h"

IMPLEMENT_DYNCREATE(CPreviewViewEx, CPreviewView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nSimplePaneIndex = 255;

/////////////////////////////////////////////////////////////////////////////
// CMFCPrintPreviewToolBar

IMPLEMENT_DYNAMIC(CMFCPrintPreviewToolBar, CMFCToolBar)

BEGIN_MESSAGE_MAP(CMFCPrintPreviewToolBar, CMFCToolBar)
	//{{AFX_MSG_MAP(CMFCPrintPreviewToolBar)
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMFCPrintPreviewToolBar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
	// Prevent print preview toolbar context menu appearing
}

INT_PTR CMFCPrintPreviewToolBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	return CMFCToolBar::OnToolHitTest(point, pTI);
}

void CMFCPrintPreviewToolBar::OnDestroy()
{
	CFrameWnd* pParentFrame = AFXGetParentFrame(this);
	ASSERT_VALID(pParentFrame);

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(pParentFrame);
	if (pDockManager != NULL)
	{
		pDockManager->RemovePaneFromDockManager(this, FALSE, FALSE, FALSE, NULL);
	}

	CMFCToolBar::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
// CPreviewViewEx

BOOL CPreviewViewEx::m_bScaleLargeImages = TRUE;

static int s_nPreviewViews = 0;

CPreviewViewEx::CPreviewViewEx()
{
	m_iPagesBtnIndex = -1;
	m_iOnePageImageIndex = -1;
	m_iTwoPageImageIndex = -1;
	m_pWndStatusBar = NULL;
	m_pWndRibbonBar = NULL;
	m_pNumPageButton = NULL;
	m_bIsStatusBarSimple = FALSE;
	m_nSimpleType = 0;
	m_nCurrentPage = 1;
	m_recentToolbarSize.cx = m_recentToolbarSize.cy = -1;
}

CPreviewViewEx::~CPreviewViewEx()
{
	if (--s_nPreviewViews == 0)
	{
		if (m_pWndStatusBar != NULL)
		{
			// Restore previous StatusBar state:
			m_pWndStatusBar->SetPaneText(nSimplePaneIndex, NULL);
		}

		if (m_pWndRibbonBar != NULL)
		{
			m_pWndRibbonBar->SetPrintPreviewMode(FALSE);
		}
	}
}

BEGIN_MESSAGE_MAP(CPreviewViewEx, CPreviewView)
	//{{AFX_MSG_MAP(CPreviewViewEx)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_UPDATE_COMMAND_UI(AFX_ID_PREVIEW_NUMPAGE, &CPreviewViewEx::OnUpdatePreviewNumPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreviewViewEx message handlers

int CPreviewViewEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPreviewView::OnCreate(lpCreateStruct) == -1)
		return -1;

	s_nPreviewViews++;

	ASSERT_VALID(m_pToolBar);

	CFrameWnd* pParentFrame = AFXGetParentFrame(this);
	ASSERT_VALID(pParentFrame);

	CFrameWnd* pToplevelFrame = pParentFrame;

	if (pToplevelFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		pToplevelFrame = pToplevelFrame->GetTopLevelFrame();
	}

	m_pWndRibbonBar = DYNAMIC_DOWNCAST(CMFCRibbonBar, pToplevelFrame->GetDlgItem(AFX_IDW_RIBBON_BAR));

	if (m_pWndRibbonBar != NULL)
	{
		if (s_nPreviewViews == 1)
		{
			m_pWndRibbonBar->SetPrintPreviewMode();
		}
	}
	else
	{
		const UINT uiToolbarHotID = afxGlobalData.Is32BitIcons() ? IDR_AFXRES_PRINT_PREVIEW32 : 0;

		if (!m_wndToolBar.Create(m_pToolBar) || !m_wndToolBar.LoadToolBar( IDR_AFXRES_PRINT_PREVIEW, 0, 0, TRUE /* Locked */, 0, 0, uiToolbarHotID))
		{
			TRACE0("Failed to create print preview toolbar\n");
			return FALSE;      // fail to create
		}

		m_wndToolBar.SetOwner(this);

		// Remember One Page/Two pages image indexes:
		m_iPagesBtnIndex = m_wndToolBar.CommandToIndex(AFX_ID_PREVIEW_NUMPAGE);
		ASSERT(m_iPagesBtnIndex >= 0);

		CMFCToolBarButton* pButton= m_wndToolBar.GetButton(m_iPagesBtnIndex);
		ASSERT_VALID(pButton);

		m_iOnePageImageIndex = pButton->GetImage();

		int iIndex = m_wndToolBar.CommandToIndex(ID_AFXRES_TWO_PAGES_DUMMY);
		ASSERT(iIndex >= 0);

		pButton= m_wndToolBar.GetButton(iIndex);
		ASSERT_VALID(pButton);

		m_iTwoPageImageIndex = pButton->GetImage();

		// Remove dummy "Two pages" button:
		m_wndToolBar.RemoveButton(iIndex);

		// Set "Print" button to image + text:
		m_wndToolBar.SetToolBarBtnText(m_wndToolBar.CommandToIndex(AFX_ID_PREVIEW_PRINT));

		// Set "Close" button to text only:
		m_wndToolBar.SetToolBarBtnText(m_wndToolBar.CommandToIndex(AFX_ID_PREVIEW_CLOSE), NULL, TRUE, FALSE);

		CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(pParentFrame);
		ASSERT_VALID(pDockManager);
		pDockManager->AddPane(&m_wndToolBar, FALSE);

		// Change the Toolbar size:
		if (!m_bScaleLargeImages && m_wndToolBar.m_bLargeIcons)
		{
			m_wndToolBar.m_sizeCurButtonLocked = m_wndToolBar.m_sizeButtonLocked;
			m_wndToolBar.m_sizeCurImageLocked = m_wndToolBar.m_sizeImageLocked;
		}

		SetToolbarSize();
	}

	// Set Application Status Bar to Simple Text:
	m_pWndStatusBar = DYNAMIC_DOWNCAST(CMFCStatusBar, pToplevelFrame->GetDlgItem(AFX_IDW_STATUS_BAR));

	if (m_pWndStatusBar != NULL)
	{
		if (s_nPreviewViews == 1)
		{
			// Set Simple Pane Style to No Borders:
			m_pWndStatusBar->SetPaneText(nSimplePaneIndex, NULL);
		}
	}

	return 0;
}

void CPreviewViewEx::OnUpdatePreviewNumPage(CCmdUI *pCmdUI)
{
	CPreviewView::OnUpdateNumPageChange(pCmdUI);

	// Change the Icon of AFX_ID_PREVIEW_NUMPAGE button:
	UINT nPages = m_nZoomState == ZOOM_OUT ? m_nPages : m_nZoomOutPages;

	if (m_pWndRibbonBar != NULL)
	{

		ASSERT_VALID(m_pWndRibbonBar);

		if (m_pNumPageButton == NULL)
		{
			m_pNumPageButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, m_pWndRibbonBar->GetActiveCategory()->FindByID(AFX_ID_PREVIEW_NUMPAGE));
		}

		if (m_pNumPageButton != NULL)
		{
			ASSERT_VALID(m_pNumPageButton);

			int nImageIndex = nPages == 1 ? 5 : 4;

			if (m_pNumPageButton->GetImageIndex(TRUE) != nImageIndex)
			{
				m_pNumPageButton->SetImageIndex(nImageIndex, TRUE);
				m_pNumPageButton->SetKeys(nPages == 1 ? _T("2") : _T("1"));
				m_pNumPageButton->Redraw();
			}
		}
	}
	else if (m_wndToolBar.GetSafeHwnd() != NULL)
	{
		CMFCToolBarButton* pButton = m_wndToolBar.GetButton(m_iPagesBtnIndex);
		ASSERT_VALID(pButton);

		pButton->SetImage(nPages == 1 ? m_iTwoPageImageIndex : m_iOnePageImageIndex);

		m_wndToolBar.InvalidateRect(pButton->Rect());
	}
}

void CPreviewViewEx::OnDisplayPageNumber(UINT nPage, UINT nPagesDisplayed)
{
	ENSURE(m_pPreviewInfo != NULL);

	CFrameWnd* pParentFrame = AFXGetParentFrame(this);
	ASSERT_VALID(pParentFrame);

	int nSubString = (nPagesDisplayed == 1) ? 0 : 1;

	CString s;
	if (AfxExtractSubString(s, m_pPreviewInfo->m_strPageDesc, nSubString))
	{
		CString strPage;

		if (nSubString == 0)
		{
			strPage.Format(s, nPage);
		}
		else
		{
			UINT nEndPage = nPage + nPagesDisplayed - 1;
			strPage.Format(s, nPage, nEndPage);
		}

		if (m_pWndStatusBar != NULL)
		{
			m_pWndStatusBar->SetPaneText(nSimplePaneIndex, strPage);
		}
		else
		{
			pParentFrame->SendMessage(WM_SETMESSAGESTRING, 0,
				(LPARAM)(LPCTSTR) strPage);
		}
	}
	else
	{
		TRACE1("Malformed Page Description string. Could not get string %d.\n", nSubString);
	}
}

void AFXPrintPreview(CView* pView)
{
	ASSERT_VALID(pView);

	CPrintPreviewState *pState= new CPrintPreviewState;

	if (!pView->DoPrintPreview(IDD_AFXBAR_RES_PRINT_PREVIEW, pView, RUNTIME_CLASS(CPreviewViewEx), pState))
	{
		TRACE0("Error: OnFilePrintPreview failed.\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		delete pState;      // preview failed to initialize, delete State now
	}
}

void CPreviewViewEx::OnSize(UINT nType, int cx, int cy)
{
	CPreviewView::OnSize(nType, cx, cy);

	// Change the Toolbar size:
	SetToolbarSize();
}

void CPreviewViewEx::SetToolbarSize()
{
	if (m_wndToolBar.GetSafeHwnd() == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pToolBar);

	CSize szSize = m_wndToolBar.CalcFixedLayout(TRUE, TRUE);

	// Print toolbar should occupy the whole width of the mainframe(Win9x):
	CFrameWnd* pParent = AFXGetParentFrame(this);
	ASSERT_VALID(pParent);

	CRect rectParent;
	pParent->GetClientRect(rectParent);
	szSize.cx = rectParent.Width();

	CRect rectToolBar;
	m_wndToolBar.GetWindowRect(rectToolBar);
	pParent->ScreenToClient(rectToolBar);

	m_pToolBar->SetWindowPos(NULL, rectToolBar.left, rectToolBar.top, szSize.cx, szSize.cy, SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, 0, 0, szSize.cx, szSize.cy, SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOZORDER);

	// Adjust parent toolbar(actually - dialog bar) size:
	m_pToolBar->m_sizeDefault.cy = szSize.cy;

	if (m_recentToolbarSize == szSize)
	{
		return;
	}

	m_recentToolbarSize = szSize;

	pParent->RecalcLayout();            // position and size everything
	pParent->UpdateWindow();
}

BOOL CPreviewViewEx::OnEraseBkgnd(CDC* pDC)
{
	ASSERT_VALID(pDC);

	CRect rectClient;
	GetClientRect(rectClient);

	if (CMFCVisualManager::GetInstance()->OnEraseMDIClientArea(pDC, rectClient))
	{
		return TRUE;
	}

	return CPreviewView::OnEraseBkgnd(pDC);
}



