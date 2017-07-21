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
#ifndef _AFX_NO_OCC_SUPPORT
#include "occimpl.h"
#endif

#pragma warning(disable:4706)
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#pragma warning(default:4706)

#include <atlacc.h>	// Accessible Proxy from ATL
#include "sal.h"


// for dll builds we just delay load it
#ifndef _AFXDLL
PROCESS_LOCAL(_AFX_HTMLHELP_STATE, _afxHtmlHelpState)
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Globals

const UINT CWnd::m_nMsgDragList = ::RegisterWindowMessage(DRAGLISTMSGSTRING);
CWnd::PFNNOTIFYWINEVENT CWnd::m_pfnNotifyWinEvent = NULL;

// CWnds for setting z-order with SetWindowPos's pWndInsertAfter parameter
const AFX_DATADEF CWnd CWnd::wndTop(HWND_TOP);
const AFX_DATADEF CWnd CWnd::wndBottom(HWND_BOTTOM);
const AFX_DATADEF CWnd CWnd::wndTopMost(HWND_TOPMOST);
const AFX_DATADEF CWnd CWnd::wndNoTopMost(HWND_NOTOPMOST);

const TCHAR _afxWnd[] = AFX_WND;
const TCHAR _afxWndControlBar[] = AFX_WNDCONTROLBAR;
const TCHAR _afxWndMDIFrame[] = AFX_WNDMDIFRAME;
const TCHAR _afxWndFrameOrView[] = AFX_WNDFRAMEORVIEW;
const TCHAR _afxWndOleControl[] = AFX_WNDOLECONTROL;

/////////////////////////////////////////////////////////////////////////////
// CWnd construction

CWnd::CWnd()
{
	m_hWnd = NULL;
	m_bEnableActiveAccessibility = false;
	m_pProxy = NULL;
	m_pStdObject = NULL;
	m_hWndOwner = NULL;
	m_nFlags = 0;
	m_pfnSuper = NULL;
	m_nModalResult = 0;
	m_pDropTarget = NULL;
#ifndef _AFX_NO_OCC_SUPPORT	
	m_pCtrlCont = NULL;
	m_pCtrlSite = NULL;
#endif

}

CWnd::CWnd(HWND hWnd)
{
	m_hWnd = hWnd;
	m_bEnableActiveAccessibility = false;
	m_pProxy = NULL;
	m_hWndOwner = NULL;
	m_nFlags = 0;
	m_pfnSuper = NULL;
	m_nModalResult = 0;
	m_pDropTarget = NULL;
#ifndef _AFX_NO_OCC_SUPPORT	
	m_pCtrlCont = NULL;
	m_pCtrlSite = NULL;
#endif
}

// Change a window's style

AFX_STATIC BOOL AFXAPI _AfxModifyStyle(HWND hWnd, int nStyleOffset,
	DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	ASSERT(hWnd != NULL);
	DWORD dwStyle = ::GetWindowLong(hWnd, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle == dwNewStyle)
		return FALSE;

	::SetWindowLong(hWnd, nStyleOffset, dwNewStyle);
	if (nFlags != 0)
	{
		::SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
	}
	return TRUE;
}

BOOL PASCAL
CWnd::ModifyStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	return _AfxModifyStyle(hWnd, GWL_STYLE, dwRemove, dwAdd, nFlags);
}

BOOL PASCAL
CWnd::ModifyStyleEx(HWND hWnd, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	return _AfxModifyStyle(hWnd, GWL_EXSTYLE, dwRemove, dwAdd, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// Special helpers for certain windows messages

AFX_STATIC void AFXAPI _AfxPreInitDialog(
	CWnd* pWnd, LPRECT lpRectOld, DWORD* pdwStyleOld)
{
	ASSERT(lpRectOld != NULL);	
	ASSERT(pdwStyleOld != NULL);

	pWnd->GetWindowRect(lpRectOld);
	*pdwStyleOld = pWnd->GetStyle();
}

AFX_STATIC void AFXAPI _AfxPostInitDialog(
	CWnd* pWnd, const RECT& rectOld, DWORD dwStyleOld)
{
	// must be hidden to start with		
	if (dwStyleOld & WS_VISIBLE)
		return;

	// must not be visible after WM_INITDIALOG
	if (pWnd->GetStyle() & (WS_VISIBLE|WS_CHILD))
		return;

	// must not move during WM_INITDIALOG
	CRect rect;
	pWnd->GetWindowRect(rect);
	if (rectOld.left != rect.left || rectOld.top != rect.top)
		return;

	// must be unowned or owner disabled
	CWnd* pParent = pWnd->GetWindow(GW_OWNER);
	if (pParent != NULL && pParent->IsWindowEnabled())
		return;

	if (!pWnd->CheckAutoCenter())
		return;

	// center modal dialog boxes/message boxes
	pWnd->CenterWindow();
}

AFX_STATIC void AFXAPI
_AfxHandleActivate(CWnd* pWnd, WPARAM nState, CWnd* pWndOther)
{
	ASSERT(pWnd != NULL);		

	// send WM_ACTIVATETOPLEVEL when top-level parents change
	if (!(pWnd->GetStyle() & WS_CHILD))
	{
		CWnd* pTopLevel= pWnd->GetTopLevelParent();
		if (pTopLevel && (pWndOther == NULL || !::IsWindow(pWndOther->m_hWnd) || pTopLevel != pWndOther->GetTopLevelParent()))
		{
			// lParam points to window getting the WM_ACTIVATE message and
			//  hWndOther from the WM_ACTIVATE.
			HWND hWnd2[2];
			hWnd2[0] = pWnd->m_hWnd;
			hWnd2[1] = pWndOther->GetSafeHwnd();
			// send it...
			pTopLevel->SendMessage(WM_ACTIVATETOPLEVEL, nState, (LPARAM)&hWnd2[0]);
		}
	}
}

AFX_STATIC BOOL AFXAPI
_AfxHandleSetCursor(CWnd* pWnd, UINT nHitTest, UINT nMsg)
{
	if (nHitTest == HTERROR &&
		(nMsg == WM_LBUTTONDOWN || nMsg == WM_MBUTTONDOWN ||
		 nMsg == WM_RBUTTONDOWN))
	{
		// activate the last active window if not active
		CWnd* pLastActive = pWnd->GetTopLevelParent();		
		if (pLastActive != NULL)
			pLastActive = pLastActive->GetLastActivePopup();
		if (pLastActive != NULL &&
			pLastActive != CWnd::GetForegroundWindow() &&
			pLastActive->IsWindowEnabled())
		{
			pLastActive->SetForegroundWindow();
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Official way to send message to a CWnd

LRESULT AFXAPI AfxCallWndProc(CWnd* pWnd, HWND hWnd, UINT nMsg,
	WPARAM wParam = 0, LPARAM lParam = 0)
{
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	MSG oldState = pThreadState->m_lastSentMsg;   // save for nesting
	pThreadState->m_lastSentMsg.hwnd = hWnd;
	pThreadState->m_lastSentMsg.message = nMsg;
	pThreadState->m_lastSentMsg.wParam = wParam;
	pThreadState->m_lastSentMsg.lParam = lParam;

#ifdef _DEBUG
	_AfxTraceMsg(_T("WndProc"), &pThreadState->m_lastSentMsg);
#endif

	// Catch exceptions thrown outside the scope of a callback
	// in debug builds and warn the user.
	LRESULT lResult;
	TRY
	{
#ifndef _AFX_NO_OCC_SUPPORT
		// special case for WM_DESTROY
		if ((nMsg == WM_DESTROY) && (pWnd->m_pCtrlCont != NULL))
			pWnd->m_pCtrlCont->OnUIActivate(NULL);				
#endif

		// special case for WM_INITDIALOG
		CRect rectOld;
		DWORD dwStyle = 0;
		if (nMsg == WM_INITDIALOG)
			_AfxPreInitDialog(pWnd, &rectOld, &dwStyle);

		// delegate to object's WindowProc
		lResult = pWnd->WindowProc(nMsg, wParam, lParam);

		// more special case for WM_INITDIALOG
		if (nMsg == WM_INITDIALOG)
			_AfxPostInitDialog(pWnd, rectOld, dwStyle);
	}
	CATCH_ALL(e)
	{
		lResult = AfxProcessWndProcException(e, &pThreadState->m_lastSentMsg);
		TRACE(traceAppMsg, 0, "Warning: Uncaught exception in WindowProc (returning %ld).\n",
			lResult);
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	pThreadState->m_lastSentMsg = oldState;
	return lResult;
}

const MSG* PASCAL CWnd::GetCurrentMessage()
{
	// fill in time and position when asked for
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	pThreadState->m_lastSentMsg.time = ::GetMessageTime();
	pThreadState->m_lastSentMsg.pt = CPoint(::GetMessagePos());
	return &pThreadState->m_lastSentMsg;
}

LRESULT CWnd::Default()
{
	// call DefWindowProc with the last message
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	return DefWindowProc(pThreadState->m_lastSentMsg.message,
		pThreadState->m_lastSentMsg.wParam, pThreadState->m_lastSentMsg.lParam);
}

/////////////////////////////////////////////////////////////////////////////
// Map from HWND to CWnd*

CHandleMap* PASCAL afxMapHWND(BOOL bCreate)
{
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	if (pState->m_pmapHWND == NULL && bCreate)
	{
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
#ifndef _AFX_PORTABLE
		_PNH pnhOldHandler = AfxSetNewHandler(&AfxCriticalNewHandler);
#endif
		pState->m_pmapHWND = new CHandleMap(RUNTIME_CLASS(CWnd),
			ConstructDestruct<CWnd>::Construct, ConstructDestruct<CWnd>::Destruct, 
			offsetof(CWnd, m_hWnd));

#ifndef _AFX_PORTABLE
		AfxSetNewHandler(pnhOldHandler);
#endif
		AfxEnableMemoryTracking(bEnable);
	}
	return pState->m_pmapHWND;
}

CWnd* PASCAL CWnd::FromHandle(HWND hWnd)
{
	CHandleMap* pMap = afxMapHWND(TRUE); //create map if not exist
	ASSERT(pMap != NULL);
	CWnd* pWnd = (CWnd*)pMap->FromHandle(hWnd);

#ifndef _AFX_NO_OCC_SUPPORT
	pWnd->AttachControlSite(pMap);
#endif

	ASSERT(pWnd == NULL || pWnd->m_hWnd == hWnd);
	return pWnd;
}

CWnd* PASCAL CWnd::FromHandlePermanent(HWND hWnd)
{
	CHandleMap* pMap = afxMapHWND();
	CWnd* pWnd = NULL;
	if (pMap != NULL)
	{
		// only look in the permanent map - does no allocations
		pWnd = (CWnd*)pMap->LookupPermanent(hWnd);
		ASSERT(pWnd == NULL || pWnd->m_hWnd == hWnd);
	}
	return pWnd;
}

BOOL CWnd::Attach(HWND hWndNew)
{
	ASSERT(m_hWnd == NULL);     // only attach once, detach on destroy
	ASSERT(FromHandlePermanent(hWndNew) == NULL);
		// must not already be in permanent map

	if (hWndNew == NULL)
		return FALSE;

	CHandleMap* pMap = afxMapHWND(TRUE); // create map if not exist
	ASSERT(pMap != NULL);

	pMap->SetPermanent(m_hWnd = hWndNew, this);


#ifndef _AFX_NO_OCC_SUPPORT
	AttachControlSite(pMap);
#endif

	return TRUE;
}

HWND CWnd::Detach()
{
	HWND hWnd = m_hWnd;
	if (hWnd != NULL)
	{
		CHandleMap* pMap = afxMapHWND(); // don't create if not exist
		if (pMap != NULL)
			pMap->RemoveHandle(m_hWnd);
	m_hWnd = NULL;
	}

#ifndef _AFX_NO_OCC_SUPPORT
	m_pCtrlSite = NULL;
#endif

	return hWnd;
}

void CWnd::PreSubclassWindow()
{
	// no default processing
}

/////////////////////////////////////////////////////////////////////////////
// CMenu functions
CMenu* CWnd::GetMenu() const
{
	ASSERT(::IsWindow(m_hWnd));
	return CMenu::FromHandle(::GetMenu(m_hWnd));
}

BOOL CWnd::SetMenu(CMenu* pMenu)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::SetMenu(m_hWnd, pMenu->GetSafeHmenu());
}

////////////////////////////////////////////////////////////////////////////
// The WndProc for all CWnd's and derived classes

LRESULT CALLBACK
AfxWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	// special message which identifies the window as using AfxWndProc
	if (nMsg == WM_QUERYAFXWNDPROC)
		return 1;

	// all other messages route through message map
	CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
	ASSERT(pWnd != NULL);					
	ASSERT(pWnd==NULL || pWnd->m_hWnd == hWnd);
	if (pWnd == NULL || pWnd->m_hWnd != hWnd)
		return ::DefWindowProc(hWnd, nMsg, wParam, lParam);
	return AfxCallWndProc(pWnd, hWnd, nMsg, wParam, lParam);
}

// always indirectly accessed via AfxGetAfxWndProc
WNDPROC AFXAPI AfxGetAfxWndProc()
{
#ifdef _AFXDLL
	return AfxGetModuleState()->m_pfnAfxWndProc;
#else
	return &AfxWndProc;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// Special WndProcs (activation handling & gray dialogs)

AFX_STATIC_DATA const TCHAR _afxOldWndProc[] = _T("AfxOldWndProc423");

LRESULT CALLBACK
_AfxActivationWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC oldWndProc = (WNDPROC)::GetProp(hWnd, _afxOldWndProc);
	ASSERT(oldWndProc != NULL);	

	LRESULT lResult = 0;
	TRY
	{
		BOOL bCallDefault = TRUE;
		switch (nMsg)
		{
		case WM_INITDIALOG:
			{
				DWORD dwStyle;
				CRect rectOld;
				CWnd* pWnd = CWnd::FromHandle(hWnd);
				_AfxPreInitDialog(pWnd, &rectOld, &dwStyle);
				bCallDefault = FALSE;
				lResult = CallWindowProc(oldWndProc, hWnd, nMsg, wParam, lParam);
				_AfxPostInitDialog(pWnd, rectOld, dwStyle);
			}
			break;

		case WM_ACTIVATE:
			_AfxHandleActivate(CWnd::FromHandle(hWnd), wParam,
				CWnd::FromHandle((HWND)lParam));
			break;

		case WM_SETCURSOR:
			bCallDefault = !_AfxHandleSetCursor(CWnd::FromHandle(hWnd),
				(short)LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_NCDESTROY:
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<INT_PTR>(oldWndProc));
			RemoveProp(hWnd, _afxOldWndProc);
			GlobalDeleteAtom(GlobalFindAtom(_afxOldWndProc));
			break;
		}

		// call original wndproc for default handling
		if (bCallDefault)
			lResult = CallWindowProc(oldWndProc, hWnd, nMsg, wParam, lParam);
	}
	CATCH_ALL(e)
	{
		// handle exception
		MSG msg;
		msg.hwnd = hWnd;
		msg.message = nMsg;
		msg.wParam = wParam;
		msg.lParam = lParam;
		lResult = AfxProcessWndProcException(e, &msg);
		TRACE(traceAppMsg, 0, "Warning: Uncaught exception in _AfxActivationWndProc (returning %ld).\n",
			lResult);
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	return lResult;
}

/////////////////////////////////////////////////////////////////////////////
// Window creation hooks

LRESULT CALLBACK
_AfxCbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	if (code != HCBT_CREATEWND)
	{
		// wait for HCBT_CREATEWND just pass others on...
		return CallNextHookEx(pThreadState->m_hHookOldCbtFilter, code,
			wParam, lParam);
	}

	ASSERT(lParam != NULL);
	LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;
	ASSERT(lpcs != NULL);

	CWnd* pWndInit = pThreadState->m_pWndInit;
	BOOL bContextIsDLL = afxContextIsDLL;
	if (pWndInit != NULL || (!(lpcs->style & WS_CHILD) && !bContextIsDLL))
	{
		// Note: special check to avoid subclassing the IME window
		if (_afxDBCS)
		{
			// check for cheap CS_IME style first...
			if (GetClassLong((HWND)wParam, GCL_STYLE) & CS_IME)
				goto lCallNextHook;

			// get class name of the window that is being created
			LPCTSTR pszClassName;
			TCHAR szClassName[_countof("ime")+1];
			if (DWORD_PTR(lpcs->lpszClass) > 0xffff)
			{
				pszClassName = lpcs->lpszClass;
			}
			else
			{
				szClassName[0] = '\0';
				GlobalGetAtomName((ATOM)lpcs->lpszClass, szClassName, _countof(szClassName));
				pszClassName = szClassName;
			}

			// a little more expensive to test this way, but necessary...
			if (::AfxInvariantStrICmp(pszClassName, _T("ime")) == 0)
				goto lCallNextHook;
		}

		ASSERT(wParam != NULL); // should be non-NULL HWND
		HWND hWnd = (HWND)wParam;
		WNDPROC oldWndProc;
		if (pWndInit != NULL)
		{
			AFX_MANAGE_STATE(pWndInit->m_pModuleState);

			// the window should not be in the permanent map at this time
			ASSERT(CWnd::FromHandlePermanent(hWnd) == NULL);

			// connect the HWND to pWndInit...
			pWndInit->Attach(hWnd);
			// allow other subclassing to occur first
			pWndInit->PreSubclassWindow();

			WNDPROC *pOldWndProc = pWndInit->GetSuperWndProcAddr();
			ASSERT(pOldWndProc != NULL);

			// subclass the window with standard AfxWndProc
			WNDPROC afxWndProc = AfxGetAfxWndProc();
			oldWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC,
				(DWORD_PTR)afxWndProc);
			ASSERT(oldWndProc != NULL);
			if (oldWndProc != afxWndProc)
				*pOldWndProc = oldWndProc;

			pThreadState->m_pWndInit = NULL;
		}
		else
		{
			ASSERT(!bContextIsDLL);   // should never get here

			static ATOM s_atomMenu = 0;
			bool bSubclass = true;			

			if (s_atomMenu == 0)
			{
				WNDCLASSEX wc;
				memset(&wc, 0, sizeof(WNDCLASSEX));
				wc.cbSize = sizeof(WNDCLASSEX);
				s_atomMenu = (ATOM)::AfxCtxGetClassInfoEx(NULL, _T("#32768"), &wc);
			}

			// Do not subclass menus.
			if (s_atomMenu != 0)
			{
				ATOM atomWnd = (ATOM)::GetClassLongPtr(hWnd, GCW_ATOM);
				if (atomWnd == s_atomMenu)
						bSubclass = false;
			}
			else
			{			
				TCHAR szClassName[256];
				if (::GetClassName(hWnd, szClassName, 256))
				{
					szClassName[255] = NULL;
					if (_tcscmp(szClassName, _T("#32768")) == 0)
						bSubclass = false;
				}
			}			
			if (bSubclass)
			{
				// subclass the window with the proc which does gray backgrounds
				oldWndProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
				if (oldWndProc != NULL && GetProp(hWnd, _afxOldWndProc) == NULL)
				{
					SetProp(hWnd, _afxOldWndProc, oldWndProc);
					if ((WNDPROC)GetProp(hWnd, _afxOldWndProc) == oldWndProc)
					{
						GlobalAddAtom(_afxOldWndProc);
						SetWindowLongPtr(hWnd, GWLP_WNDPROC, (DWORD_PTR)_AfxActivationWndProc);
						ASSERT(oldWndProc != NULL);
					}
				}
			}
		}
	}

lCallNextHook:
	LRESULT lResult = CallNextHookEx(pThreadState->m_hHookOldCbtFilter, code,
		wParam, lParam);

#ifndef _AFXDLL
	if (bContextIsDLL)
	{
		::UnhookWindowsHookEx(pThreadState->m_hHookOldCbtFilter);
		pThreadState->m_hHookOldCbtFilter = NULL;
	}
#endif
	return lResult;
}

void AFXAPI AfxHookWindowCreate(CWnd* pWnd)
{
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	if (pThreadState->m_pWndInit == pWnd)
		return;

	if (pThreadState->m_hHookOldCbtFilter == NULL)
	{
		pThreadState->m_hHookOldCbtFilter = ::SetWindowsHookEx(WH_CBT,
			_AfxCbtFilterHook, NULL, ::GetCurrentThreadId());
		if (pThreadState->m_hHookOldCbtFilter == NULL)
			AfxThrowMemoryException();
	}
	ASSERT(pThreadState->m_hHookOldCbtFilter != NULL);
	ASSERT(pWnd != NULL);
	ASSERT(pWnd->m_hWnd == NULL);   // only do once

	ASSERT(pThreadState->m_pWndInit == NULL);   // hook not already in progress
	pThreadState->m_pWndInit = pWnd;
}

BOOL AFXAPI AfxUnhookWindowCreate()
{
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
#ifndef _AFXDLL
	if (afxContextIsDLL && pThreadState->m_hHookOldCbtFilter != NULL)
	{
		::UnhookWindowsHookEx(pThreadState->m_hHookOldCbtFilter);
		pThreadState->m_hHookOldCbtFilter = NULL;
	}
#endif
	if (pThreadState->m_pWndInit != NULL)
	{
		pThreadState->m_pWndInit = NULL;
		return FALSE;   // was not successfully hooked
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWnd creation

BOOL CWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID,
		LPVOID lpParam /* = NULL */)
{
	return CreateEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), (HMENU)(UINT_PTR)nID, lpParam);
}

BOOL CWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName, DWORD dwStyle,
	int x, int y, int nWidth, int nHeight,
	HWND hWndParent, HMENU nIDorHMenu, LPVOID lpParam)
{
	ASSERT(lpszClassName == NULL || AfxIsValidString(lpszClassName) || 
		AfxIsValidAtom(lpszClassName));
	ENSURE_ARG(lpszWindowName == NULL || AfxIsValidString(lpszWindowName));
	
	// allow modification of several common create parameters
	CREATESTRUCT cs;
	cs.dwExStyle = dwExStyle;
	cs.lpszClass = lpszClassName;
	cs.lpszName = lpszWindowName;
	cs.style = dwStyle;
	cs.x = x;
	cs.y = y;
	cs.cx = nWidth;
	cs.cy = nHeight;
	cs.hwndParent = hWndParent;
	cs.hMenu = nIDorHMenu;
	cs.hInstance = AfxGetInstanceHandle();
	cs.lpCreateParams = lpParam;

	if (!PreCreateWindow(cs))
	{
		PostNcDestroy();
		return FALSE;
	}

	AfxHookWindowCreate(this);
	HWND hWnd = ::AfxCtxCreateWindowEx(cs.dwExStyle, cs.lpszClass,
			cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy,
			cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);

#ifdef _DEBUG
	if (hWnd == NULL)
	{
		TRACE(traceAppMsg, 0, "Warning: Window creation failed: GetLastError returns 0x%8.8X\n",
			GetLastError());
	}
#endif

	if (!AfxUnhookWindowCreate())
		PostNcDestroy();        // cleanup if CreateWindowEx fails too soon

	if (hWnd == NULL)
		return FALSE;
	ASSERT(hWnd == m_hWnd); // should have been set in send msg hook
	return TRUE;
}

// for child windows
BOOL CWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (cs.lpszClass == NULL)
	{
		// make sure the default window class is registered
		VERIFY(AfxDeferRegisterClass(AFX_WND_REG));

		// no WNDCLASS provided - use child window default
		ASSERT(cs.style & WS_CHILD);
		cs.lpszClass = _afxWnd;
	}
	return TRUE;
}

