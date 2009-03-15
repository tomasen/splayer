/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// PlayerToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include <math.h>
#include <atlbase.h>
#include <afxpriv.h>
#include "PlayerToolBar.h"
#include "MainFrm.h"

typedef HRESULT (__stdcall * SetWindowThemeFunct)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

// CPlayerToolBar

IMPLEMENT_DYNAMIC(CPlayerToolBar, CToolBar)
CPlayerToolBar::CPlayerToolBar()
{
}

CPlayerToolBar::~CPlayerToolBar()
{
}

BOOL CPlayerToolBar::Create(CWnd* pParentWnd)
{
	if(!__super::CreateEx(pParentWnd,
		TBSTYLE_FLAT|TBSTYLE_TRANSPARENT|TBSTYLE_AUTOSIZE,
		WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM|CBRS_TOOLTIPS, CRect(2,2,0,3)) 
	|| !LoadToolBar(IDB_PLAYERTOOLBAR))
		return FALSE;

	GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	CToolBarCtrl& tb = GetToolBarCtrl();
	tb.DeleteButton(tb.GetButtonCount()-1);
	tb.DeleteButton(tb.GetButtonCount()-1);

	SetMute(AfxGetAppSettings().fMute);

	UINT styles[] = 
	{
		TBBS_CHECKGROUP, TBBS_CHECKGROUP, TBBS_CHECKGROUP, 
		TBBS_SEPARATOR,
		TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON, 
		//TBBS_SEPARATOR,
		//TBBS_BUTTON/*|TBSTYLE_DROPDOWN*/, 
		TBBS_SEPARATOR,
		TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON, 
		TBBS_SEPARATOR,
		TBBS_BUTTON, TBBS_BUTTON, TBBS_SEPARATOR, 
		TBBS_BUTTON,
		TBBS_BUTTON,
		TBBS_DISABLED,TBBS_DISABLED,
		TBBS_CHECKBOX, 
		/*TBBS_SEPARATOR,*/
	};

	for(int i = 0; i < countof(styles); i++)
		SetButtonStyle(i, styles[i]|TBBS_DISABLED);

	/*
	SetButtonStyle(0, GetButtonStyle(0)|BS_ICON );
		CWnd* hWndBtn = GetToolBarCtrl().GetDlgItem( ID_PLAY_PLAY );
		if(hWndBtn){
			HICON hIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance,  MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, 32, 32, LR_SHARED);
			if(hIcon){
				::SendMessage( hWndBtn->m_hWnd,  BM_SETIMAGE,      IMAGE_ICON , (LPARAM)(hIcon) );  
				DestroyIcon(hIcon);
			}else{
				AfxMessageBox(_T("f"));
			}
		}else{
			AfxMessageBox(_T("s"));
		}*/
	/*
	CWinApp* aa = AfxGetApp();
		m_ToolBarImages.Create(32, 32, ILC_COLOR32, 4, 4);
		m_ToolBarImages.Add(aa->LoadIcon(IDI_PLAY));
	
		m_ToolBarDisabledImages.Create(32, 32, ILC_COLOR32, 4, 4);
		m_ToolBarDisabledImages.Add(aa->LoadIcon(IDI_PLAY));
	
		tb.SetImageList(&m_ToolBarImages);
		tb.SetDisabledImageList(&m_ToolBarDisabledImages);
	*/
	

	

	m_volctrl.Create(this);
	
	if(AfxGetAppSettings().fDisabeXPToolbars)
	{
		if(HMODULE h = LoadLibrary(_T("uxtheme.dll")))
		{
			SetWindowThemeFunct f = (SetWindowThemeFunct)GetProcAddress(h, "SetWindowTheme");
			if(f) f(m_hWnd, L" ", L" ");
			FreeLibrary(h);
		}
	}

	return TRUE;
}

BOOL CPlayerToolBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!__super::PreCreateWindow(cs))
		return FALSE;

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;
//	m_dwStyle |= CBRS_SIZE_FIXED;

	return TRUE;
}

void CPlayerToolBar::ArrangeControls()
{
	if(!::IsWindow(m_volctrl.m_hWnd)) return;

	CRect r;
	GetClientRect(&r);

	CRect br = GetBorders();

	CRect r10;
	GetItemRect(18, &r10);

	CRect vr;
	m_volctrl.GetClientRect(&vr);
	CRect vr2(r.right+br.right-60, r.top-1, r.right+br.right+6, r.bottom);
	m_volctrl.MoveWindow(vr2);

	UINT nID;
	UINT nStyle;
	int iImage;
	GetButtonInfo(20, nID, nStyle, iImage);
	SetButtonInfo(19, GetItemID(19), TBBS_SEPARATOR, vr2.left - iImage - r10.right - 19);
}

void CPlayerToolBar::SetMute(bool fMute)
{
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	bi.iImage = fMute?21:20;
	tb.SetButtonInfo(ID_VOLUME_MUTE, &bi);

	AfxGetAppSettings().fMute = fMute;
}

bool CPlayerToolBar::IsMuted()
{
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	tb.GetButtonInfo(ID_VOLUME_MUTE, &bi);
	return(bi.iImage==21);
}

int CPlayerToolBar::GetVolume()
{
	int volume = m_volctrl.GetPos();
	volume = (int)((log10(1.0*volume)-2)*5000);
	volume = max(min(volume, 0), -10000);
	return(IsMuted() ? -10000 : volume);
}

void CPlayerToolBar::SetVolume(int volume)
{
/*
	volume = (int)pow(10, ((double)volume)/5000+2);
	volume = max(min(volume, 100), 1);
*/
	m_volctrl.SetPosInternal(volume);
}

