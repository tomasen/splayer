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
#include "afxglobalutils.h"
#include "afxdockingmanager.h"
#include "afxpanecontainermanager.h"
#include "afxdockablepane.h"
#include "afxpaneframewnd.h"
#include "afxmultipaneframewnd.h"
#include "afxbasetabbedpane.h"

#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmdichildwndex.h"
#include "afxolecntrframewndex.h"
#include "afxlinkctrl.h"
#include "afxribbonres.h"

#pragma warning(disable : 4706)
#include "multimon.h"
#pragma warning(default : 4706)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGlobalUtils afxGlobalUtils;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGlobalUtils::CGlobalUtils()
{
	m_bDialogApp = FALSE;
	m_bIsDragging = FALSE;
}

CGlobalUtils::~CGlobalUtils()
{
}

BOOL CGlobalUtils::CheckAlignment(CPoint point, CBasePane* pBar, int nSensitivity, const CDockingManager* pDockManager,
	BOOL bOuterEdge, DWORD& dwAlignment, DWORD dwEnabledDockBars, LPCRECT lpRectBounds) const
{
	BOOL bSmartDocking = FALSE;
	CSmartDockingStandaloneGuide::SDMarkerPlace nHilitedSide = CSmartDockingStandaloneGuide::sdNONE;

	if (pDockManager == NULL && pBar != NULL)
	{
		pDockManager = afxGlobalUtils.GetDockingManager(pBar->GetParent());
	}

	if (pDockManager != NULL)
	{
		CSmartDockingManager* pSDManager = pDockManager->GetSmartDockingManagerPermanent();
		if (pSDManager != NULL && pSDManager->IsStarted())
		{
			bSmartDocking = TRUE;
			nHilitedSide = pSDManager->GetHighlightedGuideNo();
		}
	}

	CRect rectBounds;
	if (pBar != NULL)
	{
		pBar->GetWindowRect(rectBounds);
	}
	else if (lpRectBounds != NULL)
	{
		rectBounds = *lpRectBounds;
	}
	else
	{
		ASSERT(FALSE);
		return FALSE;
	}

	int nCaptionHeight = 0;
	int nTabAreaTopHeight = 0;
	int nTabAreaBottomHeight = 0;

	CDockablePane* pDockingBar = DYNAMIC_DOWNCAST(CDockablePane, pBar);

	if (pDockingBar != NULL)
	{
		nCaptionHeight = pDockingBar->GetCaptionHeight();

		CRect rectTabAreaTop;
		CRect rectTabAreaBottom;
		pDockingBar->GetTabArea(rectTabAreaTop, rectTabAreaBottom);
		nTabAreaTopHeight = rectTabAreaTop.Height();
		nTabAreaBottomHeight = rectTabAreaBottom.Height();
	}

	// build rect for top area
	if (bOuterEdge)
	{
		if (bSmartDocking)
		{
			switch(nHilitedSide)
			{
			case CSmartDockingStandaloneGuide::sdLEFT:
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
			case CSmartDockingStandaloneGuide::sdRIGHT:
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
			case CSmartDockingStandaloneGuide::sdTOP:
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
			case CSmartDockingStandaloneGuide::sdBOTTOM:
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
			}
		}
		else
		{
			CRect rectToCheck(rectBounds.left - nSensitivity, rectBounds.top - nSensitivity, rectBounds.right + nSensitivity, rectBounds.top);
			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_TOP)
			{
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
			}

			// build rect for left area
			rectToCheck.right = rectBounds.left;
			rectToCheck.bottom = rectBounds.bottom + nSensitivity;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_LEFT)
			{
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
			}

			// build rect for bottom area
			rectToCheck.left = rectBounds.left - nSensitivity;
			rectToCheck.top = rectBounds.bottom;
			rectToCheck.right = rectBounds.right + nSensitivity;
			rectToCheck.bottom = rectBounds.bottom + nSensitivity;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_BOTTOM)
			{
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
			}

			// build rect for right area
			rectToCheck.left = rectBounds.right;
			rectToCheck.top = rectBounds.top - nSensitivity;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_RIGHT)
			{
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
			}
		}
	}
	else
	{
		if (bSmartDocking)
		{
			switch(nHilitedSide)
			{
			case CSmartDockingStandaloneGuide::sdCLEFT:
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
			case CSmartDockingStandaloneGuide::sdCRIGHT:
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
			case CSmartDockingStandaloneGuide::sdCTOP:
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
			case CSmartDockingStandaloneGuide::sdCBOTTOM:
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
			}
		}
		else
		{
#ifdef __BOUNDS_FIX__
			CRect rectToCheck(rectBounds.left, rectBounds.top, rectBounds.right, rectBounds.top + nSensitivity + nCaptionHeight);
			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_TOP)
			{
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
			}

			// build rect for left area
			rectToCheck.right = rectBounds.left + nSensitivity;
			rectToCheck.bottom = rectBounds.bottom + nSensitivity;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_LEFT)
			{
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
			}

			// build rect for bottom area
			rectToCheck.left = rectBounds.left;
			rectToCheck.top = rectBounds.bottom - nSensitivity - nTabAreaBottomHeight;
			rectToCheck.right = rectBounds.right;
			rectToCheck.bottom = rectBounds.bottom;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_BOTTOM)
			{
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
			}

			// build rect for right area
			rectToCheck.left = rectBounds.right - nSensitivity;
			rectToCheck.top = rectBounds.top - nSensitivity;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_RIGHT)
			{
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
			}
