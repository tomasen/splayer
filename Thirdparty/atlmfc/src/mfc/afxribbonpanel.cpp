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
#include "afxdrawmanager.h"
#include "afxglobals.h"
#include "afxribbonres.h"
#include "afxvisualmanager.h"
#include "afxtoolbar.h"
#include "afxribbonbuttonsgroup.h"
#include "afxribbonbar.h"
#include "afxribbonpanelmenu.h"
#include "afxribbonbutton.h"
#include "afxribbonpalettegallery.h"
#include "afxribbonlabel.h"
#include "afxribbonundobutton.h"
#include "afxkeyboardmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CMFCRibbonLaunchButton

IMPLEMENT_DYNCREATE(CMFCRibbonLaunchButton, CMFCRibbonButton)

CMFCRibbonLaunchButton::CMFCRibbonLaunchButton()
{
	m_pParentPanel = NULL;
}

void CMFCRibbonLaunchButton::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	if (m_pParentPanel != NULL)
	{
		ASSERT_VALID(m_pParentPanel);

		CMFCVisualManager::GetInstance()->OnDrawRibbonLaunchButton(pDC, this, m_pParentPanel);
	}
	else
	{
		CMFCRibbonButton::OnDraw(pDC);
	}
}

CSize CMFCRibbonLaunchButton::GetRegularSize(CDC* pDC)
{
	ASSERT_VALID(this);

	if (m_pParentPanel != NULL)
	{
		return CSize(0, 0);
	}

	return CMFCRibbonButton::GetRegularSize(pDC);
}

void CMFCRibbonLaunchButton::OnClick(CPoint point)
{
	ASSERT_VALID(this);

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID(m_pParentMenu);
		m_pParentMenu->OnClickButton(this, point);
		return;
	}

	NotifyCommand();
}

CRect CMFCRibbonLaunchButton::GetKeyTipRect(CDC* pDC, BOOL bIsMenu)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_pParentPanel == NULL)
	{
		return CMFCRibbonButton::GetKeyTipRect(pDC, bIsMenu);
	}

	ASSERT_VALID(m_pParentPanel);

	CSize sizeKeyTip = GetKeyTipSize(pDC);
	CRect rectKeyTip(0, 0, 0, 0);

	if (sizeKeyTip == CSize(0, 0) || m_rect.IsRectEmpty())
	{
		return rectKeyTip;
	}

	rectKeyTip.top = m_rect.bottom;
	rectKeyTip.right = m_pParentPanel->GetRect().right;
	rectKeyTip.left = rectKeyTip.right - sizeKeyTip.cx;
	rectKeyTip.bottom = rectKeyTip.top + sizeKeyTip.cy;

	return rectKeyTip;
}

BOOL CMFCRibbonLaunchButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	if (!CMFCRibbonButton::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_bAccState |= STATE_SYSTEM_HASPOPUP;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMFCRibbonDefaultPanelButton

IMPLEMENT_DYNCREATE(CMFCRibbonDefaultPanelButton, CMFCRibbonButton)

CMFCRibbonDefaultPanelButton::CMFCRibbonDefaultPanelButton(CMFCRibbonPanel* pPanel)
{
	m_hIcon = NULL;
	m_pPanel = pPanel;
}

void CMFCRibbonDefaultPanelButton::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CMFCVisualManager::GetInstance()->OnDrawRibbonDefaultPaneButton
		(pDC, this);
}

void CMFCRibbonDefaultPanelButton::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonButton::CopyFrom(s);

	CMFCRibbonDefaultPanelButton& src = (CMFCRibbonDefaultPanelButton&) s;

	m_pPanel = src.m_pPanel;
	m_pParent = src.m_pParent;

	if (m_pPanel != NULL)
	{
		ASSERT_VALID(m_pPanel);
		m_strToolTip = m_pPanel->GetName();
	}
}

void CMFCRibbonDefaultPanelButton::OnLButtonDown(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnLButtonDown(point);
	OnShowPopupMenu();
}

void CMFCRibbonDefaultPanelButton::OnShowPopupMenu()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pPanel);

	m_pPanel->ShowPopup(this);
}

BOOL CMFCRibbonDefaultPanelButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pPanel);
	ASSERT_VALID(pParent);

	if (!CMFCRibbonButton::SetACCData(pParent, data))
	{
		return FALSE;
	}

	if (m_rect.Width() == 0 && m_rect.Height() == 0)
	{
		data.m_nAccRole = ROLE_SYSTEM_TOOLBAR;
		data.m_strAccValue = _T("group");
		data.m_rectAccLocation = m_pPanel->GetRect();
		pParent->ClientToScreen(&data.m_rectAccLocation);
		data.m_bAccState = 0;
		data.m_strAccDefAction = _T("");
		return TRUE;
	}

	data.m_nAccRole = ROLE_SYSTEM_BUTTONDROPDOWNGRID;
	data.m_bAccState |= STATE_SYSTEM_HASPOPUP;
	data.m_strAccDefAction = _T("Open");

	if (IsDroppedDown())
	{
		data.m_bAccState |= STATE_SYSTEM_PRESSED;
		data.m_strAccDefAction = _T("Close");
	}

	return TRUE;
}

void CMFCRibbonDefaultPanelButton::OnDrawOnList(CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL bIsSelected, BOOL bHighlighted)
{
	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectText = rect;

	rectText.left += nTextOffset;
	const int nXMargin = 3;
	rectText.DeflateRect(nXMargin, 0);

	pDC->DrawText(strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	m_bIsDisabled = bIsDisabled;

	CMFCVisualManager::GetInstance()->OnDrawRibbonDefaultPaneButtonIndicator(pDC, this, rect, bIsSelected, bHighlighted);
}

void CMFCRibbonDefaultPanelButton::DrawImage(CDC* pDC, RibbonImageType type, CRect rectImage)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CMFCRibbonDefaultPanelButton* pOrigButton = DYNAMIC_DOWNCAST(CMFCRibbonDefaultPanelButton, m_pOriginal);

	if (pOrigButton != NULL)
	{
		ASSERT_VALID(pOrigButton);

		pOrigButton->DrawImage(pDC, type, rectImage);
		return;
	}

	if (m_hIcon == NULL)
	{
		CMFCVisualManager::GetInstance()->OnDrawDefaultRibbonImage(pDC, rectImage);
		return;
	}

	CSize sizeIcon(16, 16);

	if (afxGlobalData.GetRibbonImageScale() != 1.)
	{
		sizeIcon.cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeIcon.cx);
		sizeIcon.cy = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeIcon.cy);
	}

	BOOL bIsRTL = FALSE;

	CMFCRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar();
	if (pTopLevelRibbon != NULL &&(pTopLevelRibbon->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		bIsRTL = TRUE;
	}

	if (afxGlobalData.GetRibbonImageScale() != 1. || bIsRTL)
	{
		UINT diFlags = DI_NORMAL;

		if (bIsRTL)
		{
			diFlags |= 0x0010 /*DI_NOMIRROR*/;
		}

		::DrawIconEx(pDC->GetSafeHdc(), rectImage.CenterPoint().x - sizeIcon.cx / 2, rectImage.CenterPoint().y - sizeIcon.cy / 2, m_hIcon, sizeIcon.cx, sizeIcon.cy, 0, NULL, diFlags);
	}
	else
	{
		pDC->DrawState(CPoint(rectImage.CenterPoint().x - sizeIcon.cx / 2, rectImage.CenterPoint().y - sizeIcon.cy / 2), sizeIcon, m_hIcon, DSS_NORMAL, (HBRUSH) NULL);
	}
}

