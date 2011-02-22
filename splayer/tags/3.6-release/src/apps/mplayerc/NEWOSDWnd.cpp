// NEWOSDWnd.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "NEWOSDWnd.h"

#include "ChildView.h"
#include "MainFrm.h"
#include "../../svplib/svplib.h"

// CNEWOSDWnd

IMPLEMENT_DYNAMIC(CNEWOSDWnd, CWnd)

CNEWOSDWnd::CNEWOSDWnd()
{
	
}

CNEWOSDWnd::~CNEWOSDWnd()
{
}


BEGIN_MESSAGE_MAP(CNEWOSDWnd, CWnd)
	ON_WM_GETMINMAXINFO()
	ON_WM_NCCREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_CREATE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_TIMER()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_ENABLE()
	ON_WM_CLOSE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()



// CNEWOSDWnd message handlers



void CNEWOSDWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnGetMinMaxInfo(lpMMI);
}

BOOL CNEWOSDWnd::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CWnd::OnNcCreate(lpCreateStruct))
		return FALSE;

	// TODO:  Add your specialized creation code here

	return TRUE;
}

void CNEWOSDWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

int CNEWOSDWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetSystemFontWithScale(&m_statft, 14.0);

	return 0;
}

void CNEWOSDWnd::OnMove(int x, int y)
{
	CWnd::OnMove(x, y);

	// TODO: Add your message handler code here
}

void CNEWOSDWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	//AppSettings& s = AfxGetAppSettings();

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

void CNEWOSDWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	// TODO: Add your message handler code here
}

void CNEWOSDWnd::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch(nIDEvent){
		case IDT_HIDE:
			KillTimer(IDT_HIDE);
			
			if(m_sziOsd.GetCount()){
				int minTime = 0;
				int minIdx = -1;
				int curTime = time(NULL);
				
				for(int i = (m_sziOsd.GetCount() - 1); i >=0; i--){
					int thisTime = m_sziOsd.GetAt(i);
					if( thisTime < curTime){
						m_sziOsd.RemoveAt(i);
						m_szaOsd.RemoveAt(i);
					}
				}

				for(int i = 0; i < m_sziOsd.GetCount(); i++){
					int thisTime = m_sziOsd.GetAt(i);
					if( thisTime < curTime)
						continue;
					

					if(minTime == 0){
						minTime = thisTime;
						minIdx = i;
					}else if(thisTime < minTime){
						minTime = thisTime;
						minIdx = i;
					}
				}

				if(minIdx >= 0){
					CString szOsd = m_szaOsd.GetAt(minIdx);
					if( m_osdStr != szOsd && (m_sziOsd.GetAt(minIdx) - curTime)  > 1){
						ShowWindow(SW_SHOWNOACTIVATE);
						SetTimer(IDT_HIDE , (m_sziOsd.GetAt(minIdx) - curTime) * 1000, NULL);
						CountSize();
						Invalidate();
					}else{
						SetTimer(IDT_HIDE, 1000, NULL);
					}
				}else{
					mSize = CSize(0,0);
					m_osdStr.Empty();
					ShowWindow(SW_HIDE);
				}

				
			}else{
				mSize = CSize(0,0);
				m_osdStr.Empty();
				ShowWindow(SW_HIDE);
			}
			
			break;
	}
	CWnd::OnTimer(nIDEvent);
}

BOOL CNEWOSDWnd::PreTranslateMessage(MSG* pMsg)
{
	CWnd* pParent = GetParent();
	if(pParent ){
		if(pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MYMOUSELAST)
		{
			CPoint p(pMsg->lParam);
			::MapWindowPoints(pMsg->hwnd, pParent->m_hWnd, &p, 1);
			pParent->PostMessage(pMsg->message, pMsg->wParam, MAKELPARAM(p.x, p.y));
			return TRUE;
		}else{
			//pParent->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
			//return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void CNEWOSDWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanged(lpwndpos);

	// TODO: Add your message handler code here
}

void CNEWOSDWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	AppSettings& s = AfxGetAppSettings();
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);
	hdc.FillSolidRect(rcClient,  s.GetColorFromTheme(_T("OSDBackGround"), RGB(0x3f,0x3f,0x3f) ) );

	HFONT holdft = (HFONT)hdc.SelectObject(m_statft);
	hdc.SetTextColor( s.GetColorFromTheme(_T("OSDTextColor"),0xffffff));
	if(!m_osdStr.IsEmpty()){
		rcClient.left += 2;
		::DrawText(hdc, m_osdStr, m_osdStr.GetLength(), rcClient,  DT_LEFT|DT_SINGLELINE| DT_VCENTER);

	}
	hdc.SelectObject(holdft);
	
}
void CNEWOSDWnd::CountSize(){
	int oldCX = mSize.cx;
	if(m_osdStr.IsEmpty()){
		mSize.cx = 0;

	}else{
		if(CDC* pDC = GetDC())
		{
			CDC dcMemory;
			dcMemory.CreateCompatibleDC(pDC);
			HFONT holdft = (HFONT)dcMemory.SelectObject(m_statft); 
			mSize = dcMemory.GetTextExtent(m_osdStr); 
			dcMemory.SelectObject(holdft);
			
			ReleaseDC(pDC);
			mSize.cx += 4;

		}
		else
		{
			mSize.cx = 0;
			m_osdStr.Empty();
			ShowWindow(SW_HIDE);
		}

	}
	
	
}     
int CNEWOSDWnd::SendOSDMsg(CString szMsg, int lTime ){

	KillTimer(IDT_HIDE);
	
	m_osdStr = szMsg;

	if(m_osdStr.IsEmpty()){
		ShowWindow(SW_HIDE);
	}else{
		ShowWindow(SW_SHOWNOACTIVATE);
		SetTimer(IDT_HIDE , 2000, NULL);
		CountSize();
		Invalidate();
		m_szaOsd.Add(szMsg);
        time_t lastTime = time(NULL);
//         if(m_sziOsd.GetCount() > 0){
//             lastTime = max(lastTime , m_sziOsd[m_sziOsd.GetCount()-1]);
//         }

		m_sziOsd.Add(lastTime+lTime/1000);
	}
	
	return 0;
}
void CNEWOSDWnd::OnNcPaint()
{
	// TODO: Add your message handler code here
	// Do not call CWnd::OnNcPaint() for painting messages
}

BOOL CNEWOSDWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r, 0x00000000);
	return TRUE;//CWnd::OnEraseBkgnd(pDC);
}

void CNEWOSDWnd::OnEnable(BOOL bEnable)
{
	//if(!bEnable){
		//ShowWindow(SW_HIDE);
		return;
	//}
	CWnd::OnEnable(bEnable);

	
}

void CNEWOSDWnd::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	//CWnd::OnClose();
}

void CNEWOSDWnd::OnRealClose(){
	__super::OnClose();
}
void CNEWOSDWnd::OnSetFocus(CWnd* pOldWnd)
{
	//CWnd::OnSetFocus(pOldWnd);

	AfxGetMainWnd()->SendMessage(WM_SETFOCUS, (WPARAM )m_hWnd, NULL);
}