BOOL CWnd::Create(LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName, DWORD dwStyle,
	const RECT& rect,
	CWnd* pParentWnd, UINT nID,
	CCreateContext* pContext)
{
	// can't use for desktop or pop-up windows (use CreateEx instead)
	ASSERT(pParentWnd != NULL);
	ASSERT((dwStyle & WS_POPUP) == 0);

	return CreateEx(0, lpszClassName, lpszWindowName,
		dwStyle | WS_CHILD,
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), (HMENU)(UINT_PTR)nID, (LPVOID)pContext);
}

CWnd::~CWnd()
{
	if (m_hWnd != NULL &&
		this != (CWnd*)&wndTop && this != (CWnd*)&wndBottom &&
		this != (CWnd*)&wndTopMost && this != (CWnd*)&wndNoTopMost)
	{
		TRACE(traceAppMsg, 0, _T("Warning: calling DestroyWindow in CWnd::~CWnd; ")
		   _T("OnDestroy or PostNcDestroy in derived class will not be called.\n"));
		DestroyWindow();
	}

#ifndef _AFX_NO_OCC_SUPPORT
	// cleanup control container,
	// including destroying controls

	delete m_pCtrlCont;

	// cleanup control site
	if (m_pCtrlSite != NULL && m_pCtrlSite->m_pWndCtrl == this)
		m_pCtrlSite->m_pWndCtrl = NULL;
#endif
}

void CWnd::OnDestroy()
{
#ifndef _AFX_NO_OCC_SUPPORT
	// cleanup control container
	delete m_pCtrlCont;
	m_pCtrlCont = NULL;
#endif

	// Active Accessibility
	if (m_pProxy != NULL)
		m_pProxy->SetServer(NULL, NULL);
	if (m_pStdObject != NULL)
		m_pStdObject->Release();

	Default();
}

// WM_NCDESTROY is the absolute LAST message sent.
void CWnd::OnNcDestroy()
{
	// cleanup main and active windows
	CWinThread* pThread = AfxGetThread();
	if (pThread != NULL)
	{
		if (pThread->m_pMainWnd == this)
		{
			if (!afxContextIsDLL)
			{
				// shut down current thread if possible
				if (pThread != AfxGetApp() || AfxOleCanExitApp())
					AfxPostQuitMessage(0);
			}
			pThread->m_pMainWnd = NULL;
		}
		if (pThread->m_pActiveWnd == this)
			pThread->m_pActiveWnd = NULL;
	}

#ifndef _AFX_NO_OLE_SUPPORT
	// cleanup OLE drop target interface
	if (m_pDropTarget != NULL)
	{
		m_pDropTarget->Revoke();
		m_pDropTarget = NULL;
	}
#endif

#ifndef _AFX_NO_OCC_SUPPORT
	// cleanup control container
	delete m_pCtrlCont;
	m_pCtrlCont = NULL;
#endif

	// cleanup tooltip support
	if (m_nFlags & WF_TOOLTIPS)
	{
		CToolTipCtrl* pToolTip = AfxGetModuleThreadState()->m_pToolTip;
		if (pToolTip->GetSafeHwnd() != NULL)
		{
			TOOLINFO ti; memset(&ti, 0, sizeof(TOOLINFO));
			ti.cbSize = sizeof(AFX_OLDTOOLINFO);
			ti.uFlags = TTF_IDISHWND;
			ti.hwnd = m_hWnd;
			ti.uId = (UINT_PTR)m_hWnd;
			pToolTip->SendMessage(TTM_DELTOOL, 0, (LPARAM)&ti);
		}
	}

	// call default, unsubclass, and detach from the map
	WNDPROC pfnWndProc = WNDPROC(GetWindowLongPtr(m_hWnd, GWLP_WNDPROC));
	Default();
	if (WNDPROC(GetWindowLongPtr(m_hWnd, GWLP_WNDPROC)) == pfnWndProc)
	{
		WNDPROC pfnSuper = *GetSuperWndProcAddr();
		if (pfnSuper != NULL)
			SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<INT_PTR>(pfnSuper));
	}
	Detach();
	ASSERT(m_hWnd == NULL);

	// call special post-cleanup routine
	PostNcDestroy();
}

void CWnd::PostNcDestroy()
{
	// default to nothing
}

void CWnd::OnFinalRelease()
{
	if (m_hWnd != NULL)
		DestroyWindow();    // will call PostNcDestroy
	else
		PostNcDestroy();
}

#ifdef _DEBUG
void CWnd::AssertValid() const
{
	if (m_hWnd == NULL)
		return;     // null (unattached) windows are valid

	// check for special wnd??? values
	ASSERT(HWND_TOP == NULL);       // same as desktop
	if (m_hWnd == HWND_BOTTOM)
		ASSERT(this == &CWnd::wndBottom);
	else if (m_hWnd == HWND_TOPMOST)
		ASSERT(this == &CWnd::wndTopMost);
	else if (m_hWnd == HWND_NOTOPMOST)
		ASSERT(this == &CWnd::wndNoTopMost);
	else
	{
		// should be a normal window
		ASSERT(::IsWindow(m_hWnd));

		// should also be in the permanent or temporary handle map
		CHandleMap* pMap = afxMapHWND();
		ASSERT(pMap != NULL);

		CObject* p=NULL;
		if(pMap)
		{
			ASSERT( (p = pMap->LookupPermanent(m_hWnd)) != NULL ||
					(p = pMap->LookupTemporary(m_hWnd)) != NULL);
		}
		ASSERT((CWnd*)p == this);   // must be us

		// Note: if either of the above asserts fire and you are
		// writing a multithreaded application, it is likely that
		// you have passed a C++ object from one thread to another
		// and have used that object in a way that was not intended.
		// (only simple inline wrapper functions should be used)
		//
		// In general, CWnd objects should be passed by HWND from
		// one thread to another.  The receiving thread can wrap
		// the HWND with a CWnd object by using CWnd::FromHandle.
		//
		// It is dangerous to pass C++ objects from one thread to
		// another, unless the objects are designed to be used in
		// such a manner.
	}
}

void CWnd::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "\nm_hWnd = " << (void*)m_hWnd;

	if (m_hWnd == NULL || m_hWnd == HWND_BOTTOM ||
		m_hWnd == HWND_TOPMOST || m_hWnd == HWND_NOTOPMOST)
	{
		// not a normal window - nothing more to dump
		return;
	}

	if (!::IsWindow(m_hWnd))
	{
		// not a valid window
		dc << " (illegal HWND)";
		return; // don't do anything more
	}

	CWnd* pWnd = CWnd::FromHandlePermanent(m_hWnd);
	if (pWnd != this)
		dc << " (Detached or temporary window)";
	else
		dc << " (permanent window)";

	// dump out window specific statistics
	TCHAR szBuf [64];
	if (!::SendMessage(m_hWnd, WM_QUERYAFXWNDPROC, 0, 0) && pWnd == this)
		GetWindowText(szBuf, _countof(szBuf));
	else
		::DefWindowProc(m_hWnd, WM_GETTEXT, _countof(szBuf), (LPARAM)&szBuf[0]);
	dc << "\ncaption = \"" << szBuf << "\"";

	::GetClassName(m_hWnd, szBuf, _countof(szBuf));
	dc << "\nclass name = \"" << szBuf << "\"";

	CRect rect;
	GetWindowRect(&rect);
	dc << "\nrect = " << rect;
	dc << "\nparent CWnd* = " << (void*)GetParent();

	dc << "\nstyle = " << (void*)(DWORD_PTR)::GetWindowLong(m_hWnd, GWL_STYLE);
	if (::GetWindowLong(m_hWnd, GWL_STYLE) & WS_CHILD)
		dc << "\nid = " << _AfxGetDlgCtrlID(m_hWnd);

	dc << "\n";
}
#endif

BOOL CWnd::DestroyWindow()
{
	CWnd* pWnd;
	CHandleMap* pMap;
	HWND hWndOrig;
	BOOL bResult;

	if ((m_hWnd == NULL) && (m_pCtrlSite == NULL))
		return FALSE;

	bResult = FALSE;
	pMap = NULL;
	pWnd = NULL;
	hWndOrig = NULL;
	if (m_hWnd != NULL)
	{
		pMap = afxMapHWND();
		ENSURE(pMap != NULL);
		pWnd = (CWnd*)pMap->LookupPermanent(m_hWnd);
#ifdef _DEBUG
		hWndOrig = m_hWnd;
#endif
	}

#ifdef _AFX_NO_OCC_SUPPORT
	if (m_hWnd != NULL)
		bResult = ::DestroyWindow(m_hWnd);
#else //_AFX_NO_OCC_SUPPORT
	if ((m_hWnd != NULL) || (m_pCtrlSite != NULL))
	{
		if (m_pCtrlSite == NULL)
			bResult = ::DestroyWindow(m_hWnd);
		else
			bResult = m_pCtrlSite->DestroyControl();
	}
#endif //_AFX_NO_OCC_SUPPORT

	if (hWndOrig != NULL)
	{
		// Note that 'this' may have been deleted at this point,
		//  (but only if pWnd != NULL)
		if (pWnd != NULL)
		{
			// Should have been detached by OnNcDestroy
#ifdef _DEBUG
			ASSERT(pMap->LookupPermanent(hWndOrig) == NULL);
#endif
		}
		else
		{
#ifdef _DEBUG
			ASSERT(m_hWnd == hWndOrig);
#endif
		// Detach after DestroyWindow called just in case
			Detach();
		}
	}

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// Default CWnd implementation

LRESULT CWnd::DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pfnSuper != NULL)
		return ::CallWindowProc(m_pfnSuper, m_hWnd, nMsg, wParam, lParam);

	WNDPROC pfnWndProc;
	if ((pfnWndProc = *GetSuperWndProcAddr()) == NULL)
		return ::DefWindowProc(m_hWnd, nMsg, wParam, lParam);
	else
		return ::CallWindowProc(pfnWndProc, m_hWnd, nMsg, wParam, lParam);
}

WNDPROC* CWnd::GetSuperWndProcAddr()
{
	// Note: it is no longer necessary to override GetSuperWndProcAddr
	//  for each control class with a different WNDCLASS.
	//  This implementation now uses instance data, such that the previous
	//  WNDPROC can be anything.

	return &m_pfnSuper;
}

BOOL CWnd::PreTranslateMessage(MSG* pMsg)
{
	// handle tooltip messages (some messages cancel, some may cause it to popup)
	AFX_MODULE_STATE* pModuleState = _AFX_CMDTARGET_GETSTATE();
	if (pModuleState->m_pfnFilterToolTipMessage != NULL)
		(*pModuleState->m_pfnFilterToolTipMessage)(pMsg, this);

	// no default processing
	return FALSE;
}

void PASCAL CWnd::CancelToolTips(BOOL bKeys)
{
	// check for active tooltip
	AFX_MODULE_THREAD_STATE* pModuleThreadState = AfxGetModuleThreadState();
	CToolTipCtrl* pToolTip = pModuleThreadState->m_pToolTip;
	if (pToolTip->GetSafeHwnd() != NULL)
		pToolTip->SendMessage(TTM_ACTIVATE, FALSE);

	// check for active control bar fly-by status
	CControlBar* pLastStatus = pModuleThreadState->m_pLastStatus;
	if (bKeys && pLastStatus != NULL && GetKeyState(VK_LBUTTON) >= 0)
		pLastStatus->SetStatusText(static_cast<INT_PTR>(-1));
}

INT_PTR CWnd::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// find child window which hits the point
	// (don't use WindowFromPoint, because it ignores disabled windows)
	HWND hWndChild = _AfxChildWindowFromPoint(m_hWnd, point);
	if (hWndChild != NULL)
	{
		// return positive hit if control ID isn't -1
		INT_PTR nHit = _AfxGetDlgCtrlID(hWndChild);

		// hits against child windows always center the tip
		if (pTI != NULL && pTI->cbSize >= sizeof(AFX_OLDTOOLINFO))
		{
			// setup the TOOLINFO structure
			pTI->hwnd = m_hWnd;
			pTI->uId = (UINT_PTR)hWndChild;
			pTI->uFlags |= TTF_IDISHWND;
			pTI->lpszText = LPSTR_TEXTCALLBACK;

			// set TTF_NOTBUTTON and TTF_CENTERTIP if it isn't a button
			if (!(::SendMessage(hWndChild, WM_GETDLGCODE, 0, 0) & DLGC_BUTTON))
				pTI->uFlags |= TTF_NOTBUTTON|TTF_CENTERTIP;
		}
		return nHit;
	}
	return -1;  // not found
}

void CWnd::GetWindowText(CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));

#ifndef _AFX_NO_OCC_SUPPORT
	if (m_pCtrlSite == NULL)
	{
#endif
		int nLen = ::GetWindowTextLength(m_hWnd);
		::GetWindowText(m_hWnd, rString.GetBufferSetLength(nLen), nLen+1);
		rString.ReleaseBuffer();

#ifndef _AFX_NO_OCC_SUPPORT
	}
	else
	{
		m_pCtrlSite->GetWindowText(rString);
	}
#endif
}

int CWnd::GetDlgItemText(int nID, CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));
	rString = _T("");    // empty without deallocating

#ifndef _AFX_NO_OCC_SUPPORT
	if (m_pCtrlCont == NULL)
	{
#endif
		HWND hWnd = ::GetDlgItem(m_hWnd, nID);
		if (hWnd != NULL)
		{
			int nLen = ::GetWindowTextLength(hWnd);
			::GetWindowText(hWnd, rString.GetBufferSetLength(nLen), nLen+1);
			rString.ReleaseBuffer();
		}

#ifndef _AFX_NO_OCC_SUPPORT
	}
	else
	{
		CWnd* pWnd = GetDlgItem(nID);
		if (pWnd != NULL)
			pWnd->GetWindowText(rString);
	}
#endif

	return (int)rString.GetLength();
}

BOOL CWnd::GetWindowPlacement(WINDOWPLACEMENT* lpwndpl) const
{
	ASSERT(::IsWindow(m_hWnd));
	lpwndpl->length = sizeof(WINDOWPLACEMENT);
	return ::GetWindowPlacement(m_hWnd, lpwndpl);
}

BOOL CWnd::SetWindowPlacement(const WINDOWPLACEMENT* lpwndpl)
{
	ASSERT(::IsWindow(m_hWnd));
	((WINDOWPLACEMENT*)lpwndpl)->length = sizeof(WINDOWPLACEMENT);
	return ::SetWindowPlacement(m_hWnd, lpwndpl);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd will delegate owner draw messages to self drawing controls

// Drawing: for all 4 control types
void CWnd::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (lpDrawItemStruct->CtlType == ODT_MENU)
	{
		CMenu* pMenu = CMenu::FromHandlePermanent(
			(HMENU)lpDrawItemStruct->hwndItem);
		if (pMenu != NULL)
		{
			pMenu->DrawItem(lpDrawItemStruct);
			return; // eat it
		}
	}

	// reflect notification to child window control
	if (ReflectLastMsg(lpDrawItemStruct->hwndItem))
		return;     // eat it

	// not handled - do default
	Default();
}

