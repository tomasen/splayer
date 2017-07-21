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
#include "afxglobals.h"
#include "afxtoolbar.h"
#include "afxtoolbarmenubutton.h"
#include "afxmenuimages.h"
#include "afxtoolbardatetimectrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMFCToolBarDateTimeCtrl, CMFCToolBarButton, 1)

static const int nDefaultSize = 100;
static const int nHorzMargin = 3;
static const int nVertMargin = 3;

BEGIN_MESSAGE_MAP(CMFCToolBarDateTimeCtrlImpl, CDateTimeCtrl)
	//{{AFX_MSG_MAP(CMFCToolBarDateTimeCtrlImpl)
	ON_NOTIFY_REFLECT(DTN_DATETIMECHANGE, &CMFCToolBarDateTimeCtrlImpl::OnDateTimeChange)
	ON_NOTIFY_REFLECT(DTN_DROPDOWN, &CMFCToolBarDateTimeCtrlImpl::OnDateTimeDropDown)
	ON_NOTIFY_REFLECT(DTN_CLOSEUP, &CMFCToolBarDateTimeCtrlImpl::OnDateTimeCloseUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#pragma warning(disable : 4310)

void CMFCToolBarDateTimeCtrlImpl::OnDateTimeChange(NMHDR* pNotifyStruct, LRESULT* pResult)
{
	ENSURE(pNotifyStruct != NULL);

	// Send a WM_COMMAND message with the DTN_DATETIMECHANGE notification(truncated to lower 16 bits)
	// so that we will process it first then it will get forwarded by CMFCToolBar to the current
	// frame window, otherwise everyone has to subclass this class and reflect the WM_NOTIFY messages.
	// Yeah, a hack, but a pretty safe one.
	if (!m_bMonthCtrlDisplayed)
	{
		LPNMDATETIMECHANGE pNotify = (LPNMDATETIMECHANGE) pNotifyStruct;
		GetOwner()->PostMessage(WM_COMMAND, pNotify->nmhdr.idFrom);
	}

	*pResult = 0;
}

void CMFCToolBarDateTimeCtrlImpl::OnDateTimeDropDown(NMHDR* /* pNotifyStruct */, LRESULT* pResult)
{
	m_bMonthCtrlDisplayed = true;
	*pResult = 0;
}

void CMFCToolBarDateTimeCtrlImpl::OnDateTimeCloseUp(NMHDR* pNotifyStruct, LRESULT* pResult)
{
	ENSURE(pNotifyStruct != NULL);

	m_bMonthCtrlDisplayed = false;

	LPNMDATETIMECHANGE pNotify = (LPNMDATETIMECHANGE) pNotifyStruct;
	GetOwner()->PostMessage(WM_COMMAND, pNotify->nmhdr.idFrom);

	*pResult = 0;
}

#pragma warning(default : 4310)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCToolBarDateTimeCtrl::CMFCToolBarDateTimeCtrl()
{
	m_dwStyle = WS_CHILD | WS_VISIBLE;
	m_iWidth = nDefaultSize;

	Initialize();
}

CMFCToolBarDateTimeCtrl::CMFCToolBarDateTimeCtrl(UINT uiId, int iImage, DWORD dwStyle, int iWidth) : CMFCToolBarButton(uiId, iImage)
{
	m_dwStyle = dwStyle | WS_CHILD | WS_VISIBLE;
	m_iWidth = (iWidth == 0) ? nDefaultSize : iWidth;

	Initialize();
}

void CMFCToolBarDateTimeCtrl::Initialize()
{
	m_pWndDateTime = NULL;
	m_bHorz = TRUE;
	m_dwTimeStatus = GDT_VALID;
	m_time = CTime::GetCurrentTime();
}

CMFCToolBarDateTimeCtrl::~CMFCToolBarDateTimeCtrl()
{
	if (m_pWndDateTime != NULL)
	{
		m_pWndDateTime->DestroyWindow();
		delete m_pWndDateTime;
	}
}

void CMFCToolBarDateTimeCtrl::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarButton::CopyFrom(s);

	DuplicateData();

	const CMFCToolBarDateTimeCtrl& src = (const CMFCToolBarDateTimeCtrl&) s;
	m_dwStyle = src.m_dwStyle;
	m_iWidth = src.m_iWidth;
}

