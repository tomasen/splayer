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
#include <dde.h>        // for DDE execute shell requests



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CRect for creating windows with the default position/size

const AFX_DATADEF CRect CFrameWnd::rectDefault(
	CW_USEDEFAULT, CW_USEDEFAULT,
	0 /* 2*CW_USEDEFAULT */, 0 /* 2*CW_USEDEFAULT */);

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd

BEGIN_MESSAGE_MAP(CFrameWnd, CWnd)
	//{{AFX_MSG_MAP(CFrameWnd)
	ON_WM_INITMENU()
	ON_WM_INITMENUPOPUP()
	ON_WM_MENUSELECT()
	ON_WM_MENUCHAR()
	ON_MESSAGE(WM_POPMESSAGESTRING, &CFrameWnd::OnPopMessageString)
	ON_MESSAGE(WM_SETMESSAGESTRING, &CFrameWnd::OnSetMessageString)
	ON_MESSAGE(WM_HELPPROMPTADDR, &CFrameWnd::OnHelpPromptAddr)
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, CFrameWnd::OnIdleUpdateCmdUI)
	ON_WM_ENTERIDLE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_SYSCOMMAND()
	ON_WM_DROPFILES()
	ON_WM_QUERYENDSESSION()
	ON_WM_ENDSESSION()
	ON_WM_SETCURSOR()
	ON_WM_ENABLE()
	// OLE palette support
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_MESSAGE(WM_COMMANDHELP, &CFrameWnd::OnCommandHelp)
	ON_MESSAGE(WM_HELPHITTEST, &CFrameWnd::OnHelpHitTest)
	ON_MESSAGE(WM_ACTIVATETOPLEVEL, &CFrameWnd::OnActivateTopLevel)
	// turning on and off standard frame gadgetry
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, &CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_STATUS_BAR, &CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, &CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, &CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REBAR, &CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_REBAR, &CFrameWnd::OnBarCheck)
	// turning on and off standard mode indicators
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CAPS, &CFrameWnd::OnUpdateKeyIndicator)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_NUM, &CFrameWnd::OnUpdateKeyIndicator)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SCRL, &CFrameWnd::OnUpdateKeyIndicator)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_KANA, &CFrameWnd::OnUpdateKeyIndicator)
	// standard help handling
	ON_UPDATE_COMMAND_UI(ID_CONTEXT_HELP, &CFrameWnd::OnUpdateContextHelp)
	// toolbar "tooltip" notification
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CFrameWnd::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CFrameWnd::OnToolTipText)
	ON_NOTIFY_EX_RANGE(RBN_CHEVRONPUSHED, 0, 0xFFFF, &CFrameWnd::OnChevronPushed)
	//}}AFX_MSG_MAP
	// message handling for standard DDE commands
	ON_MESSAGE(WM_DDE_INITIATE, &CFrameWnd::OnDDEInitiate)
	ON_MESSAGE(WM_DDE_EXECUTE, &CFrameWnd::OnDDEExecute)
	ON_MESSAGE(WM_DDE_TERMINATE, &CFrameWnd::OnDDETerminate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd construction/destruction

CFrameWnd::CFrameWnd()
{
	ASSERT(m_hWnd == NULL);

	m_nWindow = -1;                 // unknown window ID
	m_bAutoMenuEnable = TRUE;       // auto enable on by default
	m_lpfnCloseProc = NULL;
	m_hMenuDefault = NULL;
	m_hAccelTable = NULL;
	m_nIDHelp = 0;
	m_nIDTracking = 0;
	m_nIDLastMessage = 0;
	m_pViewActive = NULL;

	m_cModalStack = 0;              // initialize modality support
	m_phWndDisable = NULL;
	m_pNotifyHook = NULL;
	m_hMenuAlt = NULL;
	m_nIdleFlags = 0;               // no idle work at start
	m_rectBorder.SetRectEmpty();

	m_bHelpMode = HELP_INACTIVE;    // not in Shift+F1 help mode
	m_dwPromptContext = 0;

	m_pNextFrameWnd = NULL;         // not in list yet

	m_bInRecalcLayout = FALSE;
	m_pFloatingFrameClass = NULL;
	m_nShowDelay = -1;              // no delay pending

	m_dwMenuBarVisibility = AFX_MBV_KEEPVISIBLE;
	m_dwMenuBarState = AFX_MBS_VISIBLE;
	m_hMenu = NULL;
	m_bTempShowMenu = FALSE;
	m_bMouseHitMenu = FALSE;

	AddFrameWnd();
}

CFrameWnd::~CFrameWnd()
{
	AFX_BEGIN_DESTRUCTOR

		RemoveFrameWnd();

		// If we're the current routing frame, pop the routing frame stack
		_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
		while (pThreadState->m_pRoutingFrame == this)
			pThreadState->m_pPushRoutingFrame->Pop();

		if (m_phWndDisable != NULL)
			delete[] (void*)m_phWndDisable;

	AFX_END_DESTRUCTOR
}

void CFrameWnd::AddFrameWnd()
{
	// hook it into the CFrameWnd list
	AFX_MODULE_THREAD_STATE* pState = _AFX_CMDTARGET_GETSTATE()->m_thread;
	pState->m_frameList.AddHead(this);
}

void CFrameWnd::RemoveFrameWnd()
{
	// remove this frame window from the list of frame windows
	AFX_MODULE_THREAD_STATE* pState = _AFX_CMDTARGET_GETSTATE()->m_thread;
	pState->m_frameList.Remove(this);
}

/////////////////////////////////////////////////////////////////////////////
// Special processing etc

BOOL CFrameWnd::LoadAccelTable(LPCTSTR lpszResourceName)
{
	ASSERT(m_hAccelTable == NULL);  // only do once
	ASSERT(lpszResourceName != NULL);

	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, ATL_RT_ACCELERATOR);
	m_hAccelTable = ::LoadAccelerators(hInst, lpszResourceName);
	return (m_hAccelTable != NULL);
}

HACCEL CFrameWnd::GetDefaultAccelerator()
{
	// use document specific accelerator table over m_hAccelTable
	HACCEL hAccelTable = m_hAccelTable;
	HACCEL hAccel;
	CDocument* pDoc = GetActiveDocument();
	if (pDoc != NULL && (hAccel = pDoc->GetDefaultAccelerator()) != NULL)
		hAccelTable = hAccel;

	return hAccelTable;
}

BOOL CFrameWnd::PreTranslateMessage(MSG* pMsg)
{
	ENSURE_ARG(pMsg != NULL);
	// check for special cancel modes for combo boxes
	if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_NCLBUTTONDOWN)
		AfxCancelModes(pMsg->hwnd);    // filter clicks

	// check for key strokes that may affect the menu bar state
	if ((m_dwMenuBarVisibility & AFX_MBV_DISPLAYONF10) &&
		 pMsg->message == WM_SYSKEYUP &&
		 pMsg->wParam == VK_F10)
	{
		SetMenuBarState(AFX_MBS_VISIBLE);
	}

	if (m_dwMenuBarVisibility & AFX_MBV_DISPLAYONFOCUS)
	{
		if (pMsg->message == WM_SYSKEYUP &&
			pMsg->wParam == VK_MENU)
		{
			SetMenuBarState(m_dwMenuBarState == AFX_MBS_VISIBLE ? AFX_MBS_HIDDEN : AFX_MBS_VISIBLE);
		}
		else if (pMsg->message == WM_SYSCHAR &&
				m_dwMenuBarState == AFX_MBS_HIDDEN)
		{
			// temporarily show the menu bar to enable menu access keys
			SetMenuBarState(AFX_MBS_VISIBLE);
			m_bTempShowMenu = TRUE;
		}
	}

	if ((m_dwMenuBarVisibility & AFX_MBV_KEEPVISIBLE) == 0)
	{
		if ( (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) ||
			 (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_RBUTTONDOWN) ||
			((pMsg->message == WM_NCLBUTTONDOWN || pMsg->message == WM_NCRBUTTONDOWN) && pMsg->wParam != HTMENU))
		{
			SetMenuBarState(AFX_MBS_HIDDEN);
		}
	}

	if (pMsg->message == WM_NCLBUTTONDOWN || pMsg->message == WM_NCRBUTTONDOWN)
	{
		m_bMouseHitMenu = (pMsg->wParam == HTMENU);
	}
	else if (pMsg->message == WM_NCLBUTTONUP || pMsg->message == WM_NCRBUTTONUP ||
		pMsg->message == WM_LBUTTONUP || pMsg->message == WM_RBUTTONUP)
	{
		m_bMouseHitMenu = FALSE;
	}

	// allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

#ifndef _AFX_NO_OLE_SUPPORT
	// allow hook to consume message
	if (m_pNotifyHook != NULL && m_pNotifyHook->OnPreTranslateMessage(pMsg))
		return TRUE;
#endif

	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		// finally, translate the message
		HACCEL hAccel = GetDefaultAccelerator();
		return hAccel != NULL &&  ::TranslateAccelerator(m_hWnd, hAccel, pMsg);
	}
	return FALSE;
}

void CFrameWnd::PostNcDestroy()
{
	// default for frame windows is to allocate them on the heap
	//  the default post-cleanup is to 'delete this'.
	// never explicitly call 'delete' on a CFrameWnd, use DestroyWindow instead
	delete this;
}

void CFrameWnd::OnPaletteChanged(CWnd* pFocusWnd)
{
	CWnd::OnPaletteChanged(pFocusWnd);
#ifndef _AFX_NO_OLE_SUPPORT
	if (m_pNotifyHook != NULL)
		m_pNotifyHook->OnPaletteChanged(pFocusWnd);
#endif
}