// Drawing: for all 4 control types
int CWnd::OnCompareItem(int /*nIDCtl*/, LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	// reflect notification to child window control
	LRESULT lResult;
	if (ReflectLastMsg(lpCompareItemStruct->hwndItem, &lResult))
		return (int)lResult;        // eat it

	// not handled - do default
	return (int)Default();
}

void CWnd::OnDeleteItem(int /*nIDCtl*/, LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	// reflect notification to child window control
	if (ReflectLastMsg(lpDeleteItemStruct->hwndItem))
		return;     // eat it
	// not handled - do default
	Default();
}

int CWnd::OnCharToItem(UINT, CListBox* pWnd, UINT)
{
	if (pWnd != NULL)
	{
		LRESULT lResult;
		if (pWnd->SendChildNotifyLastMsg(&lResult))
			return (int)lResult;     // eat it
	}
	// not handled - do default
	return (int)Default();
}

int CWnd::OnVKeyToItem(UINT, CListBox* pWnd, UINT)
{
	if (pWnd != NULL)
	{
		LRESULT lResult;
		if (pWnd->SendChildNotifyLastMsg(&lResult))
			return (int)lResult;     // eat it
	}
	// not handled - do default
	return (int)Default();
}

/////////////////////////////////////////////////////////////////////////////
// Self drawing menus are a little trickier

BOOL CMenu::TrackPopupMenu(UINT nFlags, int x, int y,
		CWnd* pWnd, LPCRECT lpRect)
{
	ASSERT(m_hMenu != NULL);

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	HWND hWndOld = pThreadState->m_hTrackingWindow;
	HMENU hMenuOld = pThreadState->m_hTrackingMenu;
	pThreadState->m_hTrackingWindow = pWnd->GetSafeHwnd();
	pThreadState->m_hTrackingMenu = m_hMenu;
	BOOL bOK = ::TrackPopupMenu(m_hMenu, nFlags, x, y, 0,
			pThreadState->m_hTrackingWindow, lpRect);
	pThreadState->m_hTrackingWindow = hWndOld;
	pThreadState->m_hTrackingMenu = hMenuOld;

	return bOK;
}

BOOL CMenu::TrackPopupMenuEx(UINT fuFlags, int x, int y, CWnd* pWnd, LPTPMPARAMS lptpm)
{
	ASSERT(m_hMenu != NULL);

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	HWND hWndOld = pThreadState->m_hTrackingWindow;
	HMENU hMenuOld = pThreadState->m_hTrackingMenu;
	pThreadState->m_hTrackingWindow = pWnd->GetSafeHwnd();
	pThreadState->m_hTrackingMenu = m_hMenu;
	BOOL bOK = ::TrackPopupMenuEx(m_hMenu, fuFlags, x, y,
			pThreadState->m_hTrackingWindow, lptpm);
	pThreadState->m_hTrackingWindow = hWndOld;
	pThreadState->m_hTrackingMenu = hMenuOld;

	return bOK;
}	

AFX_STATIC CMenu* AFXAPI _AfxFindPopupMenuFromID(CMenu* pMenu, UINT nID)
{
	ENSURE_VALID(pMenu);
	// walk through all items, looking for ID match
	UINT nItems = pMenu->GetMenuItemCount();
	for (int iItem = 0; iItem < (int)nItems; iItem++)
	{
		CMenu* pPopup = pMenu->GetSubMenu(iItem);
		if (pPopup != NULL)
		{
			if(pPopup->m_hMenu == (HMENU)(UINT_PTR)nID) {
				pMenu = CMenu::FromHandlePermanent(pMenu->m_hMenu);
				return pMenu;
			}
			// recurse to child popup
			pPopup = _AfxFindPopupMenuFromID(pPopup, nID);
			// check popups on this popup
			if (pPopup != NULL)
				return pPopup;
		}
		else if (pMenu->GetMenuItemID(iItem) == nID)
		{
			// it is a normal item inside our popup
			pMenu = CMenu::FromHandlePermanent(pMenu->m_hMenu);
			return pMenu;
		}
	}
	// not found
	return NULL;
}

// Measure item implementation relies on unique control/menu IDs
void CWnd::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (lpMeasureItemStruct->CtlType == ODT_MENU)
	{
		ASSERT(lpMeasureItemStruct->CtlID == 0);
		CMenu* pMenu=NULL;

		_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
		if (pThreadState->m_hTrackingWindow == m_hWnd)
		{
			// start from popup
			pMenu = CMenu::FromHandle(pThreadState->m_hTrackingMenu);
		}
		else
		{
			// start from menubar
			pMenu = GetMenu();
		}

		ENSURE(pMenu);

		pMenu = _AfxFindPopupMenuFromID(pMenu, lpMeasureItemStruct->itemID);
		if (pMenu != NULL)
		{
			pMenu->MeasureItem(lpMeasureItemStruct);
		}
		else
		{
			TRACE(traceAppMsg, 0, "Warning: unknown WM_MEASUREITEM for menu item 0x%04X.\n",
				lpMeasureItemStruct->itemID);
		}
	}
	else
	{
		CWnd* pChild = GetDescendantWindow(lpMeasureItemStruct->CtlID, TRUE);
		if (pChild != NULL && pChild->SendChildNotifyLastMsg())
			return;     // eaten by child
	}
	// not handled - do default
	Default();
}

/////////////////////////////////////////////////////////////////////////////
// Additional helpers for WNDCLASS init

// like RegisterClass, except will automatically call UnregisterClass
BOOL AFXAPI AfxRegisterClass(WNDCLASS* lpWndClass)
{
	WNDCLASS wndcls;		
	if (AfxCtxGetClassInfo(lpWndClass->hInstance, lpWndClass->lpszClassName,
		&wndcls))
	{
		// class already registered
		return TRUE;
	}

	if (!::AfxCtxRegisterClass(lpWndClass))
	{
		TRACE(traceAppMsg, 0, _T("Can't register window class named %s\n"),
			lpWndClass->lpszClassName);
		return FALSE;
	}

	BOOL bRet = TRUE;

	if (afxContextIsDLL)
	{
		AfxLockGlobals(CRIT_REGCLASSLIST);
		TRY
		{
			// class registered successfully, add to registered list
			AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
			pModuleState->m_strUnregisterList+=lpWndClass->lpszClassName;
			pModuleState->m_strUnregisterList+='\n';
		}
		CATCH_ALL(e)
		{
			AfxUnlockGlobals(CRIT_REGCLASSLIST);
			THROW_LAST();
			// Note: DELETE_EXCEPTION not required.
		}
		END_CATCH_ALL
		AfxUnlockGlobals(CRIT_REGCLASSLIST);
	}

	return bRet;
}

LPCTSTR AFXAPI AfxRegisterWndClass(UINT nClassStyle,
	HCURSOR hCursor, HBRUSH hbrBackground, HICON hIcon)
{
	// Returns a temporary string name for the class
	//  Save in a CString if you want to use it for a long time
	LPTSTR lpszName = AfxGetThreadState()->m_szTempClassName;

	// generate a synthetic name for this class
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (hCursor == NULL && hbrBackground == NULL && hIcon == NULL)
	{
		ATL_CRT_ERRORCHECK_SPRINTF(_sntprintf_s(lpszName, _AFX_TEMP_CLASS_NAME_SIZE, _AFX_TEMP_CLASS_NAME_SIZE - 1, _T("Afx:%p:%x"), hInst, nClassStyle));
	}
	else
	{
		ATL_CRT_ERRORCHECK_SPRINTF(_sntprintf_s(lpszName, _AFX_TEMP_CLASS_NAME_SIZE, _AFX_TEMP_CLASS_NAME_SIZE - 1, _T("Afx:%p:%x:%p:%p:%p"), hInst, nClassStyle,
			hCursor, hbrBackground, hIcon));
	}
	
	// see if the class already exists
	WNDCLASS wndcls;
	if (::AfxCtxGetClassInfo(hInst, lpszName, &wndcls))
	{
		// already registered, assert everything is good
		ASSERT(wndcls.style == nClassStyle);

		// NOTE: We have to trust that the hIcon, hbrBackground, and the
		//  hCursor are semantically the same, because sometimes Windows does
		//  some internal translation or copying of those handles before
		//  storing them in the internal WNDCLASS retrieved by GetClassInfo.
		return lpszName;
	}

	// otherwise we need to register a new class
	wndcls.style = nClassStyle;
	wndcls.lpfnWndProc = DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hInstance = hInst;
	wndcls.hIcon = hIcon;
	wndcls.hCursor = hCursor;
	wndcls.hbrBackground = hbrBackground;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = lpszName;
	if (!AfxRegisterClass(&wndcls))
		AfxThrowResourceException();

	// return thread-local pointer
	return lpszName;
}

struct AFX_CTLCOLOR
{
	HWND hWnd;
	HDC hDC;
	UINT nCtlType;
};

LRESULT CWnd::OnNTCtlColor(WPARAM wParam, LPARAM lParam)
{
	// fill in special struct for compatiblity with 16-bit WM_CTLCOLOR
	AFX_CTLCOLOR ctl;
	ctl.hDC = (HDC)wParam;
	ctl.hWnd = (HWND)lParam;
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	ctl.nCtlType = pThreadState->m_lastSentMsg.message - WM_CTLCOLORMSGBOX;
	//ASSERT(ctl.nCtlType >= CTLCOLOR_MSGBOX);
	ASSERT(ctl.nCtlType <= CTLCOLOR_STATIC);

	// Note: We call the virtual WindowProc for this window directly,
	//  instead of calling AfxCallWindowProc, so that Default()
	//  will still work (it will call the Default window proc with
	//  the original Win32 WM_CTLCOLOR message).
	return WindowProc(WM_CTLCOLOR, 0, (LPARAM)&ctl);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd extensions for help support

void CWnd::WinHelp(DWORD_PTR dwData, UINT nCmd)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	ASSERT(pApp->m_pszHelpFilePath != NULL);
	ASSERT(pApp->m_eHelpType == afxWinHelp);

	CWaitCursor wait;

	PrepareForHelp();

	// need to use top level parent (for the case where m_hWnd is in DLL)
	CWnd* pWnd = EnsureTopLevelParent();

	TRACE(traceAppMsg, 0, _T("WinHelp: pszHelpFile = '%s', dwData: $%lx, fuCommand: %d.\n"), pApp->m_pszHelpFilePath, dwData, nCmd);

	// finally, run the Windows Help engine
	if (!::WinHelp(pWnd->m_hWnd, pApp->m_pszHelpFilePath, nCmd, dwData))
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
	}
}

void CWnd::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	ASSERT(pApp->m_pszHelpFilePath != NULL);
	// to call HtmlHelp the m_fUseHtmlHelp must be set in
	// the application's constructor
	ASSERT(pApp->m_eHelpType == afxHTMLHelp);

	CWaitCursor wait;

	PrepareForHelp();

	// need to use top level parent (for the case where m_hWnd is in DLL)
	CWnd* pWnd = EnsureTopLevelParent();

	TRACE(traceAppMsg, 0, _T("HtmlHelp: pszHelpFile = '%s', dwData: $%lx, fuCommand: %d.\n"), pApp->m_pszHelpFilePath, dwData, nCmd);

	// run the HTML Help engine
	if (!AfxHtmlHelp(pWnd->m_hWnd, pApp->m_pszHelpFilePath, nCmd, dwData))
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
	}
}

void CWnd::PrepareForHelp()
{
	if (IsFrameWnd())
	{
		// CFrameWnd windows should be allowed to exit help mode first
		CFrameWnd* pFrameWnd = static_cast<CFrameWnd*>(this);
		pFrameWnd->ExitHelpMode();
	}

	// cancel any tracking modes
	SendMessage(WM_CANCELMODE);
	SendMessageToDescendants(WM_CANCELMODE, 0, 0, TRUE, TRUE);

	// need to use top level parent (for the case where m_hWnd is in DLL)
	CWnd* pWnd = EnsureTopLevelParent();
	pWnd->SendMessage(WM_CANCELMODE);
	pWnd->SendMessageToDescendants(WM_CANCELMODE, 0, 0, TRUE, TRUE);

	// attempt to cancel capture
	HWND hWndCapture = ::GetCapture();
	if (hWndCapture != NULL)
		::SendMessage(hWndCapture, WM_CANCELMODE, 0, 0);
}

// for dll builds we just delay load it
#ifndef _AFXDLL
_AFX_HTMLHELP_STATE::~_AFX_HTMLHELP_STATE()
{
	if (m_hInstHtmlHelp)
		FreeLibrary(m_hInstHtmlHelp);
}
#endif

HWND WINAPI AfxHtmlHelp(HWND hWnd, LPCTSTR szHelpFilePath, UINT nCmd, DWORD_PTR dwData)
{
// for dll builds we just delay load it
#ifndef _AFXDLL
#ifdef _UNICODE
	#define _HTMLHELP_ENTRY "HtmlHelpW"
#else
	#define _HTMLHELP_ENTRY "HtmlHelpA"
#endif

	AfxLockGlobals(CRIT_DYNDLLLOAD);
	// check if the HtmlHelp library was loaded
	_AFX_HTMLHELP_STATE* pState = _afxHtmlHelpState.GetData();
	if (!pState->m_pfnHtmlHelp)
	{
		// load the library
		ASSERT(!pState->m_hInstHtmlHelp);
		pState->m_hInstHtmlHelp = AfxCtxLoadLibraryA("hhctrl.ocx");
		if (!pState->m_hInstHtmlHelp)
			return NULL;
		pState->m_pfnHtmlHelp = (HTMLHELPPROC *) GetProcAddress(pState->m_hInstHtmlHelp, _HTMLHELP_ENTRY);
		if (!pState->m_pfnHtmlHelp)
		{
			FreeLibrary(pState->m_hInstHtmlHelp);
			pState->m_hInstHtmlHelp = NULL;
			return NULL;
		}
	}
	AfxUnlockGlobals(CRIT_DYNDLLLOAD);
	// now call it
	return (*(pState->m_pfnHtmlHelp))(hWnd, szHelpFilePath, nCmd, dwData);
#else
	return ::HtmlHelp(hWnd, szHelpFilePath, nCmd, dwData);
#endif
}

void CWnd::WinHelpInternal(DWORD_PTR dwData, UINT nCmd)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	if (pApp->m_eHelpType == afxHTMLHelp)
	{
		// translate from WinHelp commands and data to to HtmlHelp
		ASSERT((nCmd == HELP_CONTEXT) || (nCmd == HELP_CONTENTS) || (nCmd == HELP_FINDER));
		if (nCmd == HELP_CONTEXT)
			nCmd = HH_HELP_CONTEXT;
		else if (nCmd == HELP_CONTENTS)
			nCmd = HH_DISPLAY_TOC;
		else if (nCmd == HELP_FINDER)
			nCmd = HH_HELP_FINDER;
		HtmlHelp(dwData, nCmd);
	}
	else
		WinHelp(dwData, nCmd);
}

/////////////////////////////////////////////////////////////////////////////
// Message table implementation

BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
	ON_MESSAGE(WM_CTLCOLORSTATIC, &CWnd::OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLOREDIT, &CWnd::OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORBTN, &CWnd::OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORLISTBOX, &CWnd::OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORDLG, &CWnd::OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORMSGBOX, &CWnd::OnNTCtlColor)
	ON_MESSAGE(WM_CTLCOLORSCROLLBAR, &CWnd::OnNTCtlColor)
	//{{AFX_MSG_MAP(CWnd)
   ON_WM_SETFOCUS()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_WM_CTLCOLOR()
	ON_WM_COMPAREITEM()
	ON_WM_ENTERIDLE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DELETEITEM()
	ON_WM_CHARTOITEM()
	ON_WM_VKEYTOITEM()
	ON_WM_NCDESTROY()
	ON_WM_PARENTNOTIFY()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_DEVMODECHANGE()
	ON_WM_HELPINFO()
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
#ifndef _AFX_NO_OCC_SUPPORT
	ON_WM_DESTROY()
#endif
	ON_MESSAGE(WM_ACTIVATETOPLEVEL, &CWnd::OnActivateTopLevel)
	ON_MESSAGE(WM_DISPLAYCHANGE, &CWnd::OnDisplayChange)
	ON_REGISTERED_MESSAGE(CWnd::m_nMsgDragList, &CWnd::OnDragList)
	ON_MESSAGE(WM_GETOBJECT, &CWnd::OnGetObject)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Routines for fast search of message maps

const AFX_MSGMAP_ENTRY* AFXAPI
AfxFindMessageEntry(const AFX_MSGMAP_ENTRY* lpEntry,
	UINT nMsg, UINT nCode, UINT nID)
{
#if defined(_M_IX86) && !defined(_AFX_PORTABLE)
// 32-bit Intel 386/486 version.

	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nMessage) == 0);
	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nCode) == 4);
	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nID) == 8);
	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nLastID) == 12);
	ASSERT(offsetof(AFX_MSGMAP_ENTRY, nSig) == 16);

	_asm
	{
			MOV     EBX,lpEntry
			MOV     EAX,nMsg
			MOV     EDX,nCode
			MOV     ECX,nID
	__loop:
			CMP     DWORD PTR [EBX+16],0        ; nSig (0 => end)
			JZ      __failed
			CMP     EAX,DWORD PTR [EBX]         ; nMessage
			JE      __found_message
	__next:
			ADD     EBX,SIZE AFX_MSGMAP_ENTRY
			JMP     short __loop
	__found_message:
			CMP     EDX,DWORD PTR [EBX+4]       ; nCode
			JNE     __next
	// message and code good so far
	// check the ID
			CMP     ECX,DWORD PTR [EBX+8]       ; nID
			JB      __next
			CMP     ECX,DWORD PTR [EBX+12]      ; nLastID
			JA      __next
	// found a match
			MOV     lpEntry,EBX                 ; return EBX
			JMP     short __end
	__failed:
			XOR     EAX,EAX                     ; return NULL
			MOV     lpEntry,EAX
	__end:
	}
	return lpEntry;
#else  // _AFX_PORTABLE
	// C version of search routine
	while (lpEntry->nSig != AfxSig_end)
	{
		if (lpEntry->nMessage == nMsg && lpEntry->nCode == nCode &&
			nID >= lpEntry->nID && nID <= lpEntry->nLastID)
		{
			return lpEntry;
		}
		lpEntry++;
	}
	return NULL;    // not found
#endif  // _AFX_PORTABLE
}

/////////////////////////////////////////////////////////////////////////////
// Cache of most recently sent messages

#ifndef iHashMax
// iHashMax must be a power of two
	#define iHashMax 512
#endif

struct AFX_MSG_CACHE
{
	UINT nMsg;
	const AFX_MSGMAP_ENTRY* lpEntry;
	const AFX_MSGMAP* pMessageMap;
};

AFX_MSG_CACHE _afxMsgCache[iHashMax];

void AFXAPI AfxResetMsgCache()
{
	memset(_afxMsgCache, 0, sizeof(_afxMsgCache));
}

/////////////////////////////////////////////////////////////////////////////
// main WindowProc implementation

LRESULT CWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// OnWndMsg does most of the work, except for DefWindowProc call
	LRESULT lResult = 0;
	if (!OnWndMsg(message, wParam, lParam, &lResult))
		lResult = DefWindowProc(message, wParam, lParam);
	return lResult;
}

