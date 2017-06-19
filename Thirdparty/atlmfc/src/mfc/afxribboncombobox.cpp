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
#include "afxribboncombobox.h"
#include "afxribboncategory.h"
#include "afxribbonpanelmenu.h"
#include "afxvisualmanager.h"
#include "afxdropdownlistbox.h"
#include "afxribbonbar.h"
#include "afxribbonpanel.h"
#include "afxmenuimages.h"
#include "afxtrackmouse.h"
#include "afxtoolbarmenubutton.h"
#include "afxribbonres.h"
#include "afxribbonedit.h"
#include "afxribbonminitoolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nDefaultComboHeight = 150;
static const int nDefaultWidth = 108;

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonComboBox

IMPLEMENT_DYNCREATE(CMFCRibbonComboBox, CMFCRibbonEdit)

// Construction/Destruction
CMFCRibbonComboBox::CMFCRibbonComboBox(UINT uiID, BOOL bHasEditBox, int nWidth, LPCTSTR lpszLabel, int nImage) :
	CMFCRibbonEdit(uiID, nWidth == -1 ? nDefaultWidth : nWidth, lpszLabel, nImage)
{
	CommonInit();

	m_bHasEditBox = bHasEditBox;
}

CMFCRibbonComboBox::CMFCRibbonComboBox()
{
	CommonInit();
}

void CMFCRibbonComboBox::CommonInit()
{
	m_iSelIndex = -1;
	m_nDropDownHeight = nDefaultComboHeight;
	m_bHasEditBox = FALSE;
	m_bHasDropDownList = TRUE;
	m_nMenuArrowMargin = 3;
	m_bResizeDropDownList = TRUE;
}

CMFCRibbonComboBox::~CMFCRibbonComboBox()
{
	ClearData();
}

void CMFCRibbonComboBox::ClearData()
{
}

INT_PTR CMFCRibbonComboBox::AddItem(LPCTSTR lpszItem, DWORD_PTR dwData)
{
	ASSERT_VALID(this);
	ENSURE(lpszItem != NULL);

	if (FindItem(lpszItem) < 0)
	{
		m_lstItems.AddTail(lpszItem);
		m_lstItemData.AddTail(dwData);
	}

	return m_lstItems.GetCount() - 1;
}

LPCTSTR CMFCRibbonComboBox::GetItem(int iIndex) const
{
	ASSERT_VALID(this);

	if (iIndex == -1) // Current selection
	{
		if ((iIndex = m_iSelIndex) == -1)
		{
			return NULL;
		}
	}

	POSITION pos = m_lstItems.FindIndex(iIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	return m_lstItems.GetAt(pos);
}

DWORD_PTR CMFCRibbonComboBox::GetItemData(int iIndex) const
{
	ASSERT_VALID(this);

	if (iIndex == -1) // Current selection
	{
		if ((iIndex = m_iSelIndex) == -1)
		{
			return 0;
		}
	}

	POSITION pos = m_lstItemData.FindIndex(iIndex);
	if (pos == NULL)
	{
		return 0;
	}

	return m_lstItemData.GetAt(pos);
}

void CMFCRibbonComboBox::RemoveAllItems()
{
	ASSERT_VALID(this);

	ClearData();

	m_lstItems.RemoveAll();
	m_lstItemData.RemoveAll();
	m_strEdit.Empty();

	m_iSelIndex = -1;

	Redraw();
}

BOOL CMFCRibbonComboBox::SelectItem(int iIndex)
{
	ASSERT_VALID(this);

	if (iIndex >= m_lstItems.GetCount())
	{
		return FALSE;
	}

	m_iSelIndex = max(-1, iIndex);

	LPCTSTR lpszText = GetItem(m_iSelIndex);

	m_strEdit = lpszText == NULL ? _T("") : lpszText;

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->SetWindowText(m_strEdit);
	}

	if (!m_bDontNotify)
	{
		CMFCRibbonBar* pRibbonBar = GetTopLevelRibbonBar();
		if (pRibbonBar != NULL)
		{
			ASSERT_VALID(pRibbonBar);

			CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> arButtons;
			pRibbonBar->GetElementsByID(m_nID, arButtons);

			for (int i = 0; i < arButtons.GetSize(); i++)
			{
				CMFCRibbonComboBox* pOther = DYNAMIC_DOWNCAST(CMFCRibbonComboBox, arButtons [i]);

				if (pOther != NULL && pOther != this)
				{
					ASSERT_VALID(pOther);

					pOther->m_bDontNotify = TRUE;
					pOther->SelectItem(iIndex);
					pOther->m_bDontNotify = FALSE;
				}
			}
		}
	}

	Redraw();
	return TRUE;
}

