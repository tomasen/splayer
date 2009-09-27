// PlayerFloatToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerFloatToolBar.h"


// CPlayerFloatToolBar

IMPLEMENT_DYNAMIC(CPlayerFloatToolBar, CWnd)

CPlayerFloatToolBar::CPlayerFloatToolBar()
{

}

CPlayerFloatToolBar::~CPlayerFloatToolBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerFloatToolBar, CWnd)
	ON_WM_SIZE()
	ON_WM_ENABLE()
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_SETFOCUS()
	ON_WM_SHOWWINDOW()
	ON_WM_KILLFOCUS()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CPlayerFloatToolBar message handlers



void CPlayerFloatToolBar::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnEnable(BOOL bEnable)
{
	CWnd::OnEnable(bEnable);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnClose();
}

BOOL CPlayerFloatToolBar::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CWnd::PreTranslateMessage(pMsg);
}

void CPlayerFloatToolBar::OnMove(int x, int y)
{
	CWnd::OnMove(x, y);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
}

void CPlayerFloatToolBar::OnRealClose()
{
	CWnd::OnClose();
}
int CPlayerFloatToolBar::GetUIHeight(){
	return 100;
}
void CPlayerFloatToolBar::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnTimer(nIDEvent);
}
