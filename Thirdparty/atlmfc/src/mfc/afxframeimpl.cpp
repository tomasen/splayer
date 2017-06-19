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
#include "multimon.h"

#include "afxframeimpl.h"
#include "afxtoolbar.h"
#include "afxmenubar.h"
#include "afxribbonres.h"
#include "afxpopupmenu.h"
#include "afxtoolbarmenubutton.h"
#include "afxwinappex.h"
#include "afxregpath.h"
#include "afxsettingsstore.h"
#include "afxmenutearoffmanager.h"
#include "afxdocksite.h"
#include "afxkeyboardmanager.h"
#include "afxpaneframewnd.h"
#include "afxpreviewviewex.h"
#include "afxcustomizemenubutton.h"
#include "afxcustomizebutton.h"
#include "afxtoolbarscustomizedialog.h"
#include "afxvisualmanager.h"
#include "afxdropdowntoolbar.h"
#include "afxmdiframewndex.h"
#include "afxframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxribbonbar.h"
#include "afxribbonstatusbar.h"
#include "afxcaptionbutton.h"
#include "afxglobalutils.h"
#include "afxdropdownlistbox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CObList afxAllToolBars;

class CMFCCustomizeButton;

static const CString strTearOffBarsRegEntry = _T("ControlBars-TearOff");

extern BOOL g_bInSettingChange;

BOOL CFrameImpl::m_bControlBarExtraPixel = TRUE;
CList<CFrameWnd*, CFrameWnd*> CFrameImpl::m_lstFrames;

UINT AFX_WM_POSTSETPREVIEWFRAME = ::RegisterWindowMessage(_T("AFX_WM_POSTSETPREVIEWFRAME"));

#pragma warning(disable : 4355)

// Construction/Destruction
CFrameImpl::CFrameImpl(CFrameWnd* pFrame) : m_pFrame(pFrame), m_pDockManager(NULL), m_uiUserToolbarFirst((UINT)-1), m_uiUserToolbarLast((UINT)-1), m_pMenuBar(NULL), m_hDefaultMenu(NULL), m_nIDDefaultResource(0), m_FullScreenMgr(this), m_bLoadDockState(TRUE), m_uiControlbarsMenuEntryID(0), m_bViewMenuShowsToolbarsOnly(FALSE), m_pRibbonBar(NULL), m_pRibbonStatusBar(NULL), m_bCaptured(FALSE), m_nHotSysButton(HTNOWHERE), m_nHitSysButton(HTNOWHERE), m_bIsWindowRgn(FALSE), m_bHasBorder(FALSE), m_bIsOleInPlaceActive(FALSE), m_bHadCaption(TRUE), m_bWindowPosChanging(FALSE)
{
	ASSERT_VALID(m_pFrame);

	m_pCustomUserToolBarRTC = RUNTIME_CLASS(CMFCToolBar);
	m_rectRedraw.SetRectEmpty();

	m_bIsMDIChildFrame = m_pFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWnd));
}

#pragma warning(default : 4355)

CFrameImpl::~CFrameImpl()
{
	// Clear user-defined toolbars:
	while (!m_listUserDefinedToolbars.IsEmpty())
	{
		delete m_listUserDefinedToolbars.RemoveHead();
	}

	// Clear tear-off toolbars:
	while (!m_listTearOffToolbars.IsEmpty())
	{
		delete m_listTearOffToolbars.RemoveHead();
	}

	// Clear caption system buttons:
	while (!m_lstCaptionSysButtons.IsEmpty())
	{
		delete m_lstCaptionSysButtons.RemoveHead();
	}
}

void CFrameImpl::OnCloseFrame()
{
	ASSERT_VALID(m_pFrame);

	// Automatically load state and frame position if CWinAppEx is used:
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp != NULL)
	{
		if (m_FullScreenMgr.IsFullScreen())
		{
			if (::IsWindow(m_pFrame->GetSafeHwnd()))
			{
				m_FullScreenMgr.RestoreState(m_pFrame);
			}
		}

		pApp->OnClosingMainFrame(this);

		// Store the Windowplacement:
		StoreWindowPlacement();
	}
}

void CFrameImpl::StoreWindowPlacement()
{
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp != NULL && ::IsWindow(m_pFrame->GetSafeHwnd()))
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);

		if (m_pFrame->GetWindowPlacement(&wp))
		{
			// Make sure we don't pop up
			// minimized the next time
			if (wp.showCmd != SW_SHOWMAXIMIZED)
			{
				wp.showCmd = SW_SHOWNORMAL;
			}

			RECT rectDesktop;
			SystemParametersInfo(SPI_GETWORKAREA,0, (PVOID)&rectDesktop,0);
			OffsetRect(&wp.rcNormalPosition, rectDesktop.left, rectDesktop.top);

			pApp->StoreWindowPlacement(wp.rcNormalPosition, wp.flags, wp.showCmd);
		}
	}
}

void CFrameImpl::RestorePosition(CREATESTRUCT& cs)
{
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp != NULL && cs.hInstance != NULL)
	{
		CRect rectNormal(CPoint(cs.x, cs.y), CSize(cs.cx, cs.cy));
		int nFlags = 0;
		int nShowCmd = SW_SHOWNORMAL;

		if (!pApp->LoadWindowPlacement(rectNormal, nFlags, nShowCmd))
		{
			return;
		}

		if (nShowCmd != SW_MAXIMIZE)
		{
			nShowCmd = SW_SHOWNORMAL;
		}

		switch (AfxGetApp()->m_nCmdShow)
		{
		case SW_MAXIMIZE:
		case SW_MINIMIZE:
		case SW_SHOWMINIMIZED:
		case SW_SHOWMINNOACTIVE:
			break; // don't change!

		default:
			AfxGetApp()->m_nCmdShow = nShowCmd;
		}

		CRect rectDesktop;
		CRect rectInter;

		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(MonitorFromPoint(rectNormal.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
		{
			rectDesktop = mi.rcWork;
		}
		else
		{
			::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectDesktop, 0);
		}

		if (nShowCmd == SW_MAXIMIZE)
		{
			cs.x = rectDesktop.left;
			cs.y = rectDesktop.top;
			cs.cx = rectDesktop.Width();
			cs.cy = rectDesktop.Height();

			return;
		}

		if (rectInter.IntersectRect(&rectDesktop, &rectNormal))
		{
			cs.x = rectInter.left;
			cs.y = rectInter.top;
			cs.cx = rectNormal.Width();
			cs.cy = rectNormal.Height();
		}
	}
}