BOOL CMFCRibbonComboBox::SelectItem(DWORD_PTR dwData)
{
	ASSERT_VALID(this);

	int iIndex = 0;
	for (POSITION pos = m_lstItemData.GetHeadPosition(); pos != NULL; iIndex ++)
	{
		if (m_lstItemData.GetNext(pos) == dwData)
		{
			return SelectItem(iIndex);
		}
	}

	return FALSE;
}

BOOL CMFCRibbonComboBox::SelectItem(LPCTSTR lpszText)
{
	ASSERT_VALID(this);
	ENSURE(lpszText != NULL);

	int iIndex = FindItem(lpszText);
	if (iIndex < 0)
	{
		return FALSE;
	}

	return SelectItem(iIndex);
}

BOOL CMFCRibbonComboBox::DeleteItem(int iIndex)
{
	ASSERT_VALID(this);

	if (iIndex < 0 || iIndex >= m_lstItems.GetCount())
	{
		return FALSE;
	}

	POSITION pos = m_lstItems.FindIndex(iIndex);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_lstItems.RemoveAt(pos);

	pos = m_lstItemData.FindIndex(iIndex);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_lstItemData.RemoveAt(pos);

	if (iIndex == m_iSelIndex)
	{
		int iSelIndex = m_iSelIndex;
		if (iSelIndex >= m_lstItems.GetCount())
		{
			iSelIndex = (int) m_lstItems.GetCount() - 1;
		}

		SelectItem(iSelIndex);
	}

	return TRUE;
}

BOOL CMFCRibbonComboBox::DeleteItem(DWORD_PTR dwData)
{
	ASSERT_VALID(this);

	int iIndex = 0;
	for (POSITION pos = m_lstItemData.GetHeadPosition(); pos != NULL; iIndex ++)
	{
		if (m_lstItemData.GetNext(pos) == dwData)
		{
			return DeleteItem(iIndex);
		}
	}

	return FALSE;
}

BOOL CMFCRibbonComboBox::DeleteItem(LPCTSTR lpszText)
{
	ASSERT_VALID(this);
	ENSURE(lpszText != NULL);

	int iIndex = FindItem(lpszText);
	if (iIndex < 0)
	{
		return FALSE;
	}

	return DeleteItem(iIndex);
}

int CMFCRibbonComboBox::FindItem(LPCTSTR lpszText) const
{
	ASSERT_VALID(this);
	ENSURE(lpszText != NULL);

	int iIndex = 0;
	for (POSITION pos = m_lstItems.GetHeadPosition(); pos != NULL; iIndex++)
	{
		if (m_lstItems.GetNext(pos).CompareNoCase(lpszText) == 0)
		{
			return iIndex;
		}
	}

	return -1;
}

void CMFCRibbonComboBox::SetDropDownHeight(int nHeight)
{
	ASSERT_VALID(this);
	m_nDropDownHeight = nHeight;
}

CSize CMFCRibbonComboBox::GetIntermediateSize(CDC* pDC)
{
	ASSERT_VALID(this);

	CSize size = CMFCRibbonEdit::GetIntermediateSize(pDC);

	size.cx += GetDropDownImageWidth() + 2 * m_nMenuArrowMargin + 1;

	return size;
}

