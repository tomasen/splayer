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

#include "afxtoolbardropsource.h"
#include "afxribbonres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarDropSource

CMFCToolBarDropSource::CMFCToolBarDropSource()
{
	m_bDeleteOnDrop = TRUE;
	m_bEscapePressed = FALSE;
	m_bDragStarted = FALSE;

	m_hcurDelete = NULL;
	m_hcurMove = NULL;
	m_hcurCopy = NULL;
}

CMFCToolBarDropSource::~CMFCToolBarDropSource()
{
	if (m_hcurDelete != NULL)
	{
		::DeleteObject(m_hcurDelete);
	}

	if (m_hcurMove != NULL)
	{
		::DeleteObject(m_hcurMove);
	}

	if (m_hcurCopy != NULL)
	{
		::DeleteObject(m_hcurCopy);
	}
}

BEGIN_MESSAGE_MAP(CMFCToolBarDropSource, COleDropSource)
	//{{AFX_MSG_MAP(CMFCToolBarDropSource)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarDropSource message handlers

SCODE CMFCToolBarDropSource::GiveFeedback(DROPEFFECT dropEffect)
{
	HCURSOR hcur = NULL;

	switch(dropEffect)
	{
	case DROPEFFECT_MOVE:
		hcur = m_hcurMove;
		break;

	case DROPEFFECT_COPY:
		hcur = m_hcurCopy;
		break;

	default:
		hcur = m_hcurDelete;
		break;
	}

	if (hcur == NULL)
	{
		return COleDropSource::GiveFeedback(dropEffect);
	}

	::SetCursor(hcur);
	return S_OK;
}

SCODE CMFCToolBarDropSource::QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState)
{
	if (m_bDeleteOnDrop && m_hcurDelete != NULL)
	{
		::SetCursor(m_hcurDelete);
	}

	m_bEscapePressed = bEscapePressed;
	return COleDropSource::QueryContinueDrag(bEscapePressed, dwKeyState);
}

BOOL CMFCToolBarDropSource::OnBeginDrag(CWnd* pWnd)
{
	if (m_hcurDelete == NULL)
	{
		m_hcurDelete = AfxGetApp()->LoadCursor(IDC_AFXBARRES_DELETE);
		m_hcurMove = AfxGetApp()->LoadCursor(IDC_AFXBARRES_MOVE);
		m_hcurCopy = AfxGetApp()->LoadCursor(IDC_AFXBARRES_COPY);
	}

	m_bDragStarted = TRUE;
	return COleDropSource::OnBeginDrag(pWnd);
}


