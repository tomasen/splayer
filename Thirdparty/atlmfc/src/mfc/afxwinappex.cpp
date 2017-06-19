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
#include "afxglobals.h"
#include "afxcontrolbarutil.h"
#include "afxtoolbar.h"
#include "afxwinappex.h"

#include "afxframeimpl.h"
#include "afxmdiframewndex.h"
#include "afxframewndex.h"
#include "afxoleipframewndex.h"

#include "afxmousemanager.h"
#include "afxcontextmenumanager.h"
#include "afxkeyboardmanager.h"
#include "afxusertoolsmanager.h"
#include "afxmenutearoffmanager.h"
#include "afxshellmanager.h"
#include "afxtooltipmanager.h"

#include "afxsettingsstore.h"
#include "afxregpath.h"
#include "afxrebarstate.h"

#ifndef _MFC_USER_BUILD
#include "version.h"
#endif

IMPLEMENT_DYNAMIC(CWinAppEx, CWinApp)

static const CString strRegEntryNameControlBars = _T("\\ControlBars");
static const CString strWindowPlacementRegSection = _T("WindowPlacement");
static const CString strRectMainKey = _T("MainWindowRect");
static const CString strFlagsKey = _T("Flags");
static const CString strShowCmdKey = _T("ShowCmd");
static const CString strRegEntryNameSizingBars = _T("\\SizingBars");
static const CString strRegEntryVersion = _T("ControlBarVersion");
static const CString strVersionMajorKey = _T("Major");
static const CString strVersionMinorKey = _T("Minor");

extern CObList afxAllToolBars;

CWinAppEx::CWinAppEx(BOOL bResourceSmartUpdate/* = TRUE*/) :
	m_bResourceSmartUpdate(bResourceSmartUpdate)
{
	m_bKeyboardManagerAutocreated = FALSE;
	m_bContextMenuManagerAutocreated = FALSE;
	m_bMouseManagerAutocreated = FALSE;
	m_bUserToolsManagerAutoCreated = FALSE;
	m_bTearOffManagerAutoCreated = FALSE;
	m_bShellManagerAutocreated = FALSE;
	m_bTooltipManagerAutocreated = FALSE;

	const CString strRegEntryNameWorkspace = _T("Workspace");
	m_strRegSection = strRegEntryNameWorkspace;

	m_iSavedVersionMajor = -1;
	m_iSavedVersionMinor = -1;

	m_bForceDockStateLoad = FALSE;
	m_bLoadSaveFrameBarsOnly = FALSE;

	m_bSaveState = TRUE;
	m_bForceImageReset = FALSE;

	m_bLoadUserToolbars = TRUE;

	m_bLoadWindowPlacement = TRUE;
}

int CWinAppEx::ExitInstance() 
{
	ControlBarCleanUp();
	return CWinApp::ExitInstance();
}

CWinAppEx::~CWinAppEx()
{
	// Delete autocreated managers
	if (m_bKeyboardManagerAutocreated && afxKeyboardManager != NULL)
	{
		delete afxKeyboardManager;
		afxKeyboardManager = NULL;
	}

	if (m_bContextMenuManagerAutocreated && afxContextMenuManager != NULL)
	{
		delete afxContextMenuManager;
		afxContextMenuManager = NULL;
	}

	if (m_bMouseManagerAutocreated && afxMouseManager != NULL)
	{
		delete afxMouseManager;
		afxMouseManager = NULL;
	}

	if (m_bUserToolsManagerAutoCreated && afxUserToolsManager != NULL)
	{
		delete afxUserToolsManager;
		afxUserToolsManager = NULL;
	}

	if (m_bTearOffManagerAutoCreated && g_pTearOffMenuManager != NULL)
	{
		delete g_pTearOffMenuManager;
		g_pTearOffMenuManager = NULL;
	}

	if (m_bShellManagerAutocreated && afxShellManager != NULL)
	{
		delete afxShellManager;
		afxShellManager = NULL;
	}

	if (m_bTooltipManagerAutocreated && afxTooltipManager != NULL)
	{
		delete afxTooltipManager;
		afxTooltipManager = NULL;
	}
}

LPCTSTR CWinAppEx::SetRegistryBase(LPCTSTR lpszSectionName /*= NULL*/)
{
	m_strRegSection = (lpszSectionName != NULL) ? lpszSectionName : _T("");
	return m_strRegSection;
}

