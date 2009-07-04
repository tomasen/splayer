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

// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "mplayerc.h"
#include "ChildView.h"
#include "MainFrm.h"
#include "libpng.h"
#include "..\..\svplib\SVPToolBox.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView() : m_vrect(0,0,0,0)
{
	m_lastlmdowntime = 0;
	m_lastlmdownpoint.SetPoint(0, 0);

	//m_watermark.LoadFromResource(IDF_LOGO2);
	LoadLogo();
	CSUIButton * btnFileOpen = new CSUIButton(L"BTN_BIGOPEN.BMP" , ALIGN_TOPLEFT, CRect(-50 , -62, 0,0)  , FALSE, ID_FILE_OPENQUICK, FALSE  ) ;
	m_btnList.AddTail( btnFileOpen);

	m_btnList.AddTail( new CSUIButton(L"BTN_OPENADV.BMP" ,ALIGN_TOPLEFT, CRect(-50 , -62, 0,0)  , FALSE, ID_FILE_OPENMEDIA, FALSE, ALIGN_LEFT,btnFileOpen,  CRect(3,3,3,3) ) ) ;
	
	m_btnList.AddTail( new CSUIButton(L"WATERMARK2.BMP" , ALIGN_BOTTOMRIGHT, CRect(6 , 6, 0,6)  , TRUE, 0, FALSE  ) );
}

CChildView::~CChildView()
{
}

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if(!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.style &= ~WS_BORDER;
	
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_HAND), HBRUSH(COLOR_WINDOW+1), NULL);


	return TRUE;
}

