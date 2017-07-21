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
#include "occimpl.h"



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

#define CH_SYSMENU ' '

#define WS_TYPEMASK 0xC0000000
#ifndef BS_TYPEMASK
#define BS_TYPEMASK 0x0000000FL
#endif

static inline DWORD TestStyle(CWnd* pWnd, DWORD dwStyle)
	{ return GetWindowLong(pWnd->m_hWnd, GWL_STYLE) & dwStyle; }

static inline DWORD TestExStyle(CWnd* pWnd, DWORD dwExStyle)
	{ return GetWindowLong(pWnd->m_hWnd, GWL_EXSTYLE) & dwExStyle; }

static inline BOOL HasChildStyle(CWnd* pWnd)
	{ return TestStyle(pWnd, WS_TYPEMASK) == WS_CHILD; }

static inline BOOL IsControlParent(CWnd* pWnd)
	{ return TestExStyle(pWnd, WS_EX_CONTROLPARENT); }

AFX_STATIC DWORD AFXAPI _AfxGetDlgCode(CWnd* pWnd, LPMSG lpMsg=NULL)
{
	if (pWnd == NULL)
		return 0;

	WPARAM wParam = (lpMsg == NULL) ? 0 : lpMsg->wParam;

	return (DWORD)SendMessage(pWnd->m_hWnd, WM_GETDLGCODE,
		wParam, (LPARAM)(LPMSG)lpMsg);
}

AFX_STATIC void AFXAPI _AfxDlgSetFocus(CWnd* pWnd)
{
	// Select all text in an edit control.
	if (_AfxGetDlgCode(pWnd) & DLGC_HASSETSEL)
		pWnd->SendMessage(EM_SETSEL, 0, -1);

	// Set focus as normal.
	pWnd->SetFocus();
}

AFX_STATIC CWnd* AFXAPI _AfxGetChildControl(CWnd* pWndRoot, CWnd* pWndChild)
{
	CWnd* pWndControl = NULL;

	while ((pWndChild != NULL) && HasChildStyle(pWndChild) &&
		(pWndChild != pWndRoot))
	{
		pWndControl = pWndChild;
		pWndChild = pWndChild->GetParent();

		if (IsControlParent(pWndChild))
			break;
	}
	return pWndControl;
}

AFX_STATIC CWnd* AFXAPI _AfxNextControl(CWnd* pWndRoot, CWnd* pWndStart, UINT uFlags)
{
	// if pWndStart is already equal to pWndRoot, this confuses this function
	// badly.
	ASSERT(pWndRoot != pWndStart);

	if (pWndStart == NULL)
	{
FirstChild:
		pWndStart = pWndRoot->GetTopWindow();
		if (pWndStart == NULL)
			return pWndRoot;

		goto Found;
	}
	else
	{
		// Are we at the last control within some parent?  If so, pop back up.
		while (pWndStart->GetNextWindow() == NULL)
		{
			// Popup to previous real ancestor.  pWndStart will be NULL,
			// pWndRoot, or the child of a recursive dialog.
			pWndStart = _AfxGetChildControl(pWndRoot, pWndStart->GetParent());
			if ((pWndStart == NULL) || (pWndStart == pWndRoot))
			{
				goto FirstChild;
			}
		}

		ASSERT(pWndStart != NULL);
		pWndStart = pWndStart->GetNextWindow();
	}

Found:
	if (IsControlParent(pWndStart))
	{
		if (((uFlags & CWP_SKIPINVISIBLE) && !pWndStart->IsWindowVisible()) ||
			((uFlags & CWP_SKIPDISABLED) && !pWndStart->IsWindowEnabled()))
			pWndStart = _AfxNextControl(pWndRoot, pWndStart, uFlags);
		else
			pWndStart = _AfxNextControl(pWndStart, NULL, uFlags);
	}

	return pWndStart;
}

BOOL AFX_CDECL COccManager::IsMatchingMnemonic(CWnd* pWnd, LPMSG lpMsg)
{
	ENSURE_ARG(pWnd!=NULL);
	return (pWnd->m_pCtrlSite != NULL) &&
		pWnd->m_pCtrlSite->IsMatchingMnemonic(lpMsg);
}