BEGIN_MESSAGE_MAP(CPlayerToolBar, CToolBar)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
	ON_COMMAND_EX(ID_VOLUME_MUTE, OnVolumeMute)
	ON_UPDATE_COMMAND_UI(ID_VOLUME_MUTE, OnUpdateVolumeMute)
	ON_COMMAND_EX(ID_VOLUME_UP, OnVolumeUp)
	ON_COMMAND_EX(ID_VOLUME_DOWN, OnVolumeDown)
	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CPlayerToolBar message handlers

void CPlayerToolBar::OnPaint()
{
	if(m_bDelayedButtonLayout)
		Layout();

	CPaintDC dc(this); // device context for painting

	DefWindowProc(WM_PAINT, WPARAM(dc.m_hDC), 0);

	{
		UINT nID;
		UINT nStyle = 0;
		int iImage = 0;
		GetButtonInfo(19, nID, nStyle, iImage);
		CRect ItemRect;
		GetItemRect(19, ItemRect);
		dc.FillSolidRect(ItemRect, GetSysColor(COLOR_BTNFACE));
		//dc.FillSolidRect(ItemRect, RGB(214,219,239) );   
	}
}
void CPlayerToolBar::OnNcPaint() // when using XP styles the NC area isn't drawn for our toolbar...
{
	CRect wr, cr;

	CWindowDC dc(this);
	GetClientRect(&cr);
	ClientToScreen(&cr);
	GetWindowRect(&wr);
	cr.OffsetRect(-wr.left, -wr.top);
	wr.OffsetRect(-wr.left, -wr.top);
	dc.ExcludeClipRect(&cr);
	dc.FillSolidRect(wr, GetSysColor(COLOR_BTNFACE));

	//Ìî³ä±³¾°-----------------------------------------   
	//dc.FillSolidRect(wr, RGB(214,219,239) );   

	// Do not call CToolBar::OnNcPaint() for painting messages
}
void CPlayerToolBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	ArrangeControls();
}

void CPlayerToolBar::OnInitialUpdate()
{
	ArrangeControls();
}

BOOL CPlayerToolBar::OnVolumeMute(UINT nID)
{
	SetMute(!IsMuted()); 
	return FALSE;
}

void CPlayerToolBar::OnUpdateVolumeMute(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(IsMuted());
}

BOOL CPlayerToolBar::OnVolumeUp(UINT nID)
{
	m_volctrl.IncreaseVolume();
	return FALSE;
}

BOOL CPlayerToolBar::OnVolumeDown(UINT nID)
{
	m_volctrl.DecreaseVolume();
	return FALSE;
}


#define TIMER_FASTFORWORD 251
void CPlayerToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	iBottonClicked = -1;
	KillTimer(TIMER_FASTFORWORD);

	for(int i = 0, j = GetToolBarCtrl().GetButtonCount(); i < j; i++)
	{
		if(GetButtonStyle(i)&(TBBS_SEPARATOR|TBBS_DISABLED))
			continue;

		CRect r;
		GetItemRect(i, r);
		if(r.PtInRect(point))
		{
			UINT iButtonID , iStyle ;
			int iImage ;
			GetButtonInfo(i,iButtonID,iStyle,iImage );
			if(iButtonID == ID_PLAY_BWD || iButtonID == ID_PLAY_FWD){
				iBottonClicked = iButtonID;
				iFastFFWCount = 0;
				SetTimer(TIMER_FASTFORWORD, 350, NULL);
			}
			__super::OnLButtonDown(nFlags, point);
			
			return;
		}
	} 

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	if(!pFrame->m_fFullScreen)
	{
		MapWindowPoints(pFrame, &point, 1);
		pFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	}
}
void CPlayerToolBar::OnTimer(UINT nIDEvent){
	switch(nIDEvent){
		case TIMER_FASTFORWORD:
			if(iBottonClicked < 0 ){
				KillTimer(TIMER_FASTFORWORD);
				break;
			}
			iFastFFWCount++;
			//fast forward or backword
			{
				CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
				int iMsg;
				if(iFastFFWCount > 10){
					if(iBottonClicked == ID_PLAY_BWD){
						iMsg = ID_PLAY_SEEKBACKWARDLARGE;
					}else if(iBottonClicked == ID_PLAY_FWD){
						iMsg = ID_PLAY_SEEKFORWARDLARGE;
					}
				}else{
					if(iBottonClicked == ID_PLAY_BWD){
						iMsg = ID_PLAY_SEEKBACKWARDMED;
					}else if(iBottonClicked == ID_PLAY_FWD){
						iMsg = ID_PLAY_SEEKKEYFORWARD;
					}
				}
				pFrame->PostMessage( WM_COMMAND, iMsg);
			}
			break;
	}

	__super::OnTimer(nIDEvent);
}
void CPlayerToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	KillTimer(TIMER_FASTFORWORD);
	for(int i = 0, j = GetToolBarCtrl().GetButtonCount(); i < j; i++)
	{
		CRect r;
		GetItemRect(i, r);
		if(r.PtInRect(point))
		{
			UINT iButtonID, iStyle ;
			int iImage ;
			GetButtonInfo(i,iButtonID, iStyle , iImage );
			if(iButtonID == iBottonClicked ){
				if(iFastFFWCount == 0){
					int iMsg;
					CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
					// not increase or decrease play rate
					if(iBottonClicked == ID_PLAY_BWD){
						iMsg = ID_PLAY_SEEKBACKWARDMED;
					}else if(iBottonClicked == ID_PLAY_FWD){
						iMsg = ID_PLAY_SEEKFORWARDMED;
					}
					pFrame->PostMessage( WM_COMMAND, iMsg);
				}
			}
			break;
		}
	}
	iBottonClicked = -1;
	
	__super::OnLButtonUp(nFlags, point);
	return;
}