void CMFCToolBarDateTimeCtrl::Serialize(CArchive& ar)
{
	CMFCToolBarButton::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_dwTimeStatus;
		ar >> m_time;

		if (m_pWndDateTime->GetSafeHwnd () != NULL &&
			m_dwTimeStatus == GDT_VALID)
		{
			m_pWndDateTime->SetTime(&m_time);
		}

		DuplicateData();
	}
	else
	{
		if (m_pWndDateTime->GetSafeHwnd () != NULL)
		{
			m_dwTimeStatus = m_pWndDateTime->GetTime(m_time);
		}

		ar << m_iWidth;
		ar << m_dwStyle;
		ar << m_dwTimeStatus;
		ar << m_time;
	}
}

SIZE CMFCToolBarDateTimeCtrl::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (!IsVisible())
	{
		return CSize(0,0);
	}

	m_bHorz = bHorz;
	m_sizeText = CSize(0, 0);

	if (bHorz)
	{
		if (m_pWndDateTime->GetSafeHwnd() != NULL && !m_bIsHidden)
		{
			m_pWndDateTime->ShowWindow(SW_SHOWNOACTIVATE);
		}

		if (m_bTextBelow && !m_strText.IsEmpty())
		{
			CRect rectText(0, 0, m_iWidth, sizeDefault.cy);
			pDC->DrawText(m_strText, rectText, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
			m_sizeText = rectText.Size();
		}

		return CSize(m_iWidth, sizeDefault.cy + m_sizeText.cy);
	}
	else
	{
		if (m_pWndDateTime->GetSafeHwnd() != NULL && (m_pWndDateTime->GetStyle() & WS_VISIBLE))
		{
			m_pWndDateTime->ShowWindow(SW_HIDE);
		}

		return CMFCToolBarButton::OnCalculateSize(pDC, sizeDefault, bHorz);
	}
}

void CMFCToolBarDateTimeCtrl::OnMove()
{
	if (m_pWndDateTime->GetSafeHwnd() == NULL || (m_pWndDateTime->GetStyle() & WS_VISIBLE) == 0)
	{
		return;
	}

	CRect rectDateTime;
	m_pWndDateTime->GetWindowRect(rectDateTime);

	m_pWndDateTime->SetWindowPos(NULL, m_rect.left + nHorzMargin, m_rect.top +(m_rect.Height() - m_sizeText.cy - rectDateTime.Height()) / 2,
		m_rect.Width() - 2 * nHorzMargin, afxGlobalData.GetTextHeight() + 2 * nVertMargin, SWP_NOZORDER | SWP_NOACTIVATE);

	AdjustRect();
}

void CMFCToolBarDateTimeCtrl::OnSize(int iSize)
{
	m_iWidth = iSize;
	m_rect.right = m_rect.left + m_iWidth;

	if (m_pWndDateTime->GetSafeHwnd() != NULL && (m_pWndDateTime->GetStyle() & WS_VISIBLE))
	{
		m_pWndDateTime->SetWindowPos(NULL, m_rect.left + nHorzMargin, m_rect.top,
			m_rect.Width() - 2 * nHorzMargin, afxGlobalData.GetTextHeight() + 2 * nVertMargin, SWP_NOZORDER | SWP_NOACTIVATE);

		AdjustRect();
	}
}