BOOL AFX_CDECL COccManager::IsMatchingMnemonic(COleControlSiteOrWnd* pSiteOrWnd, LPMSG lpMsg)
{
	return (pSiteOrWnd->m_pSite != NULL) &&
		pSiteOrWnd->m_pSite->IsMatchingMnemonic(lpMsg);
}

COleControlSiteOrWnd* AFXAPI _AfxFindSiteOrWnd(CWnd *pWndDlg, CWnd *pWnd)
{
	COleControlContainer *pCtrlCont = pWndDlg->GetControlContainer();
	if (pCtrlCont == NULL) 
		return NULL;
		
	POSITION pos = pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
	while (pos != NULL)
	{
		COleControlSiteOrWnd* pSiteOrWnd = pCtrlCont->m_listSitesOrWnds.GetNext(pos);
		if ((pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->m_hWnd == pWnd->m_hWnd) ||
			(pSiteOrWnd->m_hWnd == pWnd->m_hWnd))
			return pSiteOrWnd;
	}
	return NULL;
}

AFX_STATIC COleControlSiteOrWnd* AFXAPI _AfxFindNextMnem(CWnd* pWndDlg, COleControlSiteOrWnd* pSiteOrWnd, LPMSG lpMsg)
{
	ENSURE_ARG(pWndDlg!=NULL);
	COleControlSiteOrWnd* pWndStart = pSiteOrWnd;
	COleControlSiteOrWnd* pWndT;
	int i = 0;

	// Check if we are in a group box so we can find local mnemonics.
	HWND hWnd = NULL;
	if (pSiteOrWnd != NULL)
	{
		hWnd = pSiteOrWnd->m_pSite ? pSiteOrWnd->m_pSite->m_hWnd : pSiteOrWnd->m_hWnd;
	}

	if (hWnd != NULL)
	{
		CWnd *pTemp = _AfxGetChildControl(pWndDlg, CWnd::FromHandle(hWnd));
		if (pTemp)
			pWndStart = _AfxFindSiteOrWnd(pWndDlg, pTemp);
	}

	while ((pWndT = pWndDlg->GetNextDlgGroupItem(pWndStart)) != NULL)
	{
		i++;

		// Avoid infinite looping.
		if (pWndT == pSiteOrWnd || i > 60)
			break;

		pWndStart = pWndT;

		if (COccManager::IsMatchingMnemonic(pWndT, lpMsg))
			return pWndT;
	}

	// walk list to find pSiteOrWnd
	COleControlContainer *pCtrlCont = pWndDlg->GetControlContainer();
	if (pCtrlCont == NULL) 
		return NULL;
		
	POSITION pos = pCtrlCont->m_listSitesOrWnds.GetHeadPosition();

	if (pSiteOrWnd)
	{
		pWndT = NULL;
		while (pos != NULL && pWndT != pSiteOrWnd)
			pWndT = pCtrlCont->m_listSitesOrWnds.GetNext(pos);
	}
	else
		pWndT = pSiteOrWnd = pCtrlCont->m_listSitesOrWnds.GetNext(pos);

	if (!pos || !pWndT)
		return NULL;

	// walk list from pSiteOrWnd checking for matching mnemonic
	pWndT = NULL;
	while (pWndT != pSiteOrWnd)
	{
		if (!pos)
			pos = pCtrlCont->m_listSitesOrWnds.GetHeadPosition();

		pWndT = pCtrlCont->m_listSitesOrWnds.GetNext(pos);
		if (COccManager::IsMatchingMnemonic(pWndT, lpMsg))
		{
			HWND hWndSiteOrWnd = NULL;
			if (pWndT != NULL)
			{
				hWndSiteOrWnd = pWndT->m_pSite ? pWndT->m_pSite->m_hWnd : pWndT->m_hWnd;
			}

			if (hWndSiteOrWnd)
			{
				if (::IsWindowEnabled(hWndSiteOrWnd))
					return pWndT;
			}
			else
			{
				ENSURE_VALID(pWndT->m_pSite);
				DWORD dwStyle = pWndT->m_pSite->GetStyle();
				if (!(dwStyle & WS_DISABLED))
					return pWndT;
			}
		}
	}
	return NULL;
}

