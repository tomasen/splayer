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

// PlayerSeekBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerSeekBar.h"
#include "MainFrm.h"
#include "..\..\svplib\svplib.h"
// CPlayerSeekBar

IMPLEMENT_DYNAMIC(CPlayerSeekBar, CDialogBar)

CPlayerSeekBar::CPlayerSeekBar() : 
	m_start(0), m_stop(100), m_pos(0), m_posreal(0), 
	m_fEnabled(false)
{
}

CPlayerSeekBar::~CPlayerSeekBar()
{
}
BOOL CPlayerSeekBar::Create(CWnd* pParentWnd)
{

	if(!CDialogBar::Create(pParentWnd, IDD_PLAYERSEEKBAR, WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM, IDD_PLAYERSEEKBAR))
		return FALSE;

	cursorHand = ::LoadCursor(NULL, IDC_HAND);

	/*
	m_toolTip.CreateEx(this,0,WS_EX_NOACTIVATE);
	m_toolTip.ModifyStyleEx( WS_EX_LAYERED , NULL);
	m_ti.cbSize = sizeof(m_ti);
	m_ti.uFlags = TTF_TRACK|TTF_ABSOLUTE|TTF_TRANSPARENT;
	m_ti.hwnd = m_hWnd;
	m_ti.hinst = NULL;
	m_ti.uId = (UINT)0;
	m_ti.lpszText = LPSTR_TEXTCALLBACK;
	m_ti.rect.left = 0;    
	m_ti.rect.top = 0;
	m_ti.rect.right = 0;
	m_ti.rect.bottom = 0;
	EnableToolTips(FALSE);
	m_toolTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&m_ti);*/
	return TRUE;
}

BOOL CPlayerSeekBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!CDialogBar::PreCreateWindow(cs))
		return FALSE;

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;
	m_dwStyle |= CBRS_SIZE_FIXED;

	return TRUE;
}

void CPlayerSeekBar::Enable(bool fEnable)
{
	m_fEnabled = fEnable;
	Invalidate();
}

void CPlayerSeekBar::GetRange(__int64& start, __int64& stop)
{
	start = m_start;
	stop = m_stop;
}

void CPlayerSeekBar::SetRange(__int64 start, __int64 stop) 
{
	if(start > stop) start ^= stop, stop ^= start, start ^= stop;
	m_start = start;
	m_stop = stop;
	if(m_pos < m_start || m_pos >= m_stop) SetPos(m_start);
}

__int64 CPlayerSeekBar::GetPos()
{
	return(m_pos);
}

__int64 CPlayerSeekBar::GetPosReal()
{
	return(m_posreal);
}

void CPlayerSeekBar::SetPos(__int64 pos)
{
	CWnd* w = GetCapture();
	if(w && w->m_hWnd == m_hWnd) return;

	SetPosInternal(pos);
}

void CPlayerSeekBar::SetPosInternal(__int64 pos)
{
	if(m_pos == pos) return;

	CRect before = GetThumbRect();
	m_pos = min(max(pos, m_start), m_stop);
	m_posreal = pos;
	CRect after = GetThumbRect();

	if(before != after) InvalidateRect(before | after);
}

CRect CPlayerSeekBar::GetChannelRect()
{
	CRect r;
	GetClientRect(&r);
	r.DeflateRect(5, 3, 5, 2); //
	r.bottom = r.top + 10;
	return(r);
}

CRect CPlayerSeekBar::GetThumbRect()
{
//	bool fEnabled = m_fEnabled || m_start >= m_stop;

	CRect r = GetChannelRect();

	int x = r.left + (int)((m_start < m_stop /*&& fEnabled*/) ? (__int64)r.Width() * (m_pos - m_start) / (m_stop - m_start) : 0);
	int y = r.CenterPoint().y;

	r.SetRect(x, y, x, y);
	r.InflateRect(6, 7, 7, 8);

	return(r);
}

CRect CPlayerSeekBar::GetInnerThumbRect()
{
  CRect r = GetThumbRect();

	bool fEnabled = m_fEnabled && m_start < m_stop;
	r.DeflateRect(3, fEnabled ? 5 : 4, 3, fEnabled ? 5 : 4);

	return(r);
}