BOOL CMFCRibbonDefaultPanelButton::OnKey(BOOL /*bIsMenuKey*/)
{
	ASSERT_VALID(this);

	if (IsDisabled())
	{
		return FALSE;
	}

	if (!m_pPanel->GetRect ().IsRectEmpty () && !m_pPanel->IsCollapsed () && !IsQATMode ())
	{
		return FALSE;
	}

	OnShowPopupMenu();
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CMFCRibbonPanel

UINT CMFCRibbonPanel::m_nNextPanelID = (UINT)-10;

IMPLEMENT_DYNCREATE(CMFCRibbonPanel, CObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#pragma warning(disable : 4355)

CMFCRibbonPanel::CMFCRibbonPanel(LPCTSTR lpszName, HICON hIcon) : m_btnDefault(this)
{
	CommonInit(lpszName, hIcon);
}

CMFCRibbonPanel::CMFCRibbonPanel(CMFCRibbonGallery* pPaletteButton) : m_btnDefault(this)
{
	CommonInit();

	ASSERT_VALID(pPaletteButton);
	m_pPaletteButton = pPaletteButton;
}

void CMFCRibbonPanel::CopyFrom(CMFCRibbonPanel& src)
{
	m_strName = src.m_strName;
	m_dwData = src.m_dwData;
	m_pParent = src.m_pParent;
	m_nXMargin = src.m_nXMargin;
	m_nYMargin = src.m_nYMargin;
	m_bShowCaption = src.m_bShowCaption;
	m_bAlignByColumn = src.m_bAlignByColumn;
	m_bCenterColumnVert = src.m_bCenterColumnVert;
	m_bJustifyColumns = src.m_bJustifyColumns;

	int i = 0;

	for (i = 0; i < src.m_arWidths.GetSize(); i++)
	{
		m_arWidths.Add(src.m_arWidths [i]);
	}

	for (i = 0; i < src.m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pSrcElem = src.m_arElements [i];
		ASSERT_VALID(pSrcElem);

		CMFCRibbonBaseElement* pElem = (CMFCRibbonBaseElement*) pSrcElem->GetRuntimeClass()->CreateObject();
		ASSERT_VALID(pElem);

		pElem->CopyFrom(*pSrcElem);
		pElem->SetOriginal(pSrcElem);

		m_arElements.Add(pElem);
	}

	m_btnLaunch.CopyFrom(src.m_btnLaunch);
	m_btnLaunch.SetOriginal(&src.m_btnLaunch);
}

void CMFCRibbonPanel::CommonInit(LPCTSTR lpszName, HICON hIcon)
{
	m_strName = lpszName != NULL ? lpszName : _T("");

	int nIndex = m_strName.Find(_T('\n'));
	if (nIndex >= 0)
	{
		m_btnDefault.SetKeys(m_strName.Mid(nIndex + 1));
		m_strName = m_strName.Left(nIndex);
	}

	m_dwData = 0;
	m_btnDefault.m_hIcon = hIcon;
	m_btnDefault.SetText(m_strName);
	m_btnDefault.SetID(m_nNextPanelID--);

	m_rect.SetRectEmpty();
	m_pParent = NULL;
	m_pParentMenuBar = NULL;
	m_nCurrWidthIndex = 0;
	m_nFullWidth = 0;
	m_nRows = 0;
	m_nXMargin = 4;
	m_nYMargin = 2;
	m_bShowCaption = FALSE;
	m_bForceCollpapse = FALSE;
	m_bIsHighlighted = FALSE;
	m_bIsCalcWidth = FALSE;
	m_pHighlighted = NULL;
	m_bAlignByColumn = TRUE;
	m_bCenterColumnVert = FALSE;
	m_bFloatyMode = FALSE;
	m_bIsQATPopup = FALSE;
	m_bMenuMode = FALSE;
	m_bIsDefaultMenuLook = FALSE;
	m_bIsFirst = TRUE;
	m_bIsLast = TRUE;
	m_rectCaption.SetRectEmpty();
	m_pPaletteButton = NULL;
	m_rectMenuAreaTop.SetRectEmpty();
	m_rectMenuAreaBottom.SetRectEmpty();
	m_pScrollBar = NULL;
	m_nScrollOffset = 0;
	m_bSizeIsLocked = FALSE;
	m_bJustifyColumns = FALSE;
	m_bScrollDnAvailable = FALSE;
	m_bTrancateCaption = FALSE;
}

#pragma warning(default : 4355)

CMFCRibbonPanel::~CMFCRibbonPanel()
{
	CMFCRibbonBaseElement* pDroppedDown = GetDroppedDown();
	if (pDroppedDown != NULL)
	{
		ASSERT_VALID(pDroppedDown);
		pDroppedDown->ClosePopupMenu();
	}

	RemoveAll();
}

void CMFCRibbonPanel::EnableLaunchButton(UINT uiCmdID, int nIconIndex, LPCTSTR lpszKeys)
{
	ASSERT_VALID(this);

	m_btnLaunch.SetID(uiCmdID);
	m_btnLaunch.m_nSmallImageIndex = nIconIndex;
	m_btnLaunch.SetKeys(lpszKeys);
}

void CMFCRibbonPanel::Add(CMFCRibbonBaseElement* pElem)
{
	Insert(pElem, (int) m_arElements.GetSize());
}

void CMFCRibbonPanel::AddSeparator()
{
	InsertSeparator((int) m_arElements.GetSize());
}

BOOL CMFCRibbonPanel::Insert(CMFCRibbonBaseElement* pElem, int nIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	if (nIndex == -1)
	{
		nIndex = (int) m_arElements.GetSize();
	}

	if (nIndex < 0 || nIndex > m_arElements.GetSize())
	{
		return FALSE;
	}

	pElem->SetParentCategory(m_pParent);

	if (!pElem->IsAlignByColumn() && m_bAlignByColumn)
	{
		// If 2 or more elements are aligned by row, set this
		// flag for whole panel:
		for (int i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pListElem = m_arElements [i];
			ASSERT_VALID(pListElem);

			if (!pListElem->IsAlignByColumn())
			{
				m_bAlignByColumn = FALSE;
				break;
			}
		}
	}

	if (nIndex == m_arElements.GetSize())
	{
		m_arElements.Add(pElem);
	}
	else
	{
		m_arElements.InsertAt(nIndex, pElem);
	}

	return TRUE;
}

BOOL CMFCRibbonPanel::InsertSeparator(int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex > m_arElements.GetSize())
	{
		return FALSE;
	}

	CMFCRibbonSeparator* pSeparator = new CMFCRibbonSeparator;
	ASSERT_VALID(pSeparator);

	pSeparator->m_pParent = m_pParent;

	if (nIndex == m_arElements.GetSize())
	{
		m_arElements.Add(pSeparator);
	}
	else
	{
		m_arElements.InsertAt(nIndex, pSeparator);
	}

	return TRUE;
}

CMFCRibbonButtonsGroup* CMFCRibbonPanel::AddToolBar(UINT uiToolbarResID, UINT uiColdResID, UINT uiHotResID, UINT uiDisabledResID)
{
	ASSERT_VALID(this);

	// Create temporary toolbar and load bitmaps:
	CMFCToolBar wndToolbar;
	if (!wndToolbar.LoadToolBar(uiToolbarResID, uiColdResID, 0, TRUE, uiDisabledResID, 0, uiHotResID))
	{
		return NULL;
	}

	CMFCToolBarImages* pImages = wndToolbar.GetLockedImages();
	CMFCToolBarImages* pColdImages = wndToolbar.GetLockedColdImages();
	CMFCToolBarImages* pDisabledImages = wndToolbar.GetLockedDisabledImages();
	CMFCToolBarImages* pHotImages = NULL;

	if (pColdImages != NULL && pColdImages->GetCount() > 0)
	{
		pHotImages = uiHotResID != 0 ? pImages : NULL;
		pImages = pColdImages;
	}

	CList<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> lstButtons;

	for (int i = 0; i < wndToolbar.GetCount(); i++)
	{
		CMFCToolBarButton* pToolbarButton = wndToolbar.GetButton(i);
		ASSERT_VALID(pToolbarButton);

		if (pToolbarButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (!lstButtons.IsEmpty())
			{
				CMFCRibbonButtonsGroup* pGroup = new CMFCRibbonButtonsGroup;

				pGroup->AddButtons(lstButtons);
				pGroup->SetImages(pImages, pHotImages, pDisabledImages);

				Add(pGroup);

				lstButtons.RemoveAll();
			}
		}
		else
		{
			CMFCRibbonButton* pButton = new CMFCRibbonButton;

			pButton->SetID(pToolbarButton->m_nID);
			pButton->SetText(pToolbarButton->m_strText);
			pButton->m_nSmallImageIndex = pToolbarButton->GetImage();

			lstButtons.AddTail(pButton);
		}
	}

	if (!lstButtons.IsEmpty())
	{
		CMFCRibbonButtonsGroup* pGroup = new CMFCRibbonButtonsGroup;

		pGroup->AddButtons(lstButtons);
		pGroup->SetImages(pImages, pHotImages, pDisabledImages);

		Add(pGroup);
		return pGroup;
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::GetElement(int nIndex) const
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arElements.GetSize())
	{
		ASSERT(FALSE);
		return NULL;
	}

	return m_arElements [nIndex];
}

int CMFCRibbonPanel::GetCount() const
{
	ASSERT_VALID(this);
	return(int) m_arElements.GetSize();
}

BOOL CMFCRibbonPanel::Remove(int nIndex, BOOL bDelete)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arElements.GetSize())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCRibbonBaseElement* pElem = m_arElements [nIndex];
	ASSERT_VALID(pElem);

	if (pElem == m_pHighlighted)
	{
		m_pHighlighted = NULL;
	}

	m_arElements.RemoveAt(nIndex);

	if (bDelete)
	{
		delete pElem;
	}

	return TRUE;
}

void CMFCRibbonPanel::RemoveAll()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		delete m_arElements [i];
	}

	m_arElements.RemoveAll();
	m_bAlignByColumn = TRUE;
}

void CMFCRibbonPanel::DoPaint(CDC* pDC)
{
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CRect rectClip;
	pDC->GetClipBox(rectClip);

	CRect rectInter;

	if (!rectInter.IntersectRect(m_rect, rectClip))
	{
		return;
	}

	COLORREF clrTextOld = pDC->GetTextColor();

	// Fill panel background:
	COLORREF clrText = m_pParent == NULL || m_pPaletteButton != NULL ? afxGlobalData.clrBarText : CMFCVisualManager::GetInstance()->OnDrawRibbonPanel(pDC, this, m_rect, m_rectCaption);

	// Draw panel caption:
	if (!m_rectCaption.IsRectEmpty() && rectInter.IntersectRect(m_rectCaption, rectClip))
	{
		CMFCVisualManager::GetInstance()->OnDrawRibbonPanelCaption(pDC, this, m_rectCaption);
	}

	// Draw launch button:
	if (rectInter.IntersectRect(m_btnLaunch.GetRect(), rectClip))
	{
		m_btnLaunch.OnDraw(pDC);
	}

	pDC->SetTextColor(clrText);

	if (!m_btnDefault.GetRect().IsRectEmpty())
	{
		// Panel is collapsed, draw default button only:
		if (rectInter.IntersectRect(m_btnDefault.GetRect(), rectClip))
		{
			m_btnDefault.OnDraw(pDC);
		}
	}
	else if (m_pPaletteButton != NULL)
	{
		OnDrawPaletteMenu(pDC);
	}
	else
	{
		if (m_bIsDefaultMenuLook && m_pParentMenuBar != NULL)
		{
			ASSERT_VALID(m_pParentMenuBar);

			BOOL bDisableSideBarInXPMode = m_pParentMenuBar->m_bDisableSideBarInXPMode;
			m_pParentMenuBar->m_bDisableSideBarInXPMode = FALSE;

			CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, m_pParentMenuBar, m_rect, m_rect);

			m_pParentMenuBar->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
		}

		// Draw panel elements:
		for (int i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			if (rectInter.IntersectRect(pElem->GetRect(), rectClip))
			{
				pDC->SetTextColor(clrText);

				BOOL bIsHighlighted = pElem->m_bIsHighlighted;

				if (IsMenuMode() && pElem->IsDroppedDown() && m_pHighlighted == NULL)
				{
					pElem->m_bIsHighlighted = TRUE;
				}

				pElem->OnDraw(pDC);

				pElem->m_bIsHighlighted = bIsHighlighted;
			}
		}
	}

	pDC->SetTextColor(clrTextOld);
}

void CMFCRibbonPanel::OnDrawPaletteMenu(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pPaletteButton);

	const BOOL bNoSideBar = m_pPaletteButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonUndoButton));
	const BOOL bIsXPSideBar = !bNoSideBar && m_pPaletteButton->IsMenuSideBar();

	int i = 0;

	CRect rectIcons = m_rect;

	CRect rectSeparatorBottom;
	rectSeparatorBottom.SetRectEmpty();

	CRect rectSeparatorTop;
	rectSeparatorTop.SetRectEmpty();

	if (!m_rectMenuAreaBottom.IsRectEmpty())
	{
		if (m_pParentMenuBar != NULL && !bIsXPSideBar && !bNoSideBar)
		{
			BOOL bDisableSideBarInXPMode = m_pParentMenuBar->m_bDisableSideBarInXPMode;
			m_pParentMenuBar->m_bDisableSideBarInXPMode = FALSE;

			CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, m_pParentMenuBar, m_rectMenuAreaBottom, m_rectMenuAreaBottom);

			m_pParentMenuBar->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
		}

		rectSeparatorBottom = m_rectMenuAreaBottom;
		rectSeparatorBottom.top--;
		rectSeparatorBottom.bottom = rectSeparatorBottom.top + 1;

		rectIcons.bottom = m_rectMenuAreaBottom.top - 1;
	}

	if (!m_rectMenuAreaTop.IsRectEmpty())
	{
		if (m_pParentMenuBar != NULL && !bIsXPSideBar)
		{
			BOOL bDisableSideBarInXPMode = m_pParentMenuBar->m_bDisableSideBarInXPMode;
			m_pParentMenuBar->m_bDisableSideBarInXPMode = FALSE;

			CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, m_pParentMenuBar, m_rectMenuAreaTop, m_rectMenuAreaTop);

			m_pParentMenuBar->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
		}

		rectSeparatorTop = m_rectMenuAreaTop;
		rectSeparatorTop.bottom++;
		rectSeparatorTop.top = rectSeparatorTop.bottom - 1;

		rectIcons.top = m_rectMenuAreaTop.bottom + 1;
	}

	if (m_pParentMenuBar != NULL && bIsXPSideBar)
	{
		BOOL bDisableSideBarInXPMode = m_pParentMenuBar->m_bDisableSideBarInXPMode;
		m_pParentMenuBar->m_bDisableSideBarInXPMode = FALSE;

		CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, m_pParentMenuBar, m_rect, m_rect);

		m_pParentMenuBar->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
	}

	CRgn rgnClip;

	rgnClip.CreateRectRgnIndirect(rectIcons);
	pDC->SelectClipRgn(&rgnClip);

	CAfxDrawState ds;

	if (m_pPaletteButton->m_imagesPalette.GetCount() > 0)
	{
		m_pPaletteButton->m_imagesPalette.SetTransparentColor(afxGlobalData.clrBtnFace);
		m_pPaletteButton->m_imagesPalette.PrepareDrawImage(ds, m_pPaletteButton->GetIconSize());
	}

	// First, draw icons + labels:
	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		BOOL bIsLabel = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel));
		BOOL bIsIcon = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon));

		if (bIsLabel || bIsIcon)
		{
			pElem->OnDraw(pDC);
		}
	}

	pDC->SelectClipRgn(NULL);

	// Draw rest of elements:
	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		BOOL bIsLabel = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel));
		BOOL bIsIcon = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon));

		if (!bIsLabel && !bIsIcon)
		{
			pElem->m_bIsDefaultMenuLook = TRUE;
			pElem->OnDraw(pDC);
		}
	}

	if (!rectSeparatorTop.IsRectEmpty())
	{
		CMFCVisualManager::GetInstance()->OnDrawSeparator(pDC, m_pParentMenuBar, rectSeparatorTop, FALSE);
	}

	if (!rectSeparatorBottom.IsRectEmpty())
	{
		CMFCVisualManager::GetInstance()->OnDrawSeparator(pDC, m_pParentMenuBar, rectSeparatorBottom, FALSE);
	}

	if (m_pPaletteButton->m_imagesPalette.GetCount() > 0)
	{
		m_pPaletteButton->m_imagesPalette.EndDrawImage(ds);
	}
}

