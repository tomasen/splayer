#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerEQControlBar.h"
#include "MainFrm.h"


IMPLEMENT_DYNAMIC(CPlayerEQControlBar, CSVPDialog)

CPlayerEQControlBar::CPlayerEQControlBar(void)
: m_nLogDPIY(96)
{
}

CPlayerEQControlBar::~CPlayerEQControlBar(void)
{
}

BEGIN_MESSAGE_MAP(CPlayerEQControlBar, CSVPDialog)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTONRESETEQCONTROL,   OnButtonReset)
END_MESSAGE_MAP()



// CPlayerChannelNormalizer message handlers



int CPlayerEQControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSVPDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	CRect r;
	GetClientRect(r);

	GetSystemFontWithScale(&m_font );

	cb_reset.Create( ResStr(IDS_COLOR_CONTROL_BUTTON_RESET), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER, r , this, IDC_BUTTONRESETEQCONTROL);
	cb_reset.SetFont(&m_font);

	for(int i = 0;i < MAX_EQ_BAND; i++){

		cslLabel[i].Create( _T("1"),WS_CHILD|SS_CENTERIMAGE , 
			r, this, IDC_STATIC1+i);
		cslLabel[i].m_dwAlign = DT_CENTER;
		cslLabel[i].SetFont(&m_font);


		csl_trans[i].Create( WS_CHILD|TBS_AUTOTICKS|TBS_VERT|TBS_NOTICKS  , r, this, IDC_SLIDER1+i);
		csl_trans[i].SetThumbLength(18);
		csl_trans[i].EnableWindow();
		csl_trans[i].SetRange(0, 100);
		csl_trans[i].SetPos(50);

	}
	return 0;
}

void CPlayerEQControlBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	AppSettings& s = AfxGetAppSettings();


	int i = pScrollBar->GetDlgCtrlID()  - IDC_SLIDER1;

	if(i >= 0 && i < MAX_EQ_BAND){
		KillTimer(TIMER_SETEQCONTROL);
		int pos = csl_trans[i].GetPos();

		s.pEQBandControlCustom[i] = -float(pos-50) / 50; // -1.0 - +1.0

		s.pEQBandControlPerset = 0;
		SetTimer(TIMER_SETEQCONTROL, 700, NULL);

	}


}

void CPlayerEQControlBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	__super::OnHScroll( nSBCode,  nPos,  pScrollBar);

}

void CPlayerEQControlBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Relayout();
}

bool CPlayerEQControlBar::InitSettings()
{

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if(pFrame ){
		m_nLogDPIY = pFrame->m_nLogDPIY;
		if( pFrame->IsSomethingLoaded()){
			m_pASF = FindFilter(__uuidof(CAudioSwitcherFilter), pFrame->pGB);

		}

	}
	Relayout();
	
	AppSettings& s = AfxGetAppSettings();

	for(int i = 0;i < MAX_EQ_BAND; i++){
		csl_trans[i].SetPos( min( 100, max( 0 , -s.pEQBandControlCustom[i] * 50 + 50 )) );
	}
	return false;
}
CSize CPlayerEQControlBar::getSizeOfWnd()
{

	return CSize( (5+ 25*MAX_EQ_BAND + 100) * m_nLogDPIY / 96 , 120 * m_nLogDPIY / 96 );

}
void CPlayerEQControlBar::Relayout()
{

	CRect r;
	GetClientRect(r);

	for(int i = 0;i < MAX_EQ_BAND; i++){

			CRect r2(r);
			r2.top += 8;
			r2.left += 10 + (25 * i);
			r2.right = r2.left + 10;
			r2.bottom -= 18;
			csl_trans[i].MoveWindow(&r2);
			csl_trans[i].ShowWindow(SW_SHOW);

			CRect r3(r);
			r3.bottom -= 6;
			r3.top = r3.bottom - 12;
			r3.left = r2.left - 2;
			r3.right = r2.right + 7;
			cslLabel[i].SetWindowText(L"*");
			cslLabel[i].MoveWindow(&r3);
			cslLabel[i].ShowWindow(SW_SHOW);
		

	}


	CRect r2(r);
	r2.right -= 35;
	r2.left = r2.right - 40;
	r2.bottom -= 20;
	r2.top = r2.bottom - 20;
	cb_reset.MoveWindow(&r2);
	cb_reset.EnableWindow(TRUE);

	Invalidate();

}

void CPlayerEQControlBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	__super::OnRButtonUp(nFlags, point);

	enum 
	{
		M_RESET=1
	};
	CMenu m;
	m.CreatePopupMenu();
	m.AppendMenu(MF_STRING|MF_ENABLED, M_RESET, ResStr(IDS_COLOR_CONTROL_BUTTON_RESET));

	ClientToScreen(&point);
	int nID = (int)m.TrackPopupMenu(TPM_LEFTBUTTON|TPM_RETURNCMD, point.x, point.y, this);
	switch(nID)
	{
	case M_RESET:
		{
			OnButtonReset();
		}
		break;
	}
}

void CPlayerEQControlBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CSVPDialog::OnMouseMove(nFlags, point);
}

void CPlayerEQControlBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CSVPDialog::OnLButtonDown(nFlags, point);
}

void CPlayerEQControlBar::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == TIMER_SETEQCONTROL){
		AppSettings& s = AfxGetAppSettings();

		if(m_pASF)
			m_pASF->SetEQControl(s.pEQBandControlPerset, s.pEQBandControlCustom);
			

	}
	CSVPDialog::OnTimer(nIDEvent);
}

void  CPlayerEQControlBar::OnButtonReset()
{
	AppSettings& s = AfxGetAppSettings();
	for(int i = 0;i < MAX_EQ_BAND; i++){
		csl_trans[i].SetPos( 50 );
		s.pEQBandControlCustom[i] = 0;
		s.pEQBandControlPerset = 0;
	}
	if(m_pASF)
		m_pASF->SetEQControl(s.pEQBandControlPerset, s.pEQBandControlCustom);


}