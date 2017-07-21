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
#include "afxribbonedit.h"
#include "afxvisualmanager.h"
#include "afxvisualmanageroffice2007.h"
#include "afxglobals.h"
#include "afxribbonbar.h"
#include "afxribbonpanel.h"
#include "afxtoolbarcomboboxbutton.h"
#include "afxtrackmouse.h"
#include "afxpopupmenu.h"
#include "afxspinbuttonctrl.h"
#include "afxdrawmanager.h"
#include "afxribboncategory.h"
#include "afxribbonpanelmenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CMFCRibbonSpinButtonCtrl : public CMFCSpinButtonCtrl
{
	friend class CMFCRibbonEdit;

	CMFCRibbonSpinButtonCtrl(CMFCRibbonEdit* pEdit = NULL)
	{
		m_bQuickAccessMode = FALSE;
		m_pEdit = pEdit;
	}

	virtual void OnDraw(CDC* pDC)
	{
		BOOL bIsDrawOnGlass = CMFCToolBarImages::m_bIsDrawOnGlass;

		if (m_bQuickAccessMode)
		{
			CMFCRibbonBar* pRibbonBar = DYNAMIC_DOWNCAST(CMFCRibbonBar, GetParent());
			if (pRibbonBar != NULL)
			{
				ASSERT_VALID(pRibbonBar);

				if (pRibbonBar->IsQuickAccessToolbarOnTop() && pRibbonBar->IsTransparentCaption())
				{
					CMFCToolBarImages::m_bIsDrawOnGlass = TRUE;
				}
			}
		}

		CMFCSpinButtonCtrl::OnDraw(pDC);

		CMFCToolBarImages::m_bIsDrawOnGlass = bIsDrawOnGlass;
	}

	//{{AFX_MSG(CMFCRibbonSpinButtonCtrl)
	afx_msg void OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL m_bQuickAccessMode;
	CMFCRibbonEdit* m_pEdit;
};

BEGIN_MESSAGE_MAP(CMFCRibbonSpinButtonCtrl, CMFCSpinButtonCtrl)
	//{{AFX_MSG_MAP(CMFCRibbonSpinButtonCtrl)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS, &CMFCRibbonSpinButtonCtrl::OnDeltapos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMFCRibbonSpinButtonCtrl::OnDeltapos(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	if (m_pEdit != NULL && GetBuddy()->GetSafeHwnd() != NULL)
	{
		CString str;
		GetBuddy()->GetWindowText(str);

		GetBuddy()->SetFocus();
		m_pEdit->SetEditText(str);
		m_pEdit->NotifyCommand(TRUE);
	}

	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonEdit

IMPLEMENT_DYNCREATE(CMFCRibbonEdit, CMFCRibbonButton)

// Construction/Destruction
CMFCRibbonEdit::CMFCRibbonEdit(UINT uiID, int nWidth, LPCTSTR lpszLabel, int nImage)
{
	CommonInit();

	m_nID = uiID;
	m_nWidth = nWidth;
	m_nWidthFloaty = nWidth;
	m_nSmallImageIndex = nImage;

	SetText(lpszLabel);
}

CMFCRibbonEdit::CMFCRibbonEdit()
{
	CommonInit();
}

void CMFCRibbonEdit::CommonInit()
{
	m_bIsEditFocused = FALSE;
	m_nWidth = 0;
	m_nWidthFloaty = 0;
	m_pWndEdit = NULL;
	m_pWndSpin = NULL;
	m_bForceDrawBorder = TRUE;
	m_bHasDropDownList = FALSE;
	m_bHasSpinButtons = FALSE;
	m_nMin = INT_MAX;
	m_nMax = INT_MAX;
	m_nAlign = ES_LEFT;
	m_szMargin = CSize(2, 3);
	m_nLabelImageWidth = 0;
}

CMFCRibbonEdit::~CMFCRibbonEdit()
{
	DestroyCtrl();
}

