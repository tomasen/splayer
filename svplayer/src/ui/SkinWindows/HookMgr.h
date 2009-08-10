// HookMgr.h: interface for the CHookMgr class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

static inline BOOL ClassMatches(HWND hwnd, LPCTSTR szClassType)
{
	if (!szClassType || !lstrlen(szClassType))
		return TRUE;
	
	// else
	static TCHAR szClassName[40];
	::GetClassName(hwnd, szClassName, 40);
	
	return (lstrcmpi(szClassType, szClassName) == 0);
}

#define INITHOOK(hook, flag, type, function) \
{ \
	if (dwOptions & flag) \
		hook = SetWindowsHookEx(type, function, NULL, GetCurrentThreadId()); \
}

#define RELEASEHOOK(hook) \
{ \
	if (hook) \
		UnhookWindowsHookEx(hook); \
\
	hook = NULL; \
}

enum 
{
	HM_CALLWNDPROC		= 0x0001,
	HM_CALLWNDPROCRET	= 0x0002,
	HM_CBT				= 0x0004,
	HM_FOREGROUNDIDLE	= 0x0008,
	HM_GETMESSAGE		= 0x0010,
	HM_KEYBOARD			= 0x0020,
	HM_MOUSE			= 0x0040,
	HM_MSGFILTER		= 0x0080,
	HM_SHELL			= 0x0100,
	HM_SYSMSGFILTER		= 0x0200,
};

#ifndef HSHELL_APPCOMMAND
#define HSHELL_APPCOMMAND           12
#endif

template<class MGRTYPE>
class CHookMgr  
{
public:
	virtual ~CHookMgr() { Release(); }


protected:
	BOOL InitHooks(DWORD dwOptions = HM_CALLWNDPROC, LPCTSTR szClassFilter = NULL)
	{
		Release(); // reset

		INITHOOK(m_hCallWndHook, HM_CALLWNDPROC, WH_CALLWNDPROC, CallWndProc);
		INITHOOK(m_hCallWndRetHook, HM_CALLWNDPROCRET, WH_CALLWNDPROCRET, CallWndRetProc);
		INITHOOK(m_hCbtHook, HM_CBT, WH_CBT, CbtProc);
		INITHOOK(m_hForegroundIdleHook, HM_FOREGROUNDIDLE, WH_FOREGROUNDIDLE, ForegroundIdleProc);
		INITHOOK(m_hGetMessageHook, HM_GETMESSAGE, WH_GETMESSAGE, GetMessageProc);
		INITHOOK(m_hKeyboardHook, HM_KEYBOARD, WH_KEYBOARD, KeyboardProc);
		INITHOOK(m_hMouseHook, HM_MOUSE, WH_MOUSE, MouseProc);
		INITHOOK(m_hMsgFilterHook, HM_MSGFILTER, WH_MSGFILTER, MsgFilterProc);
		INITHOOK(m_hShellHook, HM_SHELL, WH_SHELL, ShellProc);
		INITHOOK(m_hSysMsgFilterHook, HM_SYSMSGFILTER, WH_SYSMSGFILTER, SysMsgFilterProc);

		m_sClassFilter = szClassFilter;

		return TRUE;
	}

	void Release()
	{
		RELEASEHOOK(m_hCallWndHook);
		RELEASEHOOK(m_hCallWndRetHook);
		RELEASEHOOK(m_hCbtHook);
		RELEASEHOOK(m_hForegroundIdleHook);
		RELEASEHOOK(m_hGetMessageHook);
		RELEASEHOOK(m_hKeyboardHook);
		RELEASEHOOK(m_hMouseHook);
		RELEASEHOOK(m_hMsgFilterHook);
		RELEASEHOOK(m_hShellHook);
		RELEASEHOOK(m_hSysMsgFilterHook);
	}

protected:
	HHOOK m_hCallWndHook;
	HHOOK m_hCallWndRetHook;
	HHOOK m_hCbtHook;
	HHOOK m_hForegroundIdleHook;
	HHOOK m_hGetMessageHook;
	HHOOK m_hKeyboardHook;
	HHOOK m_hMouseHook;
	HHOOK m_hMsgFilterHook;
	HHOOK m_hShellHook;
	HHOOK m_hSysMsgFilterHook;
	CString m_sClassFilter;

protected:
	static MGRTYPE& GetInstance()
	{ 
		static MGRTYPE manager; 
		return manager; 
	}

	CHookMgr() // cannot instanciate one of these directly
	{
		m_hCallWndHook = NULL;
		m_hCallWndRetHook = NULL;
		m_hCbtHook = NULL;
		m_hForegroundIdleHook = NULL;
		m_hGetMessageHook = NULL;
		m_hKeyboardHook = NULL;
		m_hMouseHook = NULL;
		m_hMsgFilterHook = NULL;
		m_hShellHook = NULL;
		m_hSysMsgFilterHook = NULL;
	}

