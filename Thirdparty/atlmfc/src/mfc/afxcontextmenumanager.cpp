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
#include "afxcontextmenumanager.h"
#include "afxmenutearoffmanager.h"
#include "afxpopupmenu.h"
#include "afxmenuhash.h"
#include "afxglobals.h"
#include "afxregpath.h"
#include "afxdialogex.h"
#include "afxpropertypage.h"
#include "afxwinappex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const CString strMenusProfile = _T("ContextMenuManager");

CContextMenuManager* afxContextMenuManager = NULL;

// Construction/Destruction
CContextMenuManager::CContextMenuManager()
{
	ENSURE(afxContextMenuManager == NULL);
	afxContextMenuManager = this;
	m_nLastCommandID = 0;
	m_bTrackMode = FALSE;
	m_bDontCloseActiveMenu = FALSE;
}

CContextMenuManager::~CContextMenuManager()
{
	POSITION pos = NULL;

	for (pos = m_Menus.GetStartPosition(); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc(pos, uiResId, hMenu);
		::DestroyMenu(hMenu);
	}

	for (pos = m_MenuOriginalItems.GetStartPosition(); pos != NULL;)
	{
		UINT uiResId;
		CObList* pLstOrginItems = NULL;

		m_MenuOriginalItems.GetNextAssoc(pos, uiResId, pLstOrginItems);
		ASSERT_VALID(pLstOrginItems);

		while (!pLstOrginItems->IsEmpty())
		{
			delete pLstOrginItems->RemoveHead();
		}

		delete pLstOrginItems;
	}

	afxContextMenuManager = NULL;
}

BOOL CContextMenuManager::AddMenu(UINT uiMenuNameResId, UINT uiMenuResId)
{
	CString strMenuName;
	ENSURE(strMenuName.LoadString(uiMenuNameResId));

	return AddMenu(strMenuName, uiMenuResId);
}

BOOL CContextMenuManager::AddMenu(LPCTSTR lpszName, UINT uiMenuResId)
{
	ENSURE(lpszName != NULL);

	CMenu menu;
	if (!menu.LoadMenu(uiMenuResId))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	HMENU hExMenu;
	if (m_Menus.Lookup(uiMenuResId, hExMenu))
	{
		// Menu with the same name is already exist!
		return FALSE;
	}

	HMENU hMenu = menu.Detach();

	if (g_pTearOffMenuManager != NULL)
	{
		g_pTearOffMenuManager->SetupTearOffMenus(hMenu);
	}

	m_Menus.SetAt(uiMenuResId, hMenu);
	m_MenuNames.SetAt(lpszName, hMenu);

	return TRUE;
}

BOOL CContextMenuManager::ShowPopupMenu(UINT uiMenuResId, int x, int y, CWnd* pWndOwner, BOOL bOwnMessage, BOOL bRightAlign)
{
	HMENU hMenu;
	if (!m_Menus.Lookup(uiMenuResId, hMenu) || hMenu == NULL)
	{
		return FALSE;
	}

	if (x == -1 && y == -1 && // Undefined position
		pWndOwner != NULL)
	{
		CRect rectParent;
		pWndOwner->GetClientRect(&rectParent);
		pWndOwner->ClientToScreen(&rectParent);

		x = rectParent.left + 5;
		y = rectParent.top + 5;
	}

	HMENU hmenuPopup = ::GetSubMenu(hMenu, 0);
	if (hmenuPopup == NULL)
	{
#ifdef _DEBUG

		MENUITEMINFO info;
		memset(&info, 0, sizeof(MENUITEMINFO));

		if (!::GetMenuItemInfo(hMenu, 0, TRUE, &info))
		{
			TRACE(_T("Invalid menu: %d\n"), uiMenuResId);
		}
		else
		{
			ASSERT(info.hSubMenu == NULL);
			TRACE(_T("Menu %d, first option '%s' doesn't contain popup menu!\n"), uiMenuResId, info.dwTypeData);
		}

#endif // _DEBUG
		return FALSE;
	}

	return ShowPopupMenu(hmenuPopup, x, y, pWndOwner, bOwnMessage, TRUE, bRightAlign) != NULL;
}