CSize CMFCRibbonEdit::GetIntermediateSize(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	int cx = m_bFloatyMode ? m_nWidthFloaty : m_nWidth;
	if (afxGlobalData.GetRibbonImageScale() > 1.)
	{
		cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * cx);
	}

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);

	int nTextHeight = tm.tmHeight;
	if ((nTextHeight % 2) != 0)
	{
		nTextHeight++;
	}

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);

		int nSmallButtonHeight = m_pParent->GetImageSize(FALSE).cy + 6;

		m_szMargin.cy = max(2, (nSmallButtonHeight - nTextHeight) / 2);
	}

	int cy = nTextHeight + 2 * m_szMargin.cy;

	m_nLabelImageWidth = 0;

	if (!m_bQuickAccessMode && !m_bFloatyMode)
	{
		CSize sizeImageSmall = GetImageSize(RibbonImageSmall);

		if (sizeImageSmall != CSize(0, 0))
		{
			m_nLabelImageWidth += sizeImageSmall.cx + 2 * m_szMargin.cx;
			cy = max(cy, sizeImageSmall.cy);
		}

		if (m_sizeTextRight.cx > 0)
		{
			m_nLabelImageWidth += m_sizeTextRight.cx + 2 * m_szMargin.cx;
			cy = max(cy, m_sizeTextRight.cy);
		}

		cx += m_nLabelImageWidth;
	}

	return CSize(cx, cy);
}

CSize CMFCRibbonEdit::GetCompactSize(CDC* pDC)
{
	ASSERT_VALID(this);

	int nLabelWidth = m_sizeTextRight.cx;
	m_sizeTextRight.cx = 0;

	CSize size = GetIntermediateSize(pDC);

	m_sizeTextRight.cx = nLabelWidth;
	return size;
}

void CMFCRibbonEdit::SetEditText(CString strText)
{
	ASSERT_VALID(this);

	if (m_strEdit != strText)
	{
		m_strEdit = strText;

		if (m_pWndEdit->GetSafeHwnd() != NULL)
		{
			m_pWndEdit->SetWindowText(m_strEdit);
		}

		Redraw();
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
				CMFCRibbonEdit* pOther = DYNAMIC_DOWNCAST(CMFCRibbonEdit, arButtons [i]);

				if (pOther != NULL && pOther != this)
				{
					ASSERT_VALID(pOther);

					pOther->m_bDontNotify = TRUE;
					pOther->SetEditText(strText);
					pOther->m_bDontNotify = FALSE;
				}
			}
		}
	}
}

void CMFCRibbonEdit::EnableSpinButtons(int nMin, int nMax)
{
	ASSERT_VALID(this);

	m_nMin = nMin;
	m_nMax = nMax;

	m_bHasSpinButtons = TRUE;

	if (m_pWndSpin->GetSafeHwnd() != NULL)
	{
		m_pWndSpin->SetRange32(m_nMin, m_nMax);
	}
}

void CMFCRibbonEdit::SetTextAlign(int nAlign)
{
	ASSERT_VALID(this);
	ASSERT(nAlign == ES_LEFT || nAlign == ES_CENTER || nAlign == ES_RIGHT);

	m_nAlign = nAlign;
}

void CMFCRibbonEdit::SetWidth(int nWidth, BOOL bInFloatyMode)
{
	ASSERT_VALID(this);

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (bInFloatyMode)
	{
		m_nWidthFloaty = nWidth;
	}
	else
	{
		m_nWidth = nWidth;
	}
}

void CMFCRibbonEdit::OnDraw(CDC* pDC)
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
	CRect rectCommandSaved = m_rectCommand;

	int cx = m_bFloatyMode ? m_nWidthFloaty : m_nWidth;
	if (afxGlobalData.GetRibbonImageScale() > 1.)
	{
		cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * cx);
	}

	m_rectCommand.left = m_rect.left = m_rect.right - cx;

	CMFCVisualManager::GetInstance()->OnFillRibbonButton(pDC, this);

	if (m_pWndEdit->GetSafeHwnd() == NULL)
	{
		CRect rectText = m_rectCommand;
		rectText.DeflateRect(m_szMargin);

		UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX;

		if (m_nAlign == ES_CENTER)
		{
			uiDTFlags |= DT_CENTER;
		}
		else if (m_nAlign == ES_RIGHT)
		{
			uiDTFlags |= DT_RIGHT;
		}

		DrawRibbonText(pDC, m_strEdit, rectText, uiDTFlags);
	}

	CMFCVisualManager::GetInstance()->OnDrawRibbonButtonBorder
		(pDC, this);

	m_bIsHighlighted = bIsHighlighted;
	m_rect = rectSaved;
	m_rectCommand = rectCommandSaved;
}

