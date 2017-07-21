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
#include "afxkeyboardmanager.h"
#include "afxmultidoctemplateex.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxsettingsstore.h"
#include "afxacceleratorkey.h"
#include "afxtoolbar.h"
#include "afxregpath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_REG_SECTION_FMT _T("%sKeyboard-%d")
#define AFX_REG_ENTRY_DATA _T("Accelerators")

CKeyboardManager* afxKeyboardManager = NULL;

static const CString strKbProfile = _T("KeyboardManager");

LPACCEL CKeyboardManager::m_lpAccel = NULL;
LPACCEL CKeyboardManager::m_lpAccelDefault = NULL;
int CKeyboardManager::m_nAccelDefaultSize = 0;
int CKeyboardManager::m_nAccelSize = 0;
HACCEL CKeyboardManager::m_hAccelDefaultLast = NULL;
HACCEL CKeyboardManager::m_hAccelLast = NULL;
BOOL CKeyboardManager::m_bAllAccelerators = FALSE;
CString CKeyboardManager::m_strDelimiter = _T("; ");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKeyboardManager::CKeyboardManager()
{
	ENSURE(afxKeyboardManager == NULL);
	afxKeyboardManager = this;
}

CKeyboardManager::~CKeyboardManager()
{
	afxKeyboardManager = NULL;
}

BOOL CKeyboardManager::UpdateAccelTable(CMultiDocTemplate* pTemplate, LPACCEL lpAccel, int nSize, CFrameWnd* pDefaultFrame)
{
	ENSURE(lpAccel != NULL);

	// Create a new accelerator table:
	HACCEL hAccelNew = ::CreateAcceleratorTable(lpAccel, nSize);
	if (hAccelNew == NULL)
	{
		TRACE(_T("Can't create accelerator table!\n"));
		return FALSE;
	}

	if (!UpdateAccelTable(pTemplate, hAccelNew, pDefaultFrame))
	{
		::DestroyAcceleratorTable(hAccelNew);
		return FALSE;
	}

	return TRUE;
}

BOOL CKeyboardManager::UpdateAccelTable(CMultiDocTemplate* pTemplate, HACCEL hAccelNew, CFrameWnd* pDefaultFrame)
{
	ENSURE(hAccelNew != NULL);

	// Find an existing accelerator table associated with template:
	HACCEL hAccelTable = NULL;

	if (pTemplate != NULL)
	{
		ENSURE(pDefaultFrame == NULL);

		ASSERT_VALID(pTemplate);
		hAccelTable = pTemplate->m_hAccelTable;
		ENSURE(hAccelTable != NULL);

		pTemplate->m_hAccelTable = hAccelNew;

		// Walk trougth all template's documents and change
		// frame's accelerator tables:
		for (POSITION pos = pTemplate->GetFirstDocPosition(); pos != NULL;)
		{
			CDocument* pDoc = pTemplate->GetNextDoc(pos);
			ASSERT_VALID(pDoc);

			for (POSITION posView = pDoc->GetFirstViewPosition(); posView != NULL;)
			{
				CView* pView = pDoc->GetNextView(posView);
				ASSERT_VALID(pView);

				CFrameWnd* pFrame = pView->GetParentFrame();
				ASSERT_VALID(pFrame);

				if (pFrame->m_hAccelTable == hAccelTable)
				{
					pFrame->m_hAccelTable = hAccelNew;
				}
			}
		}
	}
	else
	{
		if (pDefaultFrame == NULL)
		{
			pDefaultFrame = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd());
		}

		if (pDefaultFrame != NULL)
		{
			hAccelTable = pDefaultFrame->m_hAccelTable;
			pDefaultFrame->m_hAccelTable = hAccelNew;
		}
	}

	if (hAccelTable == NULL)
	{
		TRACE(_T("Accelerator table not found!\n"));
		return FALSE;
	}

	::DestroyAcceleratorTable(hAccelTable);
	return TRUE;
}

