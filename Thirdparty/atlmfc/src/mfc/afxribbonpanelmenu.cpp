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
#include "afxtrackmouse.h"
#include "afxribbonpanelmenu.h"
#include "afxribbonpanel.h"
#include "afxribboncategory.h"
#include "afxribbonbar.h"
#include "afxvisualmanager.h"
#include "afxtooltipmanager.h"
#include "afxtooltipctrl.h"
#include "afxribbonpalettegallery.h"
#include "afxribbonminitoolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nPopupTimerEvent = 1;
static const int nRemovePopupTimerEvent = 2;
static const UINT IdAutoCommand = 3;
static const int nScrollBarID = 1;

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonPanelMenuBar window

IMPLEMENT_DYNAMIC(CMFCRibbonPanelMenuBar, CMFCPopupMenuBar)

#pragma warning(disable : 4355)

CMFCRibbonPanelMenuBar::CMFCRibbonPanelMenuBar(CMFCRibbonPanel* pPanel)
{
	ASSERT_VALID(pPanel);

	m_pPanel = DYNAMIC_DOWNCAST(CMFCRibbonPanel, pPanel->GetRuntimeClass()->CreateObject());
	ASSERT_VALID(m_pPanel);

	m_pPanel->CopyFrom(*pPanel);

	CommonInit();

	m_pPanelOrigin = pPanel;

	m_pPanel->m_pParentMenuBar = this;
	m_pPanel->m_btnLaunch.SetParentMenu(this);

	for (int i = 0; i < m_pPanel->m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_pPanel->m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->SetParentMenu(this);
	}

	m_pRibbonBar = m_pPanel->m_pParent->GetParentRibbonBar();
}

CMFCRibbonPanelMenuBar::CMFCRibbonPanelMenuBar(CMFCRibbonBar* pRibbonBar, const CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arButtons, BOOL bIsFloatyMode)
{
	m_pPanel = new CMFCRibbonPanel;

	CommonInit();
	AddButtons(pRibbonBar, arButtons, bIsFloatyMode);
}

CMFCRibbonPanelMenuBar::CMFCRibbonPanelMenuBar(CMFCRibbonGallery* pPaletteButton)
{
	ASSERT_VALID(pPaletteButton);

	m_pPanel = new CMFCRibbonPanel(pPaletteButton);

	CommonInit();

	// Create array without scroll buttons:
	CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> arButtons;
	pPaletteButton->GetMenuItems(arButtons);

	AddButtons(pPaletteButton->GetTopLevelRibbonBar(), arButtons, FALSE);
}

CMFCRibbonPanelMenuBar::CMFCRibbonPanelMenuBar(CMFCRibbonCategory* pCategory, CSize size)
{
	ASSERT_VALID(pCategory);

	m_pPanel = NULL;

	CommonInit();

	m_pCategory = (CMFCRibbonCategory*) pCategory->GetRuntimeClass()->CreateObject();
	ASSERT_VALID(m_pCategory);

	m_pCategory->CopyFrom(*pCategory);
	m_pCategory->m_pParentMenuBar = this;

	for (int iPanel = 0; iPanel < m_pCategory->GetPanelCount(); iPanel++)
	{
		CMFCRibbonPanel* pPanel = m_pCategory->GetPanel(iPanel);
		ASSERT_VALID(pPanel);

		pPanel->m_pParentMenuBar = this;
		pPanel->m_btnLaunch.SetParentMenu(this);
		pPanel->m_btnDefault.SetParentMenu(this);

		for (int i = 0; i < pPanel->m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = pPanel->m_arElements [i];
			ASSERT_VALID(pElem);

			pElem->SetParentMenu(this);
		}
	}

	m_sizeCategory = size;
	m_pRibbonBar = m_pCategory->GetParentRibbonBar();
}

void CMFCRibbonPanelMenuBar::AddButtons(CMFCRibbonBar* pRibbonBar, const CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arButtons, BOOL bFloatyMode)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pRibbonBar);

	m_bSimpleMode = TRUE;
	m_pRibbonBar = pRibbonBar;

	m_pPanel->m_pParentMenuBar = this;
	m_pPanel->m_bFloatyMode = bFloatyMode;
	m_pPanel->m_nXMargin = 2;
	m_pPanel->m_nYMargin = 2;
	m_pPanel->RemoveAll();

	for (int i = 0; i < arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pSrcButton = arButtons [i];
		ASSERT_VALID(pSrcButton);

		CMFCRibbonBaseElement* pButton = (CMFCRibbonBaseElement*) pSrcButton->GetRuntimeClass()->CreateObject();
		ASSERT_VALID(pButton);

		pButton->CopyFrom(*pSrcButton);
		pButton->SetOriginal(pSrcButton);
		pButton->m_bCompactMode = TRUE;

		pButton->SetParentMenu(this);

		m_pPanel->Add(pButton);
	}
}

CMFCRibbonPanelMenuBar::CMFCRibbonPanelMenuBar()
{
	m_pPanel = new CMFCRibbonPanel;
	CommonInit();
}

void CMFCRibbonPanelMenuBar::CommonInit()
{
	if (m_pPanel != NULL)
	{
		ASSERT_VALID(m_pPanel);
		m_pPanel->m_pParentMenuBar = this;
	}

	m_pCategory = NULL;
	m_sizeCategory = CSize(0, 0);

	m_pDelayedCloseButton = NULL;
	m_pDelayedButton = NULL;
	m_pPressed = NULL;
	m_rectAutoCommand.SetRectEmpty();

	m_bSimpleMode = FALSE;
	m_bIsMenuMode = FALSE;
	m_bIsDefaultMenuLook = FALSE;
	m_pPanelOrigin = NULL;
	m_pRibbonBar = NULL;

	m_bTracked = FALSE;
	m_pToolTip = NULL;
	m_bDisableSideBarInXPMode = TRUE;

	m_sizePrefered = CSize(0, 0);
	m_bIsQATPopup = FALSE;
	m_bCustomizeMenu = TRUE;
	m_bIsFloaty = FALSE;
	m_bSetKeyTips = FALSE;
	m_bHasKeyTips = FALSE;
	m_bAutoCommandTimer = FALSE;

	m_ptStartMenu = CPoint(-1, -1);
}

#pragma warning(default : 4355)

