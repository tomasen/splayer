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
#include <ddeml.h> // for MSGF_DDEMGR



/////////////////////////////////////////////////////////////////////////////
// other globals (internal library use)

/////////////////////////////////////////////////////////////////////////////
// Standard cleanup called by WinMain and AfxAbort

void AFXAPI AfxUnregisterWndClasses()
{
	// unregister Window classes
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_REGCLASSLIST);
	int start=0;
	CString className = pModuleState->m_strUnregisterList.Tokenize(_T("\n"),start);
	while (!className.IsEmpty())
	{
		UnregisterClass(static_cast<LPCTSTR>(className), AfxGetInstanceHandle());
		className = pModuleState->m_strUnregisterList.Tokenize(_T("\n"),start);
	}
	pModuleState->m_strUnregisterList.Empty();
	AfxUnlockGlobals(CRIT_REGCLASSLIST);

}
void AFXAPI AfxWinTerm(void)
{	
	AfxUnregisterWndClasses();
	// cleanup OLE if required
	CWinThread* pThread = AfxGetApp();
	if (pThread != NULL && pThread->m_lpfnOleTermOrFreeLib != NULL)
		(*pThread->m_lpfnOleTermOrFreeLib)(TRUE, FALSE);

	// cleanup thread local tooltip window
	AFX_MODULE_THREAD_STATE* pModuleThreadState = AfxGetModuleThreadState();
	if (pModuleThreadState->m_pToolTip != NULL)
	{
		if (pModuleThreadState->m_pToolTip->DestroyToolTipCtrl())
			pModuleThreadState->m_pToolTip = NULL;
	}

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!afxContextIsDLL)
	{
		// unhook windows hooks
		if (pThreadState->m_hHookOldMsgFilter != NULL)
		{
			::UnhookWindowsHookEx(pThreadState->m_hHookOldMsgFilter);
			pThreadState->m_hHookOldMsgFilter = NULL;
		}
		if (pThreadState->m_hHookOldCbtFilter != NULL)
		{
			::UnhookWindowsHookEx(pThreadState->m_hHookOldCbtFilter);
			pThreadState->m_hHookOldCbtFilter = NULL;
		}
	}
    // We used to suppress all exceptions here. But that's the wrong thing
    // to do. If this process crashes, we should allow Windows to crash
    // the process and invoke watson.
}

/////////////////////////////////////////////////////////////////////////////