void CFrameImpl::OnLoadFrame()
{
	ASSERT_VALID(m_pFrame);

	// Automatically load state if CWinAppEx is used:
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp != NULL)
	{
		pApp->LoadState(0, this);
	}

	if (m_pRibbonStatusBar->GetSafeHwnd() != NULL)
	{
		m_pFrame->SetWindowPos(NULL, -1, -1, -1, -1, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	else if (m_pRibbonBar->GetSafeHwnd() != NULL)
	{
		m_pRibbonBar->RecalcLayout();
	}

	afxGlobalData.m_bIsRTL = (m_pFrame->GetExStyle() & WS_EX_LAYOUTRTL);
}

void CFrameImpl::LoadUserToolbars()
{
	ASSERT_VALID(m_pFrame);
	ENSURE(m_pCustomUserToolBarRTC != NULL);

	if (m_uiUserToolbarFirst == (UINT) -1 || m_uiUserToolbarLast == (UINT) -1)
	{
		return;
	}

	for (UINT uiNewToolbarID = m_uiUserToolbarFirst; uiNewToolbarID <= m_uiUserToolbarLast; uiNewToolbarID ++)
	{
		CMFCToolBar* pNewToolbar = (CMFCToolBar*) m_pCustomUserToolBarRTC->CreateObject();
		if (!pNewToolbar->Create(m_pFrame, AFX_DEFAULT_TOOLBAR_STYLE, uiNewToolbarID))
		{
			TRACE0("Failed to create a new toolbar!\n");
			delete pNewToolbar;
			continue;
		}

		if (!pNewToolbar->LoadState(m_strControlBarRegEntry))
		{
			pNewToolbar->DestroyWindow();
			delete pNewToolbar;
		}
		else
		{
			pNewToolbar->SetPaneStyle(pNewToolbar->GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
			pNewToolbar->EnableDocking(CBRS_ALIGN_ANY);

			ASSERT_VALID(m_pDockManager);
			m_pDockManager->DockPane(pNewToolbar);
			m_listUserDefinedToolbars.AddTail(pNewToolbar);
		}
	}
}

void CFrameImpl::SaveUserToolbars(BOOL bFrameBarsOnly)
{
	for (POSITION pos = m_listUserDefinedToolbars.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBar* pUserToolBar = (CMFCToolBar*) m_listUserDefinedToolbars.GetNext(pos);
		ASSERT_VALID(pUserToolBar);

		if (!bFrameBarsOnly || pUserToolBar->GetTopLevelFrame() == m_pFrame)
		{
			pUserToolBar->SaveState(m_strControlBarRegEntry);
		}
	}
}

CMFCToolBar* CFrameImpl::GetUserToolBarByIndex(int iIndex) const
{
	POSITION pos = m_listUserDefinedToolbars.FindIndex(iIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	CMFCToolBar* pUserToolBar = (CMFCToolBar*) m_listUserDefinedToolbars.GetAt(pos);
	ASSERT_VALID(pUserToolBar);

	return pUserToolBar;
}

BOOL CFrameImpl::IsUserDefinedToolbar(const CMFCToolBar* pToolBar) const
{
	ASSERT_VALID(pToolBar);

	UINT uiCtrlId = pToolBar->GetDlgCtrlID();
	return uiCtrlId >= m_uiUserToolbarFirst && uiCtrlId <= m_uiUserToolbarLast;
}

BOOL CFrameImpl::IsDockStateValid(const CDockState& /*state*/)
{
	ASSERT_VALID(m_pFrame);
	return TRUE;
}

void CFrameImpl::InitUserToolbars( LPCTSTR lpszRegEntry, UINT uiUserToolbarFirst, UINT uiUserToolbarLast)
{
	ASSERT(uiUserToolbarLast >= uiUserToolbarFirst);

	if (uiUserToolbarFirst == (UINT) -1 || uiUserToolbarLast == (UINT) -1)
	{
		ASSERT(FALSE);
		return;
	}

	m_uiUserToolbarFirst = uiUserToolbarFirst;
	m_uiUserToolbarLast = uiUserToolbarLast;

	// Get Path automatically from CWinAppEx if needed
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());

	m_strControlBarRegEntry = (lpszRegEntry == NULL) ? (pApp != NULL ? pApp->GetRegSectionPath() : _T("") ) : lpszRegEntry;
}

UINT __stdcall CFrameImpl::GetFreeCtrlBarID(UINT uiFirstID, UINT uiLastID, const CObList& lstCtrlBars)
{
	if (uiFirstID == (UINT)-1 || uiLastID == (UINT)-1)
	{
		return 0;
	}

	int iMaxToolbars = uiLastID - uiFirstID + 1;
	if (lstCtrlBars.GetCount() == iMaxToolbars)
	{
		return 0;
	}

	for (UINT uiNewToolbarID = uiFirstID; uiNewToolbarID <= uiLastID; uiNewToolbarID ++)
	{
		BOOL bUsed = FALSE;

		for (POSITION pos = lstCtrlBars.GetHeadPosition(); !bUsed && pos != NULL;)
		{
			CMFCToolBar* pToolBar = (CMFCToolBar*) lstCtrlBars.GetNext(pos);
			ASSERT_VALID(pToolBar);

			bUsed = (pToolBar->GetDlgCtrlID() == (int) uiNewToolbarID);
		}

		if (!bUsed)
		{
			return uiNewToolbarID;
		}
	}

	return 0;
}

void CFrameImpl::SetNewUserToolBarRTC(CRuntimeClass* pCustomUserToolBarRTC)
{
	ENSURE(pCustomUserToolBarRTC != NULL);
	m_pCustomUserToolBarRTC = pCustomUserToolBarRTC;
}

const CMFCToolBar* CFrameImpl::CreateNewToolBar(LPCTSTR lpszName)
{
	ASSERT_VALID(m_pFrame);
	ENSURE(lpszName != NULL);

	UINT uiNewToolbarID = GetFreeCtrlBarID(m_uiUserToolbarFirst, m_uiUserToolbarLast, m_listUserDefinedToolbars);

	if (uiNewToolbarID == 0)
	{
		CString strError;
		strError.Format(IDS_AFXBARRES_TOO_MANY_TOOLBARS_FMT, m_uiUserToolbarLast - m_uiUserToolbarFirst + 1);

		AfxMessageBox(strError, MB_OK | MB_ICONASTERISK);
		return NULL;
	}

	CMFCToolBar* pNewToolbar = (CMFCToolBar*) m_pCustomUserToolBarRTC->CreateObject();
	if (!pNewToolbar->Create(m_pFrame, AFX_DEFAULT_TOOLBAR_STYLE, uiNewToolbarID))
	{
		TRACE0("Failed to create a new toolbar!\n");
		delete pNewToolbar;
		return NULL;
	}

	pNewToolbar->SetWindowText(lpszName);

	pNewToolbar->SetPaneStyle(pNewToolbar->GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	pNewToolbar->EnableDocking(CBRS_ALIGN_ANY);

	CRect rectBar;
	pNewToolbar->GetWindowRect(rectBar);
	int nLeft = ::GetSystemMetrics(SM_CXFULLSCREEN) / 2;
	int nTop  = ::GetSystemMetrics(SM_CYFULLSCREEN) / 2;

	CRect rectFloat(nLeft, nTop, nLeft + rectBar.Width(), nTop + rectBar.Height());
	pNewToolbar->FloatPane(rectFloat, DM_UNKNOWN);
	pNewToolbar->m_nMRUWidth = 32767;
	m_pFrame->RecalcLayout();

	m_listUserDefinedToolbars.AddTail(pNewToolbar);
	return pNewToolbar;
}

void CFrameImpl::AddTearOffToolbar(CBasePane* pToolBar)
{
	ASSERT_VALID(pToolBar);
	m_listTearOffToolbars.AddTail(pToolBar);
}

void CFrameImpl::RemoveTearOffToolbar(CBasePane* pToolBar)
{
	ASSERT_VALID(pToolBar);

	POSITION pos = m_listTearOffToolbars.Find(pToolBar);
	if (pos != NULL)
	{
		m_listTearOffToolbars.RemoveAt(pos);
	}
}

void CFrameImpl::LoadTearOffMenus()
{
	ASSERT_VALID(m_pFrame);

	// Remove current tear-off bars:
	for (POSITION pos = m_listTearOffToolbars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*) m_listTearOffToolbars.GetNext(pos);
		ASSERT_VALID(pBar);

		if (pBar->IsDocked())
		{
			pBar->UndockPane(TRUE);
		}

		pBar->DestroyWindow();
		delete pBar;
	}

	m_listTearOffToolbars.RemoveAll();

	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	CString strProfileName = pApp != NULL ? pApp->GetRegSectionPath() : _T("");

	strProfileName += strTearOffBarsRegEntry;

	for (int iIndex = 0;; iIndex++)
	{
		CString strKey;
		strKey.Format(_T("%s-%d"), (LPCTSTR)strProfileName, iIndex);

		int iId = 0;
		CMFCToolBar* pToolBar = NULL;
		CString strName;

		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, TRUE);

		if (!reg.Open(strKey) || !reg.Read(_T("ID"), iId) || !reg.Read(_T("Name"), strName) || !reg.Read(_T("State"), (CObject*&) pToolBar))
		{
			break;
		}

		ASSERT_VALID(pToolBar);

		if (!pToolBar->Create(m_pFrame, AFX_DEFAULT_TOOLBAR_STYLE, (UINT) iId))
		{
			TRACE0("Failed to create a new toolbar!\n");
			delete pToolBar;
			break;
		}

		pToolBar->SetWindowText(strName);

		pToolBar->SetPaneStyle(pToolBar->GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
		pToolBar->EnableDocking(CBRS_ALIGN_ANY);

		ASSERT_VALID(m_pDockManager);
		m_listTearOffToolbars.AddTail(pToolBar);
		pToolBar->LoadState(strProfileName, iIndex);
		m_pDockManager->DockPane(pToolBar);
	}
}

void CFrameImpl::SaveTearOffMenus(BOOL bFrameBarsOnly)
{
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());

	CString strProfileName = pApp != NULL ? pApp->GetRegSectionPath() : _T("");
	strProfileName += strTearOffBarsRegEntry;

	int iIndex = 0;

	// First, clear old tear-off toolbars in registry:
	for (iIndex = 0;; iIndex++)
	{
		CString strKey;
		strKey.Format(_T("%s-%d"), (LPCTSTR)strProfileName, iIndex);

		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, FALSE);

		if (!reg.DeleteKey(strKey))
		{
			break;
		}
	}

	iIndex = 0;

	for (POSITION pos = m_listTearOffToolbars.GetHeadPosition(); pos != NULL; iIndex ++)
	{
		CBasePane* pBar = (CBasePane*) m_listTearOffToolbars.GetNext(pos);
		ASSERT_VALID(pBar);

		if ((!bFrameBarsOnly || pBar->GetTopLevelFrame() == m_pFrame) && pBar->IsPaneVisible())
		{
			CString strName;
			pBar->GetWindowText(strName);

			CString strKey;
			strKey.Format(_T("%s-%d"), (LPCTSTR)strProfileName, iIndex);

			CSettingsStoreSP regSP;
			CSettingsStore& reg = regSP.Create(FALSE, FALSE);

			reg.CreateKey(strKey);

			reg.Write(_T("ID"), (int) pBar->GetDlgCtrlID());
			reg.Write(_T("Name"), strName);
			reg.Write(_T("State"), pBar);
			pBar->SaveState(strProfileName, iIndex);
		}
	}
}