BOOL CChildView::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MYMOUSELAST)
	{
		CWnd* pParent = GetParent();
		CPoint p(pMsg->lParam);
		::MapWindowPoints(pMsg->hwnd, pParent->m_hWnd, &p, 1);

		bool fDblClick = false;

		bool fInteractiveVideo = ((CMainFrame*)AfxGetMainWnd())->IsInteractiveVideo();
/*
		if(fInteractiveVideo)
		{
			if(pMsg->message == WM_LBUTTONDOWN)
			{
				if((pMsg->time - m_lastlmdowntime) <= GetDoubleClickTime()
				&& abs(pMsg->pt.x - m_lastlmdownpoint.x) <= GetSystemMetrics(SM_CXDOUBLECLK)
				&& abs(pMsg->pt.y - m_lastlmdownpoint.y) <= GetSystemMetrics(SM_CYDOUBLECLK))
				{
					fDblClick = true;
					m_lastlmdowntime = 0;
					m_lastlmdownpoint.SetPoint(0, 0);
				}
				else
				{
					m_lastlmdowntime = pMsg->time;
					m_lastlmdownpoint = pMsg->pt;
				}
			}
			else if(pMsg->message == WM_LBUTTONDBLCLK)
			{
				m_lastlmdowntime = pMsg->time;
				m_lastlmdownpoint = pMsg->pt;
			}
		}
*/
		if((pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP || pMsg->message == WM_MOUSEMOVE)
		&& fInteractiveVideo)
		{
			if(pMsg->message == WM_MOUSEMOVE)
			{
				pParent->PostMessage(pMsg->message, pMsg->wParam, MAKELPARAM(p.x, p.y));
			}

			if(fDblClick)
			{
				pParent->PostMessage(WM_LBUTTONDOWN, pMsg->wParam, MAKELPARAM(p.x, p.y));
				pParent->PostMessage(WM_LBUTTONDBLCLK, pMsg->wParam, MAKELPARAM(p.x, p.y));
			}
		}
		else
		{
			pParent->PostMessage(pMsg->message, pMsg->wParam, MAKELPARAM(p.x, p.y));
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CChildView::SetVideoRect(CRect r)
{
	m_vrect = r;

	Invalidate();
}

void CChildView::LoadLogo()
{
	AppSettings& s = AfxGetAppSettings();

	CAutoLock cAutoLock(&m_csLogo);

	m_logo.Destroy();
	
	if(s.logoext)
	{
		if(s.fXpOrBetter)
			m_logo.Load(s.logofn);
		else if(HANDLE h = LoadImage(NULL, s.logofn, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))
			m_logo.Attach((HBITMAP)h); // win9x bug: Inside Attach GetObject() will return all zeros in DIBSECTION and silly CImage uses that to init width, height, bpp, ... so we can't use CImage::Draw later
	}

	if(m_logo.IsNull() && IDF_LOGO7 == s.logoid)
	{
		CString OEMBGPath;
		CSVPToolBox svpTool;
		OEMBGPath = svpTool.GetPlayerPath(_T("skins\\oembg.png"));
		if(svpTool.ifFileExist(OEMBGPath)){
			if(s.fXpOrBetter)
				m_logo.Load(OEMBGPath);
			else if(HANDLE h = LoadImage(NULL, OEMBGPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))
				m_logo.Attach((HBITMAP)h); 
		}


	}
	if(m_logo.IsNull())
	{
		m_logo.LoadFromResource(s.logoid);
	}

	if(m_logo.IsNull())
	{
		m_logo.LoadFromResource(IDF_LOGO7);
	}
	if(m_hWnd) Invalidate();
}

CSize CChildView::GetLogoSize()
{
	CSize ret(0,0);
	if(!m_logo.IsNull())
		ret.SetSize(m_logo.GetWidth(), m_logo.GetHeight());
	return ret;
}

IMPLEMENT_DYNAMIC(CChildView, CWnd)

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	ON_COMMAND_EX(ID_PLAY_PLAYPAUSE, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_PLAY, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_PAUSE, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_STOP, OnPlayPlayPauseStop)
	ON_WM_SETCURSOR()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers
#include "../../svplib/svplib.h"
void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
	pFrame->RepaintVideo();

	
	if(!pFrame->IsSomethingLoaded()){
		AppSettings& s = AfxGetAppSettings();

		CRect rcWnd;
		GetWindowRect(rcWnd);
		CRect rcClient;
		GetClientRect(&rcClient);
		CMemoryDC hdc(&dc, rcClient);
		hdc.FillSolidRect( rcClient, 0);
		if( !m_logo.IsNull() ){ 
			BITMAP bm;
			GetObject(m_logo, sizeof(bm), &bm);

			CRect r;
			GetClientRect(r);
			if( s.logostretch == 1){ // 保持宽高  不缩放
				int w = min(bm.bmWidth, r.Width());
				int h = min(abs(bm.bmHeight), r.Height());
				int x = (r.Width() - w) / 2;
				int y = (r.Height() - h) / 2;
				m_logo_r = CRect(CPoint(x, y), CSize(w, h));
			}else if( s.logostretch == 2){ // 缩放 不保持宽高比
				m_logo_r = r;
			}else{// 缩放 保持宽高比
				if ( bm.bmWidth * 100 / bm.bmHeight > r.Width() * 100 / r.Height() ){
					//以r.Width()为准
					int w = r.Width();
					int h = r.Width() * bm.bmHeight/ bm.bmWidth ;
					int x = 0;
					int y = (r.Height() - h) / 2;
					m_logo_r = CRect(CPoint(x, y), CSize(w, h));
				}else{
					int w = r.Height() * bm.bmWidth / bm.bmHeight;
					int h = r.Height();
					int x = (r.Width() - w) / 2;
					int y = 0;
					m_logo_r = CRect(CPoint(x, y), CSize(w, h));
				}
				
			}
			
			int oldmode = hdc.SetStretchBltMode(STRETCH_HALFTONE);
			m_logo.StretchBlt(hdc, m_logo_r, CRect(0,0,bm.bmWidth,abs(bm.bmHeight)));
		}

		m_btnList.PaintAll( &hdc, rcWnd );
	}
	// Do not call CWnd::OnPaint() for painting messages
}
void CChildView::ReCalcBtn(){
	CRect rcWnd;
	GetWindowRect(rcWnd);
	m_btnList.OnSize(rcWnd);
}
void CChildView::OnSetFocus(CWnd* pOldWnd){
	
}
BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	CRect r;

	CAutoLock cAutoLock(&m_csLogo);
	if(((CMainFrame*)GetParentFrame())->IsSomethingLoaded())
	{
		//pDC->FillSolidRect(CRect(30,30,60,60) , RGB(0xff,0,0));
		pDC->ExcludeClipRect(m_vrect);
		GetClientRect(r);
		pDC->FillSolidRect(r, 0);
	}
	else 
	{
		
				
		
		/*
		if(!m_watermark.IsNull()){
					BITMAP bmw;
					GetObject(m_watermark, sizeof(bmw), &bmw);
					CRect rw;
					GetClientRect(rw);
					int ww = min(bmw.bmWidth, rw.Width());
					int hw = min(abs(bmw.bmHeight), rw.Height());
					rw = CRect(CPoint(rw.Width() - ww , rw.Height() - hw), CSize(ww, hw));
					m_watermark.StretchBlt( *pDC ,  rw,  CRect(0,0,bmw.bmWidth,abs(bmw.bmHeight)));
					pDC->ExcludeClipRect(rw);
				}* /
		
//		m_logo.Draw(*pDC, r);
		pDC->SetStretchBltMode(oldmode);
		pDC->ExcludeClipRect(r);
*/

		/*
CRect rcWnd;
		GetWindowRect(rcWnd);
		CRect rcClient;
		GetClientRect(rcClient);
		//pDC->FillSolidRect(rcClient, 0);

		CMemoryDC hdc(pDC, rcClient);
		hdc.FillSolidRect( rcClient, 0);
		m_btnBBList.PaintAll( &hdc, rcWnd );
*/

		
	}

	
	

	
	return TRUE;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	((CMainFrame*)GetParentFrame())->MoveVideoWindow();

	ReCalcBtn();
}