BOOL CWinAppEx::InitShellManager()
{
	if (afxShellManager != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	afxShellManager = new CShellManager;
	m_bShellManagerAutocreated = TRUE;
	return TRUE;
}

BOOL CWinAppEx::InitTooltipManager()
{
	if (afxTooltipManager != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	afxTooltipManager = new CTooltipManager;
	m_bTooltipManagerAutocreated = TRUE;
	return TRUE;
}

BOOL CWinAppEx::InitMouseManager()
{
	if (afxMouseManager != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	afxMouseManager = new CMouseManager;
	m_bMouseManagerAutocreated = TRUE;
	return TRUE;
}

BOOL CWinAppEx::InitContextMenuManager()
{
	if (afxContextMenuManager != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	afxContextMenuManager = new CContextMenuManager;
	m_bContextMenuManagerAutocreated = TRUE;

	return TRUE;
}

BOOL CWinAppEx::InitKeyboardManager()
{
	if (afxKeyboardManager != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	afxKeyboardManager = new CKeyboardManager;
	m_bKeyboardManagerAutocreated = TRUE;

	return TRUE;
}

BOOL CWinAppEx::EnableUserTools(const UINT uiCmdToolsDummy, const UINT uiCmdFirst, const UINT uiCmdLast,
	CRuntimeClass* pToolRTC, UINT uArgMenuID, UINT uInitDirMenuID)

{
	if (afxUserToolsManager != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	afxUserToolsManager = new
		CUserToolsManager( uiCmdToolsDummy, uiCmdFirst, uiCmdLast, pToolRTC, uArgMenuID, uInitDirMenuID);
	m_bUserToolsManagerAutoCreated = TRUE;

	return TRUE;
}

BOOL CWinAppEx::EnableTearOffMenus(LPCTSTR lpszRegEntry, const UINT uiCmdFirst, const UINT uiCmdLast)
{
	if (g_pTearOffMenuManager != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	g_pTearOffMenuManager = new CMenuTearOffManager;
	m_bTearOffManagerAutoCreated = TRUE;

	return g_pTearOffMenuManager->Initialize(lpszRegEntry, uiCmdFirst, uiCmdLast);
}

CMouseManager* CWinAppEx::GetMouseManager()
{
	if (afxMouseManager == NULL)
	{
		InitMouseManager();
	}

	ASSERT_VALID(afxMouseManager);
	return afxMouseManager;
}

CShellManager* CWinAppEx::GetShellManager()
{
	if (afxShellManager == NULL)
	{
		InitShellManager();
	}

	ASSERT_VALID(afxShellManager);
	return afxShellManager;
}

CTooltipManager* CWinAppEx::GetTooltipManager()
{
	if (afxTooltipManager == NULL)
	{
		InitTooltipManager();
	}

	ASSERT_VALID(afxTooltipManager);
	return afxTooltipManager;
}

CContextMenuManager* CWinAppEx::GetContextMenuManager()
{
	if (afxContextMenuManager == NULL)
	{
		InitContextMenuManager();
	}

	ASSERT_VALID(afxContextMenuManager);
	return afxContextMenuManager;
}

CKeyboardManager* CWinAppEx::GetKeyboardManager()
{
	if (afxKeyboardManager == NULL)
	{
		InitKeyboardManager();
	}

	ASSERT_VALID(afxKeyboardManager);
	return afxKeyboardManager;
}

CUserToolsManager* CWinAppEx::GetUserToolsManager()
{
	return afxUserToolsManager;
}

CString CWinAppEx::GetRegSectionPath(LPCTSTR szSectionAdd /*=NULL*/)
{
	CString strSectionPath = ::AFXGetRegPath(m_strRegSection);
	if (szSectionAdd != NULL && _tcslen(szSectionAdd) != 0)
	{
		strSectionPath += szSectionAdd;
		strSectionPath += _T("\\");
	}

	return strSectionPath;
}

BOOL CWinAppEx::LoadState(LPCTSTR lpszSectionName /*=NULL*/, CFrameImpl* pFrameImpl /*= NULL*/)
{
	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath();

	//-----------------------------
	// Other things to do before ?:
	//-----------------------------
	PreLoadState();

	//------------------------
	// Loaded library version:
	//------------------------
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(GetRegSectionPath(strRegEntryVersion)))
	{
		reg.Read(strVersionMajorKey, m_iSavedVersionMajor);
		reg.Read(strVersionMinorKey, m_iSavedVersionMinor);
	}

	//--------------------------------------
	// Save general toolbar/menu parameters:
	//--------------------------------------
	CMFCToolBar::LoadParameters(strSection);
	afxCommandManager->LoadState(strSection);

	BOOL bResetImages = FALSE; // Reset images to default

	if (m_bResourceSmartUpdate)
	{
		CMFCToolBarButton::m_bUpdateImages = FALSE;
	}

	if (pFrameImpl != NULL)
	{
		ASSERT_VALID(pFrameImpl->m_pFrame);

		//-------------------
		// Load rebars state:
		//-------------------
		CMFCReBarState::LoadState(strSection, pFrameImpl->m_pFrame);

		BOOL bPrevDisableRecalcLayout = CDockingManager::m_bDisableRecalcLayout;
		CDockingManager::m_bDisableRecalcLayout = TRUE;

		//-----------------------------------------------------
		// Load all toolbars, menubar and docking control bars:
		//-----------------------------------------------------
		for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
		{
			CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(posTlb);
			ENSURE(pToolBar != NULL);

			if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				if (!m_bLoadSaveFrameBarsOnly || pToolBar->GetTopLevelFrame() == pFrameImpl->m_pFrame)
				{
					if (!pToolBar->IsFloating())
					{
						pToolBar->LoadState(strSection);
						if (pToolBar->IsResourceChanged())
						{
							bResetImages = TRUE;
						}
					}
				}
			}
		}

		//----------------------------
		// Load user defined toolbars:
		//----------------------------
		if (m_bLoadUserToolbars)
		{
			pFrameImpl->LoadUserToolbars();
		}

		//------------------------
		// Load tear-off toolbars:
		//------------------------
		pFrameImpl->LoadTearOffMenus();

		CDockingManager::m_bDisableRecalcLayout = bPrevDisableRecalcLayout;

		CDockState dockState;
		dockState.LoadState(m_strRegSection + strRegEntryNameControlBars);

		if (m_bForceDockStateLoad || pFrameImpl->IsDockStateValid(dockState))
		{
			if ((GetDataVersionMajor() != -1) &&(GetDataVersionMinor() != -1))
			{
				pFrameImpl->LoadDockState(strSection);
				pFrameImpl->SetDockState(dockState);
			}
		}

		if (m_bLoadWindowPlacement)
		{
			//--------------------------------------------------------
			// Set frame default(restored) size:
			//--------------------------------------------------------
			ReloadWindowPlacement(pFrameImpl->m_pFrame);
		}
	}

	//--------------------------------------
	// Load mouse/keyboard/menu managers:
	//--------------------------------------
	if (afxMouseManager != NULL)
	{
		afxMouseManager->LoadState(strSection);
	}

	if (afxContextMenuManager != NULL)
	{
		afxContextMenuManager->LoadState(strSection);
	}

	if (afxKeyboardManager != NULL)
	{
		afxKeyboardManager->LoadState(strSection, pFrameImpl == NULL ? NULL : pFrameImpl->m_pFrame);
	}

	if (afxUserToolsManager != NULL)
	{
		afxUserToolsManager->LoadState(strSection);
	}

	if (m_bResourceSmartUpdate)
	{
		CMFCToolBarButton::m_bUpdateImages = TRUE;
	}

	if (m_bForceImageReset ||(m_bResourceSmartUpdate && bResetImages))
	{
		for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
		{
			CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(posTlb);
			ENSURE(pToolBar != NULL);

			if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				pToolBar->ResetImages();
			}
		}

		if (pFrameImpl != NULL)
		{
			ASSERT_VALID(pFrameImpl->m_pFrame);
			pFrameImpl->m_pFrame->RecalcLayout();
		}
	}

	//----------
	// Call Hook
	//----------
	LoadCustomState();

	//----------------------------------------------------------------------
	// To not confuse internal serialization, set version number to current:
	//----------------------------------------------------------------------
#ifndef _MFC_USER_BUILD
	m_iSavedVersionMajor = rmj;
	m_iSavedVersionMinor = rmm;
#else
	m_iSavedVersionMajor = 9;
	m_iSavedVersionMinor = 0;
#endif

	if (pFrameImpl != NULL)
	{
		ASSERT_VALID(pFrameImpl->m_pFrame);

		if (pFrameImpl->m_pFrame->IsZoomed())
		{
			pFrameImpl->m_pFrame->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}

	return TRUE;
}

BOOL CWinAppEx::LoadState(CMDIFrameWndEx* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{
	ASSERT_VALID(pFrame);
	return LoadState(lpszSectionName, &pFrame->m_Impl);
}

BOOL CWinAppEx::LoadState(CFrameWndEx* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{
	ASSERT_VALID(pFrame);
	return LoadState(lpszSectionName, &pFrame->m_Impl);
}

BOOL CWinAppEx::LoadState(COleIPFrameWndEx* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{
	ASSERT_VALID(pFrame);
	return LoadState(lpszSectionName, &pFrame->m_Impl);
}

BOOL CWinAppEx::CleanState(LPCTSTR lpszSectionName /*=NULL*/)
{
	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath();

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	return reg.DeleteKey(strSection);
}

BOOL CWinAppEx::SaveState(LPCTSTR lpszSectionName  /*=NULL*/, CFrameImpl* pFrameImpl /*= NULL*/)
{
	if (!m_bSaveState)
	{
		return FALSE;
	}

	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath();

	//-----------------------------
	// Other things to do before ?:
	//-----------------------------
	PreSaveState();

	//----------------------
	// Save library version:
	//----------------------
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(GetRegSectionPath(strRegEntryVersion)))
	{
#ifndef _MFC_USER_BUILD
		reg.Write(strVersionMajorKey, rmj);
		reg.Write(strVersionMinorKey, rmm);
#else
		reg.Write(strVersionMajorKey, 9);
		reg.Write(strVersionMinorKey, 0);
#endif
	}

	//--------------------------------------
	// Save general toolbar/menu parameters:
	//--------------------------------------
	CMFCToolBar::SaveParameters(strSection);
	afxCommandManager->SaveState(strSection);

	if (pFrameImpl != NULL)
	{
		CDockState dockState;

		pFrameImpl->m_pFrame->GetDockState(dockState);
		dockState.SaveState(m_strRegSection + strRegEntryNameControlBars);

		pFrameImpl->SaveDockState(strSection);

		//-----------------------------------------------------
		// Save all toolbars, menubar and docking control bars:
		//-----------------------------------------------------
		for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
		{
			CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(posTlb);
			ENSURE(pToolBar != NULL);

			if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				if (!m_bLoadSaveFrameBarsOnly || pToolBar->GetTopLevelFrame() == pFrameImpl->m_pFrame)
				{
					pToolBar->SaveState(strSection);
				}
			}
		}

		//----------------------------
		// Save user defined toolbars:
		//----------------------------
		pFrameImpl->SaveUserToolbars(m_bLoadSaveFrameBarsOnly);

		//------------------------
		// Save tear-off toolbars:
		//------------------------
		pFrameImpl->SaveTearOffMenus(m_bLoadSaveFrameBarsOnly);

		//-------------------
		// Save rebars state:
		//-------------------
		CMFCReBarState::SaveState(strSection, pFrameImpl->m_pFrame);

		//--------------------------
		// Store window placement
		//--------------------------
		pFrameImpl->StoreWindowPlacement();
	}

	//------------------
	// Save user images:
	//------------------
	if (CMFCToolBar::m_pUserImages != NULL)
	{
		ASSERT_VALID(CMFCToolBar::m_pUserImages);
		CMFCToolBar::m_pUserImages->Save();
	}

	//--------------------------------------
	// Save mouse/keyboard/menu managers:
	//--------------------------------------
	if (afxMouseManager != NULL)
	{
		afxMouseManager->SaveState(strSection);
	}

	if (afxContextMenuManager != NULL)
	{
		afxContextMenuManager->SaveState(strSection);
	}

	if (afxKeyboardManager != NULL)
	{
		afxKeyboardManager->SaveState(strSection, pFrameImpl == NULL ? NULL : pFrameImpl->m_pFrame);
	}

	if (afxUserToolsManager != NULL)
	{
		afxUserToolsManager->SaveState(strSection);
	}

	SaveCustomState();
	return TRUE;
}


// Overidables for customization

void CWinAppEx::OnClosingMainFrame(CFrameImpl* pFrame)
{
	// Defaults to automatically saving state.
	SaveState(0, pFrame);
}

//--------------------------------------------------------
// the next one have to be called explicitly in your code:
//--------------------------------------------------------
BOOL CWinAppEx::OnViewDoubleClick(CWnd* pWnd, int iViewId)
{
	if (afxMouseManager == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID(afxMouseManager);

	UINT uiCmd = afxMouseManager->GetViewDblClickCommand(iViewId);
	if (uiCmd > 0 && uiCmd != (UINT) -1)
	{
		if (afxUserToolsManager != NULL && afxUserToolsManager->InvokeTool(uiCmd))
		{
			return TRUE;
		}

		CWnd* pTargetWnd = (pWnd == NULL) ? AfxGetMainWnd() : AFXGetTopLevelFrame(pWnd);
		ASSERT_VALID(pTargetWnd);

		pTargetWnd->SendMessage(WM_COMMAND, uiCmd);
		return TRUE;
	}

	MessageBeep((UINT) -1);
	return FALSE;
}

BOOL CWinAppEx::ShowPopupMenu(UINT uiMenuResId, const CPoint& point, CWnd* pWnd)
{
	if (afxContextMenuManager == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID(afxContextMenuManager);
	return afxContextMenuManager->ShowPopupMenu(uiMenuResId, point.x, point.y, pWnd);
}

BOOL CWinAppEx::ReloadWindowPlacement(CFrameWnd* pFrameWnd)
{
	ASSERT_VALID(pFrameWnd);

	CCommandLineInfo cmdInfo;
	AfxGetApp()->ParseCommandLine(cmdInfo);
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		//Don't show the main window if Application
		//was run with /Embedding or /Automation.
		return FALSE;
	}

	CRect rectNormal;
	int nFlags = 0;
	int nShowCmd = SW_SHOWNORMAL;
	BOOL bRet = FALSE;

	if (LoadWindowPlacement(rectNormal, nFlags, nShowCmd))
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);

		if (pFrameWnd->GetWindowPlacement(&wp))
		{
			wp.rcNormalPosition = rectNormal;
			wp.showCmd = nShowCmd;

			RECT rectDesktop;
			SystemParametersInfo(SPI_GETWORKAREA,0, (PVOID)&rectDesktop,0);
			OffsetRect(&wp.rcNormalPosition, -rectDesktop.left, -rectDesktop.top);

			pFrameWnd->SetWindowPlacement(&wp);

			bRet = TRUE;
		}
	}

	if (pFrameWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CDockingManager *pDockingManager = ((CMDIFrameWndEx *)pFrameWnd)->GetDockingManager();
		pDockingManager->ShowDelayShowMiniFrames(TRUE);
	}
	else if (pFrameWnd->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		CDockingManager *pDockingManager = ((CFrameWndEx *)pFrameWnd)->GetDockingManager();
		pDockingManager->ShowDelayShowMiniFrames(TRUE);
	}

	return bRet;
}

BOOL CWinAppEx::LoadWindowPlacement(CRect& rectNormalPosition, int& nFlags, int& nShowCmd)
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(GetRegSectionPath(strWindowPlacementRegSection)))
	{
		return FALSE;
	}

	return reg.Read(strRectMainKey, rectNormalPosition) && reg.Read(strFlagsKey, nFlags) && reg.Read(strShowCmdKey, nShowCmd);
}

BOOL CWinAppEx::StoreWindowPlacement(const CRect& rectNormalPosition, int nFlags, int nShowCmd)
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (!reg.CreateKey(GetRegSectionPath(strWindowPlacementRegSection)))
	{
		return FALSE;
	}

	return reg.Write(strRectMainKey, rectNormalPosition) && reg.Write(strFlagsKey, nFlags) && reg.Write(strShowCmdKey, nShowCmd);
}

// These functions load and store values from the "Custom" subkey
// To use subkeys of the "Custom" subkey use GetSectionInt() etc.
// instead
int CWinAppEx::GetInt(LPCTSTR lpszEntry, int nDefault /*= 0*/)
{
	return GetSectionInt(_T(""), lpszEntry, nDefault);
}

CString CWinAppEx::GetString(LPCTSTR lpszEntry, LPCTSTR lpszDefault /*= ""*/)
{
	return GetSectionString(_T(""), lpszEntry, lpszDefault);
}

BOOL CWinAppEx::GetBinary(LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes)
{
	return GetSectionBinary(_T(""), lpszEntry, ppData, pBytes);
}

BOOL CWinAppEx::GetObject(LPCTSTR lpszEntry, CObject& obj)
{
	return GetSectionObject(_T(""), lpszEntry, obj);
}

BOOL CWinAppEx::WriteInt(LPCTSTR lpszEntry, int nValue )
{
	return WriteSectionInt(_T(""), lpszEntry, nValue);
}

BOOL CWinAppEx::WriteString(LPCTSTR lpszEntry, LPCTSTR lpszValue )
{
	return WriteSectionString(_T(""), lpszEntry, lpszValue);
}

BOOL CWinAppEx::WriteBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
	return WriteSectionBinary(_T(""), lpszEntry, pData, nBytes);
}

BOOL CWinAppEx::WriteObject(LPCTSTR lpszEntry, CObject& obj)
{
	return WriteSectionObject(_T(""), lpszEntry, obj);
}

// These functions load and store values from a given subkey
// of the "Custom" subkey. For simpler access you may use
// GetInt() etc.
int CWinAppEx::GetSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nDefault /*= 0*/)
{
	ENSURE(lpszSubSection != NULL);
	ENSURE(lpszEntry != NULL);

	int nRet = nDefault;

	CString strSection = GetRegSectionPath(lpszSubSection);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strSection))
	{
		reg.Read(lpszEntry, nRet);
	}
	return nRet;
}