int CMFCRibbonPanel::GetHeight(CDC* pDC) const
{
	const int nVertMargin = 3;

	ASSERT_VALID (pDC);

	((CMFCRibbonPanel*)this)->m_btnDefault.OnCalcTextSize (pDC);

	int nRowHeight = 0;
	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);

		TEXTMETRIC tm;
		pDC->GetTextMetrics (&tm);
		nRowHeight = max (m_pParent->GetImageSize (FALSE).cy, tm.tmHeight) + 2 * nVertMargin;
	}

	int nMaxHeight = max (nRowHeight * 3, ((CMFCRibbonPanel*)this)->m_btnDefault.GetRegularSize (pDC).cy);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnCalcTextSize (pDC);
		nMaxHeight = max (nMaxHeight, pElem->GetRegularSize (pDC).cy);
	}

	return nMaxHeight + 2 * m_nYMargin + nVertMargin;
}

void CMFCRibbonPanel::Reposition(CDC* pDC, const CRect& rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	m_rectCaption.SetRectEmpty();
	m_rectMenuAreaTop.SetRectEmpty();
	m_rectMenuAreaBottom.SetRectEmpty();

	if (m_pPaletteButton != NULL)
	{
		ReposPalette(pDC, rect);
		return;
	}

	if (m_bMenuMode)
	{
		RepositionMenu(pDC, rect);
		return;
	}

	m_btnDefault.m_pParent = m_pParent;
	m_btnLaunch.m_pParent = m_pParent;
	m_btnLaunch.m_pParentPanel = this;

	m_btnDefault.OnCalcTextSize(pDC);
	const int cxDefaultButton = m_btnDefault.GetRegularSize(pDC).cx;

	m_rect = rect;

	m_btnLaunch.SetRect(CRect(0, 0, 0, 0));

	if (m_bForceCollpapse)
	{
		ASSERT(!m_bIsCalcWidth);
		ASSERT(!m_bIsQATPopup);

		ShowDefaultButton(pDC);
		return;
	}

	m_btnDefault.SetRect(CRect(0, 0, 0, 0));

	m_nFullWidth = 0;
	m_nRows = 0;
	m_bShowCaption = TRUE;

	const CSize sizeCaption = GetCaptionSize(pDC);

	if (!m_bTrancateCaption)
	{
		m_rect.right = m_rect.left + max(rect.Width(), sizeCaption.cx);
	}

	CSize size = rect.Size();
	size.cx -= m_nXMargin;
	size.cy -= sizeCaption.cy + m_nYMargin;

	// First, set all elements to the initial mode:
	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnCalcTextSize(pDC);

		if (!m_bFloatyMode)
		{
			pElem->SetInitialMode();
		}
		else
		{
			pElem->SetCompactMode(TRUE);
		}

		pElem->m_bFloatyMode = m_bFloatyMode;
		pElem->m_nRow = -1;
	}

	// Now, try to repos all elements inside bounds:
	int x = 0;
	int y = 0;

	if (!m_bAlignByColumn || m_bFloatyMode)
	{
		int nRowHeight = 0;
		int nBottom = 0;
		int xStart = 0;
		int i = 0;
		BOOL bIsFullHeight = (m_rect.bottom == 32767);
		int cx = size.cx;

		int cxFull = 0;

		if (!m_bIsCalcWidth && m_mapNonOptWidths.Lookup(cx, cxFull))
		{
			cx = cxFull;
		}

		CArray<int,int> arRowWidths;

		if (!m_bFloatyMode)
		{
			// Put all large buttons first:
			BOOL bPrevLarge = FALSE;
			BOOL bPrevSeparator = FALSE;

			CSize sizePrevLargeButton(0, 0);

			for (i = 0; i < m_arElements.GetSize(); i++)
			{
				CMFCRibbonBaseElement* pElem = m_arElements [i];
				ASSERT_VALID(pElem);

				CSize sizeElem = pElem->GetSize(pDC);

				BOOL bIsLargeButton = pElem->HasLargeMode() && !pElem->m_bCompactMode && !pElem->m_bIntermediateMode;

				BOOL bDrawSeparator = FALSE;

				if (pElem->IsSeparator())
				{
					bDrawSeparator = bPrevLarge && !bPrevSeparator;
				}

				if (bIsLargeButton || bDrawSeparator)
				{
					if (pElem->IsSeparator())
					{
						if (sizePrevLargeButton != CSize(0, 0))
						{
							sizeElem.cy = sizePrevLargeButton.cy;
						}
					}
					else
					{
						sizePrevLargeButton = sizeElem;
					}

					CRect rectElem(CPoint(rect.left + x + m_nXMargin, rect.top), CSize(sizeElem.cx, bIsFullHeight ? sizeElem.cy : size.cy));

					pElem->SetRect(rectElem);
					pElem->m_nRow = 999;

					x += sizeElem.cx + m_nXMargin;
					xStart = x;
					y = 0;
				}

				bPrevLarge = bIsLargeButton;
				bPrevSeparator = bDrawSeparator;
			}
		}

		for (i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			CSize sizeElem = pElem->GetSize(pDC);

			if (sizeElem == CSize(0, 0))
			{
				pElem->SetRect(CRect(0, 0, 0, 0));
				continue;
			}

			if (pElem->m_nRow != -1)
			{
				// Already positioned
				continue;
			}

			if (pElem->IsSeparator())
			{
				pElem->SetRect(CRect(0, 0, 0, 0));
				continue;
			}

			if (x + sizeElem.cx + m_nXMargin - 1 > cx)
			{
				// We should start next row now:

				if (x == xStart)
				{
					ShowDefaultButton(pDC);
					return;
				}

				y += nRowHeight;

				if (m_bFloatyMode)
				{
					y += m_nYMargin;
				}

				arRowWidths.Add(x);

				m_nRows++;

				x = xStart;
				nRowHeight = 0;
			}

			if (y + sizeElem.cy > size.cy)
			{
				// Cannot repos elemnts: panel is too small:
				ShowDefaultButton(pDC);
				return;
			}

			CRect rectElem(CPoint(rect.left + x + m_nXMargin, rect.top + y + m_nYMargin), sizeElem);

			pElem->SetRect(rectElem);
			pElem->m_nRow = m_nRows;

			nRowHeight = max(nRowHeight, sizeElem.cy);
			x += sizeElem.cx + m_nXMargin - 1;

			m_nFullWidth = max(m_nFullWidth, x - 1);

			nBottom = max(nBottom, rectElem.bottom);
		}

		arRowWidths.Add(x);
		m_nRows++;

		if (bIsFullHeight)
		{
			m_rect.bottom = nBottom + sizeCaption.cy + m_nYMargin;
			size.cy = m_rect.Height() - sizeCaption.cy - m_nYMargin;
		}

		if (!m_bIsQATPopup && m_nRows > 1)
		{
			// Optimize elemnents location:
			BOOL bRecalcFullWidth = FALSE;

			while (TRUE)
			{
				// Find widest row:
				int nMaxRowWidth = 0;
				int nMaxRowIndex = -1;

				for (i = 0; i < arRowWidths.GetSize(); i++)
				{
					if (arRowWidths [i] > nMaxRowWidth)
					{
						nMaxRowWidth = arRowWidths [i];
						nMaxRowIndex = i;
					}
				}

				if (nMaxRowIndex < 0)
				{
					break;
				}

				// Find smallest element in the widest row:
				int nMinWidth = 9999;
				CMFCRibbonBaseElement* pMinElem = NULL;

				for (i = 0; i < m_arElements.GetSize(); i++)
				{
					CMFCRibbonBaseElement* pElem = m_arElements [i];
					ASSERT_VALID(pElem);

					if (pElem->m_nRow == nMaxRowIndex)
					{
						CRect rectElem = pElem->GetRect();

						if (!rectElem.IsRectEmpty() && rectElem.Width() < nMinWidth)
						{
							nMinWidth = rectElem.Width();
							pMinElem = pElem;
						}
					}
				}

				if (pMinElem == NULL)
				{
					break;
				}

				// Try to move this elemnt to another row:
				BOOL bMoved = FALSE;

				for (i = nMaxRowIndex + 1; i < arRowWidths.GetSize(); i++)
				{
					if (arRowWidths [i] + nMinWidth < nMaxRowWidth)
					{
						// There is enough space in current row,
						// move element to here

						int x1 = 0;
						int y1 = 0;

						for (int j = 0; j < m_arElements.GetSize(); j++)
						{
							CMFCRibbonBaseElement* pElem = m_arElements [j];
							ASSERT_VALID(pElem);

							if (pElem->m_nRow == i)
							{
								x1 = max(pElem->GetRect().right + m_nXMargin, x1);
								y1 = pElem->GetRect().top;
							}
							else if (pElem->m_nRow == nMaxRowIndex)
							{
								CRect rectElem = pElem->GetRect();

								if (rectElem.left > pMinElem->GetRect().left)
								{
									rectElem.OffsetRect(-(nMinWidth + m_nXMargin), 0);
									pElem->SetRect(rectElem);
								}
							}
						}

						pMinElem->SetRect(CRect(CPoint(x1, y1), pMinElem->GetRect().Size()));
						pMinElem->m_nRow = i;

						arRowWidths [i] += nMinWidth;
						arRowWidths [nMaxRowIndex] -= nMinWidth;

						bRecalcFullWidth = TRUE;
						bMoved = TRUE;
						break;
					}
				}

				if (!bMoved)
				{
					break;
				}
			}

			if (bRecalcFullWidth)
			{
				m_nFullWidth = 0;

				for (i = 0; i < m_arElements.GetSize(); i++)
				{
					CMFCRibbonBaseElement* pElem = m_arElements [i];
					ASSERT_VALID(pElem);

					m_nFullWidth = max(m_nFullWidth, pElem->GetRect().right);
				}

				m_nFullWidth -= m_rect.left + m_nXMargin;
			}
		}

		if (!bIsFullHeight && !m_bFloatyMode && m_nRows > 1)
		{
			// Add some space between rows:
			int yOffset = (size.cy - m_nRows * nRowHeight) /(m_nRows + 1);
			if (yOffset > 0)
			{
				for (i = 0; i < m_arElements.GetSize(); i++)
				{
					CMFCRibbonBaseElement* pElem = m_arElements [i];
					ASSERT_VALID(pElem);

					int nRow = pElem->m_nRow;
					CRect rectElem = pElem->GetRect();

					if (nRow != 999 && !rectElem.IsRectEmpty())
					{
						rectElem.OffsetRect(0, yOffset *(nRow + 1) - nRow);
						pElem->SetRect(rectElem);
					}
				}
			}
		}

		if (m_bIsQATPopup && nRowHeight > 0 && m_arElements.GetSize() > 0)
		{
			// Last element(customize button) should occopy the whole row height:
			CMFCRibbonBaseElement* pElem = m_arElements [m_arElements.GetSize() - 1];
			ASSERT_VALID(pElem);

			CRect rectElem = pElem->GetRect();
			rectElem.bottom = rectElem.top + nRowHeight;

			pElem->SetRect(rectElem);
		}
	}
	else
	{
		const int nElementsInColumn = 3;

		while (TRUE)
		{
			int nColumnWidth = 0;
			int i = 0;
			x = 0;
			y = 0;

			CMap<int,int,int,int> mapColumnElements;

			for (i = 0; i < m_arElements.GetSize(); i++)
			{
				CMFCRibbonBaseElement* pElem = m_arElements [i];
				ASSERT_VALID(pElem);

				CSize sizeElem = pElem->GetSize(pDC);

				if (sizeElem == CSize(0, 0))
				{
					pElem->SetRect(CRect(0, 0, 0, 0));
					continue;
				}

				if (pElem->IsSeparator())
				{
					x += nColumnWidth;

					CRect rectSeparator(CPoint(rect.left + x + m_nXMargin, rect.top + m_nYMargin), CSize(sizeElem.cx, size.cy));

					pElem->SetRect(rectSeparator);

					x += sizeElem.cx + m_nXMargin;
					y = 0;
					nColumnWidth = 0;
					continue;
				}

				if (pElem->IsWholeRowHeight())
				{
					sizeElem.cy = size.cy;
				}

				if (y + sizeElem.cy > size.cy)
				{
					// We should start next column now:
					if (y == 0)
					{
						ShowDefaultButton(pDC);
						return;
					}

					x += nColumnWidth;
					y = 0;

					nColumnWidth = 0;
				}

				const int xColumn = rect.left + x + m_nXMargin;

				CRect rectElem(CPoint(xColumn, rect.top + y + m_nYMargin), sizeElem);
				pElem->SetRect(rectElem);

				int nCount = 1;

				if (mapColumnElements.Lookup(xColumn, nCount))
				{
					nCount++;
				}

				mapColumnElements.SetAt(xColumn, nCount);

				nColumnWidth = max(nColumnWidth, sizeElem.cx);
				y += sizeElem.cy;
			}

			const int nFullWidth = x + nColumnWidth;

			if (nFullWidth <= size.cx)
			{
				m_nFullWidth = nFullWidth;
				break;
			}

			if (nColumnWidth == 0)
			{
				ShowDefaultButton(pDC);
				return;
			}

			BOOL bChanged = FALSE;

			// Find element that can can be stretched horizontally:
			for (i = 0; i < m_arElements.GetSize() && !bChanged; i++)
			{
				CMFCRibbonBaseElement* pElem = m_arElements [i];
				ASSERT_VALID(pElem);

				if (!pElem->GetRect().IsRectEmpty() && pElem->CanBeStretchedHorizontally())
				{
					pElem->StretchHorizontally();
					bChanged = TRUE;
				}
			}

			if (bChanged)
			{
				continue;
			}

			// Find 'nElementsInColumn' large buttons and make them intermediate or compact:
			int nLargeCount = 0;
			int nLargeTotal = min(nElementsInColumn, (int) m_arElements.GetSize());

			for (i = (int) m_arElements.GetSize() - 1; !bChanged && i >= 0; i--)
			{
				CMFCRibbonBaseElement* pElem = m_arElements [i];
				ASSERT_VALID(pElem);

				if (pElem->GetRect().IsRectEmpty())
				{
					continue;
				}

				if (!pElem->IsLargeMode() || !pElem->CanBeCompacted())
				{
					nLargeCount = 0;
					continue;
				}

				nLargeCount++;

				if (nLargeCount == nLargeTotal)
				{
					bChanged = TRUE;

					for (int j = 0; j < nLargeCount; j++)
					{
						pElem = m_arElements [i + j];
						ASSERT_VALID(pElem);

						if (pElem->GetRect().IsRectEmpty())
						{
							j++;
						}
						else
						{
							pElem->SetCompactMode();
						}
					}
				}
			}

			if (bChanged)
			{
				continue;
			}

			// Find 'nElementsInColumn' intermediate buttons in one column and
			// make them compact:
			int nIntermediateCount = 0;
			int xColumn = -1;

			for (i = (int) m_arElements.GetSize() - 1; !bChanged && i >= 0; i--)
			{
				CMFCRibbonBaseElement* pElem = m_arElements [i];
				ASSERT_VALID(pElem);

				if (pElem->GetRect().IsRectEmpty())
				{
					continue;
				}

				if (xColumn != -1 && pElem->GetRect().left != xColumn)
				{
					nIntermediateCount = 0;
					xColumn = -1;
					continue;
				}

				xColumn = pElem->GetRect().left;

				if (!pElem->IsIntermediateMode() || !pElem->HasCompactMode())
				{
					nIntermediateCount = 0;
					xColumn = -1;
					continue;
				}

				nIntermediateCount++;

				if (nIntermediateCount == nElementsInColumn)
				{
					bChanged = TRUE;

					for (int j = 0; j < nIntermediateCount; j++)
					{
						pElem = m_arElements [i + j];
						ASSERT_VALID(pElem);

						if (pElem->GetRect().IsRectEmpty())
						{
							j++;
						}
						else
						{
							pElem->SetCompactMode();
						}
					}
				}
			}

			if (bChanged)
			{
				continue;
			}

			const int nStart = m_arElements.GetSize() < 3 ? 0 : 1;

			// Find 1 large button near intermediate and make it intermediate too:
			for (i = nStart; i < m_arElements.GetSize() && !bChanged; i++)
			{
				CMFCRibbonBaseElement* pElem = m_arElements [i];
				ASSERT_VALID(pElem);

				if (pElem->GetRect().IsRectEmpty())
				{
					continue;
				}

				if (!pElem->IsLargeMode() || !pElem->CanBeCompacted())
				{
					continue;
				}

				if (i < m_arElements.GetSize() - 1 && m_arElements [i + 1]->m_bIntermediateMode)
				{
					int nColumnElements = 0;

					if (mapColumnElements.Lookup(m_arElements [i + 1]->GetRect().left, nColumnElements) && nColumnElements < nElementsInColumn)
					{
						pElem->m_bIntermediateMode = TRUE;
						pElem->m_bCompactMode = FALSE;

						bChanged = TRUE;
					}
					break;
				}
			}

			if (bChanged)
			{
				continue;
			}

			// Last step - try to compact rest of elements:
			for (i = nStart; i < m_arElements.GetSize(); i++)
			{
				CMFCRibbonBaseElement* pElem = m_arElements [i];
				ASSERT_VALID(pElem);

				if (!pElem->GetRect().IsRectEmpty() && pElem->m_bIntermediateMode && pElem->HasCompactMode())
				{
					pElem->m_bIntermediateMode = FALSE;
					pElem->m_bCompactMode = TRUE;
					bChanged = TRUE;
				}
			}

			if (bChanged)
			{
				continue;
			}

			ShowDefaultButton(pDC);
			return;
		}
	}

	if (m_bFloatyMode)
	{
		return;
	}

	if (m_bAlignByColumn &&(m_bCenterColumnVert || m_bJustifyColumns))
	{
		int nFirstInColumnIndex = -1;
		int nLastInColumnIndex = -1;
		int xCurr = -1;

		for (int i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			CRect rectElem = pElem->m_rect;
			if (rectElem.IsRectEmpty())
			{
				continue;
			}

			if (nFirstInColumnIndex == -1)
			{
				nLastInColumnIndex = nFirstInColumnIndex = i;
				xCurr = rectElem.left;
			}

			if (xCurr != rectElem.left)
			{
				if (m_bCenterColumnVert)
				{
					CenterElementsInColumn(nFirstInColumnIndex, nLastInColumnIndex, sizeCaption.cy);
				}

				if (m_bJustifyColumns)
				{
					JustifyElementsInColumn(nFirstInColumnIndex, nLastInColumnIndex);
				}

				nLastInColumnIndex = nFirstInColumnIndex = i;
				xCurr = rectElem.left;
			}
			else
			{
				nLastInColumnIndex = i;
			}
		}

		if (m_bCenterColumnVert)
		{
			CenterElementsInColumn(nFirstInColumnIndex, nLastInColumnIndex, sizeCaption.cy);
		}

		if (m_bJustifyColumns)
		{
			JustifyElementsInColumn(nFirstInColumnIndex, nLastInColumnIndex);
		}
	}

	int nTotalWidth = !m_bAlignByColumn || m_bFloatyMode ? m_nFullWidth - 1 : CalcTotalWidth();

	if (nTotalWidth < sizeCaption.cx && !m_bTrancateCaption)
	{
		// Panel is too narrow: center it horizontaly:
		const int xOffset = (sizeCaption.cx - nTotalWidth) / 2;

		for (int i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			CRect rectElem = pElem->m_rect;
			rectElem.OffsetRect(xOffset, 0);

			pElem->SetRect(rectElem);
		}

		nTotalWidth = sizeCaption.cx;
	}

	if (nTotalWidth < cxDefaultButton)
	{
		m_rect.right = m_rect.left + cxDefaultButton + m_nXMargin;
	}
	else
	{
		m_rect.right = m_rect.left + nTotalWidth + 2 * m_nXMargin;
	}

	// Set launch button rectangle:
	if (m_btnLaunch.GetID() > 0)
	{
		CRect rectLaunch = m_rect;

		rectLaunch.DeflateRect(1, 1);

		rectLaunch.top = rectLaunch.bottom - sizeCaption.cy + 1;
		rectLaunch.left = rectLaunch.right - sizeCaption.cy;
		rectLaunch.bottom--;
		rectLaunch.right--;

		m_btnLaunch.SetRect(rectLaunch);
	}

	// Set caption rectangle:
	if (m_bShowCaption)
	{
		m_rectCaption = m_rect;
		m_rectCaption.top = m_rectCaption.bottom - sizeCaption.cy - 1;
	}
}

