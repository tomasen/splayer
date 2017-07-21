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

#include "afxdocksite.h"
#include "afxbasetoolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCBaseToolBar,CPane)

/////////////////////////////////////////////////////////////////////////////
// CMFCBaseToolBar

CMFCBaseToolBar::CMFCBaseToolBar()
{
}

CMFCBaseToolBar::~CMFCBaseToolBar()
{
}

BEGIN_MESSAGE_MAP(CMFCBaseToolBar, CPane)
	//{{AFX_MSG_MAP(CMFCBaseToolBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCBaseToolBar message handlers

void CMFCBaseToolBar::OnAfterChangeParent(CWnd* /*pWndOldParent*/)
{
}

void CMFCBaseToolBar::OnAfterStretch(int /*nStretchSize*/)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	if (rectClient.Width() != m_rectVirtual.Width())
	{
		Invalidate();
		UpdateWindow();
	}
}


