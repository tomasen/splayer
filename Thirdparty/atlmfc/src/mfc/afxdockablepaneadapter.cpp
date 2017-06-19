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
#include "afxdockablepaneadapter.h"

#include "afxregpath.h"
#include "afxsettingsstore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const CString strControlBarProfile = _T("Panes");

#define AFX_REG_SECTION_FMT _T("%sDockablePaneAdapter-%d")
#define AFX_REG_SECTION_FMT_EX _T("%sDockablePaneAdapter-%d%x")

IMPLEMENT_SERIAL(CDockablePaneAdapter, CDockablePane, VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CDockablePaneAdapter

CDockablePaneAdapter::CDockablePaneAdapter()
{
	m_pWnd = NULL;
	m_dwEnabledAlignmentInitial = CBRS_ALIGN_ANY;
	m_rectInitial.SetRect(30, 30, 180, 180);
}

CDockablePaneAdapter::~CDockablePaneAdapter()
{
}

BEGIN_MESSAGE_MAP(CDockablePaneAdapter, CDockablePane)
	//{{AFX_MSG_MAP(CDockablePaneAdapter)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDockablePaneAdapter message handlers
void CDockablePaneAdapter::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	if (m_pWnd != NULL)
	{
		m_pWnd->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER/* | SWP_NOREDRAW*/);
	}
}
//------------------------------------------------------------------------------------
BOOL CDockablePaneAdapter::SetWrappedWnd(CWnd* pWnd)
{
	ASSERT_VALID(pWnd);
	ASSERT(IsWindow(m_hWnd));

	pWnd->SetParent(this);

	m_pWnd = pWnd;

	if (pWnd->IsKindOf(RUNTIME_CLASS(CBasePane)))
	{
		CBasePane* pBar = (CBasePane*) pWnd;
		EnableDocking(pBar->GetEnabledAlignment());
		m_bRecentVisibleState = pBar->GetRecentVisibleState();
		SetRestoredFromRegistry(pBar->IsRestoredFromRegistry());
		if (pWnd->IsKindOf(RUNTIME_CLASS(CPane)))
		{
			m_rectSavedDockedRect = ((CPane*) pBar)->m_rectSavedDockedRect;
		}
	}
	else
	{
		EnableDocking(m_dwEnabledAlignmentInitial);
	}

	return TRUE;
}
//------------------------------------------------------------------------------------
BOOL CDockablePaneAdapter::SaveState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		CString strName;
		GetWindowText(strName);
		reg.Write(_T("BarName"), strName);

	}
	return CDockablePane::SaveState(lpszProfileName, nIndex, uiID);
}
//------------------------------------------------------------------------------------
BOOL CDockablePaneAdapter::LoadState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	CString strName;
	reg.Read(_T("BarName"), strName);
	if (!strName.IsEmpty())
	{
		SetWindowText(strName);
	}

	return CDockablePane::LoadState(lpszProfileName, nIndex, uiID);
}


