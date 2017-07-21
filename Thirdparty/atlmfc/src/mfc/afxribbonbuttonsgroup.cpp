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
#include "afxglobals.h"
#include "afxribbonbuttonsgroup.h"
#include "afxvisualmanager.h"
#include "afxribbonbar.h"
#include "afxribbonstatusbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCRibbonButtonsGroup, CMFCRibbonBaseElement)

// Construction/Destruction
CMFCRibbonButtonsGroup::CMFCRibbonButtonsGroup()
{
}

CMFCRibbonButtonsGroup::CMFCRibbonButtonsGroup(CMFCRibbonBaseElement* pButton)
{
	AddButton(pButton);
}

CMFCRibbonButtonsGroup::~CMFCRibbonButtonsGroup()
{
	RemoveAll();
}

void CMFCRibbonButtonsGroup::AddButton(CMFCRibbonBaseElement* pButton)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pButton);

	pButton->SetParentCategory(m_pParent);
	pButton->m_pParentGroup = this;

	m_arButtons.Add(pButton);
}

void CMFCRibbonButtonsGroup::AddButtons(const CList<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& lstButtons)
{
	ASSERT_VALID(this);

	for (POSITION pos = lstButtons.GetHeadPosition(); pos != NULL;)
	{
		AddButton(lstButtons.GetNext(pos));
	}
}

void CMFCRibbonButtonsGroup::RemoveAll()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		delete m_arButtons [i];
	}

	m_arButtons.RemoveAll();
}

void CMFCRibbonButtonsGroup::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	// Fill group background:
	COLORREF clrText = CMFCVisualManager::GetInstance()->OnDrawRibbonButtonsGroup(pDC, this, m_rect);
	COLORREF clrTextOld = (COLORREF)-1;
	if (clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor(clrText);
	}

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		if (pButton->m_rect.IsRectEmpty())
		{
			continue;
		}

		CString strText = pButton->m_strText;

		if (pButton->GetImageSize(CMFCRibbonBaseElement::RibbonImageSmall) != CSize(0, 0))
		{
			pButton->m_strText.Empty();
		}

		pButton->OnDraw(pDC);

		pButton->m_strText = strText;
	}

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}
}

CSize CMFCRibbonButtonsGroup::GetRegularSize(CDC* pDC)
{
	ASSERT_VALID(this);

	const BOOL bIsOnStatusBar = DYNAMIC_DOWNCAST(CMFCRibbonStatusBar, GetParentRibbonBar()) != NULL;

	CSize size(0, 0);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->SetInitialMode(TRUE);
		pButton->OnCalcTextSize(pDC);

		CSize sizeButton = pButton->GetSize(pDC);

		size.cx += sizeButton.cx;
		size.cy = max(size.cy, sizeButton.cy);
	}

	if (bIsOnStatusBar)
	{
		size.cx += 2;
	}

	return size;
}

void CMFCRibbonButtonsGroup::OnUpdateCmdUI(CMFCRibbonCmdUI* pCmdUI, CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->OnUpdateCmdUI(pCmdUI, pTarget, bDisableIfNoHndler);
	}
}

