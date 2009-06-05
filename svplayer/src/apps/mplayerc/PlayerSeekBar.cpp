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
	r.DeflateRect(5, 2, 5, 2); //
	r.bottom = r.top + 8;
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
	ON_COMMAND_EX(ID_PLAY_STOP, OnPlayStop)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BOOL CPlayerSeekBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message){

	SetCursor(cursorHand );
	return TRUE;
	
	//return CWnd::OnSetCursor(pWnd, 0, 0);
}

// CPlayerSeekBar message handlers

void CPlayerSeekBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	bool fEnabled = m_fEnabled && m_start < m_stop;

	COLORREF 
		white = NEWUI_COLOR_SEEKBAR_PLAYED,
		shadow = NEWUI_COLOR_BG, 
		light = NEWUI_COLOR_BG, 
		bkg = NEWUI_COLOR_TOOLBAR_UPPERBG;

	CBrush bBkg(bkg);
	// thumb
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

	// channel
	{
		CRect r = GetChannelRect();

		int cur = r.left + (int)((m_start < m_stop /*&& fEnabled*/) ? (__int64)r.Width() * (m_pos - m_start) / (m_stop - m_start) : 0);
		
#define CORBARS 8
		COLORREF havntplayed = 0x00434343;
		COLORREF Bars[CORBARS] = {0x000f412d
		, 0x0083ffdf, 0x0071fdd4, 0x0061f9c6 ,0x005ff5ba ,	0x0064f1b2,	0x006fefb0,	0x000f412d};

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
			COLORREF P2 = 0x000d3324;
			COLORREF P1 = 0x00091611;
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

void CPlayerSeekBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(m_fEnabled && (GetChannelRect() | GetThumbRect()).PtInRect(point))
	{
		SetCapture();
		MoveThumb(point);
		GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_pos, SB_THUMBPOSITION), (LPARAM)m_hWnd);
	}

	CDialogBar::OnLButtonDown(nFlags, point);
}

void CPlayerSeekBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();

	CDialogBar::OnLButtonUp(nFlags, point);
}

void CPlayerSeekBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd* w = GetCapture();
	if(w && w->m_hWnd == m_hWnd && (nFlags & MK_LBUTTON))
	{
		MoveThumb(point);
		GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_pos, SB_THUMBTRACK), (LPARAM)m_hWnd);
	}

	CDialogBar::OnMouseMove(nFlags, point);
}

BOOL CPlayerSeekBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BOOL CPlayerSeekBar::OnPlayStop(UINT nID)
{
	SetPos(0);
	return FALSE;
}