BOOL AFX_CDECL COccManager::IsLabelControl(CWnd* pWnd)
{
	ENSURE_ARG(pWnd!=NULL);
	return pWnd->IsWindowEnabled() && (pWnd->m_pCtrlSite != NULL) &&
		pWnd->m_pCtrlSite->m_dwMiscStatus & OLEMISC_ACTSLIKELABEL;
}

BOOL AFX_CDECL COccManager::IsLabelControl(COleControlSiteOrWnd* pSiteOrWnd)
{
	HWND hWnd = NULL;
    if(pSiteOrWnd==NULL)
    {
        return FALSE;
    }

	hWnd = pSiteOrWnd->m_pSite ? pSiteOrWnd->m_pSite->m_hWnd : pSiteOrWnd->m_hWnd;

	if (hWnd)
	{
		if (!::IsWindowEnabled(hWnd))
			return FALSE;
	}
	else
	{
		if (pSiteOrWnd->m_pSite)
		{
			DWORD dwStyle = pSiteOrWnd->m_pSite->GetStyle();
			if (dwStyle & WS_DISABLED)
				return FALSE;
		}
	}
	return ((pSiteOrWnd->m_pSite != NULL) && 
			(pSiteOrWnd->m_pSite->m_dwMiscStatus & OLEMISC_ACTSLIKELABEL));
}

AFX_STATIC COleControlSiteOrWnd* AFXAPI _AfxGetNextMnem(CWnd* pWndDlg, CWnd* pWnd, LPMSG lpMsg)
{
	COleControlSiteOrWnd* pWndFirstFound = NULL;
	COleControlSiteOrWnd* pSiteOrWnd = _AfxFindSiteOrWnd(pWndDlg, pWnd);

	// set pSiteOrWnd to control with focus
	if (!pSiteOrWnd)
	{
		COleControlContainer *pCtrlCont = pWndDlg->GetControlContainer();
		if (pCtrlCont)
		{
			POSITION pos = pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
			while(pos)
			{
				COleControlSiteOrWnd *pSiteOrWndFocus = pCtrlCont->m_listSitesOrWnds.GetNext(pos);
				// Find control with focus in m_listSitesOrWnds
				if((pSiteOrWndFocus->m_pSite && pSiteOrWndFocus->m_pSite == pCtrlCont->m_pSiteFocus) ||
					(pSiteOrWndFocus->m_hWnd && pSiteOrWndFocus->m_hWnd == ::GetFocus()))
				{
					pSiteOrWnd = pSiteOrWndFocus;
					break;
				}
			}
		}
	}

	// Loop for a long time but not long enough so we hang...
	for (int count = 0; count < 256*2; count++)
	{
		// If the dialog box doesn't have the mnemonic specified, return NULL.
		if ((pSiteOrWnd = _AfxFindNextMnem(pWndDlg, pSiteOrWnd, lpMsg)) == NULL)
			return NULL;

		// If a non-disabled static item, then jump ahead to nearest tabstop.
		if (COccManager::IsLabelControl(pSiteOrWnd))
		{
			pSiteOrWnd = pWndDlg->GetNextDlgTabItem(pSiteOrWnd, FALSE);
			if (pSiteOrWnd == NULL)
				return NULL;
		}

		HWND hWnd = NULL;
		if (pSiteOrWnd != NULL)
		{
			hWnd = pSiteOrWnd->m_pSite ? pSiteOrWnd->m_pSite->m_hWnd : pSiteOrWnd->m_hWnd;
		}

		if (hWnd)
		{
			if (::IsWindowEnabled(hWnd))
				return pSiteOrWnd;
		}
		else
		{
			ENSURE_VALID(pSiteOrWnd->m_pSite);
			DWORD dwStyle = pSiteOrWnd->m_pSite->GetStyle();
			if (!(dwStyle & WS_DISABLED))
				return pSiteOrWnd;
		}

		// Stop if we've looped back to the first item we checked
		if (pSiteOrWnd == pWndFirstFound)
			return NULL;

		if (pWndFirstFound == NULL)
			pWndFirstFound = pSiteOrWnd;
	}

	return NULL;
}