CString CWinAppEx::GetSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault /*= ""*/)
{
	ENSURE(lpszSubSection != NULL);
	ENSURE(lpszEntry != NULL);
	ENSURE(lpszDefault != NULL);

	CString strRet = lpszDefault;

	CString strSection = GetRegSectionPath(lpszSubSection);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strSection))
	{
		reg.Read(lpszEntry, strRet);
	}
	return strRet;
}

BOOL CWinAppEx::GetSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes)
{
	ENSURE(lpszSubSection != NULL);
	ENSURE(lpszEntry != NULL);
	ENSURE(ppData != NULL);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strSection) && reg.Read(lpszEntry, ppData, pBytes) )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CWinAppEx::GetSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj)
{
	ENSURE(lpszSubSection != NULL);
	ENSURE(lpszEntry != NULL);
	ASSERT_VALID(&obj);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strSection) && reg.Read(lpszEntry, obj))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CWinAppEx::WriteSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nValue )
{
	ENSURE(lpszSubSection != NULL);
	ENSURE(lpszEntry != NULL);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		return reg.Write(lpszEntry, nValue);
	}
	return FALSE;
}

BOOL CWinAppEx::WriteSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszValue )
{
	ENSURE(lpszSubSection != NULL);
	ENSURE(lpszEntry != NULL);
	ENSURE(lpszValue != NULL);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		return reg.Write(lpszEntry, lpszValue);
	}
	return FALSE;
}