void CPlayerSeekBar::MoveThumb(CPoint point)
{
	CRect r = GetChannelRect();
	
	if(r.left >= r.right) return;

	if(point.x < r.left) SetPos(m_start);
	else if(point.x >= r.right) SetPos(m_stop);
	else
	{
		__int64 w = r.right - r.left;
		if(m_start < m_stop)
			SetPosInternal(m_start + ((m_stop - m_start) * (point.x - r.left) + (w/2)) / w);
	}
}

BEGIN_MESSAGE_MAP(CPlayerSeekBar, CDialogBar)
	//{{AFX_MSG_MAP(CPlayerSeekBar)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
//	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnTtnNeedText)
	ON_COMMAND_EX(ID_PLAY_STOP, OnPlayStop)
	ON_WM_SETCURSOR()
	ON_WM_MOUSELEAVE()
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

BOOL CPlayerSeekBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message){

	SetCursor(cursorHand );
	return TRUE;
	
	//return CWnd::OnSetCursor(pWnd, 0, 0);
}

// CPlayerSeekBar message handlers
/*
BOOL CPlayerSeekBar::OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	//AfxMessageBox(_T("x")); //where is my tooltip!?!
	UNREFERENCED_PARAMETER(id);

	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	UINT_PTR nID = pNMHDR->idFrom;
	BOOL bRet = FALSE;
	
	if(nID == 0){
		CString toolTip;
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);
		CRect r = GetChannelRect();

		if(r.left <= r.right){

			__int64 mPos = 0;
			if(point.x < r.left) mPos = m_start;
			else if(point.x >= r.right) mPos = m_stop;
			else
			{
				__int64 w = r.right - r.left;
				if(m_start < m_stop)
					mPos = (m_start + ((m_stop - m_start) * (point.x - r.left) + (w/2)) / w);
			}
			DVD_HMSF_TIMECODE tcNow = RT2HMSF(mPos);
			
			if( tcNow.bHours > 0)
				toolTip.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
			else 
				toolTip.Format(_T("%02d:%02d"), tcNow.bMinutes, tcNow.bSeconds);

			
			//AfxMessageBox(toolTip);
			//if(toolTip.IsEmpty())
			//	toolTip = _T("Unkown");

			if(!toolTip.IsEmpty()){
				pTTT->lpszText = toolTip.GetBuffer();
				pTTT->hinst = AfxGetResourceHandle();
				bRet = TRUE;
				
			}
		}
	}

	*pResult = 0;

	return bRet;
}*/
void CPlayerSeekBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	bool fEnabled = m_fEnabled && m_start < m_stop;

	AppSettings& s = AfxGetAppSettings();

	COLORREF 
