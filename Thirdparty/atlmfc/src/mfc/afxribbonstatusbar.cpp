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
#include "afxcontrolbarutil.h"
#include "afxcontextmenumanager.h"
#include "afxribbonstatusbar.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxribbonstatusbarpane.h"
#include "afxribbonpanelmenu.h"
#include "afxribbonlabel.h"
#include "afxtoolbarmenubutton.h"
#include "afxregpath.h"
#include "afxsettingsstore.h"
#include "afxribbonres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const CString strRibbonProfile = _T("MFCRibbons");

#define AFX_UM_UPDATE_SHADOWS (WM_USER + 101)

#define AFX_REG_SECTION_FMT _T("%sMFCRibbonBar-%d")
#define AFX_REG_SECTION_FMT_EX _T("%sMFCRibbonBar-%d%x")
#define AFX_REG_ENTRY_STATUSBAR_PANES _T("MFCStatusBarPanes")

static const int nMaxValueLen = 50;

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonStatusBarCustomizeButton

class CMFCRibbonStatusBarCustomizeButton : public CMFCRibbonButton
{
	DECLARE_DYNCREATE(CMFCRibbonStatusBarCustomizeButton)

public:
	CMFCRibbonStatusBarCustomizeButton() {}

	CMFCRibbonStatusBarCustomizeButton(LPCTSTR lpszLabel) : CMFCRibbonButton(0, lpszLabel)
	{
	}

	virtual CSize GetIntermediateSize(CDC* pDC)
	{
		ASSERT_VALID(pDC);

		CMFCRibbonBaseElement* pElement = (CMFCRibbonBaseElement*) m_dwData;
		ASSERT_VALID(pElement);

		CSize size = CMFCRibbonButton::GetIntermediateSize(pDC);
		size.cx += size.cy * 2; // Reserve space for checkbox

		CString strValue = pElement->GetText();
		if (strValue.GetLength() > nMaxValueLen)
		{
			strValue = strValue.Left(nMaxValueLen - 1);
		}

		if (!strValue.IsEmpty())
		{
			size.cx += pDC->GetTextExtent(strValue).cx + 4 * m_szMargin.cx;
		}

		return size;
	}

	virtual void OnDraw(CDC* pDC)
	{
		ASSERT_VALID(pDC);

		CMFCRibbonBaseElement* pElement = (CMFCRibbonBaseElement*) m_dwData;
		ASSERT_VALID(pElement);

		CMFCToolBarMenuButton dummy;
		dummy.m_strText = m_strText;

		CString strValue = pElement->GetText();
		if (strValue.GetLength() > nMaxValueLen)
		{
			strValue = strValue.Left(nMaxValueLen - 1);
		}

		if (!strValue.IsEmpty())
		{
			dummy.m_strText += _T('\t');
			dummy.m_strText += strValue;
		}

		dummy.m_bMenuMode = TRUE;
		dummy.m_pWndParent = GetParentWnd();

		if (pElement->IsVisible())
		{
			dummy.m_nStyle |= TBBS_CHECKED;
		}

		dummy.OnDraw(pDC, m_rect, NULL, TRUE, FALSE, m_bIsHighlighted);
	}

	virtual void OnClick(CPoint /*point*/)
	{
		CMFCRibbonBaseElement* pElement = (CMFCRibbonBaseElement*) m_dwData;
		ASSERT_VALID(pElement);

		pElement->SetVisible(!pElement->IsVisible());
		Redraw();

		CMFCRibbonBar* pRibbonStatusBar = pElement->GetParentRibbonBar();
		ASSERT_VALID(pRibbonStatusBar);

		pRibbonStatusBar->RecalcLayout();
		pRibbonStatusBar->RedrawWindow();

		CFrameWnd* pParentFrame = pRibbonStatusBar->GetParentFrame();
		ASSERT_VALID(pParentFrame);

		pParentFrame->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);

		CRect rectScreen;
		pRibbonStatusBar->GetWindowRect(&rectScreen);

		CMFCPopupMenu::UpdateAllShadows(rectScreen);
	}
};

IMPLEMENT_DYNCREATE(CMFCRibbonStatusBarCustomizeButton, CMFCRibbonButton)

const int xExtAreaMargin = 5;

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonStatusBar

IMPLEMENT_DYNAMIC(CMFCRibbonStatusBar, CMFCRibbonBar)

