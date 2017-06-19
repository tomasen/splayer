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
#include "afxribbonlinkctrl.h"
#include "afxribbonres.h"
#include "afxglobals.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static BOOL __stdcall ShellExecute(LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters = NULL, LPCTSTR lpDirectory = NULL, UINT nShow = SW_SHOWNORMAL)
{
	SHELLEXECUTEINFO sei;
	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

	sei.cbSize       = sizeof(SHELLEXECUTEINFO);
	sei.fMask        = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd         = NULL;
	sei.lpVerb       = lpOperation;
	sei.lpFile       = lpFile;
	sei.lpParameters = lpParameters;
	sei.lpDirectory  = lpDirectory;
	sei.nShow        = nShow;
	sei.hInstApp     = NULL;

	return ::ShellExecuteEx(&sei);
}

IMPLEMENT_DYNCREATE(CMFCRibbonLinkCtrl, CMFCRibbonButton)

// Construction/Destruction
CMFCRibbonLinkCtrl::CMFCRibbonLinkCtrl()
{
}

CMFCRibbonLinkCtrl::CMFCRibbonLinkCtrl(UINT nID, LPCTSTR lpszText, LPCTSTR lpszLink)
{
	ENSURE(lpszText != NULL);
	ENSURE(lpszLink != NULL);

	m_nID = nID;
	SetText(lpszText);
	SetLink(lpszLink);
}

CMFCRibbonLinkCtrl::~CMFCRibbonLinkCtrl()
{
}

// Overrides
CSize CMFCRibbonLinkCtrl::GetRegularSize(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CSize size(0, 0);

	OnSetIcon();

	if (m_bQuickAccessMode || m_bFloatyMode)
	{
		size = GetImageSize(RibbonImageSmall);
	}
	else
	{
		size = pDC->GetTextExtent(m_strText);
	}

	return CSize(size.cx + 2 * m_szMargin.cx, size.cy + 2 * m_szMargin.cy);
}

CSize CMFCRibbonLinkCtrl::GetCompactSize(CDC* pDC)
{
	ASSERT_VALID(this);
	return GetRegularSize(pDC);
}

void CMFCRibbonLinkCtrl::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	if (m_bQuickAccessMode || m_bFloatyMode)
	{
		OnSetIcon();
		CMFCRibbonButton::OnDraw(pDC);

		return;
	}

	// Set font:
	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontUnderline);
	ENSURE(pOldFont != NULL);

	COLORREF clrTextOld = pDC->SetTextColor(CMFCVisualManager::GetInstance()->GetRibbonHyperlinkTextColor(this));

	CRect rectText = m_rect;
	rectText.DeflateRect(m_szMargin.cx, m_szMargin.cy);

	DrawRibbonText(pDC, m_strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(clrTextOld);
}

void CMFCRibbonLinkCtrl::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonButton::CopyFrom(s);
	CMFCRibbonLinkCtrl& src = (CMFCRibbonLinkCtrl&) s;

	m_strLink = src.m_strLink;
}

BOOL CMFCRibbonLinkCtrl::OpenLink()
{
	ASSERT_VALID(this);

	if (m_strLink.IsEmpty())
	{
		return FALSE;
	}

	return ShellExecute(NULL, m_strLink);
}

void CMFCRibbonLinkCtrl::OnMouseMove(CPoint point)
{
	ASSERT_VALID(this);
	CMFCRibbonButton::OnMouseMove(point);
	::SetCursor(afxGlobalData.GetHandCursor());
}

void CMFCRibbonLinkCtrl::OnSetIcon()
{
	ASSERT_VALID(this);

	if (m_hIcon == NULL)
	{
		if (afxGlobalData.m_hiconLink == NULL)
		{
			LPCTSTR lpszResourceName = MAKEINTRESOURCE(IDI_AFXRES_COLORS);
			ENSURE(lpszResourceName != NULL);

			afxGlobalData.m_hiconLink = (HICON) ::LoadImage(
				AfxFindResourceHandle(lpszResourceName, RT_ICON),
				lpszResourceName, IMAGE_ICON, 16, 16, LR_SHARED);
		}

		m_hIcon = afxGlobalData.m_hiconLink;
	}
}

void CMFCRibbonLinkCtrl::SetLink(LPCTSTR lpszLink)
{
	m_strLink = lpszLink;
}