CMFCRibbonPanelMenuBar::~CMFCRibbonPanelMenuBar()
{
	if (m_pPanel != NULL)
	{
		ASSERT_VALID(m_pPanel);

		if (m_pRibbonBar != NULL && m_pRibbonBar->GetKeyboardNavLevelCurrent() == m_pPanel)
		{
			m_pRibbonBar->DeactivateKeyboardFocus(FALSE);
		}

		delete m_pPanel;
	}

	if (m_pCategory != NULL)
	{
		ASSERT_VALID(m_pCategory);

		if (m_pRibbonBar != NULL && m_pRibbonBar->GetKeyboardNavLevelCurrent() == m_pCategory)
		{
			m_pRibbonBar->DeactivateKeyboardFocus(FALSE);
		}

		delete m_pCategory;

		if (m_pRibbonBar != NULL && m_pRibbonBar->GetActiveCategory() != NULL)
		{
			// Redraw ribbon tab:
			ASSERT_VALID(m_pRibbonBar);
			ASSERT_VALID(m_pRibbonBar->GetActiveCategory());

			if (!m_pRibbonBar->IsQuickAccessToolbarOnTop())
			{
				CMFCRibbonTab& tab = m_pRibbonBar->GetActiveCategory()->m_Tab;

				tab.m_bIsDroppedDown = FALSE;
				tab.m_bIsHighlighted = FALSE;

				CRect rectRedraw = tab.GetRect();
				rectRedraw.bottom = m_pRibbonBar->GetQuickAccessToolbarLocation().bottom;
				rectRedraw.InflateRect(1, 1);

				m_pRibbonBar->RedrawWindow(rectRedraw);
			}
		}
	}

	if (m_bHasKeyTips)
	{
		CWnd* pMenu = CMFCPopupMenu::GetActiveMenu();

		if (pMenu != NULL && CWnd::FromHandlePermanent(pMenu->GetSafeHwnd()) != NULL && pMenu->IsWindowVisible())
		{
			CMFCPopupMenu::UpdateAllShadows();
		}
	}
}

//{{AFX_MSG_MAP(CMFCRibbonPanelMenuBar)
BEGIN_MESSAGE_MAP(CMFCRibbonPanelMenuBar, CMFCPopupMenuBar)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCRibbonPanelMenuBar::OnMouseLeave)
	ON_REGISTERED_MESSAGE(AFX_WM_UPDATETOOLTIPS, &CMFCRibbonPanelMenuBar::OnUpdateToolTips)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CMFCRibbonPanelMenuBar::OnNeedTipText)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

void CMFCRibbonPanelMenuBar::AdjustLocations()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRibbonBar);

	if (m_bInUpdateShadow)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	CClientDC dc(this);

	CFont* pOldFont = dc.SelectObject(m_pRibbonBar->GetFont());
	ENSURE(pOldFont != NULL);

	if (m_pCategory != NULL)
	{
		ASSERT_VALID(m_pCategory);

		m_pCategory->m_rect = rectClient;
		m_pCategory->RecalcLayout(&dc);
	}
	else if (m_pPanel != NULL)
	{
		m_pPanel->m_bSizeIsLocked = m_bResizeTracking;
		m_pPanel->m_nScrollOffset = m_iOffset;

		m_pPanel->Reposition(&dc, rectClient);
		m_pPanel->OnAfterChangeRect(&dc);

		m_pPanel->m_bSizeIsLocked = FALSE;
	}

	dc.SelectObject(pOldFont);
}

void CMFCRibbonPanelMenuBar::SetPreferedSize(CSize size)
{
	ASSERT_VALID(this);

	CSize sizePalette(0, 0);

	if (m_pPanel != NULL)
	{
		ASSERT_VALID(m_pPanel);

		if (m_pPanel->m_pPaletteButton != NULL)
		{
			sizePalette = m_pPanel->GetPaltteMinSize();
			sizePalette.cx -= ::GetSystemMetrics(SM_CXVSCROLL) + 2;
		}
	}

	m_sizePrefered = CSize(max(size.cx, sizePalette.cx), size.cy);
}

CSize CMFCRibbonPanelMenuBar::CalcSize(BOOL /*bVertDock*/)
{
	ASSERT_VALID(this);

	if (m_pCategory != NULL)
	{
		ASSERT_VALID(m_pCategory);
		ASSERT(m_sizeCategory != CSize(0, 0));

		return m_sizeCategory;
	}

	ASSERT_VALID(m_pRibbonBar);
	ASSERT_VALID(m_pPanel);

	m_pPanel->m_bIsQATPopup = m_bIsQATPopup;

	CClientDC dc(m_pRibbonBar);

	CFont* pOldFont = dc.SelectObject(m_pRibbonBar->GetFont());
	ENSURE(pOldFont != NULL);

	if (m_bIsMenuMode)
	{
		m_pPanel->m_bMenuMode = TRUE;
		m_pPanel->m_bIsDefaultMenuLook = m_bIsDefaultMenuLook;

		m_pPanel->Reposition(&dc, CRect(0, 0, m_sizePrefered.cx, m_sizePrefered.cy));

		dc.SelectObject(pOldFont);

		CSize size = m_pPanel->m_rect.Size();

		if (m_sizePrefered != CSize(0, 0))
		{
			size.cx = max(m_sizePrefered.cx, size.cx);

			if (m_sizePrefered.cy <= 0)
			{
				size.cy = m_pPanel->m_rect.Size().cy;
			}
			else
			{
				if (m_pPanel->m_pPaletteButton != NULL)
				{
					size.cy = max(size.cy, m_sizePrefered.cy);
				}
				else
				{
					if (size.cy > m_sizePrefered.cy)
					{
						CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
						if (pParentMenu != NULL)
						{
							pParentMenu->m_bScrollable = TRUE;
						}
					}

					size.cy = m_sizePrefered.cy;
				}
			}
		}

		return size;
	}

	if (m_bSimpleMode && m_pPanel->m_arWidths.GetSize() == 0)
	{
		CWaitCursor wait;
		m_pPanel->RecalcWidths(&dc, 32767);
	}

	const int nWidthSize = (int) m_pPanel->m_arWidths.GetSize();
	if (nWidthSize == 0)
	{
		dc.SelectObject(pOldFont);
		return CSize(10, 10);
	}

	if (m_pPanel->m_bAlignByColumn && !m_pPanel->m_bFloatyMode && !m_pPanel->IsFixedSize())
	{
		const int nHeight = m_pRibbonBar->GetCategoryHeight() - 2 * m_pPanel->m_nYMargin;
		m_pPanel->Reposition(&dc, CRect(0, 0, 32767, nHeight));
	}
	else if (m_bIsQATPopup)
	{
		int nWidth = m_pPanel->m_arWidths [0] + 2 * m_pPanel->m_nXMargin;
		m_pPanel->Reposition(&dc, CRect(0, 0, nWidth, 32767));
	}
	else
	{
		int nWidth = 0;
		int nHeight = 0;

		if (!m_pPanel->m_bFloatyMode)
		{
			nWidth = m_pPanel->m_arWidths [0] + 4 * m_pPanel->m_nXMargin;
			nHeight = m_pRibbonBar->GetCategoryHeight() - 2 * m_pPanel->m_nYMargin;
		}
		else
		{
			nWidth = m_pPanel->m_arWidths [nWidthSize > 2 ? 1 : 0] + 4 * m_pPanel->m_nXMargin;
			nHeight = 32767;
		}

		m_pPanel->Reposition(&dc, CRect(0, 0, nWidth, nHeight));
	}

	CSize size = m_pPanel->m_rect.Size();
	dc.SelectObject(pOldFont);

	if (m_bSimpleMode && m_pPanel->GetCount() > 0 && !m_bIsQATPopup)
	{
		int xMin = 32767;
		int xMax = 0;

		int yMin = 32767;
		int yMax = 0;

		for (int i = 0; i < m_pPanel->GetCount(); i++)
		{
			CMFCRibbonBaseElement* pButton = m_pPanel->GetElement(i);
			ASSERT_VALID(pButton);

			CRect rectButton = pButton->GetRect();

			xMin = min(xMin, rectButton.left);
			yMin = min(yMin, rectButton.top);

			xMax = max(xMax, rectButton.right);
			yMax = max(yMax, rectButton.bottom);
		}

		return CSize(xMax - xMin + 2 * m_pPanel->m_nXMargin, yMax - yMin + 2 * m_pPanel->m_nYMargin);
	}

	return size;
}