BOOL CFrameImpl::DeleteToolBar(CMFCToolBar* pToolBar)
{
	ASSERT_VALID(m_pFrame);
	ASSERT_VALID(pToolBar);

	POSITION pos = m_listUserDefinedToolbars.Find(pToolBar);
	if (pos == NULL)
	{
		return FALSE;
	}

	m_listUserDefinedToolbars.RemoveAt(pos);
	pToolBar->RemoveStateFromRegistry(m_strControlBarRegEntry);

	CDockSite* pParentDockBar = pToolBar->GetParentDockSite();
	CPaneFrameWnd* pParentMiniFrame = pToolBar->GetParentMiniFrame();
	if (pParentDockBar != NULL)
	{
		ASSERT_VALID(pParentDockBar);
		pParentDockBar->RemovePane(pToolBar, DM_UNKNOWN);
	}
	else if (pParentMiniFrame != NULL)
	{
		ASSERT_VALID(pParentMiniFrame);
		pParentMiniFrame->RemovePane(pToolBar);
	}

	pToolBar->DestroyWindow();
	delete pToolBar;

	m_pFrame->RecalcLayout();
	return TRUE;
}

void CFrameImpl::SetMenuBar(CMFCMenuBar* pMenuBar)
{
	ASSERT_VALID(m_pFrame);
	ASSERT_VALID(pMenuBar);
	ENSURE(m_pMenuBar == NULL); // Method should be called once!

	m_pMenuBar = pMenuBar;

	m_hDefaultMenu=*m_pFrame->GetMenu();

	// Support for dynamic menu
	m_pMenuBar->OnDefaultMenuLoaded(m_hDefaultMenu);
	m_pMenuBar->CreateFromMenu(m_hDefaultMenu, TRUE /* Default menu */);

	m_pFrame->SetMenu(NULL);

	m_pMenuBar->SetDefaultMenuResId(m_nIDDefaultResource);
}

BOOL CFrameImpl::ProcessKeyboard(int nKey, BOOL* pbProcessAccel)
{
	ASSERT_VALID(m_pFrame);

	if (pbProcessAccel != NULL)
	{
		*pbProcessAccel = TRUE;
	}

	// If popup menu is active, pass keyboard control to menu:
	if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd))
	{
		CWnd* pFocus = CWnd::GetFocus();

		if (CMFCPopupMenu::m_pActivePopupMenu->IsRibbonMiniToolBar())
		{
			BOOL bIsFloatyActive = (pFocus->GetSafeHwnd() != NULL &&
				(CMFCPopupMenu::m_pActivePopupMenu->IsChild(pFocus) || pFocus->GetSafeHwnd() == CMFCPopupMenu::m_pActivePopupMenu->GetSafeHwnd()));

			if (bIsFloatyActive)
			{
				return FALSE;
			}

			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);
			return FALSE;
		}

		if (pFocus->GetSafeHwnd() != NULL && CMFCPopupMenu::m_pActivePopupMenu->IsChild(pFocus))
		{
			return FALSE;
		}

		BOOL bIsDropList = CMFCPopupMenu::m_pActivePopupMenu->GetMenuBar()->IsDropDownListMode();

		CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_KEYDOWN, nKey);
		if (!bIsDropList)
		{
			return TRUE;
		}

		CMFCDropDownListBox* pDropDownList = DYNAMIC_DOWNCAST(CMFCDropDownListBox, CMFCPopupMenu::m_pActivePopupMenu);

		return pDropDownList == NULL || !pDropDownList->IsEditFocused();
	}

	// If appication is minimized, don't handle
	// any keyboard accelerators:
	if (m_pFrame->IsIconic())
	{
		return TRUE;
	}

	// Don't handle keybaord accererators in customization mode:
	if (CMFCToolBar::IsCustomizeMode())
	{
		return FALSE;
	}

	// Is any toolbar control(such as combobox) has focus?
	BOOL bIsToolbarCtrlFocus = FALSE;
	for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); !bIsToolbarCtrlFocus && posTlb != NULL;)
	{
		CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(posTlb);
		ENSURE(pToolBar != NULL);

		if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			for (int i = 0; i < pToolBar->GetCount(); i++)
			{
				CMFCToolBarButton* pButton = pToolBar->GetButton(i);
				ASSERT_VALID(pButton);

				if (pButton->HasFocus())
				{
					bIsToolbarCtrlFocus = TRUE;
					break;
				}
			}
		}
	}

	// Check for the keyboard accelerators:
	BYTE fVirt = 0;

	if (::GetAsyncKeyState(VK_CONTROL) & 0x8000)
	{
		fVirt |= FCONTROL;
	}

	if (::GetAsyncKeyState(VK_MENU) & 0x8000)
	{
		fVirt |= FALT;
	}

	if (::GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		fVirt |= FSHIFT;
	}

	if (!bIsToolbarCtrlFocus)
	{
		if (CKeyboardManager::IsKeyHandled((WORD) nKey, (BYTE)(fVirt | FVIRTKEY), m_pFrame, TRUE) ||
			CKeyboardManager::IsKeyHandled((WORD) nKey, (BYTE)(fVirt | FVIRTKEY), m_pFrame->GetActiveFrame(), FALSE))
		{
			return FALSE;
		}
	}

	if (m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible() && fVirt == FCONTROL && nKey == VK_F1 && m_pRibbonBar->GetActiveCategory() != NULL)
	{
		m_pRibbonBar->ToggleMimimizeState();
		return TRUE;
	}

	if (fVirt == FALT)
	{
		// Handle menu accelerators(such as "Alt-F"):
		if (OnMenuChar(nKey))
		{
			return TRUE;
		}
	}

	if (bIsToolbarCtrlFocus && pbProcessAccel != NULL)
	{
		// Don't process default keyboard accelerators:
		*pbProcessAccel = FALSE;
	}

	return FALSE;
}

BOOL CFrameImpl::ProcessMouseClick(UINT uiMsg, POINT pt, HWND hwnd)
{
	ASSERT_VALID(m_pFrame);

	if (m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible())
	{
		CRect rectRibbon;
		m_pRibbonBar->GetWindowRect (rectRibbon);

		m_pRibbonBar->DeactivateKeyboardFocus (rectRibbon.PtInRect (pt));
	}

	// Maybe user start drag the button with control?
	if (uiMsg == WM_LBUTTONDOWN && (CMFCToolBar::IsCustomizeMode() || (::GetAsyncKeyState(VK_MENU) & 0x8000))) // ALT is pressed
	{
		for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
		{
			CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(posTlb);
			ENSURE(pToolBar != NULL);

			if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				CPoint ptToolBar = pt;
				pToolBar->ScreenToClient(&ptToolBar);

				int iHit = pToolBar->HitTest(ptToolBar);
				if (iHit >= 0)
				{
					CMFCToolBarButton* pButton = pToolBar->GetButton(iHit);
					ASSERT_VALID(pButton);

					if (pButton->GetHwnd() != NULL && pButton->GetHwnd() == hwnd && pButton->Rect().PtInRect(ptToolBar))
					{
						pToolBar->SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(ptToolBar.x, ptToolBar.y));
						return TRUE;
					}

					break;
				}
			}
		}
	}

	BOOL bStopProcessing = FALSE;

	if (!CMFCToolBar::IsCustomizeMode() && CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd))
	{
		CMFCPopupMenu::MENUAREA_TYPE clickArea = CMFCPopupMenu::m_pActivePopupMenu->CheckArea(pt);

		if (clickArea == CMFCPopupMenu::OUTSIDE)
		{
			// Click outside of menu
			// Maybe click on connected floaty?
			CMFCPopupMenu* pMenuWithFloaty = CMFCPopupMenu::FindMenuWithConnectedFloaty();

			if (pMenuWithFloaty != NULL && ::IsWindow(pMenuWithFloaty->m_hwndConnectedFloaty))
			{
				CRect rectFloaty;
				::GetWindowRect(pMenuWithFloaty->m_hwndConnectedFloaty, &rectFloaty);

				if (rectFloaty.PtInRect(pt))
				{
					// Disconnect floaty from the menu:

					CMFCPopupMenu* pFloaty = DYNAMIC_DOWNCAST(CMFCPopupMenu, CWnd::FromHandlePermanent(pMenuWithFloaty->m_hwndConnectedFloaty));

					pMenuWithFloaty->m_hwndConnectedFloaty = NULL;
					pMenuWithFloaty->SendMessage(WM_CLOSE);

					CMFCPopupMenu::m_pActivePopupMenu = pFloaty;
					return FALSE;
				}
			}

			// Maybe secondary click on the parent button?
			CRect rectParentBtn;
			CWnd* pWndParent = CMFCPopupMenu::m_pActivePopupMenu-> GetParentArea(rectParentBtn);

			if (pWndParent != NULL)
			{
				CMFCPopupMenuBar* pWndParentPopupMenuBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, pWndParent);

				CPoint ptClient = pt;
				pWndParent->ScreenToClient(&ptClient);

				if (rectParentBtn.PtInRect(ptClient))
				{
					// If user clicks second time on the parent button,
					// we should close an active menu on the toolbar/menubar
					// and leave it on the popup menu:
					if ((pWndParentPopupMenuBar == NULL || pWndParentPopupMenuBar->IsRibbonPanelInRegularMode()) && !CMFCPopupMenu::m_pActivePopupMenu->InCommand())
					{
						// Toolbar/menu bar: close an active menu!
						CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);
					}
					else if ((uiMsg == WM_RBUTTONDOWN || uiMsg == WM_RBUTTONUP) && CMFCPopupMenu::m_pActivePopupMenu->GetParentRibbonElement() != NULL)
					{
						return FALSE;
					}

					return TRUE;
				}

				if (pWndParentPopupMenuBar != NULL && !pWndParentPopupMenuBar->IsRibbonPanelInRegularMode())
				{
					pWndParentPopupMenuBar->CloseDelayedSubMenu();

					CMFCPopupMenu* pWndParentPopupMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pWndParentPopupMenuBar->GetParent());

					if (pWndParentPopupMenu != NULL)
					{
						CMFCPopupMenu::MENUAREA_TYPE clickAreaParent = pWndParentPopupMenu->CheckArea(pt);

						switch (clickAreaParent)
						{
						case CMFCPopupMenu::MENU:
						case CMFCPopupMenu::TEAROFF_CAPTION:
						case CMFCPopupMenu::LOGO:
							return FALSE;

						case CMFCPopupMenu::SHADOW_RIGHT:
						case CMFCPopupMenu::SHADOW_BOTTOM:
							pWndParentPopupMenu->SendMessage(WM_CLOSE);
							m_pFrame->SetFocus();

							return TRUE;
						}
					}
				}
			}

			if (!CMFCPopupMenu::m_pActivePopupMenu->InCommand())
			{
				bStopProcessing = !CMFCPopupMenu::m_pActivePopupMenu->DefaultMouseClickOnClose();

				CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);

				CWnd* pWndFocus = CWnd::GetFocus();
				if (pWndFocus != NULL && pWndFocus->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
				{
					m_pFrame->SetFocus();
				}

				if (clickArea != CMFCPopupMenu::OUTSIDE) // Click on shadow
				{
					return TRUE;
				}

				if (bStopProcessing)
				{
					// We need to stop processing in case of clicking inside the active view only!
					CView* pView = DYNAMIC_DOWNCAST(CView, CWnd::WindowFromPoint(pt));
					if (pView->GetSafeHwnd() == NULL)
					{
						bStopProcessing = FALSE;
					}
				}
			}
		}
		else if (clickArea == CMFCPopupMenu::SHADOW_RIGHT || clickArea == CMFCPopupMenu::SHADOW_BOTTOM)
		{
			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);
			m_pFrame->SetFocus();

			return TRUE;
		}
	}

	return bStopProcessing;
}

