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
#include "afxoutlookbar.h"
#include "afxoutlookbarpaneadapter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMFCOutlookBarPaneAdapter, CDockablePaneAdapter, VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarPaneAdapter

CMFCOutlookBarPaneAdapter::CMFCOutlookBarPaneAdapter()
{
	m_pTabbedControlBarRTC = RUNTIME_CLASS(CMFCOutlookBar);
}

CMFCOutlookBarPaneAdapter::~CMFCOutlookBarPaneAdapter()
{
}

BEGIN_MESSAGE_MAP(CMFCOutlookBarPaneAdapter, CDockablePaneAdapter)
	//{{AFX_MSG_MAP(CMFCOutlookBarPaneAdapter)
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarPaneAdapter message handlers

void CMFCOutlookBarPaneAdapter::OnNcDestroy()
{
	CDockablePaneAdapter::OnNcDestroy();
	delete this;
}