CMFCRibbonStatusBar::CMFCRibbonStatusBar()
{
	m_cxSizeBox = 0;
	m_cxFree = -1;
	m_rectSizeBox.SetRectEmpty();
	m_rectResizeBottom.SetRectEmpty();
	m_bBottomFrame = FALSE;
	m_rectInfo.SetRectEmpty();
}

CMFCRibbonStatusBar::~CMFCRibbonStatusBar()
{
	RemoveAll();
}

//{{AFX_MSG_MAP(CMFCRibbonStatusBar)
BEGIN_MESSAGE_MAP(CMFCRibbonStatusBar, CMFCRibbonBar)
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_MESSAGE(AFX_UM_UPDATE_SHADOWS, &CMFCRibbonStatusBar::OnUpdateShadows)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonStatusBar message handlers

BOOL CMFCRibbonStatusBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if ((m_dwStyle &(CBRS_ALIGN_ANY|CBRS_BORDER_ANY)) == CBRS_BOTTOM)
	{
		m_dwStyle &= ~(CBRS_BORDER_ANY|CBRS_BORDER_3D);
	}

	return CMFCRibbonBar::PreCreateWindow(cs);
}

BOOL CMFCRibbonStatusBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	return CreateEx(pParentWnd, 0, dwStyle, nID);
}

BOOL CMFCRibbonStatusBar::CreateEx(CWnd* pParentWnd, DWORD /*dwCtrlStyle*/, DWORD dwStyle, UINT nID)
{
	ASSERT_VALID(pParentWnd);   // must have a parent

	// save the style
	SetPaneAlignment(dwStyle & CBRS_ALL);

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();

	m_dwControlBarStyle = 0; // can't float, resize, close, slide

	if (pParentWnd->GetStyle() & WS_THICKFRAME)
	{
		dwStyle |= SBARS_SIZEGRIP;
	}

	if (!CWnd::Create(afxGlobalData.RegisterWindowClass(_T("Afx:RibbonStatusBar")), NULL, dwStyle | WS_CLIPSIBLINGS, rect, pParentWnd, nID))
	{
		return FALSE;
	}

	if (pParentWnd->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		((CFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else if (pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		((CMDIFrameWndEx*) pParentWnd)->AddPane(this);
	}
	else
	{
		ASSERT(FALSE);
		return FALSE;
	}
	return TRUE;
}

CSize CMFCRibbonStatusBar::CalcFixedLayout(BOOL, BOOL /*bHorz*/)
{
	ASSERT_VALID(this);

	CClientDC dc(this);

	CFont* pOldFont = dc.SelectObject(GetFont());
	ENSURE(pOldFont != NULL);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	int i = 0;
	int cyMax = tm.tmHeight;

	for (i = 0; i < m_arExElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arExElements [i];
		ASSERT_VALID(pElem);

		pElem->OnCalcTextSize(&dc);
		pElem->SetInitialMode();

		CSize sizeElem = pElem->GetSize(&dc);
		cyMax = max(cyMax, sizeElem.cy + 1);
	}

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnCalcTextSize(&dc);
		pElem->SetInitialMode();

		CSize sizeElem = pElem->GetSize(&dc);
		cyMax = max(cyMax, sizeElem.cy + 1);
	}

	dc.SelectObject(pOldFont);

	int nMinHeight = 24;

	if (afxGlobalData.GetRibbonImageScale() != 1.)
	{
		nMinHeight = (int)(.5 + afxGlobalData.GetRibbonImageScale() * nMinHeight);
	}

	return CSize(32767, max(nMinHeight, cyMax));
}

void CMFCRibbonStatusBar::AddElement(CMFCRibbonBaseElement* pElement, LPCTSTR lpszLabel, BOOL bIsVisible)
{
	ASSERT_VALID(this);
	ENSURE(pElement != NULL);
	ASSERT_VALID(pElement);
	ENSURE(lpszLabel != NULL);

	pElement->SetParentRibbonBar(this);
	pElement->m_bIsVisible = bIsVisible;

	m_arElements.Add(pElement);
	m_arElementLabels.Add(lpszLabel);

	CleanUpCustomizeItems();
}

void CMFCRibbonStatusBar::AddExtendedElement(CMFCRibbonBaseElement* pElement, LPCTSTR lpszLabel, BOOL bIsVisible)
{
	ASSERT_VALID(this);
	ENSURE(pElement != NULL);
	ASSERT_VALID(pElement);
	ENSURE(lpszLabel != NULL);

	pElement->SetParentRibbonBar(this);
	pElement->m_bIsVisible = bIsVisible;

	CMFCRibbonStatusBarPane* pPane = DYNAMIC_DOWNCAST(CMFCRibbonStatusBarPane, pElement);

	if (pPane != NULL)
	{
		ASSERT_VALID(pPane);
		pPane->m_bIsExtended = TRUE;
	}

	m_arExElements.Add(pElement);
	m_arExElementLabels.Add(lpszLabel);

	CleanUpCustomizeItems();
}

void CMFCRibbonStatusBar::AddSeparator()
{
	ASSERT_VALID(this);

	CMFCRibbonSeparator* pSeparator = new CMFCRibbonSeparator;
	pSeparator->SetParentRibbonBar(this);

	m_arElements.Add(pSeparator);
	m_arElementLabels.Add(_T(""));

	CleanUpCustomizeItems();
}

void CMFCRibbonStatusBar::AddDynamicElement(CMFCRibbonBaseElement* pElement)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElement);

	pElement->SetParentRibbonBar(this);
	pElement->m_bIsVisible = TRUE;

	m_arElements.Add(pElement);
	m_arElementLabels.Add(_T(""));

	m_lstDynElements.AddTail(pElement);
}