BOOL CWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LRESULT lResult = 0;
	union MessageMapFunctions mmf;
	mmf.pfn = 0;
	CInternalGlobalLock winMsgLock;
	// special case for commands
	if (message == WM_COMMAND)
	{
		if (OnCommand(wParam, lParam))
		{
			lResult = 1;
			goto LReturnTrue;
		}
		return FALSE;
	}

	// special case for notifies
	if (message == WM_NOTIFY)
	{
		NMHDR* pNMHDR = (NMHDR*)lParam;
		if (pNMHDR->hwndFrom != NULL && OnNotify(wParam, lParam, &lResult))
			goto LReturnTrue;
		return FALSE;
	}

	// special case for activation
	if (message == WM_ACTIVATE)
		_AfxHandleActivate(this, wParam, CWnd::FromHandle((HWND)lParam));

	// special case for set cursor HTERROR
	if (message == WM_SETCURSOR &&
		_AfxHandleSetCursor(this, (short)LOWORD(lParam), HIWORD(lParam)))
	{
		lResult = 1;
		goto LReturnTrue;
	}

   // special case for windows that contain windowless ActiveX controls
   BOOL bHandled;

   bHandled = FALSE;
   if ((m_pCtrlCont != NULL) && (m_pCtrlCont->m_nWindowlessControls > 0))
   {
	  if (((message >= WM_MOUSEFIRST) && (message <= AFX_WM_MOUSELAST)) ||
		 ((message >= WM_KEYFIRST) && (message <= WM_IME_KEYLAST)) ||
		 ((message >= WM_IME_SETCONTEXT) && (message <= WM_IME_KEYUP)))
	  {
		 bHandled = m_pCtrlCont->HandleWindowlessMessage(message, wParam, lParam, &lResult);
	  }
   }
   if (bHandled)
   {
	  goto LReturnTrue;
   }

	const AFX_MSGMAP* pMessageMap; pMessageMap = GetMessageMap();
	UINT iHash; iHash = (LOWORD((DWORD_PTR)pMessageMap) ^ message) & (iHashMax-1);
	winMsgLock.Lock(CRIT_WINMSGCACHE);
	AFX_MSG_CACHE* pMsgCache; pMsgCache = &_afxMsgCache[iHash];
	const AFX_MSGMAP_ENTRY* lpEntry;
	if (message == pMsgCache->nMsg && pMessageMap == pMsgCache->pMessageMap)
	{
		// cache hit
		lpEntry = pMsgCache->lpEntry;
		winMsgLock.Unlock();
		if (lpEntry == NULL)
			return FALSE;

		// cache hit, and it needs to be handled
		if (message < 0xC000)
			goto LDispatch;
		else
			goto LDispatchRegistered;
	}
	else
	{
		// not in cache, look for it
		pMsgCache->nMsg = message;
		pMsgCache->pMessageMap = pMessageMap;

		for (/* pMessageMap already init'ed */; pMessageMap->pfnGetBaseMap != NULL;
			pMessageMap = (*pMessageMap->pfnGetBaseMap)())
		{
			// Note: catch not so common but fatal mistake!!
			//      BEGIN_MESSAGE_MAP(CMyWnd, CMyWnd)
			ASSERT(pMessageMap != (*pMessageMap->pfnGetBaseMap)());
			if (message < 0xC000)
			{
				// constant window message
				if ((lpEntry = AfxFindMessageEntry(pMessageMap->lpEntries,
					message, 0, 0)) != NULL)
				{
					pMsgCache->lpEntry = lpEntry;
					winMsgLock.Unlock();
					goto LDispatch;
				}
			}
			else
			{
				// registered windows message
				lpEntry = pMessageMap->lpEntries;
				while ((lpEntry = AfxFindMessageEntry(lpEntry, 0xC000, 0, 0)) != NULL)
				{
					UINT* pnID = (UINT*)(lpEntry->nSig);
					ASSERT(*pnID >= 0xC000 || *pnID == 0);
						// must be successfully registered
					if (*pnID == message)
					{
						pMsgCache->lpEntry = lpEntry;
						winMsgLock.Unlock();
						goto LDispatchRegistered;
					}
					lpEntry++;      // keep looking past this one
				}
			}
		}

		pMsgCache->lpEntry = NULL;
		winMsgLock.Unlock();
		return FALSE;
	}

