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
#include "afxcontrolbarutil.h"
#include "afxusertoolsmanager.h"
#include "afxtoolbar.h"
#include "afxregpath.h"
#include "afxsettingsstore.h"

#include "afxribbonres.h"

static const CString strUserToolsProfile = _T("UserToolsManager");
static const CString strUserToolsEntry = _T("Tools");

CUserToolsManager* afxUserToolsManager = NULL;
extern CObList afxAllToolBars;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUserToolsManager::CUserToolsManager() :
	m_uiCmdToolsDummy(0), m_uiCmdFirst(0), m_uiCmdLast(0), m_pToolRTC(NULL), m_uiArgumentsMenuID(0), m_uiInitialDirMenuID(0)
{
	ENSURE(afxUserToolsManager == NULL);
	afxUserToolsManager = this;
}

CUserToolsManager::CUserToolsManager(const UINT uiCmdToolsDummy, const UINT uiCmdFirst, const UINT uiCmdLast, CRuntimeClass* pToolRTC, UINT uArgMenuID, UINT uInitDirMenuID) :
	m_uiCmdToolsDummy(uiCmdToolsDummy), m_uiCmdFirst(uiCmdFirst), m_uiCmdLast(uiCmdLast), m_pToolRTC(pToolRTC), m_uiArgumentsMenuID(uArgMenuID), m_uiInitialDirMenuID(uInitDirMenuID)
{
	ENSURE(afxUserToolsManager == NULL);
	afxUserToolsManager = this;

	ENSURE(m_pToolRTC != NULL);
	ENSURE(m_pToolRTC->IsDerivedFrom(RUNTIME_CLASS(CUserTool)));

	ENSURE(m_uiCmdFirst <= m_uiCmdLast);

	//---------------------
	// Load default filter:
	//---------------------
	ENSURE(m_strFilter.LoadString(IDS_AFXBARRES_CMD_FILTER));

	m_strDefExt = _T("exe");
}

CUserToolsManager::~CUserToolsManager()
{
	while (!m_lstUserTools.IsEmpty())
	{
		delete m_lstUserTools.RemoveHead();
	}

	afxUserToolsManager = NULL;
}

BOOL CUserToolsManager::LoadState(LPCTSTR lpszProfileName)
{
	CString strProfileName = ::AFXGetRegPath(strUserToolsProfile, lpszProfileName);

	while (!m_lstUserTools.IsEmpty())
	{
		delete m_lstUserTools.RemoveHead();
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (!reg.Open(strProfileName))
	{
		return FALSE;
	}

	if (!reg.Read(strUserToolsEntry, m_lstUserTools))
	{
		//---------------------------------------------------------
		// Tools objects may be corrupted, so, I don't delete them.
		// Memory leak is possible!
		//---------------------------------------------------------
		m_lstUserTools.RemoveAll();
		return FALSE;
	}

	return TRUE;
}

BOOL CUserToolsManager::SaveState(LPCTSTR lpszProfileName)
{
	CString strProfileName = ::AFXGetRegPath(strUserToolsProfile, lpszProfileName);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (!reg.CreateKey(strProfileName))
	{
		return FALSE;
	}

	return reg.Write(strUserToolsEntry, m_lstUserTools);
}

CUserTool* CUserToolsManager::CreateNewTool()
{
	if (m_pToolRTC == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	if (m_lstUserTools.GetCount() >= GetMaxTools())
	{
		TRACE(_T("Too many user-defined tools. The max. number is %d"), GetMaxTools());
		return NULL;
	}

	//-----------------------------------
	// Find a first available command id:
	//-----------------------------------
	UINT uiCmdId = 0;
	for (uiCmdId = m_uiCmdFirst; uiCmdId <= m_uiCmdLast; uiCmdId ++)
	{
		BOOL bIsCmdAvailable = TRUE;

		for (POSITION pos = m_lstUserTools.GetHeadPosition(); pos != NULL;)
		{
			CUserTool* pListTool = (CUserTool*) m_lstUserTools.GetNext(pos);
			ASSERT_VALID(pListTool);

			if (pListTool->GetCommandId() == uiCmdId)
			{
				bIsCmdAvailable = FALSE;
				break;
			}
		}

		if (bIsCmdAvailable)
		{
			break;
		}
	}

	if (uiCmdId > m_uiCmdLast)
	{
		return NULL;
	}

	CUserTool* pTool = (CUserTool*) m_pToolRTC->CreateObject();
	if (pTool == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	ASSERT_VALID(pTool);

	pTool->m_uiCmdId = uiCmdId;

	m_lstUserTools.AddTail(pTool);
	return pTool;
}

BOOL CUserToolsManager::RemoveTool(CUserTool* pTool)
{
	ASSERT_VALID(pTool);
	POSITION pos = m_lstUserTools.Find(pTool);

	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_lstUserTools.RemoveAt(pos);

	UINT uiCmdId = pTool->GetCommandId();
	delete pTool;

	//------------------------------------
	// Remove user tool from all toolbars:
	//------------------------------------
	for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
	{
		CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(posTlb);
		ENSURE(pToolBar != NULL);

		BOOL bToolIsFound = FALSE;

		int iIndex = -1;
		while ((iIndex = pToolBar->CommandToIndex(uiCmdId)) >= 0)
		{
			pToolBar->RemoveButton(iIndex);
			bToolIsFound = TRUE;
		}

		if (bToolIsFound)
		{
			pToolBar->AdjustLayout();
		}
	}

	return TRUE;
}

BOOL CUserToolsManager::MoveToolUp(CUserTool* pTool)
{
	ASSERT_VALID(pTool);

	POSITION pos = m_lstUserTools.Find(pTool);
	if (pos == NULL)
	{
		return FALSE;
	}

	POSITION posPrev = pos;
	m_lstUserTools.GetPrev(posPrev);
	if (posPrev == NULL)
	{
		return FALSE;
	}

	m_lstUserTools.RemoveAt(pos);
	m_lstUserTools.InsertBefore(posPrev, pTool);

	return TRUE;
}

BOOL CUserToolsManager::MoveToolDown(CUserTool* pTool)
{
	ASSERT_VALID(pTool);

	POSITION pos = m_lstUserTools.Find(pTool);
	if (pos == NULL)
	{
		return FALSE;
	}

	POSITION posNext = pos;
	m_lstUserTools.GetNext(posNext);
	if (posNext == NULL)
	{
		return FALSE;
	}

	m_lstUserTools.RemoveAt(pos);
	m_lstUserTools.InsertAfter(posNext, pTool);

	return TRUE;
}

BOOL CUserToolsManager::InvokeTool(UINT uiCmdId)
{
	CUserTool* pTool = FindTool(uiCmdId);
	if (pTool == NULL)
	{
		return FALSE;
	}

	return pTool->Invoke();
}

CUserTool* CUserToolsManager::FindTool(UINT uiCmdId) const
{
	if (uiCmdId < m_uiCmdFirst || uiCmdId > m_uiCmdLast)
	{
		return NULL;
	}

	for (POSITION pos = m_lstUserTools.GetHeadPosition(); pos != NULL;)
	{
		CUserTool* pListTool = (CUserTool*) m_lstUserTools.GetNext(pos);
		ASSERT_VALID(pListTool);

		if (pListTool->GetCommandId() == uiCmdId)
		{
			return pListTool;
		}
	}

	return NULL;
}