void CMFCRibbonButtonsGroup::OnAfterChangeRect(CDC* pDC)
{
	ASSERT_VALID(this);

	BOOL bIsFirst = TRUE;

	const BOOL bIsOnStatusBar = DYNAMIC_DOWNCAST(CMFCRibbonStatusBar, GetParentRibbonBar()) != NULL;
	const BOOL bIsQATOnBottom = IsQuickAccessToolBar() && !m_pRibbonBar->IsQuickAccessToolbarOnTop();

	const int nMarginX = IsQuickAccessToolBar() ? 2 : 0;
	const int nMarginTop = bIsQATOnBottom ? 2 : bIsOnStatusBar ? 1 : 0;
	const int nMarginBottom = IsQuickAccessToolBar() || bIsOnStatusBar ? 1 : 0;

	const int nButtonHeight = m_rect.Height() - nMarginTop - nMarginBottom;

	CRect rectGroup = m_rect;

	int x = rectGroup.left + nMarginX;

	int nCustomizeButtonIndex = -1;

	if (IsQuickAccessToolBar() && m_arButtons.GetSize() > 0)
	{
		// Last button is customize - it always visible.
		// Leave space for it:
		nCustomizeButtonIndex = (int) m_arButtons.GetSize() - 1;

		CMFCRibbonBaseElement* pButton = m_arButtons [nCustomizeButtonIndex];
		ASSERT_VALID(pButton);

		CSize sizeButton = pButton->GetSize(pDC);
		rectGroup.right -= sizeButton.cx;
	}

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->m_bShowGroupBorder = TRUE;

		if (pButton->m_pRibbonBar != NULL && !pButton->m_pRibbonBar->IsShowGroupBorder(this))
		{
			pButton->m_bShowGroupBorder = FALSE;
		}

		if (m_rect.IsRectEmpty())
		{
			pButton->m_rect = CRect(0, 0, 0, 0);
			pButton->OnAfterChangeRect(pDC);
			continue;
		}

		BOOL bIsLast = i == m_arButtons.GetSize() - 1;

		pButton->SetParentCategory(m_pParent);

		CSize sizeButton = pButton->GetSize(pDC);
		sizeButton.cy = i != nCustomizeButtonIndex ? nButtonHeight : nButtonHeight - 1;

		const int y = i != nCustomizeButtonIndex ? rectGroup.top + nMarginTop : rectGroup.top;

		pButton->m_rect = CRect(CPoint(x, y), sizeButton);

		if (pButton->m_rect.right > rectGroup.right && i != nCustomizeButtonIndex)
		{
			pButton->m_rect = CRect(0, 0, 0, 0);
		}
		else
		{
			x += sizeButton.cx;
		}

		pButton->OnAfterChangeRect(pDC);

		if (bIsFirst && bIsLast)
		{
			pButton->m_Location = RibbonElementSingleInGroup;
		}
		else if (bIsFirst)
		{
			pButton->m_Location = RibbonElementFirstInGroup;
		}
		else if (bIsLast)
		{
			pButton->m_Location = RibbonElementLastInGroup;
		}
		else
		{
			pButton->m_Location = RibbonElementMiddleInGroup;
		}

		bIsFirst = FALSE;
	}
}

void CMFCRibbonButtonsGroup::OnShow(BOOL bShow)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->OnShow(bShow);
	}
}

CMFCRibbonBaseElement* CMFCRibbonButtonsGroup::HitTest(CPoint point)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		if (pButton->m_rect.PtInRect(point))
		{
			return pButton;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonButtonsGroup::Find(const CMFCRibbonBaseElement* pElement)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		CMFCRibbonBaseElement* pElem = pButton->Find(pElement);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonButtonsGroup::FindByID(UINT uiCmdID)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		CMFCRibbonBaseElement* pElem = pButton->FindByID(uiCmdID);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonButtonsGroup::FindByData(DWORD_PTR dwData)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		CMFCRibbonBaseElement* pElem = pButton->FindByData(dwData);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonButtonsGroup::FindByOriginal(CMFCRibbonBaseElement* pOriginal)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pOriginal);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		CMFCRibbonBaseElement* pElem = pButton->FindByOriginal(pOriginal);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonButtonsGroup::GetPressed()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		CMFCRibbonBaseElement* pElem = pButton->GetPressed();
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonButtonsGroup::GetDroppedDown()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		CMFCRibbonBaseElement* pElem = pButton->GetDroppedDown();
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonButtonsGroup::GetHighlighted()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		CMFCRibbonBaseElement* pElem = pButton->GetHighlighted();
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

BOOL CMFCRibbonButtonsGroup::ReplaceByID(UINT uiCmdID, CMFCRibbonBaseElement* pElem)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		if (pButton->GetID() == uiCmdID)
		{
			pElem->CopyFrom(*pButton);
			m_arButtons [i] = pElem;

			delete pButton;
			return TRUE;
		}

		if (pButton->ReplaceByID(uiCmdID, pElem))
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMFCRibbonButtonsGroup::SetImages(CMFCToolBarImages* pImages, CMFCToolBarImages* pHotImages, CMFCToolBarImages* pDisabledImages)
{
	ASSERT_VALID(this);

	if (pImages != NULL)
	{
		pImages->CopyTo(m_Images);
	}

	if (pHotImages != NULL)
	{
		pHotImages->CopyTo(m_HotImages);
	}

	if (pDisabledImages != NULL)
	{
		pDisabledImages->CopyTo(m_DisabledImages);
	}
}