LDispatch:
	ASSERT(message < 0xC000);

	mmf.pfn = lpEntry->pfn;

	switch (lpEntry->nSig)
	{
	default:
		ASSERT(FALSE);
		break;
	case AfxSig_l_p:
		{
			CPoint point(lParam);		
			lResult = (this->*mmf.pfn_l_p)(point);
			break;
		}		
	case AfxSig_b_D_v:
		lResult = (this->*mmf.pfn_b_D)(CDC::FromHandle(reinterpret_cast<HDC>(wParam)));
		break;

	case AfxSig_b_b_v:
		lResult = (this->*mmf.pfn_b_b)(static_cast<BOOL>(wParam));
		break;

	case AfxSig_b_u_v:
		lResult = (this->*mmf.pfn_b_u)(static_cast<UINT>(wParam));
		break;

	case AfxSig_b_h_v:
		lResult = (this->*mmf.pfn_b_h)(reinterpret_cast<HANDLE>(wParam));
		break;

	case AfxSig_i_u_v:
		lResult = (this->*mmf.pfn_i_u)(static_cast<UINT>(wParam));
		break;

	case AfxSig_C_v_v:
		lResult = reinterpret_cast<LRESULT>((this->*mmf.pfn_C_v)());
		break;

	case AfxSig_v_u_W:
		(this->*mmf.pfn_v_u_W)(static_cast<UINT>(wParam), 
			CWnd::FromHandle(reinterpret_cast<HWND>(lParam)));
		break;

	case AfxSig_u_u_v:
		lResult = (this->*mmf.pfn_u_u)(static_cast<UINT>(wParam));
		break;

	case AfxSig_b_v_v:
		lResult = (this->*mmf.pfn_b_v)();
		break;

	case AfxSig_b_W_uu:
		lResult = (this->*mmf.pfn_b_W_u_u)(CWnd::FromHandle(reinterpret_cast<HWND>(wParam)),
			LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_b_W_COPYDATASTRUCT:
		lResult = (this->*mmf.pfn_b_W_COPYDATASTRUCT)(
			CWnd::FromHandle(reinterpret_cast<HWND>(wParam)),
			reinterpret_cast<COPYDATASTRUCT*>(lParam));
		break;

	case AfxSig_b_v_HELPINFO:
		lResult = (this->*mmf.pfn_b_HELPINFO)(reinterpret_cast<LPHELPINFO>(lParam));
		break;

	case AfxSig_CTLCOLOR:
		{
			// special case for OnCtlColor to avoid too many temporary objects
			ASSERT(message == WM_CTLCOLOR);
			AFX_CTLCOLOR* pCtl = reinterpret_cast<AFX_CTLCOLOR*>(lParam);
			CDC dcTemp; 
			dcTemp.m_hDC = pCtl->hDC;
			CWnd wndTemp; 
			wndTemp.m_hWnd = pCtl->hWnd;
			UINT nCtlType = pCtl->nCtlType;
			// if not coming from a permanent window, use stack temporary
			CWnd* pWnd = CWnd::FromHandlePermanent(wndTemp.m_hWnd);
			if (pWnd == NULL)
			{
#ifndef _AFX_NO_OCC_SUPPORT
				// determine the site of the OLE control if it is one
				COleControlSite* pSite;
				if (m_pCtrlCont != NULL && (pSite = (COleControlSite*)
					m_pCtrlCont->m_siteMap.GetValueAt(wndTemp.m_hWnd)) != NULL)
				{
					wndTemp.m_pCtrlSite = pSite;
				}
#endif
				pWnd = &wndTemp;
			}
			HBRUSH hbr = (this->*mmf.pfn_B_D_W_u)(&dcTemp, pWnd, nCtlType);
			// fast detach of temporary objects
			dcTemp.m_hDC = NULL;
			wndTemp.m_hWnd = NULL;
			lResult = reinterpret_cast<LRESULT>(hbr);
		}
		break;

	case AfxSig_CTLCOLOR_REFLECT:
		{
			// special case for CtlColor to avoid too many temporary objects
			ASSERT(message == WM_REFLECT_BASE+WM_CTLCOLOR);
			AFX_CTLCOLOR* pCtl = reinterpret_cast<AFX_CTLCOLOR*>(lParam);
			CDC dcTemp; 
			dcTemp.m_hDC = pCtl->hDC;
			UINT nCtlType = pCtl->nCtlType;
			HBRUSH hbr = (this->*mmf.pfn_B_D_u)(&dcTemp, nCtlType);
			// fast detach of temporary objects
			dcTemp.m_hDC = NULL;
			lResult = reinterpret_cast<LRESULT>(hbr);
		}
		break;

	case AfxSig_i_u_W_u:
		lResult = (this->*mmf.pfn_i_u_W_u)(LOWORD(wParam),
			CWnd::FromHandle(reinterpret_cast<HWND>(lParam)), HIWORD(wParam));
		break;

	case AfxSig_i_uu_v:
		lResult = (this->*mmf.pfn_i_u_u)(LOWORD(wParam), HIWORD(wParam));
		break;

	case AfxSig_i_W_uu:
		lResult = (this->*mmf.pfn_i_W_u_u)(CWnd::FromHandle(reinterpret_cast<HWND>(wParam)),
			LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_i_v_s:
		lResult = (this->*mmf.pfn_i_s)(reinterpret_cast<LPTSTR>(lParam));
		break;

	case AfxSig_l_w_l:
		lResult = (this->*mmf.pfn_l_w_l)(wParam, lParam);
		break;

	case AfxSig_l_uu_M:
		lResult = (this->*mmf.pfn_l_u_u_M)(LOWORD(wParam), HIWORD(wParam), 
			CMenu::FromHandle(reinterpret_cast<HMENU>(lParam)));
		break;
		
	case AfxSig_v_b_h:
	    (this->*mmf.pfn_v_b_h)(static_cast<BOOL>(wParam), 
			reinterpret_cast<HANDLE>(lParam));
		break;

	case AfxSig_v_h_v:
	    (this->*mmf.pfn_v_h)(reinterpret_cast<HANDLE>(wParam));
		break;

	case AfxSig_v_h_h:
	    (this->*mmf.pfn_v_h_h)(reinterpret_cast<HANDLE>(wParam), 
			reinterpret_cast<HANDLE>(lParam));
		break;

	case AfxSig_v_v_v:
		(this->*mmf.pfn_v_v)();
		break;

	case AfxSig_v_u_v:
		(this->*mmf.pfn_v_u)(static_cast<UINT>(wParam));
		break;

	case AfxSig_v_u_u:
		(this->*mmf.pfn_v_u_u)(static_cast<UINT>(wParam), static_cast<UINT>(lParam));
		break;

	case AfxSig_v_uu_v:
		(this->*mmf.pfn_v_u_u)(LOWORD(wParam), HIWORD(wParam));
		break;

	case AfxSig_v_v_ii:
		(this->*mmf.pfn_v_i_i)(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case AfxSig_v_u_uu:
		(this->*mmf.pfn_v_u_u_u)(static_cast<UINT>(wParam), LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_v_u_ii:
		(this->*mmf.pfn_v_u_i_i)(static_cast<UINT>(wParam), LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_v_w_l:
		(this->*mmf.pfn_v_w_l)(wParam, lParam);
		break;

	case AfxSig_MDIACTIVATE:
		(this->*mmf.pfn_v_b_W_W)(m_hWnd == reinterpret_cast<HWND>(lParam),
			CWnd::FromHandle(reinterpret_cast<HWND>(lParam)),
			CWnd::FromHandle(reinterpret_cast<HWND>(wParam)));
		break;

	case AfxSig_v_D_v:
		(this->*mmf.pfn_v_D)(CDC::FromHandle(reinterpret_cast<HDC>(wParam)));
		break;

	case AfxSig_v_M_v:
		(this->*mmf.pfn_v_M)(CMenu::FromHandle(reinterpret_cast<HMENU>(wParam)));
		break;

	case AfxSig_v_M_ub:
		(this->*mmf.pfn_v_M_u_b)(CMenu::FromHandle(reinterpret_cast<HMENU>(wParam)),
			GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case AfxSig_v_W_v:
		(this->*mmf.pfn_v_W)(CWnd::FromHandle(reinterpret_cast<HWND>(wParam)));
		break;

	case AfxSig_v_v_W:
		(this->*mmf.pfn_v_W)(CWnd::FromHandle(reinterpret_cast<HWND>(lParam)));
		break;

	case AfxSig_v_W_uu:
		(this->*mmf.pfn_v_W_u_u)(CWnd::FromHandle(reinterpret_cast<HWND>(wParam)), LOWORD(lParam),
			HIWORD(lParam));
		break;

	case AfxSig_v_W_p:
		{
			CPoint point(lParam);
			(this->*mmf.pfn_v_W_p)(CWnd::FromHandle(reinterpret_cast<HWND>(wParam)), point);
		}
		break;

	case AfxSig_v_W_h:
		(this->*mmf.pfn_v_W_h)(CWnd::FromHandle(reinterpret_cast<HWND>(wParam)),
				reinterpret_cast<HANDLE>(lParam));
		break;

	case AfxSig_ACTIVATE:
		(this->*mmf.pfn_v_u_W_b)(LOWORD(wParam),
			CWnd::FromHandle(reinterpret_cast<HWND>(lParam)), HIWORD(wParam));
		break;

	case AfxSig_SCROLL:
	case AfxSig_SCROLL_REFLECT:
		{
			// special case for WM_VSCROLL and WM_HSCROLL
			ASSERT(message == WM_VSCROLL || message == WM_HSCROLL ||
				message == WM_VSCROLL+WM_REFLECT_BASE || message == WM_HSCROLL+WM_REFLECT_BASE);
			int nScrollCode = (short)LOWORD(wParam);
			int nPos = (short)HIWORD(wParam);
			if (lpEntry->nSig == AfxSig_SCROLL)
				(this->*mmf.pfn_v_u_u_W)(nScrollCode, nPos,
					CWnd::FromHandle(reinterpret_cast<HWND>(lParam)));
			else
				(this->*mmf.pfn_v_u_u)(nScrollCode, nPos);
		}
		break;

	case AfxSig_v_v_s:
		(this->*mmf.pfn_v_s)(reinterpret_cast<LPTSTR>(lParam));
		break;

	case AfxSig_v_u_cs:
		(this->*mmf.pfn_v_u_cs)(static_cast<UINT>(wParam), reinterpret_cast<LPCTSTR>(lParam));
		break;

	case AfxSig_OWNERDRAW:
		(this->*mmf.pfn_v_i_s)(static_cast<int>(wParam), reinterpret_cast<LPTSTR>(lParam));
		lResult = TRUE;
		break;

	case AfxSig_i_i_s:
		lResult = (this->*mmf.pfn_i_i_s)(static_cast<int>(wParam), reinterpret_cast<LPTSTR>(lParam));
		break;

	case AfxSig_u_v_p:
		{
			CPoint point(lParam);
			lResult = (this->*mmf.pfn_u_p)(point);
		}
		break;

	case AfxSig_u_v_v:
		lResult = (this->*mmf.pfn_u_v)();
		break;

	case AfxSig_v_b_NCCALCSIZEPARAMS:
		(this->*mmf.pfn_v_b_NCCALCSIZEPARAMS)(static_cast<BOOL>(wParam), 
			reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam));
		break;

	case AfxSig_v_v_WINDOWPOS:
		(this->*mmf.pfn_v_v_WINDOWPOS)(reinterpret_cast<WINDOWPOS*>(lParam));
		break;

	case AfxSig_v_uu_M:
		(this->*mmf.pfn_v_u_u_M)(LOWORD(wParam), HIWORD(wParam), reinterpret_cast<HMENU>(lParam));
		break;

	case AfxSig_v_u_p:
		{
			CPoint point(lParam);
			(this->*mmf.pfn_v_u_p)(static_cast<UINT>(wParam), point);
		}
		break;

	case AfxSig_SIZING:
		(this->*mmf.pfn_v_u_pr)(static_cast<UINT>(wParam), reinterpret_cast<LPRECT>(lParam));
		lResult = TRUE;
		break;

	case AfxSig_MOUSEWHEEL:
		lResult = (this->*mmf.pfn_b_u_s_p)(LOWORD(wParam), (short)HIWORD(wParam),
			CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		if (!lResult)
			return FALSE;
		break;
	case AfxSig_MOUSEHWHEEL:
		(this->*mmf.pfn_MOUSEHWHEEL)(LOWORD(wParam), (short)HIWORD(wParam),
			CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		break;
	case AfxSig_l:
		lResult = (this->*mmf.pfn_l_v)();
		if (lResult != 0)
			return FALSE;
		break;
	case AfxSig_u_W_u:
		lResult = (this->*mmf.pfn_u_W_u)(CWnd::FromHandle(reinterpret_cast<HWND>(wParam)), static_cast<UINT>(lParam));
		break;
	case AfxSig_v_u_M:
		(this->*mmf.pfn_v_u_M)(static_cast<UINT>(wParam), CMenu::FromHandle(reinterpret_cast<HMENU>(lParam)));
		break;
	case AfxSig_u_u_M:
		lResult = (this->*mmf.pfn_u_u_M)(static_cast<UINT>(wParam), CMenu::FromHandle(reinterpret_cast<HMENU>(lParam)));
		break;
	case AfxSig_u_v_MENUGETOBJECTINFO:
		lResult = (this->*mmf.pfn_u_v_MENUGETOBJECTINFO)(reinterpret_cast<MENUGETOBJECTINFO*>(lParam));
		break;
	case AfxSig_v_M_u:
		(this->*mmf.pfn_v_M_u)(CMenu::FromHandle(reinterpret_cast<HMENU>(wParam)), static_cast<UINT>(lParam));
		break;
	case AfxSig_v_u_LPMDINEXTMENU:
		(this->*mmf.pfn_v_u_LPMDINEXTMENU)(static_cast<UINT>(wParam), reinterpret_cast<LPMDINEXTMENU>(lParam));
		break;
	case AfxSig_APPCOMMAND:
		(this->*mmf.pfn_APPCOMMAND)(CWnd::FromHandle(reinterpret_cast<HWND>(wParam)), static_cast<UINT>(GET_APPCOMMAND_LPARAM(lParam)), static_cast<UINT>(GET_DEVICE_LPARAM(lParam)), static_cast<UINT>(GET_KEYSTATE_LPARAM(lParam)));
		lResult = TRUE;
		break;
	case AfxSig_RAWINPUT:
		(this->*mmf.pfn_RAWINPUT)(static_cast<UINT>(GET_RAWINPUT_CODE_WPARAM(wParam)), reinterpret_cast<HRAWINPUT>(lParam));
		break;
	case AfxSig_u_u_u:
		lResult = (this->*mmf.pfn_u_u_u)(static_cast<UINT>(wParam), static_cast<UINT>(lParam));
		break;
	case AfxSig_MOUSE_XBUTTON:
		(this->*mmf.pfn_MOUSE_XBUTTON)(static_cast<UINT>(GET_KEYSTATE_WPARAM(wParam)), static_cast<UINT>(GET_XBUTTON_WPARAM(wParam)), CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		lResult = TRUE;
		break;
	case AfxSig_MOUSE_NCXBUTTON:
		(this->*mmf.pfn_MOUSE_NCXBUTTON)(static_cast<short>(GET_NCHITTEST_WPARAM(wParam)), static_cast<UINT>(GET_XBUTTON_WPARAM(wParam)), CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		lResult = TRUE;
		break;
	case AfxSig_INPUTLANGCHANGE:
		(this->*mmf.pfn_INPUTLANGCHANGE)(static_cast<BYTE>(wParam), static_cast<UINT>(lParam));
		lResult = TRUE;
		break;
	case AfxSig_INPUTDEVICECHANGE:
		(this->*mmf.pfn_INPUTDEVICECHANGE)(GET_DEVICE_CHANGE_LPARAM(wParam));
		break;
	case AfxSig_v_u_hkl:
		(this->*mmf.pfn_v_u_h)(static_cast<UINT>(wParam), reinterpret_cast<HKL>(lParam));
		break;
	}
	goto LReturnTrue;

LDispatchRegistered:    // for registered windows messages
	ASSERT(message >= 0xC000);
	ASSERT(sizeof(mmf) == sizeof(mmf.pfn));
	mmf.pfn = lpEntry->pfn;
	lResult = (this->*mmf.pfn_l_w_l)(wParam, lParam);

LReturnTrue:
	if (pResult != NULL)
		*pResult = lResult;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTestCmdUI - used to test for disabled commands before dispatching

class CTestCmdUI : public CCmdUI
{
public:
	CTestCmdUI();

public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetRadio(BOOL bOn);
	virtual void SetText(LPCTSTR);

	BOOL m_bEnabled;
};

CTestCmdUI::CTestCmdUI()
{
	m_bEnabled = TRUE;  // assume it is enabled
}

void CTestCmdUI::Enable(BOOL bOn)
{
	m_bEnabled = bOn;
	m_bEnableChanged = TRUE;
}

void CTestCmdUI::SetCheck(int)
{
	// do nothing -- just want to know about calls to Enable
}

void CTestCmdUI::SetRadio(BOOL)
{
	// do nothing -- just want to know about calls to Enable
}

void CTestCmdUI::SetText(LPCTSTR)
{
	// do nothing -- just want to know about calls to Enable
}

/////////////////////////////////////////////////////////////////////////////
// CWnd command handling

BOOL CWnd::OnCommand(WPARAM wParam, LPARAM lParam)
	// return TRUE if command invocation was attempted
{
	UINT nID = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	int nCode = HIWORD(wParam);

	// default routing for command messages (through closure table)

	if (hWndCtrl == NULL)
	{
		// zero IDs for normal commands are not allowed
		if (nID == 0)
			return FALSE;

		// make sure command has not become disabled before routing
		CTestCmdUI state;
		state.m_nID = nID;
		OnCmdMsg(nID, CN_UPDATE_COMMAND_UI, &state, NULL);
		if (!state.m_bEnabled)
		{
			TRACE(traceAppMsg, 0, "Warning: not executing disabled command %d\n", nID);
			return TRUE;
		}

		// menu or accelerator
		nCode = CN_COMMAND;
	}
	else
	{
		// control notification
		ASSERT(nID == 0 || ::IsWindow(hWndCtrl));

		if (_afxThreadState->m_hLockoutNotifyWindow == m_hWnd)
			return TRUE;        // locked out - ignore control notification

		// reflect notification to child window control
		if (ReflectLastMsg(hWndCtrl))
			return TRUE;    // eaten by child

		// zero IDs for normal commands are not allowed
		if (nID == 0)
			return FALSE;
	}

#ifdef _DEBUG
	if (nCode < 0 && nCode != (int)0x8000)
		TRACE(traceAppMsg, 0, "Implementation Warning: control notification = $%X.\n",
			nCode);
#endif

	return OnCmdMsg(nID, nCode, NULL, NULL);
}

BOOL CWnd::OnNotify(WPARAM, LPARAM lParam, LRESULT* pResult)
{
	ASSERT(pResult != NULL);
	NMHDR* pNMHDR = (NMHDR*)lParam;
	HWND hWndCtrl = pNMHDR->hwndFrom;

	// get the child ID from the window itself
	UINT_PTR nID = _AfxGetDlgCtrlID(hWndCtrl);
	int nCode = pNMHDR->code;

	ASSERT(hWndCtrl != NULL);
	ASSERT(::IsWindow(hWndCtrl));

	if (_afxThreadState->m_hLockoutNotifyWindow == m_hWnd)
		return TRUE;        // locked out - ignore control notification

	// reflect notification to child window control
	if (ReflectLastMsg(hWndCtrl, pResult))
		return TRUE;        // eaten by child

	AFX_NOTIFY notify;
	notify.pResult = pResult;
	notify.pNMHDR = pNMHDR;
	return OnCmdMsg((UINT)nID, MAKELONG(nCode, WM_NOTIFY), &notify, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd extensions

CFrameWnd* CWnd::GetParentFrame() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
	{
		return NULL;
	}

	ASSERT_VALID(this);

	CWnd* pParentWnd = GetParent();  // start with one parent up
	while (pParentWnd != NULL)
	{
		if (pParentWnd->IsFrameWnd())
		{
			return (CFrameWnd*)pParentWnd;
		}
		pParentWnd = pParentWnd->GetParent();
	}
	return NULL;
}

HWND AFXAPI AfxGetParentOwner(HWND hWnd)
{
	// check for permanent-owned window first
	CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
	if (pWnd != NULL)
		return pWnd->GetOwner()->GetSafeHwnd();

	// otherwise, return parent in the Windows sense
	return (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) ?
		::GetParent(hWnd) : ::GetWindow(hWnd, GW_OWNER);
}

CWnd* CWnd::GetTopLevelParent() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	HWND hWndParent = m_hWnd;
	HWND hWndT;
	while ((hWndT = AfxGetParentOwner(hWndParent)) != NULL)
		hWndParent = hWndT;

	return CWnd::FromHandle(hWndParent);
}

CWnd* CWnd::GetTopLevelOwner() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	HWND hWndOwner = m_hWnd;
	HWND hWndT;
	while ((hWndT = ::GetWindow(hWndOwner, GW_OWNER)) != NULL)
		hWndOwner = hWndT;

	return CWnd::FromHandle(hWndOwner);
}

CWnd* CWnd::GetParentOwner() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	HWND hWndParent = m_hWnd;
	HWND hWndT;
	while ((::GetWindowLong(hWndParent, GWL_STYLE) & WS_CHILD) &&
		(hWndT = ::GetParent(hWndParent)) != NULL)
	{
		hWndParent = hWndT;
	}

	return CWnd::FromHandle(hWndParent);
}

BOOL CWnd::IsTopParentActive() const
{
	ASSERT(m_hWnd != NULL);
	ASSERT_VALID(this);

    CWnd *pWndTopLevel=EnsureTopLevelParent();

	return CWnd::GetForegroundWindow() == pWndTopLevel->GetLastActivePopup();
}

void CWnd::ActivateTopParent()
{
	// special activate logic for floating toolbars and palettes
	CWnd* pActiveWnd = GetForegroundWindow();
	if (pActiveWnd == NULL || !(pActiveWnd->m_hWnd == m_hWnd || ::IsChild(pActiveWnd->m_hWnd, m_hWnd)))
	{
		// clicking on floating frame when it does not have
		// focus itself -- activate the toplevel frame instead.
		EnsureTopLevelParent()->SetForegroundWindow();
	}
}

CFrameWnd* CWnd::GetTopLevelFrame() const
{
	if (GetSafeHwnd() == NULL) // no Window attached
		return NULL;

	ASSERT_VALID(this);

	CFrameWnd* pFrameWnd = (CFrameWnd*)this;
	if (!IsFrameWnd())
		pFrameWnd = GetParentFrame();

	if (pFrameWnd != NULL)
	{
		CFrameWnd* pTemp;
		while ((pTemp = pFrameWnd->GetParentFrame()) != NULL)
			pFrameWnd = pTemp;
	}
	return pFrameWnd;
}

CWnd* PASCAL CWnd::GetSafeOwner(CWnd* pParent, HWND* pWndTop)
{
	HWND hWnd = GetSafeOwner_(pParent->GetSafeHwnd(), pWndTop);
	return CWnd::FromHandle(hWnd);
}

int CWnd::MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption, UINT nType)
{
	if (lpszCaption == NULL)
		lpszCaption = AfxGetAppName();
	int nResult = ::AfxCtxMessageBox(GetSafeHwnd(), lpszText, lpszCaption, nType);
	return nResult;
}

CWnd* PASCAL CWnd::GetDescendantWindow(HWND hWnd, int nID, BOOL bOnlyPerm)
{
	// GetDlgItem recursive (return first found)
	// breadth-first for 1 level, then depth-first for next level

	// use GetDlgItem since it is a fast USER function
	HWND hWndChild;
	CWnd* pWndChild;
	if ((hWndChild = ::GetDlgItem(hWnd, nID)) != NULL)
	{
		if (::GetTopWindow(hWndChild) != NULL)
		{
			// children with the same ID as their parent have priority
			pWndChild = GetDescendantWindow(hWndChild, nID, bOnlyPerm);
			if (pWndChild != NULL)
				return pWndChild;
		}
		// return temporary handle if allowed
		if (!bOnlyPerm)
			return CWnd::FromHandle(hWndChild);

		// return only permanent handle
		pWndChild = CWnd::FromHandlePermanent(hWndChild);
		if (pWndChild != NULL)
			return pWndChild;
	}

	// walk each child
	for (hWndChild = ::GetTopWindow(hWnd); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		pWndChild = GetDescendantWindow(hWndChild, nID, bOnlyPerm);
		if (pWndChild != NULL)
			return pWndChild;
	}
	return NULL;    // not found
}

void PASCAL CWnd::SendMessageToDescendants(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam, BOOL bDeep, BOOL bOnlyPerm)
{
	// walk through HWNDs to avoid creating temporary CWnd objects
	// unless we need to call this function recursively
	for (HWND hWndChild = ::GetTopWindow(hWnd); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// if bOnlyPerm is TRUE, don't send to non-permanent windows
		if (bOnlyPerm)
		{
			CWnd* pWnd = CWnd::FromHandlePermanent(hWndChild);
			if (pWnd != NULL)
			{
				// call window proc directly since it is a C++ window
				AfxCallWndProc(pWnd, pWnd->m_hWnd, message, wParam, lParam);
			}
		}
		else
		{
			// send message with Windows SendMessage API
			::SendMessage(hWndChild, message, wParam, lParam);
		}
		if (bDeep && ::GetTopWindow(hWndChild) != NULL)
		{
			// send to child windows after parent
			SendMessageToDescendants(hWndChild, message, wParam, lParam,
				bDeep, bOnlyPerm);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Scroll bar helpers
//  hook for CWnd functions
//    only works for derived class (eg: CView) that override 'GetScrollBarCtrl'
// if the window doesn't have a _visible_ windows scrollbar - then
//   look for a sibling with the appropriate ID

CScrollBar* CWnd::GetScrollBarCtrl(int) const
{
	return NULL;        // no special scrollers supported
}

int CWnd::SetScrollPos(int nBar, int nPos, BOOL bRedraw)
{
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		return pScrollBar->SetScrollPos(nPos, bRedraw);
	else
		return ::SetScrollPos(m_hWnd, nBar, nPos, bRedraw);
}

int CWnd::GetScrollPos(int nBar) const
{
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		return pScrollBar->GetScrollPos();
	else
		return ::GetScrollPos(m_hWnd, nBar);
}

void CWnd::SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw)
{
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		pScrollBar->SetScrollRange(nMinPos, nMaxPos, bRedraw);
	else
		::SetScrollRange(m_hWnd, nBar, nMinPos, nMaxPos, bRedraw);
}

void CWnd::GetScrollRange(int nBar, LPINT lpMinPos, LPINT lpMaxPos) const
{
	CScrollBar* pScrollBar;
	if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
		pScrollBar->GetScrollRange(lpMinPos, lpMaxPos);
	else
		::GetScrollRange(m_hWnd, nBar, lpMinPos, lpMaxPos);
}

// Turn on/off non-control scrollbars
//   for WS_?SCROLL scrollbars - show/hide them
//   for control scrollbar - enable/disable them
void CWnd::EnableScrollBarCtrl(int nBar, BOOL bEnable)
{
	CScrollBar* pScrollBar;
	if (nBar == SB_BOTH)
	{
		EnableScrollBarCtrl(SB_HORZ, bEnable);
		EnableScrollBarCtrl(SB_VERT, bEnable);
	}
	else if ((pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
	{
		// control scrollbar - enable or disable
		pScrollBar->EnableWindow(bEnable);
	}
	else
	{
		// WS_?SCROLL scrollbar - show or hide
		ShowScrollBar(nBar, bEnable);
	}
}

BOOL CWnd::SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw)
{
	ASSERT(lpScrollInfo != NULL);

	HWND hWnd = m_hWnd;
	CScrollBar* pScrollBar;
	if (nBar != SB_CTL && (pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
	{
		hWnd = pScrollBar->m_hWnd;
		nBar = SB_CTL;
	}
	lpScrollInfo->cbSize = sizeof(*lpScrollInfo);
	::SetScrollInfo(hWnd, nBar, lpScrollInfo, bRedraw);
	return TRUE;
}

BOOL CWnd::GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask)
{
	ASSERT(lpScrollInfo != NULL);

	HWND hWnd = m_hWnd;
	CScrollBar* pScrollBar;
	if (nBar != SB_CTL && (pScrollBar = GetScrollBarCtrl(nBar)) != NULL)
	{
		hWnd = pScrollBar->m_hWnd;
		nBar = SB_CTL;
	}
	lpScrollInfo->cbSize = sizeof(*lpScrollInfo);
	lpScrollInfo->fMask = nMask;
	return ::GetScrollInfo(hWnd, nBar, lpScrollInfo);
}

int CWnd::GetScrollLimit(int nBar)
{
	int nMin, nMax;
	GetScrollRange(nBar, &nMin, &nMax);
	SCROLLINFO info;
	if (GetScrollInfo(nBar, &info, SIF_PAGE))
	{
		nMax -= __max(info.nPage-1,0);
	}
	return nMax;
}

void CWnd::ScrollWindow(int xAmount, int yAmount,
	LPCRECT lpRect, LPCRECT lpClipRect)
{
	ASSERT(::IsWindow(m_hWnd));

	if (IsWindowVisible() || lpRect != NULL || lpClipRect != NULL)
	{
		// When visible, let Windows do the scrolling
		::ScrollWindow(m_hWnd, xAmount, yAmount, lpRect, lpClipRect);
	}
	else
	{
		// Windows does not perform any scrolling if the window is
		// not visible.  This leaves child windows unscrolled.
		// To account for this oversight, the child windows are moved
		// directly instead.
		HWND hWndChild = ::GetWindow(m_hWnd, GW_CHILD);
		if (hWndChild != NULL)
		{
			for (; hWndChild != NULL;
				hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
			{
				CRect rect;
				::GetWindowRect(hWndChild, &rect);
				ScreenToClient(&rect);
				::SetWindowPos(hWndChild, NULL,
					rect.left+xAmount, rect.top+yAmount, 0, 0,
					SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
			}
		}
	}

#ifndef _AFX_NO_OCC_SUPPORT

	if ((m_pCtrlCont == NULL) || (lpRect != NULL))
		return;

	// the following code is for OLE control containers only

	m_pCtrlCont->ScrollChildren(xAmount, yAmount);

#endif // !_AFX_NO_OCC_SUPPORT
}

/////////////////////////////////////////////////////////////////////////////
// minimal layout support

void CWnd::RepositionBars(UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver,
	UINT nFlags, LPRECT lpRectParam, LPCRECT lpRectClient, BOOL bStretch)
{
	ASSERT(nFlags == 0 || (nFlags & ~reposNoPosLeftOver) == reposQuery || 
			(nFlags & ~reposNoPosLeftOver) == reposExtra);

	// walk kids in order, control bars get the resize notification
	//   which allow them to shrink the client area
	// remaining size goes to the 'nIDLeftOver' pane
	// NOTE: nIDFirst->nIDLast are usually 0->0xffff

	AFX_SIZEPARENTPARAMS layout;
	HWND hWndLeftOver = NULL;

	layout.bStretch = bStretch;
	layout.sizeTotal.cx = layout.sizeTotal.cy = 0;
	if (lpRectClient != NULL)
		layout.rect = *lpRectClient;    // starting rect comes from parameter
	else
		GetClientRect(&layout.rect);    // starting rect comes from client rect

	if ((nFlags & ~reposNoPosLeftOver) != reposQuery)
		layout.hDWP = ::BeginDeferWindowPos(8); // reasonable guess
	else
		layout.hDWP = NULL; // not actually doing layout

	for (HWND hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL;
		hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		UINT_PTR nIDC = _AfxGetDlgCtrlID(hWndChild);
		CWnd* pWnd = CWnd::FromHandlePermanent(hWndChild);
		if (nIDC == nIDLeftOver)
			hWndLeftOver = hWndChild;
		else if (nIDC >= nIDFirst && nIDC <= nIDLast && pWnd != NULL)
			::SendMessage(hWndChild, WM_SIZEPARENT, 0, (LPARAM)&layout);
	}

	// if just getting the available rectangle, return it now...
	if ((nFlags & ~reposNoPosLeftOver) == reposQuery)
	{
		ASSERT(lpRectParam != NULL);
		if (bStretch)
			::CopyRect(lpRectParam, &layout.rect);
		else
		{
			lpRectParam->left = lpRectParam->top = 0;
			lpRectParam->right = layout.sizeTotal.cx;
			lpRectParam->bottom = layout.sizeTotal.cy;
		}
		return;
	}

	// the rest is the client size of the left-over pane
	if (nIDLeftOver != 0 && hWndLeftOver != NULL)
	{
		CWnd* pLeftOver = CWnd::FromHandle(hWndLeftOver);
		// allow extra space as specified by lpRectBorder
		if ((nFlags & ~reposNoPosLeftOver) == reposExtra)
		{
			ASSERT(lpRectParam != NULL);
			layout.rect.left += lpRectParam->left;
			layout.rect.top += lpRectParam->top;
			layout.rect.right -= lpRectParam->right;
			layout.rect.bottom -= lpRectParam->bottom;
		}
		// reposition the window
		if ((nFlags & reposNoPosLeftOver) != reposNoPosLeftOver)
		{
			pLeftOver->CalcWindowRect(&layout.rect);
			AfxRepositionWindow(&layout, hWndLeftOver, &layout.rect);
		}
	}

	// move and resize all the windows at once!
	if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
		TRACE(traceAppMsg, 0, "Warning: DeferWindowPos failed - low system resources.\n");
}

void AFXAPI AfxRepositionWindow(AFX_SIZEPARENTPARAMS* lpLayout,
	HWND hWnd, LPCRECT lpRect)
{
	ASSERT(hWnd != NULL);
	ASSERT(lpRect != NULL);
	HWND hWndParent = ::GetParent(hWnd);
	ASSERT(hWndParent != NULL);

	if (lpLayout != NULL && lpLayout->hDWP == NULL)
		return;

	// first check if the new rectangle is the same as the current
	CRect rectOld;
	::GetWindowRect(hWnd, rectOld);
	::ScreenToClient(hWndParent, &rectOld.TopLeft());
	::ScreenToClient(hWndParent, &rectOld.BottomRight());
	if (::EqualRect(rectOld, lpRect))
		return;     // nothing to do

	// try to use DeferWindowPos for speed, otherwise use SetWindowPos
	if (lpLayout != NULL)
	{
		lpLayout->hDWP = ::DeferWindowPos(lpLayout->hDWP, hWnd, NULL,
			lpRect->left, lpRect->top,  lpRect->right - lpRect->left,
			lpRect->bottom - lpRect->top, SWP_NOACTIVATE|SWP_NOZORDER);
	}
	else
	{
		::SetWindowPos(hWnd, NULL, lpRect->left, lpRect->top,
			lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
			SWP_NOACTIVATE|SWP_NOZORDER);
	}
}

void CWnd::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	DWORD dwExStyle = GetExStyle();
	if (nAdjustType == 0)
		dwExStyle &= ~WS_EX_CLIENTEDGE;
	::AdjustWindowRectEx(lpClientRect, GetStyle(), FALSE, dwExStyle);
}

/////////////////////////////////////////////////////////////////////////////
// Special keyboard/system command processing

BOOL CWnd::HandleFloatingSysCommand(UINT nID, LPARAM lParam)
{
	CWnd* pParent = GetTopLevelParent();
	switch (nID & 0xfff0)
	{
	case SC_PREVWINDOW:
	case SC_NEXTWINDOW:
		if (LOWORD(lParam) == VK_F6 && pParent != NULL)
		{
			pParent->SetFocus();
			return TRUE;
		}
		break;

	case SC_CLOSE:
	case SC_KEYMENU:
		// Check lParam.  If it is 0L, then the user may have done
		// an Alt+Tab, so just ignore it.  This breaks the ability to
		// just press the Alt-key and have the first menu selected,
		// but this is minor compared to what happens in the Alt+Tab
		// case.
		if ((nID & 0xfff0) == SC_CLOSE || lParam != 0L)
		{
			if (pParent != NULL)
			{
				// Sending the above WM_SYSCOMMAND may destroy the app,
				// so we have to be careful about restoring activation
				// and focus after sending it.
				HWND hWndSave = m_hWnd;
				HWND hWndFocus = ::GetFocus();
				pParent->SetActiveWindow();
				pParent->SendMessage(WM_SYSCOMMAND, nID, lParam);

				// be very careful here...
				if (::IsWindow(hWndSave))
					::SetActiveWindow(hWndSave);
				if (::IsWindow(hWndFocus))
					::SetFocus(hWndFocus);
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL PASCAL CWnd::WalkPreTranslateTree(HWND hWndStop, MSG* pMsg)
{
	ASSERT(hWndStop == NULL || ::IsWindow(hWndStop));
	ASSERT(pMsg != NULL);

	// walk from the target window up to the hWndStop window checking
	//  if any window wants to translate this message

	for (HWND hWnd = pMsg->hwnd; hWnd != NULL; hWnd = ::GetParent(hWnd))
	{
		CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
		if (pWnd != NULL)
		{
			// target window is a C++ window
			if (pWnd->PreTranslateMessage(pMsg))
				return TRUE; // trapped by target window (eg: accelerators)
		}

		// got to hWndStop window without interest
		if (hWnd == hWndStop)
			break;
	}
	return FALSE;       // no special processing
}

BOOL CWnd::SendChildNotifyLastMsg(LRESULT* pResult)
{
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	return OnChildNotify(pThreadState->m_lastSentMsg.message,
		pThreadState->m_lastSentMsg.wParam, pThreadState->m_lastSentMsg.lParam, pResult);
}

BOOL PASCAL CWnd::ReflectLastMsg(HWND hWndChild, LRESULT* pResult)
{
	// get the map, and if no map, then this message does not need reflection
	CHandleMap* pMap = afxMapHWND();
	if (pMap == NULL)
		return FALSE;

	// check if in permanent map, if it is reflect it (could be OLE control)
	CWnd* pWnd = (CWnd*)pMap->LookupPermanent(hWndChild);
	ASSERT(pWnd == NULL || pWnd->m_hWnd == hWndChild);
	if (pWnd == NULL)
	{
#ifndef _AFX_NO_OCC_SUPPORT
		// check if the window is an OLE control
		CWnd* pWndParent = (CWnd*)pMap->LookupPermanent(::GetParent(hWndChild));
		if (pWndParent != NULL && pWndParent->m_pCtrlCont != NULL)
		{
			// If a matching control site exists, it's an OLE control
			COleControlSite* pSite = (COleControlSite*)pWndParent->
				m_pCtrlCont->m_siteMap.GetValueAt(hWndChild);
			if (pSite != NULL)
			{
				CWnd wndTemp(hWndChild);
				wndTemp.m_pCtrlSite = pSite;
				LRESULT lResult = wndTemp.SendChildNotifyLastMsg(pResult);
				wndTemp.m_hWnd = NULL;
				return lResult != 0;
			}
		}
#endif //!_AFX_NO_OCC_SUPPORT
		return FALSE;
	}

	// only OLE controls and permanent windows will get reflected msgs
	ASSERT(pWnd != NULL);
	return pWnd->SendChildNotifyLastMsg(pResult);
}

BOOL CWnd::OnChildNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
#ifndef _AFX_NO_OCC_SUPPORT
	if (m_pCtrlSite != NULL)
	{
		// first forward raw OCM_ messages to OLE control sources
		LRESULT lResult = SendMessage(OCM__BASE+uMsg, wParam, lParam);
		if (uMsg >= WM_CTLCOLORMSGBOX && uMsg <= WM_CTLCOLORSTATIC &&
			(HBRUSH)lResult == NULL)
		{
			// for WM_CTLCOLOR msgs, returning NULL implies continue routing
			return FALSE;
		}
		if (pResult != NULL)
			*pResult = lResult;
		return TRUE;
	}
#endif

	return ReflectChildNotify(uMsg, wParam, lParam, pResult);
}

BOOL CWnd::ReflectChildNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// Note: reflected messages are send directly to CWnd::OnWndMsg
	//  and CWnd::OnCmdMsg for speed and because these messages are not
	//  routed by normal OnCmdMsg routing (they are only dispatched)

	switch (uMsg)
	{
	// normal messages (just wParam, lParam through OnWndMsg)
	case WM_HSCROLL:
	case WM_VSCROLL:
	case WM_PARENTNOTIFY:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
	case WM_COMPAREITEM:
		// reflect the message through the message map as WM_REFLECT_BASE+uMsg
		return CWnd::OnWndMsg(WM_REFLECT_BASE+uMsg, wParam, lParam, pResult);

	// special case for WM_COMMAND
	case WM_COMMAND:
		{
			// reflect the message through the message map as OCM_COMMAND
			int nCode = HIWORD(wParam);
			if (CWnd::OnCmdMsg(0, MAKELONG(nCode, WM_REFLECT_BASE+WM_COMMAND), NULL, NULL))
			{
				if (pResult != NULL)
					*pResult = 1;
				return TRUE;
			}
		}
		break;

	// special case for WM_NOTIFY
	case WM_NOTIFY:
		{
			// reflect the message through the message map as OCM_NOTIFY
			NMHDR* pNMHDR = (NMHDR*)lParam;
			int nCode = pNMHDR->code;
			AFX_NOTIFY notify;
			notify.pResult = pResult;
			notify.pNMHDR = pNMHDR;
			return CWnd::OnCmdMsg(0, MAKELONG(nCode, WM_REFLECT_BASE+WM_NOTIFY), &notify, NULL);
		}

	// other special cases (WM_CTLCOLOR family)
	default:
		if (uMsg >= WM_CTLCOLORMSGBOX && uMsg <= WM_CTLCOLORSTATIC)
		{
			// fill in special struct for compatiblity with 16-bit WM_CTLCOLOR
			AFX_CTLCOLOR ctl;
			ctl.hDC = (HDC)wParam;
			ctl.nCtlType = uMsg - WM_CTLCOLORMSGBOX;
			//ASSERT(ctl.nCtlType >= CTLCOLOR_MSGBOX);
			ASSERT(ctl.nCtlType <= CTLCOLOR_STATIC);

			// reflect the message through the message map as OCM_CTLCOLOR
			BOOL bResult = CWnd::OnWndMsg(WM_REFLECT_BASE+WM_CTLCOLOR, 0, (LPARAM)&ctl, pResult);
			if ((HBRUSH)*pResult == NULL)
				bResult = FALSE;
			return bResult;
		}
		break;
	}

	return FALSE;   // let the parent handle it
}

void CWnd::OnParentNotify(UINT message, LPARAM lParam)
{
	if ((LOWORD(message) == WM_CREATE || LOWORD(message) == WM_DESTROY))
	{
		if (ReflectLastMsg((HWND)lParam))
			return;     // eat it
	}
	// not handled - do default
	Default();
}

void CWnd::OnSetFocus(CWnd*)
{ 
   BOOL bHandled;

   bHandled = FALSE;
#ifndef _AFX_NO_OCC_SUPPORT
   if (m_pCtrlCont != NULL)
   {
	  bHandled = m_pCtrlCont->HandleSetFocus();
   }
#endif  //!_AFX_NO_OCC_SUPPORT
   if( !bHandled )
   {
	  Default();
   }
}

LRESULT CWnd::OnActivateTopLevel(WPARAM wParam, LPARAM)
{
	if (LOWORD(wParam) == WA_INACTIVE)
	{
		AFX_MODULE_THREAD_STATE* pModuleThreadState = AfxGetModuleThreadState();
		if (pModuleThreadState->m_pLastInfo != NULL &&
			!(pModuleThreadState->m_pLastInfo->uFlags & TTF_ALWAYSTIP))
			CancelToolTips(TRUE);
	}

	return 0;
}

void CWnd::OnSysColorChange()
{
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && pApp->m_pMainWnd == this)
	{
		// recolor global brushes used by control bars
		afxData.UpdateSysColors();
	}

	// forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
		SendMessageToDescendants(WM_SYSCOLORCHANGE, 0, 0L, TRUE, TRUE);

	Default();
}

BOOL _afxGotScrollLines;

void CWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	UNUSED_ALWAYS(uFlags);
	UNUSED_ALWAYS(lpszSection);

	// force refresh of settings that we cache
	_afxGotScrollLines = FALSE;

	if (m_pCtrlCont != NULL) 
	{
		m_pCtrlCont->BroadcastAmbientPropertyChange( DISPID_AMBIENT_LOCALEID );
	}

	CWnd::OnDisplayChange(0, 0);    // to update system metrics, etc.
}

void CWnd::OnDevModeChange(_In_z_ LPTSTR lpDeviceName)
{
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && pApp->m_pMainWnd == this)
		pApp->DevModeChange(lpDeviceName);

	// forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
	{
		const MSG* pMsg = GetCurrentMessage();
		SendMessageToDescendants(pMsg->message, pMsg->wParam, pMsg->lParam,
			TRUE, TRUE);
	}
}

BOOL CWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	if (!(GetStyle() & WS_CHILD))
	{
		CWnd* pMainWnd = AfxGetMainWnd();
		if (pMainWnd != NULL &&
			GetKeyState(VK_SHIFT) >= 0 &&
			GetKeyState(VK_CONTROL) >= 0 &&
			GetKeyState(VK_MENU) >= 0)
		{
			pMainWnd->SendMessage(WM_COMMAND, ID_HELP);
			return TRUE;
		}
	}
	return Default() != 0;
}

LRESULT CWnd::OnDisplayChange(WPARAM, LPARAM)
{
	// update metrics if this window is the main window
	if (AfxGetMainWnd() == this)
	{
		// update any system metrics cache
		afxData.UpdateSysMetrics();
	}

	// forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
	{
		const MSG* pMsg = GetCurrentMessage();
		SendMessageToDescendants(pMsg->message, pMsg->wParam, pMsg->lParam,
			TRUE, TRUE);
	}

	return Default();
}

LRESULT CWnd::OnDragList(WPARAM, LPARAM lParam)
{
	LPDRAGLISTINFO lpInfo = (LPDRAGLISTINFO)lParam;
	ASSERT(lpInfo != NULL);

	LRESULT lResult;
	if (ReflectLastMsg(lpInfo->hWnd, &lResult))
		return (int)lResult;    // eat it

	// not handled - do default
	return (int)Default();
}

// Accessibility
LRESULT CWnd::OnGetObject(WPARAM wParam, LPARAM lParam)
{
	if (!m_bEnableActiveAccessibility)
		return Default();

	LRESULT lRet = 0;
	HRESULT hr = CreateAccessibleProxy(wParam, lParam, &lRet);
	if (FAILED(hr))
		return Default();
	return lRet;
}

//Helper class for Active Accesibility Proxy
//Base is the user's class that derives from CComObjectRoot and whatever
//interfaces the user wants to support on the object
void AFXAPI AfxOleLockApp();
void AFXAPI AfxOleUnlockApp();

template <class Base>
class CMFCComObject : public Base
{
public:
	typedef Base _BaseClass;
	CMFCComObject(void* = NULL)
	{
		AfxOleLockApp();
	}
	// Set refcount to -(LONG_MAX/2) to protect destruction and 
	// also catch mismatched Release in debug builds
	~CMFCComObject()
	{
		m_dwRef = -(LONG_MAX/2);
		FinalRelease();
#ifdef _ATL_DEBUG_INTERFACES
		_AtlDebugInterfacesModule.DeleteNonAddRefThunk(_GetRawUnknown());
#endif
		AfxOleUnlockApp();
	}
	//If InternalAddRef or InternalRelease is undefined then your class
	//doesn't derive from CComObjectRoot
	STDMETHOD_(ULONG, AddRef)() throw() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)() throw()
	{
		ULONG l = InternalRelease();
		if (l == 0)
			delete this;
		return l;
	}
	//if _InternalQueryInterface is undefined then you forgot BEGIN_COM_MAP
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) throw()
	{return _InternalQueryInterface(iid, ppvObject);}
	template <class Q>
	HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp)
	{
		return QueryInterface(__uuidof(Q), (void**)pp);
	}

	static HRESULT WINAPI CreateInstance(CMFCComObject<Base>** pp)
	{
		ATLASSERT(pp != NULL);
		if (pp == NULL)
			return E_POINTER;
		*pp = NULL;

		HRESULT hRes = E_OUTOFMEMORY;
		CMFCComObject<Base>* p = NULL;
		ATLTRY(p = new CMFCComObject<Base>())
		if (p != NULL)
		{
			p->SetVoid(NULL);
			p->InternalFinalConstructAddRef();
			hRes = p->_AtlInitialConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->FinalConstruct();
			if (SUCCEEDED(hRes))
				hRes = p->_AtlFinalConstruct();
			p->InternalFinalConstructRelease();
			if (hRes != S_OK)
			{
				delete p;
				p = NULL;
			}
		}
		*pp = p;
		return hRes;
	}	
};

BEGIN_INTERFACE_MAP(CWnd, CCmdTarget)
//	INTERFACE_PART(CWnd, __uuidof(IAccessible), Accessible)
//	INTERFACE_PART(CWnd, __uuidof(IAccessibleServer), AccessibleServer)
END_INTERFACE_MAP()

ULONG FAR EXPORT CWnd::XAccessible::AddRef()
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->ExternalAddRef();
}

ULONG FAR EXPORT CWnd::XAccessible::Release()
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->ExternalRelease();
}

HRESULT FAR EXPORT CWnd::XAccessible::QueryInterface(
	REFIID iid, void FAR* FAR* ppvObj)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

HRESULT CWnd::XAccessible::Invoke(
			/* [in] */ DISPID dispIdMember,
			/* [in] */ REFIID refiid,
			/* [in] */ LCID lcid,
			/* [in] */ WORD wFlags,
			/* [out][in] */ DISPPARAMS *pDispParams,
			/* [out] */ VARIANT *pVarResult,
			/* [out] */ EXCEPINFO *pExcepInfo,
			/* [out] */ UINT *puArgErr) 
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return AtlIAccessibleInvokeHelper((IAccessible*)(void*)this, dispIdMember, refiid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
} 

HRESULT CWnd::XAccessible::GetIDsOfNames(
	REFIID refiid,
	LPOLESTR *rgszNames,
	UINT cNames,
	LCID lcid,
	DISPID *rgDispId) 
{
	return AtlIAccessibleGetIDsOfNamesHelper(refiid, rgszNames, cNames, lcid, rgDispId);
}

HRESULT CWnd::XAccessible::GetTypeInfoCount(unsigned int*  pctinfo) 
{
	if (pctinfo == NULL) 
	{
		return E_POINTER;
	}
	*pctinfo = 1;
	return S_OK;
}
HRESULT CWnd::XAccessible::GetTypeInfo(unsigned int /*iTInfo*/, LCID /*lcid*/, ITypeInfo** /*ppTInfo*/) 
{
	return E_NOTIMPL;
}

HRESULT CWnd::XAccessible::get_accParent(IDispatch **ppdispParent)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accParent(ppdispParent);
}
HRESULT CWnd::XAccessible::get_accChildCount(long *pcountChildren)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accChildCount(pcountChildren);
}
HRESULT CWnd::XAccessible::get_accChild(VARIANT varChild, IDispatch **ppdispChild)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accChild(varChild, ppdispChild);
}
HRESULT CWnd::XAccessible::get_accName(VARIANT varChild, BSTR *pszName)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accName(varChild, pszName);
}
HRESULT CWnd::XAccessible::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accValue(varChild, pszValue);
}
HRESULT CWnd::XAccessible::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accDescription(varChild, pszDescription);
}
HRESULT CWnd::XAccessible::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accRole(varChild, pvarRole);
}
HRESULT CWnd::XAccessible::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accState(varChild, pvarState);
}
HRESULT CWnd::XAccessible::get_accHelp(VARIANT varChild, BSTR *pszHelp)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accHelp(varChild, pszHelp);
}
HRESULT CWnd::XAccessible::get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
}
HRESULT CWnd::XAccessible::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}
HRESULT CWnd::XAccessible::get_accFocus(VARIANT *pvarChild)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accFocus(pvarChild);
}
HRESULT CWnd::XAccessible::get_accSelection(VARIANT *pvarChildren)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accSelection(pvarChildren);
}
HRESULT CWnd::XAccessible::get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->get_accDefaultAction(varChild, pszDefaultAction);
}
HRESULT CWnd::XAccessible::accSelect(long flagsSelect, VARIANT varChild)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->accSelect(flagsSelect, varChild);
}
HRESULT CWnd::XAccessible::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
}
HRESULT CWnd::XAccessible::accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->accNavigate(navDir, varStart, pvarEndUpAt);
}
HRESULT CWnd::XAccessible::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->accHitTest(xLeft, yTop, pvarChild);
}
HRESULT CWnd::XAccessible::accDoDefaultAction(VARIANT varChild)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->accDoDefaultAction(varChild);
}
//Obsolete
HRESULT CWnd::XAccessible::put_accName(VARIANT varChild, BSTR szName)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->put_accName(varChild, szName);
}
//Obsolete
HRESULT CWnd::XAccessible::put_accValue(VARIANT varChild, BSTR szValue)
{
	METHOD_PROLOGUE(CWnd, Accessible)
	return pThis->put_accValue(varChild, szValue);
}