BOOL CFrameImpl::ProcessMouseMove(POINT pt)
{
	if (!CMFCToolBar::IsCustomizeMode() && CMFCPopupMenu::m_pActivePopupMenu != NULL)
	{
		CMFCPopupMenu* pMenuWithFloaty = CMFCPopupMenu::FindMenuWithConnectedFloaty();

		if (pMenuWithFloaty != NULL && ::IsWindow(pMenuWithFloaty->m_hwndConnectedFloaty))
		{
			CRect rectFloaty;
			::GetWindowRect(pMenuWithFloaty->m_hwndConnectedFloaty, &rectFloaty);

			if (rectFloaty.PtInRect(pt))
			{
				return FALSE; // Default processing
			}
		}

		CRect rectMenu;
		CMFCPopupMenu::m_pActivePopupMenu->GetWindowRect(rectMenu);

		if (rectMenu.PtInRect(pt) || CMFCPopupMenu::m_pActivePopupMenu->GetMenuBar()->FindDestintationToolBar(pt) != NULL)
		{
			return FALSE; // Default processing
		}

		return TRUE; // Active menu "capturing"
	}

	return FALSE; // Default processing
}

BOOL CFrameImpl::ProcessMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd))
	{
		if (CMFCPopupMenu::m_pActivePopupMenu->IsScrollable())
		{
			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_MOUSEWHEEL, wParam, lParam);
		}

		if (CMFCPopupMenu::m_pActivePopupMenu->IsRibbonMiniToolBar ())
		{
			CWnd* pFocus = CWnd::GetFocus();

			BOOL bIsFloatyActive = (pFocus->GetSafeHwnd () != NULL && 
				(CMFCPopupMenu::m_pActivePopupMenu->IsChild (pFocus) || 
				pFocus->GetSafeHwnd () == CMFCPopupMenu::m_pActivePopupMenu->GetSafeHwnd ()));

			if (!bIsFloatyActive)
			{
				CMFCPopupMenu::m_pActivePopupMenu->SendMessage (WM_CLOSE);
			}
		}

		return TRUE;
	}

	if (m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible())
	{
		return(BOOL) m_pRibbonBar->SendMessage(WM_MOUSEWHEEL, wParam, lParam);
	}

	return FALSE;
}

BOOL CFrameImpl::OnShowPopupMenu(CMFCPopupMenu* pMenuPopup, CFrameWnd* /*pWndFrame*/)
{
	CSmartDockingManager* pSDMananger = m_pDockManager == NULL ? NULL : m_pDockManager->GetSmartDockingManagerPermanent();
	if (pSDMananger != NULL && pSDMananger->IsStarted())
	{
		return FALSE;
	}

	if (pMenuPopup != NULL && m_uiControlbarsMenuEntryID != 0)
	{
		CMFCPopupMenuBar* pPopupMenuBar = pMenuPopup->GetMenuBar();

		if (m_pDockManager != NULL && pPopupMenuBar->CommandToIndex(m_uiControlbarsMenuEntryID) >= 0)
		{
			if (CMFCToolBar::IsCustomizeMode())
			{
				return FALSE;
			}

			pMenuPopup->RemoveAllItems();

			CMenu menu;
			menu.CreatePopupMenu();

			m_pDockManager->BuildPanesMenu(menu, m_bViewMenuShowsToolbarsOnly);

			pMenuPopup->GetMenuBar()->ImportFromMenu((HMENU) menu, TRUE);
			m_pDockManager->m_bControlBarsMenuIsShown = TRUE;
		}
	}

	CMFCPopupMenu::m_pActivePopupMenu = pMenuPopup;

	if (pMenuPopup != NULL && IsCustomizePane(pMenuPopup))
	{
		ShowQuickCustomizePane(pMenuPopup);
	}

	if (pMenuPopup != NULL && !CMFCToolBar::IsCustomizeMode())
	{
		ASSERT_VALID(pMenuPopup);

		CBasePane* pTopLevelBar = NULL;

		for (CMFCPopupMenu* pMenu = pMenuPopup; pMenu != NULL; pMenu = pMenu->GetParentPopupMenu())
		{
			CMFCToolBarMenuButton* pParentButton = pMenu->GetParentButton();
			if (pParentButton == NULL)
			{
				break;
			}

			pTopLevelBar = DYNAMIC_DOWNCAST(CBasePane, pParentButton->GetParentWnd());
		}

		if (pTopLevelBar != NULL && !pTopLevelBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
		{
			ASSERT_VALID(pTopLevelBar);

			if (pTopLevelBar->IsDocked() && ::GetFocus() != pTopLevelBar->GetSafeHwnd() && CMFCPopupMenu::GetForceMenuFocus())
			{
				pTopLevelBar->SetFocus();
			}
		}
	}

	return TRUE;
}

void CFrameImpl::SetupToolbarMenu(CMenu& menu, const UINT uiViewUserToolbarCmdFirst, const UINT uiViewUserToolbarCmdLast)
{
	// Replace toolbar dummy items to the user-defined toolbar names:
	for (int i = 0; i <(int) menu.GetMenuItemCount();)
	{
		UINT uiCmd = menu.GetMenuItemID(i);

		if (uiCmd >= uiViewUserToolbarCmdFirst && uiCmd <= uiViewUserToolbarCmdLast)
		{
			// "User toolbar" item. First check that toolbar number 'x' is exist:
			CMFCToolBar* pToolBar = GetUserToolBarByIndex(uiCmd - uiViewUserToolbarCmdFirst);
			if (pToolBar != NULL)
			{
				ASSERT_VALID(pToolBar);

				// Modify the current menu item text to the toolbar title and
				// move next:
				CString strToolbarName;
				pToolBar->GetWindowText(strToolbarName);

				menu.ModifyMenu(i ++, MF_BYPOSITION | MF_STRING, uiCmd, strToolbarName);
			}
			else
			{
				menu.DeleteMenu(i, MF_BYPOSITION);
			}
		}
		else // Not "user toolbar" item, move next
		{
			i ++;
		}
	}
}

BOOL CFrameImpl::OnMenuChar(UINT nChar)
{
	ASSERT_VALID(m_pFrame);

	if (m_pRibbonBar != NULL && (m_pRibbonBar->GetStyle() & WS_VISIBLE) && m_pRibbonBar->TranslateChar(nChar))
	{
		return TRUE;
	}

	BOOL bInPrintPreview = m_pDockManager != NULL && m_pDockManager->IsPrintPreviewValid();

	if (!bInPrintPreview)
	{
		if (m_pMenuBar != NULL && (m_pMenuBar->GetStyle() & WS_VISIBLE) && m_pMenuBar->TranslateChar(nChar))
		{
			return TRUE;
		}
	}

	for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
	{
		CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(posTlb);
		ENSURE(pToolBar != NULL);

		if (bInPrintPreview && !pToolBar->IsKindOf(RUNTIME_CLASS(CMFCPrintPreviewToolBar)))
		{
			continue;
		}

		if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL && pToolBar != m_pMenuBar &&
			(pToolBar->GetStyle() & WS_VISIBLE) && pToolBar->GetTopLevelFrame() == m_pFrame && pToolBar->TranslateChar(nChar))
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CFrameImpl::SaveDockState(LPCTSTR lpszSectionName)
{
	if (m_pDockManager != NULL)
	{
		m_pDockManager->SaveState(lpszSectionName, m_nIDDefaultResource);
	}
}

void CFrameImpl::LoadDockState(LPCTSTR lpszSectionName)
{
	if (m_pDockManager != NULL && m_bLoadDockState)
	{
		m_pDockManager->LoadState(lpszSectionName, m_nIDDefaultResource);
	}
}

void CFrameImpl::SetDockState(const CDockState& /*state*/)
{
	ASSERT_VALID(m_pFrame);
	ASSERT_VALID(m_pDockManager);

	if (m_pDockManager != NULL)
	{
		m_pDockManager->SetDockState();
	}
}

BOOL __stdcall CFrameImpl::IsHelpKey(LPMSG lpMsg)
{
	return lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_F1 && !(HIWORD(lpMsg->lParam) & KF_REPEAT) &&
		GetKeyState(VK_SHIFT) >= 0 && GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_MENU) >= 0;
}

void CFrameImpl::DeactivateMenu()
{
	if (!CMFCToolBar::IsCustomizeMode() && CMFCPopupMenu::m_pActivePopupMenu != NULL)
	{
		if (m_pMenuBar != NULL)
		{
			m_pMenuBar->Deactivate();
		}
	}

	if (m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible())
	{
		m_pRibbonBar->DeactivateKeyboardFocus(FALSE);
	}
}

BOOL CFrameImpl::LoadLargeIconsState()
{
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp != NULL)
	{
		return CMFCToolBar::LoadLargeIconsState(pApp->GetRegSectionPath());
	}
	else
	{
		return FALSE;
	}
}