BOOL CMFCRibbonStatusBar::RemoveElement(UINT uiID)
{
	ASSERT_VALID(this);

	int i = 0;

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (pElem->GetID() == uiID)
		{
			POSITION pos = m_lstDynElements.Find(pElem);
			if (pos != NULL)
			{
				// Element is dynamic: remove it from dynamic elements list
				m_lstDynElements.RemoveAt(pos);
			}

			if (pElem == m_pHighlighted)
			{
				m_pHighlighted = NULL;
			}

			if (pElem == m_pPressed)
			{
				m_pPressed = NULL;
			}

			delete pElem;
			m_arElements.RemoveAt(i);

			return TRUE;
		}
	}

	for (i = 0; i < m_arExElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arExElements [i];
		ASSERT_VALID(pElem);

		if (pElem->GetID() == uiID)
		{
			if (pElem == m_pHighlighted)
			{
				m_pHighlighted = NULL;
			}

			if (pElem == m_pPressed)
			{
				m_pPressed = NULL;
			}

			delete pElem;
			m_arExElements.RemoveAt(i);

			return TRUE;
		}
	}

	return FALSE;
}

void CMFCRibbonStatusBar::RemoveAll()
{
	ASSERT_VALID(this);

	int i = 0;

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		delete m_arElements [i];
	}

	m_arElements.RemoveAll();

	for (i = 0; i < m_arExElements.GetSize(); i++)
	{
		delete m_arExElements [i];
	}

	m_arExElements.RemoveAll();

	m_arElementLabels.RemoveAll();
	m_arExElementLabels.RemoveAll();

	CleanUpCustomizeItems();
}

int CMFCRibbonStatusBar::GetCount() const
{
	ASSERT_VALID(this);
	return(int) m_arElements.GetSize();
}

int CMFCRibbonStatusBar::GetExCount() const
{
	ASSERT_VALID(this);
	return(int) m_arExElements.GetSize();
}

CMFCRibbonBaseElement* CMFCRibbonStatusBar::GetElement(int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= (int) m_arElements.GetSize())
	{
		ASSERT(FALSE);
		return NULL;
	}

	return m_arElements [nIndex];
}

CMFCRibbonBaseElement* CMFCRibbonStatusBar::GetExElement(int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= (int) m_arExElements.GetSize())
	{
		ASSERT(FALSE);
		return NULL;
	}

	return m_arExElements [nIndex];
}

CMFCRibbonBaseElement* CMFCRibbonStatusBar::FindElement(UINT uiID)
{
	ASSERT_VALID(this);

	int i = 0;

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arElements [i]);
		if (m_arElements [i]->GetID() == uiID)
		{
			return m_arElements [i];
		}
	}

	for (i = 0; i < m_arExElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arExElements [i]);
		if (m_arExElements [i]->GetID() == uiID)
		{
			return m_arExElements [i];
		}
	}

	return NULL;
}

