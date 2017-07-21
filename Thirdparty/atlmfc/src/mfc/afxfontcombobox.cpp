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
#include "afxfontcombobox.h"
#include "afxtoolbar.h"
#include "afxtoolbarfontcombobox.h"
#include "afxribbonres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int nImageHeight = 16;
const int nImageWidth = 16;

BOOL CMFCFontComboBox::m_bDrawUsingFont = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CMFCFontComboBox

CMFCFontComboBox::CMFCFontComboBox() : m_bToolBarMode(FALSE)
{
}

CMFCFontComboBox::~CMFCFontComboBox()
{
}

BEGIN_MESSAGE_MAP(CMFCFontComboBox, CComboBox)
	//{{AFX_MSG_MAP(CMFCFontComboBox)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCFontComboBox message handlers

BOOL CMFCFontComboBox::PreTranslateMessage(MSG* pMsg)
{
	if (m_bToolBarMode && pMsg->message == WM_KEYDOWN && !CMFCToolBarFontComboBox::IsFlatMode())
	{
		CMFCToolBar* pBar = (CMFCToolBar*) GetParent();

		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
			if (AFXGetTopLevelFrame(this) != NULL)
			{
				AFXGetTopLevelFrame(this)->SetFocus();
			}
			return TRUE;

		case VK_TAB:
			if (pBar != NULL)
			{
				pBar->GetNextDlgTabItem(this)->SetFocus();
			}
			return TRUE;

		case VK_UP:
		case VK_DOWN:
			if ((GetKeyState(VK_MENU) >= 0) &&(GetKeyState(VK_CONTROL) >=0) && !GetDroppedState())
			{
				ShowDropDown();
				return TRUE;
			}
		}
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

int CMFCFontComboBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
	ASSERT(lpCIS->CtlType == ODT_COMBOBOX);

	int id1 = (int)(WORD)lpCIS->itemID1;
	if (id1 == -1)
	{
		return -1;
	}

	CString str1;
	GetLBText(id1, str1);

	int id2 = (int)(WORD)lpCIS->itemID2;
	if (id2 == -1)
	{
		return 1;
	}

	CString str2;
	GetLBText(id2, str2);

	return str1.Collate(str2);
}

void CMFCFontComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	if (m_Images.GetSafeHandle() == NULL)
	{
		m_Images.Create(IDB_AFXBARRES_FONT, nImageWidth, 0, RGB(255, 255, 255));
	}

	ASSERT(lpDIS->CtlType == ODT_COMBOBOX);

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID(pDC);

	CRect rc = lpDIS->rcItem;

	if (lpDIS->itemState & ODS_FOCUS)
	{
		pDC->DrawFocusRect(rc);
	}

	int nIndexDC = pDC->SaveDC();

	CBrush brushFill;
	if (lpDIS->itemState & ODS_SELECTED)
	{
		brushFill.CreateSolidBrush(afxGlobalData.clrHilite);
		pDC->SetTextColor(afxGlobalData.clrTextHilite);
	}
	else
	{
		brushFill.CreateSolidBrush(pDC->GetBkColor());
	}

	pDC->SetBkMode(TRANSPARENT);
	pDC->FillRect(rc, &brushFill);

	int id = (int)lpDIS->itemID;
	if (id >= 0)
	{
		CFont fontSelected;
		CFont* pOldFont = NULL;

		CMFCFontInfo* pDesc = (CMFCFontInfo*)lpDIS->itemData;
		if (pDesc != NULL)
		{
			if (pDesc->m_nType &(DEVICE_FONTTYPE | TRUETYPE_FONTTYPE))
			{
				CPoint ptImage(rc.left, rc.top +(rc.Height() - nImageHeight) / 2);
				m_Images.Draw(pDC, (pDesc->m_nType & DEVICE_FONTTYPE) ? 0 : 1, ptImage, ILD_NORMAL);
			}

			rc.left += nImageWidth + 6;

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
		}

		CString strText;
		GetLBText(id, strText);

		pDC->DrawText(strText, rc, DT_SINGLELINE | DT_VCENTER);

		if (pOldFont != NULL)
		{
			pDC->SelectObject(pOldFont);
		}
	}

	pDC->RestoreDC(nIndexDC);
}

void CMFCFontComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	ASSERT(lpMIS->CtlType == ODT_COMBOBOX);

	CRect rc;
	GetWindowRect(&rc);
	lpMIS->itemWidth = rc.Width();

	int nFontHeight = max(afxGlobalData.GetTextHeight(), CMFCToolBarFontComboBox::m_nFontHeight);
	lpMIS->itemHeight = max(nImageHeight, nFontHeight);
}

void CMFCFontComboBox::PreSubclassWindow()
{
	CComboBox::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init();
	}
}

int CMFCFontComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	Init();
	return 0;
}

void CMFCFontComboBox::Init()
{
	CWnd* pWndParent = GetParent();
	ASSERT_VALID(pWndParent);

	m_bToolBarMode = pWndParent->IsKindOf(RUNTIME_CLASS(CMFCToolBar));
	if (!m_bToolBarMode)
	{
		Setup();
	}
}

void CMFCFontComboBox::CleanUp()
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(m_hWnd));

	if (m_bToolBarMode)
	{
		// Font data will be destroyed by CMFCToolBarFontComboBox object
		return;
	}

	for (int i = 0; i < GetCount(); i++)
	{
		delete(CMFCFontInfo*) GetItemData(i);
	}

	ResetContent();
}

BOOL CMFCFontComboBox::Setup(int nFontType, BYTE nCharSet, BYTE nPitchAndFamily)
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(m_hWnd));

	if (m_bToolBarMode)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CleanUp();

	CMFCToolBarFontComboBox combo(0, (UINT)-1, nFontType, nCharSet, CBS_DROPDOWN, 0, nPitchAndFamily);

	for (int i = 0; i < combo.GetCount(); i++)
	{
		CString strFont = combo.GetItem(i);

		CMFCFontInfo* pFontDescrSrc = (CMFCFontInfo*) combo.GetItemData(i);
		ASSERT_VALID(pFontDescrSrc);

		if (FindStringExact(-1, strFont) <= 0)
		{
			CMFCFontInfo* pFontDescr = new CMFCFontInfo(*pFontDescrSrc);
			int iIndex = AddString(strFont);
			SetItemData(iIndex, (DWORD_PTR) pFontDescr);
		}
	}

	return TRUE;
}

void CMFCFontComboBox::OnDestroy()
{
	CleanUp();
	CComboBox::OnDestroy();
}

BOOL CMFCFontComboBox::SelectFont(CMFCFontInfo* pDesc)
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(m_hWnd));
	ASSERT_VALID(pDesc);

	for (int i = 0; i < GetCount(); i++)
	{
		CMFCFontInfo* pFontDescr = (CMFCFontInfo*) GetItemData(i);
		ASSERT_VALID(pFontDescr);

		if (*pDesc == *pFontDescr)
		{
			SetCurSel(i);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CMFCFontComboBox::SelectFont(LPCTSTR lpszName, BYTE nCharSet/* = DEFAULT_CHARSET*/)
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(m_hWnd));
	ENSURE(lpszName != NULL);

	for (int i = 0; i < GetCount(); i++)
	{
		CMFCFontInfo* pFontDescr = (CMFCFontInfo*) GetItemData(i);
		ASSERT_VALID(pFontDescr);

		if (pFontDescr->m_strName == lpszName)
		{
			if (nCharSet == DEFAULT_CHARSET || nCharSet == pFontDescr->m_nCharSet)
			{
				SetCurSel(i);
				return TRUE;
			}
		}
	}

	return FALSE;
}

CMFCFontInfo* CMFCFontComboBox::GetSelFont() const
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(m_hWnd));

	int iIndex = GetCurSel();
	if (iIndex < 0)
	{
		return NULL;
	}

	return(CMFCFontInfo*) GetItemData(iIndex);
}