BOOL CFrameWnd::OnQueryNewPalette()
{
#ifndef _AFX_NO_OLE_SUPPORT
	if (m_pNotifyHook != NULL && m_pNotifyHook->OnQueryNewPalette())
		return TRUE;
#endif
	return CWnd::OnQueryNewPalette();
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd support for context sensitive help.

void CFrameWnd::ExitHelpMode()
{
	// if not in help mode currently, this is a no-op
	if (!m_bHelpMode)
		return;

	// only post new WM_EXITHELPMODE message if one doesn't already exist
	//  in the queue.
	MSG msg;
	if (!::PeekMessage(&msg, m_hWnd, WM_EXITHELPMODE, WM_EXITHELPMODE,
		PM_REMOVE|PM_NOYIELD))
	{
		VERIFY(::PostMessage(m_hWnd, WM_EXITHELPMODE, 0, 0));
	}

	// release capture if this window has it
	if (::GetCapture() == m_hWnd)
		ReleaseCapture();

	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	ENSURE_VALID(pFrameWnd);
	pFrameWnd->m_bHelpMode = m_bHelpMode = HELP_INACTIVE;
	PostMessage(WM_KICKIDLE);   // trigger idle update
}

BOOL CFrameWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	ENSURE_VALID(pFrameWnd);
	if (pFrameWnd->m_bHelpMode)
	{
		SetCursor(afxData.hcurHelp);
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CFrameWnd::OnCommandHelp(WPARAM, LPARAM lParam)
{
	if (lParam == 0)
	{
		if (IsTracking())
			lParam = HID_BASE_COMMAND+m_nIDTracking;
		else
			lParam = HID_BASE_RESOURCE+m_nIDHelp;
	}
	if (lParam != 0)
	{
		CWinApp* pApp = AfxGetApp();
		if (pApp != NULL)
			pApp->WinHelpInternal(lParam);
		return TRUE;
	}
	return FALSE;
}

LRESULT CFrameWnd::OnHelpHitTest(WPARAM, LPARAM)
{
	if (m_nIDHelp != 0)
		return HID_BASE_RESOURCE+m_nIDHelp;
	else
		return 0;
}

BOOL CFrameWnd::OnCommand(WPARAM wParam, LPARAM lParam)
	// return TRUE if command invocation was attempted
{
	HWND hWndCtrl = (HWND)lParam;
	UINT nID = LOWORD(wParam);

	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	ENSURE_VALID(pFrameWnd);
	if (pFrameWnd->m_bHelpMode && hWndCtrl == NULL &&
		nID != ID_HELP && nID != ID_DEFAULT_HELP && nID != ID_CONTEXT_HELP)
	{
		// route as help
		if (!SendMessage(WM_COMMANDHELP, 0, HID_BASE_COMMAND+nID))
			SendMessage(WM_COMMAND, ID_DEFAULT_HELP);
		return TRUE;
	}

	// route as normal command
	return CWnd::OnCommand(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd support for modality

BOOL AFXAPI AfxIsDescendant(HWND hWndParent, HWND hWndChild)
	// helper for detecting whether child descendent of parent
	//  (works with owned popups as well)
{
	ASSERT(::IsWindow(hWndParent));
	ASSERT(::IsWindow(hWndChild));

	do
	{
		if (hWndParent == hWndChild)
			return TRUE;

		hWndChild = AfxGetParentOwner(hWndChild);
	} while (hWndChild != NULL);

	return FALSE;
}

void CFrameWnd::BeginModalState()
{
	ASSERT(m_hWnd != NULL);
	ASSERT(::IsWindow(m_hWnd));

	// allow stacking, but don't do anything
	if (++m_cModalStack > 1)
		return;

	// determine top-level parent, since that is the true parent of any
	//  modeless windows anyway...
	CWnd* pParent = EnsureTopLevelParent();

	CArray<HWND,HWND> arrDisabledWnds;	
	// disable all windows connected to this frame (and add them to the list)
	HWND hWnd = ::GetWindow(::GetDesktopWindow(), GW_CHILD);
	while (hWnd != NULL)
	{
		if (::IsWindowEnabled(hWnd) &&
			CWnd::FromHandlePermanent(hWnd) != NULL &&
			AfxIsDescendant(pParent->m_hWnd, hWnd) &&
			::SendMessage(hWnd, WM_DISABLEMODAL, 0, 0) == 0)
		{
			::EnableWindow(hWnd, FALSE);			
			arrDisabledWnds.Add(hWnd);				
		}
		hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
	}
	INT_PTR nCount = arrDisabledWnds.GetCount();
	if (nCount == 0)
	{
		return;
	}
	ENSURE(nCount > 0);
	m_phWndDisable = new HWND[nCount+1];
	// Terminate the list with a NULL
	m_phWndDisable[nCount] = NULL;
	ENSURE(arrDisabledWnds.GetData()!=NULL);
	// Copy the HWNDs from local array to m_phWndDisable, to be enabled later.
	Checked::memcpy_s(m_phWndDisable,sizeof(HWND)*nCount,arrDisabledWnds.GetData(),sizeof(HWND)*nCount);
}

void CFrameWnd::EndModalState()
{
	// pop one off the stack (don't undo modalness unless stack is down to zero)
	if (m_cModalStack == 0 || --m_cModalStack > 0 || m_phWndDisable == NULL)
		return;

	// enable all the windows disabled by BeginModalState
	ASSERT(m_phWndDisable != NULL);
	UINT nIndex = 0;
	while (m_phWndDisable[nIndex] != NULL)
	{
		ASSERT(m_phWndDisable[nIndex] != NULL);
		if (::IsWindow(m_phWndDisable[nIndex]))
			::EnableWindow(m_phWndDisable[nIndex], TRUE);
		++nIndex;
	}
	delete[] (void*)m_phWndDisable;
	m_phWndDisable = NULL;
}

void CFrameWnd::ShowOwnedWindows(BOOL bShow)
{
	// walk through all top-level windows
	HWND hWnd = ::GetWindow(::GetDesktopWindow(), GW_CHILD);
	while (hWnd != NULL)
	{
		CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
		if (pWnd != NULL && m_hWnd != hWnd && AfxIsDescendant(m_hWnd, hWnd))
		{
			DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
			if (!bShow && (dwStyle & (WS_VISIBLE|WS_DISABLED)) == WS_VISIBLE)
			{
				::ShowWindow(hWnd, SW_HIDE);
				pWnd->m_nFlags |= WF_TEMPHIDE;
			}
			// don't show temporarily hidden windows if we're in print preview mode
			else if (bShow && (dwStyle & (WS_VISIBLE|WS_DISABLED)) == 0 &&
				(pWnd->m_nFlags & WF_TEMPHIDE) && !m_lpfnCloseProc)
			{
				::ShowWindow(hWnd, SW_SHOWNOACTIVATE);
				pWnd->m_nFlags &= ~WF_TEMPHIDE;
			}
		}
		hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
	}
}

void CFrameWnd::OnEnable(BOOL bEnable)
{
	if (bEnable && (m_nFlags & WF_STAYDISABLED))
	{
		// Work around for MAPI support. This makes sure the main window
		// remains disabled even when the mail system is booting.
		EnableWindow(FALSE);
		::SetFocus(NULL);
		return;
	}

	// only for top-level (and non-owned) windows of our process
	CWnd *pParentWnd = GetParent();
	if (pParentWnd != NULL)
	{
#ifndef _AFX_NO_OLE_SUPPORT
		DWORD dwProcessId = 0;
		::GetWindowThreadProcessId(pParentWnd->GetSafeHwnd(), &dwProcessId);
		if (::GetCurrentProcessId() == dwProcessId)
#endif
			return;
	}

	// this causes modal dialogs to be "truly modal"
	if (!bEnable && !InModalState())
	{
		ASSERT((m_nFlags & WF_MODALDISABLE) == 0);
		m_nFlags |= WF_MODALDISABLE;
		BeginModalState();
	}
	else if (bEnable && (m_nFlags & WF_MODALDISABLE))
	{
		m_nFlags &= ~WF_MODALDISABLE;
		EndModalState();

		// cause normal focus logic to kick in
		if (::GetActiveWindow() == m_hWnd)
			SendMessage(WM_ACTIVATE, WA_ACTIVE);
	}

	// force WM_NCACTIVATE because Windows may think it is unecessary
	if (bEnable && (m_nFlags & WF_STAYACTIVE))
		SendMessage(WM_NCACTIVATE, TRUE);
	// force WM_NCACTIVATE for floating windows too
	NotifyFloatingWindows(bEnable ? FS_ENABLE : FS_DISABLE);
}

void CFrameWnd::NotifyFloatingWindows(DWORD dwFlags)
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	// get top level parent frame window first unless this is a child window
	CFrameWnd* pParent = (GetStyle() & WS_CHILD) ? this : GetTopLevelFrame();
	ENSURE(pParent != NULL);
	if (dwFlags & (FS_DEACTIVATE|FS_ACTIVATE))
	{
		// update parent window activation state
		BOOL bActivate = !(dwFlags & FS_DEACTIVATE);
		BOOL bEnabled = pParent->IsWindowEnabled();

		if (bActivate && bEnabled && pParent != this)
		{
			// Excel will try to Activate itself when it receives a
			// WM_NCACTIVATE so we need to keep it from doing that here.
			m_nFlags |= WF_KEEPMINIACTIVE;
			pParent->SendMessage(WM_NCACTIVATE, TRUE);
			m_nFlags &= ~WF_KEEPMINIACTIVE;
		}
		else
		{
			pParent->SendMessage(WM_NCACTIVATE, FALSE);
		}
	}

	// then update the state of all floating windows owned by the parent
	HWND hWnd = ::GetWindow(::GetDesktopWindow(), GW_CHILD);
	while (hWnd != NULL)
	{
		if (AfxIsDescendant(pParent->m_hWnd, hWnd))
			::SendMessage(hWnd, WM_FLOATSTATUS, dwFlags, 0);
		hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd second phase creation

BOOL CFrameWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (cs.lpszClass == NULL)
	{
		VERIFY(AfxDeferRegisterClass(AFX_WNDFRAMEORVIEW_REG));
		cs.lpszClass = _afxWndFrameOrView;  // COLOR_WINDOW background
	}

	if (cs.style & FWS_ADDTOTITLE)
		cs.style |= FWS_PREFIXTITLE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;

	return TRUE;
}

BOOL CFrameWnd::Create(LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName,
	DWORD dwStyle,
	const RECT& rect,
	CWnd* pParentWnd,
	LPCTSTR lpszMenuName,
	DWORD dwExStyle,
	CCreateContext* pContext)
{
	HMENU hMenu = NULL;
	if (lpszMenuName != NULL)
	{
		// load in a menu that will get destroyed when window gets destroyed
		HINSTANCE hInst = AfxFindResourceHandle(lpszMenuName, ATL_RT_MENU);
		if ((hMenu = ::LoadMenu(hInst, lpszMenuName)) == NULL)
		{
			TRACE(traceAppMsg, 0, "Warning: failed to load menu for CFrameWnd.\n");
			PostNcDestroy();            // perhaps delete the C++ object
			return FALSE;
		}
	}

	m_strTitle = lpszWindowName;    // save title for later

	if (!CreateEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), hMenu, (LPVOID)pContext))
	{
		TRACE(traceAppMsg, 0, "Warning: failed to create CFrameWnd.\n");
		if (hMenu != NULL)
			DestroyMenu(hMenu);
		return FALSE;
	}

	return TRUE;
}

CWnd* CFrameWnd::CreateView(CCreateContext* pContext, UINT nID)
{
	ASSERT(m_hWnd != NULL);
	ASSERT(::IsWindow(m_hWnd));
	ENSURE_ARG(pContext != NULL);
	ENSURE_ARG(pContext->m_pNewViewClass != NULL);

	// Note: can be a CWnd with PostNcDestroy self cleanup
	CWnd* pView = (CWnd*)pContext->m_pNewViewClass->CreateObject();
	if (pView == NULL)
	{
		TRACE(traceAppMsg, 0, "Warning: Dynamic create of view type %hs failed.\n",
			pContext->m_pNewViewClass->m_lpszClassName);
		return NULL;
	}
	ASSERT_KINDOF(CWnd, pView);

	// views are always created with a border!
	if (!pView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0,0,0,0), this, nID, pContext))
	{
		TRACE(traceAppMsg, 0, "Warning: could not create view for frame.\n");
		return NULL;        // can't continue without a view
	}

	if (pView->GetExStyle() & WS_EX_CLIENTEDGE)
	{
		// remove the 3d style from the frame, since the view is
		//  providing it.
		// make sure to recalc the non-client area
		ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);
	}
	return pView;
}