#else

			// build rect for top area
			CRect rectToCheck(rectBounds.left - nSensitivity, rectBounds.top - nSensitivity, rectBounds.right + nSensitivity, rectBounds.top + nSensitivity + nCaptionHeight);
			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_TOP)
			{
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
			}

			// build rect for left area
			rectToCheck.right = rectBounds.left + nSensitivity;
			rectToCheck.bottom = rectBounds.bottom + nSensitivity;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_LEFT)
			{
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
			}

			// build rect for bottom area
			rectToCheck.left = rectBounds.left - nSensitivity;
			rectToCheck.top = rectBounds.bottom - nSensitivity - nTabAreaBottomHeight;
			rectToCheck.right = rectBounds.right + nSensitivity;
			rectToCheck.bottom = rectBounds.bottom + nSensitivity;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_BOTTOM)
			{
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
			}

			// build rect for right area
			rectToCheck.left = rectBounds.right - nSensitivity;
			rectToCheck.top = rectBounds.top - nSensitivity;

			if (rectToCheck.PtInRect(point) && dwEnabledDockBars & CBRS_ALIGN_RIGHT)
			{
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
			}
#endif
		}
	}

	return FALSE;
}

CDockingManager* CGlobalUtils::GetDockingManager(CWnd* pWnd)
{
	if (pWnd == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pWnd);

	if (pWnd->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		return((CFrameWndEx*) pWnd)->GetDockingManager();
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		return((CMDIFrameWndEx*) pWnd)->GetDockingManager();
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		return((COleIPFrameWndEx*) pWnd)->GetDockingManager();
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		return((COleDocIPFrameWndEx*) pWnd)->GetDockingManager();
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		return((CMDIChildWndEx*) pWnd)->GetDockingManager();
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(CDialog)) || pWnd->IsKindOf(RUNTIME_CLASS(CPropertySheet)))
	{
		m_bDialogApp = TRUE;
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		return((COleCntrFrameWndEx*) pWnd)->GetDockingManager();
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
	{
		CPaneFrameWnd* pMiniFrameWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWnd);
		ASSERT_VALID(pMiniFrameWnd);

		CDockingManager* pManager = pMiniFrameWnd->GetDockingManager();
		return pManager != NULL ? pManager : GetDockingManager(pWnd->GetParent());
	}

	return NULL;
}