BOOL CKeyboardManager::SaveAcceleratorState(LPCTSTR lpszProfileName, UINT uiResId, HACCEL hAccelTable)
{
	ENSURE(hAccelTable != NULL);

	CString strSection;
	strSection.Format(AFX_REG_SECTION_FMT, lpszProfileName, uiResId);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	int nAccelSize = ::CopyAcceleratorTable(hAccelTable, NULL, 0);
	if (nAccelSize == 0)
	{
		return FALSE;
	}

	if (!reg.CreateKey(strSection))
	{
		return FALSE;
	}

	LPACCEL lpAccel = new ACCEL [nAccelSize];
	ENSURE(lpAccel != NULL);

	::CopyAcceleratorTable(hAccelTable, lpAccel, nAccelSize);

	reg.Write(AFX_REG_ENTRY_DATA, (LPBYTE) lpAccel, nAccelSize * sizeof(ACCEL));

	delete [] lpAccel;
	return TRUE;
}

BOOL CKeyboardManager::LoadAcceleratorState(LPCTSTR lpszProfileName, UINT uiResId, HACCEL& hAccelTable)
{
	ENSURE(hAccelTable == NULL);

	CString strSection;
	strSection.Format(AFX_REG_SECTION_FMT, lpszProfileName, uiResId);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	UINT uiSize;
	LPACCEL lpAccel;

	if (reg.Read(AFX_REG_ENTRY_DATA, (LPBYTE*) &lpAccel, &uiSize))
	{
		int nAccelSize = uiSize / sizeof(ACCEL);

		ENSURE(lpAccel != NULL);

		for (int i = 0; i < nAccelSize; i ++)
		{
			if (!CMFCToolBar::IsCommandPermitted(lpAccel [i].cmd))
			{
				lpAccel [i].cmd = 0;
			}
		}

		hAccelTable = ::CreateAcceleratorTable(lpAccel, nAccelSize);
	}

	delete [] lpAccel;
	return hAccelTable != NULL;
}

BOOL CKeyboardManager::LoadState(LPCTSTR lpszProfileName, CFrameWnd* pDefaultFrame)
{
	CString strProfileName = ::AFXGetRegPath(strKbProfile, lpszProfileName);

	CDocManager* pDocManager = AfxGetApp()->m_pDocManager;
	if (pDocManager != NULL)
	{
		// Walk all templates in the application:
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition(); pos != NULL;)
		{
			CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) pDocManager->GetNextDocTemplate(pos);
			ASSERT_VALID(pTemplate);
			ASSERT_KINDOF(CDocTemplate, pTemplate);

			// We are interessing CMultiDocTemplate objects with
			// the sahred menu only....
			if (!pTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)) || pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId();
			ENSURE(uiResId != 0);

			HACCEL hAccelTable = NULL;
			if (LoadAcceleratorState(strProfileName, uiResId, hAccelTable))
			{
				UpdateAccelTable(pTemplate, hAccelTable);
			}
		}
	}

	// Save default accelerator table:
	if (pDefaultFrame == NULL)
	{
		pDefaultFrame = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd());
	}

	if (pDefaultFrame != NULL && pDefaultFrame->m_hAccelTable != NULL)
	{
		HACCEL hAccelTable = NULL;
		if (LoadAcceleratorState(strProfileName, 0, hAccelTable))
		{
			UpdateAccelTable(NULL, hAccelTable, pDefaultFrame);
		}
	}

	return TRUE;
}

BOOL CKeyboardManager::SaveState(LPCTSTR lpszProfileName, CFrameWnd* pDefaultFrame)
{
	CString strProfileName = ::AFXGetRegPath(strKbProfile, lpszProfileName);

	CDocManager* pDocManager = AfxGetApp()->m_pDocManager;
	if (pDocManager != NULL)
	{
		// Walk all templates in the application:
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition(); pos != NULL;)
		{
			CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) pDocManager->GetNextDocTemplate(pos);
			ASSERT_VALID(pTemplate);
			ASSERT_KINDOF(CDocTemplate, pTemplate);

			// We are interessing CMultiDocTemplate objects in
			// the shared accelerator table only....
			if (!pTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)) || pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId();
			ENSURE(uiResId != 0);

			SaveAcceleratorState(strProfileName, uiResId, pTemplate->m_hAccelTable);
		}
	}

	// Save default accelerator table:
	if (pDefaultFrame == NULL)
	{
		pDefaultFrame = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd());
	}

	if (pDefaultFrame != NULL && pDefaultFrame->m_hAccelTable != NULL)
	{
		SaveAcceleratorState(strProfileName, 0, pDefaultFrame->m_hAccelTable);
	}

	return TRUE;
}