BOOL CMFCRibbonStatusBar::GetExtendedArea(CRect& rect) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arExElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arExElements [i];
		ASSERT_VALID(pElem);

		CRect rectElem = pElem->GetRect();

		if (!rectElem.IsRectEmpty())
		{
			CRect rectClient;
			GetClientRect(rectClient);

			rect = rectClient;
			rect.left = rectElem.left - xExtAreaMargin;

			return TRUE;
		}
	}

	return FALSE;
}

void CMFCRibbonStatusBar::OnSize(UINT nType, int cx, int cy)
{
	CMFCRibbonBar::OnSize(nType, cx, cy);

	RecalcLayout();
	RedrawWindow();
}

LRESULT CMFCRibbonStatusBar::OnNcHitTest(CPoint point)
{
	BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	// hit test the size box - convert to HTCAPTION if so
	if (!m_rectSizeBox.IsRectEmpty())
	{
		CRect rect = m_rectSizeBox;
		ClientToScreen(&rect);

		if (rect.PtInRect(point))
		{
			OnCancelMode();
			return bRTL ? HTBOTTOMLEFT : HTBOTTOMRIGHT;
		}

		rect = m_rectResizeBottom;
		ClientToScreen(&rect);

		if (rect.PtInRect(point))
		{
			OnCancelMode();
			return HTBOTTOM;
		}
	}

	return CMFCRibbonBar::OnNcHitTest(point);
}

void CMFCRibbonStatusBar::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (m_cxSizeBox != 0 &&(nID & 0xFFF0) == SC_SIZE)
	{
		CFrameWnd* pFrameWnd = AFXGetParentFrame(this);
		if (pFrameWnd != NULL)
		{
			pFrameWnd->SendMessage(WM_SYSCOMMAND, (WPARAM)nID, lParam);
			return;
		}
	}

	CPane::OnSysCommand(nID, lParam);
}

void CMFCRibbonStatusBar::RecalcLayout()
{
	ASSERT_VALID(this);
	ASSERT(GetSafeHwnd() != NULL);

	// get the drawing area for the status bar
	CRect rect;
	GetClientRect(rect);

	// the size box is based off the size of a scrollbar
	m_cxSizeBox = min(GetSystemMetrics(SM_CXVSCROLL)+1, rect.Height());

	CFrameWnd* pFrameWnd = AFXGetParentFrame(this);
	if (pFrameWnd != NULL && pFrameWnd->IsZoomed())
	{
		m_cxSizeBox = 0;
	}

	if ((GetStyle() & SBARS_SIZEGRIP) == 0)
	{
		m_cxSizeBox = 0;
	}

	CClientDC dc(this);

	CFont* pOldFont = dc.SelectObject(GetFont());
	ENSURE(pOldFont != NULL);

	int xMax = (rect.right -= m_cxSizeBox);

	m_rectResizeBottom.SetRectEmpty();

	if (m_cxSizeBox != 0)
	{
		int cxMax = min(m_cxSizeBox, rect.Height()+m_cyTopBorder);

		m_rectSizeBox = rect;
		m_rectSizeBox.left = rect.right;
		m_rectSizeBox.right = m_rectSizeBox.left + cxMax;

		if (m_bBottomFrame)
		{
			m_rectSizeBox.OffsetRect(0, -GetSystemMetrics(SM_CYSIZEFRAME));

			m_rectResizeBottom = rect;
			m_rectResizeBottom.top = m_rectResizeBottom.bottom - GetSystemMetrics(SM_CYSIZEFRAME);
		}
	}
	else
	{
		m_rectSizeBox.SetRectEmpty();
	}

	int i = 0;

	rect.DeflateRect(0, 2);

	// Reposition extended(right) elements:
	for (i = (int) m_arExElements.GetSize() - 1; i >= 0; i--)
	{
		CMFCRibbonBaseElement* pElem = m_arExElements [i];
		ASSERT_VALID(pElem);

		pElem->OnCalcTextSize(&dc);

		CSize sizeElem = pElem->GetSize(&dc);

		if (xMax - sizeElem.cx < rect.left || !pElem->m_bIsVisible)
		{
			pElem->SetRect(CRect(0, 0, 0, 0));
		}
		else
		{
			if (pElem->CanBeStretched())
			{
				pElem->SetRect(CRect(xMax - sizeElem.cx, rect.top, xMax, rect.bottom));
			}
			else
			{
				int yOffset = max(0, (rect.Height() - sizeElem.cy) / 2);
				pElem->SetRect(CRect(CPoint(xMax - sizeElem.cx, rect.top + yOffset), sizeElem));
			}

			xMax = pElem->GetRect().left;
		}

		pElem->OnAfterChangeRect(&dc);
	}

	xMax -= 2 * xExtAreaMargin;

	// Reposition main(left) elements:
	int x = rect.left;

	if (IsInformationMode())
	{
		m_rectInfo = rect;
		m_rectInfo.right = xMax;

		for (i = (int) m_arElements.GetSize() - 1; i >= 0; i--)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			pElem->SetRect(CRect(0, 0, 0, 0));
		}
	}
	else
	{
		m_rectInfo.SetRectEmpty();

		m_cxFree = xMax - rect.left;

		BOOL bIsPrevSeparator = TRUE;
		CMFCRibbonBaseElement* pLastVisibleElem = NULL;

		for (i = 0; i <(int) m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			BOOL bIsSeparator = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonSeparator));

			if (bIsSeparator && bIsPrevSeparator)
			{
				pElem->SetRect(CRect(0, 0, 0, 0));
				continue;
			}

			pElem->OnCalcTextSize(&dc);

			CSize sizeElem = pElem->GetSize(&dc);

			if (x + sizeElem.cx > xMax || !pElem->m_bIsVisible)
			{
				pElem->SetRect(CRect(0, 0, 0, 0));
			}
			else
			{
				if (pElem->CanBeStretched())
				{
					pElem->SetRect(CRect(x, rect.top, x + sizeElem.cx, rect.bottom));
				}
				else
				{
					sizeElem.cy = min(sizeElem.cy, rect.Height());
					int yOffset = max(0, (rect.Height() - sizeElem.cy) / 2);

					pElem->SetRect(CRect(CPoint(x, rect.top + yOffset), sizeElem));
				}

				x += sizeElem.cx;

				m_cxFree = xMax - x;
				bIsPrevSeparator = bIsSeparator;

				pLastVisibleElem = pElem;
			}

			pElem->OnAfterChangeRect(&dc);
		}

		if (pLastVisibleElem != NULL)
		{
			ASSERT_VALID(pLastVisibleElem);

			if (pLastVisibleElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonSeparator)))
			{
				// Last visible element is separator - hide it:
				pLastVisibleElem->SetRect(CRect(0, 0, 0, 0));
				pLastVisibleElem->OnAfterChangeRect(&dc);
			}
		}
	}

	dc.SelectObject(pOldFont);
}

void CMFCRibbonStatusBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CMemDC memDC(dc, this);
	CDC* pDC = &memDC.GetDC();

	CRect rectClip;
	dc.GetClipBox(rectClip);

	CRgn rgnClip;

	if (!rectClip.IsRectEmpty())
	{
		rgnClip.CreateRectRgnIndirect(rectClip);
		pDC->SelectClipRgn(&rgnClip);
	}

	pDC->SetBkMode(TRANSPARENT);

	CRect rectClient;
	GetClientRect(rectClient);

	OnFillBackground(pDC, rectClient);

	// draw the size box in the bottom right corner
	if (!m_rectSizeBox.IsRectEmpty())
	{
		CRect rectSizeBox = m_rectSizeBox;

		if (m_bBottomFrame)
		{
			rectSizeBox.OffsetRect(-2, -2);
		}

		CMFCVisualManager::GetInstance()->OnDrawStatusBarSizeBox(pDC, NULL, rectSizeBox);
	}

	CFont* pOldFont = pDC->SelectObject(GetFont());
	ENSURE(pOldFont != NULL);

	int i = 0;

	if (IsInformationMode())
	{
		OnDrawInformation(pDC, m_strInfo, m_rectInfo);
	}
	else
	{
		for (i = 0; i <(int) m_arElements.GetSize(); i++)
		{
			ASSERT_VALID(m_arElements [i]);
			m_arElements [i]->OnDraw(pDC);
		}
	}

	for (i = 0; i <(int) m_arExElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arExElements [i]);
		m_arExElements [i]->OnDraw(pDC);
	}

	pDC->SelectObject(pOldFont);
	pDC->SelectClipRgn(NULL);
}