void CMFCToolBarDateTimeCtrl::OnChangeParentWnd(CWnd* pWndParent)
{
	CMFCToolBarButton::OnChangeParentWnd(pWndParent);

	if (m_pWndDateTime->GetSafeHwnd() != NULL)
	{
		CWnd* pWndParentCurr = m_pWndDateTime->GetParent();
		ENSURE(pWndParentCurr != NULL);

		if (pWndParent != NULL && pWndParentCurr->GetSafeHwnd() == pWndParent->GetSafeHwnd())
		{
			return;
		}

		m_pWndDateTime->DestroyWindow();
		delete m_pWndDateTime;
		m_pWndDateTime = NULL;
	}

	if (pWndParent == NULL || pWndParent->GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rect = m_rect;
	rect.InflateRect(-2, 0);
	rect.bottom = rect.top + afxGlobalData.GetTextHeight() + 2 * nVertMargin;

	if ((m_pWndDateTime = CreateDateTimeCtrl(pWndParent, rect)) == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	AdjustRect();

	m_pWndDateTime->SetFont(&afxGlobalData.fontRegular);
	if (m_dwTimeStatus == GDT_VALID)
	{
		m_pWndDateTime->SetTime(&m_time);
	}
}

void CMFCToolBarDateTimeCtrl::AdjustRect()
{
	if (m_pWndDateTime->GetSafeHwnd() == NULL || (m_pWndDateTime->GetStyle() & WS_VISIBLE) == 0 || m_rect.IsRectEmpty())
	{
		return;
	}

	m_pWndDateTime->GetWindowRect(&m_rect);
	m_pWndDateTime->ScreenToClient(&m_rect);
	m_pWndDateTime->MapWindowPoints(m_pWndDateTime->GetParent(), &m_rect);
	m_rect.InflateRect(nHorzMargin, nVertMargin);
}


#pragma warning(disable : 4310)

BOOL CMFCToolBarDateTimeCtrl::NotifyCommand(int iNotifyCode)
{
	if (m_pWndDateTime->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	switch(iNotifyCode)
	{
	case LOWORD(DTN_DATETIMECHANGE):
	case DTN_DATETIMECHANGE:
		{
			m_dwTimeStatus = m_pWndDateTime->GetTime(m_time);

			//------------------------------------------------------
			// Try set selection in ALL DateTimeCtrl's with the same ID:
			//------------------------------------------------------
			CObList listButtons;
			if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition(); posCombo != NULL;)
				{
					CMFCToolBarDateTimeCtrl* pDateTime = DYNAMIC_DOWNCAST(CMFCToolBarDateTimeCtrl, listButtons.GetNext(posCombo));
					ENSURE(pDateTime != NULL);

					if (pDateTime != this && m_dwTimeStatus == GDT_VALID)
					{
						pDateTime->m_pWndDateTime->SetTime(&m_time);
					}
				}
			}
		}

		return TRUE;
	}

	return TRUE;
}

#pragma warning(default : 4310)


void CMFCToolBarDateTimeCtrl::OnAddToCustomizePage()
{
	CObList listButtons; // Existing buttons with the same command ID

	if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) == 0)
	{
		return;
	}

	CMFCToolBarDateTimeCtrl* pOther = (CMFCToolBarDateTimeCtrl*) listButtons.GetHead();
	ASSERT_VALID(pOther);
	ASSERT_KINDOF(CMFCToolBarDateTimeCtrl, pOther);

	CopyFrom(*pOther);
}

HBRUSH CMFCToolBarDateTimeCtrl::OnCtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor(afxGlobalData.clrWindowText);
	pDC->SetBkColor(afxGlobalData.clrWindow);

	return(HBRUSH) afxGlobalData.brWindow.GetSafeHandle();
}

void CMFCToolBarDateTimeCtrl::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages,
	BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	if (m_pWndDateTime->GetSafeHwnd() == NULL || (m_pWndDateTime->GetStyle() & WS_VISIBLE) == 0)
	{
		CMFCToolBarButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);
	}
	else if ((m_bTextBelow && bHorz) && !m_strText.IsEmpty())
	{
		//--------------------
		// Draw button's text:
		//--------------------
		BOOL bDisabled = (bCustomizeMode && !IsEditable()) || (!bCustomizeMode &&(m_nStyle & TBBS_DISABLED));

		pDC->SetTextColor(bDisabled ? afxGlobalData.clrGrayedText : (bHighlight) ? CMFCToolBar::GetHotTextColor() : afxGlobalData.clrBarText);
		CRect rectText;
		rectText.left = (rect.left + rect.right - m_sizeText.cx) / 2;
		rectText.right = (rect.left + rect.right + m_sizeText.cx) / 2;
		rectText.top = rect.bottom + rect.top;
		rectText.bottom = rectText.top + m_sizeText.cy;
		pDC->DrawText(m_strText, &rectText, DT_CENTER | DT_WORDBREAK);
	}
}

BOOL CMFCToolBarDateTimeCtrl::OnClick(CWnd* /*pWnd*/, BOOL /*bDelay*/)
{
	return m_pWndDateTime->GetSafeHwnd() != NULL && (m_pWndDateTime->GetStyle() & WS_VISIBLE);
}