CMFCPopupMenu* CContextMenuManager::ShowPopupMenu(HMENU hmenuPopup, int x, int y, CWnd* pWndOwner, BOOL bOwnMessage, BOOL /*bAutoDestroy*/, BOOL bRightAlign)
{
	if (pWndOwner != NULL && pWndOwner->IsKindOf(RUNTIME_CLASS(CDialogEx)) && !bOwnMessage)
	{
		// CDialogEx should own menu messages
		ASSERT(FALSE);
		return NULL;
	}

	if (pWndOwner != NULL && pWndOwner->IsKindOf(RUNTIME_CLASS(CMFCPropertyPage)) && !bOwnMessage)
	{
		// CMFCPropertyPage should own menu messages
		ASSERT(FALSE);
		return NULL;
	}

	ENSURE(hmenuPopup != NULL);
	if (g_pTearOffMenuManager != NULL)
	{
		g_pTearOffMenuManager->SetupTearOffMenus(hmenuPopup);
	}

	if (m_bTrackMode)
	{
		bOwnMessage = TRUE;
	}

	if (!bOwnMessage)
	{
		while (pWndOwner != NULL && pWndOwner->GetStyle() & WS_CHILD)
		{
			pWndOwner = pWndOwner->GetParent();
		}
	}

	CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;
	pPopupMenu->SetAutoDestroy(FALSE);

	pPopupMenu->m_bTrackMode = m_bTrackMode;
	pPopupMenu->SetRightAlign(bRightAlign);

	CMFCPopupMenu* pMenuActive = CMFCPopupMenu::GetActiveMenu();
	if (!m_bDontCloseActiveMenu && pMenuActive != NULL)
	{
		pMenuActive->SendMessage(WM_CLOSE);
	}

	if (!pPopupMenu->Create(pWndOwner, x, y, hmenuPopup, FALSE, bOwnMessage))
	{
		return NULL;
	}

	return pPopupMenu;
}

UINT CContextMenuManager::TrackPopupMenu(HMENU hmenuPopup, int x, int y, CWnd* pWndOwner, BOOL bRightAlign)
{
	m_nLastCommandID = 0;

	CWinThread* pCurrThread = ::AfxGetThread();
	if (pCurrThread == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	m_bTrackMode = TRUE;

	CMFCPopupMenu* pMenu = ShowPopupMenu(hmenuPopup, x, y, pWndOwner, FALSE, TRUE, bRightAlign);

	if (pMenu != NULL)
	{
		CRect rect;
		pMenu->GetWindowRect(&rect);
		pMenu->UpdateShadow(&rect);
	}

	CDialogEx* pParentDlg = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent() != NULL)
	{
		pParentDlg = DYNAMIC_DOWNCAST(CDialogEx, pWndOwner->GetParent());
		if (pParentDlg != NULL)
		{
			pParentDlg->SetActiveMenu(pMenu);
		}
	}

	CMFCPropertyPage* pParentPropPage = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent() != NULL)
	{
		pParentPropPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, pWndOwner->GetParent());
		if (pParentPropPage != NULL)
		{
			pParentPropPage->SetActiveMenu(pMenu);
		}
	}

	m_bTrackMode = FALSE;

	if (pMenu != NULL && pCurrThread != NULL)
	{
		ASSERT_VALID(pMenu);

		CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
		if (pApp == NULL || !pApp->OnWorkspaceIdle(pMenu))
		{
			LONG lIdleCount = 0;
			HWND hwndMenu = pMenu->GetSafeHwnd();

			while (::IsWindow(hwndMenu))
			{
				MSG msg;
				while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						PostThreadMessage(GetCurrentThreadId(), msg.message, msg.wParam, msg.lParam);
						return 0;
					}

					if (!::IsWindow(hwndMenu))
					{
						break;
					}

					switch (msg.message)
					{
					case WM_NCLBUTTONDOWN:
						pMenu->DestroyWindow();

						PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);

						if (pParentDlg != NULL)
						{
							pParentDlg->SetActiveMenu(NULL);
						}

						if (pParentPropPage != NULL)
						{
							pParentPropPage->SetActiveMenu(NULL);
						}

						return 0;
					}

					if (::IsWindow(hwndMenu) && !pCurrThread->PreTranslateMessage(&msg))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}

					if (::IsWindow(hwndMenu) && pMenu->IsIdle())
					{
						pCurrThread->OnIdle(lIdleCount++);
					}
				}

				// reset "no idle" state after pumping "normal" message
				if (pCurrThread->IsIdleMessage(&msg))
				{
					lIdleCount = 0;
				}

				if (!::IsWindow(hwndMenu))
				{
					break;
				}

				WaitMessage();
			}
		}
	}

	if (pParentDlg != NULL)
	{
		pParentDlg->SetActiveMenu(NULL);
	}

	if (pParentPropPage != NULL)
	{
		pParentPropPage->SetActiveMenu(NULL);
	}

	return m_nLastCommandID;
}