void CChildView::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanged(lpwndpos);

	((CMainFrame*)GetParentFrame())->MoveVideoWindow();
}

BOOL CChildView::OnPlayPlayPauseStop(UINT nID)
{
	if(nID == ID_PLAY_STOP) SetVideoRect();
	return FALSE;
}

BOOL CChildView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(((CMainFrame*)GetParentFrame())->m_fHideCursor)
	{
		SetCursor(NULL);
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CChildView::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	if(!((CMainFrame*)GetParentFrame())->IsFrameLessWindow()) 
	{
		//InflateRect(&lpncsp->rgrc[0], -1, -1);
	}

	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CChildView::OnNcPaint()
{
	if(!((CMainFrame*)GetParentFrame())->IsFrameLessWindow()) 
	{
// 		CRect r;
// 		GetWindowRect(r);
// 		r.OffsetRect(-r.left, -r.top);
// 
// 		CWindowDC(this).Draw3dRect(&r, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT)); 
	}
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	

	return 0;
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CSize diff = m_lastMouseMove - point;
	BOOL bMouseMoved =  diff.cx || diff.cy ;
	m_lastMouseMove = point;

	CRect rc;
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	GetWindowRect(&rc);
	point += rc.TopLeft() ;

	if(bMouseMoved){

		UINT ret = m_btnList.OnHitTest(point,rc);
		m_nItemToTrack = ret;
		
			
			if( m_btnList.HTRedrawRequired ){
				Invalidate();
			}
		
	}
	CWnd::OnMouseMove(nFlags, point);
}
static BOOL m_bMouseDown = FALSE;
void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	iBottonClicked = -1;
	m_bMouseDown = TRUE;
	CRect rc;
	GetWindowRect(&rc);

	point += rc.TopLeft() ;
	UINT ret = m_btnList.OnHitTest(point,rc);
	if( m_btnList.HTRedrawRequired ){
		if(ret)
			SetCapture();
		Invalidate();
	}
	m_nItemToTrack = ret;

	CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	KillTimer(TIMER_FASTFORWORD);
	ReleaseCapture();

	CRect rc;
	GetWindowRect(&rc);

	CPoint xpoint = point + rc.TopLeft() ;
	UINT ret = m_btnList.OnHitTest(xpoint,rc);
	if( m_btnList.HTRedrawRequired ){
		if(ret)
			pFrame->PostMessage( WM_COMMAND, ret);
		Invalidate();
	}
	m_nItemToTrack = ret;

	//	__super::OnLButtonUp(nFlags, point);
	m_bMouseDown = FALSE;
}
