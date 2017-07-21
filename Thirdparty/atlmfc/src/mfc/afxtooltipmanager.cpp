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
#include "afxtooltipmanager.h"
#include "afxglobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef TTM_SETTITLE
#define TTM_SETTITLE(WM_USER + 32)
#endif

UINT AFX_WM_UPDATETOOLTIPS = ::RegisterWindowMessage(_T("AFX_WM_UPDATETOOLTIPS"));
CTooltipManager* afxTooltipManager = NULL;

BOOL __stdcall CTooltipManager::CreateToolTip(CToolTipCtrl*& pToolTip, CWnd* pWndParent, UINT nType)
{
	UINT nCurrType = AFX_TOOLTIP_TYPE_DEFAULT;
	int nIndex = -1;

	for (int i = 0; i < AFX_TOOLTIP_TYPES; i++)
	{
		if (nCurrType == nType)
		{
			nIndex = i;
			break;
		}

		nCurrType <<= 1;
	}

	if (nIndex == -1)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (pToolTip != NULL)
	{
		ASSERT_VALID(pToolTip);

		if (pToolTip->GetSafeHwnd() != NULL)
		{
			pToolTip->DestroyWindow();
		}

		delete pToolTip;
		pToolTip = NULL;
	}

	if (afxTooltipManager != NULL)
	{
		if (!afxTooltipManager->CreateToolTipObject(pToolTip, nIndex))
		{
			return FALSE;
		}
	}
	else
	{
		pToolTip = new CToolTipCtrl;
		ASSERT_VALID(pToolTip);
	}

	if (!pToolTip->Create(pWndParent, TTS_ALWAYSTIP | TTS_NOPREFIX))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pToolTip->Activate(TRUE);

	if (afxGlobalData.m_nMaxToolTipWidth != -1)
	{
		pToolTip->SetMaxTipWidth(afxGlobalData.m_nMaxToolTipWidth);
	}

	if (pWndParent->GetSafeHwnd() != NULL && afxTooltipManager != NULL && afxTooltipManager->m_lstOwners.Find(pWndParent->GetSafeHwnd()) == NULL)
	{
		afxTooltipManager->m_lstOwners.AddTail(pWndParent->GetSafeHwnd());
	}

	return TRUE;
}

void __stdcall CTooltipManager::DeleteToolTip(CToolTipCtrl*& pToolTip)
{
	if (pToolTip != NULL)
	{
		ASSERT_VALID(pToolTip);

		if (pToolTip->GetSafeHwnd() != NULL)
		{
			HWND hwndParent = pToolTip->GetParent()->GetSafeHwnd();

			if (afxTooltipManager != NULL && hwndParent != NULL)
			{
				POSITION pos = afxTooltipManager->m_lstOwners.Find(hwndParent);
				if (pos != NULL)
				{
					afxTooltipManager->m_lstOwners.RemoveAt(pos);
				}
			}

			pToolTip->DestroyWindow();
		}

		delete pToolTip;
		pToolTip = NULL;
	}
}