void CMFCRibbonPanel::RepositionMenu(CDC* pDC, const CRect& rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	m_bScrollDnAvailable = FALSE;

	m_nXMargin = 0;
	m_nYMargin = 0;

	CSize size = rect.Size();

	int y = 0;
	int i = 0;

	int nImageWidth = 0;

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);
		nImageWidth = m_pParent->GetImageSize(TRUE).cx;
	}

	int nColumnWidth = 0;

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (i < m_nScrollOffset)
		{
			pElem->SetRect(CRect(-1, -1, -1, -1));
			continue;
		}

		pElem->OnCalcTextSize(pDC);
		pElem->SetCompactMode(FALSE);
		pElem->SetTextAlwaysOnRight();

		CSize sizeElem = pElem->GetSize(pDC);

		if (sizeElem == CSize(0, 0))
		{
			pElem->SetRect(CRect(0, 0, 0, 0));
			continue;
		}

		if (!rect.IsRectEmpty())
		{
			sizeElem.cx = rect.Width();

			if (m_bIsDefaultMenuLook)
			{
				pElem->m_nImageOffset = CMFCToolBar::GetMenuImageSize().cx;
			}
		}

		CRect rectElem = CRect(CPoint(rect.left + m_nXMargin, rect.top + y + m_nYMargin), sizeElem);

		pElem->SetRect(rectElem);

		nColumnWidth = max(nColumnWidth, sizeElem.cx);
		y += sizeElem.cy;

		if (y > rect.bottom)
		{
			m_bScrollDnAvailable = TRUE;
		}
	}

	if (m_bIsDefaultMenuLook)
	{
		nColumnWidth += CMFCToolBar::GetMenuImageSize().cx + 2 * CMFCVisualManager::GetInstance()->GetMenuImageMargin();
	}

	m_nFullWidth = nColumnWidth;

	if (rect.IsRectEmpty())
	{
		// All menu elements should have the same width:
		for (i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			CRect rectElem = pElem->GetRect();

			if (!rectElem.IsRectEmpty())
			{
				rectElem.right = rectElem.left + nColumnWidth;

				if (nImageWidth > 0 && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonSeparator)))
				{
					rectElem.left += nImageWidth + m_nXMargin;
				}

				pElem->SetRect(rectElem);
			}

			pElem->OnAfterChangeRect(pDC);
		}
	}

	m_rect = rect;
	m_rect.bottom = m_rect.top + y;
	m_rect.right = m_rect.left + m_nFullWidth;
}