void CMFCRibbonPanelMenuBar::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);
	ASSERT_VALID(m_pRibbonBar);

	CMemDC memDC(*pDCPaint, this);
	CDC* pDC = &memDC.GetDC();

	CRect rectClip;
	pDCPaint->GetClipBox(rectClip);

	CRgn rgnClip;

	if (!rectClip.IsRectEmpty())
	{
		rgnClip.CreateRectRgnIndirect(rectClip);
		pDC->SelectClipRgn(&rgnClip);
	}

	CFont* pOldFont = pDC->SelectObject(m_pRibbonBar->GetFont());
	ENSURE(pOldFont != NULL);

	pDC->SetBkMode(TRANSPARENT);

	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectFill = rectClient;
	rectFill.InflateRect(3, 3);

	if (m_pCategory != NULL)
	{
		ASSERT_VALID(m_pCategory);

		CMFCVisualManager::GetInstance()->OnDrawRibbonCategory(pDC, m_pCategory, rectFill);

		m_pCategory->OnDraw(pDC);
	}
	else
	{
		ASSERT_VALID(m_pPanel);

		if (m_pPanel->m_pParent != NULL)
		{
			CMFCRibbonCategory* pCategory = m_pPanel->m_pParent;
			ASSERT_VALID(pCategory);

			CMFCRibbonPanelMenuBar* pMenuBarSaved = pCategory->m_pParentMenuBar;
			pCategory->m_pParentMenuBar = this;

			CMFCVisualManager::GetInstance()->OnDrawRibbonCategory(pDC, pCategory, rectFill);

			pCategory->m_pParentMenuBar = pMenuBarSaved;
		}
		else if (m_bIsQATPopup)
		{
			CMFCVisualManager::GetInstance()->OnFillRibbonQuickAccessToolBarPopup(pDC, this, rectClient);
		}
		else
		{
			CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, this, rectClient, rectClient);
		}

		m_pPanel->DoPaint(pDC);
	}

	pDC->SelectObject(pOldFont);
	pDC->SelectClipRgn(NULL);
}

void CMFCRibbonPanelMenuBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CMFCPopupMenuBar::OnMouseMove(nFlags, point);

	if (m_pPanel != NULL && afxGlobalData.IsAccessibilitySupport())
	{
		int nIndex = m_pPanel->HitTestEx(point);
		if (nIndex != -1)
		{
			if (nIndex != m_iAccHotItem)
			{
				m_iAccHotItem = nIndex;
				SetTimer(AFX_ACCELERATOR_NOTIFY_EVENT, AFX_ACCELERATOR_TIMER_DELAY, NULL);
			}
		}
	}

	if (m_pPanel != NULL && !m_pPanel->m_bMenuMode && m_pPanel->GetDroppedDown() != NULL)
	{
		return;
	}

	if (m_pCategory != NULL && m_pCategory->GetDroppedDown() != NULL)
	{
		return;
	}

	if (m_ptStartMenu != CPoint(-1, -1))
	{
		// Check if mouse was moved from the menu start point:

		CPoint ptCursor;
		::GetCursorPos(&ptCursor);

		if (abs(ptCursor.x - m_ptStartMenu.x) < 10 && abs(ptCursor.y - m_ptStartMenu.y) < 10)
		{
			return;
		}

		m_ptStartMenu = CPoint(-1, -1);
	}

	if (point == CPoint(-1, -1))
	{
		m_bTracked = FALSE;
	}
	else if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::AFXTrackMouse(&trackmouseevent);

		CMFCRibbonBaseElement* pPressed = NULL;

		if (m_pCategory != NULL)
		{
		}
		else
		{
			pPressed = m_pPanel->GetPressed();
		}

		if (pPressed != NULL &&((nFlags & MK_LBUTTON) == 0))
		{
			ASSERT_VALID(pPressed);
			pPressed->m_bIsPressed = FALSE;
		}
	}

	if (m_pCategory != NULL)
	{
		m_pCategory->OnMouseMove(point);
	}
	else if (m_pPanel != NULL)
	{
		BOOL bWasHighlighted = m_pPanel->IsHighlighted();
		m_pPanel->Highlight(TRUE, point);

		if (!bWasHighlighted)
		{
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}
}