BOOL CFrameWnd::OnCreateClient(LPCREATESTRUCT, CCreateContext* pContext)
{
	// default create client will create a view if asked for it
	if (pContext != NULL && pContext->m_pNewViewClass != NULL)
	{
		if (CreateView(pContext, AFX_IDW_PANE_FIRST) == NULL)
			return FALSE;
	}
	return TRUE;
}

int CFrameWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	ENSURE_ARG(lpcs != NULL);
	CCreateContext* pContext = (CCreateContext*)lpcs->lpCreateParams;
	return OnCreateHelper(lpcs, pContext);
}

int CFrameWnd::OnCreateHelper(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	if (CWnd::OnCreate(lpcs) == -1)
		return -1;

	// create special children first
	if (!OnCreateClient(lpcs, pContext))
	{
		TRACE(traceAppMsg, 0, "Failed to create client pane/view for frame.\n");
		return -1;
	}

	// post message for initial message string
	PostMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);

	// make sure the child windows have been properly sized
	RecalcLayout();

	return 0;   // create ok
}

LPCTSTR CFrameWnd::GetIconWndClass(DWORD dwDefaultStyle, UINT nIDResource)
{
	ASSERT_VALID_IDR(nIDResource);
	HINSTANCE hInst = AfxFindResourceHandle(
		ATL_MAKEINTRESOURCE(nIDResource), ATL_RT_GROUP_ICON);
	HICON hIcon = ::LoadIcon(hInst, ATL_MAKEINTRESOURCE(nIDResource));
	if (hIcon != NULL)
	{
		CREATESTRUCT cs;
		memset(&cs, 0, sizeof(CREATESTRUCT));
		cs.style = dwDefaultStyle;
		PreCreateWindow(cs);
			// will fill lpszClassName with default WNDCLASS name
			// ignore instance handle from PreCreateWindow.

		WNDCLASS wndcls;
		if (cs.lpszClass != NULL &&
			AfxCtxGetClassInfo(AfxGetInstanceHandle(), cs.lpszClass, &wndcls) &&
			wndcls.hIcon != hIcon)
		{
			// register a very similar WNDCLASS
			return AfxRegisterWndClass(wndcls.style,
				wndcls.hCursor, wndcls.hbrBackground, hIcon);
		}
	}
	return NULL;        // just use the default
}

BOOL CFrameWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
	CWnd* pParentWnd, CCreateContext* pContext)
{
	// only do this once
	ASSERT_VALID_IDR(nIDResource);
	ASSERT(m_nIDHelp == 0 || m_nIDHelp == nIDResource);

	m_nIDHelp = nIDResource;    // ID for help context (+HID_BASE_RESOURCE)

	CString strFullString;
	if (strFullString.LoadString(nIDResource))
		AfxExtractSubString(m_strTitle, strFullString, 0);    // first sub-string

	VERIFY(AfxDeferRegisterClass(AFX_WNDFRAMEORVIEW_REG));

	// attempt to create the window
	LPCTSTR lpszClass = GetIconWndClass(dwDefaultStyle, nIDResource);
	CString strTitle = m_strTitle;
	if (!Create(lpszClass, strTitle, dwDefaultStyle, rectDefault,
	  pParentWnd, ATL_MAKEINTRESOURCE(nIDResource), 0L, pContext))
	{
		return FALSE;   // will self destruct on failure normally
	}

	// save the default menu handle
	ASSERT(m_hWnd != NULL);
	m_hMenuDefault = m_dwMenuBarState == AFX_MBS_VISIBLE ? ::GetMenu(m_hWnd) : m_hMenu;

	// load accelerator resource
	LoadAccelTable(ATL_MAKEINTRESOURCE(nIDResource));

	if (pContext == NULL)   // send initial update
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

	return TRUE;
}

void CFrameWnd::OnUpdateFrameMenu(HMENU hMenuAlt)
{
	if (hMenuAlt == NULL)
	{
		// attempt to get default menu from document
		CDocument* pDoc = GetActiveDocument();
		if (pDoc != NULL)
			hMenuAlt = pDoc->GetDefaultMenu();
		// use default menu stored in frame if none from document
		if (hMenuAlt == NULL)
			hMenuAlt = m_hMenuDefault;
	}
	// finally, set the menu
	if (m_dwMenuBarState == AFX_MBS_VISIBLE)
	{
		::SetMenu(m_hWnd, hMenuAlt);
	}
	else if (m_dwMenuBarState == AFX_MBS_HIDDEN)
	{
		m_hMenu = hMenuAlt;
	}
}

void CFrameWnd::InitialUpdateFrame(CDocument* pDoc, BOOL bMakeVisible)
{
	// if the frame does not have an active view, set to first pane
	CView* pView = NULL;
	if (GetActiveView() == NULL)
	{
		CWnd* pWnd = GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
		if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CView)))
		{
			pView = (CView*)pWnd;
			SetActiveView(pView, FALSE);
		}
	}

	if (bMakeVisible)
	{
		// send initial update to all views (and other controls) in the frame
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		// give view a chance to save the focus (CFormView needs this)
		if (pView != NULL)
			pView->OnActivateFrame(WA_INACTIVE, this);

		// finally, activate the frame
		// (send the default show command unless the main desktop window)
		int nCmdShow = -1;      // default
		CWinApp* pApp = AfxGetApp();
		if (pApp != NULL && pApp->m_pMainWnd == this)
		{
			nCmdShow = pApp->m_nCmdShow; // use the parameter from WinMain
			pApp->m_nCmdShow = -1; // set to default after first time
		}
		ActivateFrame(nCmdShow);
		if (pView != NULL)
			pView->OnActivateView(TRUE, pView, pView);
	}

	// update frame counts and frame title (may already have been visible)
	if (pDoc != NULL)
		pDoc->UpdateFrameCounts();
	OnUpdateFrameTitle(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd closing down

void CFrameWnd::OnClose()
{
	if (m_lpfnCloseProc != NULL)
		(*m_lpfnCloseProc)(this);

	// Note: only queries the active document
	CDocument* pDocument = GetActiveDocument();
	if (pDocument != NULL && !pDocument->CanCloseFrame(this))
	{
		// document can't close right now -- don't close it
		return;
	}
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && pApp->m_pMainWnd == this)
	{
		// attempt to save all documents
		if (pDocument == NULL && !pApp->SaveAllModified())
			return;     // don't close it

		// hide the application's windows before closing all the documents
		pApp->HideApplication();

		// close all documents first
		pApp->CloseAllDocuments(FALSE);

		// don't exit if there are outstanding component objects
		if (!AfxOleCanExitApp())
		{
			// take user out of control of the app
			AfxOleSetUserCtrl(FALSE);

			// don't destroy the main window and close down just yet
			//  (there are outstanding component (OLE) objects)
			return;
		}

		// there are cases where destroying the documents may destroy the
		//  main window of the application.
		if (!afxContextIsDLL && pApp->m_pMainWnd == NULL)
		{
			AfxPostQuitMessage(0);
			return;
		}
	}

	// detect the case that this is the last frame on the document and
	// shut down with OnCloseDocument instead.
	if (pDocument != NULL && pDocument->m_bAutoDelete)
	{
		BOOL bOtherFrame = FALSE;
		POSITION pos = pDocument->GetFirstViewPosition();
		while (pos != NULL)
		{
			CView* pView = pDocument->GetNextView(pos);
			ENSURE_VALID(pView);
			if (pView->GetParentFrame() != this)
			{
				bOtherFrame = TRUE;
				break;
			}
		}
		if (!bOtherFrame)
		{
			pDocument->OnCloseDocument();
			return;
		}

		// allow the document to cleanup before the window is destroyed
		pDocument->PreCloseFrame(this);
	}

	// then destroy the window
	DestroyWindow();
}

void CFrameWnd::OnDestroy()
{
	DestroyDockBars();

	// reset menu to default before final shutdown
	if (m_hMenuDefault != NULL && ::GetMenu(m_hWnd) != m_hMenuDefault)
	{
		::SetMenu(m_hWnd, m_hMenuDefault);
		ASSERT(::GetMenu(m_hWnd) == m_hMenuDefault);
	}

	// Automatically quit when the main window is destroyed.
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && pApp->m_pMainWnd == this && pApp->m_eHelpType == afxWinHelp)
	{
		// closing the main application window
		::WinHelp(m_hWnd, NULL, HELP_QUIT, 0L);

		// will call PostQuitMessage in CWnd::OnNcDestroy
	}
	CWnd::OnDestroy();
}