void CMFCRibbonPanel::ReposPalette(CDC* pDC, const CRect& rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pScrollBar);
	ASSERT_VALID(m_pPaletteButton);

	const int yOffset = 4;

	if (rect == CRect(0, 0, 0, 0))
	{
		return;
	}

	const BOOL bNoSideBar = m_pPaletteButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonUndoButton));

	BOOL bShowAllItems = FALSE;

	m_nScrollOffset = 0;

	CRect rectInitial = rect;

	if (rectInitial.bottom <= 0)
	{
		rectInitial.bottom = rectInitial.top + 32676;
		bShowAllItems = TRUE;
	}

	m_nXMargin = 0;
	m_nYMargin = 0;

	const int cxScroll = bShowAllItems && !m_pPaletteButton->IsMenuResizeEnabled() ? 0 : ::GetSystemMetrics(SM_CXVSCROLL);

	int nScrollTotal = 0;

	int x = rectInitial.left;
	int y = rectInitial.top;

	m_rect = rectInitial;

	if (m_bSizeIsLocked)
	{
		rectInitial.right -= cxScroll;
	}
	else
	{
		m_rect.right += cxScroll;
	}

	int i = 0;
	BOOL bHasBottomItems = FALSE;
	BOOL bHasTopItems = FALSE;

	int nMaxItemHeight = 0;
	int nMaxImageWidth = 0;

	if (m_bSizeIsLocked)
	{
		// First, find all menu items and place them on top/bottom:

		int yTop = rectInitial.top;
		int yBottom = rectInitial.bottom;
		m_rectMenuAreaTop = m_rect;
		m_rectMenuAreaBottom = m_rect;

		for (i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			BOOL bIsLabel = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel));
			BOOL bIsIcon = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon));

			if (bIsIcon || bIsLabel)
			{
				continue;
			}

			if (pElem->m_bIsOnPaletteTop)
			{
				bHasTopItems = TRUE;
			}
			else
			{
				bHasBottomItems = TRUE;
			}

			pElem->OnCalcTextSize(pDC);
			pElem->SetCompactMode(TRUE);

			if (pElem->GetImageSize(CMFCRibbonBaseElement::RibbonImageLarge) == CSize(0, 0))
			{
				pElem->SetCompactMode(FALSE);
			}

			pElem->SetTextAlwaysOnRight();

			CSize sizeElem = pElem->GetSize(pDC);
			sizeElem.cx = m_rect.Width();

			CRect rectElem(0, 0, 0, 0);

			if (pElem->m_bIsOnPaletteTop)
			{
				rectElem = CRect(CPoint(rectInitial.left, yTop), sizeElem);

				yTop += sizeElem.cy;
				rectInitial.top = yTop + yOffset;
				m_rectMenuAreaTop.bottom = yTop;

				y += sizeElem.cy;
			}
			else
			{
				rectElem = CRect(CPoint(rectInitial.left, yBottom - sizeElem.cy), sizeElem);

				yBottom -= sizeElem.cy;
				rectInitial.bottom = yBottom - yOffset;
				m_rectMenuAreaBottom.top = yBottom;
			}

			pElem->SetRect(rectElem);
		}
	}
	else
	{
		// Reposition all top items:
		m_rectMenuAreaTop = m_rect;

		for (i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			if (pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon)) || pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel)))
			{
				continue;
			}

			pElem->OnCalcTextSize(pDC);
			pElem->SetCompactMode(TRUE);

			if (pElem->GetImageSize(CMFCRibbonBaseElement::RibbonImageLarge) == CSize(0, 0))
			{
				pElem->SetCompactMode(FALSE);
			}

			pElem->SetTextAlwaysOnRight();

			CSize sizeElem = pElem->GetSize(pDC);
			sizeElem.cx += 2 * AFX_TEXT_MARGIN;

			nMaxItemHeight = max(nMaxItemHeight, sizeElem.cy);
			nMaxImageWidth = max(nMaxImageWidth, pElem->GetImageSize(CMFCRibbonBaseElement::RibbonImageSmall).cx);
		}

		if (nMaxImageWidth == 0 && !bNoSideBar)
		{
			nMaxImageWidth = CMFCToolBar::GetMenuImageSize().cx + 2 * CMFCVisualManager::GetInstance()->GetMenuImageMargin();

			if (m_pParent != NULL)
			{
				nMaxImageWidth = m_pParent->GetImageSize(FALSE).cx;
			}
		}

		for (i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			if (pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon)) || pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel)) || !pElem->m_bIsOnPaletteTop)
			{
				continue;
			}

			CSize sizeElem = pElem->GetSize(pDC);

			if (sizeElem == CSize(0, 0))
			{
				pElem->SetRect(CRect(0, 0, 0, 0));
				continue;
			}

			pElem->m_nImageOffset = nMaxImageWidth;

			sizeElem.cx = m_rect.Width();
			sizeElem.cy = nMaxItemHeight;

			CRect rectElem = CRect(CPoint(rectInitial.left, rectInitial.top + y), sizeElem);

			pElem->SetRect(rectElem);

			y += sizeElem.cy;
		}

		m_rectMenuAreaTop.bottom = y;
	}

	// Set palette icons location:
	int yNextLine = m_rect.bottom;
	BOOL bIsFirstPaletteElem = TRUE;

	if (!m_bSizeIsLocked)
	{
		m_rect.bottom = y;
	}

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		BOOL bIsLabel = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel));
		BOOL bIsIcon = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon));

		if (!bIsIcon && !bIsLabel)
		{
			if (pElem->m_bIsOnPaletteTop)
			{
				bHasTopItems = TRUE;
			}
			else
			{
				bHasBottomItems = TRUE;
			}
		}

		CSize sizeElem(0, 0);

		if (bIsLabel)
		{
			if (x > rectInitial.left)
			{
				y = yNextLine;
			}

			if (i > 0)
			{
				y++;
			}

			CString strLabel = pElem->GetText();
			CRect rectElem(0, 0, 0, 0);

			if (strLabel.IsEmpty())
			{
				if (!bIsFirstPaletteElem)
				{
					y += m_pPaletteButton->GetGroupOffset();
				}
			}
			else
			{
				pElem->OnCalcTextSize(pDC);

				sizeElem = pElem->GetSize(pDC);
				sizeElem.cx = rectInitial.Width();

				rectElem = CRect(CPoint(rectInitial.left, y), sizeElem);

				y += sizeElem.cy + m_pPaletteButton->GetGroupOffset();

				bIsFirstPaletteElem = FALSE;
			}

			pElem->SetRect(rectElem);

			if (!m_bSizeIsLocked)
			{
				m_rect.bottom = rectElem.bottom;
			}

			nScrollTotal = yNextLine = rectElem.bottom;

			x = rectInitial.left;
		}
		else if (bIsIcon)
		{
			bIsFirstPaletteElem = FALSE;

			pElem->SetCompactMode(FALSE);

			sizeElem = pElem->GetSize(pDC);

			if (x + sizeElem.cx > rectInitial.right && x != rectInitial.left)
			{
				x = rectInitial.left;
				y += sizeElem.cy;
			}

			CRect rectElem = CRect(CPoint(x, y), sizeElem);

			pElem->SetRect(rectElem);

			if (!m_bSizeIsLocked)
			{
				m_rect.bottom = rectElem.bottom;
			}

			nScrollTotal = yNextLine = rectElem.bottom;

			x += sizeElem.cx;
		}
	}

	if (!m_bSizeIsLocked)
	{
		m_rect.bottom = min(m_rect.bottom, rectInitial.bottom);
	}

	m_nFullWidth = m_rect.Width();

	if (bHasBottomItems && !m_bSizeIsLocked)
	{
		// Set menu items location(on bottom):
		y = m_rect.bottom + yOffset;
		m_rectMenuAreaBottom = m_rect;
		m_rectMenuAreaBottom.top = y;

		for (i = 0; i < m_arElements.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pElem = m_arElements [i];
			ASSERT_VALID(pElem);

			if (pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon)) || pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel)) || pElem->m_bIsOnPaletteTop)
			{
				continue;
			}

			CSize sizeElem = pElem->GetSize(pDC);

			if (sizeElem == CSize(0, 0))
			{
				pElem->SetRect(CRect(0, 0, 0, 0));
				continue;
			}

			pElem->m_nImageOffset = nMaxImageWidth;

			sizeElem.cx = m_rect.Width();
			sizeElem.cy = nMaxItemHeight;

			CRect rectElem = CRect
				(CPoint(rectInitial.left, rectInitial.top + y), sizeElem);

			m_rect.bottom = rectElem.bottom;

			pElem->SetRect(rectElem);

			y += sizeElem.cy;
		}

		m_rectMenuAreaBottom.bottom = y;
	}

	if (!bHasBottomItems)
	{
		m_rectMenuAreaBottom.SetRectEmpty();
	}

	if (!bHasTopItems)
	{
		m_rectMenuAreaTop.SetRectEmpty();
	}

	// Define icon postions in matrix:
	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonGalleryIcon* pIcon = DYNAMIC_DOWNCAST(CMFCRibbonGalleryIcon, m_arElements [i]);
		if (pIcon == NULL)
		{
			continue;
		}

		ASSERT_VALID(pIcon);

		pIcon->m_bIsFirstInRow = FALSE;
		pIcon->m_bIsLastInRow = FALSE;
		pIcon->m_bIsFirstInColumn = FALSE;
		pIcon->m_bIsLastInColumn = FALSE;

		CRect rectIcon = pIcon->GetRect();

		if (rectIcon.IsRectEmpty())
		{
			continue;
		}

		pIcon->m_bIsFirstInRow = DYNAMIC_DOWNCAST(CMFCRibbonGalleryIcon, HitTest(CPoint(rectIcon.left - 2, rectIcon.CenterPoint().y))) == NULL;
		pIcon->m_bIsLastInRow = DYNAMIC_DOWNCAST(CMFCRibbonGalleryIcon, HitTest(CPoint(rectIcon.right + 2, rectIcon.CenterPoint().y))) == NULL;
		pIcon->m_bIsFirstInColumn = DYNAMIC_DOWNCAST(CMFCRibbonGalleryIcon, HitTest(CPoint(rectIcon.CenterPoint().x, rectIcon.top - 2))) == NULL;
		pIcon->m_bIsLastInColumn = DYNAMIC_DOWNCAST(CMFCRibbonGalleryIcon, HitTest(CPoint(rectIcon.CenterPoint().x, rectIcon.bottom + 2))) == NULL;
	}

	if (!bShowAllItems || m_pPaletteButton->IsMenuResizeEnabled())
	{
		const int ySB = bHasTopItems ? m_rectMenuAreaTop.bottom + 1 : rectInitial.top;
		const int ySBBottom = bHasBottomItems ? m_rectMenuAreaBottom.top - 1 : m_rect.bottom;

		m_pScrollBar->SetWindowPos(NULL, m_rect.right - cxScroll, ySB, cxScroll, ySBBottom - ySB - 1, SWP_NOZORDER | SWP_NOACTIVATE);

		SCROLLINFO si;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;

		si.nMin = 0;

		nScrollTotal -= m_rectMenuAreaTop.Height();

		if (nScrollTotal > rectInitial.Height())
		{
			si.nMax = nScrollTotal;
			si.nPage = rectInitial.Height();

			m_pScrollBar->SetScrollInfo(&si, TRUE);
			m_pScrollBar->EnableScrollBar(ESB_ENABLE_BOTH);
			m_pScrollBar->EnableWindow();
		}
		else if (!bShowAllItems)
		{
			m_pScrollBar->EnableScrollBar(ESB_DISABLE_BOTH);
		}
	}
}

void CMFCRibbonPanel::ShowDefaultButton(CDC* pDC)
{
	// Show panel default button only
	const int cxDefaultButton = m_btnDefault.GetRegularSize(pDC).cx;

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->SetRect(CRect(0, 0, 0, 0));
	}

	m_rect.right = m_rect.left + cxDefaultButton;

	m_btnDefault.SetRect(m_rect);
	m_nRows = 0;
	m_bShowCaption = FALSE;
	m_bForceCollpapse = FALSE;
}

int CMFCRibbonPanel::CalcTotalWidth()
{
	// Total width will be right edge of the last visible element
	// in the right column
	int xRight = 0;

	for (int i = (int) m_arElements.GetSize() - 1; i >= 0; i--)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (pElem->m_rect.IsRectEmpty())
		{
			continue;
		}

		xRight = max(xRight, pElem->m_rect.right);
	}

	return max(0, xRight - m_rect.left - m_nXMargin / 2 - 1);
}

CSize CMFCRibbonPanel::GetCaptionSize(CDC* pDC) const
{
	ASSERT_VALID(pDC);

	if (m_bFloatyMode)
	{
		return CSize(0, 0);
	}

	CSize size = pDC->GetTextExtent(m_strName);

	size.cy += m_nYMargin + 1;

	if (m_btnLaunch.GetID() > 0)
	{
		size.cx += size.cy + 1;
	}

	return size;
}