void CFrameImpl::ShowQuickCustomizePane(CMFCPopupMenu* pMenuPopup)
{
	// Get Actual toolbar pointer
	CMFCToolBar* pWndParentToolbar = NULL;

	CMFCPopupMenu* pPopupLevel2 = pMenuPopup->GetParentPopupMenu();
	if (pPopupLevel2 == NULL)
	{
		return;
	}

	CMFCPopupMenu* pPopupLevel1 = pPopupLevel2->GetParentPopupMenu();
	if (pPopupLevel1 == NULL)
	{
		return;
	}

	CMFCCustomizeButton* pCustom = (CMFCCustomizeButton*)pPopupLevel1->GetParentButton();
	if (pCustom == NULL)
	{
		//May be MiniFrameWnd
		CWnd* pFrame = pPopupLevel1->GetOwner();
		if (pFrame == NULL)
		{
			return;
		}

		if (pFrame->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
		{
			CPaneFrameWnd* pMinFrm = (CPaneFrameWnd*)pFrame;

			pWndParentToolbar = (CMFCToolBar*)pMinFrm->GetPane();

		}else
		{
			return;
		}
	}
	else
	{
		if (!pCustom->IsKindOf(RUNTIME_CLASS(CMFCCustomizeButton)))
		{
			return;
		}

		CMFCToolBar* pCurrentToolBar = pCustom->GetParentToolbar();

		CMFCToolBarMenuButton* btnDummy = pMenuPopup->GetMenuItem(0);
		int nID = _ttoi(btnDummy->m_strText);

		const CObList& lstAllToolbars = CMFCToolBar::GetAllToolbars();

		CMFCToolBar* pRealToolBar = NULL;
		for (POSITION pos = lstAllToolbars.GetHeadPosition(); pos != NULL;)
		{
			pRealToolBar = (CMFCToolBar*) lstAllToolbars.GetNext(pos);
			ENSURE(pRealToolBar != NULL);
			if (nID == pRealToolBar->GetDlgCtrlID() && pRealToolBar->IsAddRemoveQuickCustomize())
			{
				break;
			}

			pRealToolBar = NULL;
		}

		if (pRealToolBar == NULL)
		{
			pWndParentToolbar = pCurrentToolBar;
		}
		else
		{
			pWndParentToolbar = pRealToolBar;
		}
	}

	pMenuPopup->RemoveAllItems();

	CMFCToolBarsCustomizeDialog* pStdCust = new CMFCToolBarsCustomizeDialog(m_pFrame, TRUE, AFX_CUSTOMIZE_MENUAMPERS);

	CMFCCustomizeMenuButton::SetParentToolbar(pWndParentToolbar);

	// Populate pop-up menu
	UINT uiRealCount = 0;
	CMFCCustomizeMenuButton::m_mapPresentIDs.RemoveAll();

	UINT uiCount = pWndParentToolbar->GetCount();
	for (UINT i=0; i< uiCount; i++)
	{
		CMFCToolBarButton* pBtn = pWndParentToolbar->GetButton(i);

		if (pBtn->IsKindOf(RUNTIME_CLASS(CMFCCustomizeButton)) ||(pBtn->m_nStyle & TBBS_SEPARATOR))
		{
			continue;
		}

		CMFCCustomizeMenuButton::m_mapPresentIDs.SetAt(pBtn->m_nID, 0);

		//Find Command Text if empty
		CString strText = pBtn->m_strText;
		if (pBtn->m_strText.IsEmpty())
		{
			strText = pStdCust->GetCommandName(pBtn->m_nID);
		}

		UINT uiID = pBtn->m_nID;
		if ((pBtn->m_nID == 0) ||(pBtn->m_nID == -1))
		{
			uiID = AFX_CUSTOMIZE_INTERNAL_ID;
		}

		CMFCCustomizeMenuButton button(uiID, NULL, pBtn->GetImage(), strText, pBtn->m_bUserButton);
		button.SetItemIndex(i);
		pMenuPopup->InsertItem(button);

		uiRealCount++;
	}

	delete pStdCust;

	pMenuPopup->SetQuickCustomizeType(CMFCPopupMenu::QUICK_CUSTOMIZE_PANE);

	//Give User ability to customize pane
	OnShowCustomizePane(pMenuPopup, pWndParentToolbar->GetResourceID());

	if (uiRealCount > 0)
	{
		pMenuPopup->InsertSeparator();
	}

	// Add Reset Toolbar Button
	CString strCommand;
	ENSURE(strCommand.LoadString(IDS_AFXBARRES_RESET_TOOLBAR));

	CMFCCustomizeMenuButton btnReset(AFX_CUSTOMIZE_INTERNAL_ID, NULL, -1, strCommand, FALSE);
	btnReset.SetItemIndex(ID_AFXBARRES_TOOLBAR_RESET_PROMT);

	pMenuPopup->InsertItem(btnReset);
}

BOOL CFrameImpl::OnShowCustomizePane(CMFCPopupMenu* pMenuPane, UINT uiToolbarID)
{
	BOOL bResult = FALSE;

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, m_pFrame);

	if (pMainFrame != NULL)
	{
		bResult = pMainFrame->OnShowCustomizePane(pMenuPane, uiToolbarID);
	}
	else // Maybe, SDI frame...
	{
		CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, m_pFrame);
		if (pFrame != NULL)
		{
			bResult = pFrame->OnShowCustomizePane(pMenuPane, uiToolbarID);

		}else // Maybe, OLE frame
		{
			COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, m_pFrame);
			if (pOleFrame != NULL)
			{
				bResult = pOleFrame->OnShowCustomizePane(pMenuPane, uiToolbarID);
			}
			else
			{
				COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, m_pFrame);
				if (pOleDocFrame != NULL)
				{
					bResult = pOleDocFrame->OnShowCustomizePane(pMenuPane, uiToolbarID);
				}
			}
		}
	}

	return bResult;
}