void CMFCRibbonEdit::OnDrawLabelAndImage(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_bQuickAccessMode || m_bFloatyMode)
	{
		return;
	}

	CSize sizeImageSmall = GetImageSize(RibbonImageSmall);

	int x = m_rect.left;

	if (sizeImageSmall != CSize(0, 0))
	{
		CRect rectImage = m_rect;
		rectImage.left += m_szMargin.cx;
		rectImage.right = rectImage.left + sizeImageSmall.cx;
		rectImage.OffsetRect(0, max(0, (rectImage.Height() - sizeImageSmall.cy) / 2));

		DrawImage(pDC, RibbonImageSmall, rectImage);

		x = rectImage.right;
	}

	if (m_sizeTextRight.cx > 0 && !m_bCompactMode)
	{
		COLORREF clrTextOld = (COLORREF)-1;

		if (IsDisabled())
		{
			clrTextOld = pDC->SetTextColor(CMFCVisualManager::GetInstance()->GetToolbarDisabledTextColor());
		}

		CRect rectText = m_rect;
		rectText.left = x + m_szMargin.cx;

		UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX;

		if (m_nAlign == ES_CENTER)
		{
			uiDTFlags |= DT_CENTER;
		}
		else if (m_nAlign == ES_RIGHT)
		{
			uiDTFlags |= DT_RIGHT;
		}

		DrawRibbonText(pDC, m_strText, rectText, uiDTFlags);

		if (clrTextOld != (COLORREF)-1)
		{
			pDC->SetTextColor(clrTextOld);
		}
	}
}

void CMFCRibbonEdit::OnDrawOnList(CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL /*bIsSelected*/, BOOL /*bHighlighted*/)
{
	const int nEditWidth = rect.Height() * 2;

	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectImage = rect;
	rectImage.right = rect.left + nTextOffset;

	CSize sizeImageSmall = GetImageSize(RibbonImageSmall);
	if (sizeImageSmall != CSize(0, 0))
	{
		rectImage.DeflateRect(1, 0);
		rectImage.top += max(0, (rectImage.Height() - sizeImageSmall.cy) / 2);
		rectImage.bottom = rectImage.top + sizeImageSmall.cy;

		DrawImage(pDC, RibbonImageSmall, rectImage);
	}

	CRect rectEdit = rect;
	rectEdit.left = rectEdit.right - nEditWidth;
	rectEdit.DeflateRect(1, 1);

	CRect rectText = rect;
	rectText.left += nTextOffset;
	rectText.right = rectEdit.left;
	const int nXMargin = 3;
	rectText.DeflateRect(nXMargin, 0);

	pDC->DrawText(strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	CRect rectSaved = m_rect;
	CRect rectCommandSaved = m_rectCommand;

	m_rect = rectEdit;
	m_rectCommand = m_rect;
	m_rectCommand.right -= 15;

	CMFCVisualManager::GetInstance()->OnFillRibbonButton(pDC, this);
	CMFCVisualManager::GetInstance()->OnDrawRibbonButtonBorder
		(pDC, this);

	if (m_bHasDropDownList)
	{
		CMFCToolBarComboBoxButton buttonDummy;
		CRect rectDropButton = rectEdit;
		rectDropButton.left = m_rectCommand.right;
		rectDropButton.DeflateRect(2, 2);

		CMFCVisualManager::GetInstance()->OnDrawComboDropButton(pDC, rectDropButton, FALSE, FALSE, FALSE, &buttonDummy);
	}
	else
	{
		CRect rectCaret = rectEdit;
		rectCaret.DeflateRect(3, 3);
		rectCaret.bottom--;

		rectCaret.right = rectCaret.left + 7;

		CPen* pOldPen = (CPen*) pDC->SelectStockObject(BLACK_PEN);

		pDC->MoveTo(rectCaret.left, rectCaret.top);
		pDC->LineTo(rectCaret.right, rectCaret.top);

		pDC->MoveTo(rectCaret.CenterPoint().x, rectCaret.top);
		pDC->LineTo(rectCaret.CenterPoint().x, rectCaret.bottom);

		pDC->MoveTo(rectCaret.left, rectCaret.bottom);
		pDC->LineTo(rectCaret.right, rectCaret.bottom);

		pDC->SelectObject(pOldPen);
	}

	m_rect = rectSaved;
	m_rectCommand = rectCommandSaved;
	m_bIsDisabled = bIsDisabled;
}

void CMFCRibbonEdit::OnLButtonDown(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnLButtonDown(point);
}

void CMFCRibbonEdit::OnLButtonUp(CPoint /*point*/)
{
	ASSERT_VALID(this);
}

void CMFCRibbonEdit::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonButton::CopyFrom(s);

	CMFCRibbonEdit& src = (CMFCRibbonEdit&) s;

	m_strEdit = src.m_strEdit;

	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;
		m_pWndEdit = NULL;
	}

	if (m_pWndSpin != NULL)
	{
		m_pWndSpin->DestroyWindow();
		delete m_pWndSpin;
		m_pWndSpin = NULL;
	}

	m_nWidth = src.m_nWidth;
	m_nWidthFloaty = src.m_nWidthFloaty;
	m_bHasSpinButtons = src.m_bHasSpinButtons;
	m_bHasDropDownList = src.m_bHasDropDownList;
	m_nMin = src.m_nMin;
	m_nMax = src.m_nMax;
	m_nAlign = src.m_nAlign;
	m_nLabelImageWidth = src.m_nLabelImageWidth;
}

