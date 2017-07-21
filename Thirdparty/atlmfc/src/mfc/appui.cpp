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
#include "sal.h"



/////////////////////////////////////////////////////////////////////////////
// CWinApp User Interface Extensions

void CWinApp::OnAppExit()
{
	// same as double-clicking on main window close box
	ASSERT(m_pMainWnd != NULL);
	m_pMainWnd->SendMessage(WM_CLOSE);
}


void CWinApp::HideApplication()
{
	ASSERT_VALID(m_pMainWnd);

	// hide the application's windows before closing all the documents
	m_pMainWnd->ShowWindow(SW_HIDE);
	m_pMainWnd->ShowOwnedPopups(FALSE);

	// put the window at the bottom of zorder, so it isn't activated
	m_pMainWnd->SetWindowPos(&CWnd::wndBottom, 0, 0, 0, 0,
		SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}

void CWinApp::DoWaitCursor(int nCode)
{
	// 0 => restore, 1=> begin, -1=> end
	ENSURE_ARG(nCode == 0 || nCode == 1 || nCode == -1);
	ENSURE(afxData.hcurWait != NULL);
	AfxLockGlobals(CRIT_WAITCURSOR);
	m_nWaitCursorCount += nCode;
	if (m_nWaitCursorCount > 0)
	{
		HCURSOR hcurPrev = ::SetCursor(afxData.hcurWait);
		if (nCode > 0 && m_nWaitCursorCount == 1)
			m_hcurWaitCursorRestore = hcurPrev;
	}
	else
	{
		// turn everything off
		m_nWaitCursorCount = 0;     // prevent underflow
		::SetCursor(m_hcurWaitCursorRestore);
	}
	AfxUnlockGlobals(CRIT_WAITCURSOR);
}


BOOL CWinApp::SaveAllModified()
{
	if (m_pDocManager != NULL)
		return m_pDocManager->SaveAllModified();
	return TRUE;
}

void CWinApp::AddToRecentFileList(LPCTSTR lpszPathName)
{
	ASSERT_VALID(this);
	ENSURE_ARG(lpszPathName != NULL);
	ASSERT(AfxIsValidString(lpszPathName));
	
	if (m_pRecentFileList != NULL)
		m_pRecentFileList->Add(lpszPathName);
}

CDocument* CWinApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	ENSURE_VALID(m_pDocManager);
	return m_pDocManager->OpenDocumentFile(lpszFileName);
}

void CWinApp::CloseAllDocuments(BOOL bEndSession)
{
	if (m_pDocManager != NULL)
		m_pDocManager->CloseAllDocuments(bEndSession);
}

/////////////////////////////////////////////////////////////////////////////
// MRU file list default implementation

void CWinApp::OnUpdateRecentFileMenu(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);
	ENSURE_ARG(pCmdUI != NULL);
	if (m_pRecentFileList == NULL) // no MRU files
		pCmdUI->Enable(FALSE);
	else
		m_pRecentFileList->UpdateMenu(pCmdUI);
}

/////////////////////////////////////////////////////////////////////////////
// DDE and ShellExecute support

BOOL CWinApp::OnDDECommand(_In_z_ LPTSTR lpszCommand)
{
	if (m_pDocManager != NULL)
		return m_pDocManager->OnDDECommand(lpszCommand);
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// MRU file list default implementation

BOOL CWinApp::OnOpenRecentFile(UINT nID)
{
	ASSERT_VALID(this);
	ENSURE(m_pRecentFileList != NULL);

	ENSURE_ARG(nID >= ID_FILE_MRU_FILE1);
	ENSURE_ARG(nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;
	ASSERT((*m_pRecentFileList)[nIndex].GetLength() != 0);

	TRACE(traceAppMsg, 0, _T("MRU: open file (%d) '%s'.\n"), (nIndex) + 1,
			(LPCTSTR)(*m_pRecentFileList)[nIndex]);

	if (OpenDocumentFile((*m_pRecentFileList)[nIndex]) == NULL)
		m_pRecentFileList->Remove(nIndex);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