void CFrameWnd::RemoveControlBar(CControlBar *pBar)
{
	POSITION pos = m_listControlBars.Find(pBar);
	if (pos != NULL)
		m_listControlBars.RemoveAt(pos);
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd command/message routing

BOOL CFrameWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo)
{
	CPushRoutingFrame push(this);

	// pump through current view FIRST
	CView* pView = GetActiveView();
	if (pView != NULL && pView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// then pump through frame
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// last but not least, pump through app
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && pApp->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return FALSE;
}

// Delegate scroll messages to active view as well
void CFrameWnd::OnHScroll(UINT, UINT, CScrollBar*)
{
	CWnd* pActiveView = GetActiveView();
	if (pActiveView != NULL)
	{
		const MSG* pMsg = GetCurrentMessage();
		pActiveView->SendMessage(WM_HSCROLL, pMsg->wParam, pMsg->lParam);
	}
}

void CFrameWnd::OnVScroll(UINT, UINT, CScrollBar*)
{
	CWnd* pActiveView = GetActiveView();
	if (pActiveView != NULL)
	{
		const MSG* pMsg = GetCurrentMessage();
		pActiveView->SendMessage(WM_VSCROLL, pMsg->wParam, pMsg->lParam);
	}
}

LRESULT CFrameWnd::OnActivateTopLevel(WPARAM wParam, LPARAM lParam)
{
	CWnd::OnActivateTopLevel(wParam, lParam);

	// exit Shift+F1 help mode on activation changes
	ExitHelpMode();

#ifndef _AFX_NO_OLE_SUPPORT
	// allow OnFrameWindowActivate to be sent to in-place items
	if (m_pNotifyHook != NULL)
	{
		// activate when active and when not minimized
		m_pNotifyHook->OnActivate(
			LOWORD(wParam) != WA_INACTIVE && !HIWORD(wParam));
	}
#endif

	// deactivate current active view
	CWinThread *pThread = AfxGetThread();
	ASSERT(pThread);
	if (pThread->m_pMainWnd == this)
	{
		CView* pActiveView = GetActiveView();
		if (pActiveView == NULL)
			pActiveView = GetActiveFrame()->GetActiveView();
		if (pActiveView != NULL)
			pActiveView->OnActivateView(FALSE, pActiveView, pActiveView);
	}

	// force idle processing to update any key state indicators
	PostMessage(WM_KICKIDLE);

	return 0;
}

void CFrameWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	// hide the menu bar
	if (nState == WA_INACTIVE && (m_dwMenuBarVisibility & AFX_MBV_KEEPVISIBLE) == 0)
	{
		SetMenuBarState(AFX_MBS_HIDDEN);
	}

	// get top level frame unless this is a child window
	// determine if window should be active or not
	CFrameWnd* pTopLevel = (GetStyle() & WS_CHILD) ? this : GetTopLevelFrame();
	ENSURE_VALID(pTopLevel);
	CWnd* pActive = (nState == WA_INACTIVE ? pWndOther : this);
	BOOL bStayActive = (pActive != NULL) ?
		(pTopLevel == pActive ||
		(pTopLevel == pActive->GetTopLevelFrame() &&
		(pActive == pTopLevel ||
			pActive->SendMessage(WM_FLOATSTATUS, FS_SYNCACTIVE) != 0)))
			: FALSE;
	pTopLevel->m_nFlags &= ~WF_STAYACTIVE;
	if (bStayActive)
		pTopLevel->m_nFlags |= WF_STAYACTIVE;

	// sync floating windows to the new state
	NotifyFloatingWindows(bStayActive ? FS_ACTIVATE : FS_DEACTIVATE);

	// get active view (use active frame if no active view)
	CView* pActiveView = GetActiveView();
	if (pActiveView == NULL)
		pActiveView = GetActiveFrame()->GetActiveView();

	// when frame gets activated, re-activate current view
	if (pActiveView != NULL)
	{
		if (nState != WA_INACTIVE && !bMinimized)
			pActiveView->OnActivateView(TRUE, pActiveView, pActiveView);

		// always notify the view of frame activations
		pActiveView->OnActivateFrame(nState, this);
	}
}

BOOL CFrameWnd::OnNcActivate(BOOL bActive)
{
	// stay active if WF_STAYACTIVE bit is on
	if (m_nFlags & WF_STAYACTIVE)
		bActive = TRUE;

	// but do not stay active if the window is disabled
	if (!IsWindowEnabled())
		bActive = FALSE;

	// do not call the base class because it will call Default()
	//  and we may have changed bActive.
	return (BOOL)DefWindowProc(WM_NCACTIVATE, bActive, 0L);
}

void CFrameWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	ENSURE_VALID(pFrameWnd);

	// set status bar as appropriate
	UINT nItemID = (nID & 0xFFF0);

	// don't interfere with system commands if not in help mode
	if (pFrameWnd->m_bHelpMode)
	{
		switch (nItemID)
		{
		case SC_SIZE:
		case SC_MOVE:
		case SC_MINIMIZE:
		case SC_MAXIMIZE:
		case SC_NEXTWINDOW:
		case SC_PREVWINDOW:
		case SC_CLOSE:
		case SC_RESTORE:
		case SC_TASKLIST:
			if (!SendMessage(WM_COMMANDHELP, 0,
			  HID_BASE_COMMAND+ID_COMMAND_FROM_SC(nItemID)))
				SendMessage(WM_COMMAND, ID_DEFAULT_HELP);
			return;
		}
	}

	// call default functionality
	CWnd::OnSysCommand(nID, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// default frame processing

// default drop processing will attempt to open the file
void CFrameWnd::OnDropFiles(HDROP hDropInfo)
{
	SetActiveWindow();      // activate us first !
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);

	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp != NULL);
	for (UINT iFile = 0; iFile < nFiles; iFile++)
	{
		TCHAR szFileName[_MAX_PATH];
		::DragQueryFile(hDropInfo, iFile, szFileName, _MAX_PATH);
		pApp->OpenDocumentFile(szFileName);
	}
	::DragFinish(hDropInfo);
}

// query end session for main frame will attempt to close it all down
BOOL CFrameWnd::OnQueryEndSession()
{
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && pApp->m_pMainWnd == this)
		return pApp->SaveAllModified();

	return TRUE;
}

// when Windows session ends, close all documents
void CFrameWnd::OnEndSession(BOOL bEnding)
{
	if (!bEnding)
		return;

	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && pApp->m_pMainWnd == this)
	{
		AfxOleSetUserCtrl(TRUE);    // keeps from randomly shutting down
		pApp->CloseAllDocuments(TRUE);

		// allow application to save settings, etc.
		pApp->ExitInstance();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Support for Shell DDE Execute messages

LRESULT CFrameWnd::OnDDEInitiate(WPARAM wParam, LPARAM lParam)
{
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && 
		LOWORD(lParam) != 0 && HIWORD(lParam) != 0 &&
		(ATOM)LOWORD(lParam) == pApp->m_atomApp &&
		(ATOM)HIWORD(lParam) == pApp->m_atomSystemTopic)
	{
		// make duplicates of the incoming atoms (really adding a reference)
		TCHAR szAtomName[_MAX_PATH];
		VERIFY(GlobalGetAtomName(pApp->m_atomApp,
			szAtomName, _MAX_PATH - 1) != 0);
		VERIFY(GlobalAddAtom(szAtomName) == pApp->m_atomApp);
		VERIFY(GlobalGetAtomName(pApp->m_atomSystemTopic,
			szAtomName, _MAX_PATH - 1) != 0);
		VERIFY(GlobalAddAtom(szAtomName) == pApp->m_atomSystemTopic);

		// send the WM_DDE_ACK (caller will delete duplicate atoms)
		::SendMessage((HWND)wParam, WM_DDE_ACK, (WPARAM)m_hWnd,
			MAKELPARAM(pApp->m_atomApp, pApp->m_atomSystemTopic));
	}
	return 0L;
}

// always ACK the execute command - even if we do nothing
LRESULT CFrameWnd::OnDDEExecute(WPARAM wParam, LPARAM lParam)
{
	// unpack the DDE message
   UINT_PTR unused;
	HGLOBAL hData;
   //IA64: Assume DDE LPARAMs are still 32-bit
	VERIFY(UnpackDDElParam(WM_DDE_EXECUTE, lParam, &unused, (UINT_PTR*)&hData));

	// get the command string
	LPCTSTR lpsz = (LPCTSTR)GlobalLock(hData);
	CString strCommand;
	TRY
	{
		strCommand = lpsz;
		GlobalUnlock(hData);
	}
	CATCH(CMemoryException, e)
	{
		GlobalUnlock(hData);
		DELETE_EXCEPTION(e);
	}
	END_CATCH

	// acknowledge now - before attempting to execute
	::PostMessage((HWND)wParam, WM_DDE_ACK, (WPARAM)m_hWnd,
	  //IA64: Assume DDE LPARAMs are still 32-bit
		ReuseDDElParam(lParam, WM_DDE_EXECUTE, WM_DDE_ACK,
		(UINT)0x8000, (UINT_PTR)hData));

	// don't execute the command when the window is disabled
	if (!IsWindowEnabled())
	{
		TRACE(traceAppMsg, 0, _T("Warning: DDE command '%s' ignored because window is disabled.\n"),
			strCommand.GetString());
		return 0;
	}

	// execute the command
	LPTSTR lpszCommand = strCommand.GetBuffer();
	if (!AfxGetApp()->OnDDECommand(lpszCommand))
		TRACE(traceAppMsg, 0, _T("Error: failed to execute DDE command '%s'.\n"), lpszCommand);
	strCommand.ReleaseBuffer();
	return 0L;
}