LRESULT CMFCRibbonPanelMenuBar::OnMouseLeave(WPARAM,LPARAM)
{
	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point);

	CRect rectClient;
	GetClientRect(rectClient);

	if (!rectClient.PtInRect(point))
	{
		OnMouseMove(0, CPoint(-1, -1));
		m_bTracked = FALSE;

		if (m_pPanel != NULL)
		{
			m_pPanel->Highlight(FALSE, CPoint(-1, -1));
		}

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}

	m_bTracked = FALSE;
	return 0;
}

void CMFCRibbonPanelMenuBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bAutoCommandTimer)
	{
		KillTimer(IdAutoCommand);
		m_bAutoCommandTimer = FALSE;
		m_pPressed = NULL;
		m_rectAutoCommand.SetRectEmpty();
	}

	HWND hwndThis = GetSafeHwnd();

	CMFCPopupMenuBar::OnLButtonUp(nFlags, point);

	if (m_pCategory != NULL)
	{
		m_pCategory->OnLButtonUp(point);
	}
	else
	{
		m_pPanel->MouseButtonUp(point);
	}

	if (::IsWindow(hwndThis))
	{
		::GetCursorPos(&point);
		ScreenToClient(&point);

		OnMouseMove(nFlags, point);
	}
}

void CMFCRibbonPanelMenuBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CMFCPopupMenuBar::OnLButtonDown(nFlags, point);

	CMFCRibbonBaseElement* pDroppedDown = GetDroppedDown();
	if (pDroppedDown != NULL)
	{
		ASSERT_VALID(pDroppedDown);
		pDroppedDown->ClosePopupMenu();
	}

	OnMouseMove(nFlags, point);

	m_pPressed = NULL;
	m_rectAutoCommand.SetRectEmpty();

	HWND hwndThis = GetSafeHwnd();

	CMFCRibbonBaseElement* pPressed = NULL;

	if (m_pCategory != NULL)
	{
		pPressed = m_pCategory->OnLButtonDown(point);
	}
	else
	{
		pPressed = m_pPanel->MouseButtonDown(point);
	}

	if (!::IsWindow(hwndThis))
	{
		return;
	}

	m_pPressed = pPressed;

	if (m_pPressed != NULL)
	{
		ASSERT_VALID(m_pPressed);

		int nDelay = 100;

		if (m_pPressed->IsAutoRepeatMode(nDelay))
		{
			SetTimer(IdAutoCommand, nDelay, NULL);
			m_bAutoCommandTimer = TRUE;
			m_rectAutoCommand = m_pPressed->GetRect();
		}
	}
}

void CMFCRibbonPanelMenuBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CMFCPopupMenuBar::OnLButtonDblClk(nFlags, point);

	if (IsRibbonPanelInRegularMode())
	{
		CMFCRibbonButton* pDroppedDown = GetDroppedDown();
		if (pDroppedDown != NULL)
		{
			pDroppedDown->ClosePopupMenu();
		}
	}

	CMFCRibbonBaseElement* pHit = HitTest(point);
	if (pHit != NULL)
	{
		ASSERT_VALID(pHit);

		pHit->OnLButtonDblClk(point);
	}
}

void CMFCRibbonPanelMenuBar::OnClickButton(CMFCRibbonButton* pButton, CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pButton);

	pButton->m_bIsHighlighted = pButton->m_bIsPressed = FALSE;
	RedrawWindow(pButton->GetRect());

	if (m_pPanel != NULL)
	{
		ASSERT_VALID(m_pPanel);

		if (m_pPanel->m_pPaletteButton != NULL)
		{
			ASSERT_VALID(m_pPanel->m_pPaletteButton);

			if (m_pPanel->m_pPaletteButton->OnClickPaletteSubItem(pButton, this))
			{
				return;
			}
		}
	}

	BOOL bInvoked = pButton->NotifyCommand(TRUE);

	if (IsRibbonMiniToolBar())
	{
		CMFCRibbonMiniToolBar* pFloaty = DYNAMIC_DOWNCAST(CMFCRibbonMiniToolBar, GetParent());

		if (pFloaty != NULL)
		{
			return;
		}
	}

	if (bInvoked)
	{
		CMFCRibbonPanelMenu* pPopupMenu = DYNAMIC_DOWNCAST (CMFCRibbonPanelMenu, GetParent ());
		if (pPopupMenu != NULL)
		{
			ASSERT_VALID(pPopupMenu);

			CMFCRibbonPanelMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCRibbonPanelMenu, pPopupMenu->GetParentPopupMenu ());
			if (pParentMenu != NULL)
			{
				ASSERT_VALID(pParentMenu);
				pParentMenu->m_bForceClose = TRUE;
			}
		}
	}

	CFrameWnd* pParentFrame = AFXGetParentFrame(this);
	ASSERT_VALID(pParentFrame);

	pParentFrame->DestroyWindow();
}