int CMFCToolBarDateTimeCtrl::OnDrawOnCustomizeList(CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int iWidth = CMFCToolBarButton::OnDrawOnCustomizeList(pDC, rect, bSelected) + 10;

	//------------------------------
	// Simulate DateTimeCtrl appearance:
	//------------------------------
	CRect rectDateTime = rect;
	int iDateTimeWidth = rect.Width() - iWidth;

	if (iDateTimeWidth < 20)
	{
		iDateTimeWidth = 20;
	}

	rectDateTime.left = rectDateTime.right - iDateTimeWidth;
	rectDateTime.DeflateRect(2, 3);

	pDC->FillSolidRect(rectDateTime, afxGlobalData.clrWindow);
	pDC->Draw3dRect(&rectDateTime, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarHilite);

	rectDateTime.DeflateRect(1, 1);

	pDC->Draw3dRect(&rectDateTime, afxGlobalData.clrBarShadow, afxGlobalData.clrBarLight);

	CRect rectBtn = rectDateTime;
	rectBtn.left = rectBtn.right - rectBtn.Height();
	rectBtn.DeflateRect(1, 1);

	pDC->FillSolidRect(rectBtn, afxGlobalData.clrBarFace);
	pDC->Draw3dRect(&rectBtn, afxGlobalData.clrBarHilite, afxGlobalData.clrBarDkShadow);

	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rectBtn);

	return rect.Width();
}

