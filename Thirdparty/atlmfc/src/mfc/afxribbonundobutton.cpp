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
#include "afxribbonbar.h"
#include "afxribbonundobutton.h"
#include "afxribbonlabel.h"
#include "afxribbonpanelmenu.h"
#include "afxribbonres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int nTextMarginHorz = 5;

class CRibbonUndoLabel : public CMFCRibbonButton
{
public:
	DECLARE_DYNCREATE(CRibbonUndoLabel)

	CRibbonUndoLabel(LPCTSTR lpszText = NULL) : CMFCRibbonButton(0, lpszText)
	{
		m_szMargin = CSize(0, 0); // Make it smaller
	}

	virtual BOOL IsTabStop() const
	{
		return FALSE; // User can't activate it by keyboard
	}

	virtual void OnDraw(CDC* pDC)
	{
		ASSERT_VALID(this);
		ASSERT_VALID(pDC);

		CRect rectText = m_rect;
		rectText.DeflateRect(nTextMarginHorz, 0);

		DrawRibbonText(pDC, m_strText, rectText, DT_SINGLELINE | DT_VCENTER);
	}
};

IMPLEMENT_DYNCREATE(CRibbonUndoLabel, CMFCRibbonButton)

IMPLEMENT_DYNCREATE(CMFCRibbonUndoButton, CMFCRibbonGallery)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCRibbonUndoButton::CMFCRibbonUndoButton()
{
	CommonInit();
}

CMFCRibbonUndoButton::CMFCRibbonUndoButton(UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex) :
	CMFCRibbonGallery(nID, lpszText, nSmallImageIndex, nLargeImageIndex, CSize(0, 0), 0, FALSE)
{
	CommonInit();
}

CMFCRibbonUndoButton::CMFCRibbonUndoButton(UINT nID, LPCTSTR lpszText, HICON hIcon) : CMFCRibbonGallery(nID, lpszText, -1, -1, CSize(0, 0), 0, FALSE)
{
	CommonInit();
	m_hIcon = hIcon;
}

void CMFCRibbonUndoButton::CommonInit()
{
	m_nActionNumber = -1;

	SetButtonMode(TRUE);
	SetIconsInRow(1);
	SetDefaultCommand();

	m_sizeMaxText = CSize(0, 0);

	ENSURE(m_strCancel.LoadString(IDS_AFXBARRES_CANCEL));
	ENSURE(m_strUndoOne.LoadString(IDS_AFXBARRES_UNDO_ONE));
	ENSURE(m_strUndoFmt.LoadString(IDS_AFXBARRES_UNDO_FMT));

	AddSubItem(new CRibbonUndoLabel(m_strCancel));
}

CMFCRibbonUndoButton::~CMFCRibbonUndoButton()
{
}

void CMFCRibbonUndoButton::AddUndoAction(LPCTSTR lpszLabel)
{
	ASSERT_VALID(this);
	ASSERT(lpszLabel != NULL);

	Clear();

	m_arLabels.Add(lpszLabel);
	m_nIcons = (int) m_arLabels.GetSize();

	m_sizeMaxText = CSize(0, 0);
}

void CMFCRibbonUndoButton::CleanUpUndoList()
{
	ASSERT_VALID(this);

	Clear();

	m_arLabels.RemoveAll();
	m_sizeMaxText = CSize(0, 0);
}

void CMFCRibbonUndoButton::NotifyHighlightListItem(int nIndex)
{
	ASSERT_VALID(this);

	if (m_pPopupMenu != NULL)
	{
		m_nActionNumber = nIndex + 1;

		// Change label:
		CString strLabel = m_strCancel;

		if (m_nActionNumber > 0)
		{
			if (m_nActionNumber == 1)
			{
				strLabel = m_strUndoOne;
			}
			else
			{
				strLabel.Format(m_strUndoFmt, m_nActionNumber);
			}
		}

		CMFCRibbonPanelMenu* pPanelMenu = DYNAMIC_DOWNCAST(CMFCRibbonPanelMenu, m_pPopupMenu);
		if (pPanelMenu != NULL)
		{
			ASSERT_VALID(pPanelMenu);

			if (pPanelMenu->GetPanel() != NULL)
			{
				CMFCRibbonBaseElement* pMenuElem = pPanelMenu->GetPanel()->FindByID(0);

				if (pMenuElem != NULL)
				{
					pMenuElem->SetText(strLabel);
					pMenuElem->Redraw();
				}
			}
		}

		RedrawIcons();
	}

	CMFCRibbonGallery::NotifyHighlightListItem(nIndex);
}

