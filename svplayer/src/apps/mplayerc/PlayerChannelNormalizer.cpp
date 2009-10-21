

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerChannelNormalizer.h"
#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CPlayerChannelNormalizer, CSVPDialog)

CPlayerChannelNormalizer::CPlayerChannelNormalizer(void)
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
END_MESSAGE_MAP()



// CPlayerChannelNormalizer message handlers



int CPlayerChannelNormalizer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSVPDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_bgColor = 0x333333;

	CRect r;
	GetClientRect(r);

	return 0;
}

void CPlayerChannelNormalizer::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

	
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

void CPlayerChannelNormalizer::Relayout()
{
	
	Invalidate();

}