void CKeyboardManager::ResetAll()
{
	CDocManager* pDocManager = AfxGetApp()->m_pDocManager;
	if (pDocManager != NULL)
	{
		// Walk all templates in the application:
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition(); pos != NULL;)
		{
			CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) pDocManager->GetNextDocTemplate(pos);
			ASSERT_VALID(pTemplate);
			ASSERT_KINDOF(CDocTemplate, pTemplate);

			// We are interessing CMultiDocTemplate objects in
			// the shared accelerator table only....
			if (!pTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)) || pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId();
			ENSURE(uiResId != 0);

			LPCTSTR lpszResourceName = MAKEINTRESOURCE(uiResId);
			ENSURE(lpszResourceName != NULL);

			HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_MENU);

			HACCEL hAccellTable = ::LoadAccelerators(hInst, lpszResourceName);
			if (hAccellTable != NULL)
			{
				UpdateAccelTable(pTemplate, hAccellTable);
			}
		}
	}

	// Restore default accelerator table:
	CFrameWnd* pWndMain = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd());
	if (pWndMain != NULL && pWndMain->m_hAccelTable != NULL)
	{
		UINT uiResId = 0;

		CMDIFrameWndEx* pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, AfxGetMainWnd());
		if (pMDIFrame != NULL)
		{
			uiResId = pMDIFrame->GetDefaultResId();
		}
		else // Maybe, SDI frame...
		{
			CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, AfxGetMainWnd());
			if (pFrame != NULL)
			{
				uiResId = pFrame->GetDefaultResId();
			}
			else // Maybe, OLE frame...
			{
				COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, AfxGetMainWnd());
				if (pOleFrame != NULL)
				{
					uiResId = pOleFrame->GetDefaultResId();
				}
			}
		}

		if (uiResId != 0)
		{
			LPCTSTR lpszResourceName = MAKEINTRESOURCE(uiResId);
			ENSURE(lpszResourceName != NULL);

			HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_MENU);

			HACCEL hAccellTable = ::LoadAccelerators(hInst, lpszResourceName);
			if (hAccellTable != NULL)
			{
				UpdateAccelTable(NULL, hAccellTable);
			}
		}
	}
}

BOOL __stdcall CKeyboardManager::FindDefaultAccelerator(UINT uiCmd, CString& str, CFrameWnd* pWndFrame, BOOL bIsDefaultFrame)
{
	str.Empty();

	if (pWndFrame == NULL)
	{
		return FALSE;
	}

	HACCEL hAccelTable = pWndFrame->GetDefaultAccelerator();
	if (hAccelTable == NULL)
	{
		return FALSE;
	}

	int& nSize = bIsDefaultFrame ? m_nAccelDefaultSize : m_nAccelSize;
	LPACCEL& lpAccel = bIsDefaultFrame ? m_lpAccelDefault : m_lpAccel;

	SetAccelTable( lpAccel, bIsDefaultFrame ? m_hAccelDefaultLast : m_hAccelLast, nSize, hAccelTable);

	ENSURE(lpAccel != NULL);

	BOOL bFound = FALSE;
	for (int i = 0; i < nSize; i ++)
	{
		if (lpAccel [i].cmd == uiCmd)
		{
			bFound = TRUE;

			CMFCAcceleratorKey helper(&lpAccel [i]);

			CString strKey;
			helper.Format(strKey);

			if (!str.IsEmpty())
			{
				str += m_strDelimiter;
			}

			str += strKey;

			if (!m_bAllAccelerators)
			{
				break;
			}
		}
	}

	return bFound;
}

void __stdcall CKeyboardManager::SetAccelTable(LPACCEL& lpAccel, HACCEL& hAccelLast, int& nSize, const HACCEL hAccelCur)
{
	ENSURE(hAccelCur != NULL);
	if (hAccelCur == hAccelLast)
	{
		ENSURE(lpAccel != NULL);
		return;
	}

	// Destroy old acceleration table:
	if (lpAccel != NULL)
	{
		delete [] lpAccel;
		lpAccel = NULL;
	}

	nSize = ::CopyAcceleratorTable(hAccelCur, NULL, 0);

	lpAccel = new ACCEL [nSize];
	ENSURE(lpAccel != NULL);

	::CopyAcceleratorTable(hAccelCur, lpAccel, nSize);

	hAccelLast = hAccelCur;
}

