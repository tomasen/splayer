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
#include "afxribbonpanel.h"
#include "afxribboncategory.h"
#include "afxribbonbar.h"
#include "afxvisualmanager.h"
#include "afxribbonpanelmenu.h"
#include "multimon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nPanelMarginLeft = 2;
static const int nPanelMarginRight = 2;
static const int nPanelMarginTop = 3;
static const int nPanelMarginBottom = 4;

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonTab

CMFCRibbonTab::CMFCRibbonTab()
{
	m_bIsTruncated = FALSE;
	m_Color = AFX_CategoryColor_None;
	m_nFullWidth = 0;
}

CString CMFCRibbonTab::GetToolTipText() const
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	if (!m_bIsTruncated)
	{
		return _T("");
	}

	CString strToolTipText = m_pParent->m_strName;
	strToolTipText.Remove(_T('&'));

	return strToolTipText;
}

void CMFCRibbonTab::CopyFrom(const CMFCRibbonBaseElement& s)
{
	CMFCRibbonBaseElement::CopyFrom(s);

	CMFCRibbonTab& src = (CMFCRibbonTab&) s;
	m_Color = src.m_Color;
	m_nFullWidth = src.m_nFullWidth;
}

void CMFCRibbonTab::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pParent);
	ASSERT_VALID(m_pParent->GetParentRibbonBar());

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	COLORREF clrText = CMFCVisualManager::GetInstance()->OnDrawRibbonCategoryTab(pDC, this, m_pParent->IsActive() || GetDroppedDown() != NULL);
	COLORREF clrTextOld = pDC->SetTextColor(clrText);

	CRect rectTab = m_rect;
	CRect rectTabText = m_rect;

	pDC->DrawText(m_pParent->m_strName, rectTabText, DT_CALCRECT | DT_SINGLELINE | DT_VCENTER);

	const int cxTabText = rectTabText.Width();
	const int cxTabTextMargin = max(4, (rectTab.Width() - cxTabText) / 2);

	rectTab.DeflateRect(cxTabTextMargin, 0);
	rectTab.top += nPanelMarginTop;

	pDC->DrawText(m_pParent->m_strName, rectTab, DT_SINGLELINE | DT_VCENTER);
	pDC->SetTextColor(clrTextOld);
}

CRect CMFCRibbonTab::GetKeyTipRect(CDC* pDC, BOOL /*bIsMenu*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pParent);
	ASSERT_VALID(m_pParent->m_pParentRibbonBar);

	CSize sizeKeyTip = GetKeyTipSize(pDC);

	if (sizeKeyTip == CSize(0, 0) || m_rect.IsRectEmpty())
	{
		return CRect(0, 0, 0, 0);
	}

	CRect rectKeyTip(0, 0, 0, 0);

	CRect rectTab = m_rect;
	CRect rectTabText = m_rect;

	pDC->DrawText(m_pParent->m_strName, rectTabText, DT_CALCRECT | DT_SINGLELINE | DT_VCENTER);

	const int cxTabText = rectTabText.Width();
	const int cxTabTextMargin = max(4, (rectTab.Width() - cxTabText) / 2);

	rectTab.DeflateRect(cxTabTextMargin, 0);
	rectTab.top += nPanelMarginTop;

	rectKeyTip.left = rectTab.CenterPoint().x - sizeKeyTip.cx / 2;
	rectKeyTip.right = rectKeyTip.left + sizeKeyTip.cx;

	rectKeyTip.top = rectTabText.bottom - 2;
	rectKeyTip.bottom = rectKeyTip.top + sizeKeyTip.cy;

	return rectKeyTip;
}

BOOL CMFCRibbonTab::OnKey(BOOL /*bIsMenuKey*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	CMFCRibbonBar* pBar = m_pParent->GetParentRibbonBar();
	ASSERT_VALID(pBar);

	if (IsDisabled())
	{
		return FALSE;
	}

	pBar->SetActiveCategory(m_pParent);

	if ((pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ELEMENTS) == 0)
	{
		pBar->SetKeyboardNavigationLevel(m_pParent);
	}

	return FALSE;
}

BOOL CMFCRibbonTab::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	CMFCRibbonBar* pBar = m_pParent->GetParentRibbonBar();
	ASSERT_VALID(pBar);

	const BOOL bIsRibbonMinimized = (pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ELEMENTS) != 0;

	if (!CMFCRibbonBaseElement::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_bAccState = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;

	if (bIsRibbonMinimized)
	{
		data.m_bAccState |= STATE_SYSTEM_HASPOPUP;

		if (IsDroppedDown())
		{
			data.m_bAccState |= STATE_SYSTEM_SELECTED | STATE_SYSTEM_PRESSED;
			data.m_strAccDefAction = _T("Close");
		}
		else
		{
			data.m_strAccDefAction = _T("Open");
		}
	}
	else
	{
		if (m_pParent->IsActive())
		{
			data.m_bAccState |= STATE_SYSTEM_SELECTED;
		}

		data.m_strAccDefAction = _T("Switch");
	}

	data.m_strAccName = m_pParent->m_strName;
	data.m_nAccRole = ROLE_SYSTEM_PAGETAB;
	data.m_strAccKeys = _T("Alt, ") + m_strKeys;
	return TRUE;
}

CSize CMFCRibbonTab::GetRegularSize(CDC* /*pDC*/)
{
	return CSize(0, 0);
}

