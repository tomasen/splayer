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
#include "afxmenutearoffmanager.h"
#include "afxwinappex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMenuTearOffManager* g_pTearOffMenuManager = NULL;

static const TCHAR cIDChar = 1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuTearOffManager::CMenuTearOffManager() : m_uiTearOffMenuFirst(0), m_uiTearOffMenuLast(0)
{
}

CMenuTearOffManager::~CMenuTearOffManager()
{
	g_pTearOffMenuManager = NULL;
}

BOOL CMenuTearOffManager::Initialize( LPCTSTR lpszRegEntry, UINT uiTearOffMenuFirst, UINT uiTearOffMenuLast)
{
	ENSURE(g_pTearOffMenuManager != NULL);
	ASSERT(uiTearOffMenuLast >= uiTearOffMenuFirst);

	if (uiTearOffMenuFirst == 0 || uiTearOffMenuLast == 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	g_pTearOffMenuManager = this;

	m_uiTearOffMenuFirst = uiTearOffMenuFirst;
	m_uiTearOffMenuLast = uiTearOffMenuLast;

	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());

	m_strTearOffBarRegEntry = (lpszRegEntry == NULL) ? ( pApp ? pApp->GetRegSectionPath() : _T("")) : lpszRegEntry;

	int nCount = uiTearOffMenuLast - uiTearOffMenuFirst + 1;
	m_arTearOffIDsUsage.SetSize(nCount);

	for (int i = 0; i < nCount; i ++)
	{
		m_arTearOffIDsUsage [i] = 0;
	}

	return TRUE;
}

void CMenuTearOffManager::Reset(HMENU hMenu)
{
	int nCount = m_uiTearOffMenuLast - m_uiTearOffMenuFirst + 1;

	if (hMenu == NULL) // Reset all
	{
		for (int i = 0; i < nCount; i ++)
		{
			m_arTearOffIDsUsage [i] = 0;
		}

		return;
	}

	CMenu* pMenu = CMenu::FromHandle(hMenu);
	if (pMenu == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	int iCount = (int) pMenu->GetMenuItemCount();
	for (int i = 0; i < iCount; i ++)
	{
		CString str;
		pMenu->GetMenuString(i, str, MF_BYPOSITION);

		UINT uiTearOffID = Parse(str);
		if (uiTearOffID >= m_uiTearOffMenuFirst && uiTearOffID <= m_uiTearOffMenuLast)
		{
			m_arTearOffIDsUsage [uiTearOffID - m_uiTearOffMenuFirst] = 0;
		}

		if (pMenu->GetMenuItemID(i) == (UINT)-1)
		{
			CMenu* pPopupMenu = pMenu->GetSubMenu(i);
			ENSURE(pPopupMenu != NULL);

			Reset(pPopupMenu->GetSafeHmenu());
		}
	}
}

UINT CMenuTearOffManager::GetFreeTearOffID()
{
	if (m_uiTearOffMenuFirst == 0 || m_uiTearOffMenuLast == 0)
	{
		ASSERT(FALSE);
		return 0;
	}

	int nCount = m_uiTearOffMenuLast - m_uiTearOffMenuFirst + 1;
	for (int i = 0; i < nCount; i ++)
	{
		if (m_arTearOffIDsUsage [i] == 0)
		{
			m_arTearOffIDsUsage [i] = 1;
			return m_uiTearOffMenuFirst + i;
		}
	}

	return 0;
}

void CMenuTearOffManager::SetupTearOffMenus(HMENU hMenu)
{
	ENSURE(hMenu != NULL);

	CMenu* pMenu = CMenu::FromHandle(hMenu);
	if (pMenu == NULL)
	{
		return;
	}

	int iCount = (int) pMenu->GetMenuItemCount();
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiID = pMenu->GetMenuItemID(i);
		if (uiID != (UINT) -1)
		{
			continue;
		}

		UINT uiState = pMenu->GetMenuState(i, MF_BYPOSITION);
		if (uiState & MF_MENUBARBREAK)
		{
			CString str;
			pMenu->GetMenuString(i, str, MF_BYPOSITION);

			if (str [0] != cIDChar)
			{
				UINT uiCtrlBarId = GetFreeTearOffID();
				if (uiCtrlBarId == 0) // No more free IDs!
				{ // Reserve more IDs in Initialize!!!
					ASSERT(FALSE);
					return;
				}

				Build(uiCtrlBarId, str);
				pMenu->ModifyMenu(i, MF_BYPOSITION, i, str);
			}
		}

		CMenu* pPopupMenu = pMenu->GetSubMenu(i);
		if (pPopupMenu != NULL)
		{
			SetupTearOffMenus(pPopupMenu->GetSafeHmenu());
		}
	}
}

void CMenuTearOffManager::SetInUse(UINT uiCmdId, BOOL bUse/* = TRUE*/)
{
	if (uiCmdId < m_uiTearOffMenuFirst || uiCmdId > m_uiTearOffMenuLast)
	{
		return;
	}

	int nDelta = bUse ? 1 : -1;
	int iIndex = uiCmdId - m_uiTearOffMenuFirst;

	m_arTearOffIDsUsage [iIndex] += nDelta;

	if (m_arTearOffIDsUsage [iIndex] < 0)
	{
		m_arTearOffIDsUsage [iIndex] = 0;
	}
}

UINT CMenuTearOffManager::Parse(CString& str)
{
	if (str.IsEmpty() || str [0] != cIDChar)
	{
		return 0;
	}

	UINT uiID = _ttol(str.Mid(1));
	ASSERT(uiID != 0);

	int iOffset = str.ReverseFind(cIDChar);
	if (iOffset == -1)
	{
		ASSERT(FALSE);
		return 0;
	}

	str = str.Mid(iOffset + 1);
	return uiID;
}

void CMenuTearOffManager::Build(UINT uiTearOffBarID, CString& strText)
{
	ASSERT(uiTearOffBarID != 0);

	CString strNew;
	strNew.Format(_T("%c%d%c%s"), cIDChar, uiTearOffBarID, cIDChar, (LPCTSTR)strText);
	strText = strNew;
}