void CMFCRibbonPanelMenuBar::OnChangeHighlighted(CMFCRibbonBaseElement* pHot)
{
	ASSERT_VALID(this);

	if (m_pPanel == NULL || !m_pPanel->m_bMenuMode)
	{
		return;
	}

	CMFCRibbonButton* pDroppedDown = DYNAMIC_DOWNCAST(CMFCRibbonButton, m_pPanel->GetDroppedDown());
	CMFCRibbonButton* pHotButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, pHot);

	if (pDroppedDown != NULL && pHot == NULL)
	{
		return;
	}

	BOOL bHotWasChanged = pDroppedDown != pHot;

	if (pHotButton != NULL && pDroppedDown == pHotButton && !pHotButton->GetMenuRect().IsRectEmpty() && !pHotButton->IsMenuAreaHighlighted())
	{
		// Mouse moved away from the menu area, hide menu:
		bHotWasChanged = TRUE;
	}

	if (bHotWasChanged)
	{
		CMFCRibbonPanelMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCRibbonPanelMenu, GetParent());

		if (pDroppedDown != NULL)
		{
			const MSG* pMsg = GetCurrentMessage();

			if (CMFCToolBar::IsCustomizeMode() || (pMsg != NULL && pMsg->message == WM_KEYDOWN))
			{
				KillTimer(nRemovePopupTimerEvent);
				m_pDelayedCloseButton = NULL;

				pDroppedDown->ClosePopupMenu();

				if (pParentMenu != NULL)
				{
					CMFCPopupMenu::ActivatePopupMenu(AFXGetTopLevelFrame(this), pParentMenu);
				}
			}
			else
			{
				m_pDelayedCloseButton = pDroppedDown;
				m_pDelayedCloseButton->m_bToBeClosed = TRUE;

				SetTimer(nRemovePopupTimerEvent, max(0, m_uiPopupTimerDelay - 1), NULL);

				pDroppedDown->Redraw();
			}
		}

		if (pHotButton != NULL && pHotButton->HasMenu())
		{
			if (m_pDelayedButton != NULL)
			{
				KillTimer(nPopupTimerEvent);
			}

			if ((m_pDelayedButton = pHotButton) != NULL)
			{
				if (m_pDelayedButton == m_pDelayedCloseButton)
				{
					BOOL bRestoreSubMenu = TRUE;

					CRect rectMenu = m_pDelayedButton->GetMenuRect();

					if (!rectMenu.IsRectEmpty())
					{
						CPoint point;

						::GetCursorPos(&point);
						ScreenToClient(&point);

						if (!rectMenu.PtInRect(point))
						{
							bRestoreSubMenu = FALSE;
						}
					}

					if (bRestoreSubMenu)
					{
						RestoreDelayedSubMenu();
						m_pDelayedButton = NULL;
					}
				}
				else
				{
					SetTimer(nPopupTimerEvent, m_uiPopupTimerDelay, NULL);
				}
			}
		}

		// Maybe, this menu will be closed by the parent menu bar timer proc.?
		CMFCRibbonPanelMenuBar* pParentBar = NULL;

		if (pParentMenu != NULL && (pParentBar = pParentMenu->GetParentRibbonMenuBar()) != NULL && pParentBar->m_pDelayedCloseButton == pParentMenu->GetParentRibbonElement())
		{
			pParentBar->RestoreDelayedSubMenu();
		}

		if (pParentMenu != NULL && pParentMenu->GetParentRibbonElement() != NULL)
		{
			ASSERT_VALID(pParentMenu->GetParentRibbonElement());
			pParentMenu->GetParentRibbonElement()->OnChangeMenuHighlight(this, pHotButton);
		}
	}
	else if (pHotButton != NULL && pHotButton == m_pDelayedCloseButton)
	{
		m_pDelayedCloseButton->m_bToBeClosed = FALSE;
		m_pDelayedCloseButton = NULL;

		KillTimer(nRemovePopupTimerEvent);
	}

	if (pHot == NULL)
	{
		CMFCRibbonPanelMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCRibbonPanelMenu, GetParent());

		if (pParentMenu != NULL && pParentMenu->GetParentRibbonElement() != NULL)
		{
			ASSERT_VALID(pParentMenu->GetParentRibbonElement());
			pParentMenu->GetParentRibbonElement()->OnChangeMenuHighlight(this, NULL);
		}
	}
}

void CMFCRibbonPanelMenuBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	ASSERT_VALID(this);

	CMFCRibbonCmdUI state;
	state.m_pOther = this;

	if (m_pCategory != NULL)
	{
		m_pCategory->OnUpdateCmdUI(&state, pTarget, bDisableIfNoHndler);
	}
	else
	{
		m_pPanel->OnUpdateCmdUI(&state, pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the ribbon
	UpdateDialogControls(pTarget, bDisableIfNoHndler);

	if (bDisableIfNoHndler && m_bSetKeyTips)
	{
		if (m_pPanel != NULL)
		{
			if (m_pPanel->GetDroppedDown () == NULL)
			{
				m_pRibbonBar->SetKeyboardNavigationLevel (m_pPanel, FALSE);
			}
		}
		else if (m_pCategory != NULL)
		{
			m_pRibbonBar->SetKeyboardNavigationLevel(m_pCategory, FALSE);
		}

		m_bSetKeyTips = FALSE;
		CMFCPopupMenu::UpdateAllShadows();
	}
}

void CMFCRibbonPanelMenuBar::OnDrawMenuBorder(CDC* pDC)
{
	ASSERT_VALID(this);

	if (m_pCategory != NULL)
	{
		m_pCategory->OnDrawMenuBorder(pDC, this);
	}
	else
	{
		m_pPanel->OnDrawMenuBorder(pDC, this);
	}
}

int CMFCRibbonPanelMenuBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCPopupMenuBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!IsRibbonMiniToolBar() || IsQATPopup())
	{
		CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_RIBBON);

		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			CRect rectClient;
			GetClientRect(&rectClient);

			m_pToolTip->SetMaxTipWidth(640);
			m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectClient, GetDlgCtrlID());
		}
	}

	if (m_pPanel != NULL && m_pPanel->m_pPaletteButton != NULL)
	{
		m_wndScrollBarVert.Create(WS_CHILD | WS_VISIBLE | SBS_VERT, CRect(0, 0, 0, 0), this, nScrollBarID);
		m_pPanel->m_pScrollBar = &m_wndScrollBarVert;
	}

	if (m_pRibbonBar != NULL && m_pRibbonBar->GetKeyboardNavigationLevel() >= 0)
	{
		m_bSetKeyTips = TRUE;
		m_bHasKeyTips = TRUE;
	}

	::GetCursorPos(&m_ptStartMenu);

	return 0;
}

void CMFCRibbonPanelMenuBar::OnDestroy()
{
	if (m_pToolTip != NULL)
	{
		CTooltipManager::DeleteToolTip(m_pToolTip);
	}

	CMFCPopupMenuBar::OnDestroy();
}

void CMFCRibbonPanelMenuBar::OnSize(UINT nType, int cx, int cy)
{
	CMFCPopupMenuBar::OnSize(nType, cx, cy);

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		m_pToolTip->SetToolRect(this, GetDlgCtrlID(), CRect(0, 0, cx, cy));
	}
}