void CFrameImpl::AddDefaultButtonsToCustomizePane(CMFCPopupMenu* pMenuPane, UINT /*uiToolbarID*/)
{
	CMFCToolBar* pWndParentToolbar = CMFCCustomizeMenuButton::GetParentToolbar();

	if (pWndParentToolbar == NULL)
	{
		return;
	}

	CMFCToolBarsCustomizeDialog* pStdCust = new CMFCToolBarsCustomizeDialog(m_pFrame, TRUE, AFX_CUSTOMIZE_MENUAMPERS);

	const CObList& lstOrigButtons = pWndParentToolbar->GetOrigResetButtons();

	int i = 0;
	int nTmp = 0;
	for (POSITION posCurr = lstOrigButtons.GetHeadPosition(); posCurr != NULL; i++)
	{
		CMFCToolBarButton* pButtonCurr = (CMFCToolBarButton*)lstOrigButtons.GetNext(posCurr);

		UINT uiID = pButtonCurr->m_nID;

		if ((pButtonCurr == NULL) ||
			(pButtonCurr->m_nStyle & TBBS_SEPARATOR) ||
			(pButtonCurr->IsKindOf(RUNTIME_CLASS(CMFCCustomizeButton))) || CMFCCustomizeMenuButton::m_mapPresentIDs.Lookup(uiID, nTmp))
		{
			continue;
		}

		if (pButtonCurr->IsKindOf(RUNTIME_CLASS(CMFCDropDownToolbarButton)))
		{
			CMFCDropDownToolbarButton* pDropButton = DYNAMIC_DOWNCAST(CMFCDropDownToolbarButton, pButtonCurr);

			CMFCToolBar* pDropToolBar = pDropButton->GetDropDownToolBar();
			if (pDropToolBar != NULL)
			{
				int nIndex = pDropToolBar->CommandToIndex(uiID);
				if (nIndex != -1)
				{
					continue;
				}
			}
		}

		if (pButtonCurr->IsKindOf(RUNTIME_CLASS(CMFCToolBarMenuButton)))
		{
			CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButtonCurr);

			if (pMenuButton->IsMenuPaletteMode())
			{
				const CObList& lstMenuItems = pMenuButton->GetCommands();
				BOOL bIsExist = FALSE;

				for (POSITION posCommand = lstMenuItems.GetHeadPosition(); !bIsExist && posCommand != NULL;)
				{
					CMFCToolBarMenuButton* pMenuItem = (CMFCToolBarMenuButton*) lstMenuItems.GetNext(posCommand);
					ASSERT_VALID(pMenuItem);

					bIsExist = (pMenuItem->m_nID == uiID);
				}

				if (bIsExist)
				{
					continue;
				}
			}
		}

		if ((pButtonCurr->m_nID == 0) ||(pButtonCurr->m_nID == -1))
		{
			uiID = AFX_CUSTOMIZE_INTERNAL_ID;
		}

		CMFCCustomizeMenuButton button(uiID, NULL, pButtonCurr->GetImage(), pStdCust->GetCommandName(pButtonCurr->m_nID), pButtonCurr->m_bUserButton);

		button.SetItemIndex(i, FALSE);

		int nIndex = pMenuPane->InsertItem(button, i);
		if (nIndex == -1)
		{
			pMenuPane->InsertItem(button);
		}
	}

	delete pStdCust;
}

BOOL CFrameImpl::IsCustomizePane(const CMFCPopupMenu* pMenuPopup) const
{
	CMFCPopupMenu* pPopupLevel2 = pMenuPopup->GetParentPopupMenu();

	if (pPopupLevel2 == NULL)
	{
		return FALSE;
	}

	CString strLabel;
	ENSURE(strLabel.LoadString(IDS_AFXBARRES_ADD_REMOVE_BTNS));

	CMFCToolBarMenuButton* pButton = pPopupLevel2->GetParentButton();
	if (pButton != NULL && pButton->m_strText.Find(strLabel) == -1)
	{
		return FALSE;
	}

	CMFCPopupMenu* pPopupLevel1 = pPopupLevel2->GetParentPopupMenu();

	if (pPopupLevel1 == NULL)
	{
		return FALSE;
	}

	if (pPopupLevel1->GetQuickCustomizeType() == CMFCPopupMenu::QUICK_CUSTOMIZE_ADDREMOVE)
	{
		return TRUE;
	}

	return FALSE;
}

void CFrameImpl::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
	if (m_bWindowPosChanging)
	{
		return;
	}

	ASSERT_VALID(m_pFrame);

	if (m_bIsOleInPlaceActive)
	{
		return;
	}

	if (((lpwndpos->flags & SWP_NOSIZE) == 0 ||(lpwndpos->flags & SWP_FRAMECHANGED)) && (m_pRibbonBar != NULL || IsOwnerDrawCaption()))
	{
		m_bWindowPosChanging = TRUE;

		BOOL oldState = FALSE;

		if (m_pDockManager != NULL)
		{
			oldState = m_pDockManager->m_bDisableRecalcLayout;
			m_pDockManager->m_bDisableRecalcLayout = TRUE;
		}

		m_bIsWindowRgn = CMFCVisualManager::GetInstance()->OnSetWindowRegion(m_pFrame, CSize(lpwndpos->cx, lpwndpos->cy));

		if (m_pDockManager != NULL)
		{
			m_pDockManager->m_bDisableRecalcLayout = oldState;
		}

		m_bWindowPosChanging = FALSE;
	}
}

BOOL CFrameImpl::OnNcPaint()
{
	ASSERT_VALID(m_pFrame);

	if (!IsOwnerDrawCaption() || g_bInSettingChange)
	{
		return FALSE;
	}

	return CMFCVisualManager::GetInstance()->OnNcPaint(m_pFrame, m_lstCaptionSysButtons, m_rectRedraw);
}

void CFrameImpl::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	ASSERT_VALID(m_pFrame);
	ENSURE(lpMMI != NULL);

	if ((m_pFrame->GetStyle() & WS_CAPTION) == 0 || (m_pFrame->GetStyle() & WS_BORDER) == 0)
	{
		CRect rectWindow;
		m_pFrame->GetWindowRect(&rectWindow);

		CRect rect(0, 0, 0, 0);

		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);

		if (GetMonitorInfo(MonitorFromPoint(rectWindow.CenterPoint(), MONITOR_DEFAULTTONEAREST), &mi))
		{
			CRect rectWork = mi.rcWork;
			CRect rectScreen = mi.rcMonitor;

			rect.left = rectWork.left - rectScreen.left;
			rect.top = rectWork.top - rectScreen.top;

			rect.right = rect.left + rectWork.Width();
			rect.bottom = rect.top + rectWork.Height();
		}
		else
		{
			::SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		}

		int nShellAutohideBars = afxGlobalData.GetShellAutohideBars();

		if (nShellAutohideBars & AFX_AUTOHIDE_BOTTOM)
		{
			rect.bottom -= 2;
		}

		if (nShellAutohideBars & AFX_AUTOHIDE_TOP)
		{
			rect.top += 2;
		}

		if (nShellAutohideBars & AFX_AUTOHIDE_RIGHT)
		{
			rect.right -= 2;
		}

		if (nShellAutohideBars & AFX_AUTOHIDE_LEFT)
		{
			rect.left += 2;
		}

		lpMMI->ptMaxPosition.x = rect.left;
		lpMMI->ptMaxPosition.y = rect.top;
		lpMMI->ptMaxSize.x = rect.Width();
		lpMMI->ptMaxSize.y = rect.Height();
	}
}

BOOL CFrameImpl::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	ASSERT_VALID(m_pFrame);
	ENSURE(lpncsp != NULL);

	BOOL bIsRibbonCaption = FALSE;

	if (m_pRibbonBar->GetSafeHwnd() != NULL && (m_pRibbonBar->IsWindowVisible() || !m_pFrame->IsWindowVisible()) && m_pRibbonBar->IsReplaceFrameCaption())
	{
		bIsRibbonCaption = TRUE;

		if (afxGlobalData.DwmIsCompositionEnabled())
		{
			lpncsp->rgrc[0].bottom -= GetSystemMetrics(SM_CYSIZEFRAME);
			lpncsp->rgrc[0].left += GetSystemMetrics(SM_CYSIZEFRAME);
			lpncsp->rgrc[0].right -= GetSystemMetrics(SM_CXSIZEFRAME);

			return TRUE;
		}
	}

	if (m_pRibbonStatusBar->GetSafeHwnd() != NULL && (m_pRibbonStatusBar->IsWindowVisible() || !m_pFrame->IsWindowVisible()))
	{
		ASSERT_VALID(m_pRibbonStatusBar);

		BOOL bBottomFrame = m_pRibbonStatusBar->m_bBottomFrame;

		if (IsOwnerDrawCaption() && !m_pFrame->IsZoomed())
		{
			m_pRibbonStatusBar->m_bBottomFrame = TRUE;
			lpncsp->rgrc[0].bottom += GetSystemMetrics(SM_CYSIZEFRAME);
		}
		else
		{
			m_pRibbonStatusBar->m_bBottomFrame = FALSE;
		}

		if (bBottomFrame != m_pRibbonStatusBar->m_bBottomFrame)
		{
			m_pRibbonStatusBar->RecalcLayout();
		}
	}

	if (!bIsRibbonCaption && IsOwnerDrawCaption())
	{
		lpncsp->rgrc[0].top += ::GetSystemMetrics(SM_CYCAPTION);
	}

	return(m_pFrame->GetStyle() & WS_MAXIMIZE) == WS_MAXIMIZE && (bIsRibbonCaption || IsOwnerDrawCaption());
}