CMFCRibbonBaseElement* CMFCRibbonStatusBar::HitTest(CPoint point, BOOL /*bCheckActiveCategory*/, BOOL /*bCheckPanelCaption*/)
{
	ASSERT_VALID(this);

	int i = 0;

	for (i = 0; i <(int) m_arElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arElements [i]);

		if (m_arElements [i]->GetRect().PtInRect(point))
		{
			return m_arElements [i]->HitTest(point);
		}
	}

	for (i = 0; i <(int) m_arExElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arExElements [i]);

		if (m_arExElements [i]->GetRect().PtInRect(point))
		{
			return m_arExElements [i]->HitTest(point);
		}
	}

	return NULL;
}

void CMFCRibbonStatusBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	ASSERT_VALID(this);

	CMFCRibbonCmdUI state;
	state.m_pOther = this;

	int i = 0;

	for (i = 0; i <(int) m_arElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arElements [i]);
		m_arElements [i]->OnUpdateCmdUI(&state, pTarget, bDisableIfNoHndler);
	}

	for (i = 0; i <(int) m_arExElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arExElements [i]);
		m_arExElements [i]->OnUpdateCmdUI(&state, pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the ribbon
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

CMFCRibbonBaseElement* CMFCRibbonStatusBar::GetDroppedDown()
{
	ASSERT_VALID(this);

	int i = 0;

	for (i = 0; i <(int) m_arElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arElements [i]);

		if (m_arElements [i]->GetDroppedDown() != NULL)
		{
			return m_arElements [i];
		}
	}

	for (i = 0; i <(int) m_arExElements.GetSize(); i++)
	{
		ASSERT_VALID(m_arExElements [i]);

		if (m_arExElements [i]->GetDroppedDown() != NULL)
		{
			return m_arExElements [i];
		}
	}

	return NULL;
}

void CMFCRibbonStatusBar::OnPaneContextMenu(CWnd* /*pParentFrame*/, CPoint point)
{
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) // Left mouse button is pressed
	{
		return;
	}

	if (m_arCustomizeItems.GetSize() == 0)
	{
		CString strCaption;
		ENSURE(strCaption.LoadString(IDS_AFXBARRES_STATBAR_CUSTOMIZE));

		m_arCustomizeItems.Add(new CMFCRibbonLabel(strCaption));

		int i = 0;

		for (i = 0; i <(int) m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			if (m_lstDynElements.Find(pElem) != NULL)
			{
				// Dynamic element, don't add it to customization menu
				continue;
			}

			if (pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonSeparator)))
			{
				CMFCRibbonSeparator* pSeparator = new CMFCRibbonSeparator(TRUE);
				pSeparator->SetDefaultMenuLook();

				m_arCustomizeItems.Add(pSeparator);
			}
			else
			{
				CMFCRibbonStatusBarCustomizeButton* pItem = new CMFCRibbonStatusBarCustomizeButton(m_arElementLabels [i]);

				pItem->SetData((DWORD_PTR) pElem);
				pItem->SetDefaultMenuLook();

				m_arCustomizeItems.Add(pItem);
			}
		}

		if ((int) m_arCustomizeItems.GetSize() > 1 && m_arExElements.GetSize() > 0)
		{
			CMFCRibbonSeparator* pSeparator = new CMFCRibbonSeparator(TRUE);
			pSeparator->SetDefaultMenuLook();

			m_arCustomizeItems.Add(pSeparator);
		}

		for (i = 0; i <(int) m_arExElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arExElements [i];
			ASSERT_VALID(pElem);

			if (pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonSeparator)))
			{
				CMFCRibbonSeparator* pSeparator = new CMFCRibbonSeparator(TRUE);
				pSeparator->SetDefaultMenuLook();

				m_arCustomizeItems.Add(pSeparator);
			}
			else
			{
				CMFCRibbonStatusBarCustomizeButton* pItem = new CMFCRibbonStatusBarCustomizeButton(m_arExElementLabels [i]);

				pItem->SetData((DWORD_PTR) pElem);
				m_arCustomizeItems.Add(pItem);
			}
		}
	}

	CMFCRibbonPanelMenu* pMenu = new CMFCRibbonPanelMenu(this, m_arCustomizeItems);
	pMenu->SetMenuMode();
	pMenu->SetDefaultMenuLook();
	pMenu->EnableCustomizeMenu(FALSE);

	pMenu->Create(this, point.x, point.y, (HMENU) NULL);
}