void CMFCRibbonPanel::CenterElementsInColumn(int nFirstInColumnIndex, int nLastInColumnIndex, int nCaptionHeight)
{
	if (nFirstInColumnIndex > nLastInColumnIndex || nFirstInColumnIndex < 0 || nLastInColumnIndex < 0)
	{
		return;
	}

	// Center all elements in column vertically:
	CMFCRibbonBaseElement* pLastElem = m_arElements [nLastInColumnIndex];
	ASSERT_VALID(pLastElem);

	const int nColumnHeight = m_rect.Height() - nCaptionHeight - 2 * m_nYMargin;
	const int nTotalHeight = pLastElem->m_rect.bottom - m_rect.top - m_nYMargin;
	const int nOffset = max(0, (nColumnHeight - nTotalHeight) / 2);

	for (int i = nFirstInColumnIndex; i <= nLastInColumnIndex; i++)
	{
		CMFCRibbonBaseElement* pColumnElem = m_arElements [i];
		ASSERT_VALID(pColumnElem);

		CRect rectElem = pColumnElem->m_rect;
		rectElem.OffsetRect(0, nOffset);
		pColumnElem->SetRect(rectElem);
	}
}

void CMFCRibbonPanel::JustifyElementsInColumn(int nFirstInColumnIndex, int nLastInColumnIndex)
{
	if (nFirstInColumnIndex > nLastInColumnIndex || nFirstInColumnIndex < 0 || nLastInColumnIndex < 0)
	{
		return;
	}

	// Set same width(largets) to all column elements:
	int nColumnWidth = 0;
	int i = 0;

	for (i = nFirstInColumnIndex; i <= nLastInColumnIndex; i++)
	{
		CMFCRibbonBaseElement* pColumnElem = m_arElements [i];
		ASSERT_VALID(pColumnElem);

		nColumnWidth = max(nColumnWidth, pColumnElem->m_rect.Width());
	}

	for (i = nFirstInColumnIndex; i <= nLastInColumnIndex; i++)
	{
		CMFCRibbonBaseElement* pColumnElem = m_arElements [i];
		ASSERT_VALID(pColumnElem);

		CRect rectElem = pColumnElem->m_rect;
		rectElem.right = rectElem.left + nColumnWidth;

		pColumnElem->SetRect(rectElem);
	}
}

void CMFCRibbonPanel::RecalcWidths(CDC* pDC, int nHeight)
{
	ASSERT_VALID(pDC);

	CRect rectScreen;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);

	m_btnDefault.OnCalcTextSize(pDC);

	const int cxDefaultButton = m_btnDefault.GetRegularSize(pDC).cx;

	m_arWidths.RemoveAll();
	m_mapNonOptWidths.RemoveAll();

	m_nCurrWidthIndex = 0;
	m_bIsCalcWidth = TRUE;

	int nLastWidth = -1;
	const int dx = 16;

	if (m_bAlignByColumn && !m_bFloatyMode)
	{
		CRect rect(0, 0, 32767, nHeight);

		do
		{
			Reposition(pDC, rect);

			if (!m_bShowCaption)
			{
				break;
			}

			if (nLastWidth == -1 || m_nFullWidth < nLastWidth)
			{
				nLastWidth = m_nFullWidth;

				if (nLastWidth <= cxDefaultButton ||
					(nLastWidth <= 3 * cxDefaultButton / 2 && m_arElements.GetSize() == 1))
				{
					if (m_arWidths.GetSize() == 0)
					{
						// Panel has onle one layout and it smaller then collapsed.
						// Use this layout only and don't allow to collapse the panel
						m_arWidths.Add(nLastWidth);
						m_bIsCalcWidth = FALSE;
						return;
					}

					break;
				}

				m_arWidths.Add(nLastWidth);
				rect.right = nLastWidth - dx;
			}
			else
			{
				rect.right -= dx;
			}
		}
		while (rect.Width() > 2 * m_nXMargin);
	}
	else if (m_bIsQATPopup)
	{
		CRect rect(0, 0, rectScreen.Width() - 10, nHeight);

		Reposition(pDC, rect);
		m_arWidths.Add(m_nFullWidth);
	}
	else
	{
		const int nMaxRows = m_bIsQATPopup ? 50 : 3;

		for (int nRows = 1; nRows <= nMaxRows; nRows++)
		{
			CRect rect(0, 0, cxDefaultButton + 1, nHeight);

			for (;; rect.right += dx)
			{
				if (rect.Width() >= rectScreen.Width())
				{
					break;
				}

				Reposition(pDC, rect);

				if (nLastWidth != -1 && m_nFullWidth > nLastWidth)
				{
					break;
				}

				if (m_nRows == nRows && m_nFullWidth > 0)
				{
					if (m_nRows == nMaxRows - 1 && !m_bFloatyMode)
					{
						// Don't add 1 row layout:
						m_arWidths.RemoveAll();
					}

					m_arWidths.Add(m_nFullWidth);
					m_mapNonOptWidths.SetAt(m_nFullWidth + m_nXMargin, rect.Width());
					nLastWidth = m_nFullWidth;
					break;
				}
			}
		}
	}

	m_arWidths.Add(cxDefaultButton);

	m_bIsCalcWidth = FALSE;
}

int CMFCRibbonPanel::GetMinWidth(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	m_btnDefault.OnCalcTextSize(pDC);
	return m_btnDefault.GetRegularSize(pDC).cx;
}

void CMFCRibbonPanel::Highlight(BOOL bHighlight, CPoint point)
{
	ASSERT_VALID(this);

	BOOL bRedrawPanel = m_bIsHighlighted != bHighlight;

	m_bIsHighlighted = bHighlight;

	CMFCRibbonBaseElement* pHighlighted = NULL;
	if (bHighlight)
	{
		pHighlighted = HitTest(point);

		if (pHighlighted != NULL)
		{
			ASSERT_VALID(pHighlighted);
			pHighlighted->OnMouseMove(point);
		}
	}

	BOOL bNotifyParent = FALSE;

	if (pHighlighted != m_pHighlighted)
	{
		if (m_pParent != NULL && m_pParent->GetParentRibbonBar() != NULL)
		{
			m_pParent->GetParentRibbonBar()->PopTooltip();
		}

		if (m_pParentMenuBar != NULL)
		{
			ASSERT_VALID(m_pParentMenuBar);
			m_pParentMenuBar->PopTooltip();
		}

		if (m_pHighlighted != NULL)
		{
			ASSERT_VALID(m_pHighlighted);
			m_pHighlighted->m_bIsHighlighted = FALSE;
			m_pHighlighted->OnHighlight(FALSE);
			m_pHighlighted->m_bIsFocused = FALSE;
			m_pHighlighted->OnSetFocus(FALSE);

			RedrawElement(m_pHighlighted);
			m_pHighlighted = NULL;
		}

		bNotifyParent = TRUE;
	}

	if (pHighlighted != NULL)
	{
		ASSERT_VALID(pHighlighted);

		m_pHighlighted = pHighlighted;

		if (!m_pHighlighted->m_bIsHighlighted)
		{
			m_pHighlighted->OnHighlight(TRUE);
			m_pHighlighted->m_bIsHighlighted = TRUE;
			RedrawElement(m_pHighlighted);
		}
	}

	if (bRedrawPanel && m_pParent != NULL && GetParentWnd() != NULL)
	{
		GetParentWnd()->RedrawWindow(m_rect);
	}

	if (m_bFloatyMode && bRedrawPanel)
	{
		ASSERT_VALID(m_pParentMenuBar);
		m_pParentMenuBar->SetActive(m_bIsHighlighted);
	}

	if (bNotifyParent)
	{
		if (m_pParentMenuBar != NULL)
		{
			ASSERT_VALID(m_pParentMenuBar);
			m_pParentMenuBar->OnChangeHighlighted(m_pHighlighted);
		}
	}
}

CMFCRibbonBaseElement* CMFCRibbonPanel::HitTest(CPoint point, BOOL bCheckPanelCaption)
{
	if (!m_btnDefault.m_rect.IsRectEmpty() && m_btnDefault.m_rect.PtInRect(point))
	{
		return &m_btnDefault;
	}

	if (!m_btnLaunch.m_rect.IsRectEmpty() && m_btnLaunch.m_rect.PtInRect(point))
	{
		return &m_btnLaunch;
	}

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (!pElem->m_rect.IsRectEmpty() && pElem->m_rect.PtInRect(point))
		{
			return pElem->HitTest(point);
		}
	}

	if (bCheckPanelCaption && m_rectCaption.PtInRect(point))
	{
		return &m_btnDefault;
	}

	return NULL;
}

int CMFCRibbonPanel::HitTestEx(CPoint point) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (!pElem->m_rect.IsRectEmpty() && pElem->m_rect.PtInRect(point))
		{
			return i;
		}
	}

	return -1;
}

int CMFCRibbonPanel::GetIndex(CMFCRibbonBaseElement* pElem)  const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pNextElem = m_arElements [i];
		ASSERT_VALID(pNextElem);

		if (pNextElem == pElem)
		{
			return i;
		}
	}

	return -1;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::MouseButtonDown(CPoint point)
{
	ASSERT_VALID(this);

	if (m_pHighlighted != NULL)
	{
		ASSERT_VALID (m_pHighlighted);

		BOOL bSetPressed = TRUE;

		if (m_pHighlighted->HasMenu ())
		{
			CMFCRibbonButton* pButton = DYNAMIC_DOWNCAST (CMFCRibbonButton, m_pHighlighted);
			if (pButton != NULL)
			{
				ASSERT_VALID (pButton);

				const CRect rectCmd = pButton->GetCommandRect ();
				bSetPressed = !rectCmd.IsRectEmpty () && rectCmd.PtInRect (point);
			}
		}

		if (bSetPressed)
		{
			m_pHighlighted->m_bIsPressed = TRUE;
			RedrawElement (m_pHighlighted);
		}

		HWND hwndMenu = m_pParentMenuBar->GetSafeHwnd();

		m_pHighlighted->OnLButtonDown(point);

		if (hwndMenu != NULL && !::IsWindow(hwndMenu))
		{
			return NULL;
		}
	}

	return m_pHighlighted;
}

void CMFCRibbonPanel::MouseButtonUp(CPoint point)
{
	ASSERT_VALID(this);

	if (m_pHighlighted != NULL)
	{
		ASSERT_VALID(m_pHighlighted);

		HWND hwndParent = GetParentWnd()->GetSafeHwnd();

		CMFCRibbonBaseElement* pHighlighted = m_pHighlighted;
		m_pHighlighted->OnLButtonUp(point);

		if (::IsWindow(hwndParent) && pHighlighted->m_bIsPressed)
		{
			pHighlighted->m_bIsPressed = FALSE;
			RedrawElement(pHighlighted);

			if (m_pHighlighted != NULL && m_pHighlighted != pHighlighted)
			{
				RedrawElement(m_pHighlighted);
			}
		}
	}
}

void CMFCRibbonPanel::CancelMode()
{
	ASSERT_VALID(this);

	if (m_pHighlighted != NULL)
	{
		ASSERT_VALID(m_pHighlighted);

		m_pHighlighted->m_bIsHighlighted = FALSE;
		m_pHighlighted->OnHighlight(FALSE);
		m_pHighlighted->m_bIsPressed = FALSE;
		m_pHighlighted->m_bIsFocused = FALSE;
		m_pHighlighted->OnSetFocus(FALSE);

		RedrawElement(m_pHighlighted);
		m_pHighlighted = NULL;
	}

	if (m_bIsHighlighted)
	{
		m_bIsHighlighted = FALSE;

		if (GetParentWnd()->GetSafeHwnd() != NULL)
		{
			GetParentWnd()->RedrawWindow(m_rect);
		}
	}
}

void CMFCRibbonPanel::OnUpdateCmdUI(CMFCRibbonCmdUI* pCmdUI, CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnUpdateCmdUI(pCmdUI, pTarget, bDisableIfNoHndler);
	}

	m_btnLaunch.OnUpdateCmdUI(pCmdUI, pTarget, bDisableIfNoHndler);
}