void CFrameImpl::OnActivateApp(BOOL bActive)
{
	ASSERT_VALID(m_pFrame);

	if (m_bIsOleInPlaceActive)
	{
		return;
	}

	CMFCVisualManager::GetInstance()->OnActivateApp(m_pFrame, bActive);

	if (!bActive && m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible())
	{
		m_pRibbonBar->HideKeyTips();
		m_pRibbonBar->OnCancelMode();
	}
}

void CFrameImpl::OnSetText(LPCTSTR /*lpszText*/)
{
	ASSERT_VALID(m_pFrame);

	if (m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible() && m_pRibbonBar->IsReplaceFrameCaption())
	{
		m_pRibbonBar->RedrawWindow();
	}
}

BOOL CFrameImpl::OnNcActivate(BOOL bActive)
{
	ASSERT_VALID(m_pFrame);

	if (m_bIsOleInPlaceActive)
	{
		return FALSE;
	}

	if (!bActive && m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible())
	{
		m_pRibbonBar->HideKeyTips();
		m_pRibbonBar->DeactivateKeyboardFocus(FALSE);
	}

	if (!m_pFrame->IsWindowVisible())
	{
		return FALSE;
	}

	BOOL bRes = CMFCVisualManager::GetInstance()->OnNcActivate(m_pFrame, bActive);
	BOOL bFrameIsRedrawn = FALSE;

	if (bRes && m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible() && m_pRibbonBar->IsReplaceFrameCaption())
	{
		m_pRibbonBar->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

		m_pFrame->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);

		bFrameIsRedrawn = TRUE;
	}

	if (m_pRibbonStatusBar->GetSafeHwnd() != NULL)
	{
		m_pRibbonStatusBar->RedrawWindow();
	}

	if (!bFrameIsRedrawn && IsOwnerDrawCaption())
	{
		m_pFrame->RedrawWindow(CRect(0, 0, 0, 0), NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);
	}

	return bRes && !afxGlobalData.DwmIsCompositionEnabled();
}

CRect CFrameImpl::GetCaptionRect()
{
	ASSERT_VALID(m_pFrame);

	CSize szSystemBorder(::GetSystemMetrics(SM_CXSIZEFRAME), ::GetSystemMetrics(SM_CYSIZEFRAME));

	if (m_pFrame->IsIconic() || (m_pFrame->GetStyle() & WS_MAXIMIZE) == WS_MAXIMIZE)
	{
		szSystemBorder = CSize(0, 0);
	}

	CRect rectWnd;
	m_pFrame->GetWindowRect(&rectWnd);

	m_pFrame->ScreenToClient(&rectWnd);

	int cyOffset = szSystemBorder.cy;
	if (!m_pFrame->IsIconic())
	{
		cyOffset += ::GetSystemMetrics(SM_CYCAPTION);
	}

	rectWnd.OffsetRect(szSystemBorder.cx, cyOffset);

	CRect rectCaption(rectWnd.left + szSystemBorder.cx, rectWnd.top + szSystemBorder.cy,
		rectWnd.right - szSystemBorder.cx, rectWnd.top + szSystemBorder.cy + ::GetSystemMetrics(SM_CYCAPTION));

	if (m_pFrame->IsIconic())
	{
		rectCaption.top += ::GetSystemMetrics(SM_CYSIZEFRAME);
		rectCaption.right -= ::GetSystemMetrics(SM_CXSIZEFRAME);
	}

	return rectCaption;
}

void CFrameImpl::UpdateCaption()
{
	ASSERT_VALID(m_pFrame);

	if (!IsOwnerDrawCaption())
	{
		return;
	}

	if (m_lstCaptionSysButtons.IsEmpty())
	{
		// Create caption buttons:
		const DWORD dwStyle = m_pFrame->GetStyle();

		HMENU hSysMenu = NULL;
		CMenu* pSysMenu = m_pFrame->GetSystemMenu(FALSE);

		if (pSysMenu != NULL && ::IsMenu(pSysMenu->m_hMenu))
		{
			hSysMenu = pSysMenu->GetSafeHmenu();
			if (!::IsMenu(hSysMenu) ||(m_pFrame->GetStyle() & WS_SYSMENU) == 0)
			{
				hSysMenu = NULL;
			}
		}

		if (hSysMenu != NULL)
		{
			m_lstCaptionSysButtons.AddTail(new CMFCCaptionButtonEx(AFX_HTCLOSE));

			if ((dwStyle & WS_MAXIMIZEBOX) == WS_MAXIMIZEBOX)
			{
				m_lstCaptionSysButtons.AddTail(new CMFCCaptionButtonEx(AFX_HTMAXBUTTON));
			}

			if ((dwStyle & WS_MINIMIZEBOX) == WS_MINIMIZEBOX)
			{
				m_lstCaptionSysButtons.AddTail(new CMFCCaptionButtonEx(AFX_HTMINBUTTON));
			}
		}
	}

	CRect rectCaption = GetCaptionRect();

	CSize sizeButton = CMFCVisualManager::GetInstance()->GetNcBtnSize(FALSE);
	sizeButton.cy = min(sizeButton.cy, rectCaption.Height() - 2);

	int x = rectCaption.right - sizeButton.cx;
	int y = rectCaption.top + max(0, (rectCaption.Height() - sizeButton.cy) / 2);

	for (POSITION pos = m_lstCaptionSysButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButtonEx* pButton = (CMFCCaptionButtonEx*)m_lstCaptionSysButtons.GetNext(pos);
		ASSERT_VALID(pButton);

		pButton->SetRect(CRect(CPoint(x, y), sizeButton));

		x -= sizeButton.cx;
	}

	m_pFrame->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);
}

void __stdcall CFrameImpl::AddFrame(CFrameWnd* pFrame)
{
	ASSERT_VALID(pFrame);

	for (POSITION pos = m_lstFrames.GetHeadPosition(); pos != NULL;)
	{
		CFrameWnd* pListFrame = m_lstFrames.GetNext(pos);

		if (pListFrame->GetSafeHwnd() == pFrame->GetSafeHwnd())
		{
			return;
		}
	}

	m_lstFrames.AddTail(pFrame);
}

void __stdcall CFrameImpl::RemoveFrame(CFrameWnd* pFrame)
{
	for (POSITION pos = m_lstFrames.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;

		CFrameWnd* pListFrame = m_lstFrames.GetNext(pos);

		if (pListFrame->GetSafeHwnd() == pFrame->GetSafeHwnd())
		{
			m_lstFrames.RemoveAt(posSave);
			return;
		}
	}
}

UINT CFrameImpl::OnNcHitTest(CPoint point)
{
	ASSERT_VALID(m_pFrame);

	if (m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible() && m_pRibbonBar->IsReplaceFrameCaption() && m_pRibbonBar->IsTransparentCaption() && afxGlobalData.DwmIsCompositionEnabled())
	{
		return(UINT) afxGlobalData.DwmDefWindowProc(m_pFrame->GetSafeHwnd(), WM_NCHITTEST, 0, MAKELPARAM(point.x, point.y));
	}

	if (!IsOwnerDrawCaption())
	{
		return HTNOWHERE;
	}

	m_pFrame->ScreenToClient(&point);

	const CSize szSystemBorder(::GetSystemMetrics(SM_CXSIZEFRAME), ::GetSystemMetrics(SM_CYSIZEFRAME));

	int cyOffset = szSystemBorder.cy;
	if (!m_pFrame->IsIconic())
	{
		cyOffset += ::GetSystemMetrics(SM_CYCAPTION);
	}

	point.Offset(szSystemBorder.cx, cyOffset);

	for (POSITION pos = m_lstCaptionSysButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButtonEx* pButton = (CMFCCaptionButtonEx*)m_lstCaptionSysButtons.GetNext(pos);
		ASSERT_VALID(pButton);

		if (pButton->GetRect().PtInRect(point))
		{
			return pButton->m_nHit;
		}
	}

	CRect rectCaption = GetCaptionRect();
	if (rectCaption.PtInRect(point))
	{
		CRect rectSysMenu = rectCaption;
		rectSysMenu.right = rectSysMenu.left + ::GetSystemMetrics(SM_CYCAPTION) + 2 * szSystemBorder.cx;

		return rectSysMenu.PtInRect(point) ? HTSYSMENU : HTCAPTION;
	}

	return HTNOWHERE;
}

void CFrameImpl::OnNcMouseMove(UINT /*nHitTest*/, CPoint point)
{
	if (!IsOwnerDrawCaption())
	{
		return;
	}

	if (!m_bCaptured)
	{
		OnTrackCaptionButtons(point);
	}
}

void CFrameImpl::OnLButtonDown(CPoint /*point*/)
{
	if (m_nHotSysButton == HTNOWHERE)
	{
		return;
	}

	CMFCCaptionButtonEx* pBtn = GetSysButton(m_nHotSysButton);
	if (pBtn != NULL)
	{
		m_nHitSysButton = m_nHotSysButton;
		pBtn->m_bPushed = TRUE;
		RedrawCaptionButton(pBtn);
	}
}

