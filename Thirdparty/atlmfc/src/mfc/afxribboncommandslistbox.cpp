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
#include "afxribboncommandslistbox.h"
#include "afxribbonbar.h"
#include "afxribboncategory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonCommandsListBox

CMFCRibbonCommandsListBox::CMFCRibbonCommandsListBox(CMFCRibbonBar* pRibbonBar, BOOL bIncludeSeparator/* = TRUE*/, BOOL bDrawDefaultIcon/* = FALSE*/)
{
	ASSERT_VALID(pRibbonBar);

	m_pRibbonBar = pRibbonBar;
	m_nTextOffset = 0;
	m_bDrawDefaultIcon = bDrawDefaultIcon;

	if (bIncludeSeparator)
	{
		m_pSeparator = new CMFCRibbonSeparator(TRUE);
	}
	else
	{
		m_pSeparator = NULL;
	}
}

CMFCRibbonCommandsListBox::~CMFCRibbonCommandsListBox()
{
	if (m_pSeparator != NULL)
	{
		delete m_pSeparator;
	}
}

BEGIN_MESSAGE_MAP(CMFCRibbonCommandsListBox, CListBox)
	//{{AFX_MSG_MAP(CMFCRibbonCommandsListBox)
	ON_WM_DRAWITEM_REFLECT()
	ON_WM_MEASUREITEM_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonCommandsListBox message handlers

void CMFCRibbonCommandsListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID(pDC);

	CRect rect = lpDIS->rcItem;

	if (lpDIS->itemID == (UINT)-1)
	{
		return;
	}

	BOOL bIsRibbonImageScale = afxGlobalData.IsRibbonImageScaleEnabled();
	afxGlobalData.EnableRibbonImageScale(FALSE);

	pDC->SetBkMode(TRANSPARENT);

	BOOL bIsHighlighted = (lpDIS->itemState & ODS_SELECTED) &&(lpDIS->itemState & ODS_FOCUS);
	BOOL bIsSelected = (lpDIS->itemState & ODS_SELECTED);

	CMFCRibbonBaseElement* pCommand = (CMFCRibbonBaseElement*) GetItemData(lpDIS->itemID);
	ASSERT_VALID(pCommand);

	CString strText;
	GetText(lpDIS->itemID, strText);

	if (bIsHighlighted)
	{
		::FillRect(pDC->GetSafeHdc(), rect, GetSysColorBrush(COLOR_HIGHLIGHT));
		pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else if (bIsSelected)
	{
		pDC->FillRect(rect, &afxGlobalData.brBtnFace);
		pDC->SetTextColor(afxGlobalData.clrBtnText);
	}
	else
	{
		pDC->FillRect(rect, &afxGlobalData.brWindow);
		pDC->SetTextColor(afxGlobalData.clrWindowText);
	}

	BOOL bDrawDefaultIconSaved = pCommand->m_bDrawDefaultIcon;
	pCommand->m_bDrawDefaultIcon = m_bDrawDefaultIcon;
	pCommand->OnDrawOnList(pDC, strText, m_nTextOffset, rect, bIsSelected, bIsHighlighted);
	pCommand->m_bDrawDefaultIcon = bDrawDefaultIconSaved;

	afxGlobalData.EnableRibbonImageScale(bIsRibbonImageScale);
}

void CMFCRibbonCommandsListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	ENSURE(lpMIS != NULL);

	CClientDC dc(this);
	CFont* pOldFont = (CFont*) dc.SelectStockObject(DEFAULT_GUI_FONT);
	ASSERT_VALID(pOldFont);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	lpMIS->itemHeight = tm.tmHeight + 6;

	dc.SelectObject(pOldFont);
}

void CMFCRibbonCommandsListBox::FillFromCategory(CMFCRibbonCategory* pCategory)
{
	ASSERT_VALID(this);

	ResetContent();
	m_nTextOffset = 0;

	if (pCategory == NULL)
	{
		return;
	}

	ASSERT_VALID(pCategory);

	CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> arElements;
	pCategory->GetElements(arElements);

	FillFromArray(arElements, TRUE, TRUE);

	if (m_pSeparator != NULL)
	{
		ASSERT_VALID(m_pSeparator);
		m_pSeparator->AddToListBox(this, FALSE);
	}
}