LRESULT CFrameWnd::OnDDETerminate(WPARAM wParam, LPARAM lParam)
{
	::PostMessage((HWND)wParam, WM_DDE_TERMINATE, (WPARAM)m_hWnd, lParam);
	return 0L;
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd attributes

CView* CFrameWnd::GetActiveView() const
{
	ASSERT(m_pViewActive == NULL ||
		m_pViewActive->IsKindOf(RUNTIME_CLASS(CView)));
	return m_pViewActive;
}

void CFrameWnd::SetActiveView(CView* pViewNew, BOOL bNotify)
{
#ifdef _DEBUG
	if (pViewNew != NULL)
	{
		ASSERT(IsChild(pViewNew));
		ASSERT_KINDOF(CView, pViewNew);
	}
#endif //_DEBUG

	CView* pViewOld = m_pViewActive;
	if (pViewNew == pViewOld)
		return;     // do not re-activate if SetActiveView called more than once

	m_pViewActive = NULL;   // no active for the following processing

	// deactivate the old one
	if (pViewOld != NULL)
		pViewOld->OnActivateView(FALSE, pViewNew, pViewOld);

	// if the OnActivateView moves the active window,
	//    that will veto this change
	if (m_pViewActive != NULL)
		return;     // already set
	m_pViewActive = pViewNew;

	// activate
	if (pViewNew != NULL && bNotify)
		pViewNew->OnActivateView(TRUE, pViewNew, pViewOld);
}

/////////////////////////////////////////////////////////////////////////////
// Special view swapping/activation

void CFrameWnd::OnSetFocus(CWnd* pOldWnd)
{
	if (m_pViewActive != NULL)
		m_pViewActive->SetFocus();
	else
		CWnd::OnSetFocus(pOldWnd);
}

CDocument* CFrameWnd::GetActiveDocument()
{
	ASSERT_VALID(this);
	CView* pView = GetActiveView();
	if (pView != NULL)
		return pView->GetDocument();
	return NULL;
}

void CFrameWnd::ShowControlBar(CControlBar* pBar, BOOL bShow, BOOL bDelay)
{
	ENSURE_VALID(pBar);
	CFrameWnd* pParentFrame = pBar->GetDockingFrame();
	ASSERT(pParentFrame->GetTopLevelParent() == GetTopLevelParent());
		// parent frame of bar must be related

	if (bDelay)
	{
		pBar->DelayShow(bShow);
		pParentFrame->DelayRecalcLayout();
	}
	else
	{
		pBar->SetWindowPos(NULL, 0, 0, 0, 0,
			SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|
			(bShow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
		// call DelayShow to clear any contradictory DelayShow
		pBar->DelayShow(bShow);
		if (bShow || !pBar->IsFloating())
			pParentFrame->RecalcLayout(FALSE);
	}

	// show or hide the floating frame as appropriate
	if (pBar->IsFloating())
	{
		int nVisCount = pBar->m_pDockBar != NULL ?
			pBar->m_pDockBar->GetDockedVisibleCount() : bShow ? 1 : 0;
		if (nVisCount == 1 && bShow)
		{
			pParentFrame->m_nShowDelay = -1;
			if (bDelay)
			{
				pParentFrame->m_nShowDelay = SW_SHOWNA;
				pParentFrame->RecalcLayout(FALSE);
			}
			else
				pParentFrame->ShowWindow(SW_SHOWNA);
		}
		else if (nVisCount == 0)
		{
			ASSERT(!bShow);
			pParentFrame->m_nShowDelay = -1;
			if (bDelay)
				pParentFrame->m_nShowDelay = SW_HIDE;
			else
				pParentFrame->ShowWindow(SW_HIDE);
		}
		else if (!bDelay)
		{
			pParentFrame->RecalcLayout(FALSE);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Command prompts

void CFrameWnd::OnInitMenu(CMenu* pMenu)
{
#ifndef _AFX_NO_OLE_SUPPORT
	// allow hook to consume message
	if (m_pNotifyHook != NULL)
		m_pNotifyHook->OnInitMenu(pMenu);
#endif

	Default();
}

void CFrameWnd::OnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu)
{
	AfxCancelModes(m_hWnd);

	if (bSysMenu)
		return;     // don't support system menu

#ifndef _AFX_NO_OLE_SUPPORT
	// allow hook to consume message
	if (m_pNotifyHook != NULL &&
		m_pNotifyHook->OnInitMenuPopup(pMenu, nIndex, bSysMenu))
	{
		return;
	}
#endif

	ENSURE_VALID(pMenu);
	
	// check the enabled state of various menu items

	CCmdUI state;
	state.m_pMenu = pMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// determine if menu is popup in top-level menu and set m_pOther to
	//  it if so (m_pParentMenu == NULL indicates that it is secondary popup)
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pMenu->m_hMenu)
		state.m_pParentMenu = pMenu;    // parent == child for tracking popup
	else if ((hParentMenu = (m_dwMenuBarState == AFX_MBS_VISIBLE) ? ::GetMenu(m_hWnd) : m_hMenu) != NULL)
	{
		CWnd* pParent = GetTopLevelParent();
			// child windows don't have menus -- need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = pParent->GetMenu()->GetSafeHmenu()) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nItemIndex = 0; nItemIndex < nIndexMax; nItemIndex++)
			{
				if (::GetSubMenu(hParentMenu, nItemIndex) == pMenu->m_hMenu)
				{
					// when popup is found, m_pParentMenu is containing menu
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	  state.m_nIndex++)
	{
		state.m_nID = pMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // menu separator or invalid cmd - ignore it

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// possibly a popup menu, route to first item of that popup
			state.m_pSubMenu = pMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // first item of popup can't be routed to
			}
			state.DoUpdate(this, FALSE);    // popups are never auto disabled
		}
		else
		{
			// normal menu item
			// Auto enable/disable if frame window has 'm_bAutoMenuEnable'
			//    set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, m_bAutoMenuEnable && state.m_nID < 0xF000);
		}

		// adjust for menu deletions and additions
		UINT nCount = pMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}

void CFrameWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	ENSURE_VALID(pFrameWnd);

#ifndef _AFX_NO_OLE_SUPPORT
	// allow hook to consume message
	if (m_pNotifyHook != NULL &&
		m_pNotifyHook->OnMenuSelect(nItemID, nFlags, hSysMenu))
	{
		return;
	}
#endif

	// set the tracking state (update on idle)
	if (nFlags == 0xFFFF)
	{
		// cancel menu operation (go back to idle now)
		m_nFlags &= ~WF_NOPOPMSG;
		if (!pFrameWnd->m_bHelpMode)
			m_nIDTracking = AFX_IDS_IDLEMESSAGE;
		else
			m_nIDTracking = AFX_IDS_HELPMODEMESSAGE;
		SendMessage(WM_SETMESSAGESTRING, (WPARAM)m_nIDTracking);
		ASSERT(m_nIDTracking == m_nIDLastMessage);

		// update right away
		CWnd* pWnd = GetMessageBar();
		if (pWnd != NULL)
			pWnd->UpdateWindow();

		if (hSysMenu == 0 && // the menu has been closed
			(m_dwMenuBarVisibility & AFX_MBV_KEEPVISIBLE) == 0 &&
			::GetKeyState(VK_F10) >= 0 && // not pressed
			::GetKeyState(VK_MENU) >= 0 &&
			!m_bMouseHitMenu)
		{
			SetMenuBarState(AFX_MBS_HIDDEN);
		}
	}
	else
	{
		if (m_bTempShowMenu)
		{
			m_bTempShowMenu = FALSE;
			if ((nFlags & MF_SYSMENU) &&
				(m_dwMenuBarVisibility & AFX_MBV_KEEPVISIBLE) == 0)
			{
				SetMenuBarState(AFX_MBS_HIDDEN);
			}
		}

		if (nItemID == 0 || nFlags & (MF_SEPARATOR|MF_POPUP))
		{
			// nothing should be displayed
			m_nIDTracking = 0;
		}
		else if (nItemID >= 0xF000 && nItemID < 0xF1F0) // max of 31 SC_s
		{
			// special strings table entries for system commands
			m_nIDTracking = ID_COMMAND_FROM_SC(nItemID);
			ASSERT(m_nIDTracking >= AFX_IDS_SCFIRST &&
				m_nIDTracking < AFX_IDS_SCFIRST + 31);
		}
		else if (nItemID >= AFX_IDM_FIRST_MDICHILD)
		{
			// all MDI Child windows map to the same help id
			m_nIDTracking = AFX_IDS_MDICHILD;
		}
		else
		{
			// track on idle
			m_nIDTracking = nItemID;
		}
		pFrameWnd->m_nFlags |= WF_NOPOPMSG;
	}

	// when running in-place, it is necessary to cause a message to
	//  be pumped through the queue.
	if (m_nIDTracking != m_nIDLastMessage && GetParent() != NULL)
		PostMessage(WM_KICKIDLE);
}

void CFrameWnd::GetMessageString(UINT nID, CString& rMessage) const
{
	// load appropriate string
	LPTSTR lpsz = rMessage.GetBuffer(255);
	if (AfxLoadString(nID, lpsz) != 0)
	{
		// first newline terminates actual string
		lpsz = _tcschr(lpsz, '\n');
		if (lpsz != NULL)
			*lpsz = '\0';
	}
	else
	{
		// not found
		TRACE(traceAppMsg, 0, "Warning: no message line prompt for ID 0x%04X.\n", nID);
	}
	rMessage.ReleaseBuffer();
}

LRESULT CFrameWnd::OnPopMessageString(WPARAM wParam, LPARAM lParam)
{
	if (m_nFlags & WF_NOPOPMSG)
		return 0;

	return SendMessage(WM_SETMESSAGESTRING, wParam, lParam);
}

LRESULT CFrameWnd::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	UINT nIDLast = m_nIDLastMessage;
	m_nFlags &= ~WF_NOPOPMSG;

	CWnd* pMessageBar = GetMessageBar();
	if (pMessageBar != NULL)
	{
		LPCTSTR lpsz = NULL;
		CString strMessage;

		// set the message bar text
		if (lParam != 0)
		{
			ASSERT(wParam == 0);    // can't have both an ID and a string
			lpsz = (LPCTSTR)lParam; // set an explicit string
		}
		else if (wParam != 0)
		{
			// map SC_CLOSE to PREVIEW_CLOSE when in print preview mode
			if (wParam == AFX_IDS_SCCLOSE && m_lpfnCloseProc != NULL)
				wParam = AFX_IDS_PREVIEW_CLOSE;

			// get message associated with the ID indicated by wParam
		 //NT64: Assume IDs are still 32-bit
			GetMessageString((UINT)wParam, strMessage);
			lpsz = strMessage;
		}
		pMessageBar->SetWindowText(lpsz);

		// update owner of the bar in terms of last message selected
		CFrameWnd* pFrameWnd = pMessageBar->GetParentFrame();
		if (pFrameWnd != NULL)
		{
			pFrameWnd->m_nIDLastMessage = (UINT)wParam;
			pFrameWnd->m_nIDTracking = (UINT)wParam;
		}
	}

	m_nIDLastMessage = (UINT)wParam;    // new ID (or 0)
	m_nIDTracking = (UINT)wParam;       // so F1 on toolbar buttons work
	return nIDLast;
}

LRESULT CFrameWnd::OnHelpPromptAddr(WPARAM, LPARAM)
{
	return (LRESULT)&m_dwPromptContext;
}

CWnd* CFrameWnd::GetMessageBar()
{
	return GetDescendantWindow(AFX_IDW_STATUS_BAR, TRUE);
}

void CFrameWnd::OnEnterIdle(UINT nWhy, CWnd* pWho)
{
	CWnd::OnEnterIdle(nWhy, pWho);

	if (nWhy != MSGF_MENU || m_nIDTracking == m_nIDLastMessage)
		return;

	SetMessageText(m_nIDTracking);
	ASSERT(m_nIDTracking == m_nIDLastMessage);
}

void CFrameWnd::SetMessageText(LPCTSTR lpszText)
{
	SendMessage(WM_SETMESSAGESTRING, 0, (LPARAM)lpszText);
}

void CFrameWnd::SetMessageText(UINT nID)
{
	SendMessage(WM_SETMESSAGESTRING, (WPARAM)nID);
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd standard control bar management


void CFrameWnd::DestroyDockBars()
{
	// create a list of all the dock bars
	// this is necessary because m_listControlBars will change
	// as the dock bars and floating frames are destroyed
	CList<HWND,HWND> listDockBars, listCtrlBars;
	POSITION pos = m_listControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		CControlBar* pBar= (CControlBar*)m_listControlBars.GetNext(pos);
		ASSERT(pBar != NULL);
		if (pBar->IsDockBar())
		{
			listDockBars.AddTail( pBar->GetSafeHwnd() );
		}
		else
		{
			listCtrlBars.AddTail( pBar->GetSafeHwnd() );
		}
	}
	pos = listDockBars.GetHeadPosition();
	while (pos != NULL)
	{
		HWND hwndDock = listDockBars.GetNext(pos);		
		CDockBar* pDockBar = DYNAMIC_DOWNCAST(CDockBar,CWnd::FromHandlePermanent(hwndDock));
		if (pDockBar)
		{
			ASSERT(IsWindow(hwndDock));
		if (pDockBar->m_bFloating)
		{
			CFrameWnd* pFrameWnd = pDockBar->EnsureParentFrame();
			pFrameWnd->DestroyWindow();
		}
		else
			{
			pDockBar->DestroyWindow();
	}
		}
	}

	pos = listCtrlBars.GetHeadPosition();
	while (pos != NULL)
	{
		HWND hwndCtrlBar = listCtrlBars.GetNext(pos);
		//pDockBar->DestroyWindow() (see listDockBars) will destroy all child windows of this dockbar, 
		//including control bars, so check if HWND is still valid. We do not want to destroy a second time. 		
		CControlBar *pCtrlBar  = DYNAMIC_DOWNCAST(CControlBar,CWnd::FromHandlePermanent(hwndCtrlBar));
		if (pCtrlBar)
		{
			ASSERT(IsWindow(hwndCtrlBar));
			pCtrlBar->DestroyWindow();
		}
	}
}

CControlBar* CFrameWnd::GetControlBar(UINT nID)
{
	if (nID == 0)
		return NULL;
	POSITION pos = m_listControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		CControlBar* pBar = (CControlBar*)m_listControlBars.GetNext(pos);
		ASSERT(pBar != NULL);
		if (_AfxGetDlgCtrlID(pBar->m_hWnd) == nID)
		{
			ASSERT_KINDOF(CControlBar, pBar);
			return pBar;
		}
	}
	return NULL;
}

void CFrameWnd::OnUpdateControlBarMenu(CCmdUI* pCmdUI)
{
	ASSERT(ID_VIEW_STATUS_BAR == AFX_IDW_STATUS_BAR);
	ASSERT(ID_VIEW_TOOLBAR == AFX_IDW_TOOLBAR);
	ASSERT(ID_VIEW_REBAR == AFX_IDW_REBAR);
	ENSURE_ARG(pCmdUI != NULL);

	CControlBar* pBar = GetControlBar(pCmdUI->m_nID);
	if (pBar != NULL)
	{
		pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
		return;
	}
	pCmdUI->ContinueRouting();
}

BOOL CFrameWnd::OnBarCheck(UINT nID)
{
	ASSERT(ID_VIEW_STATUS_BAR == AFX_IDW_STATUS_BAR);
	ASSERT(ID_VIEW_TOOLBAR == AFX_IDW_TOOLBAR);
	ASSERT(ID_VIEW_REBAR == AFX_IDW_REBAR);

	CControlBar* pBar = GetControlBar(nID);
	if (pBar != NULL)
	{
		ShowControlBar(pBar, (pBar->GetStyle() & WS_VISIBLE) == 0, FALSE);
		return TRUE;
	}
	return FALSE;
}

BOOL CFrameWnd::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	ENSURE_ARG(pNMHDR != NULL);
	ENSURE_ARG(pResult != NULL);
	ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	TCHAR szFullText[256];
	CString strTipText;
	UINT_PTR nID = pNMHDR->idFrom;
	if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		nID = _AfxGetDlgCtrlID((HWND)nID);
	}

	if (nID != 0) // will be zero on a separator
	{
		// don't handle the message if no string resource found
		if (AfxLoadString((UINT)nID, szFullText) == 0)
			return FALSE;

		// this is the command id, not the button index
		AfxExtractSubString(strTipText, szFullText, 1, '\n');
	}
#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		Checked::strncpy_s(pTTTA->szText, _countof(pTTTA->szText), strTipText, _TRUNCATE);
	else
		_mbstowcsz(pTTTW->szText, strTipText, _countof(pTTTW->szText));
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, _countof(pTTTA->szText));
	else
		Checked::wcsncpy_s(pTTTW->szText, _countof(pTTTW->szText), strTipText, _TRUNCATE);