ULONG FAR EXPORT CWnd::XAccessibleServer::AddRef()
{
	METHOD_PROLOGUE(CWnd, AccessibleServer)
	return pThis->ExternalAddRef();
}

ULONG FAR EXPORT CWnd::XAccessibleServer::Release()
{
	METHOD_PROLOGUE(CWnd, AccessibleServer)
	return pThis->ExternalRelease();
}

HRESULT FAR EXPORT CWnd::XAccessibleServer::QueryInterface(
	REFIID iid, void FAR* FAR* ppvObj)
{
	METHOD_PROLOGUE(CWnd, AccessibleServer)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

HRESULT CWnd::XAccessibleServer::SetProxy(IAccessibleProxy *pProxy)
{
	METHOD_PROLOGUE(CWnd, AccessibleServer)
	return pThis->SetProxy(pProxy);
}

HRESULT CWnd::XAccessibleServer::GetHWND(HWND *phWnd)
{
	if (phWnd == NULL)
		return E_POINTER;
	METHOD_PROLOGUE(CWnd, AccessibleServer)
	*phWnd = pThis->m_hWnd;
	return S_OK;
}

HRESULT CWnd::XAccessibleServer::GetEnumVariant(IEnumVARIANT **ppEnumVariant)
{
	if (ppEnumVariant == NULL)
		return E_POINTER;
	*ppEnumVariant = NULL;
	return E_NOTIMPL;
}


HRESULT CWnd::EnsureStdObj()
{
	if (m_pStdObject == NULL)
	{
		HRESULT hr = CreateStdAccessibleObject(m_hWnd, OBJID_CLIENT, __uuidof(IAccessible), (void**)&m_pStdObject);
		if (FAILED(hr))
			return hr;
	}
	return S_OK;
}

// Delegate to standard helper?
HRESULT CWnd::get_accParent(IDispatch **ppdispParent)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accParent(ppdispParent);
}