BOOL __stdcall CKeyboardManager::IsKeyPrintable(const UINT nChar)
{
	// Ensure the key is printable:
	BYTE lpKeyState [256];
	ENSURE(::GetKeyboardState(lpKeyState));

#ifndef _UNICODE
	WORD wChar = 0;
	int nRes = ::ToAsciiEx(nChar, MapVirtualKey(nChar, 0), lpKeyState, &wChar, 0, //FOR BETA 6.8 - CHANGE BACK TO 1 IF USERS REPORT ANY PROBLEMS
		::GetKeyboardLayout(AfxGetThread()->m_nThreadID));

#else
	TCHAR szChar [2];
	memset(szChar, 0, sizeof(TCHAR) * 2);

	int nRes = ::ToUnicodeEx(nChar, MapVirtualKey(nChar, 0), lpKeyState, szChar, 2, 0, //FOR BETA 6.8 - CHANGE BACK TO 1 IF USERS REPORT ANY PROBLEMS
		::GetKeyboardLayout(AfxGetThread()->m_nThreadID));
#endif // _UNICODE

	return nRes > 0;
}

UINT __stdcall CKeyboardManager::TranslateCharToUpper(const UINT nChar)
{
	if (nChar < VK_NUMPAD0 || nChar > VK_NUMPAD9 ||
		(::GetAsyncKeyState(VK_MENU) & 0x8000))
	{
		if (!CMFCToolBar::m_bExtCharTranslation)
		{
			// locale independent code:
			if ((nChar < 0x41 || nChar > 0x5A))
			{
				if (::GetAsyncKeyState(VK_MENU) & 0x8000)
				{
					return nChar;
				}
				else
				{
					return toupper(nChar);
				}
			}
			else // virt codes(A - Z)
			{
				return nChar;
			}
		}
	}

	// locale dependent code:
#ifndef _UNICODE
	WORD wChar = 0;
	BYTE lpKeyState [256];
	::GetKeyboardState(lpKeyState);

	::ToAsciiEx(nChar, MapVirtualKey(nChar, 0), lpKeyState, &wChar, 1, ::GetKeyboardLayout(AfxGetThread()->m_nThreadID));

	TCHAR szChar [2] = {(TCHAR) wChar, '\0'};
#else
	TCHAR szChar [2];
	memset(szChar, 0, sizeof(TCHAR) * 2);
	BYTE lpKeyState [256];
	ENSURE(::GetKeyboardState(lpKeyState));

	::ToUnicodeEx(nChar, MapVirtualKey(nChar, 0), lpKeyState, szChar, 2, 1, ::GetKeyboardLayout(AfxGetThread()->m_nThreadID));
#endif // _UNICODE

	CharUpper(szChar);

	return(UINT)szChar [0];
}

void __stdcall CKeyboardManager::CleanUp()
{
	if (m_lpAccel != NULL)
	{
		delete [] m_lpAccel;
		m_lpAccel = NULL;

	}

	if (m_lpAccelDefault != NULL)
	{
		delete [] m_lpAccelDefault;
		m_lpAccelDefault = NULL;
	}
}

BOOL __stdcall CKeyboardManager::IsKeyHandled(WORD nKey, BYTE fVirt, CFrameWnd* pWndFrame, BOOL bIsDefaultFrame)
{
	if (pWndFrame == NULL)
	{
		return FALSE;
	}

	HACCEL hAccelTable = pWndFrame->GetDefaultAccelerator();
	if (hAccelTable == NULL)
	{
		return FALSE;
	}

	int& nSize = bIsDefaultFrame ? m_nAccelDefaultSize : m_nAccelSize;
	LPACCEL& lpAccel = bIsDefaultFrame ? m_lpAccelDefault : m_lpAccel;

	SetAccelTable( lpAccel, bIsDefaultFrame ? m_hAccelDefaultLast : m_hAccelLast, nSize, hAccelTable);

	ENSURE(lpAccel != NULL);

	for (int i = 0; i < nSize; i ++)
	{
		if (lpAccel [i].key == nKey && lpAccel [i].fVirt == fVirt)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void __stdcall CKeyboardManager::ShowAllAccelerators(BOOL bShowAll, LPCTSTR lpszDelimiter)
{
	if (bShowAll)
	{
		m_bAllAccelerators = TRUE;

		if (lpszDelimiter != NULL)
		{
			m_strDelimiter = lpszDelimiter;
		}
	}
	else
	{
		m_bAllAccelerators = FALSE;
	}
}


