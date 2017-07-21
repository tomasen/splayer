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
// CCmdTarget construction/destruction

CCmdTarget::CCmdTarget()
{
	// capture module state where object was constructed
	m_pModuleState = AfxGetModuleState();
	ASSERT(m_pModuleState != NULL);

	// initialize state
#ifndef _AFX_NO_OLE_SUPPORT
	m_dwRef = 1;
	m_pOuterUnknown = NULL;
	m_xInnerUnknown = 0;
	m_xDispatch.m_vtbl = 0;
	m_bResultExpected = TRUE;
	m_xConnPtContainer.m_vtbl = 0;
#endif
}

CCmdTarget::~CCmdTarget()
{
#ifndef _AFX_NO_OLE_SUPPORT
	if (m_xDispatch.m_vtbl != 0)
		((COleDispatchImpl*)&m_xDispatch)->Disconnect();
	ASSERT(m_dwRef <= 1);
#endif
	m_pModuleState = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CCmdTarget windows message dispatching

AFX_STATIC BOOL AFXAPI _AfxDispatchCmdMsg(CCmdTarget* pTarget, UINT nID, int nCode,
	AFX_PMSG pfn, void* pExtra, UINT_PTR nSig, AFX_CMDHANDLERINFO* pHandlerInfo)
		// return TRUE to stop routing
{
	ENSURE_VALID(pTarget);
	UNUSED(nCode);   // unused in release builds

	union MessageMapFunctions mmf;
	mmf.pfn = pfn;
	BOOL bResult = TRUE; // default is ok

	if (pHandlerInfo != NULL)
	{
		// just fill in the information, don't do it
		pHandlerInfo->pTarget = pTarget;
		pHandlerInfo->pmf = mmf.pfn;
		return TRUE;
	}

	switch (nSig)
	{
	default:    // illegal
		ASSERT(FALSE);
		return 0;
		break;

	case AfxSigCmd_v:
		// normal command or control notification
		ASSERT(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		ASSERT(pExtra == NULL);
		(pTarget->*mmf.pfnCmd_v_v)();
		break;

	case AfxSigCmd_b:
		// normal command or control notification
		ASSERT(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		ASSERT(pExtra == NULL);
		bResult = (pTarget->*mmf.pfnCmd_b_v)();
		break;

	case AfxSigCmd_RANGE:
		// normal command or control notification in a range
		ASSERT(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		ASSERT(pExtra == NULL);
		(pTarget->*mmf.pfnCmd_v_u)(nID);
		break;

	case AfxSigCmd_EX:
		// extended command (passed ID, returns bContinue)
		ASSERT(pExtra == NULL);
		bResult = (pTarget->*mmf.pfnCmd_b_u)(nID);
		break;

	case AfxSigNotify_v:
		{
			AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pExtra;
			ENSURE(pNotify != NULL);
			ASSERT(pNotify->pResult != NULL);
			ASSERT(pNotify->pNMHDR != NULL);
			(pTarget->*mmf.pfnNotify_v_NMHDR_pl)(pNotify->pNMHDR, pNotify->pResult);
		}
		break;

	case AfxSigNotify_b:
		{
			AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pExtra;
			ENSURE(pNotify != NULL);
			ASSERT(pNotify->pResult != NULL);
			ASSERT(pNotify->pNMHDR != NULL);
			bResult = (pTarget->*mmf.pfnNotify_b_NMHDR_pl)(pNotify->pNMHDR, pNotify->pResult);
		}
		break;

	case AfxSigNotify_RANGE:
		{
			AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pExtra;
			ENSURE(pNotify != NULL);
			ASSERT(pNotify->pResult != NULL);
			ASSERT(pNotify->pNMHDR != NULL);
			(pTarget->*mmf.pfnNotify_v_u_NMHDR_pl)(nID, pNotify->pNMHDR,
				pNotify->pResult);
		}
		break;

	case AfxSigNotify_EX:
		{
			AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pExtra;
			ENSURE(pNotify != NULL);
			ASSERT(pNotify->pResult != NULL);
			ASSERT(pNotify->pNMHDR != NULL);
			bResult = (pTarget->*mmf.pfnNotify_b_u_NMHDR_pl)(nID, pNotify->pNMHDR,
				pNotify->pResult);
		}
		break;

	case AfxSigCmdUI:
		{
			// ON_UPDATE_COMMAND_UI or ON_UPDATE_COMMAND_UI_REFLECT case
			ASSERT(CN_UPDATE_COMMAND_UI == (UINT)-1);
			ASSERT(nCode == CN_UPDATE_COMMAND_UI || nCode == 0xFFFF);
			ENSURE_ARG(pExtra != NULL);
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			ASSERT(!pCmdUI->m_bContinueRouting);    // idle - not set
			(pTarget->*mmf.pfnCmdUI_v_C)(pCmdUI);
			bResult = !pCmdUI->m_bContinueRouting;
			pCmdUI->m_bContinueRouting = FALSE;     // go back to idle
		}
		break;

	case AfxSigCmdUI_RANGE:
		{
			// ON_UPDATE_COMMAND_UI case
			ASSERT(nCode == CN_UPDATE_COMMAND_UI);
			ENSURE_ARG(pExtra != NULL);
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			ASSERT(pCmdUI->m_nID == nID);           // sanity assert
			ASSERT(!pCmdUI->m_bContinueRouting);    // idle - not set
			(pTarget->*mmf.pfnCmdUI_v_C_u)(pCmdUI, nID);
			bResult = !pCmdUI->m_bContinueRouting;
			pCmdUI->m_bContinueRouting = FALSE;     // go back to idle
		}
		break;

	// general extensibility hooks
	case AfxSigCmd_v_pv:
		(pTarget->*mmf.pfnCmd_v_pv)(pExtra);
		break;
	case AfxSigCmd_b_pv:
		bResult = (pTarget->*mmf.pfnCmd_b_pv)(pExtra);
		break;
	/*
	case AfxSig_vv:
		// normal command or control notification
		ASSERT(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		ASSERT(pExtra == NULL);
		(pTarget->*mmf.pfn_COMMAND)();
		break;

	case AfxSig_bv:
		// normal command or control notification
		ASSERT(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		ASSERT(pExtra == NULL);
		bResult = (pTarget->*mmf.pfn_bCOMMAND)();
		break;

	case AfxSig_vw:
		// normal command or control notification in a range
		ASSERT(CN_COMMAND == 0);        // CN_COMMAND same as BN_CLICKED
		ASSERT(pExtra == NULL);
		(pTarget->*mmf.pfn_COMMAND_RANGE)(nID);
		break;

	case AfxSig_bw:
		// extended command (passed ID, returns bContinue)
		ASSERT(pExtra == NULL);
		bResult = (pTarget->*mmf.pfn_COMMAND_EX)(nID);
		break;

	case AfxSig_vNMHDRpl:
		{
			AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pExtra;
			ENSURE(pNotify != NULL);
			ASSERT(pNotify->pResult != NULL);
			ASSERT(pNotify->pNMHDR != NULL);
			(pTarget->*mmf.pfn_NOTIFY)(pNotify->pNMHDR, pNotify->pResult);
		}
		break;
	case AfxSig_bNMHDRpl:
		{
			AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pExtra;
			ENSURE(pNotify != NULL);
			ASSERT(pNotify->pResult != NULL);
			ASSERT(pNotify->pNMHDR != NULL);
			bResult = (pTarget->*mmf.pfn_bNOTIFY)(pNotify->pNMHDR, pNotify->pResult);
		}
		break;
	case AfxSig_vwNMHDRpl:
		{
			AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pExtra;
			ENSURE(pNotify != NULL);
			ASSERT(pNotify->pResult != NULL);
			ASSERT(pNotify->pNMHDR != NULL);
			(pTarget->*mmf.pfn_NOTIFY_RANGE)(nID, pNotify->pNMHDR,
				pNotify->pResult);
		}
		break;
	case AfxSig_bwNMHDRpl:
		{
			AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pExtra;
			ENSURE(pNotify != NULL);
			ASSERT(pNotify->pResult != NULL);
			ASSERT(pNotify->pNMHDR != NULL);
			bResult = (pTarget->*mmf.pfn_NOTIFY_EX)(nID, pNotify->pNMHDR,
				pNotify->pResult);
		}
		break;
	case AfxSig_cmdui:
		{
			// ON_UPDATE_COMMAND_UI or ON_UPDATE_COMMAND_UI_REFLECT case
			ASSERT(CN_UPDATE_COMMAND_UI == (UINT)-1);
			ASSERT(nCode == CN_UPDATE_COMMAND_UI || nCode == 0xFFFF);
			ENSURE_ARG(pExtra != NULL);
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			ASSERT(!pCmdUI->m_bContinueRouting);    // idle - not set
			(pTarget->*mmf.pfn_UPDATE_COMMAND_UI)(pCmdUI);
			bResult = !pCmdUI->m_bContinueRouting;
			pCmdUI->m_bContinueRouting = FALSE;     // go back to idle
		}
		break;

	case AfxSig_cmduiw:
		{
			// ON_UPDATE_COMMAND_UI case
			ASSERT(nCode == CN_UPDATE_COMMAND_UI);
			ENSURE_ARG(pExtra != NULL);
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			ASSERT(pCmdUI->m_nID == nID);           // sanity assert
			ASSERT(!pCmdUI->m_bContinueRouting);    // idle - not set
			(pTarget->*mmf.pfn_UPDATE_COMMAND_UI_RANGE)(pCmdUI, nID);
			bResult = !pCmdUI->m_bContinueRouting;
			pCmdUI->m_bContinueRouting = FALSE;     // go back to idle
		}
		break;

	// general extensibility hooks
	case AfxSig_vpv:
		(pTarget->*mmf.pfn_OTHER)(pExtra);
		break;
	case AfxSig_bpv:
		bResult = (pTarget->*mmf.pfn_OTHER_EX)(pExtra);
		break;
	*/

	}
	return bResult;
}

// compare two pointers to GUIDs -- TRUE if both pointers are NULL
// or both pointers point to same GUID; FALSE otherwise

#define IsEqualNULLGuid(pGuid1, pGuid2) \
	(((pGuid1) == NULL && (pGuid2) == NULL) || \
	 ((pGuid1) != NULL && (pGuid2) != NULL && \
		IsEqualGUID(*(pGuid1), *(pGuid2))))


BOOL CCmdTarget::OnCmdMsg(UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo)
{
#ifndef _AFX_NO_OCC_SUPPORT
	// OLE control events are a special case
	if (nCode == CN_EVENT)
	{
		ENSURE(afxOccManager != NULL);
		return afxOccManager->OnEvent(this, nID, (AFX_EVENT*)pExtra, pHandlerInfo);
	}
#endif // !_AFX_NO_OCC_SUPPORT

	// determine the message number and code (packed into nCode)
	const AFX_MSGMAP* pMessageMap;
	const AFX_MSGMAP_ENTRY* lpEntry;
	UINT nMsg = 0;

#ifndef _AFX_NO_DOCOBJECT_SUPPORT
	if (nCode == CN_OLECOMMAND)
	{
		BOOL bResult = FALSE;

		const AFX_OLECMDMAP* pOleCommandMap;
		const AFX_OLECMDMAP_ENTRY* pEntry;

		ENSURE_ARG(pExtra != NULL);
		COleCmdUI* pUI = (COleCmdUI*) pExtra;
		const GUID* pguidCmdGroup = pUI->m_pguidCmdGroup;

#ifdef _AFXDLL
		for (pOleCommandMap = GetCommandMap(); pOleCommandMap->pfnGetBaseMap != NULL && !bResult;
			pOleCommandMap = pOleCommandMap->pfnGetBaseMap())
#else
		for (pOleCommandMap = GetCommandMap(); pOleCommandMap != NULL && !bResult;
			pOleCommandMap = pOleCommandMap->pBaseMap)
#endif
		{
			for (pEntry = pOleCommandMap->lpEntries;
				pEntry->cmdID != 0 && pEntry->nID != 0 && !bResult;
				pEntry++)
			{
				if (nID == pEntry->cmdID &&
					IsEqualNULLGuid(pguidCmdGroup, pEntry->pguid))
				{
					pUI->m_nID = pEntry->nID;
					bResult = TRUE;
				}
			}
		}

		return bResult;
	}
#endif

	if (nCode != CN_UPDATE_COMMAND_UI)
	{
		nMsg = HIWORD(nCode);
		nCode = LOWORD(nCode);
	}

	// for backward compatibility HIWORD(nCode)==0 is WM_COMMAND
	if (nMsg == 0)
		nMsg = WM_COMMAND;

	// look through message map to see if it applies to us

	for (pMessageMap = GetMessageMap(); pMessageMap->pfnGetBaseMap != NULL;
	  pMessageMap = (*pMessageMap->pfnGetBaseMap)())
	{
		// Note: catches BEGIN_MESSAGE_MAP(CMyClass, CMyClass)!
		ASSERT(pMessageMap != (*pMessageMap->pfnGetBaseMap)());
		lpEntry = AfxFindMessageEntry(pMessageMap->lpEntries, nMsg, nCode, nID);
		if (lpEntry != NULL)
		{
			// found it
#ifdef _DEBUG
			if (nCode == CN_COMMAND)
				TRACE(traceCmdRouting, 1, "SENDING command id 0x%04X to %hs target.\n", nID,
					GetRuntimeClass()->m_lpszClassName);
			else if (nCode > CN_COMMAND)
				TRACE(traceCmdRouting, 1, "SENDING control notification %d from control id 0x%04X to %hs window.\n",
					nCode, nID, GetRuntimeClass()->m_lpszClassName);
#endif //_DEBUG
			return _AfxDispatchCmdMsg(this, nID, nCode,
				lpEntry->pfn, pExtra, lpEntry->nSig, pHandlerInfo);
		}
	}
	return FALSE;   // not handled
}

/////////////////////////////////////////////////////////////////////////////
// Hook to disable automation handlers

#ifndef _AFX_NO_OLE_SUPPORT

BOOL CCmdTarget::IsInvokeAllowed(DISPID)
{
	return TRUE;    // normally, invoke is always allowed
}

#endif // !_AFX_NO_OLE_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// Stubs for OLE type library functions

#ifndef _AFX_NO_OLE_SUPPORT

BOOL CCmdTarget::GetDispatchIID(IID*)
{
	// Subclass must implement (typically via COleControl implementation)
	return FALSE;
}

UINT CCmdTarget::GetTypeInfoCount()
{
	// Subclass must implement (typically via IMPLEMENT_OLETYPELIB macro)
	return 0;
}

CTypeLibCache* CCmdTarget::GetTypeLibCache()
{
	// Subclass must implement (typically via IMPLEMENT_OLETYPELIB macro)
	return NULL;
}

HRESULT CCmdTarget::GetTypeLib(LCID, LPTYPELIB*)
{
	// Subclass must implement (typically via IMPLEMENT_OLETYPELIB macro)
	return TYPE_E_CANTLOADLIBRARY;
}

#endif // !_AFX_NO_OLE_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// CCmdTarget routines that delegate to the WinApp

void CCmdTarget::BeginWaitCursor()
	{ AfxGetApp()->DoWaitCursor(1); }
void CCmdTarget::EndWaitCursor()
	{ AfxGetApp()->DoWaitCursor(-1); }
void CCmdTarget::RestoreWaitCursor()
	{ AfxGetApp()->DoWaitCursor(0); }

/////////////////////////////////////////////////////////////////////////////
// Root of message maps


const AFX_MSGMAP* CCmdTarget::GetMessageMap() const
{
	return GetThisMessageMap();
}

const AFX_MSGMAP* CCmdTarget::GetThisMessageMap()
{
	static const AFX_MSGMAP_ENTRY _messageEntries[] =
	{
		{ 0, 0, AfxSig_end, 0 }     // nothing here
	};
	static const AFX_MSGMAP messageMap =
	{
		NULL,
		&_messageEntries[0]
	};
	return &messageMap;	
}

/////////////////////////////////////////////////////////////////////////////
// Root of dispatch maps

#ifndef _AFX_NO_OLE_SUPPORT

UINT CCmdTarget::_dispatchEntryCount = (UINT)-1;
DWORD CCmdTarget::_dwStockPropMask = (DWORD)-1;

const AFX_DISPMAP CCmdTarget::dispatchMap =
{
	NULL,
	&CCmdTarget::_dispatchEntries[0],
	&CCmdTarget::_dispatchEntryCount,
	&CCmdTarget::_dwStockPropMask
};

const AFX_DISPMAP* CCmdTarget::GetDispatchMap() const
{
	return &CCmdTarget::dispatchMap;
}

#ifdef _AFXDLL
const AFX_DISPMAP* CCmdTarget::GetThisDispatchMap()
{
	return &CCmdTarget::dispatchMap;
}
#endif

const AFX_DISPMAP_ENTRY CCmdTarget::_dispatchEntries[] =
{
	{ NULL, -1, NULL, 0, (AFX_PMSG)NULL, (AFX_PMSG)NULL, (size_t)-1,
	  afxDispCustom }
	// nothing here
};

#endif //!_AFX_NO_OLE_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// Root of event sink maps

#ifndef _AFX_NO_OCC_SUPPORT

UINT CCmdTarget::_eventsinkEntryCount = (UINT)-1;

const AFX_EVENTSINKMAP CCmdTarget::eventsinkMap =
{
	NULL,
	&CCmdTarget::_eventsinkEntries[0],
	&CCmdTarget::_eventsinkEntryCount
};

const AFX_EVENTSINKMAP* CCmdTarget::GetEventSinkMap() const
{
	return &CCmdTarget::eventsinkMap;
}

#ifdef _AFXDLL
const AFX_EVENTSINKMAP* CCmdTarget::GetThisEventSinkMap()
{
	return &CCmdTarget::eventsinkMap;
}
#endif

const AFX_EVENTSINKMAP_ENTRY CCmdTarget::_eventsinkEntries[] =
{
	{ NULL, -1, NULL, 0, (AFX_PMSG)NULL, (AFX_PMSG)NULL, (size_t)-1,
	  afxDispCustom, (UINT)-1, (UINT)-1 }
	// nothing here
};

#endif //!_AFX_NO_OCC_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// Root of interface maps

#ifndef _AFX_NO_OLE_SUPPORT

const AFX_INTERFACEMAP* CCmdTarget::GetInterfaceMap() const
{
	return &CCmdTarget::interfaceMap;
}

#ifdef _AFXDLL
const AFX_INTERFACEMAP* CCmdTarget::GetThisInterfaceMap()
{
	return &CCmdTarget::interfaceMap;
}
#endif

const AFX_INTERFACEMAP CCmdTarget::interfaceMap =
{
	NULL,
	&CCmdTarget::_interfaceEntries[0]
};

const AFX_INTERFACEMAP_ENTRY CCmdTarget::_interfaceEntries[] =
{
#ifndef _AFX_NO_OLE_SUPPORT
	INTERFACE_PART(CCmdTarget, IID_IDispatch, Dispatch)
#endif
	{ NULL, (size_t)-1 }    // end of entries
};

void CCmdTarget::OnFinalRelease()
{
#ifndef _AFX_NO_OLE_SUPPORT
	AfxLockGlobals(CRIT_TYPELIBCACHE);

	// release the typelib cache, if any
	CTypeLibCache* pTypeLibCache = GetTypeLibCache();
	if (pTypeLibCache != NULL)
		pTypeLibCache->Unlock();

	AfxUnlockGlobals(CRIT_TYPELIBCACHE);
#endif

	delete this;
}

BOOL CCmdTarget::OnCreateAggregates()
{
	return TRUE;
}

LPUNKNOWN CCmdTarget::GetInterfaceHook(const void*)
{
	return NULL;
}

#endif //!_AFX_NO_OLE_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// Root of connection maps

#ifndef _AFX_NO_OLE_SUPPORT

const AFX_CONNECTIONMAP* CCmdTarget::GetConnectionMap() const
{
	return &CCmdTarget::connectionMap;
}

#ifdef _AFXDLL
const AFX_CONNECTIONMAP* CCmdTarget::GetThisConnectionMap()
{
	return &CCmdTarget::connectionMap;
}
#endif

const AFX_CONNECTIONMAP CCmdTarget::connectionMap =
{
	NULL,
	&CCmdTarget::_connectionEntries[0]
};

const AFX_CONNECTIONMAP_ENTRY CCmdTarget::_connectionEntries[] =
{
	{ NULL, (size_t)-1 }    // end of entries
};

LPCONNECTIONPOINT CCmdTarget::GetConnectionHook(const IID&)
{
	return NULL;
}

BOOL CCmdTarget::GetExtraConnectionPoints(CPtrArray*)
{
	return FALSE;
}

#endif //!_AFX_NO_OLE_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// Root of command target maps

#ifndef _AFX_NO_DOCOBJECT_SUPPORT

const AFX_OLECMDMAP CCmdTarget::commandMap =
{
	NULL,
	&CCmdTarget::_commandEntries[0]
};

const AFX_OLECMDMAP_ENTRY CCmdTarget::_commandEntries[] =
{
	{ NULL, 0, 0 }    // end of entries
};

const AFX_OLECMDMAP* CCmdTarget::GetCommandMap() const
{
	return &CCmdTarget::commandMap;
}

#ifdef _AFXDLL
const AFX_OLECMDMAP* CCmdTarget::GetThisCommandMap()
{
	return &CCmdTarget::commandMap;
}
#endif

#endif //!_AFX_NO_DOCOBJECT_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// Special access to view routing info

CView* CCmdTarget::GetRoutingView()
{
	return GetRoutingView_();
}

CFrameWnd* CCmdTarget::GetRoutingFrame()
{
	return GetRoutingFrame_();
}

/////////////////////////////////////////////////////////////////////////////
// CCmdUI - User Interface for a command

// CCmdUI is a protocol class for all command handler variants
//      CCmdUI is an implementation class for menus and general dialog
//        controls (usually buttons)

CCmdUI::CCmdUI()
{
	// zero out everything
	m_nID = m_nIndex = m_nIndexMax = 0;
	m_pMenu = m_pSubMenu = m_pParentMenu = NULL;
	m_pOther = NULL;
	m_bEnableChanged = m_bContinueRouting = FALSE;
}

// default CCmdUI implementation only works for Menu Items
void CCmdUI::Enable(BOOL bOn)
{
	if (m_pMenu != NULL)
	{
		if (m_pSubMenu != NULL)
			return; // don't change popup menus indirectly

		ENSURE(m_nIndex < m_nIndexMax);
		m_pMenu->EnableMenuItem(m_nIndex, MF_BYPOSITION |
			(bOn ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
	}
	else
	{
		// enable/disable a control (i.e. child window)
		ENSURE(m_pOther != NULL);

		// if control has the focus, move the focus before disabling
		if (!bOn && (::GetFocus() == m_pOther->m_hWnd))
			m_pOther->GetParent()->SendMessage(WM_NEXTDLGCTL, 0, FALSE);
		m_pOther->EnableWindow(bOn);
	}
	m_bEnableChanged = TRUE;
}

void CCmdUI::SetCheck(int nCheck)
{
	if (m_pMenu != NULL)
	{
		if (m_pSubMenu != NULL)
			return; // don't change popup menus indirectly

		// place checkmark next to menu item
		ENSURE(m_nIndex < m_nIndexMax);
		m_pMenu->CheckMenuItem(m_nIndex, MF_BYPOSITION |
			(nCheck ? MF_CHECKED : MF_UNCHECKED));
	}
	else
	{
		// we can only check buttons or controls acting like buttons
		ENSURE(m_pOther != NULL);
		if (m_pOther->SendMessage(WM_GETDLGCODE) & DLGC_BUTTON)
			m_pOther->SendMessage(BM_SETCHECK, nCheck);
		// otherwise ignore it
	}
}

AFX_STATIC void AFXAPI _AfxLoadDotBitmap(); // for swap tuning

void CCmdUI::SetRadio(BOOL bOn)
{
	SetCheck(bOn ? 1 : 0); // this default works for most things as well
	if (m_pMenu != NULL)
	{
		if (m_pSubMenu != NULL)
			return; // don't change popup menus indirectly

		// for menu item - use dot instead of checkmark
		ENSURE(m_nIndex < m_nIndexMax);

		if (afxData.hbmMenuDot == NULL)
			_AfxLoadDotBitmap();    // in INIT segment

		if (afxData.hbmMenuDot != NULL)
			SetMenuItemBitmaps(m_pMenu->m_hMenu, m_nIndex, MF_BYPOSITION,
				NULL, afxData.hbmMenuDot);
	}
}

void CCmdUI::SetText(LPCTSTR lpszText)
{
	ENSURE_ARG(lpszText != NULL);
	ASSERT(AfxIsValidString(lpszText));

	if (m_pMenu != NULL)
	{
		if (m_pSubMenu != NULL)
			return; // don't change popup menus indirectly

		// get current menu state so it doesn't change
		UINT nState = m_pMenu->GetMenuState(m_nIndex, MF_BYPOSITION);
		nState &= ~(MF_BITMAP|MF_OWNERDRAW|MF_SEPARATOR);

		// set menu text
		ENSURE(m_nIndex < m_nIndexMax);
		VERIFY(m_pMenu->ModifyMenu(m_nIndex, MF_BYPOSITION |
			MF_STRING | nState, m_nID, lpszText));
	}
	else
	{
		ENSURE(m_pOther != NULL);
		AfxSetWindowText(m_pOther->m_hWnd, lpszText);
	}
}

BOOL CCmdUI::DoUpdate(CCmdTarget* pTarget, BOOL bDisableIfNoHndler)
{
	if (m_nID == 0 || LOWORD(m_nID) == 0xFFFF)
		return TRUE;     // ignore invalid IDs

	ENSURE_VALID(pTarget);

	m_bEnableChanged = FALSE;
	BOOL bResult = pTarget->OnCmdMsg(m_nID, CN_UPDATE_COMMAND_UI, this, NULL);
	if (!bResult)
		ASSERT(!m_bEnableChanged); // not routed

	if (bDisableIfNoHndler && !m_bEnableChanged)
	{
		AFX_CMDHANDLERINFO info;
		info.pTarget = NULL;
		BOOL bHandler = pTarget->OnCmdMsg(m_nID, CN_COMMAND, this, &info);

#ifdef _DEBUG
		if (!bHandler)
			TRACE(traceCmdRouting, 1, "No handler for command ID 0x%04X, disabling it.\n", m_nID);
#endif
		// Enable or Disable based on whether there is a handler there
		Enable(bHandler);
	}
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// Special init


AFX_STATIC_DATA const BYTE _afxDot[] =
	{ 0x6, 0xF, 0xF, 0xF, 0x6 }; // simple byte bitmap, 1=> bit on
#define DOT_WIDTH   4
#define DOT_HEIGHT  5

AFX_STATIC void AFXAPI _AfxLoadDotBitmap()
{
	ASSERT(afxData.hbmMenuDot == NULL);
	// attempt to load special bitmap, else default to arrow
	CSize size = ::GetMenuCheckMarkDimensions();
	ENSURE(size.cx > 4 && size.cy > 5); // not too small please
	if (size.cx > 32)
		size.cx = 32;
	int iwRow = (size.cx + 15) >> 4;    // # of WORDs per raster line
	int nShift = (size.cx - DOT_WIDTH) / 2;     // # of bits to shift over
	nShift += ((iwRow * 16) - size.cx); // padding for word alignment
	if (nShift > 16 - DOT_WIDTH)
		nShift = 16 - DOT_WIDTH;    // maximum shift for 1 word

	if (size.cy > 32)
		size.cy = 32;

	// bitmap 2/4/4/4/2 pixels wide - centered (0 => black)
	BYTE rgbBitmap[32 * 2 * sizeof(WORD)];
	memset(rgbBitmap, 0xff, sizeof(rgbBitmap));

	BYTE* pbOut = &rgbBitmap[iwRow * sizeof(WORD) *
							((size.cy - (DOT_HEIGHT+1)) >> 1)];
	const BYTE* pbIn = _afxDot;
	for (int y = 0; y < DOT_HEIGHT; y++)
	{
		WORD w = (WORD)~(((DWORD)*pbIn++) << nShift);
		// bitmaps are always hi-lo
		pbOut[0] = HIBYTE(w);
		pbOut[1] = LOBYTE(w);
		pbOut += iwRow * sizeof(WORD);
	}

	afxData.hbmMenuDot = ::CreateBitmap(size.cx, size.cy, 1, 1,
			(LPVOID)rgbBitmap);
	if (afxData.hbmMenuDot == NULL)
	{
		TRACE(traceAppMsg, 0, "Warning: using system arrow bitmap instead of dot.\n");
		#define OBM_MNARROW         32739
		afxData.hbmMenuDot = ::LoadBitmap(NULL, MAKEINTRESOURCE(OBM_MNARROW));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCmdTarget diagnostics

#ifdef _DEBUG
void CCmdTarget::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

#ifndef _AFX_NO_OLE_SUPPORT
	if (m_xDispatch.m_vtbl != 0)
	{
		dc << "with IDispatch (OLE Automation) capability\n";
		dc << "m_bResultExpected = " << m_bResultExpected << "\n";
	}
	if (m_xConnPtContainer.m_vtbl != 0)
	{
		dc << "with OLE Connection Point capability\n";
	}
	if (GetInterfaceMap() != &CCmdTarget::interfaceMap)
	{
		dc << "with OLE capability";
		dc << "\nm_dwRef = " << m_dwRef;
		dc << "\nm_pOuterUnknown = " << m_pOuterUnknown;
		if (m_xInnerUnknown != 0)
			dc << "\nwith aggregation capability";
		dc << "\n";
	}
#endif //!_AFX_NO_OLE_SUPPORT
}

void CCmdTarget::AssertValid() const
{
	CObject::AssertValid();
}
#endif


IMPLEMENT_DYNAMIC(CCmdTarget, CObject)

/////////////////////////////////////////////////////////////////////////////