CMFCToolBarDateTimeCtrlImpl* CMFCToolBarDateTimeCtrl::CreateDateTimeCtrl(CWnd* pWndParent, const CRect& rect)
{
	CMFCToolBarDateTimeCtrlImpl* pWndDateTime = new CMFCToolBarDateTimeCtrlImpl;
	if (!pWndDateTime->Create(m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndDateTime;
		return NULL;
	}

	return pWndDateTime;
}

void CMFCToolBarDateTimeCtrl::OnShow(BOOL bShow)
{
	if (m_pWndDateTime->GetSafeHwnd() != NULL)
	{
		if (bShow && m_bHorz)
		{
			m_pWndDateTime->ShowWindow(SW_SHOWNOACTIVATE);
			OnMove();
		}
		else
		{
			m_pWndDateTime->ShowWindow(SW_HIDE);
		}
	}
}

BOOL CMFCToolBarDateTimeCtrl::ExportToMenuButton(CMFCToolBarMenuButton& menuButton) const
{
	CString strMessage;
	int iOffset;

	if (strMessage.LoadString(m_nID) && (iOffset = strMessage.Find(_T('\n'))) != -1)
	{
		menuButton.m_strText = strMessage.Mid(iOffset + 1);
	}

	return TRUE;
}

BOOL CMFCToolBarDateTimeCtrl::SetTime(LPSYSTEMTIME pTimeNew /* = NULL */)
{
	BOOL bResult = m_pWndDateTime->SetTime(pTimeNew);
	NotifyCommand(DTN_DATETIMECHANGE);
	return bResult;
}

BOOL CMFCToolBarDateTimeCtrl::SetTime(const COleDateTime& timeNew)
{
	BOOL bResult = m_pWndDateTime->SetTime(timeNew);
	NotifyCommand(DTN_DATETIMECHANGE);
	return bResult;
}

BOOL CMFCToolBarDateTimeCtrl::SetTime(const CTime* pTimeNew)
{
	BOOL bResult = m_pWndDateTime->SetTime(pTimeNew);
	NotifyCommand(DTN_DATETIMECHANGE);
	return bResult;
}

CMFCToolBarDateTimeCtrl* __stdcall CMFCToolBarDateTimeCtrl::GetByCmd(UINT uiCmd)
{
	CMFCToolBarDateTimeCtrl* pSrcDateTime = NULL;

	CObList listButtons;
	if (CMFCToolBar::GetCommandButtons(uiCmd, listButtons) > 0)
	{
		for (POSITION posDateTime= listButtons.GetHeadPosition(); pSrcDateTime == NULL && posDateTime != NULL;)
		{
			CMFCToolBarDateTimeCtrl* pDateTime= DYNAMIC_DOWNCAST(CMFCToolBarDateTimeCtrl, listButtons.GetNext(posDateTime));
			ENSURE(pDateTime != NULL);

			pSrcDateTime = pDateTime;
		}
	}

	return pSrcDateTime;
}

BOOL __stdcall CMFCToolBarDateTimeCtrl::SetTimeAll(UINT uiCmd, LPSYSTEMTIME pTimeNew /* = NULL */)
{
	CMFCToolBarDateTimeCtrl* pSrcDateTime = GetByCmd(uiCmd);

	if (pSrcDateTime)
	{
		pSrcDateTime->SetTime(pTimeNew);
	}

	return pSrcDateTime != NULL;
}

BOOL __stdcall CMFCToolBarDateTimeCtrl::SetTimeAll(UINT uiCmd, const COleDateTime& timeNew)
{
	CMFCToolBarDateTimeCtrl* pSrcDateTime = GetByCmd(uiCmd);

	if (pSrcDateTime)
	{
		pSrcDateTime->SetTime(timeNew);
	}

	return pSrcDateTime != NULL;
}

BOOL __stdcall CMFCToolBarDateTimeCtrl::SetTimeAll(UINT uiCmd, const CTime* pTimeNew)
{
	CMFCToolBarDateTimeCtrl* pSrcDateTime = GetByCmd(uiCmd);

	if (pSrcDateTime)
	{
		pSrcDateTime->SetTime(pTimeNew);
	}

	return pSrcDateTime != NULL;
}

DWORD __stdcall CMFCToolBarDateTimeCtrl::GetTimeAll(UINT uiCmd, LPSYSTEMTIME pTimeDest)
{
	CMFCToolBarDateTimeCtrl* pSrcDateTime = GetByCmd(uiCmd);

	if (pSrcDateTime)
	{
		return pSrcDateTime->GetTime(pTimeDest);
	}
	else
		return GDT_NONE;
}

BOOL __stdcall CMFCToolBarDateTimeCtrl::GetTimeAll(UINT uiCmd, COleDateTime& timeDest)
{
	CMFCToolBarDateTimeCtrl* pSrcDateTime = GetByCmd(uiCmd);

	if (pSrcDateTime)
	{
		return pSrcDateTime->GetTime(timeDest);
	}
	else
		return FALSE;
}

DWORD __stdcall CMFCToolBarDateTimeCtrl::GetTimeAll(UINT uiCmd, CTime& timeDest)
{
	CMFCToolBarDateTimeCtrl* pSrcDateTime = GetByCmd(uiCmd);

	if (pSrcDateTime)
	{
		return pSrcDateTime->GetTime(timeDest);
	}
	else
		return GDT_NONE;
}

void CMFCToolBarDateTimeCtrl::SetStyle(UINT nStyle)
{
	CMFCToolBarButton::SetStyle(nStyle);

	if (m_pWndDateTime != NULL && m_pWndDateTime->GetSafeHwnd() != NULL)
	{
		BOOL bDisabled = (CMFCToolBar::IsCustomizeMode() && !IsEditable()) || (!CMFCToolBar::IsCustomizeMode() &&(m_nStyle & TBBS_DISABLED));

		m_pWndDateTime->EnableWindow(!bDisabled);
	}
}

BOOL CMFCToolBarDateTimeCtrl::OnUpdateToolTip(CWnd* /*pWndParent*/, int /*iButtonIndex*/, CToolTipCtrl& wndToolTip, CString& strTipText)
{
	if (!m_bHorz)
	{
		return FALSE;
	}

	CString strTips;
	if (OnGetCustomToolTipText(strTips))
	{
		strTipText = strTips;
	}

	CDateTimeCtrl* pWndDate = GetDateTimeCtrl();
	if (pWndDate != NULL)
	{
		wndToolTip.AddTool(pWndDate, strTipText, NULL, 0);
		return TRUE;
	}

	return FALSE;
}

void CMFCToolBarDateTimeCtrl::OnGlobalFontsChanged()
{
	CMFCToolBarButton::OnGlobalFontsChanged();

	if (m_pWndDateTime->GetSafeHwnd() != NULL)
	{
		m_pWndDateTime->SetFont(&afxGlobalData.fontRegular);
	}
}