void CMFCRibbonTab::OnLButtonDown(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	m_pParent->GetParentRibbonBar()->SetActiveCategory(m_pParent);
}

void CMFCRibbonTab::OnLButtonDblClk(CPoint /*point*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	if (m_pParent->IsActive())
	{
		if (m_pParent->m_ActiveTime != (clock_t)-1 && clock () - m_pParent->m_ActiveTime < (int) GetDoubleClickTime ())
		{
			return;
		}

		CMFCRibbonBar* pBar = m_pParent->GetParentRibbonBar();
		ASSERT_VALID(pBar);

		if ((pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ELEMENTS) != 0)
		{
			// Ribbon is minimized, restore it now:
			if (IsDroppedDown())
			{
				ClosePopupMenu();
			}

			m_pParent->ShowElements();
		}
		else
		{
			// Minimize ribbon:
			m_pParent->ShowElements(FALSE);
		}

		pBar->GetParentFrame()->RecalcLayout();
		pBar->RedrawWindow();
	}
}

BOOL CMFCRibbonTab::IsSelected() const
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	if (!m_pParent->IsActive())
	{
		return FALSE;
	}

	CMFCRibbonBar* pRibbonBar = m_pParent->GetParentRibbonBar();
	ASSERT_VALID(pRibbonBar);

	if (pRibbonBar != CWnd::GetFocus())
	{
		return FALSE;
	}

	if (pRibbonBar->GetKeyboardNavigationLevel() < 0)
	{
		return FALSE;
	}

	if (pRibbonBar->GetQATDroppedDown() != NULL)
	{
		return FALSE;
	}

	if (pRibbonBar->GetApplicationButton() != NULL && pRibbonBar->GetApplicationButton()->IsDroppedDown())
	{
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CRibbonCategoryScroll

CRibbonCategoryScroll::CRibbonCategoryScroll ()
{
	m_bIsLeft = FALSE;
}

void CRibbonCategoryScroll::CopyFrom (const CMFCRibbonBaseElement& s)
{
	CMFCRibbonButton::CopyFrom (s);

	CRibbonCategoryScroll& src = (CRibbonCategoryScroll&) s;
	m_bIsLeft = src.m_bIsLeft;
}

void CRibbonCategoryScroll::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CMFCVisualManager::GetInstance ()->OnDrawRibbonCategoryScroll (
		pDC, this);
}

BOOL CRibbonCategoryScroll::OnAutoRepeat ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	if (m_rect.IsRectEmpty ())
	{
		return FALSE;
	}

	return m_pParent->OnScrollHorz (m_bIsLeft);
}

void CRibbonCategoryScroll::OnMouseMove(CPoint point) 
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	if (m_rect.IsRectEmpty ())
	{
		m_bIsHighlighted = FALSE;
		return;
	}

	BOOL bWasHighlighted = m_bIsHighlighted;
	m_bIsHighlighted = m_rect.PtInRect (point);

	if (bWasHighlighted != m_bIsHighlighted)
	{
		if (m_pParent->GetParentMenuBar () != NULL)
		{
			m_pParent->GetParentMenuBar ()->PopTooltip ();
		}
		else if (m_pParent->GetParentRibbonBar () != NULL)
		{
			m_pParent->GetParentRibbonBar ()->PopTooltip ();
		}

		Redraw ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonCategory

IMPLEMENT_DYNCREATE(CMFCRibbonCategory, CObject)

CMFCRibbonCategory::CMFCRibbonCategory()
{
	CommonInit();
}

CMFCRibbonCategory::CMFCRibbonCategory(CMFCRibbonBar* pParentRibbonBar, LPCTSTR lpszName, UINT uiSmallImagesResID, UINT uiLargeImagesResID, CSize sizeSmallImage, CSize sizeLargeImage)
{
	ASSERT_VALID(pParentRibbonBar);
	ENSURE(lpszName != NULL);

	CommonInit(pParentRibbonBar, lpszName, uiSmallImagesResID, uiLargeImagesResID, sizeSmallImage, sizeLargeImage);
}

void CMFCRibbonCategory::CommonInit(CMFCRibbonBar* pParentRibbonBar, LPCTSTR lpszName, UINT uiSmallImagesResID, UINT uiLargeImagesResID, CSize sizeSmallImage, CSize sizeLargeImage)
{
	m_pParentMenuBar = NULL;
	m_bMouseIsPressed = FALSE;
	m_bIsActive = FALSE;
	m_bIsVisible = TRUE;
	m_dwData = 0;
	m_uiContextID = 0;
	m_nLastCategoryWidth = -1;
	m_nLastCategoryOffsetY = 0;
	m_nMinWidth = -1;

	m_rect.SetRectEmpty();

	m_pParentRibbonBar = pParentRibbonBar;
	SetName(lpszName);

	// Load images:
	if (sizeSmallImage != CSize(0, 0))
	{
		m_SmallImages.SetImageSize(sizeSmallImage);
	}

	if (sizeLargeImage != CSize(0, 0))
	{
		m_LargeImages.SetImageSize(sizeLargeImage);
	}

	if (uiSmallImagesResID > 0)
	{
		if (!m_SmallImages.Load(uiSmallImagesResID))
		{
			ASSERT(FALSE);
		}
	}

	if (uiLargeImagesResID > 0)
	{
		if (!m_LargeImages.Load(uiLargeImagesResID))
		{
			ASSERT(FALSE);
		}
	}

	m_Tab.m_pParent = this;

	int nIndex = m_strName.Find(_T('\n'));
	if (nIndex >= 0)
	{
		m_Tab.SetKeys(m_strName.Mid(nIndex + 1));
		m_strName = m_strName.Left(nIndex);
	}

	m_ScrollLeft.m_pParent = this;
	m_ScrollRight.m_pParent = this;
	m_ScrollLeft.m_bIsLeft = TRUE;
	m_ScrollRight.m_bIsLeft = FALSE;

	m_nScrollOffset = 0;
	m_ActiveTime = (clock_t)-1;
}

CMFCRibbonCategory::~CMFCRibbonCategory()
{
	int i = 0;

	for (i = 0; i < m_arPanels.GetSize(); i++)
	{
		delete m_arPanels [i];
	}

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		delete m_arElements [i];
	}
}

void CMFCRibbonCategory::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CMFCVisualManager::GetInstance()->OnDrawRibbonCategory(pDC, this, m_rect);

	BOOL bClip = FALSE;

	CRgn rgnClip;

	if (!m_ScrollLeft.GetRect ().IsRectEmpty () ||
		!m_ScrollRight.GetRect ().IsRectEmpty ())
	{
		CRect rectClient = m_rect;
		rectClient.DeflateRect (nPanelMarginLeft, nPanelMarginTop, 
			nPanelMarginRight, nPanelMarginBottom);

		rgnClip.CreateRectRgnIndirect (rectClient);
		pDC->SelectClipRgn (&rgnClip);

		bClip = TRUE;
	}

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->DoPaint(pDC);
	}

	if (bClip)
	{
		pDC->SelectClipRgn (NULL);
	}

	m_ScrollLeft.OnDraw (pDC);
	m_ScrollRight.OnDraw (pDC);
}

CMFCRibbonPanel* CMFCRibbonCategory::AddPanel(LPCTSTR lpszPanelName, HICON hIcon, CRuntimeClass* pRTI)
{
	ASSERT_VALID(this);
	ENSURE(lpszPanelName != NULL);

	CMFCRibbonPanel* pPanel = NULL;

	if (pRTI != NULL)
	{
		pPanel = DYNAMIC_DOWNCAST(CMFCRibbonPanel, pRTI->CreateObject());

		if (pPanel == NULL)
		{
			ASSERT(FALSE);
			return NULL;
		}

		pPanel->CommonInit(lpszPanelName, hIcon);
	}
	else
	{
		pPanel = new CMFCRibbonPanel(lpszPanelName, hIcon);
	}

	m_arPanels.Add(pPanel);

	pPanel->m_pParent = this;
	pPanel->m_btnLaunch.m_pParent = this;
	pPanel->m_btnDefault.m_pParent = this;

	m_nLastCategoryWidth = -1;
	m_nMinWidth = -1;
	return pPanel;
}

int CMFCRibbonCategory::GetPanelCount() const
{
	ASSERT_VALID(this);
	return(int) m_arPanels.GetSize();
}

CMFCRibbonPanel* CMFCRibbonCategory::GetPanel(int nIndex)
{
	ASSERT_VALID(this);
	return m_arPanels [nIndex];
}

int CMFCRibbonCategory::GetPanelIndex(const CMFCRibbonPanel* pPanel) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pPanel);

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		if (m_arPanels [i] == pPanel)
		{
			return i;
		}
	}

	return -1;
}

int CMFCRibbonCategory::GetMaxHeight(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	int nMaxHeight = 0;

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		nMaxHeight = max(nMaxHeight, pPanel->GetHeight(pDC));
	}

	return nMaxHeight + pDC->GetTextExtent(m_strName).cy + nPanelMarginTop + nPanelMarginBottom;
}

void CMFCRibbonCategory::RecalcLayout(CDC* pDC)
{
	if (m_rect.IsRectEmpty())
	{
		return;
	}

	if (m_pParentMenuBar != NULL)
	{
		CleanUpSizes();
	}

	RecalcPanelWidths(pDC);

	if (m_arPanels.GetSize() == 0)
	{
		return;
	}

	int i = 0;
	BOOL bRedrawScroll = FALSE;

	const DWORD dwRibbonHideFlags = GetParentRibbonBar()->m_dwHideFlags;
	const BOOL bHideAll = (dwRibbonHideFlags & AFX_RIBBONBAR_HIDE_ALL) || (dwRibbonHideFlags & AFX_RIBBONBAR_HIDE_ELEMENTS);

	if (m_nMinWidth < 0)
	{
		m_nMinWidth = GetMinWidth(pDC);
	}

	if (bHideAll && m_pParentMenuBar == NULL)
	{
		for (i = 0; i < m_arPanels.GetSize(); i++)
		{
			CMFCRibbonPanel* pPanel = m_arPanels [i];
			ASSERT_VALID(pPanel);

			pPanel->Reposition(pDC, CRect(0, 0, 0, 0));
			pPanel->OnShow(FALSE);
		}

		m_nLastCategoryWidth = -1;
		m_nMinWidth = -1;
	}
	else if (m_nLastCategoryWidth != m_rect.Width() || m_nLastCategoryOffsetY != m_rect.top)
	{
		m_nLastCategoryWidth = m_rect.Width();
		m_nLastCategoryOffsetY = m_rect.top;

		CRect rectClient = m_rect;
		rectClient.DeflateRect(nPanelMarginLeft * 2, nPanelMarginTop, nPanelMarginRight * 2, nPanelMarginBottom);

		ResetPanelsLayout();

		if (rectClient.Width () <= m_nMinWidth)
		{
			//-------------------------
			// Just collapse all panes:
			//-------------------------
			for (i = 0; i < m_arPanels.GetSize (); i++)
			{
				CMFCRibbonPanel* pPanel = m_arPanels [i];
				ASSERT_VALID (pPanel);

				pPanel->m_bForceCollpapse = TRUE;
				pPanel->m_nCurrWidthIndex = (int) pPanel->m_arWidths.GetSize () - 1;
			}
		}
		else
		{
			BOOL bAutoResize = TRUE;

			if (m_arCollapseOrder.GetSize() > 0)
			{
				bAutoResize = FALSE;

				BOOL bOK = TRUE;

				for (int iNextPane = 0; iNextPane <= m_arCollapseOrder.GetSize(); iNextPane++)
				{
					bOK = SetPanelsLayout(rectClient.Width());
					if (bOK || iNextPane == m_arCollapseOrder.GetSize())
					{
						break;
					}

					// Find next pane for collapsing - from the user-defined list:
					int nPaneIndex = m_arCollapseOrder [iNextPane];
					if (nPaneIndex < 0 || nPaneIndex >= m_arPanels.GetSize())
					{
						ASSERT(FALSE);
						bOK = FALSE;
						break;
					}

					CMFCRibbonPanel* pPanel = m_arPanels [nPaneIndex];
					ASSERT_VALID(pPanel);

					if (iNextPane < m_arCollapseOrder.GetSize() - 1 && m_arCollapseOrder [iNextPane + 1] == -1)
					{
						pPanel->m_bForceCollpapse = TRUE;
						pPanel->m_nCurrWidthIndex = (int) pPanel->m_arWidths.GetSize() - 1;

						iNextPane++;
					}
					else
					{
						if (pPanel->m_nCurrWidthIndex < pPanel->m_arWidths.GetSize() - 1)
						{
							pPanel->m_nCurrWidthIndex++;
						}
					}
				}

				if (!bOK)
				{
					bAutoResize = TRUE;
					ResetPanelsLayout();
				}
			}

			if (bAutoResize)
			{
				while (TRUE)
				{
					if (SetPanelsLayout(rectClient.Width()))
					{
						break;
					}

					// Find next pane for collapsing - next matched:
					int nMaxWeightIndex = -1;
					int nMaxWeight = 1;

					for (i = 0; i < m_arPanels.GetSize(); i++)
					{
						CMFCRibbonPanel* pPanel = m_arPanels [i];
						ASSERT_VALID(pPanel);

						int nWeight = (int) pPanel->m_arWidths.GetSize() - pPanel->m_nCurrWidthIndex - 1;
						if (nWeight >= nMaxWeight)
						{
							nMaxWeightIndex = i;
							nMaxWeight = nWeight;
						}
					}

					if (nMaxWeightIndex < 0)
					{
						break;
					}

					CMFCRibbonPanel* pPanel = m_arPanels [nMaxWeightIndex];
					ASSERT_VALID(pPanel);

					pPanel->m_nCurrWidthIndex++;
				}
			}
		}

		ReposPanels(pDC);
		bRedrawScroll = TRUE;
	}

	UpdateScrollButtons ();

	if (bRedrawScroll && GetParentRibbonBar ()->GetSafeHwnd() != NULL)
	{
		if (!m_ScrollLeft.GetRect ().IsRectEmpty() ||
			!m_ScrollRight.GetRect ().IsRectEmpty())
		{
			GetParentRibbonBar ()->RedrawWindow(m_rect);
		}
	}
}

void CMFCRibbonCategory::UpdateScrollButtons ()
{
	m_ScrollLeft.m_pParentMenu = m_pParentMenuBar;
	m_ScrollRight.m_pParentMenu = m_pParentMenuBar;

	const int cxScrollWidth = (int) (afxGlobalData.GetRibbonImageScale () * 13);

	CRect rectScrollLeft (0, 0, 0, 0);
	CRect rectScrollRight (0, 0, 0, 0);

	if (m_nScrollOffset > 0)
	{
		rectScrollLeft = m_rect;
		rectScrollLeft.right = rectScrollLeft.left + cxScrollWidth;
	}

	if (m_rect.Width () + m_nScrollOffset < m_nMinWidth)
	{
		rectScrollRight = m_rect;
		rectScrollRight.left = rectScrollRight.right - cxScrollWidth;
	}

	m_ScrollLeft.SetRect (rectScrollLeft);
	m_ScrollRight.SetRect (rectScrollRight);
}

void CMFCRibbonCategory::ReposPanels (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CRect rectClient = m_rect;
	rectClient.DeflateRect (nPanelMarginLeft * 2, nPanelMarginTop, 
							nPanelMarginRight * 2, nPanelMarginBottom);

	const BOOL bForceCollpapse = (rectClient.Width () <= m_nMinWidth);

	int x = rectClient.left - m_nScrollOffset;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (bForceCollpapse)
		{
			pPanel->m_bForceCollpapse = TRUE;
			pPanel->m_nCurrWidthIndex = (int) pPanel->m_arWidths.GetSize () - 1;
		}

		const int nCurrPanelWidth = 
			pPanel->m_arWidths [pPanel->m_nCurrWidthIndex] + 
			2 * pPanel->m_nXMargin;

		CRect rectPanel = CRect (x, rectClient.top, 
								x + nCurrPanelWidth, rectClient.bottom);

		pPanel->Reposition (pDC, rectPanel);

		x = pPanel->m_rect.right + nPanelMarginRight;

		if (rectPanel.right <= rectClient.left + 2 * nPanelMarginLeft ||
			rectPanel.left >= rectClient.right - 2 * nPanelMarginRight)
		{
			rectPanel.SetRectEmpty ();



			pPanel->Reposition (pDC, rectPanel);
		}

		if (bForceCollpapse)
		{
			pPanel->m_bForceCollpapse = TRUE;
		}

		pPanel->OnAfterChangeRect (pDC);
	}
}

void CMFCRibbonCategory::RecalcPanelWidths(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	int nHeight = -1;

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		if (pPanel->m_arWidths.GetSize() == 0)
		{
			if (nHeight == -1)
			{
				nHeight = GetMaxHeight(pDC);
			}

			pPanel->RecalcWidths(pDC, nHeight);
			m_nLastCategoryWidth = -1;
		}
	}

	m_nMinWidth = -1;
}

void CMFCRibbonCategory::CleanUpSizes()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->CleanUpSizes();
		pPanel->m_arWidths.RemoveAll();
	}

	m_nLastCategoryWidth = -1;
	m_nMinWidth = -1;
}

int CMFCRibbonCategory::GetMinWidth(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	int nMinWidth = nPanelMarginLeft;

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		nMinWidth += pPanel->GetMinWidth(pDC) + nPanelMarginRight;
	}

	return nMinWidth;
}

void CMFCRibbonCategory::OnMouseMove(CPoint point)
{
	ASSERT_VALID(this);

	m_ScrollLeft.OnMouseMove (point);
	m_ScrollRight.OnMouseMove (point);

	if (m_ScrollLeft.IsHighlighted () || m_ScrollRight.IsHighlighted ())
	{
		return;
	}

	HighlightPanel(GetPanelFromPoint(point), point);
}

CMFCRibbonBaseElement* CMFCRibbonCategory::HitTest(CPoint point, BOOL bCheckPanelCaption) const
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement* pBtnScroll = HitTestScrollButtons (point);
	if (pBtnScroll != NULL)
	{
		return pBtnScroll;
	}

	CMFCRibbonPanel* pPanel = GetPanelFromPoint(point);
	if (pPanel != NULL)
	{
		ASSERT_VALID(pPanel);
		return pPanel->HitTest(point, bCheckPanelCaption);
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonCategory::HitTestScrollButtons (CPoint point) const
{
	ASSERT_VALID (this);

	if (m_ScrollLeft.GetRect ().PtInRect (point))
	{
		return (CMFCRibbonBaseElement*)&m_ScrollLeft;
	}

	if (m_ScrollRight.GetRect ().PtInRect (point))
	{
		return (CMFCRibbonBaseElement*)&m_ScrollRight;
	}

	return NULL;
}

int CMFCRibbonCategory::HitTestEx(CPoint point) const
{
	ASSERT_VALID(this);

	CMFCRibbonPanel* pPanel = GetPanelFromPoint(point);
	if (pPanel != NULL)
	{
		ASSERT_VALID(pPanel);
		return pPanel->HitTestEx(point);
	}

	return -1;
}

CMFCRibbonPanel* CMFCRibbonCategory::GetPanelFromPoint(CPoint point) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		if (pPanel->m_rect.PtInRect(point))
		{
			return pPanel;
		}
	}

	return NULL;
}

CMFCRibbonPanel* CMFCRibbonCategory::HighlightPanel(CMFCRibbonPanel* pHLPanel, CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonPanel* pPrevHLPanel = NULL;

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		if (pPanel->IsHighlighted())
		{
			if (pHLPanel != pPanel)
			{
				pPanel->Highlight(FALSE, point);
			}

			pPrevHLPanel = pPanel;
		}

		if (pHLPanel == pPanel)
		{
			pPanel->Highlight(TRUE, point);
		}
	}

	if (m_pParentMenuBar != NULL)
	{
		ASSERT_VALID(m_pParentMenuBar);
		m_pParentMenuBar->UpdateWindow();
	}
	else
	{
		ASSERT_VALID(m_pParentRibbonBar);
		m_pParentRibbonBar->UpdateWindow();
	}

	return pPrevHLPanel;
}

void CMFCRibbonCategory::OnCancelMode()
{
	m_bMouseIsPressed = FALSE;

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->CancelMode();
	}
}

CMFCRibbonBaseElement* CMFCRibbonCategory::OnLButtonDown(CPoint point)
{
	CMFCRibbonBaseElement* pBtnScroll = HitTestScrollButtons (point);
	if (pBtnScroll != NULL)
	{
		ASSERT_VALID(pBtnScroll);
		pBtnScroll->OnAutoRepeat ();

		if (HitTestScrollButtons (point) == pBtnScroll)
		{
			return pBtnScroll;
		}
		else
		{
			return NULL;
		}
	}

	CMFCRibbonPanel* pPanel = GetPanelFromPoint(point);
	if (pPanel == NULL)
	{
		return NULL;
	}

	m_bMouseIsPressed = TRUE;

	ASSERT_VALID(pPanel);
	return pPanel->MouseButtonDown(point);
}

void CMFCRibbonCategory::OnLButtonUp(CPoint point)
{
	m_ScrollLeft.m_bIsHighlighted = FALSE;
	m_ScrollRight.m_bIsHighlighted = FALSE;

	CMFCRibbonPanel* pPanel = GetPanelFromPoint(point);
	if (pPanel == NULL)
	{
		return;
	}

	m_bMouseIsPressed = FALSE;

	ASSERT_VALID(pPanel);
	pPanel->MouseButtonUp(point);
}

void CMFCRibbonCategory::OnUpdateCmdUI(CMFCRibbonCmdUI* pCmdUI, CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->OnUpdateCmdUI(pCmdUI, pTarget, bDisableIfNoHndler);
	}
}

BOOL CMFCRibbonCategory::NotifyControlCommand(BOOL bAccelerator, int nNotifyCode, WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		if (pPanel->NotifyControlCommand(bAccelerator, nNotifyCode, wParam, lParam))
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMFCRibbonCategory::SetActive(BOOL bIsActive)
{
	ASSERT_VALID(this);

	if (m_bIsActive == bIsActive)
	{
		return;
	}

	if ((m_pParentRibbonBar->m_dwHideFlags & AFX_RIBBONBAR_HIDE_ELEMENTS) != 0)
	{
		m_bIsActive = bIsActive;
		return;
	}

	ShowElements();

	m_bIsActive = bIsActive;

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->OnShow(bIsActive);
	}

	m_ActiveTime = bIsActive ? clock () : (clock_t)-1;
}

void CMFCRibbonCategory::ShowElements(BOOL bShow)
{
	ASSERT_VALID(this);

	GetParentRibbonBar()->m_dwHideFlags = bShow ? 0 : AFX_RIBBONBAR_HIDE_ELEMENTS;

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->OnShow(bShow);
	}

	GetParentRibbonBar()->GetParentFrame()->RecalcLayout();
}

CMFCRibbonBaseElement* CMFCRibbonCategory::FindByID(UINT uiCmdID, BOOL bVisibleOnly) const
{
	ASSERT_VALID(this);

	int i = 0;

	if (!bVisibleOnly)
	{
		for (i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			if (pElem->GetID() == uiCmdID)
			{
				return pElem;
			}
		}
	}

	for (i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		CMFCRibbonBaseElement* pElem = pPanel->FindByID(uiCmdID);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonCategory::FindByData(DWORD_PTR dwData, BOOL bVisibleOnly) const
{
	ASSERT_VALID(this);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		CMFCRibbonBaseElement* pElem = pPanel->FindByData(dwData);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	if (!bVisibleOnly)
	{
		for (i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			if (pElem->GetData() == dwData)
			{
				return pElem;
			}
		}
	}

	return NULL;
}

void CMFCRibbonCategory::GetElementsByID(UINT uiCmdID, CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arButtons)
{
	ASSERT_VALID(this);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->GetElementsByID(uiCmdID, arButtons);
	}

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (pElem->GetID() == uiCmdID)
		{
			arButtons.Add(pElem);
		}
	}
}

void CMFCRibbonCategory::GetItemIDsList(CList<UINT,UINT>& lstItems, BOOL bHiddenOnly) const
{
	ASSERT_VALID(this);

	int i = 0;

	if (!bHiddenOnly)
	{
		for (i = 0; i < m_arPanels.GetSize(); i++)
		{
			CMFCRibbonPanel* pPanel = m_arPanels [i];
			ASSERT_VALID(pPanel);

			pPanel->GetItemIDsList(lstItems);
		}
	}

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->GetItemIDsList(lstItems);
	}
}

CMFCRibbonPanel* CMFCRibbonCategory::FindPanelWithElem(const CMFCRibbonBaseElement* pElement)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElement);

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		if (pPanel->HasElement(pElement))
		{
			return pPanel;
		}
	}

	return NULL;
}

void CMFCRibbonCategory::AddHidden(CMFCRibbonBaseElement* pElement)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElement);

	pElement->SetParentCategory(this);
	m_arElements.Add(pElement);
}

BOOL CMFCRibbonCategory::OnDrawImage(CDC* pDC, CRect rect, CMFCRibbonBaseElement* pElement, BOOL bIsLargeImage, BOOL nImageIndex, BOOL bCenter)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElement);

	CMFCToolBarImages& image = bIsLargeImage ? m_LargeImages : m_SmallImages;

	if (nImageIndex >= image.GetCount())
	{
		return FALSE;
	}

	CAfxDrawState ds;

	CPoint ptImage = rect.TopLeft();
	const CSize sizeImage = GetImageSize(bIsLargeImage);

	if (bCenter)
	{
		ptImage.Offset(max(0, (rect.Width() - sizeImage.cx) / 2), max(0, (rect.Height() - sizeImage.cy) / 2));
	}

	image.SetTransparentColor(afxGlobalData.clrBtnFace);

	if (afxGlobalData.GetRibbonImageScale() != 1.)
	{
		image.PrepareDrawImage(ds, sizeImage);
	}
	else
	{
		image.PrepareDrawImage(ds);
	}

	image.Draw(pDC, ptImage.x, ptImage.y, nImageIndex, FALSE, pElement->IsDisabled());

	image.EndDrawImage(ds);
	return TRUE;
}

CSize CMFCRibbonCategory::GetImageSize(BOOL bIsLargeImage) const
{
	ASSERT_VALID(this);

	const CMFCToolBarImages& image = bIsLargeImage ? m_LargeImages : m_SmallImages;
	const CSize sizeImage = image.GetImageSize();

	if (afxGlobalData.GetRibbonImageScale() == 1.)
	{
		return sizeImage;
	}

	return CSize(
		(int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImage.cx),
		(int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImage.cy));
}

void CMFCRibbonCategory::GetElements(CArray <CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements)
{
	ASSERT_VALID(this);

	arElements.RemoveAll();

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->GetElements(arElements);
	}

	// Add hidden elements:
	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		arElements.Add(pElem);
	}
}

CMFCRibbonBaseElement* CMFCRibbonCategory::GetDroppedDown()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		CMFCRibbonBaseElement* pElem = pPanel->GetDroppedDown();
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

void CMFCRibbonCategory::ShowFloating(CRect rectFloating)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParentRibbonBar);

	if ((m_pParentRibbonBar->m_dwHideFlags & AFX_RIBBONBAR_HIDE_ELEMENTS) == 0)
	{
		ASSERT(FALSE);
		return;
	}

	const BOOL bIsRTL = (m_pParentRibbonBar->GetExStyle() & WS_EX_LAYOUTRTL);

	CMFCRibbonPanelMenu* pMenu = new CMFCRibbonPanelMenu(this, rectFloating.Size());

	m_Tab.SetDroppedDown(pMenu);

	pMenu->Create(m_pParentRibbonBar, bIsRTL ? rectFloating.right : rectFloating.left, rectFloating.top, (HMENU) NULL);
}

void CMFCRibbonCategory::CopyFrom(CMFCRibbonCategory& src)
{
	ASSERT_VALID(this);

	int i = 0;

	m_strName = src.m_strName;
	m_bIsActive = src.m_bIsActive;
	m_bIsVisible = src.m_bIsVisible;

	for (i = 0; i < src.m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanelSrc = src.m_arPanels [i];
		ASSERT_VALID(pPanelSrc);

		CMFCRibbonPanel* pPanel = (CMFCRibbonPanel*) pPanelSrc->GetRuntimeClass()->CreateObject();
		ASSERT_VALID(pPanel);

		pPanel->CopyFrom(*pPanelSrc);

		pPanel->m_btnDefault.CopyFrom(pPanelSrc->m_btnDefault);
		pPanel->m_btnDefault.SetOriginal(&pPanelSrc->m_btnDefault);

		m_arPanels.Add(pPanel);
	}

	m_pParentRibbonBar = src.m_pParentRibbonBar;
	m_rect = src.m_rect;
	m_Tab.CopyFrom(src.m_Tab);

	m_dwData = src.m_dwData;
	m_uiContextID = src.m_uiContextID;
	m_pParentMenuBar = src.m_pParentMenuBar;

	src.m_SmallImages.CopyTo(m_SmallImages);
	src.m_LargeImages.CopyTo(m_LargeImages);

	m_arCollapseOrder.RemoveAll();
	m_arCollapseOrder.Copy(src.m_arCollapseOrder);

	m_ScrollLeft.CopyFrom (src.m_ScrollLeft);
	m_ScrollLeft.m_pParent = this;
	m_ScrollRight.CopyFrom (src.m_ScrollRight);
	m_ScrollRight.m_pParent = this;
}

CMFCRibbonBaseElement* CMFCRibbonCategory::GetParentButton() const
{
	ASSERT_VALID(this);

	if (m_pParentMenuBar == NULL)
	{
		return NULL;
	}

	return((CMFCPopupMenu*)m_pParentMenuBar->GetParent())->GetParentRibbonElement();
}

void CMFCRibbonCategory::SetKeys(LPCTSTR lpszKeys)
{
	ASSERT_VALID(this);
	ENSURE(lpszKeys != NULL);

	m_Tab.SetKeys(lpszKeys);
}

void CMFCRibbonCategory::SetName(LPCTSTR lpszName)
{
	ASSERT_VALID(this);

	m_strName = lpszName == NULL ? _T("") : lpszName;

	// Remove '&' characters and build key string:
	CString strKeys;

	for (int i = 0; i < m_strName.GetLength(); i++)
	{
		if (m_strName [i] == _T('&'))
		{
			m_strName.Delete(i);

			if (i < m_strName.GetLength())
			{
				strKeys += m_strName [i];
			}
		}
	}

	m_Tab.SetKeys(strKeys);
}

void CMFCRibbonCategory::OnRTLChanged(BOOL bIsRTL)
{
	ASSERT_VALID(this);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->OnRTLChanged(bIsRTL);
	}

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnRTLChanged(bIsRTL);
	}

	m_nLastCategoryWidth = -1;
}

int CMFCRibbonCategory::GetTextTopLine() const
{
	ASSERT_VALID(this);

	const CSize sizeImageLarge = GetImageSize(TRUE);
	if (sizeImageLarge == CSize(0, 0))
	{
		return -1;
	}

	return sizeImageLarge.cy + 5;
}

void CMFCRibbonCategory::SetCollapseOrder(const CArray<int, int>& arCollapseOrder)
{
	ASSERT_VALID(this);

	m_arCollapseOrder.RemoveAll();
	m_arCollapseOrder.Copy(arCollapseOrder);
}

BOOL CMFCRibbonCategory::SetPanelsLayout(int nWidth)
{
	int nTotalWidth = 0;

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->m_bForceCollpapse = FALSE;

		if (pPanel->m_nCurrWidthIndex == pPanel->m_arWidths.GetSize() - 1 && pPanel->m_arWidths.GetSize() > 1)
		{
			pPanel->m_bForceCollpapse = TRUE;
		}

		const int nCurrPanelWidth = pPanel->m_arWidths [pPanel->m_nCurrWidthIndex] + 2 * pPanel->m_nXMargin;

		nTotalWidth += nCurrPanelWidth + nPanelMarginRight;

		if (nTotalWidth > nWidth)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CMFCRibbonCategory::ResetPanelsLayout()
{
	// all panels in max. width:
	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CMFCRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID(pPanel);

		pPanel->m_nCurrWidthIndex = 0;
		pPanel->m_bTrancateCaption = FALSE;
	}

	m_nScrollOffset = 0;
}

BOOL CMFCRibbonCategory::OnScrollHorz (BOOL bScrollLeft, int nScrollOffset/* = 0*/)
{
	ASSERT_VALID(this);

	const int nPrevScrollOffset = m_nScrollOffset;

	if (nScrollOffset == 0)
	{
		nScrollOffset = 50;
	}

	if (bScrollLeft)
	{
		m_nScrollOffset -= nScrollOffset;
	}
	else
	{
		m_nScrollOffset += nScrollOffset;
	}

	m_nScrollOffset = min (m_nMinWidth - m_rect.Width (), max (0, m_nScrollOffset));

	CMFCRibbonBar* pRibbonBar = GetParentRibbonBar ();
	ASSERT_VALID (pRibbonBar);

	CClientDC dc (pRibbonBar);

	CFont* pOldFont = dc.SelectObject (pRibbonBar->GetFont ());
	ASSERT (pOldFont != NULL);

	ReposPanels (&dc);

	dc.SelectObject (pOldFont);

	UpdateScrollButtons ();

	if (m_pParentMenuBar != NULL)
	{
		ASSERT_VALID (m_pParentMenuBar);
		m_pParentMenuBar->RedrawWindow ();
	}
	else
	{
		pRibbonBar->RedrawWindow (m_rect);
	}

	return nPrevScrollOffset != m_nScrollOffset;
}

