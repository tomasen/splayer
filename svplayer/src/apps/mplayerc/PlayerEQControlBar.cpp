#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerEQControlBar.h"
#include "MainFrm.h"


IMPLEMENT_DYNAMIC(CPlayerEQControlBar, CSVPDialog)

CPlayerEQControlBar::CPlayerEQControlBar(void)
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
END_MESSAGE_MAP()



// CPlayerChannelNormalizer message handlers



int CPlayerEQControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSVPDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_bgColor = 0x333333;

	CRect r;
	GetClientRect(r);

	return 0;
}

void CPlayerEQControlBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{


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

void CPlayerEQControlBar::Relayout()
{

	Invalidate();

}