void CMFCRibbonUndoButton::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonGallery::CopyFrom(s);

	CMFCRibbonUndoButton& src = (CMFCRibbonUndoButton&) s;

	m_nActionNumber = src.m_nActionNumber;

	m_arLabels.RemoveAll();
	m_arLabels.Copy(src.m_arLabels);
	m_nIcons = src.m_nIcons;

	m_sizeMaxText = src.m_sizeMaxText;
}

void CMFCRibbonUndoButton::OnClick(CPoint point)
{
	ASSERT_VALID(this);
	m_nActionNumber = -1;
	CMFCRibbonGallery::OnClick(point);
}

CSize CMFCRibbonUndoButton::GetIconSize() const
{
	ASSERT_VALID(this);
	return m_sizeMaxText;
}

void CMFCRibbonUndoButton::OnDrawPaletteIcon( CDC* pDC, CRect rectIcon, int nIconIndex, CMFCRibbonGalleryIcon* pIcon, COLORREF clrText)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(pIcon);
	ASSERT(nIconIndex >= 0);
	ASSERT(nIconIndex < m_nIcons);

	BOOL bIsChecked = pIcon->m_bIsChecked;
	BOOL bIsHighlighted = pIcon->m_bIsHighlighted;

	pIcon->m_bIsChecked = FALSE;
	pIcon->m_bIsHighlighted = nIconIndex < m_nActionNumber;

	pIcon->OnFillBackground(pDC);

	CRect rectText = rectIcon;
	rectText.DeflateRect(nTextMarginHorz, 0);

	COLORREF clrOld = (COLORREF)-1;
	if (clrText != (COLORREF)-1)
	{
		clrOld = pDC->SetTextColor(clrText);
	}

	pDC->DrawText(m_arLabels [nIconIndex], rectText, DT_VCENTER | DT_SINGLELINE | DT_LEFT);

	if (clrText != (COLORREF)-1)
	{
		pDC->SetTextColor(clrOld);
	}

	pIcon->OnDrawBorder(pDC);

	pIcon->m_bIsChecked = bIsChecked;
	pIcon->m_bIsHighlighted = bIsHighlighted;
}

BOOL CMFCRibbonUndoButton::OnClickPaletteSubItem(CMFCRibbonButton* pButton, CMFCRibbonPanelMenuBar* pMenuBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pButton);

	if (pButton->IsKindOf(RUNTIME_CLASS(CRibbonUndoLabel)))
	{
		ClosePopupMenu();
		return TRUE;
	}

	return CMFCRibbonGallery::OnClickPaletteSubItem(pButton, pMenuBar);
}

void CMFCRibbonUndoButton::OnShowPopupMenu()
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnShowPopupMenu();

	m_bSmallIcons = FALSE;

	if (m_sizeMaxText == CSize(0, 0))
	{
		CMFCRibbonBar* pRibbonBar = GetTopLevelRibbonBar();
		ASSERT_VALID(pRibbonBar);

		CClientDC dc(pRibbonBar);

		CFont* pOldFont = dc.SelectObject(pRibbonBar->GetFont());
		ASSERT(pOldFont != NULL);

		for (int i = 0; i < m_arLabels.GetSize(); i++)
		{
			CSize szText = dc.GetTextExtent(m_arLabels [i]);

			m_sizeMaxText.cx = max(m_sizeMaxText.cx, szText.cx);
			m_sizeMaxText.cy = max(m_sizeMaxText.cy, szText.cy);
		}

		m_sizeMaxText.cx = max(m_sizeMaxText.cx, dc.GetTextExtent(m_strCancel).cx);
		m_sizeMaxText.cx = max(m_sizeMaxText.cx, dc.GetTextExtent(m_strUndoOne).cx);
		m_sizeMaxText.cx = max(m_sizeMaxText.cx, dc.GetTextExtent(m_strUndoFmt).cx);

		m_sizeMaxText.cx += 2 * nTextMarginHorz;

		dc.SelectObject(pOldFont);
	}

	m_nActionNumber = -1;
	CMFCRibbonGallery::OnShowPopupMenu();
}