BOOL CMFCRibbonPanelMenuBar::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	if (m_pToolTip->GetSafeHwnd() == NULL || pNMH->hwndFrom != m_pToolTip->GetSafeHwnd())
	{
		return FALSE;
	}

	if (CMFCPopupMenu::GetActiveMenu() != NULL && CMFCPopupMenu::GetActiveMenu() != GetParent())
	{
		return FALSE;
	}

	CMFCRibbonBar* pTopRibbon = GetTopLevelRibbonBar();
	if (pTopRibbon != NULL && !pTopRibbon->IsToolTipEnabled())
	{
		return TRUE;
	}

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	CPoint point;

	::GetCursorPos(&point);
	ScreenToClient(&point);

	CMFCRibbonBaseElement* pHit = HitTest(point);
	if (pHit == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pHit);

	if (pHit->HasMenu () && IsMainPanel ())
	{
		return TRUE;
	}

	strTipText = pHit->GetToolTipText();
	if (strTipText.IsEmpty())
	{
		return FALSE;
	}

	CMFCToolTipCtrl* pToolTip = DYNAMIC_DOWNCAST(CMFCToolTipCtrl, m_pToolTip);

	if (pToolTip != NULL)
	{
		ASSERT_VALID(pToolTip);

		if (pTopRibbon != NULL)
		{
			ASSERT_VALID (pTopRibbon);

			pToolTip->SetFixedWidth (
				pTopRibbon->GetTooltipFixedWidthRegular (),
				pTopRibbon->GetTooltipFixedWidthLargeImage ());
		}

		if (pTopRibbon == NULL || pTopRibbon->IsToolTipDescrEnabled())
		{
			pToolTip->SetDescription(pHit->GetDescription());
		}

		pToolTip->SetHotRibbonButton(DYNAMIC_DOWNCAST(CMFCRibbonButton, pHit));

		if (!m_bIsMenuMode && !IsMainPanel())
		{
			CRect rectWindow;
			GetWindowRect(rectWindow);

			CRect rectElem = pHit->GetRect();
			ClientToScreen(&rectElem);

			pToolTip->SetLocation(CPoint(rectElem.left, rectWindow.bottom));
		}
	}

	if (m_bHasKeyTips)
	{
		m_pToolTip->SetWindowPos(&wndTopMost, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
	return TRUE;
}

LRESULT CMFCRibbonPanelMenuBar::OnUpdateToolTips(WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if ((nTypes & AFX_TOOLTIP_TYPE_RIBBON) &&(!IsRibbonMiniToolBar() || IsQATPopup()))
	{
		CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_RIBBON);

		CRect rectClient;
		GetClientRect(&rectClient);

		m_pToolTip->SetMaxTipWidth(640);

		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectClient, GetDlgCtrlID());
	}

	return 0;
}

void CMFCRibbonPanelMenuBar::PopTooltip()
{
	ASSERT_VALID(this);

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		m_pToolTip->Pop();
	}
}

void CMFCRibbonPanelMenuBar::SetActive(BOOL bIsActive)
{
	ASSERT_VALID(this);

	CMFCRibbonPanelMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCRibbonPanelMenu, GetParent());
	if (pParentMenu != NULL)
	{
		ASSERT_VALID(pParentMenu);
		pParentMenu->SetActive(bIsActive);
	}
}

CMFCRibbonBaseElement* CMFCRibbonPanelMenuBar::FindByOrigin(CMFCRibbonBaseElement* pOrigin) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pOrigin);

	if (m_pPanel == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pPanel);

	CArray<CMFCRibbonBaseElement*,CMFCRibbonBaseElement*> arElems;
	m_pPanel->GetElements(arElems);

	for (int i = 0; i < arElems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pListElem = arElems [i];
		ASSERT_VALID(pListElem);

		CMFCRibbonBaseElement* pElem = pListElem->FindByOriginal(pOrigin);

		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBar* CMFCRibbonPanelMenuBar::GetTopLevelRibbonBar() const
{
	ASSERT_VALID(this);

	if (m_pRibbonBar != NULL)
	{
		ASSERT_VALID(m_pRibbonBar);
		return m_pRibbonBar;
	}
	else
	{
		ASSERT_VALID(m_pPanelOrigin);
		ASSERT_VALID(m_pPanelOrigin->m_pParent);

		return m_pPanelOrigin->m_pParent->GetParentRibbonBar();
	}
}

void CMFCRibbonPanelMenuBar::OnTimer(UINT_PTR nIDEvent)
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	if (nIDEvent == nPopupTimerEvent)
	{
		KillTimer(nPopupTimerEvent);

		// Remove current tooltip(if any):
		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->ShowWindow(SW_HIDE);
		}

		if (m_pDelayedCloseButton != NULL && m_pDelayedCloseButton->GetRect().PtInRect(ptCursor))
		{
			return;
		}

		CloseDelayedSubMenu();

		CMFCRibbonButton* pDelayedPopupMenuButton = m_pDelayedButton;
		m_pDelayedButton = NULL;

		if (pDelayedPopupMenuButton != NULL && pDelayedPopupMenuButton->IsHighlighted())
		{
			pDelayedPopupMenuButton->OnShowPopupMenu();
		}
	}
	else if (nIDEvent == nRemovePopupTimerEvent)
	{
		KillTimer(nRemovePopupTimerEvent);

		if (m_pDelayedCloseButton != NULL)
		{
			ASSERT_VALID(m_pDelayedCloseButton);
			CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());

			CRect rectMenu = m_pDelayedCloseButton->GetRect();

			if (rectMenu.PtInRect(ptCursor))
			{
				return;
			}

			m_pDelayedCloseButton->ClosePopupMenu();
			m_pDelayedCloseButton = NULL;

			if (pParentMenu != NULL)
			{
				CMFCPopupMenu::ActivatePopupMenu(AFXGetTopLevelFrame(this), pParentMenu);
			}
		}
	}
	else if (nIDEvent == AFX_ACCELERATOR_NOTIFY_EVENT)
	{
		KillTimer(AFX_ACCELERATOR_NOTIFY_EVENT);

		CRect rc;
		GetClientRect(&rc);
		if (!rc.PtInRect(ptCursor))
		{
			return;
		}

		int nIndex = HitTestEx(ptCursor);
		if (nIndex != -1)
		{
			if (m_iAccHotItem == nIndex && m_iAccHotItem != -1)
			{
				::GetCursorPos(&ptCursor);
				if (OnSetAccData((LONG)MAKELPARAM(ptCursor.x, ptCursor.y)))
				{
					::NotifyWinEvent(EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT, nIndex + 1);
				}
			}
		}
	}
	else if (nIDEvent == IdAutoCommand)
	{
		if (!m_rectAutoCommand.PtInRect(ptCursor))
		{
			m_pPressed = NULL;
			KillTimer(IdAutoCommand);
			m_rectAutoCommand.SetRectEmpty();
		}
		else
		{
			if (m_pPressed != NULL)
			{
				ASSERT_VALID(m_pPressed);

				if (m_pPressed->GetRect().PtInRect(ptCursor))
				{
					if (!m_pPressed->OnAutoRepeat())
					{
						KillTimer(IdAutoCommand);
					}
				}
			}
		}
	}
}

