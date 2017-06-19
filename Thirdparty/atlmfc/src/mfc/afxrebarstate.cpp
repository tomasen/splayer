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
#include "afxsettingsstore.h"
#include "afxrebar.h"
#include "afxrebarstate.h"

static const CString strRebarKeyFmt = _T("Rebar-%ld");
static const CString strRebarKey = _T("RBI");
static const CString strRebarId = _T("IDs");

BOOL CMFCReBarState::LoadRebarStateProc(HWND hwnd, LPARAM lParam)
{
	// determine if this is a rebar:
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	if (!pWnd->IsKindOf(RUNTIME_CLASS(CMFCReBar)))
	{
		return TRUE;
	}

	CReBarCtrl& rc = reinterpret_cast<CMFCReBar*>(pWnd)->GetReBarCtrl();
	const UINT nBandInfoSize = reinterpret_cast<CMFCReBar*>(pWnd)->GetReBarBandInfoSize ();

	// retrieve our registry section:
	CString strRegSection = reinterpret_cast<LPCTSTR>(lParam);

	CString strRebar;
	strRebar.Format(strRebarKeyFmt, GetWindowLong(rc.GetSafeHwnd(), GWL_ID));

	strRegSection += strRebar;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strRegSection))
	{
		return FALSE;
	}

	UINT nBands = 0;

	// attempt to load this rebar:

	REBARBANDINFO* aBandInfo = NULL;
	if (!reg.Read(strRebarKey, reinterpret_cast<BYTE**>(&aBandInfo), &nBands))
	{
		if (aBandInfo != NULL)
		{
			delete [] aBandInfo;
		}

		return TRUE;
	}

	LONG_PTR* aBandIds = NULL;
	if (!reg.Read(strRebarId, reinterpret_cast<BYTE**>(&aBandIds),  &nBands))
	{
		delete [] aBandInfo;

		if (aBandIds != NULL)
		{
			delete [] aBandIds;
		}

		return TRUE;
	}

	// band count should be identical
	nBands /= sizeof(LONG_PTR);

	if (nBands != rc.GetBandCount())
	{
		delete [] aBandInfo;
		delete [] aBandIds;
		return TRUE;
	}

	// reorder the bands:
	REBARBANDINFO rbi;
	for (int i = 0 ; i < (int)nBands ; i++)
	{
		// check all bands(in a release build the assert above won't fire if there's a mixup
		// and we'll happily do our best)
		for (int j = i; j < (int) rc.GetBandCount(); j++)
		{
			memset(&rbi, 0, nBandInfoSize);
			rbi.cbSize = nBandInfoSize;
			rbi.fMask = RBBIM_CHILD;
			rc.GetBandInfo(j, &rbi);
			if (aBandIds[i] != GetWindowLong(rbi.hwndChild, GWL_ID))
				continue;

			if (i != j)
				rc.MoveBand(j, i);

			// make sure that unpersistable information is not used when setting the band info
			aBandInfo[i].lpText = NULL;
			aBandInfo[i].cch = 0;
			aBandInfo[i].hwndChild = NULL;
			aBandInfo[i].hbmBack = NULL;
			aBandInfo[i].lParam = NULL;
			aBandInfo[i].fMask &= ~(RBBIM_TEXT | RBBIM_CHILD | RBBIM_BACKGROUND | RBBIM_LPARAM);

			rc.SetBandInfo(i, &aBandInfo[i]);
			break;
		}
	}

	delete [] aBandInfo;
	delete [] aBandIds;
	return TRUE;
}

BOOL CMFCReBarState::SaveRebarStateProc(HWND hwnd, LPARAM lParam)
{
	// determine if this is a rebar:
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	if (!pWnd->IsKindOf(RUNTIME_CLASS(CMFCReBar)))
	{
		return TRUE;
	}

	CReBarCtrl& rc = reinterpret_cast<CMFCReBar*>(pWnd)->GetReBarCtrl();
	const UINT nBandInfoSize = reinterpret_cast<CMFCReBar*>(pWnd)->GetReBarBandInfoSize ();

	// retrieve our registry section:
	CString strRegSection = reinterpret_cast<LPCTSTR>(lParam);

	CString strRebar;
	strRebar.Format(strRebarKeyFmt, GetWindowLong(rc.GetSafeHwnd(), GWL_ID));

	strRegSection += strRebar;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (!reg.CreateKey(strRegSection))
	{
		return FALSE;
	}

	UINT nBands = rc.GetBandCount();
	if (nBands == 0)
	{
		return TRUE;
	}

#pragma warning(disable : 6211)
	REBARBANDINFO* aBandInfo = new REBARBANDINFO[nBands];

	LONG_PTR* aBandIds  = new LONG_PTR[nBands];
	memset(aBandInfo, 0, nBands * nBandInfoSize);

	for (UINT i = 0; i < nBands; i++)
	{
		REBARBANDINFO& rbi = aBandInfo [i];
		rbi.cbSize = nBandInfoSize;
		rbi.fMask = RBBIM_CHILD | RBBIM_ID | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE | RBBIM_STYLE | RBBIM_HEADERSIZE;
		rc.GetBandInfo(i, &aBandInfo[i]);

		// apparently fixed size bands mis-report their cxMinChildSize:
		rbi.cxMinChild += rbi.fStyle & RBBS_FIXEDSIZE ? 4 : 0;

		aBandIds[i] = (LONG_PTR)GetWindowLong(rbi.hwndChild, GWL_ID);
		rbi.hwndChild = 0;
		rbi.fMask ^= RBBIM_CHILD;
	}

	reg.Write(strRebarKey, reinterpret_cast<BYTE*>(aBandInfo), nBands * sizeof(REBARBANDINFO));
	reg.Write(strRebarId, reinterpret_cast<BYTE*>(aBandIds),  nBands * sizeof(LONG_PTR));

	delete [] aBandIds;
	delete [] aBandInfo;

#pragma warning(default : 6211)
	return TRUE;
}

void __stdcall CMFCReBarState::LoadState(CString& strRegKey, CFrameWnd* pFrrame)
{
	ASSERT_VALID(pFrrame);
	EnumChildWindows(pFrrame->GetSafeHwnd(), LoadRebarStateProc, (LPARAM)(LPCTSTR)strRegKey);
}

void __stdcall CMFCReBarState::SaveState(CString& strRegKey, CFrameWnd* pFrrame)
{
	ASSERT_VALID(pFrrame);
	EnumChildWindows(pFrrame->GetSafeHwnd(), SaveRebarStateProc, (LPARAM)(LPCTSTR)strRegKey);
}