//		white = s.GetColorFromTheme(_T("SeekBarPlayed"), NEWUI_COLOR_SEEKBAR_PLAYED),
//		shadow = s.GetColorFromTheme(_T("SeekBarBottomBorderBG"), NEWUI_COLOR_BG), 
//		light = s.GetColorFromTheme(_T("SeekBarPlayed"), NEWUI_COLOR_BG), 
		bkg = s.GetColorFromTheme(_T("SeekBarBG"), NEWUI_COLOR_TOOLBAR_UPPERBG);

	CBrush bBkg(bkg);
	/*/ thumb
	if(0){
		CRect r = GetThumbRect(), r2 = GetInnerThumbRect();
		CRect rt = r, rit = r2;

		dc.Draw3dRect(&r, light, 0);
		r.DeflateRect(0, 0, 1, 1);
		dc.Draw3dRect(&r, light, shadow);
		r.DeflateRect(1, 1, 1, 1);

		CBrush b(bkg);

		dc.FrameRect(&r, &b);
		r.DeflateRect(0, 1, 0, 1);
		dc.FrameRect(&r, &b);

		r.DeflateRect(1, 1, 0, 0);
		dc.Draw3dRect(&r, shadow, bkg);

		if(fEnabled)
		{
			r.DeflateRect(1, 1, 1, 2);
			CPen white(PS_INSIDEFRAME, 1, white);
			CPen* old = dc.SelectObject(&white);
			dc.MoveTo(r.left, r.top);
			dc.LineTo(r.right, r.top);
			dc.MoveTo(r.left, r.bottom);
			dc.LineTo(r.right, r.bottom);
			dc.SelectObject(old);
			dc.SetPixel(r.CenterPoint().x, r.top, 0);
			dc.SetPixel(r.CenterPoint().x, r.bottom, 0);
		}

		dc.SetPixel(r.CenterPoint().x+5, r.top-4, bkg);

		{
			CRgn rgn1, rgn2;
			rgn1.CreateRectRgnIndirect(&rt);
			rgn2.CreateRectRgnIndirect(&rit);
			ExtSelectClipRgn(dc, rgn1, RGN_DIFF);
			ExtSelectClipRgn(dc, rgn2, RGN_OR);
		}
	}
*/
	// channel
	{
		CRect r = GetChannelRect();

		int cur = r.left + (int)((m_start < m_stop /*&& fEnabled*/) ? (__int64)r.Width() * (m_pos - m_start) / (m_stop - m_start) : 0);
		
#define CORBARS 10
		COLORREF havntplayed = s.GetColorFromTheme(_T("SeekBarUnPlayed"), 0x00434343);
		COLORREF Bars[CORBARS] = {s.GetColorFromTheme(_T("SeekBarPlayed1"), 0x000f412d), 
			s.GetColorFromTheme(_T("SeekBarPlayed2"), 0x0083ffdf), 
			s.GetColorFromTheme(_T("SeekBarPlayed3"), 0x0071fdd4), 
			s.GetColorFromTheme(_T("SeekBarPlayed4"), 0x0061f9c6),
      s.GetColorFromTheme(_T("SeekBarPlayed5"), 0x0061f9c6),
      s.GetColorFromTheme(_T("SeekBarPlayed6"), 0x0061f9c6),
			s.GetColorFromTheme(_T("SeekBarPlayed7"), 0x005ff5ba),
			s.GetColorFromTheme(_T("SeekBarPlayed8"), 0x0064f1b2),
			s.GetColorFromTheme(_T("SeekBarPlayed9"), 0x006fefb0),
			s.GetColorFromTheme(_T("SeekBarPlayed10"), 0x000f412d)};

		{
			CPen line(PS_INSIDEFRAME, 1, bkg);
			CPen* old = dc.SelectObject(&line);
			dc.MoveTo( cur , r.top );
			dc.LineTo( r.right, r.top );
			dc.MoveTo( cur , r.top + CORBARS - 1);
			dc.LineTo( r.right, r.top + CORBARS - 1);
		}
		CRect rFilled(r);
		rFilled.left =   cur;
		rFilled.top++;
		rFilled.bottom--;
		dc.FillSolidRect(&rFilled,  havntplayed ); //fEnabled ?
		
		//r = GetChannelRect();
		for(int i = 0; i < CORBARS ; i++){
			CPen line(PS_INSIDEFRAME, 1, Bars[i]);
			CPen* old = dc.SelectObject(&line);
			dc.MoveTo( r.left , r.top + i);
			dc.LineTo( cur, r.top + i);
		}

		{
			COLORREF P2 = s.GetColorFromTheme(_T("SeekBarBorder1"), 0x000d3324);
			COLORREF P1 = s.GetColorFromTheme(_T("SeekBarBorder2"),0x00091611);
			dc.SetPixel(r.left-1, r.top+1, P2);
			dc.SetPixel(r.left, r.top, P2);
			dc.SetPixel(r.left-1, r.bottom-2, P2);
			dc.SetPixel(r.left, r.bottom-1, P2);
			dc.SetPixel(cur+1, r.top+1, P2);
			dc.SetPixel(cur, r.top, P2);
			dc.SetPixel(cur+1, r.bottom-2, P2);
			dc.SetPixel(cur, r.bottom-1, P2);
			dc.SetPixel(cur+1, r.top, P1);
			dc.SetPixel(cur+1, r.bottom-1, P1);
			dc.SetPixel(r.left-1, r.top, P1);
			dc.SetPixel(r.left-1, r.bottom-1, P1);

			CPen line(PS_INSIDEFRAME, 1, Bars[0]);
			CPen* old = dc.SelectObject(&line);

			dc.MoveTo( r.left-1 , r.top + 2);
			dc.LineTo( r.left-1 , r.bottom-2);
			dc.MoveTo( cur+1 , r.top + 2);
			dc.LineTo( cur+1 , r.bottom-2);
		}

		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		if( pFrame->m_aRefTime > 0){
			CRect rFilled(r);
			
			rFilled.left += (__int64)r.Width() * (pFrame->m_aRefTime - m_start) / (m_stop - m_start) ;
			if( pFrame->m_aRefTime < pFrame->m_bRefTime )
				rFilled.right = r.left + (__int64)r.Width() * (pFrame->m_bRefTime - m_start) / (m_stop - m_start) ;
			else
				rFilled.right = rFilled.left + 1;

			rFilled.top+=2;
			rFilled.bottom-=2;

			CBrush nBrush(HS_DIAGCROSS,  s.GetColorFromTheme(_T("SeekBarABArea"), 0x003fff9a ));
			dc.FillRect(&rFilled,&nBrush ); //fEnabled ?
		}
		
		/*
		switch( cur % 4 ){
					case 3:
						cur--;
						break;
					case 2:
						cur+=2;
						break;
					case 1:
						cur++;
						break;
				}
				for(int drawPos = cur ; drawPos < r.right; drawPos +=2){
					CRect step(drawPos,r.top, drawPos+2, r.bottom);
					if(drawPos % 4){
						dc.FillSolidRect( &step, NEWUI_COLOR_TOOLBAR_UPPERBG);
					}else{
						dc.FillSolidRect( &step, white);
					}
				}*/
		
		//r.InflateRect(1, 1);
		//dc.Draw3dRect(&r, shadow, light);
		dc.ExcludeClipRect(&r);
	}

	// background
	{
		CRect r;
		GetClientRect(&r);
		dc.FillRect(&r, &bBkg);
	}


	// Do not call CDialogBar::OnPaint() for painting messages
}


void CPlayerSeekBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	Invalidate();
}
void CPlayerSeekBar::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	//OnLButtonDown( nFlags, point);
	CMainFrame* pFrame = (CMainFrame*) AfxGetMainWnd();
	//pFrame->PostMessage(WM_COMMAND, ID_PLAY_PAUSE);
	
	pFrame->OnMenu(pFrame->m_ABMenu.GetSubMenu(0));

	//AfxMessageBox(_T("2"));
	//CPoint point;
	//GetCursorPos(&point);

	//m_ABMenu.TrackPopupMenu(TPM_RIGHTBUTTON|TPM_NOANIMATION, point.x+1, point.y+1, this);

	
	CDialogBar::OnRButtonDown(nFlags, point);
}

void CPlayerSeekBar::OnRButtonUp(UINT nFlags, CPoint point)
{ 
	// TODO: Add your message handler code here and/or call default

	ReleaseCapture();

	CDialogBar::OnRButtonUp(nFlags, point);
}

void CPlayerSeekBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(m_fEnabled && (GetChannelRect() | GetThumbRect()).PtInRect(point))
	{
		SetCapture();
		__int64 posBefore = m_pos;
		MoveThumb(point);
		AfxGetMainWnd()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_pos, SB_THUMBPOSITION), (LPARAM)m_hWnd);
	}

	CDialogBar::OnLButtonDown(nFlags, point);
}

void CPlayerSeekBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();

	CDialogBar::OnLButtonUp(nFlags, point);
}
static CPoint m_lastMouseMove;
void CPlayerSeekBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CSize diff = m_lastMouseMove - point;
	BOOL bMouseMoved =  diff.cx || diff.cy ;
	if(bMouseMoved)
		m_lastMouseMove = point;

	CWnd* w = GetCapture();
	if(w && w->m_hWnd == m_hWnd && (nFlags & MK_LBUTTON))
	{
		MoveThumb(point);
		AfxGetMainWnd()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_pos, SB_THUMBTRACK), (LPARAM)m_hWnd);
	}
	if(bMouseMoved){
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		if(pFrame->IsSomethingLoaded()){
			/*CPoint pt;
			GetCursorPos(&pt);
			pt.y -= 20 * pFrame->m_nLogDPIY / 96;
			pt.x += 8 ;
			m_toolTip.SendMessage(TTM_TRACKPOSITION, 0, (LPARAM)MAKELPARAM(pt.x, pt.y));
			m_toolTip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
			;*/
			if(pFrame->m_tip.IsWindowVisible()){
				SetTimecodeTip();
			}else{
				SetTimer(IDT_OPENTIPS, 140, NULL);
			}
		}else{
			CloseToolTips();
		}
	}
	KillTimer(IDT_CLOSETIPS);
	SetTimer(IDT_CLOSETIPS, 600, NULL);
	CDialogBar::OnMouseMove(nFlags, point);
}