void CMFCRibbonPanelMenuBar::CloseDelayedSubMenu()
{
	ASSERT_VALID(this);

	if (m_pDelayedCloseButton != NULL)
	{
		ASSERT_VALID(m_pDelayedCloseButton);

		KillTimer(nRemovePopupTimerEvent);

		m_pDelayedCloseButton->ClosePopupMenu();
		m_pDelayedCloseButton = NULL;
	}
}

void CMFCRibbonPanelMenuBar::RestoreDelayedSubMenu()
{
	ASSERT_VALID(this);

	if (m_pDelayedCloseButton == NULL || m_pPanel == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pDelayedCloseButton);
	m_pDelayedCloseButton->m_bToBeClosed = FALSE;

	CMFCRibbonBaseElement* pPrev = m_pPanel->GetHighlighted();

	m_pPanel->Highlight(TRUE, m_pDelayedCloseButton->GetRect().TopLeft());

	BOOL bUpdate = FALSE;

	if (m_pDelayedCloseButton != pPrev)
	{
		if (m_pDelayedCloseButton != NULL)
		{
			ASSERT_VALID(m_pDelayedCloseButton);
			InvalidateRect(m_pDelayedCloseButton->GetRect());
		}

		if (pPrev != NULL)
		{
			ASSERT_VALID(pPrev);
			InvalidateRect(pPrev->GetRect());
		}

		bUpdate = TRUE;
	}

	m_pDelayedCloseButton = NULL;

	KillTimer(nRemovePopupTimerEvent);

	if (bUpdate)
	{
		UpdateWindow();
	}
}

BOOL CMFCRibbonPanelMenuBar::OnKey(UINT nChar)
{
	ASSERT_VALID(this);

	if (m_pRibbonBar->ProcessKey(nChar))
	{
		return TRUE;
	}

	if (m_pPanel != NULL)
	{
		ASSERT_VALID(m_pPanel);

		CMFCDisableMenuAnimation disableMenuAnimation;
		return m_pPanel->OnKey(nChar);
	}

	return FALSE;
}

BOOL CMFCRibbonPanelMenuBar::OnSetAccData(long lVal)
{
	ASSERT_VALID(this);

	CPoint pt(LOWORD(lVal), HIWORD(lVal));
	ScreenToClient(&pt);

	CMFCRibbonBaseElement* pHit = HitTest(pt);
	if (pHit == NULL)
	{
		return FALSE;
	}

	m_AccData.Clear();

	ASSERT_VALID(pHit);
	return pHit->SetACCData(this, m_AccData);
}

void CMFCRibbonPanelMenuBar::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRibbonBar);

	if (m_bAutoCommandTimer)
	{
		KillTimer(IdAutoCommand);
		m_bAutoCommandTimer = FALSE;
		m_pPressed = NULL;
		m_rectAutoCommand.SetRectEmpty();
	}

	if (IsRibbonPanel() && m_bCustomizeMenu)
	{
		if (IsRibbonMiniToolBar() && !IsQATPopup())
		{
			return;
		}

		if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) // Left mouse button is pressed
		{
			return;
		}

		CPoint ptClient = point;
		ScreenToClient(&ptClient);

		CMFCRibbonButton* pDroppedDown = GetDroppedDown();
		if (pDroppedDown != NULL)
		{
			pDroppedDown->ClosePopupMenu();
		}

		if (m_pDelayedButton != NULL)
		{
			KillTimer(nPopupTimerEvent);
		}

		m_pRibbonBar->OnShowRibbonContextMenu(this, point.x, point.y, HitTest(ptClient));
	}
}

void CMFCRibbonPanelMenuBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	ASSERT_VALID(this);

	if (m_pPanel == NULL || pScrollBar->GetSafeHwnd() != m_wndScrollBarVert.GetSafeHwnd() || m_pPanel->m_pPaletteButton == NULL)
	{
		static BOOL bAlreadyHere = FALSE;
		if (bAlreadyHere)
			return;

		bAlreadyHere = TRUE;
		CMFCPopupMenuBar::OnVScroll(nSBCode, nPos, pScrollBar);
		bAlreadyHere = FALSE;
		return;
	}

	ASSERT_VALID(m_pPanel->m_pPaletteButton);

	SCROLLINFO scrollInfo;
	ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));

	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	m_wndScrollBarVert.GetScrollInfo(&scrollInfo);

	int iOffset = m_pPanel->m_nScrollOffset;
	int nMaxOffset = scrollInfo.nMax;
	int nPage = scrollInfo.nPage;

	if (nMaxOffset - nPage <= 1)
	{
		return;
	}

	int nRowHeight = m_pPanel->m_pPaletteButton->GetMenuRowHeight();

	switch (nSBCode)
	{
	case SB_LINEUP:
		iOffset -= nRowHeight;
		break;

	case SB_LINEDOWN:
		iOffset += nRowHeight;
		break;

	case SB_TOP:
		iOffset = 0;
		break;

	case SB_BOTTOM:
		iOffset = nMaxOffset;
		break;

	case SB_PAGEUP:
		iOffset -= nPage;
		break;

	case SB_PAGEDOWN:
		iOffset += nPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iOffset = nPos;
		break;

	default:
		return;
	}

	iOffset = min(max(0, iOffset), nMaxOffset - nPage);

	if (iOffset == m_pPanel->m_nScrollOffset)
	{
		return;
	}

	m_pPanel->ScrollPalette(iOffset);
	m_wndScrollBarVert.SetScrollPos(iOffset);
	RedrawWindow();
}

