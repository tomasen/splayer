// TransparentControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "TransparentControlBar.h"
#include "MainFrm.h"

// CTransparentControlBar

IMPLEMENT_DYNAMIC(CTransparentControlBar, CSVPDialog)

CTransparentControlBar::CTransparentControlBar()
{

}

CTransparentControlBar::~CTransparentControlBar()
{
}


BEGIN_MESSAGE_MAP(CTransparentControlBar, CSVPDialog)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()



// CTransparentControlBar message handlers



int CTransparentControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSVPDialog::OnCreate(lpCreateStruct) == -1)
		return -1;


	CRect r;
	GetClientRect(r);

	//csl_trans.m_bVertical = true;

	csl_trans.Create( WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_VERT|TBS_NOTICKS  , r, this, IDC_SLIDER1);
	csl_trans.SetThumbLength(15);
	csl_trans.EnableWindow();
	csl_trans.SetRange(30, 0xff);
	csl_trans.SetPos(0xff);

	return 0;
}
static bool bNeedLayed = true;
void CTransparentControlBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

	CMainFrame* pMFrame = (CMainFrame*)AfxGetMainWnd();
	if (pScrollBar == GetDlgItem(IDC_SLIDER1) && pMFrame){
		int pos = csl_trans.GetPos();
		AppSettings& s = AfxGetAppSettings();

		if(pos < 0xff){
			//::SetWindowLong(this->m_hWnd , GWL_EXSTYLE, ::GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			if(bNeedLayed){
				pMFrame->ModifyStyle(0, WS_POPUP);
				pMFrame->ModifyStyleEx(  0, WS_EX_LAYERED);
				pMFrame->m_wndToolTopBar.ModifyStyleEx(  0, WS_EX_LAYERED);
				pMFrame->m_wndNewOSD.ModifyStyleEx(  0, WS_EX_LAYERED);
				pMFrame->m_wndColorControlBar.ModifyStyleEx(  0, WS_EX_LAYERED);
				bNeedLayed = false;
			}
				
				pMFrame->SetLayeredWindowAttributes( 0, pos, LWA_ALPHA);
				pMFrame->m_wndToolTopBar.SetLayeredWindowAttributes( 0, pos, LWA_ALPHA);
				pMFrame->m_wndNewOSD.SetLayeredWindowAttributes( 0, pos, LWA_ALPHA);
				pMFrame->m_wndColorControlBar.SetLayeredWindowAttributes( 0, pos, LWA_ALPHA);
			
			//ModifyStyleEx(  0, WS_EX_LAYERED);
			//SetLayeredWindowAttributes( 0, pos, LWA_ALPHA);
		}else{
			bNeedLayed = true;
			if(s.bUserAeroUI() || s.bUserAeroTitle()){
				pMFrame->SetLayeredWindowAttributes( 0, 0xff, LWA_ALPHA);
				pMFrame->m_wndToolTopBar.SetLayeredWindowAttributes( 0, 0xff, LWA_ALPHA);
				pMFrame->m_wndNewOSD.SetLayeredWindowAttributes( 0, 0xff, LWA_ALPHA);
				pMFrame->m_wndColorControlBar.SetLayeredWindowAttributes( 0, 0xff, LWA_ALPHA);
			}else{
				pMFrame->ModifyStyleEx( WS_EX_LAYERED, 0);
				pMFrame->ModifyStyle(WS_POPUP,0 );
				pMFrame->m_wndToolTopBar.ModifyStyleEx(   WS_EX_LAYERED , 0);
				pMFrame->m_wndNewOSD.ModifyStyleEx(   WS_EX_LAYERED , 0);
				pMFrame->m_wndColorControlBar.ModifyStyleEx(   WS_EX_LAYERED , 0);
			}
			
		}

	}
}

void CTransparentControlBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	__super::OnHScroll( nSBCode,  nPos,  pScrollBar);
	
}

void CTransparentControlBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Relayout();
}

void CTransparentControlBar::Relayout()
{
	CRect r, r2;
	GetClientRect(r);
	r2 = r;
	r2.top += 15;
	r2.left += r2.Width() / 2 - 5;
	r2.right -= r2.Width() / 2 - 4;
	r2.bottom -= 5;
	csl_trans.MoveWindow(&r2);

	Invalidate();

}