void CMFCRibbonComboBox::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	OnDrawLabelAndImage(pDC);

	BOOL bIsHighlighted = m_bIsHighlighted;

	if (m_bIsFocused)
	{
		m_bIsHighlighted = TRUE;
	}

	if (IsDisabled())
	{
		m_bIsHighlighted = FALSE;
	}

	CRect rectSaved = m_rect;
	m_rect.left += m_nLabelImageWidth;

	CMFCVisualManager::GetInstance()->OnFillRibbonButton(pDC, this);

	if (m_pWndEdit->GetSafeHwnd() == NULL)
	{
		CRect rectText = m_rectCommand;
		rectText.DeflateRect(m_szMargin);

		DrawRibbonText(pDC, m_strEdit, rectText, DT_SINGLELINE | DT_VCENTER);
	}

	CMFCVisualManager::GetInstance()->OnDrawRibbonButtonBorder(pDC, this);

	CMFCToolBarComboBoxButton buttonDummy;
	buttonDummy.m_bIsRibbon = TRUE;

	BOOL bIsDropDownHighlighted = IsMenuAreaHighlighted() || m_bIsFocused || (bIsHighlighted && !m_bHasEditBox);

	CMFCVisualManager::GetInstance()->OnDrawComboDropButton(pDC, m_rectMenu, IsDisabled(), IsDroppedDown(), bIsDropDownHighlighted, &buttonDummy);

	m_bIsHighlighted = bIsHighlighted;
	m_rect = rectSaved;
}

void CMFCRibbonComboBox::OnLButtonDown(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnLButtonDown(point);

	if ((m_rectMenu.PtInRect(point) || !m_bHasEditBox) && !IsDroppedDown())
	{
		DropDownList();
	}
}

void CMFCRibbonComboBox::OnLButtonUp(CPoint /*point*/)
{
	ASSERT_VALID(this);
}

void CMFCRibbonComboBox::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonEdit::CopyFrom(s);

	CMFCRibbonComboBox& src = (CMFCRibbonComboBox&) s;

	m_strEdit = src.m_strEdit;
	m_bHasEditBox = src.m_bHasEditBox;

	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;
		m_pWndEdit = NULL;
	}

	m_lstItems.RemoveAll();
	m_lstItems.AddTail(&src.m_lstItems);

	m_lstItemData.RemoveAll();
	m_lstItemData.AddTail(&src.m_lstItemData);

	m_iSelIndex = src.m_iSelIndex;
	m_nDropDownHeight = src.m_nDropDownHeight;

	m_bResizeDropDownList = src.m_bResizeDropDownList;
}

void CMFCRibbonComboBox::DropDownList()
{
	ASSERT_VALID(this);

	if (IsDisabled())
	{
		return;
	}

	if (m_pWndEdit->GetSafeHwnd() != NULL && !m_pWndEdit->IsWindowVisible())
	{
		return;
	}

	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		if (CMFCPopupMenu::GetActiveMenu()->GetMenuBar() != m_pParentMenu)
		{
			CMFCPopupMenu::GetActiveMenu()->SendMessage(WM_CLOSE);
			return;
		}
	}

	CMFCRibbonBaseElement::OnShowPopupMenu();

	CMFCDropDownListBox* pList = new CMFCDropDownListBox(this);
	pList->SetParentRibbonElement(this);

	for (POSITION pos = m_lstItems.GetHeadPosition(); pos != NULL;)
	{
		pList->AddString(m_lstItems.GetNext(pos));
	}

	pList->SetCurSel(m_iSelIndex);
	pList->SetMaxHeight(m_nDropDownHeight);
	pList->SetMinWidth(m_rect.Width());

	CWnd* pWndParent = GetParentWnd();
	if (pWndParent == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	const BOOL bIsRTL = (pWndParent->GetExStyle() & WS_EX_LAYOUTRTL);

	CRect rect = m_rectCommand.IsRectEmpty() ? m_rect : m_rectCommand;
	pWndParent->ClientToScreen(&rect);

	SetDroppedDown(pList);

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);
		m_pParent->HighlightPanel(NULL, CPoint(-1, -1));
	}

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->SetFocus();
		m_pWndEdit->SetSel(0, -1);
	}

	if (m_bResizeDropDownList)
	{
		pList->EnableVertResize(2 * afxGlobalData.GetTextHeight());
	}

	pList->Track(CPoint(bIsRTL ? rect.right : rect.left, rect.bottom), pWndParent->GetOwner());
}