void CMFCRibbonStatusBar::CleanUpCustomizeItems()
{
	for (int i = 0; i <(int) m_arCustomizeItems.GetSize(); i++)
	{
		ASSERT_VALID(m_arCustomizeItems [i]);
		delete m_arCustomizeItems [i];
	}

	m_arCustomizeItems.RemoveAll();
}

BOOL CMFCRibbonStatusBar::SaveState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strRibbonProfile, lpszProfileName);

	BOOL bResult = FALSE;

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		CList<UINT,UINT> lstInvisiblePanes;

		int i = 0;

		for (i = 0; i <(int) m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			if (!pElem->m_bIsVisible && pElem->GetID() != 0)
			{
				lstInvisiblePanes.AddTail(pElem->GetID());
			}
		}

		for (i = 0; i <(int) m_arExElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arExElements [i];
			ASSERT_VALID(pElem);

			if (!pElem->m_bIsVisible && pElem->GetID() != 0)
			{
				lstInvisiblePanes.AddTail(pElem->GetID());
			}
		}

		reg.Write(AFX_REG_ENTRY_STATUSBAR_PANES, lstInvisiblePanes);
	}

	bResult = CPane::SaveState(lpszProfileName, nIndex, uiID);

	return bResult;
}

BOOL CMFCRibbonStatusBar::LoadState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strRibbonProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	CList<UINT,UINT> lstInvisiblePanes;
	reg.Read(AFX_REG_ENTRY_STATUSBAR_PANES, lstInvisiblePanes);

	int i = 0;

	for (i = 0; i <(int) m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (lstInvisiblePanes.Find(pElem->GetID()) != NULL)
		{
			pElem->SetVisible(FALSE);
		}
	}

	for (i = 0; i <(int) m_arExElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arExElements [i];
		ASSERT_VALID(pElem);

		if (lstInvisiblePanes.Find(pElem->GetID()) != NULL)
		{
			pElem->SetVisible(FALSE);
		}
	}

	RecalcLayout();

	return CPane::LoadState(lpszProfileName, nIndex, uiID);
}

void CMFCRibbonStatusBar::OnRTLChanged(BOOL bIsRTL)
{
	CPane::OnRTLChanged(bIsRTL);

	int i = 0;

	for (i = 0; i <(int) m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnRTLChanged(bIsRTL);
	}

	for (i = 0; i <(int) m_arExElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arExElements [i];
		ASSERT_VALID(pElem);

		pElem->OnRTLChanged(bIsRTL);
	}
}

BOOL CMFCRibbonStatusBar::IsExtendedElement(CMFCRibbonBaseElement* pElement) const
{
	ASSERT_VALID(this);

	for (int i = 0; i <(int) m_arExElements.GetSize(); i++)
	{
		if (pElement == m_arExElements [i])
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMFCRibbonStatusBar::SetInformation(LPCTSTR lpszInfo)
{
	ASSERT_VALID(this);

	CString strInfoOld = m_strInfo;

	m_strInfo = lpszInfo == NULL ? _T("") : lpszInfo;

	if (strInfoOld == m_strInfo)
	{
		return;
	}

	BOOL bRecalcLayout = m_strInfo.IsEmpty() != strInfoOld.IsEmpty();

	if (bRecalcLayout)
	{
		RecalcLayout();
		RedrawWindow();
	}
	else
	{
		RedrawWindow(m_rectInfo);
	}

	PostMessage(AFX_UM_UPDATE_SHADOWS);
}

void CMFCRibbonStatusBar::OnDrawInformation(CDC* pDC, CString& strInfo, CRect rectInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;

	rectInfo.DeflateRect(2, 0);

	COLORREF clrTextOld = pDC->SetTextColor(CMFCVisualManager::GetInstance()->GetRibbonStatusBarTextColor(this));

	pDC->DrawText(strInfo, rectInfo, uiDTFlags);
	pDC->SetTextColor(clrTextOld);
}

LRESULT CMFCRibbonStatusBar::OnUpdateShadows(WPARAM,LPARAM)
{
	CRect rectWindow;
	GetWindowRect(rectWindow);

	CMFCPopupMenu::UpdateAllShadows(rectWindow);
	return 0;
}

void CMFCRibbonStatusBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMFCRibbonBar::OnShowWindow(bShow, nStatus);

	if (GetParentFrame () != NULL)
	{
		GetParentFrame ()->PostMessage (AFX_WM_CHANGEVISUALMANAGER);
	}
}