void CContextMenuManager::GetMenuNames(CStringList& listOfNames) const
{
	listOfNames.RemoveAll();

	for (POSITION pos = m_MenuNames.GetStartPosition(); pos != NULL;)
	{
		CString strName;
		HMENU hMenu;

		m_MenuNames.GetNextAssoc(pos, strName, hMenu);
		listOfNames.AddTail(strName);
	}
}

HMENU CContextMenuManager::GetMenuByName(LPCTSTR lpszName, UINT* puiOrigResID) const
{
	HMENU hMenu;
	if (!m_MenuNames.Lookup(lpszName, hMenu))
	{
		return NULL;
	}

	if (puiOrigResID != NULL)
	{
		*puiOrigResID = 0;

		for (POSITION pos = m_Menus.GetStartPosition(); pos != NULL;)
		{
			UINT uiResId;
			HMENU hMenuMap;

			m_Menus.GetNextAssoc(pos, uiResId, hMenuMap);
			if (hMenuMap == hMenu)
			{
				*puiOrigResID = uiResId;
				break;
			}
		}
	}

	return hMenu;
}

BOOL CContextMenuManager::LoadState(LPCTSTR lpszProfileName)
{
	CString strProfileName = ::AFXGetRegPath(strMenusProfile, lpszProfileName);

	for (POSITION pos = m_Menus.GetStartPosition(); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc(pos, uiResId, hMenu);
		ENSURE(hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu(hMenu, 0);
		ENSURE(hPopupMenu != NULL);

		CMFCPopupMenuBar* pBar = new CMFCPopupMenuBar;

		CWnd* pParentWnd = AfxGetMainWnd();
		if (pParentWnd == NULL)
		{
			pParentWnd = CWnd::FromHandle(GetDesktopWindow());
		}

		if (pBar->Create(pParentWnd, AFX_DEFAULT_TOOLBAR_STYLE, 0xFFFF))
		{
			if (!pBar->ImportFromMenu(hPopupMenu))
			{
				pBar->DestroyWindow();
				delete pBar;
				return FALSE;
			}

			pBar->BuildOrigItems(uiResId);

			if (pBar->LoadState(strProfileName, 0, uiResId) && !pBar->IsResourceChanged())
			{
				afxMenuHash.SaveMenuBar(hPopupMenu, pBar);
			}

			CopyOriginalMenuItemsFromMenu(uiResId, *pBar);
			pBar->DestroyWindow();
		}

		delete pBar;
	}

	return TRUE;
}

BOOL CContextMenuManager::SaveState(LPCTSTR lpszProfileName)
{
	CString strProfileName = ::AFXGetRegPath(strMenusProfile, lpszProfileName);

	for (POSITION pos = m_Menus.GetStartPosition(); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc(pos, uiResId, hMenu);
		ENSURE(hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu(hMenu, 0);
		ENSURE(hPopupMenu != NULL);

		CMFCPopupMenuBar* pBar = new CMFCPopupMenuBar;
		if (pBar->Create(CWnd::FromHandle(GetDesktopWindow())))
		{
			if (afxMenuHash.LoadMenuBar(hPopupMenu, pBar))
			{
				CopyOriginalMenuItemsToMenu(uiResId, *pBar);

				if (!pBar->SaveState(strProfileName, 0, uiResId))
				{
					pBar->DestroyWindow();
					delete pBar;
					return FALSE;
				}
			}

			pBar->DestroyWindow();
		}
		delete pBar;
	}

	return TRUE;
}

BOOL CContextMenuManager::ResetState()
{
	POSITION pos = NULL;

	for (pos = m_Menus.GetStartPosition(); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc(pos, uiResId, hMenu);
		ENSURE(hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu(hMenu, 0);
		ENSURE(hPopupMenu != NULL);

		afxMenuHash.RemoveMenu(hPopupMenu);
	}

	for (pos = m_MenuOriginalItems.GetStartPosition(); pos != NULL;)
	{
		UINT uiResId;
		CObList* pLstOrginItems = NULL;

		m_MenuOriginalItems.GetNextAssoc(pos, uiResId, pLstOrginItems);
		ASSERT_VALID(pLstOrginItems);

		while (!pLstOrginItems->IsEmpty())
		{
			delete pLstOrginItems->RemoveHead();
		}

		delete pLstOrginItems;
	}

	m_MenuOriginalItems.RemoveAll();

	return TRUE;
}

HMENU CContextMenuManager::GetMenuById(UINT nMenuResId) const
{
	HMENU hMenu = NULL ;
	return m_Menus.Lookup(nMenuResId, hMenu) ? hMenu : NULL;
}

void CContextMenuManager::CopyOriginalMenuItemsToMenu(UINT uiResId, CMFCPopupMenuBar& menuBar)
{
	CObList* pLstOrginItems = NULL;

	if (!m_MenuOriginalItems.Lookup(uiResId, pLstOrginItems))
	{
		return;
	}

	ASSERT_VALID(pLstOrginItems);

	if (pLstOrginItems->IsEmpty())
	{
		return;
	}

	CObList lstMenuItems;

	for (POSITION pos = pLstOrginItems->GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pSrcButton = DYNAMIC_DOWNCAST(CMFCToolBarButton, pLstOrginItems->GetNext(pos));
		ASSERT_VALID(pSrcButton);

		CRuntimeClass* pClass = pSrcButton->GetRuntimeClass();
		ENSURE(pClass != NULL);

		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pClass->CreateObject();
		ASSERT_VALID(pButton);

		pButton->CopyFrom(*pSrcButton);
		lstMenuItems.AddTail(pButton);
	}

	menuBar.SetOrigButtons(lstMenuItems);
}

void CContextMenuManager::CopyOriginalMenuItemsFromMenu(UINT uiResId, CMFCPopupMenuBar& menuBar)
{
	const CObList& lstMenuItems = menuBar.GetOrigButtons();

	CObList* pLstOrginItems = NULL;

	if (m_MenuOriginalItems.Lookup(uiResId, pLstOrginItems))
	{
		ASSERT_VALID(pLstOrginItems);

		while (!pLstOrginItems->IsEmpty())
		{
			delete pLstOrginItems->RemoveHead();
		}

		if (lstMenuItems.IsEmpty())
		{
			m_MenuOriginalItems.RemoveKey(uiResId);
			delete pLstOrginItems;
			return;
		}
	}
	else
	{
		if (lstMenuItems.IsEmpty())
		{
			return;
		}

		pLstOrginItems = new CObList;
		m_MenuOriginalItems.SetAt(uiResId, pLstOrginItems);
	}

	ASSERT_VALID(pLstOrginItems);

	for (POSITION pos = lstMenuItems.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pSrcButton = DYNAMIC_DOWNCAST(CMFCToolBarButton, lstMenuItems.GetNext(pos));
		ASSERT_VALID(pSrcButton);

		CRuntimeClass* pClass = pSrcButton->GetRuntimeClass();
		ENSURE(pClass != NULL);

		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pClass->CreateObject();
		ASSERT_VALID(pButton);

		pButton->CopyFrom(*pSrcButton);
		pLstOrginItems->AddTail(pButton);
	}
}


