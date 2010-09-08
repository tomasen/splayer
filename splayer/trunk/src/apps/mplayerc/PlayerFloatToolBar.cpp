// PlayerFloatToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerFloatToolBar.h"
#include "MainFrm.h"

// CPlayerFloatToolBar

IMPLEMENT_DYNAMIC(CPlayerFloatToolBar, CFrameWnd)

CPlayerFloatToolBar::CPlayerFloatToolBar()
{
	TRACE(_T("CPlayerFloatToolBar::CPlayerFloatToolBar\n"));
}

CPlayerFloatToolBar::~CPlayerFloatToolBar()
{
	TRACE(_T("CPlayerFloatToolBar::~CPlayerFloatToolBar\n"));
}


BEGIN_MESSAGE_MAP(CPlayerFloatToolBar, CFrameWnd)
	ON_WM_SIZE()
	ON_WM_ENABLE()
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_SETFOCUS()
	ON_WM_SHOWWINDOW()
	ON_WM_KILLFOCUS()
	ON_WM_TIMER()
	ON_WM_NCCALCSIZE()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_NCHITTEST()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CPlayerFloatToolBar message handlers



void CPlayerFloatToolBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	CRect r,cr;

	{ //New UI
		CRect rc;
		//GetWindowRect(&rc);
		//rc-=rc.TopLeft();
		GetClientRect(&rc);
		



		// destroy old region
		if((HRGN)m_rgn)
		{
			m_rgn.DeleteObject();
		}

		m_rgn.CreateRoundRectRgn(2,1,rc.Width()-1,rc.Height()-1, 2,1);                 // rounded rect w/50 pixel corners

		
		SetWindowRgn(m_rgn,TRUE);

		
	}

}

void CPlayerFloatToolBar::OnEnable(BOOL bEnable)
{
	__super::OnEnable(bEnable);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	__super::OnClose();
}

BOOL CPlayerFloatToolBar::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return __super::PreTranslateMessage(pMsg);
}

void CPlayerFloatToolBar::OnMove(int x, int y)
{
	__super::OnMove(x, y);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnSetFocus(CWnd* pOldWnd)
{
	__super::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnKillFocus(CWnd* pNewWnd)
{
	__super::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnRealClose()
{
	__super::OnClose();
}
int CPlayerFloatToolBar::GetUIHeight(){
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
	if(pFrame && ::IsWindow(pFrame->m_hWnd)  )
	{
		
		m_lSeekBarHeight = 12;
		m_lToolBarHeight = pFrame->m_wndToolBar.m_nHeight * m_nLogDPIY / 96 ;
		if(pFrame->IsSomethingLoaded()){
			return m_lSeekBarHeight + m_lToolBarHeight;
		}
		
		return m_lToolBarHeight;
	}
	return 0;
}
void CPlayerFloatToolBar::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	__super::OnTimer(nIDEvent);
}

void CPlayerFloatToolBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	// TODO: Add your message handler code here and/or call default

	CFrameWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

int CPlayerFloatToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	m_nLogDPIY = pFrame->m_nLogDPIY;

	return 0;
}

void CPlayerFloatToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CSize diff = m_lastMouseMove - point;
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());

	BOOL bMouseMoved =  diff.cx || diff.cy ;
	if(bMouseMoved){
		pFrame->KillTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER);
        //SVP_LogMsg5(L" IsSomethingLoaded %d %d ", pFrame->IsSomethingLoaded(), __LINE__);
		if(pFrame->IsSomethingLoaded()){
			pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 8000, NULL); 
		}
		

		//SetTimer(IDT_CLOSE, 8000 , NULL);
		m_lastMouseMove = point;
	}
	

	CFrameWnd::OnMouseMove(nFlags, point);
}

LRESULT CPlayerFloatToolBar::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	UINT hit = __super::OnNcHitTest(point);
// 	if (m_bDraging && hit == HTCLIENT )
// 		return HTCAPTION;
// 	else

/*


	if(m_bDraging ){

		CRect r;
		GetClientRect(&r);
		ClientToScreen(&r);
		ClientToScreen(&point);

		if(r.PtInRect(point))
		{       
			if(m_mousedown)
			{   
				AfxGetMainWnd()->GetWindowRect(&r); 

				AfxGetMainWnd()->MoveWindow(
					r.left - (m_lastpoint.x - point.x),
					r.top - (m_lastpoint.y - point.y),
					r.Width(),r.Height());
				m_lastpoint = point;                
			}
		}
	}
*/

	return hit;
	
}

void CPlayerFloatToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	/*
m_mousedown = true; 
	ClientToScreen(&point);
	m_lastpoint = point;

*/

	__super::OnLButtonDown(nFlags, point);
}

void CPlayerFloatToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	/*
m_mousedown = false;
*/

	__super::OnLButtonUp(nFlags, point);
}