void CGlobalUtils::FlipRect(CRect& rect, int nDegrees)
{
	CRect rectTmp = rect;
	switch(nDegrees)
	{
	case 90:
		rect.top = rectTmp.left;
		rect.right = rectTmp.top;
		rect.bottom = rectTmp.right;
		rect.left = rectTmp.bottom;
		break;
	case 180:
		rect.top = rectTmp.bottom;
		rect.bottom = rectTmp.top;
		break;
	case 270:
	case -90:
		rect.left = rectTmp.top;
		rect.top = rectTmp.right;
		rect.right = rectTmp.bottom;
		rect.bottom = rectTmp.left;
		break;
	}
}

DWORD CGlobalUtils::GetOppositeAlignment(DWORD dwAlign)
{
	switch(dwAlign & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_LEFT:
		return CBRS_ALIGN_RIGHT;
	case CBRS_ALIGN_RIGHT:
		return CBRS_ALIGN_LEFT;
	case CBRS_ALIGN_TOP:
		return CBRS_ALIGN_BOTTOM;
	case CBRS_ALIGN_BOTTOM:
		return CBRS_ALIGN_TOP;
	}
	return 0;
}

void CGlobalUtils::SetNewParent(CObList& lstControlBars, CWnd* pNewParent, BOOL bCheckVisibility)
{
	ASSERT_VALID(pNewParent);
	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*) lstControlBars.GetNext(pos);

		if (bCheckVisibility && !pBar->IsPaneVisible())
		{
			continue;
		}
		if (!pBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
		{
			pBar->ShowWindow(SW_HIDE);
			pBar->SetParent(pNewParent);
			CRect rectWnd;
			pBar->GetWindowRect(rectWnd);
			pNewParent->ScreenToClient(rectWnd);

			pBar->SetWindowPos(NULL, -rectWnd.Width(), -rectWnd.Height(), 100, 100, SWP_NOZORDER | SWP_NOSIZE  | SWP_NOACTIVATE);
			pBar->ShowWindow(SW_SHOW);
		}
		else
		{
			pBar->SetParent(pNewParent);
		}
	}
}