#endif
	*pResult = 0;

	// bring the tooltip window above other popup windows
	::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
		SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER);

	return TRUE;    // message was handled
}

/////////////////////////////////////////////////////////////////////////////
// Support for standard status bar

void CFrameWnd::OnUpdateKeyIndicator(CCmdUI* pCmdUI)
{
	ENSURE_ARG(pCmdUI != NULL);
	UINT nVK;
	UINT flag = 0x0001;

	switch (pCmdUI->m_nID)
	{
	case ID_INDICATOR_CAPS:
		nVK = VK_CAPITAL;
		break;

	case ID_INDICATOR_NUM:
		nVK = VK_NUMLOCK;
		break;

	case ID_INDICATOR_SCRL:
		nVK = VK_SCROLL;
		break;

	case ID_INDICATOR_KANA:
		nVK = VK_KANA;
		break;

	default:
		TRACE(traceAppMsg, 0, "Warning: OnUpdateKeyIndicator - unknown indicator 0x%04X.\n",
			pCmdUI->m_nID);
		pCmdUI->ContinueRouting();
		return; // not for us
	}

	pCmdUI->Enable(::GetKeyState(nVK) & flag);
		// enable static text based on toggled key state
	ASSERT(pCmdUI->m_bEnableChanged);
}

void CFrameWnd::OnUpdateContextHelp(CCmdUI* pCmdUI)
{
	ENSURE_ARG(pCmdUI != NULL);
	if (AfxGetMainWnd() == this)
		pCmdUI->SetCheck(!!m_bHelpMode);
	else
		pCmdUI->ContinueRouting();
}

/////////////////////////////////////////////////////////////////////////////
// Setting title of frame window - UISG standard

void CFrameWnd::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
		return;     // leave it alone!

#ifndef _AFX_NO_OLE_SUPPORT
	// allow hook to set the title (used for OLE support)
	if (m_pNotifyHook != NULL && m_pNotifyHook->OnUpdateFrameTitle())
		return;
#endif

	CDocument* pDocument = GetActiveDocument();
	if (bAddToTitle && pDocument != NULL)
		UpdateFrameTitleForDocument(pDocument->GetTitle());
	else
		UpdateFrameTitleForDocument(NULL);
}

void CFrameWnd::UpdateFrameTitleForDocument(LPCTSTR lpszDocName)
{
	CString WindowText;

	if (GetStyle() & FWS_PREFIXTITLE)
	{
		// get name of currently active view
		if (lpszDocName != NULL)
		{
			WindowText += lpszDocName;

			// add current window # if needed
			if (m_nWindow > 0)
			{
				TCHAR szText[32];
				
				// :%d will produce a maximum of 11 TCHARs
				_stprintf_s(szText, _countof(szText), _T(":%d"), m_nWindow);
				WindowText += szText;
			}
			WindowText += _T(" - ");
		}
		WindowText += m_strTitle;
	}
	else
	{
		// get name of currently active view
		WindowText += m_strTitle;
		if (lpszDocName != NULL)
		{
			WindowText += _T(" - ");
			WindowText += lpszDocName;

			// add current window # if needed
			if (m_nWindow > 0)
			{
				TCHAR szText[32];
				
				// :%d will produce a maximum of 11 TCHARs
				_stprintf_s(szText, _countof(szText), _T(":%d"), m_nWindow);
				WindowText += szText;
			}
		}
	}

	// set title if changed, but don't remove completely
	// Note: will be excessive for MDI Frame with maximized child
	AfxSetWindowText(m_hWnd, (LPCTSTR) WindowText);
}

/////////////////////////////////////////////////////////////////////////////