void CMFCRibbonCommandsListBox::FillFromArray(const CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements, BOOL bDeep, BOOL bIgnoreSeparators)
{
	ASSERT_VALID(this);

	ResetContent();
	m_nTextOffset = 0;

	BOOL bIsRibbonImageScale = afxGlobalData.IsRibbonImageScaleEnabled();
	afxGlobalData.EnableRibbonImageScale(FALSE);

	for (int i = 0; i < arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = arElements [i];
		ASSERT_VALID(pElem);

		if (bIgnoreSeparators && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonSeparator)))
		{
			continue;
		}

		pElem->AddToListBox(this, bDeep);

		int nImageWidth = pElem->GetImageSize(CMFCRibbonBaseElement::RibbonImageSmall).cx;

		m_nTextOffset = max(m_nTextOffset, nImageWidth + 2);
	}

	if (GetCount() > 0)
	{
		SetCurSel(0);
	}

	afxGlobalData.EnableRibbonImageScale(bIsRibbonImageScale);
}

void CMFCRibbonCommandsListBox::FillFromIDs(const CList<UINT,UINT>& lstCommands, BOOL bDeep)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRibbonBar);

	CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> arElements;

	for (POSITION pos = lstCommands.GetHeadPosition(); pos != NULL;)
	{
		UINT uiCmd = lstCommands.GetNext(pos);

		if (uiCmd == 0)
		{
			if (m_pSeparator != NULL)
			{
				arElements.Add(m_pSeparator);
			}

			continue;
		}

		CMFCRibbonBaseElement* pElem = m_pRibbonBar->FindByID(uiCmd, FALSE);
		if (pElem == NULL)
		{
			continue;
		}

		ASSERT_VALID(pElem);
		arElements.Add(pElem);
	}

	FillFromArray(arElements, bDeep, FALSE);
}

CMFCRibbonBaseElement* CMFCRibbonCommandsListBox::GetSelected() const
{
	ASSERT_VALID(this);

	int nIndex = GetCurSel();

	if (nIndex < 0)
	{
		return NULL;
	}

	return GetCommand(nIndex);
}

CMFCRibbonBaseElement* CMFCRibbonCommandsListBox::GetCommand(int nIndex) const
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement* pCommand = (CMFCRibbonBaseElement*) GetItemData(nIndex);
	ASSERT_VALID(pCommand);

	return pCommand;
}

int CMFCRibbonCommandsListBox::GetCommandIndex(UINT uiID) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < GetCount(); i++)
	{
		CMFCRibbonBaseElement* pCommand = (CMFCRibbonBaseElement*) GetItemData(i);
		ASSERT_VALID(pCommand);

		if (pCommand->GetID() == uiID)
		{
			return i;
		}
	}

	return -1;
}

BOOL CMFCRibbonCommandsListBox::AddCommand(CMFCRibbonBaseElement* pCmd, BOOL bSelect, BOOL bDeep)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pCmd);

	int nIndex = GetCommandIndex(pCmd->GetID());
	if (nIndex >= 0 && pCmd->GetID() != 0)
	{
		return FALSE;
	}

	// Not found, add new:

	if (m_nTextOffset == 0)
	{
		BOOL bIsRibbonImageScale = afxGlobalData.IsRibbonImageScaleEnabled();
		afxGlobalData.EnableRibbonImageScale(FALSE);

		m_nTextOffset = pCmd->GetImageSize(CMFCRibbonBaseElement::RibbonImageSmall).cx + 2;

		afxGlobalData.EnableRibbonImageScale(bIsRibbonImageScale);
	}

	nIndex = pCmd->AddToListBox(this, bDeep);

	if (bSelect)
	{
		SetCurSel(nIndex);
	}

	return TRUE;
}