void CGlobalUtils::CalcExpectedDockedRect(CPaneContainerManager& barContainerManager,
	CWnd* pWndToDock, CPoint ptMouse, CRect& rectResult, BOOL& bDrawTab, CDockablePane** ppTargetBar)
{
	ENSURE(ppTargetBar != NULL);

	DWORD dwAlignment = CBRS_ALIGN_LEFT;
	BOOL bTabArea = FALSE;
	BOOL bCaption = FALSE;
	bDrawTab = FALSE;
	*ppTargetBar = NULL;

	rectResult.SetRectEmpty();

	if (GetKeyState(VK_CONTROL) < 0)
	{
		return;
	}

	if (!GetPaneAndAlignFromPoint(barContainerManager, ptMouse, ppTargetBar, dwAlignment, bTabArea, bCaption) || *ppTargetBar == NULL)
	{
		return;
	}

	CPane* pBar = NULL;

	if (pWndToDock->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
	{
		CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWndToDock);
		ASSERT_VALID(pMiniFrame);
		pBar = DYNAMIC_DOWNCAST(CPane, pMiniFrame->GetFirstVisiblePane());
	}
	else
	{
		pBar = DYNAMIC_DOWNCAST(CPane, pWndToDock);
	}

	if (*ppTargetBar != NULL)
	{
		DWORD dwTargetEnabledAlign = (*ppTargetBar)->GetEnabledAlignment();
		DWORD dwTargetCurrentAlign = (*ppTargetBar)->GetCurrentAlignment();
		BOOL bTargetBarIsFloating = ((*ppTargetBar)->GetParentMiniFrame() != NULL);

		if (pBar != NULL)
		{
			if (pBar->GetEnabledAlignment() != dwTargetEnabledAlign && bTargetBarIsFloating ||
				(pBar->GetEnabledAlignment() & dwTargetCurrentAlign) == 0 && !bTargetBarIsFloating)
			{
				return;
			}
		}
	}

	if (bTabArea || bCaption)
	{
		// can't make tab on miniframe
		bDrawTab = ((*ppTargetBar) != NULL);

		if (bDrawTab)
		{
			bDrawTab = (*ppTargetBar)->CanBeAttached() && CanBeAttached(pWndToDock) &&
				pBar != NULL &&((*ppTargetBar)->GetEnabledAlignment() == pBar->GetEnabledAlignment());
		}

		if (!bDrawTab)
		{
			return;
		}
	}

	if ((*ppTargetBar) != NULL &&(*ppTargetBar)->GetParentMiniFrame() != NULL && !CanPaneBeInFloatingMultiPaneFrameWnd(pWndToDock))
	{
		bDrawTab = FALSE;
		return;
	}

	if ((*ppTargetBar) != NULL && pWndToDock->IsKindOf(RUNTIME_CLASS(CBasePane)) && !(*ppTargetBar)->CanAcceptPane((CBasePane*) pWndToDock))
	{
		bDrawTab = FALSE;
		return;
	}

	CRect rectOriginal;
	(*ppTargetBar)->GetWindowRect(rectOriginal);

	if ((*ppTargetBar) == pWndToDock || pWndToDock->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)) &&(*ppTargetBar)->GetParentMiniFrame() == pWndToDock)
	{
		bDrawTab = FALSE;
		return;
	}

	CRect rectInserted;
	CRect rectSlider;
	DWORD dwSliderStyle;
	CSize sizeMinOriginal(0, 0);
	CSize sizeMinInserted(0, 0);

	pWndToDock->GetWindowRect(rectInserted);

	if (pBar == NULL)
	{
		return;
	}

	if ((dwAlignment & pBar->GetEnabledAlignment()) != 0 ||
		CDockingManager::m_bIgnoreEnabledAlignment)
	{
		barContainerManager.CalcRects(rectOriginal, rectInserted, rectSlider, dwSliderStyle, dwAlignment, sizeMinOriginal, sizeMinInserted);
		rectResult = rectInserted;
	}
}

BOOL CGlobalUtils::CanBeAttached(CWnd* pWnd) const
{
	ASSERT_VALID(pWnd);

	if (pWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
	{
		return((CPaneFrameWnd*) pWnd)->CanBeAttached();
	}

	if (pWnd->IsKindOf(RUNTIME_CLASS(CPane)))
	{
		return((CPane*) pWnd)->CanBeAttached();
	}

	return FALSE;
}

BOOL CGlobalUtils::CanPaneBeInFloatingMultiPaneFrameWnd(CWnd* pWnd) const
{
	CPane* pBar = NULL;

	CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWnd);

	if (pMiniFrame != NULL)
	{
		pBar = DYNAMIC_DOWNCAST(CPane, pMiniFrame->GetPane());
	}
	else
	{
		pBar = DYNAMIC_DOWNCAST(CPane, pWnd);
	}

	if (pBar == NULL)
	{
		return FALSE;
	}

	if (pBar->IsTabbed())
	{
		CWnd* pParentMiniFrame = pBar->GetParentMiniFrame();
		// tabbed bar that is floating in multi miniframe
		if (pParentMiniFrame != NULL && pParentMiniFrame->IsKindOf(RUNTIME_CLASS(CMultiPaneFrameWnd)))
		{
			return TRUE;
		}
	}

	return((pBar->GetPaneStyle() & CBRS_FLOAT_MULTI) != 0);
}

