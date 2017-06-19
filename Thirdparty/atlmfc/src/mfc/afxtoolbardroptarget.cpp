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

#include "afxtoolbarbutton.h"
#include "afxtoolbardroptarget.h"
#include "afxtoolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarDropTarget

CMFCToolBarDropTarget::CMFCToolBarDropTarget()
{
	m_pOwner = NULL;
}

CMFCToolBarDropTarget::~CMFCToolBarDropTarget()
{
}

BEGIN_MESSAGE_MAP(CMFCToolBarDropTarget, COleDropTarget)
END_MESSAGE_MAP()

BOOL CMFCToolBarDropTarget::Register(CMFCToolBar* pOwner)
{
	m_pOwner = pOwner;
	return COleDropTarget::Register(pOwner);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarDropTarget message handlers

DROPEFFECT CMFCToolBarDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	ENSURE(m_pOwner != NULL);

	if (!m_pOwner->IsCustomizeMode() || !pDataObject->IsDataAvailable(CMFCToolBarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner -> OnDragEnter(pDataObject, dwKeyState, point);
}

void CMFCToolBarDropTarget::OnDragLeave(CWnd* /*pWnd*/)
{
	ENSURE(m_pOwner != NULL);

	m_pOwner->OnDragLeave();
}

DROPEFFECT CMFCToolBarDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	ENSURE(m_pOwner != NULL);

	if (!m_pOwner->IsCustomizeMode() || !pDataObject->IsDataAvailable(CMFCToolBarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner -> OnDragOver(pDataObject, dwKeyState, point);
}

DROPEFFECT CMFCToolBarDropTarget::OnDropEx(CWnd* /*pWnd*/, COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT /*dropList*/, CPoint point)
{
	ENSURE(m_pOwner != NULL);

	if (!m_pOwner->IsCustomizeMode() || !pDataObject->IsDataAvailable(CMFCToolBarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner -> OnDrop(pDataObject, dropEffect, point) ? dropEffect : DROPEFFECT_NONE;
}