BOOL CMFCRibbonPanel::NotifyControlCommand(BOOL bAccelerator, int nNotifyCode, WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (pElem->NotifyControlCommand(bAccelerator, nNotifyCode, wParam, lParam))
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMFCRibbonPanel::OnAfterChangeRect(CDC* pDC)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnShow(!pElem->GetRect().IsRectEmpty());
		pElem->OnAfterChangeRect(pDC);
	}

	m_btnDefault.OnShow(!m_btnDefault.GetRect().IsRectEmpty());
	m_btnDefault.OnAfterChangeRect(pDC);

	m_btnLaunch.OnAfterChangeRect(pDC);
}

void CMFCRibbonPanel::OnShow(BOOL bShow)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnShow(bShow && !pElem->GetRect().IsRectEmpty());
	}
}

BOOL CMFCRibbonPanel::IsCollapsed() const
{
	ASSERT_VALID(this);

	return !m_btnDefault.GetRect().IsRectEmpty();
}

CMFCRibbonBaseElement* CMFCRibbonPanel::FindByID(UINT uiCmdID) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElemFromList = m_arElements [i];
		ASSERT_VALID(pElemFromList);

		CMFCRibbonBaseElement* pElem = pElemFromList->FindByID(uiCmdID);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	CMFCRibbonBaseElement* pElem = ((CMFCRibbonPanel*) this)->m_btnLaunch.FindByID(uiCmdID);
	if (pElem != NULL)
	{
		ASSERT_VALID(pElem);
		return pElem;
	}

	if (m_btnDefault.GetID() == uiCmdID)
	{
		return &(((CMFCRibbonPanel*) this)->m_btnDefault);
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::FindByData(DWORD_PTR dwData) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElemFromList = m_arElements [i];
		ASSERT_VALID(pElemFromList);

		CMFCRibbonBaseElement* pElem = pElemFromList->FindByData(dwData);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	CMFCRibbonBaseElement* pElem = ((CMFCRibbonPanel*) this)->m_btnLaunch.FindByData(dwData);
	if (pElem != NULL)
	{
		ASSERT_VALID(pElem);
		return pElem;
	}

	if (m_btnDefault.GetData() == dwData)
	{
		return &(((CMFCRibbonPanel*) this)->m_btnDefault);
	}

	return NULL;
}

BOOL CMFCRibbonPanel::SetElementMenu(UINT uiCmdID, HMENU hMenu, BOOL bIsDefautCommand, BOOL bRightAlign)
{
	ASSERT_VALID(this);

	CMFCRibbonButton* pButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, FindByID(uiCmdID));

	if (pButton == NULL)
	{
		TRACE(_T("Cannot find element with ID: %d\n"), uiCmdID);
		return FALSE;
	}

	ASSERT_VALID(pButton);
	pButton->SetMenu(hMenu, bIsDefautCommand, bRightAlign);

	return TRUE;
}

BOOL CMFCRibbonPanel::SetElementMenu(UINT uiCmdID, UINT uiMenuResID, BOOL bIsDefautCommand, BOOL bRightAlign)
{
	ASSERT_VALID(this);

	CMFCRibbonButton* pButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, FindByID(uiCmdID));

	if (pButton == NULL)
	{
		TRACE(_T("Cannot find element with ID: %d\n"), uiCmdID);
		return FALSE;
	}

	ASSERT_VALID(pButton);
	pButton->SetMenu(uiMenuResID, bIsDefautCommand, bRightAlign);

	return TRUE;
}

BOOL CMFCRibbonPanel::Replace(int nIndex, CMFCRibbonBaseElement* pElem)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	if (nIndex < 0 || nIndex >= m_arElements.GetSize())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCRibbonBaseElement* pOldElem = m_arElements [nIndex];
	ASSERT_VALID(pOldElem);

	pElem->CopyFrom(*pOldElem);
	m_arElements [nIndex] = pElem;

	delete pOldElem;
	return TRUE;
}

BOOL CMFCRibbonPanel::ReplaceByID(UINT uiCmdID, CMFCRibbonBaseElement* pElem)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	if (uiCmdID == 0 || uiCmdID == (UINT)-1)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElemFromList = m_arElements [i];
		ASSERT_VALID(pElemFromList);

		if (pElemFromList->GetID() == uiCmdID)
		{
			return Replace(i, pElem);
		}

		if (pElemFromList->ReplaceByID(uiCmdID, pElem))
		{
			return TRUE;
		}
	}

	return FALSE;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::SetElementRTC(int nIndex, CRuntimeClass* pRTC)
{
	ASSERT_VALID(this);
	ENSURE(pRTC != NULL);

	if (!pRTC->IsDerivedFrom(RUNTIME_CLASS(CMFCRibbonBaseElement)))
	{
		ASSERT(FALSE);
		return NULL;
	}

	CMFCRibbonBaseElement* pNewElement = DYNAMIC_DOWNCAST(CMFCRibbonBaseElement, pRTC->CreateObject());
	ASSERT_VALID(pNewElement);

	if (!Replace(nIndex, pNewElement))
	{
		delete pNewElement;
	}

	return pNewElement;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::SetElementRTCByID(UINT uiCmdID, CRuntimeClass* pRTC)
{
	ASSERT_VALID(this);
	ENSURE(pRTC != NULL);

	if (!pRTC->IsDerivedFrom(RUNTIME_CLASS(CMFCRibbonBaseElement)))
	{
		ASSERT(FALSE);
		return NULL;
	}

	CMFCRibbonBaseElement* pNewElement = DYNAMIC_DOWNCAST(CMFCRibbonBaseElement, pRTC->CreateObject());
	ASSERT_VALID(pNewElement);

	if (!ReplaceByID(uiCmdID, pNewElement))
	{
		delete pNewElement;
	}

	return pNewElement;
}

CWnd* CMFCRibbonPanel::GetParentWnd() const
{
	ASSERT_VALID(this);

	CWnd* pParentWnd = NULL;

	if (m_pParentMenuBar != NULL)
	{
		ASSERT_VALID(m_pParentMenuBar);
		pParentWnd = m_pParentMenuBar;
	}
	else if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);
		pParentWnd = m_pParent->GetParentRibbonBar();
	}

	return pParentWnd;
}

void CMFCRibbonPanel::RedrawElement(CMFCRibbonBaseElement* pElem)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	const CRect rectElem = pElem->GetRect();

	if (rectElem.IsRectEmpty())
	{
		return;
	}

	CWnd* pParentWnd = GetParentWnd();

	if (pParentWnd->GetSafeHwnd() != NULL)
	{
		ASSERT_VALID(pParentWnd);

		pParentWnd->InvalidateRect(rectElem);
		pParentWnd->UpdateWindow();
	}
}

BOOL CMFCRibbonPanel::HasElement(const CMFCRibbonBaseElement* pElem) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElemFromList = m_arElements [i];
		ASSERT_VALID(pElemFromList);

		if (pElemFromList->Find(pElem))
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMFCRibbonPanel::GetElements(CArray <CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->GetElements(arElements);
	}

	if (m_btnLaunch.GetID() > 0)
	{
		arElements.Add(&m_btnLaunch);
	}

	if (!IsMainPanel())
	{
		arElements.Add(&m_btnDefault);
	}
}

void CMFCRibbonPanel::GetItemIDsList(CList<UINT,UINT>& lstItems) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->GetItemIDsList(lstItems);
	}

	m_btnDefault.GetItemIDsList(lstItems);
}

void CMFCRibbonPanel::GetElementsByID(UINT uiCmdID, CArray <CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->GetElementsByID(uiCmdID, arElements);
	}

	m_btnDefault.GetElementsByID(uiCmdID, arElements);
	m_btnLaunch.GetElementsByID(uiCmdID, arElements);
}

