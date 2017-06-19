// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.
//
#include "stdafx.h"
#include "afxcontrolbarutil.h"
#include "afxtrackmouse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

VOID CALLBACK AFXTrackMouseTimerProc(HWND hWnd, UINT /*uMsg*/, UINT idEvent, DWORD /*dwTime*/)
{
	RECT rect;
	POINT pt;

	::GetClientRect(hWnd, &rect);
	::MapWindowPoints(hWnd, NULL, (LPPOINT)&rect, 2);

	::GetCursorPos(&pt);
	if (!::PtInRect(&rect, pt) ||(WindowFromPoint(pt) != hWnd))
	{
		if (!::KillTimer(hWnd, idEvent))
		{
			// Error killing the timer!
		}

		::PostMessage(hWnd,WM_MOUSELEAVE, 0, 0);
	}
}

BOOL AFXTrackMouse(LPTRACKMOUSEEVENT ptme)
{
	if (ptme == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (ptme->cbSize < sizeof(TRACKMOUSEEVENT))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!::IsWindow(ptme->hwndTrack))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!(ptme->dwFlags & TME_LEAVE))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return(BOOL) ::SetTimer(ptme->hwndTrack, ptme->dwFlags, 100, (TIMERPROC)AFXTrackMouseTimerProc);
}