void CMFCRibbonEdit::OnAfterChangeRect(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CMFCRibbonButton::OnAfterChangeRect(pDC);

	RepositionRibbonEditCtrl();
}

void CMFCRibbonEdit::RepositionRibbonEditCtrl()
{
	if (m_rect.IsRectEmpty())
	{
		if (m_pWndEdit->GetSafeHwnd() != NULL)
		{
			m_pWndEdit->ShowWindow(SW_HIDE);
		}

		if (m_pWndSpin->GetSafeHwnd() != NULL)
		{
			m_pWndSpin->ShowWindow(SW_HIDE);
		}

		return;
	}

	CRect rectCommandOld = m_rectCommand;

	m_Location = RibbonElementSingleInGroup;
	m_rectCommand = m_rect;

	if (m_pWndEdit == NULL)
	{
		DWORD dwEditStyle = WS_CHILD | ES_WANTRETURN | ES_AUTOHSCROLL | WS_TABSTOP;

		dwEditStyle |= m_nAlign;

		CWnd* pWndParent = GetParentWnd();
		ASSERT_VALID(pWndParent);

		if ((m_pWndEdit = CreateEdit(pWndParent, dwEditStyle)) == NULL)
		{
			return;
		}

		m_pWndEdit->SendMessage(EM_SETTEXTMODE, TM_PLAINTEXT);
		m_pWndEdit->SetEventMask(m_pWndEdit->GetEventMask() | ENM_CHANGE);
		m_pWndEdit->SetFont(GetTopLevelRibbonBar()->GetFont());
		m_pWndEdit->SetWindowText(m_strEdit);
	}

	if (rectCommandOld != m_rectCommand || !m_pWndEdit->IsWindowVisible())
	{
		CRect rectEdit = m_rectCommand;

		int cx = m_bFloatyMode ? m_nWidthFloaty : m_nWidth;
		if (afxGlobalData.GetRibbonImageScale() > 1.)
		{
			cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * cx);
		}

		rectEdit.left = rectEdit.right - cx;

		if (m_bHasSpinButtons)
		{
			rectEdit.DeflateRect(m_szMargin.cx, m_szMargin.cy, 2, m_szMargin.cy);
		}
		else
		{
			rectEdit.DeflateRect(m_szMargin.cx, m_szMargin.cy);
		}

		m_pWndEdit->SetWindowPos(NULL, rectEdit.left, rectEdit.top, rectEdit.Width(), rectEdit.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		m_pWndEdit->ShowWindow(SW_SHOWNOACTIVATE);

		if (m_pWndSpin->GetSafeHwnd() != NULL)
		{
			m_pWndSpin->m_bQuickAccessMode = m_bQuickAccessMode;
			m_pWndSpin->SetBuddy(m_pWndEdit);
			m_pWndSpin->ShowWindow(SW_SHOWNOACTIVATE);
		}
	}
}

CMFCRibbonRichEditCtrl* CMFCRibbonEdit::CreateEdit(CWnd* pWndParent, DWORD dwEditStyle)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pWndParent);

	CMFCRibbonRichEditCtrl* pWndEdit = new CMFCRibbonRichEditCtrl(*this);

	if (!pWndEdit->Create(dwEditStyle, CRect(0, 0, 0, 0), pWndParent, m_nID))
	{
		delete pWndEdit;
		return NULL;
	}

	if (m_bHasSpinButtons)
	{
		CreateSpinButton(pWndEdit, pWndParent);
	}

	return pWndEdit;
}

