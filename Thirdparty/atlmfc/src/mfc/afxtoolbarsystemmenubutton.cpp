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
#include <afxpriv.h>
#include "afxtoolbar.h"
#include "afxtoolbarsystemmenubutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMFCToolBarSystemMenuButton, CMFCToolBarMenuButton, VERSIONABLE_SCHEMA | 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCToolBarSystemMenuButton::CMFCToolBarSystemMenuButton()
{
	m_hSysMenuIcon = NULL;
	m_hSystemMenu = NULL;
}

CMFCToolBarSystemMenuButton::CMFCToolBarSystemMenuButton(HMENU hSystemMenu, HICON hSystemIcon) :
	CMFCToolBarMenuButton(0, hSystemMenu, -1)
{
	m_hSysMenuIcon = hSystemIcon;
	m_hSystemMenu = hSystemMenu;
}

CMFCToolBarSystemMenuButton::~CMFCToolBarSystemMenuButton()
{
}

void CMFCToolBarSystemMenuButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarMenuButton::CopyFrom(s);

	const CMFCToolBarSystemMenuButton& src = (const CMFCToolBarSystemMenuButton&) s;

	m_hSysMenuIcon = src.m_hSysMenuIcon;
	m_hSystemMenu = src.m_hSystemMenu;
}

SIZE CMFCToolBarSystemMenuButton::OnCalculateSize(CDC* /*pDC*/, const CSize& sizeDefault, BOOL /*bHorz*/)
{
	return CSize(::GetSystemMetrics(SM_CXMENUSIZE), sizeDefault.cy);
}

void CMFCToolBarSystemMenuButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* /*pImages*/,
	BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL /*bHighlight*/, BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	if (m_hSysMenuIcon != NULL)
	{

		CSize size(min(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CXMENUSIZE)), min(::GetSystemMetrics(SM_CYSMICON), ::GetSystemMetrics(SM_CYMENUSIZE)));

		int iOffset = (rect.Height() - size.cy) / 2;
		::DrawIconEx(*pDC, rect.left, rect.top + iOffset, m_hSysMenuIcon, size.cx, size.cy, 0, NULL, DI_NORMAL);
	}
}

void CMFCToolBarSystemMenuButton::OnDblClk(CWnd* pWnd)
{
	if (CMFCToolBar::IsCustomizeMode())
	{
		return;
	}

	ENSURE(pWnd != NULL);

	//////////////////////////////////////////////
	// Make sure to close the popup menu and
	// find the MDI frame correctly.
	//--------------------------------------------
	OnCancelMode();

	CFrameWnd* pParentFrame = AFXGetParentFrame(pWnd);
	if (pParentFrame != NULL && pParentFrame->IsKindOf(RUNTIME_CLASS(CMiniDockFrameWnd)))
	{
		pParentFrame = (CFrameWnd*) pParentFrame->GetParent();
	}

	CMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWnd, pParentFrame);
	if (pMDIFrame != NULL)
	{
		CMDIChildWnd* pChild = pMDIFrame->MDIGetActive();
		ASSERT_VALID(pChild);

		BOOL bCloseIsDisabled = FALSE;

		CMenu* pSysMenu = pChild->GetSystemMenu(FALSE);
		if (pSysMenu != NULL)
		{
			MENUITEMINFO menuInfo;
			ZeroMemory(&menuInfo,sizeof(MENUITEMINFO));
			menuInfo.cbSize = sizeof(MENUITEMINFO);
			menuInfo.fMask = MIIM_STATE;

			pSysMenu->GetMenuItemInfo(SC_CLOSE, &menuInfo);
			bCloseIsDisabled = (menuInfo.fState & MFS_DISABLED);
		}

		if (!bCloseIsDisabled)
		{
			pChild->SendMessage(WM_SYSCOMMAND, SC_CLOSE);
		}
	}
	//--------------------------------------------
	//////////////////////////////////////////////
}

void CMFCToolBarSystemMenuButton::CreateFromMenu(HMENU hMenu)
{
	m_hSystemMenu = hMenu;
}

HMENU CMFCToolBarSystemMenuButton::CreateMenu() const
{
	ENSURE(m_hSystemMenu != NULL);

	HMENU hMenu = CMFCToolBarMenuButton::CreateMenu();
	if (hMenu == NULL)
	{
		return NULL;
	}

	//---------------------------------------------------------------------
	// System menu don't produce updating command statuses via the
	// standard MFC idle command targeting. So, we should enable/disable
	// system menu items according to the standard system menu status:
	//---------------------------------------------------------------------
	CMenu* pMenu = CMenu::FromHandle(hMenu);
	ASSERT_VALID(pMenu);

	CMenu* pSysMenu = CMenu::FromHandle(m_hSystemMenu);
	ASSERT_VALID(pSysMenu);

	int iCount = (int) pSysMenu->GetMenuItemCount();
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiState = pSysMenu->GetMenuState(i, MF_BYPOSITION);
		UINT uiCmd = pSysMenu->GetMenuItemID(i);

		if (uiState & MF_CHECKED)
		{
			pMenu->CheckMenuItem(uiCmd, MF_CHECKED);
		}

		if (uiState & MF_DISABLED)
		{
			pMenu->EnableMenuItem(uiCmd, MF_DISABLED);
		}

		if (uiState & MF_GRAYED)
		{
			pMenu->EnableMenuItem(uiCmd, MF_GRAYED);
		}
	}

	return hMenu;
}

void CMFCToolBarSystemMenuButton::OnCancelMode()
{
	if (m_pPopupMenu != NULL && ::IsWindow(m_pPopupMenu->m_hWnd))
	{
		if (m_pPopupMenu->InCommand())
		{
			return;
		}

		m_pPopupMenu->SaveState();
		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->CloseMenu();
	}

	m_pPopupMenu = NULL;
	m_bToBeClosed = FALSE;
}

void CMFCToolBarSystemMenuButton::OnAfterCreatePopupMenu()
{
	if (m_pPopupMenu != NULL && ::IsWindow(m_pPopupMenu->m_hWnd))
	{
		CFrameWnd* pParentFrame = AFXGetTopLevelFrame(m_pPopupMenu);
		if (pParentFrame != NULL && pParentFrame->IsKindOf(RUNTIME_CLASS(CMiniDockFrameWnd)))
		{
			pParentFrame = (CFrameWnd*) pParentFrame->GetParent();
		}

		CMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWnd, pParentFrame);

		if (pMDIFrame != NULL)
		{
			CMDIChildWnd* pChild = pMDIFrame->MDIGetActive();
			ASSERT_VALID(pChild);

			m_pPopupMenu->SetMessageWnd(pChild);
		}
	}
}