BOOL CGlobalUtils::GetPaneAndAlignFromPoint(CPaneContainerManager& barContainerManager, CPoint pt,
	CDockablePane** ppTargetControlBar, DWORD& dwAlignment, BOOL& bTabArea, BOOL& bCaption)
{
	ENSURE(ppTargetControlBar != NULL);
	*ppTargetControlBar = NULL;

	BOOL bOuterEdge = FALSE;

	// if the mouse is over a miniframe's caption and this miniframe has only one
	// visible docking control bar, we need to draw a tab
	bCaption = barContainerManager.CheckForMiniFrameAndCaption(pt, ppTargetControlBar);
	if (bCaption)
	{
		return TRUE;
	}

	*ppTargetControlBar = barContainerManager.PaneFromPoint(pt, CDockingManager::m_nDockSensitivity, TRUE, bTabArea, bCaption);

	if ((bCaption || bTabArea) && *ppTargetControlBar != NULL)
	{
		return TRUE;
	}

	if (*ppTargetControlBar == NULL)
	{
		barContainerManager.PaneFromPoint(pt, CDockingManager::m_nDockSensitivity, FALSE, bTabArea, bCaption);
		// the exact bar was not found - it means the docked frame at the outer edge
		bOuterEdge = TRUE;
		return TRUE;
	}

	if (*ppTargetControlBar != NULL)
	{
		if (!afxGlobalUtils.CheckAlignment(pt, *ppTargetControlBar, CDockingManager::m_nDockSensitivity, NULL, bOuterEdge, dwAlignment))
		{
			// unable for some reason to determine alignment
			*ppTargetControlBar = NULL;
		}
	}

	return TRUE;
}

