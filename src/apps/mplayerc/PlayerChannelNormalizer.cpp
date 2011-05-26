

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerChannelNormalizer.h"
#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CPlayerChannelNormalizer, CSVPDialog)

CPlayerChannelNormalizer::CPlayerChannelNormalizer(void)
: m_nLogDPIY(96)
{
}

CPlayerChannelNormalizer::~CPlayerChannelNormalizer(void)
{
}

BEGIN_MESSAGE_MAP(CPlayerChannelNormalizer, CSVPDialog)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CPlayerChannelNormalizer message handlers



int CPlayerChannelNormalizer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSVPDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	GetSystemFontWithScale(&m_font , 12);

	CRect r;
	GetClientRect(r);
	for(int i = 0;i < MAX_INPUT_CHANNELS; i++){
		
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

void CPlayerChannelNormalizer::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	AppSettings& s = AfxGetAppSettings();

	
		int i = pScrollBar->GetDlgCtrlID()  - IDC_SLIDER1;

		if(i >= 0 && i < m_nChannels){
			KillTimer(TIMER_SETCHANNELMAPING);
			int pos = csl_trans[i].GetPos();

			s.pSpeakerToChannelMapOffset[m_nChannels-1][i] = -float(pos-50) / 50;

// 			CString szMsg;
// 			szMsg.Format(L"%d  %d %f" , i ,pos , s.pSpeakerToChannelMapOffset[m_nChannels-1][i] );
// 			((CMainFrame*)AfxGetMainWnd())->SendStatusMessage(szMsg , 5000);

			SetTimer(TIMER_SETCHANNELMAPING, 700, NULL);

		}
		
		
}

void CPlayerChannelNormalizer::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	__super::OnHScroll( nSBCode,  nPos,  pScrollBar);

}

void CPlayerChannelNormalizer::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Relayout();
}
bool CPlayerChannelNormalizer::InitChannels()
{

	m_nChannels = 6;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if(pFrame ){
		m_nLogDPIY = pFrame->m_nLogDPIY;
		if( pFrame->IsSomethingLoaded()){
			m_pASF = FindFilter(__uuidof(CAudioSwitcherFilter), pFrame->pGB);

			if(m_pASF)
				m_nChannels = m_pASF->GetNumberOfInputChannels();		
		}
		
	}
	Relayout();

	AppSettings& s = AfxGetAppSettings();

	for(int i = 0;i < m_nChannels; i++){
		csl_trans[i].SetPos( min( 100, max( 0 , -s.pSpeakerToChannelMapOffset[m_nChannels-1][i] * 50 + 50 )) );
	}

	return false;
}
CSize CPlayerChannelNormalizer::getSizeOfWnd()
{
	
	return CSize( (5+ 25*m_nChannels+8) * m_nLogDPIY / 96 , 120 * m_nLogDPIY / 96 );
}
void CPlayerChannelNormalizer::Relayout()
{
	CRect r;
	GetClientRect(r);

	for(int i = 0;i < MAX_INPUT_CHANNELS; i++){

		if( i < m_nChannels){
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
			cslLabel[i].SetWindowText(GetChannelDescByID(i));
			cslLabel[i].MoveWindow(&r3);
			cslLabel[i].ShowWindow(SW_SHOW);
		}else{
			cslLabel[i].ShowWindow(SW_HIDE);
			csl_trans[i].ShowWindow(SW_HIDE);
		}
		
	}
	

	Invalidate();

}

CString CPlayerChannelNormalizer::GetChannelDescByID(int iChannelID){

	switch(iChannelID){
		case 0:
			return ResStr(IDS_CHANNEL_DESCBYID_0);
			break;
		case 1:
			return ResStr(IDS_CHANNEL_DESCBYID_1);
			break;
		case 2:
			return ResStr(IDS_CHANNEL_DESCBYID_2);
			break;
		case 3:
			return ResStr(IDS_CHANNEL_DESCBYID_3);
			break;
		case 4:
			return ResStr(IDS_CHANNEL_DESCBYID_4);
			break;
		case 5:
			if(iChannelID == (m_nChannels - 1) )
				return ResStr(IDS_CHANNEL_DESCBYID_LEF);
			
			return ResStr(IDS_CHANNEL_DESCBYID_5);
			break;
		case 6:
			return ResStr(IDS_CHANNEL_DESCBYID_6);
			break;
		case 7:
			if(iChannelID == (m_nChannels - 1) )
				return ResStr(IDS_CHANNEL_DESCBYID_LEF);

			return ResStr(IDS_CHANNEL_DESCBYID_7);
			break;
		default:
			CString szTmp;
			szTmp.Format( ResStr(IDS_CHANNEL_DESCBYID_FORMAT), iChannelID+1);
			return szTmp;
			break;

	}

}
void CPlayerChannelNormalizer::OnRButtonUp(UINT nFlags, CPoint point)
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
				AppSettings& s = AfxGetAppSettings();
				for(int i = 0;i < m_nChannels; i++){
					csl_trans[i].SetPos( 50 );
					s.pSpeakerToChannelMapOffset[m_nChannels-1][i] = 0;
				}
				if(m_pASF)
					m_pASF->SetSpeakerChannelConfig( -1, s.pSpeakerToChannelMap2 , s.pSpeakerToChannelMapOffset, -1, s.iSS);

			}
			break;
	}
}

void CPlayerChannelNormalizer::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CSVPDialog::OnMouseMove(nFlags, point);
}

void CPlayerChannelNormalizer::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CSVPDialog::OnLButtonDown(nFlags, point);
}

void CPlayerChannelNormalizer::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == TIMER_SETCHANNELMAPING){
		AppSettings& s = AfxGetAppSettings();

		if(m_pASF)
			m_pASF->SetSpeakerChannelConfig( -1, s.pSpeakerToChannelMap2 , s.pSpeakerToChannelMapOffset, -1,s.iSS);

	}
	CSVPDialog::OnTimer(nIDEvent);
}