void __stdcall CTooltipManager::SetTooltipText(TOOLINFO* pTI, CToolTipCtrl* pToolTip, UINT nType, const CString strText, LPCTSTR lpszDescr)
{
	if (pToolTip == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pTI == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(pToolTip);

	int nIndex = -1;
	UINT nCurrType = AFX_TOOLTIP_TYPE_DEFAULT;

	for (int i = 0; i < AFX_TOOLTIP_TYPES; i++)
	{
		if (nCurrType == nType)
		{
			nIndex = i;
			break;
		}

		nCurrType <<= 1;
	}

	if (nIndex == -1)
	{
		ASSERT(FALSE);
		return;
	}

	CString strTipText = strText;
	CString strDescr = lpszDescr != NULL ? lpszDescr : _T("");

	if (afxTooltipManager != NULL && afxTooltipManager->m_Params [nIndex].m_bBalloonTooltip)
	{
		if (strDescr.IsEmpty())
		{
			pToolTip->SendMessage(TTM_SETTITLE, 1, (LPARAM)(LPCTSTR) strDescr);
		}
		else
		{
			pToolTip->SendMessage(TTM_SETTITLE, 1, (LPARAM)(LPCTSTR) strText);
			strTipText = strDescr;
		}
	}

	pTI->lpszText = (LPTSTR) ::calloc((strTipText.GetLength() + 1), sizeof(TCHAR));
	if (pTI->lpszText == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	lstrcpy(pTI->lpszText, strTipText);

	CMFCToolTipCtrl* pToolTipEx = DYNAMIC_DOWNCAST(CMFCToolTipCtrl, pToolTip);

	if (pToolTipEx != NULL)
	{
		pToolTipEx->SetDescription(strDescr);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTooltipManager::CTooltipManager()
{
	ENSURE(afxTooltipManager == NULL);
	afxTooltipManager = this;

	for (int i = 0; i < AFX_TOOLTIP_TYPES; i++)
	{
		m_pRTC [i] = NULL;
	}
}

CTooltipManager::~CTooltipManager()
{
	afxTooltipManager = NULL;
}

BOOL CTooltipManager::CreateToolTipObject(CToolTipCtrl*& pToolTip, UINT nType)
{
	if (nType < 0 || nType >= AFX_TOOLTIP_TYPES)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCToolTipInfo& params = m_Params [nType];
	CRuntimeClass* pRTC = m_pRTC [nType];

	if (pRTC == NULL)
	{
		pToolTip = new CToolTipCtrl;
	}
	else
	{
		pToolTip = DYNAMIC_DOWNCAST(CToolTipCtrl, pRTC->CreateObject());
	}

	if (pToolTip == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID(pToolTip);

	CMFCToolTipCtrl* pToolTipEx = DYNAMIC_DOWNCAST(CMFCToolTipCtrl, pToolTip);

	if (pToolTipEx != NULL)
	{
		pToolTipEx->SetParams(&params);
	}

	return TRUE;
}

void CTooltipManager::SetTooltipParams(UINT nTypes, CRuntimeClass* pRTC, CMFCToolTipInfo* pParams)
{
	if (pRTC == NULL || !pRTC->IsDerivedFrom(RUNTIME_CLASS(CMFCToolTipCtrl)))
	{
		if (pParams != NULL)
		{
			// Parameters can be used with CMFCToolTipCtrl class only!
			ASSERT(FALSE);
			pParams = NULL;
		}
	}

	CMFCToolTipInfo defaultParams;

	UINT nType = AFX_TOOLTIP_TYPE_DEFAULT;

	for (int i = 0; i < AFX_TOOLTIP_TYPES; i++)
	{
		if ((nType & nTypes) != 0)
		{
			if (pParams == NULL)
			{
				m_Params [i] = defaultParams;
			}
			else
			{
				m_Params [i] = *pParams;
			}

			m_pRTC [i] = pRTC;
		}

		nType <<= 1;
	}

	for (POSITION pos = m_lstOwners.GetHeadPosition(); pos != NULL;)
	{
		HWND hwndOwner = m_lstOwners.GetNext(pos);

		if (::IsWindow(hwndOwner))
		{
			::SendMessage(hwndOwner, AFX_WM_UPDATETOOLTIPS, (WPARAM) nTypes, 0);
		}
	}
}

void CTooltipManager::UpdateTooltips()
{
	for (POSITION pos = m_lstOwners.GetHeadPosition(); pos != NULL;)
	{
		HWND hwndOwner = m_lstOwners.GetNext(pos);

		if (::IsWindow(hwndOwner))
		{
			::SendMessage(hwndOwner, AFX_WM_UPDATETOOLTIPS, (WPARAM) AFX_TOOLTIP_TYPE_ALL, 0);
		}
	}
}


