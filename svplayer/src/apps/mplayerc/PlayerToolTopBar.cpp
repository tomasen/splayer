// PlayerToolTopBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerToolTopBar.h"


// PlayerToolTopBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerToolTopBar.h"


// CPlayerToolTopBar

IMPLEMENT_DYNAMIC(CPlayerToolTopBar, CWnd)

CPlayerToolTopBar::CPlayerToolTopBar():
m_hovering(0),
m_pbtnList(&m_btnList)
{

}

CPlayerToolTopBar::~CPlayerToolTopBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerToolTopBar, CWnd)
	ON_WM_CREATE()
	ON_WM_MOVE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCPAINT()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnTtnNeedText)
	ON_WM_ENABLE()
END_MESSAGE_MAP()



// CPlayerToolTopBar message handlers

BOOL CPlayerToolTopBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message){

	CPoint pt;
	::GetCursorPos (&pt);
	ScreenToClient (&pt);

	if(m_nItemToTrack){	
		SetCursor(cursorHand );
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, 0, 0);
}

BOOL CPlayerToolTopBar::OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(id);

	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	UINT_PTR nID = pNMHDR->idFrom;
	BOOL bRet = FALSE;


	// idFrom is actually the HWND of the tool
	CString toolTip;
	switch(nID){
				case ID_SUBDELAYDEC:
					toolTip = _T("¼õÉÙ×ÖÄ»ÑÓÊ±");
					break;
				
	}


	if(!toolTip.IsEmpty()){
		pTTT->lpszText = toolTip.GetBuffer();
		pTTT->hinst = AfxGetResourceHandle();
		bRet = TRUE;
	}



	*pResult = 0;

	return bRet;
}
int CPlayerToolTopBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	cursorHand = ::LoadCursor(NULL, IDC_HAND);

	GetSystemFontWithScale(&m_statft, 14.0);

	CDC ScreenDC;
	ScreenDC.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
	m_nLogDPIY = ScreenDC.GetDeviceCaps(LOGPIXELSY);


	EnableToolTips(TRUE);

	return 0;
}
void CPlayerToolTopBar::ReCalcBtnPos(){
	CRect rc;
	GetWindowRect(&rc);
	m_btnList.OnSize( rc);
}

void CPlayerToolTopBar::OnMove(int x, int y)
{
	__super::OnMove(x, y);

	// TODO: Add your message handler code here
}

void CPlayerToolTopBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);

	CRect rc;
	GetWindowRect(&rc);

	CRect rcBottomSqu = rcClient;
	rcBottomSqu.top = rcBottomSqu.bottom - 1;
	//hdc.FillSolidRect(rcBottomSqu, NEWUI_COLOR_BG);

	CRect rcUpperSqu = rcClient;
	rcUpperSqu.bottom--;
	//rcUpperSqu.right--;
	hdc.FillSolidRect(rcUpperSqu, RGB(61,65,69));

	hdc.FillSolidRect(rcBottomSqu, RGB(89,89,89));

	//rcBottomSqu = rcClient;
	//rcBottomSqu.left = rcBottomSqu.right - 1;
	//hdc.FillSolidRect(rcBottomSqu, RGB(89,89,89));
}

void CPlayerToolTopBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}


BOOL CPlayerToolTopBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return __super::OnEraseBkgnd(pDC);
}

void CPlayerToolTopBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	__super::OnLButtonUp(nFlags, point);
}

void CPlayerToolTopBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	__super::OnLButtonDown(nFlags, point);
}

void CPlayerToolTopBar::OnNcPaint()
{
	// TODO: Add your message handler code here
	// Do not call CToolBar::OnNcPaint() for painting messages
}

void CPlayerToolTopBar::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	__super::OnTimer(nIDEvent);
}

void CPlayerToolTopBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	__super::OnMouseMove(nFlags, point);
}

INT_PTR CPlayerToolTopBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// TODO: Add your specialized code here and/or call the base class

	return __super::OnToolHitTest(point, pTI);
}

void CPlayerToolTopBar::OnEnable(BOOL bEnable)
{
	//CWnd::OnEnable(bEnable);

	return;
	// TODO: Add your message handler code here
}