void CFrameWnd::OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState)
{
	ENSURE_ARG(pState != NULL);
	// default implementation changes control bars, menu and main pane window

#ifndef _AFX_NO_OLE_SUPPORT
	CFrameWnd* pActiveFrame = GetActiveFrame();
	ENSURE_VALID(pActiveFrame);
	if (bPreview && pActiveFrame->m_pNotifyHook != NULL)
		pActiveFrame->m_pNotifyHook->OnDocActivate(FALSE);
#endif

	// Set visibility of standard ControlBars (only the first 32)
	DWORD dwOldStates = 0;
	POSITION pos = m_listControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		CControlBar* pBar = (CControlBar*)m_listControlBars.GetNext(pos);
		ENSURE_VALID(pBar);
		UINT_PTR nID = _AfxGetDlgCtrlID(pBar->m_hWnd);
		if (nID >= AFX_IDW_CONTROLBAR_FIRST && nID <= AFX_IDW_CONTROLBAR_FIRST+31)
		{
			DWORD dwMask = 1L << (UINT)(nID - AFX_IDW_CONTROLBAR_FIRST);
			if (pBar->IsVisible())
				dwOldStates |= dwMask;      // save if previously visible
			if (!pBar->IsDockBar() || nID != AFX_IDW_DOCKBAR_FLOAT)
				ShowControlBar(pBar, (pState->dwStates & dwMask), TRUE);
		}
	}
	pState->dwStates = dwOldStates; // save for restore

	if (bPreview)
	{
		// Entering Print Preview

		ASSERT(m_lpfnCloseProc == NULL);    // no chaining
		m_lpfnCloseProc = pState->lpfnCloseProc;

		// show any modeless dialogs, popup windows, float tools, etc
		ShowOwnedWindows(FALSE);

		// Hide the main pane
		HWND hWnd = ::GetDlgItem(m_hWnd, pState->nIDMainPane);
		ASSERT(hWnd != NULL);       // must be one that we are hiding!
		::ShowWindow(hWnd, SW_HIDE);

		// Get rid of the menu first (will resize the window)
		pState->hMenu = m_dwMenuBarState == AFX_MBS_VISIBLE ? ::GetMenu(m_hWnd) : m_hMenu;

		if (pState->hMenu != NULL)
		{
			// Invalidate before SetMenu since we are going to replace
			//  the frame's client area anyway
			Invalidate();
			SetMenu(NULL);
			m_nIdleFlags &= ~idleMenu;  // avoid any idle menu processing
		}

		// Save the accelerator table and remove it.
		pState->hAccelTable = m_hAccelTable;
		m_hAccelTable = NULL;
		LoadAccelTable(ATL_MAKEINTRESOURCE(AFX_IDR_PREVIEW_ACCEL));

		// Make room for the PreviewView by changing AFX_IDW_PANE_FIRST's ID
		//  to AFX_IDW_PREVIEW_FIRST
		if (pState->nIDMainPane != AFX_IDW_PANE_FIRST)
			hWnd = ::GetDlgItem(m_hWnd, AFX_IDW_PANE_FIRST);
		if (hWnd != NULL)
			_AfxSetDlgCtrlID(hWnd, AFX_IDW_PANE_SAVE);

#ifdef _DEBUG
		if ((::GetWindowLong(m_hWnd, GWL_STYLE) & (WS_HSCROLL|WS_VSCROLL)) != 0)
			TRACE(traceAppMsg, 0, "Warning: scroll bars in frame windows may cause unusual behaviour.\n");
#endif
	}
	else
	{
		// Leaving Preview
		m_lpfnCloseProc = NULL;

		// shift original AFX_IDW_PANE_FIRST back to its rightful ID
		HWND hWnd = ::GetDlgItem(m_hWnd, AFX_IDW_PANE_SAVE);
		if (hWnd != NULL)
		{
			HWND hWndTemp = ::GetDlgItem(m_hWnd, AFX_IDW_PANE_FIRST);
			if (hWndTemp != NULL)
				_AfxSetDlgCtrlID(hWndTemp, AFX_IDW_PANE_SAVE);
			_AfxSetDlgCtrlID(hWnd, AFX_IDW_PANE_FIRST);
		}

		// put the menu back in place if it was removed before
		if (pState->hMenu != NULL)
		{
			// Invalidate before SetMenu since we are going to replace
			//  the frame's client area anyway
			Invalidate();
			if (m_dwMenuBarState == AFX_MBS_VISIBLE)
			{
				::SetMenu(m_hWnd, pState->hMenu);
			}
			else if (m_dwMenuBarState == AFX_MBS_HIDDEN)
			{
				m_hMenu = pState->hMenu;
			}
		}

		// recalc layout now, before showing the main pane
#ifndef _AFX_NO_OLE_SUPPORT
		if (pActiveFrame->m_pNotifyHook != NULL)
			pActiveFrame->m_pNotifyHook->OnDocActivate(TRUE);
#endif
		RecalcLayout();

		// now show main pane that was hidden
		if (pState->nIDMainPane != AFX_IDW_PANE_FIRST)
			hWnd = ::GetDlgItem(m_hWnd, pState->nIDMainPane);
		ASSERT(hWnd != NULL);
		::ShowWindow(hWnd, SW_SHOW);

		// Restore the Accelerator table
		m_hAccelTable = pState->hAccelTable;

		// show any modeless dialogs, popup windows, float tools, etc
		ShowOwnedWindows(TRUE);
	}
}

void CFrameWnd::DelayUpdateFrameMenu(HMENU hMenuAlt)
{
	m_hMenuAlt = hMenuAlt;
	m_nIdleFlags |= idleMenu;
}

void CFrameWnd::OnIdleUpdateCmdUI()
{
	// update menu if necessary
	if (m_nIdleFlags & idleMenu)
	{
		m_nIdleFlags &= ~idleMenu;
		OnUpdateFrameMenu(m_hMenuAlt);
	}

	// update title if necessary
	if (m_nIdleFlags & idleTitle)
		OnUpdateFrameTitle(TRUE);

	// recalc layout if necessary
	if (m_nIdleFlags & idleLayout)
	{
		RecalcLayout(m_nIdleFlags & idleNotify);
		UpdateWindow();
	}

	// set the current message string if necessary
	if (m_nIDTracking != m_nIDLastMessage)
	{
		SetMessageText(m_nIDTracking);
		ASSERT(m_nIDTracking == m_nIDLastMessage);
	}
	m_nIdleFlags = 0;
}

CFrameWnd* CFrameWnd::GetActiveFrame()
{
	// by default, the active frame is the frame itself (MDI is different)
	return this;
}

void CFrameWnd::RecalcLayout(BOOL bNotify)
{
	if (m_bInRecalcLayout)
		return;

	m_bInRecalcLayout = TRUE;
	// clear idle flags for recalc layout if called elsewhere
	if (m_nIdleFlags & idleNotify)
		bNotify = TRUE;
	m_nIdleFlags &= ~(idleLayout|idleNotify);

#ifndef _AFX_NO_OLE_SUPPORT
	// call the layout hook -- OLE support uses this hook
	if (bNotify && m_pNotifyHook != NULL)
		m_pNotifyHook->OnRecalcLayout();
#endif

	// reposition all the child windows (regardless of ID)
	if (GetStyle() & FWS_SNAPTOBARS)
	{
		CRect rect(0, 0, 32767, 32767);
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery,
			&rect, &rect, FALSE);
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposExtra,
			&m_rectBorder, &rect, TRUE);
		CalcWindowRect(&rect);
		SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
			SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	}
	else
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposExtra, &m_rectBorder);
	m_bInRecalcLayout = FALSE;
}

// CFrameWnd implementation of OLE border space negotiation
BOOL CFrameWnd::NegotiateBorderSpace(UINT nBorderCmd, LPRECT lpRectBorder)
{
	CRect border, request;

	switch (nBorderCmd)
	{
	case borderGet:
		ASSERT(lpRectBorder != NULL);
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery,
			lpRectBorder);
		break;

	case borderRequest:
		return TRUE;

	case borderSet:
		if (lpRectBorder == NULL)
		{
			if (!m_rectBorder.IsRectNull())
			{
				// releasing all border space -- recalc needed
				m_rectBorder.SetRectEmpty();
				return TRUE;
			}
			// original rect is empty & lpRectBorder is NULL, no recalc needed
			return FALSE;
		}
		if (!::EqualRect(m_rectBorder, lpRectBorder))
		{
			// the rects are different -- recalc needed
			m_rectBorder.CopyRect(lpRectBorder);
			return TRUE;
		}
		return FALSE;   // no recalc needed

	default:
		ASSERT(FALSE);  // invalid CFrameWnd::BorderCmd
	}

	return TRUE;
}

void CFrameWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);    // important for MDI Children
	if (nType != SIZE_MINIMIZED)
		RecalcLayout();
}

BOOL CFrameWnd::OnEraseBkgnd(CDC* pDC)
{
	if (m_pViewActive != NULL)
		return TRUE;        // active view will erase/paint itself
	// for view-less frame just use the default background fill
	return CWnd::OnEraseBkgnd(pDC);
}