BOOL CMFCRibbonEdit::CreateSpinButton(CMFCRibbonRichEditCtrl* pWndEdit, CWnd* pWndParent)
{
	ASSERT_VALID(this);
	ASSERT(m_pWndSpin == NULL);

	if (!m_bHasSpinButtons)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_pWndSpin = new CMFCRibbonSpinButtonCtrl(this);

	if (!m_pWndSpin->Create(WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT, CRect(0, 0, 0, 0), pWndParent, m_nID))
	{
		delete m_pWndSpin;
		return FALSE;
	}

	m_pWndSpin->SetBuddy(pWndEdit);
	m_pWndSpin->SetRange32(m_nMin, m_nMax);

	return TRUE;
}

void CMFCRibbonEdit::OnShow(BOOL bShow)
{
	ASSERT_VALID(this);

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->ShowWindow(bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	}

	if (m_pWndSpin->GetSafeHwnd() != NULL)
	{
		m_pWndSpin->ShowWindow(bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	}
}

void CMFCRibbonEdit::Redraw()
{
	ASSERT_VALID(this);

	CMFCRibbonButton::Redraw();

	if (m_pWndEdit->GetSafeHwnd() != NULL && m_pWndEdit->IsWindowVisible())
	{
		m_pWndEdit->RedrawWindow();
	}

	if (m_pWndSpin->GetSafeHwnd() != NULL && m_pWndSpin->IsWindowVisible())
	{
		m_pWndSpin->RedrawWindow();
	}
}

void CMFCRibbonEdit::OnHighlight(BOOL bHighlight)
{
	ASSERT_VALID(this);

	CMFCRibbonButton::OnHighlight(bHighlight);

	if (m_pWndEdit->GetSafeHwnd() != NULL && m_pWndEdit->IsWindowVisible())
	{
		m_pWndEdit->m_bIsHighlighted = bHighlight;
		m_pWndEdit->RedrawWindow();
	}
}

void CMFCRibbonEdit::OnEnable(BOOL bEnable)
{
	ASSERT_VALID(this);

	CMFCRibbonButton::OnEnable(bEnable);

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->EnableWindow(bEnable);
	}

	if (m_pWndSpin->GetSafeHwnd() != NULL)
	{
		m_pWndSpin->EnableWindow(bEnable);
	}
}

BOOL CMFCRibbonEdit::OnKey(BOOL bIsMenuKey)
{
	ASSERT_VALID(this);

	if (m_bIsDisabled)
	{
		return FALSE;
	}

	if (m_rect.IsRectEmpty())
	{
		return CMFCRibbonBaseElement::OnKey(bIsMenuKey);
	}

	CMFCRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar();
	if (pTopLevelRibbon != NULL)
	{
		pTopLevelRibbon->HideKeyTips();
	}

	if (bIsMenuKey)
	{
		DropDownList();
		return TRUE;
	}

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->SetFocus();

		if (!m_strEdit.IsEmpty())
		{
			m_pWndEdit->SetSel(0, -1);
		}

		return TRUE;
	}

	DropDownList();
	return TRUE;
}

void CMFCRibbonEdit::OnRTLChanged(BOOL bIsRTL)
{
	ASSERT_VALID(this);

	CMFCRibbonButton::OnRTLChanged(bIsRTL);

	m_rectCommand.SetRectEmpty();
	m_rectMenu.SetRectEmpty();

	if (!m_rect.IsRectEmpty() && m_pWndEdit->GetSafeHwnd() != NULL && m_pWndEdit->IsWindowVisible())
	{
		CClientDC dc(GetParentWnd());
		OnAfterChangeRect(&dc);
	}
}

void CMFCRibbonEdit::DestroyCtrl()
{
	ASSERT_VALID(this);

	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;
		m_pWndEdit = NULL;
	}

	if (m_pWndSpin != NULL)
	{
		m_pWndSpin->DestroyWindow();
		delete m_pWndSpin;
		m_pWndSpin = NULL;
	}
}