void CGlobalUtils::AdjustRectToWorkArea(CRect& rect, CRect* pRectDelta)
{
	CPoint ptStart;

	if (m_bIsDragging)
	{
		::GetCursorPos(&ptStart);
	}
	else
	{
		ptStart = rect.TopLeft();
	}

	CRect rectScreen;
	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromPoint(ptStart, MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	int nDelta = pRectDelta != NULL ? pRectDelta->left : 0;

	if (rect.right <= rectScreen.left + nDelta)
	{
		rect.OffsetRect(rectScreen.left - rect.right + nDelta, 0);
	}

	nDelta = pRectDelta != NULL ? pRectDelta->right : 0;
	if (rect.left >= rectScreen.right - nDelta)
	{
		rect.OffsetRect(rectScreen.right - rect.left - nDelta, 0);
	}

	nDelta = pRectDelta != NULL ? pRectDelta->bottom : 0;
	if (rect.top >= rectScreen.bottom - nDelta)
	{
		rect.OffsetRect(0, rectScreen.bottom - rect.top - nDelta);
	}

	nDelta = pRectDelta != NULL ? pRectDelta->top : 0;
	if (rect.bottom < rectScreen.top + nDelta)
	{
		rect.OffsetRect(0, rectScreen.top - rect.bottom + nDelta);
	}
}

void CGlobalUtils::ForceAdjustLayout(CDockingManager* pDockManager, BOOL bForce, BOOL bForceInvisible)
{
	if (pDockManager != NULL &&(CPane::m_bHandleMinSize || bForce))
	{
		CWnd* pDockSite = pDockManager->GetDockSiteFrameWnd();

		if (pDockSite == NULL)
		{
			return;
		}

		if (!pDockSite->IsWindowVisible() && !bForceInvisible)
		{
			return;
		}

		CRect rectWnd;
		pDockManager->GetDockSiteFrameWnd()->SetRedraw(FALSE);
		pDockManager->GetDockSiteFrameWnd()->GetWindowRect(rectWnd);
		pDockManager->GetDockSiteFrameWnd()->SetWindowPos(NULL, -1, -1, rectWnd.Width() + 1, rectWnd.Height() + 1, SWP_NOZORDER |  SWP_NOMOVE | SWP_NOACTIVATE);
		pDockManager->GetDockSiteFrameWnd()->SetWindowPos(NULL, -1, -1, rectWnd.Width(), rectWnd.Height(), SWP_NOZORDER |  SWP_NOMOVE  | SWP_NOACTIVATE);
		pDockManager->GetDockSiteFrameWnd()->SetRedraw(TRUE);
		pDockManager->GetDockSiteFrameWnd()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
}

BOOL CGlobalUtils::StringFromCy(CString& str, CY& cy)
{
	VARIANTARG varCy;
	VARIANTARG varBstr;
	AfxVariantInit(&varCy);
	AfxVariantInit(&varBstr);
	V_VT(&varCy) = VT_CY;
	V_CY(&varCy) = cy;
	if (FAILED(VariantChangeType(&varBstr, &varCy, 0, VT_BSTR)))
	{
		VariantClear(&varCy);
		VariantClear(&varBstr);
		return FALSE;
	}
	str = V_BSTR(&varBstr);
	VariantClear(&varCy);
	VariantClear(&varBstr);
	return TRUE;
}

BOOL CGlobalUtils::CyFromString(CY& cy, LPCTSTR psz)
{
	USES_CONVERSION;

	if (psz == NULL || _tcslen(psz) == 0)
	{
		psz = _T("0");
	}

	VARIANTARG varBstr;
	VARIANTARG varCy;
	AfxVariantInit(&varBstr);
	AfxVariantInit(&varCy);
	V_VT(&varBstr) = VT_BSTR;
	V_BSTR(&varBstr) = SysAllocString(T2COLE(psz));
	if (FAILED(VariantChangeType(&varCy, &varBstr, 0, VT_CY)))
	{
		VariantClear(&varBstr);
		VariantClear(&varCy);
		return FALSE;
	}
	cy = V_CY(&varCy);
	VariantClear(&varBstr);
	VariantClear(&varCy);
	return TRUE;
}

BOOL CGlobalUtils::StringFromDecimal(CString& str, DECIMAL& decimal)
{
	VARIANTARG varDecimal;
	VARIANTARG varBstr;
	AfxVariantInit(&varDecimal);
	AfxVariantInit(&varBstr);
	V_VT(&varDecimal) = VT_DECIMAL;
	V_DECIMAL(&varDecimal) = decimal;
	if (FAILED(VariantChangeType(&varBstr, &varDecimal, 0, VT_BSTR)))
	{
		VariantClear(&varDecimal);
		VariantClear(&varBstr);
		return FALSE;
	}
	str = V_BSTR(&varBstr);
	VariantClear(&varDecimal);
	VariantClear(&varBstr);
	return TRUE;
}

BOOL CGlobalUtils::DecimalFromString(DECIMAL& decimal, LPCTSTR psz)
{
	USES_CONVERSION;

	if (psz == NULL || _tcslen(psz) == 0)
	{
		psz = _T("0");
	}

	VARIANTARG varBstr;
	VARIANTARG varDecimal;
	AfxVariantInit(&varBstr);
	AfxVariantInit(&varDecimal);
	V_VT(&varBstr) = VT_BSTR;
	V_BSTR(&varBstr) = SysAllocString(T2COLE(psz));
	if (FAILED(VariantChangeType(&varDecimal, &varBstr, 0, VT_DECIMAL)))
	{
		VariantClear(&varBstr);
		VariantClear(&varDecimal);
		return FALSE;
	}
	decimal = V_DECIMAL(&varDecimal);
	VariantClear(&varBstr);
	VariantClear(&varDecimal);
	return TRUE;
}

HICON CGlobalUtils::GetWndIcon(CWnd* pWnd)
{
	ASSERT_VALID(pWnd);

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return NULL;
	}

	HICON hIcon = pWnd->GetIcon(FALSE);

	if (hIcon == NULL)
	{
		hIcon = pWnd->GetIcon(TRUE);

		if (hIcon != NULL)
		{
			CImageList il;
			il.Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
			il.Add(hIcon);

			if (il.GetImageCount() == 1)
			{
				hIcon = il.ExtractIcon(0);
			}
		}
	}

	if (hIcon == NULL)
	{
		hIcon = (HICON)(LONG_PTR)::GetClassLongPtr(pWnd->GetSafeHwnd(), GCLP_HICONSM);
	}

	if (hIcon == NULL)
	{
		hIcon = (HICON)(LONG_PTR)::GetClassLongPtr(pWnd->GetSafeHwnd(), GCLP_HICON);
	}

	return hIcon;
}