BOOL CWinAppEx::WriteSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
	ENSURE(lpszSubSection != NULL);
	ENSURE(lpszEntry != NULL);
	ENSURE(pData != NULL);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		return reg.Write(lpszEntry, pData, nBytes);
	}
	return FALSE;
}

BOOL CWinAppEx::WriteSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj)
{
	ENSURE(lpszSubSection != NULL);
	ENSURE(lpszEntry != NULL);
	ASSERT_VALID(&obj);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		return reg.Write(lpszEntry, obj);
	}

	return FALSE;
}

void CWinAppEx::OnAppContextHelp(CWnd* pWndControl, const DWORD dwHelpIDArray [])
{
	::WinHelp(pWndControl->GetSafeHwnd(), AfxGetApp()->m_pszHelpFilePath, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) dwHelpIDArray);
}

BOOL CWinAppEx::SaveState(CMDIFrameWndEx* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{
	ASSERT_VALID(pFrame);
	return SaveState(lpszSectionName, &pFrame->m_Impl);
}

BOOL CWinAppEx::SaveState(CFrameWndEx* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{
	ASSERT_VALID(pFrame);
	return SaveState(lpszSectionName, &pFrame->m_Impl);
}

BOOL CWinAppEx::SaveState(COleIPFrameWndEx* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{
	ASSERT_VALID(pFrame);
	return SaveState(lpszSectionName, &pFrame->m_Impl);
}

BOOL CWinAppEx::IsStateExists(LPCTSTR lpszSectionName /*=NULL*/)
{
	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath();

	//------------------------
	// Loaded library version:
	//------------------------
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	return reg.Open(GetRegSectionPath(strRegEntryVersion));
}

int CWinAppEx::GetDataVersion() const
{
	if (m_iSavedVersionMajor == -1 || m_iSavedVersionMinor == -1)
	{
		return 0xFFFFFFFF;
	}

	int nVersionMinor = m_iSavedVersionMinor / 10;
	int nVersionDigit = m_iSavedVersionMinor % 10;

	nVersionMinor *= 0x100;
	nVersionDigit *= 0x10;

	if (nVersionMinor < 10)
	{
		nVersionDigit *=0x10;
	}

	return m_iSavedVersionMajor * 0x10000 + nVersionMinor + nVersionDigit;
}