void CFrameImpl::OnLButtonUp(CPoint /*point*/)
{
	ASSERT_VALID(m_pFrame);

	if (!IsOwnerDrawCaption())
	{
		return;
	}

	if (m_bCaptured)
	{
		return;
	}

	switch (m_nHitSysButton)
	{
	case AFX_HTCLOSE:
	case AFX_HTMAXBUTTON:
	case AFX_HTMINBUTTON:
		{
			UINT nHot = m_nHotSysButton;
			UINT nHit = m_nHitSysButton;

			StopCaptionButtonsTracking();

			if (nHot == nHit)
			{
				UINT nSysCmd = 0;

				switch (nHot)
				{
				case AFX_HTCLOSE:
					nSysCmd = SC_CLOSE;
					break;

				case AFX_HTMAXBUTTON:
					nSysCmd =
						(m_pFrame->GetStyle() & WS_MAXIMIZE) == WS_MAXIMIZE ? SC_RESTORE : SC_MAXIMIZE;
					break;

				case AFX_HTMINBUTTON:
					nSysCmd = m_pFrame->IsIconic() ? SC_RESTORE : SC_MINIMIZE;
					break;
				}

				m_pFrame->PostMessage(WM_SYSCOMMAND, nSysCmd);
			}
		}
	}
}

void CFrameImpl::OnMouseMove(CPoint point)
{
	if (!IsOwnerDrawCaption())
	{
		return;
	}

	CPoint ptScreen = point;
	m_pFrame->ClientToScreen(&ptScreen);

	OnTrackCaptionButtons(ptScreen);
}

CMFCCaptionButtonEx* CFrameImpl::GetSysButton(UINT nHit)
{
	for (POSITION pos = m_lstCaptionSysButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButtonEx* pButton = (CMFCCaptionButtonEx*)
			m_lstCaptionSysButtons.GetNext(pos);
		ASSERT_VALID(pButton);

		if (pButton->m_nHit == nHit)
		{
			return pButton;
		}
	}

	return NULL;
}

void CFrameImpl::SetHighlightedSysButton(UINT nHit)
{
	BOOL bRedraw = FALSE;

	for (POSITION pos = m_lstCaptionSysButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButtonEx* pButton = (CMFCCaptionButtonEx*)
			m_lstCaptionSysButtons.GetNext(pos);
		ASSERT_VALID(pButton);

		if (pButton->m_nHit == nHit)
		{
			if (pButton->m_bFocused)
			{
				return;
			}

			pButton->m_bFocused = TRUE;
			bRedraw = TRUE;
		}
	}
}

void CFrameImpl::OnTrackCaptionButtons(CPoint point)
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	UINT nHot = m_nHotSysButton;
	CMFCCaptionButtonEx* pBtn = GetSysButton(OnNcHitTest(point));

	if (pBtn != NULL && pBtn->m_bEnabled)
	{
		m_nHotSysButton = pBtn->GetHit();
		pBtn->m_bFocused = TRUE;
	}
	else
	{
		m_nHotSysButton = HTNOWHERE;
	}

	if (m_nHotSysButton != nHot)
	{
		RedrawCaptionButton(pBtn);

		CMFCCaptionButtonEx* pBtnOld = GetSysButton(nHot);
		if (pBtnOld != NULL)
		{
			pBtnOld->m_bFocused = FALSE;
			RedrawCaptionButton(pBtnOld);
		}
	}

	if (m_nHitSysButton == HTNOWHERE)
	{
		if (nHot != HTNOWHERE && m_nHotSysButton == HTNOWHERE)
		{
			::ReleaseCapture();
		}
		else if (nHot == HTNOWHERE && m_nHotSysButton != HTNOWHERE)
		{
			m_pFrame->SetCapture();
		}
	}
}

void CFrameImpl::StopCaptionButtonsTracking()
{
	if (m_nHitSysButton != HTNOWHERE)
	{
		CMFCCaptionButtonEx* pBtn = GetSysButton(m_nHitSysButton);
		m_nHitSysButton = HTNOWHERE;

		ReleaseCapture();
		if (pBtn != NULL)
		{
			pBtn->m_bPushed = FALSE;
			RedrawCaptionButton(pBtn);
		}
	}

	if (m_nHotSysButton != HTNOWHERE)
	{
		CMFCCaptionButtonEx* pBtn = GetSysButton(m_nHotSysButton);
		m_nHotSysButton = HTNOWHERE;

		ReleaseCapture();
		if (pBtn != NULL)
		{
			pBtn->m_bFocused = FALSE;
			RedrawCaptionButton(pBtn);
		}
	}
}

void CFrameImpl::RedrawCaptionButton(CMFCCaptionButtonEx* pBtn)
{
	ASSERT_VALID(m_pFrame);

	if (pBtn == NULL)
	{
		return;
	}

	ASSERT_VALID(pBtn);

	m_rectRedraw = pBtn->GetRect();
	m_pFrame->SendMessage(WM_NCPAINT);
	m_rectRedraw.SetRectEmpty();

	m_pFrame->UpdateWindow();
}

void CFrameImpl::OnChangeVisualManager()
{
	ASSERT_VALID(m_pFrame);

	BOOL bIsRibbonCaption = FALSE;

	if (m_pRibbonBar != NULL && (m_pRibbonBar->IsWindowVisible() || !m_pFrame->IsWindowVisible()) && m_pRibbonBar->IsReplaceFrameCaption())
	{
		bIsRibbonCaption = TRUE;
		m_pRibbonBar->RecalcLayout();

		if (afxGlobalData.DwmIsCompositionEnabled())
		{
			return;
		}
	}

	CRect rectWindow;
	m_pFrame->GetWindowRect(rectWindow);

	BOOL bZoomed = m_pFrame->IsZoomed();

	if (bIsRibbonCaption || IsOwnerDrawCaption())
	{
		BOOL bChangeBorder = FALSE;

		if ((m_pFrame->GetStyle() & WS_BORDER) == WS_BORDER && m_bHasBorder && !m_bIsMDIChildFrame)
		{
			bChangeBorder = TRUE;
			m_bWindowPosChanging = TRUE;
			m_pFrame->ModifyStyle(WS_BORDER, 0, SWP_FRAMECHANGED);
			m_bWindowPosChanging = FALSE;
		}

		m_bIsWindowRgn = CMFCVisualManager::GetInstance()->OnSetWindowRegion(m_pFrame, rectWindow.Size());

		if (bZoomed && bChangeBorder && !m_bIsMDIChildFrame)
		{
			MINMAXINFO mi;
			ZeroMemory(&mi, sizeof(MINMAXINFO));
			OnGetMinMaxInfo(&mi);

			rectWindow = CRect(mi.ptMaxPosition, CSize(mi.ptMaxSize.x, mi.ptMaxSize.y));

			m_pFrame->SetWindowPos(NULL, rectWindow.left, rectWindow.top, rectWindow.Width(), rectWindow.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}
	else
	{
		BOOL bChangeBorder = FALSE;

		if ((m_pFrame->GetStyle() & WS_BORDER) == 0 && m_bHasBorder && !m_bIsMDIChildFrame)
		{
			bChangeBorder = TRUE;
			m_bWindowPosChanging = TRUE;
			m_pFrame->ModifyStyle(0, WS_BORDER, SWP_FRAMECHANGED);
			m_bWindowPosChanging = FALSE;
		}

		if (m_bIsWindowRgn)
		{
			m_bIsWindowRgn = FALSE;
			m_pFrame->SetWindowRgn(NULL, TRUE);
		}

		if (bZoomed && bChangeBorder && !m_bIsMDIChildFrame)
		{
			NCCALCSIZE_PARAMS params;
			ZeroMemory(&params, sizeof(NCCALCSIZE_PARAMS));
			params.rgrc[0].left   = rectWindow.left;
			params.rgrc[0].top    = rectWindow.top;
			params.rgrc[0].right  = rectWindow.right;
			params.rgrc[0].bottom = rectWindow.bottom;

			m_pFrame->CalcWindowRect(&params.rgrc[0], CFrameWnd::adjustBorder);

			if ((m_pFrame->GetStyle() & WS_CAPTION) == WS_CAPTION)
			{
				params.rgrc[0].top += ::GetSystemMetrics(SM_CYCAPTION);
			}

			m_pFrame->SetWindowPos(NULL, params.rgrc[0].left, params.rgrc[0].top,
				params.rgrc[0].right - params.rgrc[0].left, params.rgrc[0].bottom - params.rgrc[0].top, SWP_NOACTIVATE | SWP_NOZORDER);
		}
		else
		{
			m_pFrame->SetWindowPos(NULL, -1, -1, rectWindow.Width() + 1, rectWindow.Height() + 1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
			m_pFrame->SetWindowPos(NULL, -1, -1, rectWindow.Width(), rectWindow.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}

	UpdateCaption();
}

BOOL CFrameImpl::IsPrintPreview()
{
	return m_pDockManager != NULL && m_pDockManager->IsPrintPreviewValid();
}

void CFrameImpl::OnDWMCompositionChanged()
{
	if (m_pRibbonBar != NULL && m_pRibbonBar->IsWindowVisible() && m_pRibbonBar->IsReplaceFrameCaption())
	{
		m_pRibbonBar->DWMCompositionChanged();
	}
}