BOOL CMFCRibbonEdit::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);

	CMFCRibbonButton::SetACCData(pParent, data);

	data.m_strAccValue = m_strEdit;

	if (!IsMenuAreaHighlighted())
	{
		data.m_nAccRole = ROLE_SYSTEM_TEXT;
	}
	else
	{
		data.m_bAccState = 0;
		data.m_nAccRole = ROLE_SYSTEM_PUSHBUTTON;
	}

	if (IsFocused())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonRichEditCtrl

CMFCRibbonRichEditCtrl::CMFCRibbonRichEditCtrl(CMFCRibbonEdit& edit) : m_edit(edit)
{
	m_bTracked = FALSE;
	m_bIsHighlighted = FALSE;
	m_bIsContextMenu = FALSE;
}

CMFCRibbonRichEditCtrl::~CMFCRibbonRichEditCtrl()
{
}

//{{AFX_MSG_MAP(CMFCRibbonRichEditCtrl)
BEGIN_MESSAGE_MAP(CMFCRibbonRichEditCtrl, CRichEditCtrl)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCRibbonRichEditCtrl::OnMouseLeave)
	ON_CONTROL_REFLECT(EN_CHANGE, &CMFCRibbonRichEditCtrl::OnChange)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonRichEditCtrl message handlers

BOOL CMFCRibbonRichEditCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_ERASEBKGND)
	{
		return TRUE;
	}

	if (m_bIsContextMenu)
	{
		return CRichEditCtrl::PreTranslateMessage(pMsg);
	}

	if (pMsg->message == WM_MOUSEMOVE && !m_edit.IsDisabled())
	{
		if (!m_bTracked)
		{
			m_bTracked = TRUE;

			TRACKMOUSEEVENT trackmouseevent;
			trackmouseevent.cbSize = sizeof(trackmouseevent);
			trackmouseevent.dwFlags = TME_LEAVE;
			trackmouseevent.hwndTrack = GetSafeHwnd();
			trackmouseevent.dwHoverTime = HOVER_DEFAULT;

			::AFXTrackMouse(&trackmouseevent);

			RedrawWindow();
		}

		if (!m_bIsHighlighted && m_edit.GetParentWnd() != NULL)
		{
			CPoint point;
			::GetCursorPos(&point);

			m_bIsHighlighted = TRUE;
			RedrawWindow();

			m_edit.GetParentWnd()->ScreenToClient(&point);
			m_edit.GetParentWnd()->SendMessage(WM_MOUSEMOVE, 0, MAKELPARAM(point.x, point.y));
		}
	}

	if (pMsg->message == WM_KEYDOWN && !m_edit.IsDisabled())
	{
		if (ProcessClipboardAccelerators((UINT) pMsg->wParam))
		{
			return TRUE;
		}

		switch (pMsg->wParam)
		{
		case VK_DOWN:
			if (m_edit.m_bHasDropDownList && !m_edit.IsDroppedDown())
			{
				m_edit.DropDownList();
				return TRUE;
			}

		case VK_UP:
		case VK_PRIOR:
		case VK_NEXT:
			if (m_edit.IsDroppedDown())
			{
				::SendMessage( CMFCPopupMenu::GetActiveMenu()->GetSafeHwnd(), WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
				return TRUE;
			}
			break;

		case VK_TAB:
			if (m_edit.GetParentPanel() != NULL)
			{
				ASSERT_VALID(m_edit.GetParentPanel());
				m_edit.GetParentPanel()->OnKey(VK_TAB);
				return TRUE;
			}
			break;

		case VK_RETURN:
			if (!m_edit.IsDroppedDown())
			{
				CString str;
				GetWindowText(str);

				m_edit.SetEditText(str);
				m_edit.NotifyCommand(TRUE);

				if (m_edit.m_pParentMenu != NULL)
				{
					ASSERT_VALID(m_edit.m_pParentMenu);

					CFrameWnd* pParentFrame = AFXGetParentFrame(m_edit.m_pParentMenu);
					ASSERT_VALID(pParentFrame);

					pParentFrame->DestroyWindow();
					return TRUE;
				}

				if (GetTopLevelFrame() != NULL)
				{
					GetTopLevelFrame()->SetFocus();
					return TRUE;
				}
			}
			break;

		case VK_ESCAPE:
			if (m_edit.IsDroppedDown() && CMFCPopupMenu::GetActiveMenu() != NULL)
			{
				CMFCPopupMenu::GetActiveMenu()->SendMessage(WM_CLOSE);
				return TRUE;
			}

			if (!m_edit.IsDroppedDown())
			{
				SetWindowText(m_strOldText);
				m_edit.SetEditText(m_strOldText);
			}

			if (GetTopLevelFrame() != NULL && !m_edit.IsDroppedDown())
			{
				GetTopLevelFrame()->SetFocus();
				return TRUE;
			}
			break;
		}
	}

	return CRichEditCtrl::PreTranslateMessage(pMsg);
}

