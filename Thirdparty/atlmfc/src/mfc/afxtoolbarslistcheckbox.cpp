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
#include "afxpriv.h"
#include "afxtoolbarslistcheckbox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsListCheckBox

CMFCToolBarsListCheckBox::CMFCToolBarsListCheckBox()
{
}

CMFCToolBarsListCheckBox::~CMFCToolBarsListCheckBox()
{
}

//{{AFX_MSG_MAP(CMFCToolBarsListCheckBox)
BEGIN_MESSAGE_MAP(CMFCToolBarsListCheckBox, CCheckListBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(LB_ADDSTRING, &CMFCToolBarsListCheckBox::OnLBAddString)
	ON_MESSAGE(LB_INSERTSTRING, &CMFCToolBarsListCheckBox::OnLBInsertString)
	ON_MESSAGE(LB_RESETCONTENT, &CMFCToolBarsListCheckBox::OnLBResetContent)
	ON_MESSAGE(LB_DELETESTRING, &CMFCToolBarsListCheckBox::OnLBDeleteString)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsListCheckBox message handlers

void CMFCToolBarsListCheckBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	// determine where the click is
	BOOL bInCheck;
	int nIndex = CheckFromPoint(point, bInCheck);

	if (bInCheck && nIndex != LB_ERR && !IsCheckEnabled(nIndex))
	{
		MessageBeep((UINT) -1);
		return;
	}

	CCheckListBox::OnLButtonDown(nFlags, point);
}

void CMFCToolBarsListCheckBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE)
	{
		int nIndex = GetCaretIndex();
		if (nIndex != LB_ERR && !IsCheckEnabled(nIndex))
		{
			MessageBeep((UINT) -1);
			return;
		}
	}

	CCheckListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

LRESULT CMFCToolBarsListCheckBox::OnLBAddString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = CCheckListBox::OnLBAddString(wParam, lParam);
	OnNewString((int) lRes);
	return lRes;
}

LRESULT CMFCToolBarsListCheckBox::OnLBInsertString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = CCheckListBox::OnLBInsertString(wParam, lParam);
	OnNewString((int) lRes);
	return lRes;
}

LRESULT CMFCToolBarsListCheckBox::OnLBDeleteString(WPARAM wParam, LPARAM /*lParam*/)
{
	LRESULT lRes = Default();
	if (lRes != LB_ERR)
	{
		m_arCheckData.RemoveAt((int) wParam);
	}

	return lRes;
}

LRESULT CMFCToolBarsListCheckBox::OnLBResetContent(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_arCheckData.SetSize(0);
	return Default();
}

void CMFCToolBarsListCheckBox::EnableCheck(int nIndex, BOOL bEnable)
{
	ASSERT(nIndex >= 0 && nIndex < m_arCheckData.GetSize());
	m_arCheckData.SetAt(nIndex, bEnable);
}

BOOL CMFCToolBarsListCheckBox::IsCheckEnabled(int nIndex) const
{
	ASSERT(nIndex >= 0 && nIndex < m_arCheckData.GetSize());
	return m_arCheckData.GetAt(nIndex);
}

void CMFCToolBarsListCheckBox::OnNewString(int iIndex)
{
	if (iIndex >= 0)
	{
		int iSize = GetCount();
		m_arCheckData.SetSize(iSize);

		for (int i = iSize - 1; i > iIndex; i --)
		{
			m_arCheckData.SetAt(i, m_arCheckData.GetAt(i - 1));
		}

		m_arCheckData.SetAt(iIndex, TRUE); // Enabled by default
	}
}

void CMFCToolBarsListCheckBox::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// determine where the click is
	BOOL bInCheck;
	int nIndex = CheckFromPoint(point, bInCheck);

	if (bInCheck && nIndex != LB_ERR && !IsCheckEnabled(nIndex))
	{
		MessageBeep((UINT) -1);
		return;
	}

	CCheckListBox::OnLButtonDblClk(nFlags, point);
	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CLBN_CHKCHANGE), (LPARAM)m_hWnd);
}