CMFCRibbonBaseElement* CMFCRibbonPanel::GetPressed() const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		CMFCRibbonBaseElement* pPressedElem = pElem->GetPressed();
		if (pPressedElem != NULL)
		{
			ASSERT_VALID(pPressedElem);
			return pPressedElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::GetDroppedDown() const
{
	ASSERT_VALID(this);

	if (!m_btnDefault.m_rect.IsRectEmpty())
	{
		CMFCRibbonBaseElement* pDroppedElem =
			((CMFCRibbonPanel*) this)->m_btnDefault.GetDroppedDown();

		if (pDroppedElem != NULL)
		{
			ASSERT_VALID(pDroppedElem);
			return pDroppedElem;
		}
	}

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		CMFCRibbonBaseElement* pDroppedElem = pElem->GetDroppedDown();
		if (pDroppedElem != NULL)
		{
			ASSERT_VALID(pDroppedElem);
			return pDroppedElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::GetHighlighted() const
{
	ASSERT_VALID(this);
	return m_pHighlighted;
}

BOOL CMFCRibbonPanel::OnKey(UINT nChar)
{
	ASSERT_VALID(this);

	if (m_arElements.GetSize() == NULL)
	{
		return FALSE;
	}

	if (m_pHighlighted != NULL)
	{
		ASSERT_VALID(m_pHighlighted);

		if (m_pHighlighted->OnProcessKey(nChar))
		{
			return TRUE;
		}
	}

	if (nChar == VK_TAB)
	{
		if (::GetKeyState(VK_SHIFT) & 0x80)
		{
			nChar = VK_UP;
		}
		else
		{
			nChar = VK_DOWN;
		}
	}

	const int nStep = 5;

	CMFCRibbonBaseElement* pNewHighlighted = NULL;

	BOOL bIsCyclic = m_bMenuMode ||(m_bIsFirst && m_bIsLast);
	BOOL bInvokeCommand = FALSE;

	switch (nChar)
	{
	case VK_RETURN:
	case VK_SPACE:
		bInvokeCommand = TRUE;
		break;

	case VK_DOWN:
	case VK_UP:
		if (m_pHighlighted != NULL)
		{
			ASSERT_VALID(m_pHighlighted);

			int x = m_pHighlighted->GetRect().CenterPoint().x;

			int yStart = nChar == VK_DOWN ? m_pHighlighted->GetRect().bottom + 1 : m_pHighlighted->GetRect().top - 1;
			int yStep = nChar == VK_DOWN ? nStep : -nStep;

			for (int nTry = 0; nTry < 2 && pNewHighlighted == NULL; nTry++)
			{
				for (int i = 0; pNewHighlighted == NULL; i++)
				{
					int y = yStart;

					int x1 = x - i * nStep;
					int x2 = x + i * nStep;

					if (x1 < m_rect.left && x2 > m_rect.right)
					{
						break;
					}

					while (pNewHighlighted == NULL)
					{
						if ((pNewHighlighted = HitTest(CPoint(x1, y))) == NULL)
						{
							pNewHighlighted = HitTest(CPoint(x2, y));
						}

						if (pNewHighlighted != NULL)
						{
							ASSERT_VALID(pNewHighlighted);

							if (!pNewHighlighted->IsTabStop())
							{
								pNewHighlighted = NULL;
							}
						}

						y += yStep;

						if (nChar == VK_DOWN)
						{
							if (y > m_rect.bottom)
							{
								break;
							}
						}
						else
						{
							if (y < m_rect.top)
							{
								break;
							}
						}
					}
				}

				if (nTry == 0 && pNewHighlighted == NULL)
				{
					if (bIsCyclic)
					{
						yStart = nChar == VK_DOWN ? m_rect.top : m_rect.bottom;
					}
					else
					{
						break;
					}
				}
			}
		}
		else
		{
			pNewHighlighted = nChar == VK_DOWN ? GetFirstTabStop() : GetLastTabStop();
		}
		break;

	case VK_RIGHT:
		if (m_bMenuMode && m_pHighlighted != NULL && m_pHighlighted->HasMenu() && !m_pHighlighted->IsDroppedDown())
		{
			m_pHighlighted->OnShowPopupMenu();
			break;
		}

	case VK_LEFT:
		if (m_bMenuMode && nChar == VK_LEFT && m_pParentMenuBar != NULL)
		{
			ASSERT_VALID(m_pParentMenuBar);

			CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, m_pParentMenuBar->GetParent());

			if (pParentMenu->GetParentPopupMenu() != NULL)
			{
				CMFCRibbonBar* pRibbonBar = m_pParentMenuBar->GetTopLevelRibbonBar();
				if (pRibbonBar != NULL && pRibbonBar->GetKeyboardNavLevelCurrent() == this)
				{
					pRibbonBar->SetKeyboardNavigationLevel(pRibbonBar->GetKeyboardNavLevelParent());
				}

				pParentMenu->CloseMenu();
				return TRUE;
			}
		}

		if (m_pHighlighted != NULL)
		{
			ASSERT_VALID(m_pHighlighted);

			int y = m_pHighlighted->GetRect().CenterPoint().y;

			int xStart = nChar == VK_RIGHT ? m_pHighlighted->GetRect().right + 1 : m_pHighlighted->GetRect().left - 1;

			int xStep = nChar == VK_RIGHT ? nStep : -nStep;

			for (int nTry = 0; nTry < 2 && pNewHighlighted == NULL; nTry++)
			{
				for (int i = 0; pNewHighlighted == NULL; i++)
				{
					int x = xStart;

					int y1 = y - i * nStep;
					int y2 = y + i * nStep;

					if (y1 < m_rect.top && y2 > m_rect.bottom)
					{
						break;
					}

					while (pNewHighlighted == NULL)
					{
						if ((pNewHighlighted = HitTest(CPoint(x, y1))) == NULL)
						{
							pNewHighlighted = HitTest(CPoint(x, y2));
						}

						if (pNewHighlighted != NULL)
						{
							ASSERT_VALID(pNewHighlighted);

							if (!pNewHighlighted->IsTabStop())
							{
								pNewHighlighted = NULL;
							}
						}

						x += xStep;

						if (nChar == VK_RIGHT)
						{
							if (x > m_rect.right)
							{
								break;
							}
						}
						else
						{
							if (x < m_rect.left)
							{
								break;
							}
						}
					}
				}

				if (nTry == 0 && pNewHighlighted == NULL)
				{
					if (bIsCyclic)
					{
						xStart = nChar == VK_RIGHT ? m_rect.left : m_rect.right;
					}
					else
					{
						break;
					}
				}
			}
		}
		else
		{
			pNewHighlighted = nChar == VK_RIGHT ? GetFirstTabStop() : GetLastTabStop();
		}
		break;

	default:
		if (IsMenuMode())
		{
			BOOL bKeyIsPrintable = CKeyboardManager::IsKeyPrintable(nChar);

			UINT nUpperChar = nChar;
			if (bKeyIsPrintable)
			{
				nUpperChar = CKeyboardManager::TranslateCharToUpper(nChar);
			}

			for (int i = 0; i < m_arElements.GetSize(); i++)
			{
				CMFCRibbonBaseElement* pElem = m_arElements [i];
				ASSERT_VALID(pElem);

				if (pElem->OnMenuKey(nUpperChar))
				{
					return TRUE;
				}

				CString strLabel = pElem->GetText();

				int iAmpOffset = strLabel.Find(_T('&'));
				if (iAmpOffset >= 0 && iAmpOffset < strLabel.GetLength() - 1)
				{
					TCHAR szChar [2] = { strLabel.GetAt(iAmpOffset + 1), '\0' };
					CharUpper(szChar);

					UINT uiHotKey = (UINT)(szChar [0]);

					if (uiHotKey == nUpperChar)
					{
						if (!pElem->IsDisabled())
						{
							pNewHighlighted = pElem;
							bInvokeCommand = TRUE;
						}
						break;
					}
				}
			}
		}
	}

	BOOL bRes = FALSE;

	if (pNewHighlighted != NULL)
	{
		ASSERT_VALID(pNewHighlighted);

		if (m_pHighlighted != pNewHighlighted)
		{
			if (m_pHighlighted != NULL)
			{
				ASSERT_VALID(m_pHighlighted);
				m_pHighlighted->m_bIsHighlighted = FALSE;
				m_pHighlighted->m_bIsFocused = FALSE;
				m_pHighlighted->OnSetFocus(FALSE);

				m_pHighlighted->Redraw();
				m_pHighlighted = NULL;
			}

			if (afxGlobalData.IsAccessibilitySupport() && m_pParentMenuBar != NULL && IsMenuMode())
			{
				CRect rect = pNewHighlighted->GetRect();
				CPoint pt(rect.left, rect.top);

				m_pParentMenuBar->ClientToScreen(&pt);
				LPARAM lParam = MAKELPARAM(pt.x, pt.y);

				::NotifyWinEvent(EVENT_OBJECT_FOCUS, m_pParentMenuBar->GetSafeHwnd(), OBJID_CLIENT, (LONG)lParam);
			}

			m_pHighlighted = pNewHighlighted;
			pNewHighlighted->OnHighlight(TRUE);
			pNewHighlighted->m_bIsHighlighted = TRUE;
			pNewHighlighted->m_bIsFocused = TRUE;
			pNewHighlighted->OnSetFocus(TRUE);

			m_pHighlighted->Redraw();

			if (m_pParentMenuBar != NULL)
			{
				ASSERT_VALID(m_pParentMenuBar);

				CMFCRibbonPanelMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCRibbonPanelMenu, m_pParentMenuBar->GetParent());

				if (pParentMenu != NULL && pParentMenu->GetParentRibbonElement() != NULL)
				{
					ASSERT_VALID(pParentMenu->GetParentRibbonElement());
					pParentMenu->GetParentRibbonElement()->OnChangeMenuHighlight(m_pParentMenuBar, m_pHighlighted);
				}
			}
		}

		bRes = TRUE;
	}

	if (bInvokeCommand && m_pHighlighted != NULL)
	{
		ASSERT_VALID(m_pHighlighted);

		CMFCRibbonButton* pButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, m_pHighlighted);

		if (pButton != NULL)
		{
			if (pButton->HasMenu())
			{
				pButton->OnShowPopupMenu();
			}
			else if (!pButton->IsDisabled())
			{
				if (m_pParentMenuBar != NULL)
				{
					ASSERT_VALID(m_pParentMenuBar);

					CMFCRibbonBar* pRibbonBar = m_pParentMenuBar->GetTopLevelRibbonBar();
					if (pRibbonBar != NULL && pRibbonBar->GetKeyboardNavLevelCurrent() == this)
					{
						pRibbonBar->DeactivateKeyboardFocus(TRUE);
					}
				}

				pButton->OnClick(pButton->GetRect().TopLeft());
			}

			bRes = TRUE;
		}
	}

	return bRes;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::GetFirstTabStop() const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (pElem->IsTabStop())
		{
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonPanel::GetLastTabStop() const
{
	ASSERT_VALID(this);

	for (int i = (int) m_arElements.GetSize() - 1; i >= 0; i--)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (pElem->IsTabStop())
		{
			return pElem;
		}
	}

	return NULL;
}

void CMFCRibbonPanel::CleanUpSizes()
{
	ASSERT_VALID(this);
	m_arWidths.RemoveAll();

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->CleanUpSizes();
	}

	m_btnDefault.CleanUpSizes (); 
}

void CMFCRibbonPanel::ScrollPalette(int nScrollOffset)
{
	ASSERT_VALID(this);

	int nDelta = m_nScrollOffset - nScrollOffset;

	if (nDelta == 0)
	{
		return;
	}

	m_nScrollOffset = nScrollOffset;

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		BOOL bIsLabel = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel));
		BOOL bIsIcon = pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon));

		if (bIsLabel || bIsIcon)
		{
			pElem->m_rect.OffsetRect(0, nDelta);
		}
	}
}

CMFCRibbonBaseElement* CMFCRibbonPanel::GetParentButton() const
{
	ASSERT_VALID(this);

	if (m_pParentMenuBar == NULL)
	{
		return NULL;
	}

	return((CMFCPopupMenu*)m_pParentMenuBar->GetParent())->GetParentRibbonElement();
}

CSize CMFCRibbonPanel::GetPaltteMinSize() const
{
	ASSERT_VALID(this);

	if (m_pPaletteButton == NULL)
	{
		ASSERT(FALSE);
		return CSize(-1, -1);
	}

	ASSERT_VALID(m_pPaletteButton);

	const BOOL bNoSideBar = m_pPaletteButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonUndoButton));

	CMFCRibbonBar* pRibbonBar = m_pPaletteButton->GetTopLevelRibbonBar();
	ASSERT_VALID(pRibbonBar);

	CClientDC dc(pRibbonBar);

	CFont* pOldFont = dc.SelectObject(pRibbonBar->GetFont());
	ENSURE(pOldFont != NULL);

	const int cxScroll = ::GetSystemMetrics(SM_CXVSCROLL);

	int cxIcon = m_pPaletteButton->GetIconSize().cx;
	int cyIcon = m_pPaletteButton->GetIconSize().cy;

	int cxLabel = 0;
	int cyLabel = 0;

	int cxBottom = 0;
	int cyBottom = 0;

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->SetInitialMode();
		pElem->OnCalcTextSize(&dc);

		CSize sizeElem = pElem->GetSize(&dc);

		if (pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonGalleryIcon)))
		{
		}
		else if (pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonLabel)))
		{
			cxLabel = max(cxLabel, sizeElem.cx);
			cyLabel = max(cyLabel, sizeElem.cy + m_pPaletteButton->GetGroupOffset());
		}
		else
		{
			if (!bNoSideBar)
			{
				sizeElem.cx += 4 * AFX_TEXT_MARGIN + CMFCToolBar::GetMenuImageSize().cx + 2 * CMFCVisualManager::GetInstance()->GetMenuImageMargin();
			}

			cxBottom = max(cxBottom, sizeElem.cx);
			cyBottom += sizeElem.cy;
		}
	}

	dc.SelectObject(pOldFont);

	int cx = max(cxIcon, cxLabel);

	return CSize(max(cx + cxScroll, cxBottom), cyIcon + cyBottom + cyLabel);
}

void CMFCRibbonPanel::SetKeys(LPCTSTR lpszKeys)
{
	ASSERT_VALID(this);
	m_btnDefault.SetKeys(lpszKeys);
}

void CMFCRibbonPanel::OnRTLChanged(BOOL bIsRTL)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnRTLChanged(bIsRTL);
	}

	m_btnDefault.OnRTLChanged(bIsRTL);
	m_btnLaunch.OnRTLChanged(bIsRTL);
}

CMFCRibbonPanelMenu* CMFCRibbonPanel::ShowPopup(CMFCRibbonDefaultPanelButton* pButton/* = NULL*/)
{
	ASSERT_VALID(this);

	if (pButton == NULL)
	{
		pButton = &m_btnDefault;
	}

	ASSERT_VALID(pButton);

	CWnd* pWndParent = pButton->GetParentWnd();
	if (pWndParent == NULL)
	{
		return NULL;
	}

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);
		m_pParent->EnsureVisible(pButton);
	}

	const BOOL bIsRTL = (pWndParent->GetExStyle() & WS_EX_LAYOUTRTL);

	if (m_arWidths.GetSize() == 0)
	{
		ENSURE(m_pParent != NULL);
		ASSERT_VALID(m_pParent);

		CMFCRibbonBar* pRibbonBar = pButton->GetTopLevelRibbonBar();
		ASSERT_VALID(pRibbonBar);

		CClientDC dc(pRibbonBar);

		CFont* pOldFont = dc.SelectObject(pRibbonBar->GetFont());
		ENSURE(pOldFont != NULL);

		int nHeight = m_pParent->GetMaxHeight(&dc);
		RecalcWidths(&dc, nHeight);

		dc.SelectObject(pOldFont);
	}

	CRect rectBtn = pButton->m_rect;
	pWndParent->ClientToScreen(&rectBtn);

	CMFCRibbonPanelMenu* pMenu = new CMFCRibbonPanelMenu(this);
	pMenu->SetParentRibbonElement(pButton);

	pMenu->Create(pWndParent, bIsRTL ? rectBtn.right : rectBtn.left, rectBtn.bottom, (HMENU) NULL);

	pButton->SetDroppedDown(pMenu);

	return pMenu;
}