BOOL CMFCRibbonRichEditCtrl::ProcessClipboardAccelerators(UINT nChar)
{
	BOOL bIsCtrl = (::GetAsyncKeyState(VK_CONTROL) & 0x8000);
	BOOL bIsShift = (::GetAsyncKeyState(VK_SHIFT) & 0x8000);

	if (bIsCtrl &&(nChar == _T('C') || nChar == VK_INSERT))
	{
		SendMessage(WM_COPY);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('V') ||(bIsShift && nChar == VK_INSERT))
	{
		SendMessage(WM_PASTE);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('X') ||(bIsShift && nChar == VK_DELETE))
	{
		SendMessage(WM_CUT);
		return TRUE;
	}

	return FALSE;
}

void CMFCRibbonRichEditCtrl::OnChange()
{
	CString strText;
	GetWindowText(strText);

	m_edit.m_strEdit = strText;
	m_edit.SetEditText(strText);
}

void CMFCRibbonRichEditCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CRichEditCtrl::OnSetFocus(pOldWnd);

	m_edit.m_bIsEditFocused = TRUE;
	m_edit.Redraw();

	GetWindowText(m_strOldText);
}

void CMFCRibbonRichEditCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CRichEditCtrl::OnKillFocus(pNewWnd);

	m_edit.m_bIsEditFocused = FALSE;
	m_edit.Redraw();

	SetSel(0, 0);
	RedrawWindow();

	GetWindowText(m_strOldText);
	m_edit.m_strEdit = m_strOldText;
	m_edit.NotifyCommand(TRUE);
}

void CMFCRibbonRichEditCtrl::OnPaint()
{
	CRect rect;
	GetClientRect(rect);

	BOOL bIsHighlighted = m_edit.IsHighlighted() || m_edit.IsDroppedDown() || m_edit.IsFocused() || m_bIsHighlighted;
	BOOL bIsDisabled = m_edit.IsDisabled();
	COLORREF clrBackground = (bIsHighlighted && !bIsDisabled) ? afxGlobalData.clrWindow : afxGlobalData.clrBarFace;

	CMFCVisualManager* pVisualManager = CMFCVisualManager::GetInstance();
	CMFCVisualManagerOffice2007* pVisualManager2007 = DYNAMIC_DOWNCAST(CMFCVisualManagerOffice2007, pVisualManager);
	if (pVisualManager2007 != NULL)
	{
		clrBackground = pVisualManager2007->GetRibbonEditBackgroundColor(bIsHighlighted, bIsDisabled);
	}

	SetBackgroundColor(FALSE, clrBackground);
	Default();

	if (m_edit.m_bQuickAccessMode && m_edit.m_pRibbonBar != NULL)
	{
		ASSERT_VALID(m_edit.m_pRibbonBar);

		if (m_edit.m_pRibbonBar->IsQuickAccessToolbarOnTop() && m_edit.m_pRibbonBar->IsTransparentCaption())
		{
			CClientDC dc(this);
			CDrawingManager dm(dc);
			dm.FillAlpha(rect);
		}
	}
}

void CMFCRibbonRichEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int nOldX = GetCharPos(0).x;

	CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

	if (nOldX != GetCharPos(0).x)
	{
		RedrawWindow();
	}
}

LRESULT CMFCRibbonRichEditCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	if (m_edit.GetParentWnd() != NULL)
	{
		m_edit.GetParentWnd()->SendMessage(WM_MOUSELEAVE);
	}

	if (m_bIsHighlighted)
	{
		m_bIsHighlighted = FALSE;
		RedrawWindow();
	}

	return 0;
}

void CMFCRibbonRichEditCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	CMFCRibbonBar* pRibbonBar = m_edit.GetTopLevelRibbonBar();
	if (pRibbonBar != NULL)
	{
		ASSERT_VALID(pRibbonBar);

		m_bIsContextMenu = TRUE;

		pRibbonBar->OnEditContextMenu(this, point);

		m_bIsContextMenu = FALSE;
	}
}





