// SeekBarTip.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SeekBarTip.h"


// CSeekBarTip

IMPLEMENT_DYNAMIC(CSeekBarTip, CWnd)

CSeekBarTip::CSeekBarTip()
{

}

CSeekBarTip::~CSeekBarTip()
{
}


BEGIN_MESSAGE_MAP(CSeekBarTip, CWnd)
	ON_WM_ENABLE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()



// CSeekBarTip message handlers



void CSeekBarTip::OnEnable(BOOL bEnable)
{
	return ;
	//CWnd::OnEnable(bEnable);

	// TODO: Add your message handler code here
}
void CSeekBarTip::SetTips(CString szText, BOOL bMove , CPoint* mPoint, UINT delayOpen){
	KillTimer(IDT_DELAYOPEN);
	if(delayOpen){
		m_delayText = szText;
		m_delayMove = bMove;
		GetCursorPos(&m_delayPoint);
		SetTimer(IDT_DELAYOPEN, delayOpen,NULL);
		return;
	}
	m_text = szText;
	if(szText.IsEmpty()){
		ShowWindow(SW_HIDE);
	}else{
		CSize tipsize =CountSize();
		if(bMove){
			CPoint point;
			if(mPoint)
				point = *mPoint;
			else
				GetCursorPos(&point);
			
			CRect rcTip ( point.x + 5 , point.y - tipsize.cy - 6,point.x + 5 + tipsize.cx , point.y -  6);

			TRACE(_T("Tip %d %d %d %d\n") , rcTip.left, rcTip.top, rcTip.bottom, rcTip.right);

			//get the hMonitor from the point instead of the hwnd, or the tip won't display at the 
			//right position when the main window is on the second monitor.
			HMONITOR hMonitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
			//HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

			MONITORINFO mi;
			mi.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &mi);

			TRACE(_T("MONITORINFO %d %d %d %d\n") , mi.rcWork.left, mi.rcWork.top , mi.rcWork.bottom,mi.rcWork.right);

			if(point.y > ( ( mi.rcWork.bottom + mi.rcWork.top) / 2 + 20 ) ){
				//tip on top

			}else{
				//tip is below
				rcTip.MoveToY( point.y + 6 );
			}

			if(point.y > ( ( mi.rcWork.right + mi.rcWork.left) / 2 + 20 ) ){
				//tip on left
				rcTip.MoveToX( point.x - 5 - tipsize.cx );
			}else{
				//tip is right
				
			}

			if(rcTip.left < mi.rcWork.left){
				rcTip.MoveToX(mi.rcWork.left + 3);
			}else if(rcTip.right > mi.rcWork.right){
				rcTip.MoveToX(mi.rcWork.right - rcTip.Width() - 3);
			}

			MoveWindow( rcTip );
			ShowWindow(SW_SHOWNOACTIVATE);
			Invalidate();
			KillTimer(IDT_CLOSTTIPS);
			SetTimer(IDT_CLOSTTIPS, 3000, NULL);
		}
	}
	
}
CSize CSeekBarTip::CountSize(){
	CSize mSize ;
	
	if(m_text.IsEmpty()){
		mSize.cx = 0;
		mSize.cy = 0;
	}else{
		if(CDC* pDC = GetDC())
		{
			CDC dcMemory;
			dcMemory.CreateCompatibleDC(pDC);
			HFONT holdft = (HFONT)dcMemory.SelectObject(m_statft); 
			mSize = dcMemory.GetTextExtent(m_text); 
			dcMemory.SelectObject(holdft);

			ReleaseDC(pDC);
			mSize.cx += 9;
			mSize.cy += 7;

		}
		else
		{
			mSize.cx = 0;
			mSize.cy = 0;
			m_text.Empty();
			ShowWindow(SW_HIDE);
		}

	}
	return mSize;
}
void CSeekBarTip::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect r,cr;

	{ //New UI
		CRect rc;
		GetWindowRect(&rc);
		rc-=rc.TopLeft();



		// destroy old region
		if((HRGN)m_rgn)
		{
			m_rgn.DeleteObject();
		}

		m_rgn.CreateRoundRectRgn(0,0,rc.Width()-1,rc.Height()-1, 3,3);                 // rounded rect w/50 pixel corners


		SetWindowRgn(m_rgn,TRUE);

	}
}

void CSeekBarTip::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);
	AppSettings& s = AfxGetAppSettings();

	hdc.FillSolidRect(rcClient, s.GetColorFromTheme(_T("ToolTipBackground"), RGB(0xcf,0xcf,0xcf) ));

	HFONT holdft = (HFONT)hdc.SelectObject(m_statft);
	hdc.SetTextColor(s.GetColorFromTheme(_T("ToolTipTextColor"),0x121212));
	if(!m_text.IsEmpty()){ 
		//rcClient.left += 2; 
		//hdc.SetTextAlign( TA_TOP |TA_CENTER | TA_NOUPDATECP );
		rcClient.top += 2;
		rcClient.left += 4;
		::DrawText(hdc, m_text, m_text.GetLength(), rcClient,  DT_LEFT|DT_SINGLELINE| DT_TOP);

	}
	hdc.SelectObject(holdft);

}

int CSeekBarTip::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetSystemFontWithScale(&m_statft, 14.0);

	return 0;
}

void CSeekBarTip::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	//CWnd::OnClose();
}

void CSeekBarTip::OnRealClose(){
	CWnd::OnClose();
}
void CSeekBarTip::ClearStat(){
	KillTimer(IDT_DELAYOPEN);
	KillTimer(IDT_CLOSTTIPS);
	ShowWindow(SW_HIDE);
}
BOOL CSeekBarTip::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CWnd::PreTranslateMessage(pMsg);
}

void CSeekBarTip::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
	CWnd::OnMouseMove(nFlags, point);
}

void CSeekBarTip::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	
	if(nIDEvent == IDT_CLOSTTIPS ){
		KillTimer(IDT_CLOSTTIPS);
		
		ShowWindow(SW_HIDE);
	}else if(nIDEvent == IDT_DELAYOPEN){
		KillTimer(IDT_DELAYOPEN);
		SetTips(m_delayText, m_delayMove, &m_delayPoint);
	}
	CWnd::OnTimer(nIDEvent);
}

void CSeekBarTip::OnSetFocus(CWnd* pOldWnd)
{
	//CWnd::OnSetFocus(pOldWnd);

	AfxGetMainWnd()->SendMessage(WM_SETFOCUS, (WPARAM )m_hWnd, NULL);;
}