CMFCRibbonButton* CMFCRibbonPanelMenuBar::GetDroppedDown() const
{
	if (m_pCategory != NULL)
	{
		ASSERT_VALID(m_pCategory);

		return DYNAMIC_DOWNCAST(CMFCRibbonButton, m_pCategory->GetDroppedDown());
	}
	else
	{
		ASSERT_VALID(m_pPanel);

		return DYNAMIC_DOWNCAST(CMFCRibbonButton, m_pPanel->GetDroppedDown());
	}
}

CMFCRibbonBaseElement* CMFCRibbonPanelMenuBar::HitTest(CPoint point) const
{
	if (m_pCategory != NULL)
	{
		ASSERT_VALID(m_pCategory);
		return m_pCategory->HitTest(point, TRUE);
	}
	else
	{
		ASSERT_VALID(m_pPanel);
		return m_pPanel->HitTest(point);
	}
}

int CMFCRibbonPanelMenuBar::HitTestEx(CPoint point) const
{
	if (m_pCategory != NULL)
	{
		ASSERT_VALID(m_pCategory);
		return m_pCategory->HitTestEx(point);
	}
	else
	{
		ASSERT_VALID(m_pPanel);
		return m_pPanel->HitTestEx(point);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonPanelMenu

IMPLEMENT_DYNAMIC(CMFCRibbonPanelMenu, CMFCPopupMenu)

CMFCRibbonPanelMenu::CMFCRibbonPanelMenu(CMFCRibbonPanel* pPanel) : m_wndRibbonBar(pPanel)
{
	m_bForceClose = FALSE;
}

CMFCRibbonPanelMenu::CMFCRibbonPanelMenu(CMFCRibbonBar* pRibbonBar, const CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arButtons, BOOL bIsFloatyMode) :
	m_wndRibbonBar(pRibbonBar, arButtons, bIsFloatyMode)
{
	m_bForceClose = FALSE;
}

CMFCRibbonPanelMenu::CMFCRibbonPanelMenu(CMFCRibbonGallery* pPaletteButton) : m_wndRibbonBar(pPaletteButton)
{
	ASSERT_VALID(pPaletteButton);

	m_bForceClose = FALSE;
	m_bScrollable = TRUE;

	if (pPaletteButton->IsMenuResizeEnabled())
	{
		ASSERT_VALID(m_wndRibbonBar.m_pPanel);

		CSize sizeMin = m_wndRibbonBar.m_pPanel->GetPaltteMinSize();

		if (sizeMin.cx > 0 && sizeMin.cy > 0)
		{
			CSize sizeBorder = GetBorderSize();

			sizeMin.cx += sizeBorder.cx * 2;
			sizeMin.cy += sizeBorder.cy * 2;

			if (pPaletteButton->IsMenuResizeVertical())
			{
				EnableVertResize(sizeMin.cy);
			}
			else
			{
				EnableResize(sizeMin);
			}
		}
	}
}

CMFCRibbonPanelMenu::CMFCRibbonPanelMenu(CMFCRibbonCategory* pCategory, CSize size) : m_wndRibbonBar(pCategory, size)
{
	m_bForceClose = FALSE;
}

CMFCRibbonPanelMenu::CMFCRibbonPanelMenu()
{
	m_bForceClose = FALSE;
}

CMFCRibbonPanelMenu::~CMFCRibbonPanelMenu()
{
	m_bForceClose = FALSE;
}

BEGIN_MESSAGE_MAP(CMFCRibbonPanelMenu, CMFCPopupMenu)
	//{{AFX_MSG_MAP(CMFCRibbonPanelMenu)
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMFCRibbonPanelMenuBar* CMFCRibbonPanelMenu::GetParentRibbonMenuBar() const
{
	ASSERT_VALID(this);

	CMFCPopupMenu* pParentMenu = GetParentPopupMenu();
	if (pParentMenu == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pParentMenu);

	return DYNAMIC_DOWNCAST(CMFCRibbonPanelMenuBar, pParentMenu->GetMenuBar());
}

void CMFCRibbonPanelMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ASSERT_VALID(this);

	if (!m_wndRibbonBar.OnKey(nChar))
	{
		CMFCPopupMenu::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

BOOL CMFCRibbonPanelMenu::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	ASSERT_VALID(this);

	const int nSteps = abs(zDelta) / WHEEL_DELTA;

	for (int i = 0; i < nSteps; i++)
	{
		if (IsScrollUpAvailable() || IsScrollDnAvailable())
		{
			int iOffset = m_wndRibbonBar.GetOffset();

			if (zDelta > 0)
			{
				if (IsScrollUpAvailable())
				{
					m_wndRibbonBar.SetOffset(iOffset - 1);
					AdjustScroll();
				}
			}
			else
			{
				if (IsScrollDnAvailable())
				{
					m_wndRibbonBar.SetOffset(iOffset + 1);
					AdjustScroll();
				}
			}
		}
		else
		{
			m_wndRibbonBar.OnVScroll(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, &m_wndRibbonBar.m_wndScrollBarVert);
		}
	}

	return TRUE;
}

BOOL CMFCRibbonPanelMenu::IsAlwaysClose() const
{
	return m_bForceClose || ((CMFCRibbonPanelMenu*) this)->m_wndRibbonBar.IsMainPanel();
}

void CMFCRibbonPanelMenu::DoPaint(CDC* pDC)
{
	CMFCPopupMenu::DoPaint(pDC);
	m_wndRibbonBar.OnDrawMenuBorder(pDC);
}

void CMFCRibbonPanelMenu::OnLButtonDown(UINT nFlags, CPoint point)
{
	CMFCPopupMenu::OnLButtonDown(nFlags, point);

	if (m_wndRibbonBar.IsMainPanel())
	{
		ClientToScreen(&point);
		ScreenToClient(&point);

		m_wndRibbonBar.GetPanel()->MouseButtonDown(point);
	}
}

int CMFCRibbonPanelMenu::GetBorderSize() const
{
	return IsMenuMode() ? CMFCPopupMenu::GetBorderSize() : CMFCVisualManager::GetInstance()->GetRibbonPopupBorderSize(this);
}

BOOL CMFCRibbonPanelMenu::IsScrollUpAvailable()
{
	return m_wndRibbonBar.m_iOffset > 0;
}

BOOL CMFCRibbonPanelMenu::IsScrollDnAvailable()
{
	return m_wndRibbonBar.m_pPanel == NULL || m_wndRibbonBar.m_pPanel->m_bScrollDnAvailable;
}
