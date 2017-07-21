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
#include "afxcontrolbarutil.h"
#include "afxtoolbarspineditboxbutton.h"
#include "afxaccessibility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CMFCToolBarSpinEditBoxButton, CMFCToolBarEditBoxButton, 1)

CMFCToolBarSpinEditBoxButton::CMFCToolBarSpinEditBoxButton()
{
	Init();
}

CMFCToolBarSpinEditBoxButton::CMFCToolBarSpinEditBoxButton(UINT uiId, int iImage, DWORD dwStyle, int iWidth) :
	CMFCToolBarEditBoxButton(uiId, iImage, dwStyle, iWidth)
{
	Init();
}

void CMFCToolBarSpinEditBoxButton::Init()
{
	m_nMin = INT_MIN;
	m_nMax = INT_MAX;
}

CMFCToolBarSpinEditBoxButton::~CMFCToolBarSpinEditBoxButton()
{
	if (m_wndSpin.GetSafeHwnd() != NULL)
	{
		m_wndSpin.DestroyWindow();
	}
}

BOOL CMFCToolBarSpinEditBoxButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParent);

	if (!CMFCToolBarEditBoxButton::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_strAccValue = m_strText;
	return TRUE;
}

CEdit* CMFCToolBarSpinEditBoxButton::CreateEdit(CWnd* pWndParent, const CRect& rect)
{
	CEdit *pEdit = CMFCToolBarEditBoxButton::CreateEdit(pWndParent,rect);
	if (pEdit == NULL)
	{
		return NULL;
	}

	if (!m_wndSpin.Create(WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT, rect, pWndParent, m_nID))
		return NULL;

	m_wndSpin.SetBuddy(pEdit);
	m_wndSpin.SetRange32(m_nMin, m_nMax);

	return pEdit;
}

void CMFCToolBarSpinEditBoxButton::OnMove()
{
	CMFCToolBarEditBoxButton::OnMove();

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_wndSpin.SetBuddy(m_pWndEdit);
	}
}

void CMFCToolBarSpinEditBoxButton::GetEditBorder(CRect& rectBorder)
{
	ASSERT(m_pWndEdit->GetSafeHwnd() != NULL);

	m_pWndEdit->GetWindowRect(rectBorder);
	m_pWndEdit->GetParent()->ScreenToClient(rectBorder);

	CRect rectSpin;
	m_wndSpin.GetWindowRect(rectSpin);
	m_wndSpin.GetParent()->ScreenToClient(rectSpin);

	rectBorder.right = rectSpin.right;

	rectBorder.InflateRect(1, 1);
}

void CMFCToolBarSpinEditBoxButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarEditBoxButton::CopyFrom(s);

	const CMFCToolBarSpinEditBoxButton& src = (const CMFCToolBarSpinEditBoxButton&) s;

	m_nMin = src.m_nMin;
	m_nMax = src.m_nMax;
}

void CMFCToolBarSpinEditBoxButton::Serialize(CArchive& ar)
{
	CMFCToolBarEditBoxButton::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_nMin;
		ar >> m_nMax;
	}
	else
	{
		ar << m_nMin;
		ar << m_nMax;
	}
}

void CMFCToolBarSpinEditBoxButton::SetRange(int nMin, int nMax)
{
	ASSERT_VALID(this);

	m_nMin = nMin;
	m_nMax = nMax;

	if (m_wndSpin.GetSafeHwnd() != NULL)
	{
		m_wndSpin.SetRange32(nMin, nMax);
	}
}

void CMFCToolBarSpinEditBoxButton::GetRange(int& nMin, int& nMax)
{
	ASSERT_VALID(this);

	nMin = m_nMin;
	nMax = m_nMax;
}

BOOL CMFCToolBarSpinEditBoxButton::OnUpdateToolTip(CWnd* /*pWndParent*/, int /*iButtonIndex*/, CToolTipCtrl& wndToolTip, CString& strTipText)
{
	CEdit* pEdit = GetEditBox();
	CSpinButtonCtrl* pSpin = GetSpinControl();

	if ((pEdit != NULL) &&(::IsWindow(pEdit->GetSafeHwnd())))
	{
		CString strTips;

		if (OnGetCustomToolTipText(strTips))
		{
			wndToolTip.AddTool(pEdit, strTips, NULL, 0);
			wndToolTip.AddTool(pSpin, strTips, NULL, 0);
		}
		else
		{
			wndToolTip.AddTool(pEdit, strTipText, NULL, 0);
			wndToolTip.AddTool(pSpin, strTipText, NULL, 0);
		}

		return TRUE;
	}

	return FALSE;
}

void CMFCToolBarSpinEditBoxButton::OnShowEditbox(BOOL bShow)
{
	if (m_wndSpin.GetSafeHwnd() != NULL)
	{
		m_wndSpin.ShowWindow(bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	}
}


