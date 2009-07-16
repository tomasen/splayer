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

IMPLEMENT_DYNAMIC(CPlayerToolTopBar, CToolBar)

CPlayerToolTopBar::CPlayerToolTopBar():
m_hovering(0),
m_pbtnList(&m_btnList)
{

}

CPlayerToolTopBar::~CPlayerToolTopBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerToolTopBar, CToolBar)
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
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}
void CPlayerToolTopBar::ReCalcBtnPos(){
	CRect rc;
	GetWindowRect(&rc);
	m_btnList.OnSize( rc);
}
BOOL CPlayerToolTopBar::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	return CToolBar::PreCreateWindow(cs);
}

void CPlayerToolTopBar::OnMove(int x, int y)
{
	CToolBar::OnMove(x, y);

	// TODO: Add your message handler code here
}

void CPlayerToolTopBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CToolBar::OnPaint() for painting messages
}

void CPlayerToolTopBar::OnSize(UINT nType, int cx, int cy)
{
	CToolBar::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}

BOOL CPlayerToolTopBar::Create(CWnd* pParentWnd, DWORD dwStyle , UINT nID)
{
	// TODO: Add your specialized code here and/or call the base class

	return CToolBar::Create(pParentWnd, dwStyle, nID);
}

CSize CPlayerToolTopBar::CalcFixedLayout(BOOL bStretch,BOOL bHorz ){


	CSize size( 32767, 16 * m_nLogDPIY / 96 );

	if ( CWnd* pParent = AfxGetMainWnd() )
	{
		CRect rc;
		pParent->GetWindowRect( &rc );
		size.cx = rc.Width() - 2;
	}

	CRect rc;
	GetWindowRect(&rc);
	m_btnList.OnSize( rc);

	return size;



	//return __super::CalcFixedLayout(bStretch,bHorz);
}
BOOL CPlayerToolTopBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CToolBar::OnEraseBkgnd(pDC);
}

void CPlayerToolTopBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnLButtonUp(nFlags, point);
}

void CPlayerToolTopBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnLButtonDown(nFlags, point);
}

void CPlayerToolTopBar::OnNcPaint()
{
	// TODO: Add your message handler code here
	// Do not call CToolBar::OnNcPaint() for painting messages
}

void CPlayerToolTopBar::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnTimer(nIDEvent);
}

void CPlayerToolTopBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnMouseMove(nFlags, point);
}

INT_PTR CPlayerToolTopBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// TODO: Add your specialized code here and/or call the base class

	return CToolBar::OnToolHitTest(point, pTI);
}