	// derived classes override whatever they need
	virtual void OnCallWndProc(const MSG& msg) { ASSERT (0); }
	virtual void OnCallWndRetProc(const MSG& msg, LRESULT lResult) { ASSERT (0); }
	virtual BOOL OnCbt(int nCode, WPARAM wParam, LPARAM lParam) { ASSERT (0); return FALSE; }
	virtual void OnForegroundIdle() { ASSERT (0); }
	virtual void OnGetMessage(const MSG& msg) { ASSERT (0); }
	virtual void OnKeyboard(UINT uVirtKey, UINT uFlags) { ASSERT (0); }
	virtual void OnMouse(UINT uMouseMsg, const CPoint& ptMouse) { ASSERT (0); }
	virtual void OnMsgFilter(const MSG& msg, int nEvent) { ASSERT (0); }
	virtual BOOL OnShell(int nCode, WPARAM wParam, LPARAM lParam) { ASSERT (0); return FALSE; }
	virtual void OnSysMsgFilter(const MSG& msg, int nEvent) { ASSERT (0); }

	inline BOOL ClassMatches(HWND hwnd)
	{
		return ::ClassMatches(hwnd, m_sClassFilter);
	}

	// global app hooks
	// WH_CALLWNDPROC
	static LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (nCode == HC_ACTION)
		{
			CWPSTRUCT* pwp = (CWPSTRUCT*)lParam;

			if (GetInstance().ClassMatches(pwp->hwnd))
			{
				MSG msg = { pwp->hwnd, pwp->message, pwp->wParam, pwp->lParam, 0, { 0, 0 } };
				GetInstance().OnCallWndProc(msg);
			}
		}
		
		return CallNextHookEx(GetInstance().m_hCallWndHook, nCode, wParam, lParam);
	}

	// WH_CALLWNDRETPROC
	static LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (nCode == HC_ACTION)
		{
			CWPRETSTRUCT* pwp = (CWPRETSTRUCT*)lParam;

			if (GetInstance().ClassMatches(pwp->hwnd))
			{
				MSG msg = { pwp->hwnd, pwp->message, pwp->wParam, pwp->lParam, 0, { 0, 0 } };
				GetInstance().OnCallWndRetProc(msg, pwp->lResult);
			}
		}
		
		return CallNextHookEx(GetInstance().m_hCallWndHook, nCode, wParam, lParam);
	}

	// WH_CBT
	static LRESULT CALLBACK CbtProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (nCode == HC_ACTION)
		{
			if (GetInstance().OnCbt(nCode, wParam, lParam))
				return TRUE;
		}
		
		// else
		return CallNextHookEx(GetInstance().m_hCbtHook, nCode, wParam, lParam);
	}

	// HM_FOREGROUNDIDLE
	static LRESULT CALLBACK ForegroundIdleProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (nCode == HC_ACTION)
		{
			GetInstance().OnForegroundIdle();
		}
		
		return CallNextHookEx(GetInstance().m_hForegroundIdleHook, nCode, wParam, lParam);
	}

	// WH_GETMESSAGE
	static LRESULT CALLBACK GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

		if (nCode == HC_ACTION)
		{
			MSG* pMsg = (MSG*)lParam;

			if (GetInstance().ClassMatches(pMsg->hwnd))
			{
				GetInstance().OnGetMessage(*pMsg);
			}
		}
		
		return CallNextHookEx(GetInstance().m_hGetMessageHook, nCode, wParam, lParam);
	}

	// WH_KEYBOARD
	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (nCode == HC_ACTION)
		{
			GetInstance().OnKeyboard(wParam, lParam);
		}
		
		return CallNextHookEx(GetInstance().m_hKeyboardHook, nCode, wParam, lParam);
	}

	// WH_MOUSE
	static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (nCode == HC_ACTION)
		{
			CPoint* pCursor = (CPoint*)lParam;
			GetInstance().OnMouse(wParam, *pCursor);
		}
		
		return CallNextHookEx(GetInstance().m_hMouseHook, nCode, wParam, lParam);
	}

	// WH_MSGFILTER
	static LRESULT CALLBACK MsgFilterProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (nCode == HC_ACTION)
		{
			MSG* pMsg = (MSG*)lParam;

			if (GetInstance().ClassMatches(pMsg->hwnd))
				GetInstance().OnMsgFilter(*pMsg, nCode);
		}

		return CallNextHookEx(GetInstance().m_hMsgFilterHook, nCode, wParam, lParam);
	}

	// WH_SHELL
	static LRESULT CALLBACK ShellProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (GetInstance().OnShell(nCode, wParam, lParam))
		{
			if (nCode == HSHELL_APPCOMMAND)
				return TRUE;
		}
		
		// else
		return 0;//CallNextHookEx(GetInstance().m_hShellHook, nCode, wParam, lParam);
	}

	// WH_SYSMSGFILTER
	static LRESULT CALLBACK SysMsgFilterProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
#ifdef _USRDLL
		// If this is a DLL, need to set up MFC state
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
		
		if (nCode == HC_ACTION)
		{
			MSG* pMsg = (MSG*)lParam;

			if (GetInstance().ClassMatches(pMsg->hwnd))
				GetInstance().OnSysMsgFilter(*pMsg, nCode);
		}
		
		return CallNextHookEx(GetInstance().m_hSysMsgFilterHook, nCode, wParam, lParam);
	}

};