void AFX_CDECL COccManager::UIActivateControl(CWnd* pWndNewFocus)
{
	if (pWndNewFocus == NULL)
		return;

	// Find the nearest control in the window parent chain.
	CWnd* pWndCtrl = pWndNewFocus;
	COleControlContainer* pCtrlCont = NULL;
	COleControlSite* pCtrlSite = NULL;
	while ((pWndCtrl != NULL) &&
		((pCtrlCont = pWndCtrl->m_pCtrlCont) == NULL) &&
		((pCtrlSite = pWndCtrl->m_pCtrlSite) == NULL))
	{
		pWndCtrl = pWndCtrl->GetParent();
	}

	if ((pWndCtrl == NULL) || (pCtrlCont != NULL))
		return;

	// This will UI Activate the control.
	pCtrlSite->SetFocus();

	// Make sure focus gets set to correct child of control, if any.
	if ((CWnd::GetFocus() != pWndNewFocus) && ::IsWindow(pWndNewFocus->GetSafeHwnd()))
		pWndNewFocus->SetFocus();
}

void AFX_CDECL COccManager::UIDeactivateIfNecessary(CWnd* pWndOldFocus,
	CWnd* pWndNewFocus)
{
	if (pWndOldFocus == NULL || !::IsWindow(pWndOldFocus->m_hWnd))
		return;

	if(pWndOldFocus == pWndNewFocus)
		return;

	// Find the nearest control container in the window parent chain.
	CWnd* pWndCtrlCont = pWndOldFocus;
	COleControlContainer* pCtrlCont = NULL;

	while(pWndCtrlCont && (pCtrlCont = pWndCtrlCont->m_pCtrlCont) == NULL)
	{
		pWndCtrlCont = pWndCtrlCont->GetParent();
	}

	if(!pCtrlCont)
		return;

	// Get the current UI Active control (if any).
	CWnd* pWndUIActive = NULL;
	COleControlSite* pSite = pCtrlCont->m_pSiteUIActive;
	if(pSite && !pSite->m_bIsWindowless)
	{
		pWndUIActive = CWnd::FromHandle(pSite->m_hWnd);
		if (pWndUIActive == NULL)
			return;
	}

	// Ignore if the control getting the focus is the same control.
	if (pWndNewFocus && pWndUIActive &&
		(pWndNewFocus == pWndUIActive || pWndUIActive->IsChild(pWndNewFocus)))
		return;

	// Tell the container to UI Deactivate the UI Active control.
	pCtrlCont->OnUIActivate(NULL);
}

CWnd* AFXAPI _AfxFindDlgItem(CWnd* pWndParent, DWORD id)
{
	CWnd* pWndChild;
	CWnd* pWndOrig;

	// QUICK TRY:
	pWndChild = pWndParent->GetDlgItem(id);
	if (pWndChild != NULL)
		return pWndChild;

	pWndOrig = _AfxNextControl(pWndParent, NULL, CWP_SKIPINVISIBLE);
	if (pWndOrig == pWndParent)
		return NULL;

	pWndChild = pWndOrig;

	do
	{
		if ((DWORD)pWndChild->GetDlgCtrlID() == id)
			return(pWndChild);

		pWndChild = _AfxNextControl(pWndParent, pWndChild, CWP_SKIPINVISIBLE);
	}
	while ((pWndChild != NULL) && (pWndChild != pWndOrig));

	return NULL;
}

