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
#include "afxcmdusagecount.h"
#include "afxtoolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT CMFCCmdUsageCount::m_nStartCount = 0;
UINT CMFCCmdUsageCount::m_nMinUsagePercentage = 5;

// Construction/Destruction
CMFCCmdUsageCount::CMFCCmdUsageCount() : m_nTotalUsage(0)
{
}

CMFCCmdUsageCount::~CMFCCmdUsageCount()
{
}

void CMFCCmdUsageCount::Serialize(CArchive& ar)
{
	if (ar.IsLoading())
	{
		ar >> m_nTotalUsage;
	}
	else
	{
		ar << m_nTotalUsage;
	}

	m_CmdUsage.Serialize(ar);
}

void CMFCCmdUsageCount::AddCmd(UINT uiCmd)
{
	if (CMFCToolBar::IsCustomizeMode())
	{
		return;
	}

	if ((uiCmd == 0 || uiCmd == (UINT) -1) || // Ignore submenus and separators, CMFCToolBar::IsBasicCommand(uiCmd) || // basic commands and
		IsStandardCommand(uiCmd)) // standard commands
	{
		return;
	}

	UINT uiCount = 0;
	if (!m_CmdUsage.Lookup(uiCmd, uiCount))
	{
		uiCount = 0;
	}

	m_CmdUsage.SetAt(uiCmd, ++uiCount);
	m_nTotalUsage ++;
}

void CMFCCmdUsageCount::Reset()
{
	m_CmdUsage.RemoveAll();
	m_nTotalUsage = 0;
}

UINT CMFCCmdUsageCount::GetCount(UINT uiCmd) const
{
	UINT uiCount = 0;
	m_CmdUsage.Lookup(uiCmd, uiCount);

	return uiCount;
}

BOOL CMFCCmdUsageCount::IsFreqeuntlyUsedCmd(UINT uiCmd) const
{
	// I say, that the specific command is frequently used,
	// if the command usage percentage is more than 20%
	if (m_nTotalUsage == 0)
	{
		return FALSE;
	}

	UINT uiCount = GetCount(uiCmd);

	if (m_nMinUsagePercentage == 0)
	{
		return uiCount > 0;
	}
	else
	{
		UINT uiPercentage = uiCount * 100 / m_nTotalUsage;
		return uiPercentage > m_nMinUsagePercentage;
	}
}

BOOL CMFCCmdUsageCount::HasEnoughInformation() const
{
	return m_nTotalUsage >= m_nStartCount;
}

BOOL __stdcall CMFCCmdUsageCount::SetOptions(UINT nStartCount, UINT nMinUsagePercentage)
{
	if (nMinUsagePercentage >= 100)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_nStartCount = nStartCount;
	m_nMinUsagePercentage = nMinUsagePercentage;

	return TRUE;
}


