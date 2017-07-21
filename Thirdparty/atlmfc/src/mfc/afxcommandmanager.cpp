// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.
//

#include "stdafx.h"
#include "afxcommandmanager.h"
#include "afxsettingsstore.h"
#include "afxregpath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_REG_PARAMS_FMT _T("%sCommandManager")
#define AFX_REG_ENTRY_COMMANDS_WITHOUT_IMAGES _T("CommandsWithoutImages")
#define AFX_REG_ENTRY_MENU_USER_IMAGES _T("MenuUserImages")

static const CString strToolbarProfile = _T("ToolBars");

//////////////////////////////////////////////////////////////////////
// One global static CCommandManager Object
//////////////////////////////////////////////////////////////////////
class _STATIC_CREATOR_
{
public:
	CCommandManager s_TheCmdMgr;
};

static _STATIC_CREATOR_ STATIC_CREATOR;

CCommandManager* GetCmdMgr()
{
	return &STATIC_CREATOR.s_TheCmdMgr;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

CCommandManager::CCommandManager()
{
}

CCommandManager::~CCommandManager()
{
}

//////////////////////////////////////////////////////////////////////
// ImageHash functions
//////////////////////////////////////////////////////////////////////

void CCommandManager::SetCmdImage(UINT uiCmd, int iImage, BOOL bUserImage)
{
	if (uiCmd == 0 || uiCmd == (UINT) -1)
	{
		return;
	}

	if (bUserImage)
	{
		// If command is already associated to the "standard" image list,
		// don't assign to to the "user" images
		if (GetCmdImage(uiCmd, FALSE) < 0)
		{
			m_CommandIndexUser.SetAt(uiCmd, iImage);
		}
	}
	else
	{
		if (GetCmdImage(uiCmd, TRUE) < 0)
		{
			m_CommandIndex.SetAt(uiCmd, iImage);
		}
	}
}

int CCommandManager::GetCmdImage(UINT uiCmd, BOOL bUserImage) const
{
	int iImage = -1;

	if (bUserImage)
	{
		if (!m_CommandIndexUser.Lookup(uiCmd, iImage))
		{
			return -1;
		}
	}
	else
	{
		if (!m_CommandIndex.Lookup(uiCmd, iImage))
		{
			return -1;
		}
	}

	return iImage;
}

void CCommandManager::ClearCmdImage(UINT uiCmd)
{
	m_CommandIndexUser.RemoveKey(uiCmd);
}

void CCommandManager::ClearUserCmdImages()
{
	m_CommandIndexUser.RemoveAll();
}

void CCommandManager::ClearAllCmdImages()
{
	m_CommandIndex.RemoveAll();
	m_CommandIndexUser.RemoveAll();
	m_lstCommandsWithoutImages.RemoveAll();
	m_mapMenuUserImages.RemoveAll();
}

void CCommandManager::CleanUp()
{
	ClearAllCmdImages();
}

void CCommandManager::EnableMenuItemImage(UINT uiCmd, BOOL bEnable, int iUserImage)
{
	POSITION pos = m_lstCommandsWithoutImages.Find(uiCmd);

	if (bEnable)
	{
		if (pos != NULL)
		{
			m_lstCommandsWithoutImages.RemoveAt(pos);
		}

		if (iUserImage >= 0)
		{
			m_mapMenuUserImages.SetAt(uiCmd, iUserImage);
		}
		else
		{
			m_mapMenuUserImages.RemoveKey(uiCmd);
		}
	}
	else
	{
		m_mapMenuUserImages.RemoveKey(uiCmd);

		if (pos == NULL)
		{
			m_lstCommandsWithoutImages.AddTail(uiCmd);
		}
	}
}

BOOL CCommandManager::LoadState(LPCTSTR lpszProfileName)
{
	CString strProfileName = ::AFXGetRegPath(strToolbarProfile, lpszProfileName);

	CString strSection;
	strSection.Format(AFX_REG_PARAMS_FMT, (LPCTSTR)strProfileName);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	m_lstCommandsWithoutImages.RemoveAll();

	return reg.Read(AFX_REG_ENTRY_COMMANDS_WITHOUT_IMAGES, m_lstCommandsWithoutImages) && reg.Read(AFX_REG_ENTRY_MENU_USER_IMAGES, m_mapMenuUserImages);
}

BOOL CCommandManager::SaveState(LPCTSTR lpszProfileName)
{
	CString strProfileName = ::AFXGetRegPath(strToolbarProfile, lpszProfileName);

	CString strSection;
	strSection.Format(AFX_REG_PARAMS_FMT, (LPCTSTR)strProfileName);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		return reg.Write(AFX_REG_ENTRY_COMMANDS_WITHOUT_IMAGES, m_lstCommandsWithoutImages) && reg.Write(AFX_REG_ENTRY_MENU_USER_IMAGES, m_mapMenuUserImages);
	}

	return FALSE;
}