// Delegate to standard helper?
HRESULT CWnd::get_accChildCount(long *pcountChildren)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accChildCount(pcountChildren);
}

// Delegate to standard helper?
HRESULT CWnd::get_accChild(VARIANT varChild, IDispatch **ppdispChild)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accChild(varChild, ppdispChild);
}

// Override in users code
HRESULT CWnd::get_accName(VARIANT varChild, BSTR *pszName)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accName(varChild, pszName);
}

// Override in users code
// Default inplementation will get window text and return it.
HRESULT CWnd::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	return m_pStdObject->get_accValue(varChild, pszValue);
}

// Override in users code
HRESULT CWnd::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	return m_pStdObject->get_accDescription(varChild, pszDescription);
}

// Investigate
HRESULT CWnd::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accRole(varChild, pvarRole);
}

// Investigate
HRESULT CWnd::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accState(varChild, pvarState);
}

// Override in User's code?
HRESULT CWnd::get_accHelp(VARIANT varChild, BSTR *pszHelp)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accHelp(varChild, pszHelp);
}

// Override in user's code?
HRESULT CWnd::get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
}

// Override in user's code?
HRESULT CWnd::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}

// Delegate to standard implementation?
HRESULT CWnd::get_accFocus(VARIANT *pvarChild)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accFocus(pvarChild);
}

// Investigate
HRESULT CWnd::get_accSelection(VARIANT *pvarChildren)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accSelection(pvarChildren);
}

// Override in user's code
HRESULT CWnd::get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->get_accDefaultAction(varChild, pszDefaultAction);
}

// Investigate
HRESULT CWnd::accSelect(long flagsSelect, VARIANT varChild)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->accSelect(flagsSelect, varChild);
}

// Delegate?
HRESULT CWnd::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
}

// Delegate? May have to implement for COM children
HRESULT CWnd::accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->accNavigate(navDir, varStart, pvarEndUpAt);
}

// Delegate?
HRESULT CWnd::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->accHitTest(xLeft, yTop, pvarChild);
}

// Override in user's code
HRESULT CWnd::accDoDefaultAction(VARIANT varChild)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->accDoDefaultAction(varChild);
}

//Obsolete
HRESULT CWnd::put_accName(VARIANT varChild, BSTR szName)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->put_accName(varChild, szName);
}

//Obsolete
HRESULT CWnd::put_accValue(VARIANT varChild, BSTR szValue)
{
	ASSERT(m_pStdObject != NULL);
	return m_pStdObject->put_accName(varChild, szValue);
}

HRESULT CWnd::SetProxy(IAccessibleProxy *pProxy)
{
	m_pProxy = pProxy;
	return S_OK;
}

HRESULT CWnd::CreateAccessibleProxy(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	ASSERT(pResult != NULL);
	DWORD dwObjId = (DWORD) lParam;
	HRESULT hr = E_FAIL;

	if (dwObjId == OBJID_CLIENT)
	{
		hr = EnsureStdObj();
		if (SUCCEEDED(hr))
		{
			if (m_pProxy == NULL)
			{
				CMFCComObject<CAccessibleProxy> *p;
				hr = CMFCComObject<CAccessibleProxy>::CreateInstance(&p);
				if (SUCCEEDED(hr))
				{
					CComPtr<IAccessibleProxy> spProx;
					hr = p->QueryInterface(&spProx);
					if (SUCCEEDED(hr))
					{
						m_pProxy = spProx;
						spProx->SetServer(static_cast<IAccessible*>((void*)&m_xAccessible), static_cast<IAccessibleServer*>((void*)&m_xAccessibleServer));
						*pResult = LresultFromObject (__uuidof(IAccessible), wParam, m_pProxy);
					}
					hr = S_OK;
				}
			}
			else
			{
				*pResult = LresultFromObject (__uuidof(IAccessible), wParam, m_pProxy);
				hr = S_OK;
			}
		}
	}
	return hr;
}

// Helpers for CWnd or derived class that contains Windowless Active X controls
// Used by CView, CFormView, CDialog and CDialogBar
long CWnd::GetWindowLessChildCount()
{
	long lCount = 0;
 	if (m_pCtrlCont != NULL)
	{
		// Add to the count the number of windowless active X controls.
		POSITION pos = m_pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
		while(pos)
		{
			COleControlSiteOrWnd *pSiteOrWnd = m_pCtrlCont->m_listSitesOrWnds.GetNext(pos);
			ASSERT(pSiteOrWnd);
			if(pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->m_bIsWindowless)
				lCount ++;
		}
	}
	return lCount;
}
long CWnd::GetWindowedChildCount()
{
	long lCount = 0;
	for (CWnd* pChild = GetWindow(GW_CHILD); pChild != NULL; pChild = pChild->GetWindow(GW_HWNDNEXT), lCount++);
	return lCount;
}

long CWnd::GetAccessibleChildCount()
{
	return GetWindowedChildCount() + GetWindowLessChildCount();
}

HRESULT CWnd::GetAccessibleChild(VARIANT varChild, IDispatch** ppdispChild)
{
	if (ppdispChild == NULL)
		return E_POINTER;
	*ppdispChild = NULL;

	long lCount = varChild.lVal - 1;
	if (lCount < 0)
		return E_INVALIDARG;

	CWnd* pChild = NULL;
	for(pChild = GetWindow(GW_CHILD); pChild != NULL && lCount != 0; pChild = pChild->GetWindow(GW_HWNDNEXT), lCount--);
	if (pChild != NULL)
	{
		// Found HWND
		return AccessibleObjectFromWindow(pChild->m_hWnd, OBJID_WINDOW, IID_IAccessible, (void**)ppdispChild);
	}
	// Windowless controls don't support accessibility.
	return S_FALSE;
}


HRESULT CWnd::GetAccessibleName(VARIANT varChild, BSTR* pszName)
{
	if (varChild.lVal == CHILDID_SELF)
	{
		CString strText;
		GetWindowText(strText);
		*pszName = strText.AllocSysString();
		return S_OK;
	}
	else
	{
		// Get Number of windowed children
		long lCount = GetWindowedChildCount();
		if (varChild.lVal > lCount)
		{
 			if (m_pCtrlCont != NULL)
			{
				// Add to the count the number of windowless active X controls.
				POSITION pos = m_pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
				while(pos != NULL)
				{
					COleControlSiteOrWnd *pSiteOrWnd = m_pCtrlCont->m_listSitesOrWnds.GetNext(pos);
					ASSERT(pSiteOrWnd);
					if(pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->m_bIsWindowless)
					{
						lCount ++;
						if (lCount == varChild.lVal)
						{
							CString strText;
							pSiteOrWnd->m_pSite->GetWindowText(strText);
							*pszName = strText.AllocSysString();
							return S_OK;
						}
					}
				}
			}			
		}
	}
	//out of range
	return E_INVALIDARG;
}
HRESULT CWnd::GetAccessibilityLocation(VARIANT varChild, long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight)
{
	HRESULT hr = CWnd::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
	if (FAILED(hr))
	{
		// Get Number of windowed children
		long lCount;
		CWnd::get_accChildCount(&lCount);
		long lWindowlessCount = GetWindowLessChildCount();
		if (varChild.lVal <= lCount + lWindowlessCount)
		{
			// Add to the count the number of windowless active X controls.
			POSITION pos;
			pos = m_pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
			COleControlSiteOrWnd *pSiteOrWnd;
			while(pos)
			{
				pSiteOrWnd = m_pCtrlCont->m_listSitesOrWnds.GetNext(pos);
				ASSERT(pSiteOrWnd);
				if(pSiteOrWnd->m_pSite)
				{
					if(pSiteOrWnd->m_pSite->m_bIsWindowless)
						lCount++;

					if (lCount == varChild.lVal)
					{
						CRect rect(pSiteOrWnd->m_pSite->m_rect);
						ClientToScreen(&rect);
						*pxLeft = rect.left;
						*pyTop = rect.top;
						*pcxWidth = rect.Width();
						*pcyHeight = rect.Height();
						hr = S_OK;
					}
				}
			}
		}
	}
	return hr;
}

HRESULT CWnd::GetAccessibilityHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	// Check if it is one of the Windowless controls
 	if (m_pCtrlCont != NULL)
	{
		CPoint pt(xLeft, yTop);
		ScreenToClient(&pt);
		long lCount = GetWindowedChildCount();
		// If windowed child then let the standard object handle it.
 		if (m_pCtrlCont != NULL)
		{
			POSITION pos;
			pos = m_pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
			while(pos)
			{
				COleControlSiteOrWnd *pSiteOrWnd = m_pCtrlCont->m_listSitesOrWnds.GetNext(pos);
				ASSERT(pSiteOrWnd);
				if (pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->m_bIsWindowless)
				{
					lCount ++;
					if (pSiteOrWnd->m_pSite->m_rect.PtInRect(pt))
					{
						pvarChild->vt = VT_I4;
						pvarChild->lVal = lCount;
						return S_OK;
					}
				}
			}
		}			
	}
	return CWnd::accHitTest(xLeft, yTop, pvarChild);
}

void CWnd::OnHScroll(UINT, UINT, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	Default();
}

void CWnd::OnVScroll(UINT, UINT, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	Default();
}

void CWnd::OnPaint()
{
   if (m_pCtrlCont != NULL)
   {
	  // Paint windowless controls
	  CPaintDC dc(this);
	  m_pCtrlCont->OnPaint(&dc);
   }

   Default();
}

void CWnd::OnEnterIdle(UINT /*nWhy*/, CWnd* /*pWho*/)
{
	// In some OLE inplace active scenarios, OLE will post a
	// message instead of sending it.  This causes so many WM_ENTERIDLE
	// messages to be sent that tasks running in the background stop
	// running.  By dispatching the pending WM_ENTERIDLE messages
	// when the first one is received, we trick Windows into thinking
	// that only one was really sent and dispatched.
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, WM_ENTERIDLE, WM_ENTERIDLE, PM_REMOVE))
			DispatchMessage(&msg);
	}

	Default();
}

HBRUSH CWnd::OnCtlColor(CDC*, CWnd* pWnd, UINT)
{
	ASSERT(pWnd != NULL && pWnd->m_hWnd != NULL);
	LRESULT lResult;
	if (pWnd->SendChildNotifyLastMsg(&lResult))
		return (HBRUSH)lResult;     // eat it
	return (HBRUSH)Default();
}