BOOL CFrameWnd::OnChevronPushed(UINT id, NMHDR *pnm, LRESULT *pResult) 
{
	NMREBARCHEVRON *pnmReBarChevron = (NMREBARCHEVRON *) pnm;
	CReBar *pReBar;
	CToolBar *pToolBar;
	CString strClassName;
	int nLen;

	// Note: If your program is asserting here make sure that you only use a CToolBar control in
	// the band in which the chevron is used.  For chevron processing, MFC only handles rebar bands
	// that contain toolbar controls - any other control will assert.  If you wish to use another
	// control with the chevron you must provide your own implementation of OnChevronPushed.
	nLen = ::lstrlen(REBARCLASSNAME) + 1;
	::GetClassName(pnmReBarChevron->hdr.hwndFrom, strClassName.GetBuffer(nLen), nLen);
	strClassName.ReleaseBuffer();
	pReBar = (CReBar *) CReBar::FromHandlePermanent(pnmReBarChevron->hdr.hwndFrom);
	ASSERT(strClassName == REBARCLASSNAME && pReBar && pReBar->IsKindOf(RUNTIME_CLASS(CReBar)));
	if(strClassName == REBARCLASSNAME && pReBar && pReBar->IsKindOf(RUNTIME_CLASS(CReBar)))
	{
		// Make sure it's our rebar
		CFrameWnd *pFrameWnd = pReBar->GetParentFrame();
		if(pFrameWnd && this != pFrameWnd)
			return pFrameWnd->OnChevronPushed(id, pnm, pResult);

		// Yep, it's ours
		REBARBANDINFO BandInfo;
		IMAGEINFO ImageInfo;
		MENUITEMINFO MenuInfo;
		CRect rcBand, rcBtn, rcInt;
		DWORD dwBtnCount, i, nItemCount;
		CChevronOwnerDrawMenu menu;
		CString strRes, strMenu;
		UINT nID, nStyle;
		int iImage;
		CImageList *imgActive;
		CDC dcDest;
		CClientDC dcClient(this);
		CPoint pt(0, 0);

		BandInfo.cbSize = pReBar->GetReBarBandInfoSize();
		BandInfo.fMask = RBBIM_CHILD;

		pReBar->GetReBarCtrl().GetBandInfo(pnmReBarChevron->uBand, &BandInfo);
		pReBar->GetReBarCtrl().GetRect(pnmReBarChevron->uBand, &rcBand);

		nLen = ::lstrlen(TOOLBARCLASSNAME) + 1;
		::GetClassName(BandInfo.hwndChild, strClassName.GetBuffer(nLen), nLen);
		strClassName.ReleaseBuffer();
		pToolBar = (CToolBar *) CToolBar::FromHandlePermanent(BandInfo.hwndChild);
		ASSERT(strClassName == TOOLBARCLASSNAME && pToolBar && pToolBar->IsKindOf(RUNTIME_CLASS(CToolBar)));
		if(strClassName == TOOLBARCLASSNAME && pToolBar && pToolBar->IsKindOf(RUNTIME_CLASS(CToolBar)))
		{
			rcBand.right = pnmReBarChevron->rc.left;
			pReBar->ClientToScreen(&rcBand);
			pToolBar->ScreenToClient(&rcBand);

			i = dwBtnCount = pToolBar->GetToolBarCtrl().GetButtonCount();
			do
			{
				i--;
				pToolBar->GetToolBarCtrl().GetItemRect(i, &rcBtn);
			}
			while(!rcInt.IntersectRect(rcBand, rcBtn) && i > 0);

			ZeroMemory(&MenuInfo, sizeof(MENUITEMINFO));
			MenuInfo.cbSize = sizeof(MENUITEMINFO);

			imgActive = pToolBar->GetToolBarCtrl().GetImageList();
			CTypedPtrArray<CObArray, CBitmap*> bmp;
			bmp.SetSize(dwBtnCount - i);

			menu.CreatePopupMenu();
			dcDest.CreateCompatibleDC(&dcClient);

			nItemCount = 0;
			for(; i < dwBtnCount; i++)
			{
				pToolBar->GetButtonInfo(i, nID, nStyle, iImage);
				if(!(nStyle & BTNS_SEP))
				{
					MenuInfo.fMask = MIIM_FTYPE | MIIM_ID | MIIM_DATA | MIIM_STRING;

					// Menu Text
					if (strRes.LoadString(nID))
					{
						AfxExtractSubString(strMenu, strRes, 1, _T('\n'));
					}
					else
					{
						strMenu.Empty();
					}

					// Menu Bitmap
					bmp.SetAtGrow(nItemCount,new CBitmap);
					if (imgActive != NULL && imgActive->GetImageInfo(iImage, &ImageInfo)!= 0)
					{
						CRect rc(ImageInfo.rcImage);
						rc.OffsetRect(-rc.TopLeft());
						bmp[nItemCount]->CreateCompatibleBitmap(&dcClient, rc.right, rc.bottom);
						bmp[nItemCount] = dcDest.SelectObject(bmp[nItemCount]);
						dcDest.FillSolidRect(rc, ::GetSysColor(COLOR_MENU));
						imgActive->Draw(&dcDest, iImage, pt, ILD_TRANSPARENT);
						bmp[nItemCount] = dcDest.SelectObject(bmp[nItemCount]);

						//set the info
						MenuInfo.dwItemData = (ULONG_PTR) bmp[nItemCount];
					}
					else
					{
						// dont set the info
						MenuInfo.dwItemData = (ULONG_PTR) 0;
					}

					// Set the info
					MenuInfo.dwTypeData = const_cast<LPTSTR>(static_cast<LPCTSTR>(strMenu));
					MenuInfo.wID = nID;
					MenuInfo.fType = MFT_OWNERDRAW;

					nItemCount++;
				}
				else
				{
					// don't add a separator as the first item in the menu
					if(nItemCount == 0)
						continue;

					MenuInfo.fMask = MIIM_FTYPE;
					MenuInfo.fType = MFT_SEPARATOR;
				}
				menu.InsertMenuItem(i, &MenuInfo, TRUE);
			}

			CRect rc = pnmReBarChevron->rc;
			ClientToScreen(&rc);
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, 
				rc.left, rc.bottom, this);

			*pResult = 0;
			for(i = 0; i < nItemCount; i++)
				delete bmp[i];

			return TRUE;
		}
	}
	return FALSE;
}

void CFrameWnd::ActivateFrame(int nCmdShow)
	// nCmdShow is the normal show mode this frame should be in
{
	// translate default nCmdShow (-1)
	if (nCmdShow == -1)
	{
		if (!IsWindowVisible())
			nCmdShow = SW_SHOWNORMAL;
		else if (IsIconic())
			nCmdShow = SW_RESTORE;
	}

	// bring to top before showing
	BringToTop(nCmdShow);

	if (nCmdShow != -1)
	{
		// show the window as specified
		ShowWindow(nCmdShow);

		// and finally, bring to top after showing
		BringToTop(nCmdShow);
	}
}

void CFrameWnd::BringToTop(int nCmdShow)
{
	// place the window on top except for certain nCmdShow
	if (nCmdShow != SW_HIDE &&
		nCmdShow != SW_MINIMIZE && nCmdShow != SW_SHOWMINNOACTIVE &&
		nCmdShow != SW_SHOWNA && nCmdShow != SW_SHOWNOACTIVATE)
	{
		// if no last active popup, it will return m_hWnd
		HWND hWndLastPop = ::GetLastActivePopup(m_hWnd);
		::BringWindowToTop(hWndLastPop);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd Diagnostics

#ifdef _DEBUG
void CFrameWnd::AssertValid() const
{
	CWnd::AssertValid();
	if (m_pViewActive != NULL)
		ASSERT_VALID(m_pViewActive);
}

void CFrameWnd::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);

	dc << "m_hAccelTable = " << (void*)m_hAccelTable;
	dc << "\nm_nWindow = " << m_nWindow;
	dc << "\nm_nIDHelp = " << m_nIDHelp;
	dc << "\nm_nIDTracking = " << m_nIDTracking;
	dc << "\nm_nIDLastMessage = " << m_nIDLastMessage;
	if (m_pViewActive != NULL)
		dc << "\nwith active view: " << m_pViewActive;
	else
		dc << "\nno active view";

	dc << "\n";
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CControlBar segmentation

CFrameWnd* CControlBar::GetDockingFrame() const
{
	CFrameWnd* pFrameWnd = GetParentFrame();
	if (pFrameWnd == NULL)
		pFrameWnd = m_pDockSite;

	ASSERT(pFrameWnd != NULL);
	ASSERT_KINDOF(CFrameWnd, pFrameWnd);
	return pFrameWnd;
}

BOOL CControlBar::IsFloating() const
{
	if (IsDockBar())
		return ((CDockBar*)this)->m_bFloating;
	else
		return m_pDockBar != NULL && m_pDockBar->m_bFloating;
}


// in this file for IsKindOf library granularity (IsKindOf references these)
IMPLEMENT_DYNCREATE(CFrameWnd, CWnd)
IMPLEMENT_DYNAMIC(CView, CWnd)
IMPLEMENT_DYNAMIC(CControlBar, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd implementation of hide/unhide menu bar to confirm to Windows 
// Vista Experience

void CFrameWnd::SetMenuBarVisibility(DWORD dwStyle)
{
	ENSURE_ARG(dwStyle == AFX_MBV_KEEPVISIBLE ||
			dwStyle == AFX_MBV_DISPLAYONFOCUS ||
			dwStyle == (AFX_MBV_DISPLAYONFOCUS | AFX_MBV_DISPLAYONF10));

	if (m_dwMenuBarVisibility != dwStyle)
		switch (dwStyle)
		{
		case AFX_MBV_KEEPVISIBLE:
			m_dwMenuBarVisibility = dwStyle;
			SetMenuBarState(AFX_MBS_VISIBLE);
			break;
		case AFX_MBV_DISPLAYONFOCUS:
		case AFX_MBV_DISPLAYONFOCUS | AFX_MBV_DISPLAYONF10:		
			m_dwMenuBarVisibility = dwStyle;
			SetMenuBarState(AFX_MBS_HIDDEN);
			break;
		default:
			ASSERT(FALSE);
		}
}

DWORD CFrameWnd::GetMenuBarVisibility() const
{
	return m_dwMenuBarVisibility;
}

BOOL CFrameWnd::SetMenuBarState(DWORD dwState)
{
	ENSURE_ARG(dwState == AFX_MBS_VISIBLE ||
				dwState == AFX_MBS_HIDDEN);

	if (m_dwMenuBarState == dwState)
	{
		return FALSE;
	}

	if (dwState == AFX_MBS_VISIBLE)
	{
		OnShowMenuBar();
		::SetMenu(m_hWnd, m_hMenu);
	}
	else
	{
		m_hMenu = ::GetMenu(m_hWnd);
		OnHideMenuBar();
		::SetMenu(m_hWnd, NULL);
	}

	m_dwMenuBarState = dwState;
	return TRUE;
}

DWORD CFrameWnd::GetMenuBarState() const
{
	return m_dwMenuBarState;
}

void CFrameWnd::OnShowMenuBar()
{
	return ;
}

void CFrameWnd::OnHideMenuBar()
{
	return ;
}

LRESULT CFrameWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
	if (m_bTempShowMenu)
	{
		m_bTempShowMenu = FALSE;
		if ((m_dwMenuBarVisibility & AFX_MBV_KEEPVISIBLE) == 0)
		{
			SetMenuBarState(AFX_MBS_HIDDEN);
		}
	}
	return CWnd::OnMenuChar(nChar, nFlags, pMenu);
}

CMenu* CFrameWnd::GetMenu() const
{
	ASSERT(::IsWindow(m_hWnd));
	if (m_dwMenuBarState == AFX_MBS_VISIBLE)
	{
		return CMenu::FromHandle(::GetMenu(m_hWnd));
	}
	else
	{
		ENSURE(m_dwMenuBarState == AFX_MBS_HIDDEN);
		return CMenu::FromHandle(m_hMenu);
	}	
}

BOOL CFrameWnd::SetMenu(CMenu* pMenu)
{
	ASSERT(::IsWindow(m_hWnd));
	if (m_dwMenuBarState == AFX_MBS_VISIBLE)
	{
		return ::SetMenu(m_hWnd, pMenu->GetSafeHmenu());
	}
	else
	{
		ENSURE(m_dwMenuBarState == AFX_MBS_HIDDEN);
		m_hMenu = pMenu->GetSafeHmenu();
		return TRUE;
	}
}

BOOL CFrameWnd::GetMenuBarInfo(LONG idObject, LONG idItem, PMENUBARINFO pmbi) const
{
	ASSERT(::IsWindow(m_hWnd)); 
	ASSERT(pmbi != NULL);
	if (m_dwMenuBarState == AFX_MBS_HIDDEN && idObject == OBJID_MENU)
	{
		CFrameWnd* pFrameWnd = new CFrameWnd;
		ENSURE(pFrameWnd->Create(NULL, NULL));
		ENSURE(::SetMenu(pFrameWnd->m_hWnd, m_hMenu));
		BOOL bResult = ::GetMenuBarInfo(pFrameWnd->m_hWnd, idObject, idItem, pmbi);
		ENSURE(::SetMenu(pFrameWnd->m_hWnd, NULL));
		ENSURE(pFrameWnd->DestroyWindow());

		return bResult;
	}
	
	return ::GetMenuBarInfo(m_hWnd, idObject, idItem, pmbi);
}
/////////////////////////////////////////////////////////////////////////////