void CMFCRibbonCategory::EnsureVisible (CMFCRibbonButton* pButton)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pButton);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CRect rectClient = m_rect;
	rectClient.DeflateRect (nPanelMarginLeft * 2, nPanelMarginTop, 
		nPanelMarginRight * 2, nPanelMarginBottom);

	CRect rectButton = pButton->GetRect ();
	if (rectButton.IsRectEmpty ())
	{
		CMFCRibbonPanel* pParentPanel = pButton->GetParentPanel ();
		if (pParentPanel == NULL)
		{
			return;
		}

		ASSERT_VALID(pParentPanel);
		ASSERT(pParentPanel->GetRect ().IsRectEmpty ());

		int nPanelIndex = -1;
		int nFirstVisiblePanel = -1;
		int nLastVisiblePanel = -1;
		int i = 0;

		for (i = 0; i < m_arPanels.GetSize (); i++)
		{
			CMFCRibbonPanel* pPanel = m_arPanels [i];
			ASSERT_VALID (pPanel);

			if (pPanel == pParentPanel)
			{
				nPanelIndex = i;
			}

			if (!pPanel->GetRect ().IsRectEmpty ())
			{
				if (nFirstVisiblePanel < 0)
				{
					nFirstVisiblePanel = i;
				}

				nLastVisiblePanel = i;
			}
		}

		if (nPanelIndex == -1 || nFirstVisiblePanel == -1 || nLastVisiblePanel == -1)
		{
			return;
		}

		if (nPanelIndex < nFirstVisiblePanel)
		{
			while (OnScrollHorz (TRUE))
			{
				if (!pParentPanel->GetRect ().IsRectEmpty () &&
					pParentPanel->GetRect ().left >= rectClient.left)
				{
					break;
				}
			}
		}
		else if (nPanelIndex > nLastVisiblePanel)
		{
			while (OnScrollHorz (FALSE))
			{
				if (!pParentPanel->GetRect ().IsRectEmpty () &&
					pParentPanel->GetRect ().right <= rectClient.right)
				{
					break;
				}
			}
		}

		return;
	}

	if (rectButton.left < m_rect.left - nPanelMarginRight)
	{
		OnScrollHorz (TRUE, rectClient.left - rectButton.left);
	}
	else if (rectButton.right > m_rect.right + nPanelMarginRight)
	{
		OnScrollHorz (FALSE, rectButton.right - rectClient.right);
	}
}

void CMFCRibbonCategory::NormalizeFloatingRect (CMFCRibbonBar* pRibbonBar, CRect& rectCategory)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRibbonBar);

	CRect rectRibbon;
	pRibbonBar->GetWindowRect (rectRibbon);

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (rectRibbon.TopLeft (), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	rectCategory.right = min (rectCategory.right, rectScreen.right);
	rectCategory.left = max (rectCategory.left, rectScreen.left);
}