void CMFCRibbonButtonsGroup::OnDrawImage(CDC* pDC, CRect rectImage,  CMFCRibbonBaseElement* pButton, int nImageIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	CMFCToolBarImages& image = (pButton->IsDisabled() && m_DisabledImages.GetCount() != 0) ? m_DisabledImages :
		(pButton->IsHighlighted() && m_HotImages.GetCount() != 0) ? m_HotImages : m_Images;

	if (image.GetCount() <= 0)
	{
		return;
	}

	CAfxDrawState ds;

	CPoint ptImage = rectImage.TopLeft();
	ptImage.x++;

	image.SetTransparentColor(afxGlobalData.clrBtnFace);

	if (afxGlobalData.GetRibbonImageScale() == 1.)
	{
		image.PrepareDrawImage(ds);
	}
	else
	{
		image.PrepareDrawImage(ds, GetImageSize());
	}

	image.SetTransparentColor(afxGlobalData.clrBtnFace);
	image.Draw(pDC, ptImage.x, ptImage.y, nImageIndex, FALSE, pButton->IsDisabled() && m_DisabledImages.GetCount() == 0);

	image.EndDrawImage(ds);
}

void CMFCRibbonButtonsGroup::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::CopyFrom(s);
	CMFCRibbonButtonsGroup& src = (CMFCRibbonButtonsGroup&) s;

	RemoveAll();

	for (int i = 0; i < src.m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pSrcElem = src.m_arButtons [i];
		ASSERT_VALID(pSrcElem);

		CMFCRibbonBaseElement* pElem = (CMFCRibbonBaseElement*) pSrcElem->GetRuntimeClass()->CreateObject();
		ASSERT_VALID(pElem);

		pElem->CopyFrom(*pSrcElem);

		m_arButtons.Add(pElem);
	}

	src.m_Images.CopyTo(m_Images);
	src.m_HotImages.CopyTo(m_HotImages);
	src.m_DisabledImages.CopyTo(m_DisabledImages);
}

void CMFCRibbonButtonsGroup::SetParentMenu(CMFCRibbonPanelMenuBar* pMenuBar)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetParentMenu(pMenuBar);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->SetParentMenu(pMenuBar);
	}
}

void CMFCRibbonButtonsGroup::SetOriginal(CMFCRibbonBaseElement* pOriginal)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetOriginal(pOriginal);
	CMFCRibbonButtonsGroup* pOriginalGroup = DYNAMIC_DOWNCAST(CMFCRibbonButtonsGroup, pOriginal);

	if (pOriginalGroup == NULL)
	{
		return;
	}

	ASSERT_VALID(pOriginalGroup);

	if (pOriginalGroup->m_arButtons.GetSize() != m_arButtons.GetSize())
	{
		ASSERT(FALSE);
		return;
	}

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->SetOriginal(pOriginalGroup->m_arButtons [i]);
	}
}

void CMFCRibbonButtonsGroup::GetItemIDsList(CList<UINT,UINT>& lstItems) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->GetItemIDsList(lstItems);
	}
}

void CMFCRibbonButtonsGroup::GetElementsByID(UINT uiCmdID, CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->GetElementsByID(uiCmdID, arElements);
	}
}

int CMFCRibbonButtonsGroup::AddToListBox(CMFCRibbonCommandsListBox* pWndListBox, BOOL bDeep)
{
	ASSERT_VALID(this);

	int nIndex = -1;

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		nIndex = pButton->AddToListBox(pWndListBox, bDeep);
	}

	return nIndex;
}

void CMFCRibbonButtonsGroup::CleanUpSizes()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->CleanUpSizes();
	}
}

void CMFCRibbonButtonsGroup::SetParentRibbonBar(CMFCRibbonBar* pRibbonBar)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetParentRibbonBar(pRibbonBar);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->SetParentRibbonBar(pRibbonBar);
	}
}

void CMFCRibbonButtonsGroup::SetParentCategory(CMFCRibbonCategory* pCategory)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetParentCategory(pCategory);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->SetParentCategory(pCategory);
	}
}

void CMFCRibbonButtonsGroup::AddToKeyList(CArray<CMFCRibbonKeyTip*,CMFCRibbonKeyTip*>& arElems)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->AddToKeyList(arElems);
	}
}

void CMFCRibbonButtonsGroup::OnRTLChanged(BOOL bIsRTL)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnRTLChanged(bIsRTL);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->OnRTLChanged(bIsRTL);
	}
}

const CSize CMFCRibbonButtonsGroup::GetImageSize() const
{
	ASSERT_VALID(this);

	if (m_Images.GetCount() <= 0)
	{
		return CSize(0, 0);
	}

	const CSize sizeImage = m_Images.GetImageSize();

	if (afxGlobalData.GetRibbonImageScale() == 1.)
	{
		return sizeImage;
	}

	return CSize( (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImage.cx), (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImage.cy));
}