// implementation of OnCtlColor for default gray backgrounds
//   (works for any window containing controls)
//  return value of FALSE means caller must call DefWindowProc's default
//  TRUE means that 'hbrGray' will be used and the appropriate text
//    ('clrText') and background colors are set.
BOOL PASCAL CWnd::GrayCtlColor(HDC hDC, HWND hWnd, UINT nCtlColor,
	HBRUSH hbrGray, COLORREF clrText)
{
	if (hDC == NULL)
	{
		// sometimes Win32 passes a NULL hDC in the WM_CTLCOLOR message.
		TRACE(traceAppMsg, 0, "Warning: hDC is NULL in CWnd::GrayCtlColor; WM_CTLCOLOR not processed.\n");
		return FALSE;
	}

	if (hbrGray == NULL ||
		nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_MSGBOX ||
		nCtlColor == CTLCOLOR_SCROLLBAR)
	{
		return FALSE;
	}

	if (nCtlColor == CTLCOLOR_LISTBOX)
	{
		// only handle requests to draw the space between edit and drop button
		//  in a drop-down combo (not a drop-down list)
		if (!_AfxIsComboBoxControl(hWnd, (UINT)CBS_DROPDOWN))
			return FALSE;
	}

	// set background color and return handle to brush
	LOGBRUSH logbrush;
	VERIFY(::GetObject(hbrGray, sizeof(LOGBRUSH), (LPVOID)&logbrush));
	::SetBkColor(hDC, logbrush.lbColor);
	if (clrText == (COLORREF)-1)
		clrText = ::GetSysColor(COLOR_WINDOWTEXT);  // normal text
	::SetTextColor(hDC, clrText);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// 'dialog data' support

BOOL CWnd::UpdateData(BOOL bSaveAndValidate)
{
	ASSERT(::IsWindow(m_hWnd)); // calling UpdateData before DoModal?

	CDataExchange dx(this, bSaveAndValidate);

	// prevent control notifications from being dispatched during UpdateData
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	HWND hWndOldLockout = pThreadState->m_hLockoutNotifyWindow;
	ASSERT(hWndOldLockout != m_hWnd);   // must not recurse
	pThreadState->m_hLockoutNotifyWindow = m_hWnd;

	BOOL bOK = FALSE;       // assume failure
	TRY
	{
		DoDataExchange(&dx);
		bOK = TRUE;         // it worked
	}
	CATCH(CUserException, e)
	{
		// validation failed - user already alerted, fall through
		ASSERT(!bOK);											
		// Note: DELETE_EXCEPTION_(e) not required
	}
	AND_CATCH_ALL(e)
	{
		// validation failed due to OOM or other resource failure
		e->ReportError(MB_ICONEXCLAMATION, AFX_IDP_INTERNAL_FAILURE);
		ASSERT(!bOK);
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	pThreadState->m_hLockoutNotifyWindow = hWndOldLockout;
	return bOK;
}

CDataExchange::CDataExchange(CWnd* pDlgWnd, BOOL bSaveAndValidate)
{
	ASSERT_VALID(pDlgWnd);
	m_bSaveAndValidate = bSaveAndValidate;
	m_pDlgWnd = pDlgWnd;
	m_idLastControl = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Centering dialog support (works for any non-child window)

void CWnd::CenterWindow(CWnd* pAlternateOwner)
{
	ASSERT(::IsWindow(m_hWnd));

	// determine owner window to center against
	DWORD dwStyle = GetStyle();
	HWND hWndCenter = pAlternateOwner->GetSafeHwnd();
	if (pAlternateOwner == NULL)
	{
		if (dwStyle & WS_CHILD)
			hWndCenter = ::GetParent(m_hWnd);
		else
			hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);
		if (hWndCenter != NULL)
		{
			// let parent determine alternate center window
			HWND hWndTemp =
				(HWND)::SendMessage(hWndCenter, WM_QUERYCENTERWND, 0, 0);
			if (hWndTemp != NULL)
				hWndCenter = hWndTemp;
		}
	}

	// get coordinates of the window relative to its parent
	CRect rcDlg;
	GetWindowRect(&rcDlg);
	CRect rcArea;
	CRect rcCenter;
	HWND hWndParent;
	if (!(dwStyle & WS_CHILD))
	{
		// don't center against invisible or minimized windows
		if (hWndCenter != NULL)
		{
			DWORD dwAlternateStyle = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if (!(dwAlternateStyle & WS_VISIBLE) || (dwAlternateStyle & WS_MINIMIZE))
				hWndCenter = NULL;
		}

 		MONITORINFO mi;
		mi.cbSize = sizeof(mi);

		// center within appropriate monitor coordinates
		if (hWndCenter == NULL)
		{
			HWND hwDefault = AfxGetMainWnd()->GetSafeHwnd();

			GetMonitorInfo(
				MonitorFromWindow(hwDefault, MONITOR_DEFAULTTOPRIMARY), &mi);
			rcCenter = mi.rcWork;
			rcArea = mi.rcWork;
		}
		else
		{
			::GetWindowRect(hWndCenter, &rcCenter);
			GetMonitorInfo(
				MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST), &mi);
			rcArea = mi.rcWork;
		}
	}
	else
	{
		// center within parent client coordinates
		hWndParent = ::GetParent(m_hWnd);
		ASSERT(::IsWindow(hWndParent));

		::GetClientRect(hWndParent, &rcArea);
		ASSERT(::IsWindow(hWndCenter));
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	// find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - rcDlg.Width() / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - rcDlg.Height() / 2;

	// if the dialog is outside the screen, move it inside
	if (xLeft + rcDlg.Width() > rcArea.right)
		xLeft = rcArea.right - rcDlg.Width();
	if (xLeft < rcArea.left)
		xLeft = rcArea.left;

	if (yTop + rcDlg.Height() > rcArea.bottom)
		yTop = rcArea.bottom - rcDlg.Height();
	if (yTop < rcArea.top)
		yTop = rcArea.top;

	// map screen coordinates to child coordinates
	SetWindowPos(NULL, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BOOL CWnd::CheckAutoCenter()
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Dialog initialization support

BOOL CWnd::ExecuteDlgInit(LPCTSTR lpszResourceName)
{
	// find resource handle
	LPVOID lpResource = NULL;
	HGLOBAL hResource = NULL;
	if (lpszResourceName != NULL)
	{
		HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_DLGINIT);
		HRSRC hDlgInit = ::FindResource(hInst, lpszResourceName, RT_DLGINIT);
		if (hDlgInit != NULL)
		{
			// load it
			hResource = LoadResource(hInst, hDlgInit);
			if (hResource == NULL)
				return FALSE;
			// lock it
			lpResource = LockResource(hResource);
			ASSERT(lpResource != NULL);
		}
	}

	// execute it
	BOOL bResult = ExecuteDlgInit(lpResource);

	// cleanup
	if (lpResource != NULL && hResource != NULL)
	{
		UnlockResource(hResource);
		FreeResource(hResource);
	}
	return bResult;
}

BOOL CWnd::ExecuteDlgInit(LPVOID lpResource)
{
	BOOL bSuccess = TRUE;
	if (lpResource != NULL)
	{
		UNALIGNED WORD* lpnRes = (WORD*)lpResource;
		while (bSuccess && *lpnRes != 0)	
		{
			WORD nIDC = *lpnRes++;
			WORD nMsg = *lpnRes++;
			DWORD dwLen = *((UNALIGNED DWORD*&)lpnRes)++;

			// In Win32 the WM_ messages have changed.  They have
			// to be translated from the 32-bit values to 16-bit
			// values here.

			#define WIN16_LB_ADDSTRING  0x0401
			#define WIN16_CB_ADDSTRING  0x0403
			#define AFX_CB_ADDSTRING	0x1234

			// unfortunately, WIN16_CB_ADDSTRING == CBEM_INSERTITEM
			if (nMsg == AFX_CB_ADDSTRING)
				nMsg = CBEM_INSERTITEM;
			else if (nMsg == WIN16_LB_ADDSTRING)
				nMsg = LB_ADDSTRING;
			else if (nMsg == WIN16_CB_ADDSTRING)
				nMsg = CB_ADDSTRING;

			// check for invalid/unknown message types
#ifdef _AFX_NO_OCC_SUPPORT
			ASSERT(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING ||
				nMsg == CBEM_INSERTITEM);
#else
			ASSERT(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING ||
				nMsg == CBEM_INSERTITEM ||
				nMsg == WM_OCC_LOADFROMSTREAM ||
				nMsg == WM_OCC_LOADFROMSTREAM_EX ||
				nMsg == WM_OCC_LOADFROMSTORAGE ||
				nMsg == WM_OCC_LOADFROMSTORAGE_EX ||
				nMsg == WM_OCC_INITNEW);
#endif

#ifdef _DEBUG
			// For AddStrings, the count must exactly delimit the
			// string, including the NULL termination.  This check
			// will not catch all mal-formed ADDSTRINGs, but will
			// catch some.
			if (nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING || nMsg == CBEM_INSERTITEM)
				ASSERT(*((LPBYTE)lpnRes + (UINT)dwLen - 1) == 0);
#endif

			if (nMsg == CBEM_INSERTITEM)
			{
				COMBOBOXEXITEM item = {0};
				item.mask = CBEIF_TEXT;
				item.iItem = -1;
				CString strText(reinterpret_cast<LPTSTR>(lpnRes));				
				item.pszText = const_cast<LPTSTR>(strText.GetString());
				if (::SendDlgItemMessage(m_hWnd, nIDC, nMsg, 0, (LPARAM) &item) == -1)
					bSuccess = FALSE;
			}
#ifndef _AFX_NO_OCC_SUPPORT
			else if (nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING)
#endif // !_AFX_NO_OCC_SUPPORT
			{
				// List/Combobox returns -1 for error
				if (::SendDlgItemMessageA(m_hWnd, nIDC, nMsg, 0, (LPARAM) lpnRes) == -1)
					bSuccess = FALSE;
			}


			// skip past data
			lpnRes = (WORD*)((LPBYTE)lpnRes + (UINT)dwLen);
		}
	}

	// send update message to all controls after all other siblings loaded
	if (bSuccess)
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, FALSE, FALSE);

	return bSuccess;
}

void CWnd::UpdateDialogControls(CCmdTarget* pTarget, BOOL bDisableIfNoHndler)
{
	CCmdUI state;
	CWnd wndTemp;       // very temporary window just for CmdUI update

	// walk all the kids - assume the IDs are for buttons
	for (HWND hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL;
			hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// send to buttons
		wndTemp.m_hWnd = hWndChild; // quick and dirty attach
		state.m_nID = _AfxGetDlgCtrlID(hWndChild);
		state.m_pOther = &wndTemp;

		// check for reflect handlers in the child window
		CWnd* pWnd = CWnd::FromHandlePermanent(hWndChild);
		if (pWnd != NULL)
		{
			// call it directly to disable any routing
			if (pWnd->CWnd::OnCmdMsg(0, MAKELONG(0xffff,
				WM_COMMAND+WM_REFLECT_BASE), &state, NULL))
				continue;
		}

		// check for handlers in the parent window
		if (CWnd::OnCmdMsg((UINT)state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
			continue;

		// determine whether to disable when no handler exists
		BOOL bDisableTemp = bDisableIfNoHndler;
		if (bDisableTemp)
		{
			if ((wndTemp.SendMessage(WM_GETDLGCODE) & DLGC_BUTTON) == 0)
			{
				// non-button controls don't get automagically disabled
				bDisableTemp = FALSE;
			}
			else
			{
				// only certain button controls get automagically disabled
				UINT nStyle = (UINT)(wndTemp.GetStyle() & 0x0F);
				if (nStyle == (UINT)BS_AUTOCHECKBOX ||
					nStyle == (UINT)BS_AUTO3STATE ||
					nStyle == (UINT)BS_GROUPBOX ||
					nStyle == (UINT)BS_AUTORADIOBUTTON)
				{
					bDisableTemp = FALSE;
				}
			}
		}
		// check for handlers in the target (owner)
		state.DoUpdate(pTarget, bDisableTemp);
	}
	wndTemp.m_hWnd = NULL;      // quick and dirty detach
}

BOOL CWnd::PreTranslateInput(LPMSG lpMsg)
{
	ASSERT(::IsWindow(m_hWnd));

	// don't translate non-input events
	if ((lpMsg->message < WM_KEYFIRST || lpMsg->message > WM_KEYLAST) &&
		(lpMsg->message < WM_MOUSEFIRST || lpMsg->message > AFX_WM_MOUSELAST))
		return FALSE;

	return IsDialogMessage(lpMsg);
}

int CWnd::RunModalLoop(DWORD dwFlags)
{
	ASSERT(::IsWindow(m_hWnd)); // window must be created
	ASSERT(!(m_nFlags & WF_MODALLOOP)); // window must not already be in modal state

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;
	BOOL bShowIdle = (dwFlags & MLF_SHOWONIDLE) && !(GetStyle() & WS_VISIBLE);
	HWND hWndParent = ::GetParent(m_hWnd);
	m_nFlags |= (WF_MODALLOOP|WF_CONTINUEMODAL);
	MSG *pMsg = AfxGetCurrentMessage();

	// acquire and dispatch messages until the modal state is done
	for (;;)
	{
		ASSERT(ContinueModal());

		// phase1: check to see if we can do idle work
		while (bIdle &&
			!::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			ASSERT(ContinueModal());

			// show the dialog when the message queue goes idle
			if (bShowIdle)
			{
				ShowWindow(SW_SHOWNORMAL);
				UpdateWindow();
				bShowIdle = FALSE;
			}

			// call OnIdle while in bIdle state
			if (!(dwFlags & MLF_NOIDLEMSG) && hWndParent != NULL && lIdleCount == 0)
			{
				// send WM_ENTERIDLE to the parent
				::SendMessage(hWndParent, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)m_hWnd);
			}
			if ((dwFlags & MLF_NOKICKIDLE) ||
				!SendMessage(WM_KICKIDLE, MSGF_DIALOGBOX, lIdleCount++))
			{
				// stop idle processing next time
				bIdle = FALSE;
			}
		}

		// phase2: pump messages while available
		do
		{
			ASSERT(ContinueModal());

			// pump message, but quit on WM_QUIT
			if (!AfxPumpMessage())
			{
				AfxPostQuitMessage(0);
				return -1;
			}

			// show the window when certain special messages rec'd
			if (bShowIdle &&
				(pMsg->message == 0x118 || pMsg->message == WM_SYSKEYDOWN))
			{
				ShowWindow(SW_SHOWNORMAL);
				UpdateWindow();
				bShowIdle = FALSE;
			}

			if (!ContinueModal())
				goto ExitModal;

			// reset "no idle" state after pumping "normal" message
			if (AfxIsIdleMessage(pMsg))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}

		} while (::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE));
	}

ExitModal:
	m_nFlags &= ~(WF_MODALLOOP|WF_CONTINUEMODAL);
	return m_nModalResult;
}

BOOL CWnd::ContinueModal()
{
	return m_nFlags & WF_CONTINUEMODAL;
}

void CWnd::EndModalLoop(int nResult)
{
	ASSERT(::IsWindow(m_hWnd));

	// this result will be returned from CWnd::RunModalLoop
	m_nModalResult = nResult;

	// make sure a message goes through to exit the modal loop
	if (m_nFlags & WF_CONTINUEMODAL)
	{
		m_nFlags &= ~WF_CONTINUEMODAL;
		PostMessage(WM_NULL);
	}
}

#ifndef _AFX_NO_OCC_SUPPORT

BOOL CWnd::SetOccDialogInfo(_AFX_OCC_DIALOG_INFO*)
{
	ASSERT(FALSE); // this class doesn't support dialog creation
	return FALSE;
}
_AFX_OCC_DIALOG_INFO* CWnd::GetOccDialogInfo()
{	
	return NULL;	
}

#endif

/////////////////////////////////////////////////////////////////////////////
// Standard init called by WinMain

AFX_STATIC BOOL AFXAPI _AfxRegisterWithIcon(WNDCLASS* pWndCls,
	LPCTSTR lpszClassName, UINT nIDIcon)
{
	pWndCls->lpszClassName = lpszClassName;
	HINSTANCE hInst = AfxFindResourceHandle(
		ATL_MAKEINTRESOURCE(nIDIcon), ATL_RT_GROUP_ICON);
	if ((pWndCls->hIcon = ::LoadIcon(hInst, ATL_MAKEINTRESOURCE(nIDIcon))) == NULL)
	{
		// use default icon
		pWndCls->hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
	}
	return AfxRegisterClass(pWndCls);
}

LONG AFXAPI _AfxInitCommonControls(LPINITCOMMONCONTROLSEX lpInitCtrls, LONG fToRegister)
{
	ASSERT(fToRegister != 0);

	LONG lResult = 0;
	if (AFX_COMCTL32_IF_EXISTS(InitCommonControlsEx))
	{
		if (AfxInitCommonControlsEx(lpInitCtrls))
		{
			// InitCommonControlsEx was successful so return the full mask
			lResult = fToRegister;
		}
	}
	else
	{
		// not there, so call InitCommonControls if possible
		if ((fToRegister & AFX_WIN95CTLS_MASK) == fToRegister)
		{
			AfxInitCommonControls();
			lResult = AFX_WIN95CTLS_MASK;
		}
	}
	return lResult;
}

BOOL AFXAPI AfxEndDeferRegisterClass(LONG fToRegister)
{
	// mask off all classes that are already registered
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	fToRegister &= ~pModuleState->m_fRegisteredClasses;
	if (fToRegister == 0)
		return TRUE;

	LONG fRegisteredClasses = 0;

	// common initialization
	WNDCLASS wndcls;
	memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL defaults
	wndcls.lpfnWndProc = DefWindowProc;
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hCursor = afxData.hcurArrow;

	INITCOMMONCONTROLSEX init;
	init.dwSize = sizeof(init);

	// work to register classes as specified by fToRegister, populate fRegisteredClasses as we go
	if (fToRegister & AFX_WND_REG)
	{
		// Child windows - no brush, no icon, safest default class styles
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpszClassName = _afxWnd;
		if (AfxRegisterClass(&wndcls))
			fRegisteredClasses |= AFX_WND_REG;
	}
	if (fToRegister & AFX_WNDOLECONTROL_REG)
	{
		// OLE Control windows - use parent DC for speed
		wndcls.style |= CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpszClassName = _afxWndOleControl;
		if (AfxRegisterClass(&wndcls))
			fRegisteredClasses |= AFX_WNDOLECONTROL_REG;
	}
	if (fToRegister & AFX_WNDCONTROLBAR_REG)
	{
		// Control bar windows
		wndcls.style = 0;   // control bars don't handle double click
		wndcls.lpszClassName = _afxWndControlBar;
		wndcls.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		if (AfxRegisterClass(&wndcls))
			fRegisteredClasses |= AFX_WNDCONTROLBAR_REG;
	}
	if (fToRegister & AFX_WNDMDIFRAME_REG)
	{
		// MDI Frame window (also used for splitter window)
		wndcls.style = CS_DBLCLKS;
		wndcls.hbrBackground = NULL;
		if (_AfxRegisterWithIcon(&wndcls, _afxWndMDIFrame, AFX_IDI_STD_MDIFRAME))
			fRegisteredClasses |= AFX_WNDMDIFRAME_REG;
	}
	if (fToRegister & AFX_WNDFRAMEORVIEW_REG)
	{
		// SDI Frame or MDI Child windows or views - normal colors
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
		if (_AfxRegisterWithIcon(&wndcls, _afxWndFrameOrView, AFX_IDI_STD_FRAME))
			fRegisteredClasses |= AFX_WNDFRAMEORVIEW_REG;
	}
	if (fToRegister & AFX_WNDCOMMCTLS_REG)
	{
		// this flag is compatible with the old InitCommonControls() API
		init.dwICC = ICC_WIN95_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WIN95CTLS_MASK);
		fToRegister &= ~AFX_WIN95CTLS_MASK;
	}
	if (fToRegister & AFX_WNDCOMMCTL_UPDOWN_REG)
	{
		init.dwICC = ICC_UPDOWN_CLASS;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_UPDOWN_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_TREEVIEW_REG)
	{
		init.dwICC = ICC_TREEVIEW_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_TREEVIEW_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_TAB_REG)
	{
		init.dwICC = ICC_TAB_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_TAB_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_PROGRESS_REG)
	{
		init.dwICC = ICC_PROGRESS_CLASS;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_PROGRESS_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_LISTVIEW_REG)
	{
		init.dwICC = ICC_LISTVIEW_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_LISTVIEW_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_HOTKEY_REG)
	{
		init.dwICC = ICC_HOTKEY_CLASS;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_HOTKEY_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_BAR_REG)
	{
		init.dwICC = ICC_BAR_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_BAR_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_ANIMATE_REG)
	{
		init.dwICC = ICC_ANIMATE_CLASS;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_ANIMATE_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_INTERNET_REG)
	{
		init.dwICC = ICC_INTERNET_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_INTERNET_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_COOL_REG)
	{
		init.dwICC = ICC_COOL_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_COOL_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_USEREX_REG)
	{
		init.dwICC = ICC_USEREX_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_USEREX_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_DATE_REG)
	{
		init.dwICC = ICC_DATE_CLASSES;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_DATE_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_LINK_REG)
	{
		init.dwICC = ICC_LINK_CLASS;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_LINK_REG);
	}
	if (fToRegister & AFX_WNDCOMMCTL_PAGER_REG)
	{
		init.dwICC = ICC_PAGESCROLLER_CLASS;
		fRegisteredClasses |= _AfxInitCommonControls(&init, AFX_WNDCOMMCTL_PAGER_REG);
	}

	// save new state of registered controls
	pModuleState->m_fRegisteredClasses |= fRegisteredClasses;

	// special case for all common controls registered, turn on AFX_WNDCOMMCTLS_REG
	if ((pModuleState->m_fRegisteredClasses & AFX_WIN95CTLS_MASK) == AFX_WIN95CTLS_MASK)
	{
		pModuleState->m_fRegisteredClasses |= AFX_WNDCOMMCTLS_REG;
		fRegisteredClasses |= AFX_WNDCOMMCTLS_REG;
	}

	// must have registered at least as mamy classes as requested
	return (fToRegister & fRegisteredClasses) == fToRegister;
}

BOOL AFXAPI AfxInitNetworkAddressControl()
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	ENSURE(pModuleState);
	if (pModuleState->m_bInitNetworkAddressControlCalled == FALSE)
	{
		OSVERSIONINFO vi;
		ZeroMemory(&vi, sizeof(OSVERSIONINFO));
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		BOOL b = ::GetVersionEx(&vi);
		ENSURE(b);

		// if running under Vista
		if (vi.dwMajorVersion >= 6)
		{
			// Only try to GetProcAddress and call InitNetworkAddressControl if we are vista and higher.
			// If we call this on OS lower than Vista, this will throw.  We don't want it to throw.
			pModuleState->m_bInitNetworkAddressControl = AfxCtxInitNetworkAddressControl();
		}

		pModuleState->m_bInitNetworkAddressControlCalled = TRUE;
	}
	return pModuleState->m_bInitNetworkAddressControl;
}

/////////////////////////////////////////////////////////////////////////////
// CFrameWnd (here for library granularity)

BOOL CWnd::IsFrameWnd() const
{
	return FALSE;
}

BOOL CFrameWnd::IsFrameWnd() const
{
	return TRUE;
}

BOOL CFrameWnd::IsTracking() const
{
	return m_nIDTracking != 0 &&
		m_nIDTracking != AFX_IDS_HELPMODEMESSAGE &&
		m_nIDTracking != AFX_IDS_IDLEMESSAGE;
}

/////////////////////////////////////////////////////////////////////////////
// Extra CWnd support for dynamic subclassing of controls

BOOL CWnd::SubclassWindow(HWND hWnd)
{
	if (!Attach(hWnd))
		return FALSE;

	// allow any other subclassing to occur
	PreSubclassWindow();

	// now hook into the AFX WndProc
	WNDPROC* lplpfn = GetSuperWndProcAddr();
	WNDPROC oldWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC,
		(INT_PTR)AfxGetAfxWndProc());
	ASSERT(oldWndProc != AfxGetAfxWndProc());

	if (*lplpfn == NULL)
		*lplpfn = oldWndProc;   // the first control of that type created
#ifdef _DEBUG
	else if (*lplpfn != oldWndProc)
	{
		TRACE(traceAppMsg, 0, "Error: Trying to use SubclassWindow with incorrect CWnd\n");
		TRACE(traceAppMsg, 0, "\tderived class.\n");
		TRACE(traceAppMsg, 0, "\thWnd = $%08X (nIDC=$%08X) is not a %hs.\n", (UINT)(UINT_PTR)hWnd,
			_AfxGetDlgCtrlID(hWnd), GetRuntimeClass()->m_lpszClassName);
		ASSERT(FALSE);
		// undo the subclassing if continuing after assert
	  ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (INT_PTR)oldWndProc);
	}
#endif

	return TRUE;
}

BOOL CWnd::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	ASSERT(pParent != NULL);
	ASSERT(::IsWindow(pParent->m_hWnd));

	// check for normal dialog control first
	HWND hWndControl = ::GetDlgItem(pParent->m_hWnd, nID);
	if (hWndControl != NULL)
		return SubclassWindow(hWndControl);

#ifndef _AFX_NO_OCC_SUPPORT
	if (pParent->m_pCtrlCont != NULL)
	{
		// normal dialog control not found
		COleControlSite* pSite = pParent->m_pCtrlCont->FindItem(nID);
		if (pSite != NULL)
		{
			ASSERT(pSite->m_hWnd != NULL);
			VERIFY(SubclassWindow(pSite->m_hWnd));

#ifndef _AFX_NO_OCC_SUPPORT
			// If the control has reparented itself (e.g., invisible control),
			// make sure that the CWnd gets properly wired to its control site.
			if (pParent->m_hWnd != ::GetParent(pSite->m_hWnd))
				AttachControlSite(pParent);
#endif //!_AFX_NO_OCC_SUPPORT

			return TRUE;
		}
	}
#endif

	return FALSE;   // control not found
}

HWND CWnd::UnsubclassWindow()
{
	ASSERT(::IsWindow(m_hWnd));

	// set WNDPROC back to original value
	WNDPROC* lplpfn = GetSuperWndProcAddr();
	SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (INT_PTR)*lplpfn);
	*lplpfn = NULL;

	// and Detach the HWND from the CWnd object
	return Detach();
}

BOOL CWnd::CreateControlContainer(COleControlContainer** ppContainer)
{
   ENSURE_ARG( ppContainer != NULL );
   *ppContainer = NULL;  // Use the default container

   return TRUE;
}

BOOL CWnd::CreateControlSite(COleControlContainer*, 
   COleControlSite** ppSite, UINT /* nID */, REFCLSID /* clsid */)
{
   ENSURE_ARG( ppSite != NULL );
   *ppSite = NULL;  // Use the default site

   return TRUE;
}

COleControlContainer* CWnd::GetControlContainer()
{
	return m_pCtrlCont;
}

/////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNCREATE(CWnd, CCmdTarget)

/////////////////////////////////////////////////////////////////////////////