void COccManager::SetDefaultButton(CWnd* pWnd, BOOL bDefault)
{
	if (pWnd->m_pCtrlSite != NULL)
	{
		pWnd->m_pCtrlSite->SetDefaultButton(bDefault);
	}
	else
	{
		DWORD code = _AfxGetDlgCode(pWnd);
		if (code & (bDefault ? DLGC_UNDEFPUSHBUTTON : DLGC_DEFPUSHBUTTON))
			pWnd->SendMessage(BM_SETSTYLE,
				(WPARAM)(bDefault ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
				(LPARAM)(DWORD)TRUE);
	}
}

DWORD AFX_CDECL COccManager::GetDefBtnCode(CWnd* pWnd)
{
	if (pWnd == NULL)
		return 0;

	if (pWnd->m_pCtrlSite != NULL)
		return pWnd->m_pCtrlSite->GetDefBtnCode();

	return _AfxGetDlgCode(pWnd) & (DLGC_UNDEFPUSHBUTTON | DLGC_DEFPUSHBUTTON);
}

AFX_STATIC void AFXAPI _AfxRemoveDefaultButton(CWnd* pWndRoot, CWnd* pWndStart)
{
	if ((pWndStart == NULL) || IsControlParent(pWndStart))
		pWndStart = _AfxNextControl(pWndRoot, NULL, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
	else
		pWndStart = _AfxGetChildControl(pWndRoot, pWndStart);

	if (pWndStart == NULL)
		return;

	CWnd* pWnd = pWndStart;
	CWnd* pWndNext;

	do
	{
		COccManager::SetDefaultButton(pWnd, FALSE);
		pWndNext = _AfxNextControl(pWndRoot, pWnd, 0);
		pWnd = pWndNext;
	}
	while ((pWnd != NULL) && (pWnd != pWndStart));
}

AFX_STATIC int AFXAPI _AfxOriginalDefButton(CWnd* pWndRoot)
{
	LRESULT lResult = pWndRoot->SendMessage(DM_GETDEFID, 0, 0L);
	return HIWORD(lResult) == DC_HASDEFID ? LOWORD(lResult) : IDOK;
}

AFX_STATIC void AFXAPI _AfxCheckDefPushButton(CWnd* pWndRoot, CWnd* pWndOldFocus,
	CWnd* pWndNewFocus)
{
	DWORD code = 0;
	CWnd* pWndT;

	// If the focus has gone to a totally separate window, bail out.
	if (!pWndRoot->IsChild(pWndNewFocus))
		return;

	if (pWndNewFocus != NULL)
	{
		// Do nothing if clicking on dialog background or recursive dialog
		// background.
		if (IsControlParent(pWndNewFocus))
			return;

		code = COccManager::GetDefBtnCode(pWndNewFocus);
	}

	if (pWndOldFocus == pWndNewFocus)
	{
		// Check the default ID and see if is the same as pwndOldFocus' ID.
		// If not, find it and use it as pwndOldFocus
		if (code & DLGC_UNDEFPUSHBUTTON)
		{
			if (pWndOldFocus != NULL)
			{
				pWndOldFocus = _AfxFindDlgItem(pWndRoot, _AfxOriginalDefButton(pWndRoot));
				if ((pWndOldFocus != NULL) && (pWndOldFocus != pWndNewFocus))
				{
					if (COccManager::GetDefBtnCode(pWndOldFocus) & DLGC_DEFPUSHBUTTON)
					{
						_AfxRemoveDefaultButton(pWndRoot, pWndOldFocus);
						goto SetNewDefault;
					}
				}
			}

			COccManager::SetDefaultButton(pWndNewFocus, TRUE);
		}
		return;
	}

	// If the focus is changing to or from a pushbutton, then remove the
	// default style from the current default button
	if (((pWndOldFocus != NULL) && (COccManager::GetDefBtnCode(pWndOldFocus) != 0)) ||
		((pWndNewFocus != NULL) && (code != 0)))
	{
		_AfxRemoveDefaultButton(pWndRoot, pWndNewFocus);
	}

SetNewDefault:
	// If moving to a button, make that button the default.
	if (code & (DLGC_UNDEFPUSHBUTTON | DLGC_DEFPUSHBUTTON))
	{
		COccManager::SetDefaultButton(pWndNewFocus, TRUE);
	}
	else
	{
		// Otherwise, make sure the original default button is default

		// Get the original default button
		pWndT = _AfxFindDlgItem(pWndRoot, _AfxOriginalDefButton(pWndRoot));

		if ((COccManager::GetDefBtnCode(pWndT) & DLGC_UNDEFPUSHBUTTON) &&
			pWndT->IsWindowEnabled())
		{
			COccManager::SetDefaultButton(pWndT, TRUE);
		}
	}
}

BOOL COccManager::IsDialogMessage(CWnd* pWndDlg, LPMSG lpMsg)
{
	ASSERT(pWndDlg);

	// If an OLE Control has the focus, then give it the first crack at key
	// and mouse messages.
	HWND hWndDlg = pWndDlg->GetSafeHwnd();
	UINT uMsg = lpMsg->message;
	HWND hWndFocus = NULL;
	CWnd* pWndFocus = NULL;
	COleControlSiteOrWnd *pSiteOrWnd;

	hWndFocus = ::GetFocus();
	pWndFocus= CWnd::FromHandle(hWndFocus);

	if (((uMsg >= WM_KEYFIRST) && (uMsg <= WM_KEYLAST)) ||
		((uMsg >= WM_MOUSEFIRST) && (uMsg <= AFX_WM_MOUSELAST)))
	{
		if(pWndFocus)
		{
			CWnd* pWndCtrl = pWndFocus;

			// Walk up the parent chain, until we find an OLE control.
			while ((pWndCtrl != NULL) && (pWndCtrl->m_pCtrlSite == NULL) &&
				(pWndCtrl->GetParent() != pWndDlg))
			{
				pWndCtrl = pWndCtrl->GetParent();
			}

			// let the control attempt to translate the message
			if (pWndCtrl != NULL && pWndCtrl->m_pCtrlSite != NULL &&
				pWndCtrl->m_pCtrlSite->m_pActiveObject != NULL &&
				pWndCtrl->m_pCtrlSite->m_pActiveObject->TranslateAccelerator(lpMsg) == S_OK)
			{
				return TRUE;
			}

			// handle CTRLINFO_EATS_RETURN and CTRLINFO_EATS_ESCAPE flags
			if ((uMsg == WM_KEYUP || uMsg == WM_KEYDOWN || uMsg == WM_CHAR) &&
				pWndCtrl != NULL && pWndCtrl->m_pCtrlSite != NULL &&
				((LOWORD(lpMsg->wParam) == VK_RETURN && 
				 (pWndCtrl->m_pCtrlSite->m_ctlInfo.dwFlags & CTRLINFO_EATS_RETURN)) ||
				(LOWORD(lpMsg->wParam) == VK_ESCAPE &&
				 (pWndCtrl->m_pCtrlSite->m_ctlInfo.dwFlags & CTRLINFO_EATS_ESCAPE))))
			{
				return FALSE;
			}
		}
	}

	BOOL bResult = FALSE;
	CWnd* pWndMsg = CWnd::FromHandle(lpMsg->hwnd);
	CWnd* pWndNext = NULL;
	DWORD code;
	BOOL bBack = FALSE;
	int iOK = IDCANCEL;

	switch (uMsg)
	{
	case WM_SYSCHAR:
		// If no control has focus, and Alt not down, then ignore.
		if ((pWndFocus == NULL) && (GetKeyState(VK_MENU) >= 0))
			break;

		// If alt+menuchar, process as menu.
		if (LOWORD(lpMsg->wParam) == CH_SYSMENU)
			break;

		// FALL THRU

	case WM_CHAR:
		code = _AfxGetDlgCode(pWndMsg, lpMsg);

		// If the control wants to process the message, then don't check
		// for possible mnemonic key.
		if (uMsg == WM_CHAR && (code & (DLGC_WANTCHARS|DLGC_WANTMESSAGE)))
			break;

		// If the control wants tabs, then don't let tab fall thru here
		if (LOWORD(lpMsg->wParam) == VK_TAB && (code & DLGC_WANTTAB))
			break;

		// Don't handle space as a mnemonic
		if (LOWORD(lpMsg->wParam) == VK_SPACE)
			return FALSE;  // Let the control have a chance to handle the space key

		if ((pSiteOrWnd = _AfxGetNextMnem(pWndDlg, pWndMsg, lpMsg)) != NULL)
		{
			if (pSiteOrWnd->m_pSite != NULL)
			{
				// UI Activate new control, and send the mnemonic to it.
				pSiteOrWnd->m_pSite->SendMnemonic(lpMsg);
				bResult = TRUE;
			}
		}
		break;

	case WM_KEYDOWN:
		code = _AfxGetDlgCode(pWndMsg, lpMsg);
		switch (LOWORD(lpMsg->wParam))
		{
		case VK_TAB:
			{
				if (code & DLGC_WANTTAB)    // If control wants tabs, bail out.
					break;

				bBack = GetKeyState(VK_SHIFT) < 0;
				pSiteOrWnd = pWndDlg->GetNextDlgTabItem((COleControlSiteOrWnd*)NULL, bBack);
				if(pSiteOrWnd)
				{
					if(pSiteOrWnd->m_pSite)
					{						
						pSiteOrWnd->m_pSite->SetFocus(lpMsg);
					}
					else
					{
						// If we failed to find the next window fall back
						// on the tried-and-true method.
						pWndNext = pSiteOrWnd->m_hWnd ?
							CWnd::FromHandle(pSiteOrWnd->m_hWnd) :
							pWndDlg->GetNextDlgTabItem(pWndMsg, bBack);

						if(pWndNext)
						{
							pWndDlg->m_pCtrlCont->m_pSiteFocus = NULL;
							_AfxDlgSetFocus(pWndNext);
							UIDeactivateIfNecessary(pWndFocus, pWndNext);
						}
					}
					bResult = TRUE;
				}
			}
			break;

		case VK_LEFT:
		case VK_UP:
			bBack = TRUE;
			// FALL THRU

		case VK_RIGHT:
		case VK_DOWN:
			if (_AfxGetDlgCode(pWndFocus, lpMsg) & DLGC_WANTARROWS)
				break;

			if(bBack)
				pSiteOrWnd = pWndDlg->GetPrevDlgGroupItem();
			else
				pSiteOrWnd = pWndDlg->GetNextDlgGroupItem();

			if(pSiteOrWnd)
			{
				// If the control is an auto-check radio button we
				// need to uncheck the currently checked radio button.
				if(pSiteOrWnd->m_bAutoRadioButton)
					pWndDlg->RemoveRadioCheckFromGroup(pSiteOrWnd);

				if(pSiteOrWnd->m_pSite)
				{
					pSiteOrWnd->m_pSite->SetFocus(lpMsg);
					bResult = TRUE;
				}
				else
				{
					// Just in case, beyond all hope we failed to find then next
					// control, fall back on the tried and true method...
					pWndNext = pSiteOrWnd->m_hWnd ?
						CWnd::FromHandle(pSiteOrWnd->m_hWnd) :
						pWndDlg->GetNextDlgGroupItem(pWndFocus, bBack);

					if(pWndNext)
					{
						// If the old control was an ActiveX control, clear it.
						pWndDlg->m_pCtrlCont->m_pSiteFocus = NULL;
						_AfxDlgSetFocus(pWndNext);

						// If the new control is an auto-check radio button, set
						// the check.
						if(pSiteOrWnd->m_bAutoRadioButton)
							pWndNext->SendMessage(BM_SETCHECK, BST_CHECKED, 0);
						bResult = TRUE;
					}
				}
			}
			break;

		case VK_EXECUTE:
		case VK_RETURN:
			// Return was pressed.
		 if (code&DLGC_WANTALLKEYS)
			break;

		 // Find default button and click it.
			if (GetDefBtnCode(pWndFocus) & DLGC_DEFPUSHBUTTON)
			{
				pWndNext = pWndFocus;
				iOK = (DWORD)pWndNext->GetDlgCtrlID();
			}
			else
			{
				iOK = _AfxOriginalDefButton(pWndDlg);
			}
			// FALL THRU

		case VK_ESCAPE:
		case VK_CANCEL:
			if (pWndNext == NULL)
			{
				pWndNext = _AfxFindDlgItem(pWndDlg, iOK);
				if (pWndNext == NULL)
					break;
			}
			ASSERT(pWndNext != NULL);

			// Make sure button is not disabled.
			if (!pWndNext->IsWindowEnabled())
			{
				MessageBeep(0);
			}
			else if (pWndNext->m_pCtrlSite != NULL)
			{
				// "push" the pWndNext control.
				TRY
				{
					pWndNext->InvokeHelper(DISPID_DOCLICK, DISPATCH_METHOD,
						VT_EMPTY, NULL, VTS_NONE);
				}
				END_TRY
				bResult = TRUE;
			}
			break;
		}
		break;
	}

	// As a last resort, delegate to the Windows implementation
	if (!bResult && (pWndDlg->m_nFlags & WF_ISWINFORMSVIEWWND)==0)
	{
		bResult = ::IsDialogMessage(pWndDlg->m_hWnd, lpMsg);
		if (bResult && (CWnd::GetFocus() != pWndFocus))
			UIActivateControl(CWnd::GetFocus());
	}

	if (::IsWindow(hWndFocus))
	{
		UIDeactivateIfNecessary(pWndFocus, CWnd::GetFocus());
		if (::IsWindow(hWndDlg))
			_AfxCheckDefPushButton(pWndDlg, pWndFocus, CWnd::GetFocus());
	}
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