BOOL CPlayerSeekBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BOOL CPlayerSeekBar::OnPlayStop(UINT nID)
{
	SetPos(0);
	CloseToolTips();
	return FALSE;
}

void CPlayerSeekBar::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default
	CloseToolTips();
	CDialogBar::OnMouseLeave();
}

void CPlayerSeekBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if(bShow != SW_SHOW){
		CloseToolTips();
	}

	__super::OnShowWindow(bShow, nStatus);


}
void CPlayerSeekBar::CloseToolTips(){
	//m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
	KillTimer(IDT_OPENTIPS);
	KillTimer(IDT_CLOSETIPS);
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (::IsWindow(pFrame->m_tip.m_hWnd))
		pFrame->m_tip.ClearStat();
}
void CPlayerSeekBar::SetTimecodeTip(){
	CString toolTip;
	CPoint point , screenPos;
	GetCursorPos(&point);
	screenPos = point;
	ScreenToClient(&point);
	CRect rcClient;
	if( !rcClient.PtInRect(point)){

		//return;
	}
	CRect r = GetChannelRect();

	if(r.left <= r.right){

		__int64 mPos = 0;
		if(point.x < r.left) mPos = m_start;
		else if(point.x >= r.right) mPos = m_stop;
		else
		{
			__int64 w = r.right - r.left;
			if(m_start < m_stop)
				mPos = (m_start + ((m_stop - m_start) * (point.x - r.left) + (w/2)) / w);
		}
		DVD_HMSF_TIMECODE tcNow = RT2HMSF(mPos);

		if( tcNow.bHours > 0)
			toolTip.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
		else 
			toolTip.Format(_T("%02d:%02d"), tcNow.bMinutes, tcNow.bSeconds);


		//AfxMessageBox(toolTip);
		//if(toolTip.IsEmpty())
		//	toolTip = _T("Unkown");

		if(!toolTip.IsEmpty()){
			CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
			pFrame->m_tip.m_text = toolTip;
			CSize tipsize = pFrame->m_tip.CountSize();
			point = screenPos;
			CRect rcTip ( point.x - tipsize.cx/2 , point.y - tipsize.cy - 6,point.x + tipsize.cx/2 , point.y -  6);
			//SVP_LogMsg5(_T("Tip %d %d %d %d") , rcTip.left,rcTip.top , rcTip.right,rcTip.left);

			HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

			MONITORINFO mi;
			mi.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &mi);

			if(rcTip.left < mi.rcWork.left){
				rcTip.MoveToX(mi.rcWork.left + 3);
			}else if(rcTip.right > mi.rcWork.right){
				rcTip.MoveToX(mi.rcWork.right - rcTip.Width() - 3);
			}

			pFrame->m_tip.MoveWindow( rcTip );
			pFrame->m_tip.ShowWindow(SW_SHOWNOACTIVATE);
			pFrame->m_tip.Invalidate();
		}
	}
}
void CPlayerSeekBar::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(IDT_CLOSETIPS == nIDEvent){
		KillTimer(IDT_CLOSETIPS);
		CloseToolTips();
	}else if(nIDEvent == IDT_OPENTIPS){
		KillTimer(IDT_OPENTIPS);
		SetTimecodeTip();

	}
	CDialogBar::OnTimer(nIDEvent);
}

void CPlayerSeekBar::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->m_tip.OnRealClose();
	CDialogBar::OnClose();
}


BOOL CPlayerSeekBar::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MYMOUSELAST && WM_MOUSEMOVE != pMsg->message ){
		CMainFrame* pFrame = (CMainFrame*) AfxGetMainWnd();
		if(pFrame){
			pFrame->KillTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER);
			//SVP_LogMsg5(L" IsSomethingLoaded %d %d ", pFrame->IsSomethingLoaded(), __LINE__);
			if( pFrame->IsSomethingLoaded()){
				AppSettings& s = AfxGetAppSettings();
				if(s.bUserAeroUI())
					pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 5000, NULL);
				else
					pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 3000, NULL);

			}
		}
	}
	return CDialogBar::PreTranslateMessage(pMsg);
}
