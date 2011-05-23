#include "stdafx.h"
#include "mplayerc.h"
#include "SVPSubVoteControlBar.h"
#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CSVPSubVoteControlBar, CSVPDialog)

CSVPSubVoteControlBar::CSVPSubVoteControlBar(void)
: m_nLogDPIY(96)
{
}

CSVPSubVoteControlBar::~CSVPSubVoteControlBar(void)
{
}

BEGIN_MESSAGE_MAP(CSVPSubVoteControlBar, CSVPDialog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CSVPSubVoteControlBar::InitSettings()
{

}

CSize CSVPSubVoteControlBar::getSizeOfWnd()
{

	return CSize( (5+ 25*MAX_EQ_BAND + 70) * m_nLogDPIY / 96 , 120 * m_nLogDPIY / 96 );

}
int CSVPSubVoteControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSVPDialog::OnCreate(lpCreateStruct) == -1)
		return -1;


	CRect r;
	GetClientRect(r);

	AppSettings& s = AfxGetAppSettings();

	GetSystemFontWithScale(&m_font );

	//cb_reset.Create( ResStr(IDS_COLOR_CONTROL_BUTTON_RESET), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER, r , this, IDC_BUTTONRESETEQCONTROL);
	//cb_reset.SetFont(&m_font);

	return 0;
}


void CSVPSubVoteControlBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Relayout();
}


void CSVPSubVoteControlBar::Relayout()
{

	CRect r;
	GetClientRect(r);

	AppSettings& s = AfxGetAppSettings();

	

	Invalidate();

}


void CSVPSubVoteControlBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CSVPDialog::OnMouseMove(nFlags, point);
}