void CMFCRibbonComboBox::OnSelectItem(int nItem)
{
	ASSERT_VALID(this);

	SelectItem(nItem);

	NotifyCommand(TRUE);

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID(m_pParentMenu);

		if (m_pParentMenu->IsRibbonMiniToolBar())
		{
			CMFCRibbonMiniToolBar* pFloaty = DYNAMIC_DOWNCAST(CMFCRibbonMiniToolBar, m_pParentMenu->GetParent());

			if (pFloaty != NULL && !pFloaty->IsContextMenuMode())
			{
				if (m_pWndEdit->GetSafeHwnd() != NULL && m_pWndEdit->GetTopLevelFrame() != NULL)
				{
					m_pWndEdit->GetTopLevelFrame()->SetFocus();
				}

				Redraw();
				return;
			}
		}

		CFrameWnd* pParentFrame = AFXGetParentFrame(m_pParentMenu);
		ASSERT_VALID(pParentFrame);

		pParentFrame->PostMessage(WM_CLOSE);
	}
	else
	{
		if (m_pWndEdit->GetSafeHwnd() != NULL && m_pWndEdit->GetTopLevelFrame() != NULL)
		{
			m_pWndEdit->GetTopLevelFrame()->SetFocus();
		}

		Redraw();
	}
}

void CMFCRibbonComboBox::OnAfterChangeRect(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CMFCRibbonButton::OnAfterChangeRect(pDC);

	if (m_rect.IsRectEmpty())
	{
		if (m_pWndEdit->GetSafeHwnd() != NULL)
		{
			m_pWndEdit->ShowWindow(SW_HIDE);
		}
		return;
	}

	CRect rectCommandOld = m_rectCommand;

	m_Location = RibbonElementSingleInGroup;

	m_rectMenu = m_rect;
	m_rectMenu.left = m_rectMenu.right - GetDropDownImageWidth() - 2 * m_nMenuArrowMargin;

	m_rectCommand = m_rect;
	m_rectCommand.right = m_rectMenu.left;
	m_rectCommand.left += m_nLabelImageWidth;

	int cx = m_bFloatyMode ? m_nWidthFloaty : m_nWidth;
	if (afxGlobalData.GetRibbonImageScale () > 1.)
	{
		cx = (int)(.5 + afxGlobalData.GetRibbonImageScale () * cx);
	}

	if (m_rectCommand.Width () > cx)
	{
		m_rectCommand.left = m_rectCommand.right - cx;
	}

	m_rectMenu.DeflateRect(1, 1);

	m_bMenuOnBottom = FALSE;

	if (!m_bHasEditBox)
	{
		return;
	}

	if (m_pWndEdit == NULL)
	{
		DWORD dwEditStyle = WS_CHILD | ES_WANTRETURN | ES_AUTOHSCROLL | WS_TABSTOP;

		CWnd* pWndParent = GetParentWnd();
		ASSERT_VALID(pWndParent);

		if ((m_pWndEdit = CreateEdit(pWndParent, dwEditStyle)) == NULL)
		{
			return;
		}

		m_pWndEdit->SendMessage(EM_SETTEXTMODE, TM_PLAINTEXT);
		m_pWndEdit->SetFont(GetTopLevelRibbonBar()->GetFont());
		m_pWndEdit->SetWindowText(m_strEdit);
	}

	if (rectCommandOld != m_rectCommand || !m_pWndEdit->IsWindowVisible())
	{
		CRect rectEdit = m_rectCommand;

		rectEdit.DeflateRect(m_szMargin.cx, m_szMargin.cy, 0, m_szMargin.cy);

		m_pWndEdit->SetWindowPos(NULL, rectEdit.left, rectEdit.top, rectEdit.Width(), rectEdit.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		m_pWndEdit->ShowWindow(SW_SHOWNOACTIVATE);
	}
}

BOOL CMFCRibbonComboBox::OnDrawDropListItem(CDC* /*pDC*/, int /*nIndex*/, CMFCToolBarMenuButton* /*pItem*/, BOOL /*bHighlight*/)
{
	ASSERT_VALID(this);
	return FALSE;
}

CSize CMFCRibbonComboBox::OnGetDropListItemSize(CDC* /*pDC*/, int /*nIndex*/, CMFCToolBarMenuButton* /*pItem*/, CSize /*sizeDefault*/)
{
	ASSERT_VALID(this);
	return CSize(0, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonFontComboBox

BOOL CMFCRibbonFontComboBox::m_bDrawUsingFont = FALSE;

const int nImageHeight = 16;
const int nImageWidth = 16;
const int nImageMargin = 6;

IMPLEMENT_DYNCREATE(CMFCRibbonFontComboBox, CMFCRibbonComboBox)

CMFCRibbonFontComboBox::CMFCRibbonFontComboBox(UINT uiID, int nFontType, BYTE nCharSet, BYTE nPitchAndFamily, int nWidth) :
	CMFCRibbonComboBox(uiID, TRUE, nWidth), m_nFontType(DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE), m_nCharSet(DEFAULT_CHARSET), m_nPitchAndFamily(DEFAULT_PITCH)
{
	BuildFonts(nFontType, nCharSet, nPitchAndFamily);
}

CMFCRibbonFontComboBox::CMFCRibbonFontComboBox() :
	m_nFontType(DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE), m_nCharSet(DEFAULT_CHARSET), m_nPitchAndFamily(DEFAULT_PITCH)
{
}

CMFCRibbonFontComboBox::~CMFCRibbonFontComboBox()
{
	ClearData();
}

void CMFCRibbonFontComboBox::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonComboBox::CopyFrom(s);
	CMFCRibbonFontComboBox& src = (CMFCRibbonFontComboBox&) s;

	m_nFontType = src.m_nFontType;
	m_nCharSet = src.m_nCharSet;
	m_nPitchAndFamily = src.m_nPitchAndFamily;
}

void CMFCRibbonFontComboBox::DropDownList()
{
	ASSERT_VALID(this);

	BuildFonts(m_nFontType, m_nCharSet, m_nPitchAndFamily);

	CMFCRibbonComboBox::DropDownList();
}

void CMFCRibbonFontComboBox::BuildFonts(int nFontType/* = DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE*/,
	BYTE nCharSet/* = DEFAULT_CHARSET*/, BYTE nPitchAndFamily/* = DEFAULT_PITCH*/)
{
	if (m_lstItems.IsEmpty() || m_nFontType != nFontType || m_nCharSet != nCharSet || m_nPitchAndFamily != nPitchAndFamily)
	{
		m_nFontType = nFontType;
		m_nCharSet = nCharSet;
		m_nPitchAndFamily = nPitchAndFamily;

		RebuildFonts();
	}
}

void CMFCRibbonFontComboBox::RebuildFonts()
{
	RemoveAllItems();

	CObList lstFonts;

	CMFCToolBarFontComboBox tlbFontCombo(&lstFonts, m_nFontType, m_nCharSet, m_nPitchAndFamily);

	POSITION pos = NULL;

	for (pos = lstFonts.GetHeadPosition(); pos != NULL;)
	{
		CMFCFontInfo* pDesc = (CMFCFontInfo*) lstFonts.GetNext(pos);
		ASSERT_VALID(pDesc);

		if ((m_nFontType & pDesc->m_nType) != 0)
		{
			BOOL bIsUnique = GetFontsCount(pDesc->m_strName, lstFonts) <= 1;
			AddItem(bIsUnique ? pDesc->m_strName : pDesc->GetFullName(), (DWORD_PTR) pDesc);
		}
	}

	// Delete unused items:
	for (pos = lstFonts.GetHeadPosition(); pos != NULL;)
	{
		CMFCFontInfo* pDesc = (CMFCFontInfo*) lstFonts.GetNext(pos);
		ASSERT_VALID(pDesc);

		if ((m_nFontType & pDesc->m_nType) == 0)
		{
			delete pDesc;
		}
	}
}

int CMFCRibbonFontComboBox::GetFontsCount(LPCTSTR lpszName, const CObList& lstFonts)
{
	ASSERT(!lstFonts.IsEmpty());

	int nCount = 0;

	for (POSITION pos = lstFonts.GetHeadPosition(); pos != NULL;)
	{
		CMFCFontInfo* pDesc = (CMFCFontInfo*) lstFonts.GetNext(pos);
		ASSERT_VALID(pDesc);

		if (pDesc->m_strName == lpszName)
		{
			nCount++;
		}
	}

	return nCount;
}

void CMFCRibbonFontComboBox::ClearData()
{
	ASSERT_VALID(this);

	if (m_pOriginal != NULL)
	{
		return;
	}

	for (POSITION pos = m_lstItemData.GetHeadPosition(); pos != NULL;)
	{
		CMFCFontInfo* pDesc = (CMFCFontInfo*) m_lstItemData.GetNext(pos);
		ASSERT_VALID(pDesc);

		delete pDesc;
	}
}

static BOOL __stdcall CompareFonts(const CMFCFontInfo* pDesc, LPCTSTR lpszName, BYTE nCharSet, BOOL bExact)
{
	ASSERT_VALID(pDesc);

	CString strName = pDesc->GetFullName();
	strName.MakeLower();

	if (bExact)
	{
		if (strName == lpszName || (pDesc->m_strName.CompareNoCase(lpszName) == 0 && (nCharSet == pDesc->m_nCharSet || nCharSet == DEFAULT_CHARSET)))
		{
			return TRUE;
		}
	}
	else if (strName.Find(lpszName) == 0 && (nCharSet == DEFAULT_CHARSET || pDesc->m_nCharSet == nCharSet))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CMFCRibbonFontComboBox::SetFont(LPCTSTR lpszName, BYTE nCharSet, BOOL bExact)
{
	ASSERT_VALID(this);
	ENSURE(lpszName != NULL);

	CString strNameFind = lpszName;
	strNameFind.MakeLower();

	const CMFCFontInfo* pCurrFont = GetFontDesc();
	if (pCurrFont != NULL && CompareFonts(pCurrFont, strNameFind, nCharSet, bExact))
	{
		// Font is already selected
		return TRUE;
	}

	for (POSITION pos = m_lstItemData.GetHeadPosition(); pos != NULL;)
	{
		CMFCFontInfo* pDesc = (CMFCFontInfo*) m_lstItemData.GetNext(pos);

		if (CompareFonts(pDesc, strNameFind, nCharSet, bExact))
		{
			SelectItem((DWORD_PTR) pDesc);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CMFCRibbonFontComboBox::OnDrawDropListItem(CDC* pDC, int nIndex, CMFCToolBarMenuButton* pItem, BOOL /*bHighlight*/)
{
	ASSERT_VALID(this);

	if (m_Images.GetSafeHandle() == NULL)
	{
		m_Images.Create(IDB_AFXBARRES_FONT, nImageWidth, 0, RGB(255, 255, 255));
	}

	CRect rc = pItem->Rect();

	CMFCFontInfo* pDesc = (CMFCFontInfo*) GetItemData(nIndex);
	LPCTSTR lpszText = GetItem(nIndex);

	if (pDesc == NULL || lpszText == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pDesc);

	CFont fontSelected;
	CFont* pOldFont = NULL;

	if (pDesc->m_nType &(DEVICE_FONTTYPE | TRUETYPE_FONTTYPE))
	{
		CPoint ptImage(rc.left, rc.top +(rc.Height() - nImageHeight) / 2);
		m_Images.Draw(pDC, (pDesc->m_nType & DEVICE_FONTTYPE) ? 0 : 1, ptImage, ILD_NORMAL);
	}

	rc.left += nImageWidth + nImageMargin;

	if (m_bDrawUsingFont && pDesc->m_nCharSet != SYMBOL_CHARSET)
	{
		LOGFONT lf;
		afxGlobalData.fontRegular.GetLogFont(&lf);

		lstrcpy(lf.lfFaceName, pDesc->m_strName);

		if (pDesc->m_nCharSet != DEFAULT_CHARSET)
		{
			lf.lfCharSet = pDesc->m_nCharSet;
		}

		if (lf.lfHeight < 0)
		{
			lf.lfHeight -= 4;
		}
		else
		{
			lf.lfHeight += 4;
		}

		fontSelected.CreateFontIndirect(&lf);
		pOldFont = pDC->SelectObject(&fontSelected);
	}

	CString strText = lpszText;

	pDC->DrawText(strText, rc, DT_SINGLELINE | DT_VCENTER);

	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}

	return TRUE;
}

CSize CMFCRibbonFontComboBox::OnGetDropListItemSize(CDC* /*pDC*/, int /*nIndex*/, CMFCToolBarMenuButton* /*pItem*/, CSize sizeDefault)
{
	ASSERT_VALID(this);

	CSize size = sizeDefault;
	size.cx += nImageWidth + nImageMargin;

	return size;
}



