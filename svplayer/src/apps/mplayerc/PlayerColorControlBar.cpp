// PlayerColorControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"

#include "MainFrm.h"
#include "PlayerColorControlBar.h"


// CPlayerColorControlBar

IMPLEMENT_DYNAMIC(CPlayerColorControlBar, CDialogBar)

CPlayerColorControlBar::CPlayerColorControlBar()
{

}

CPlayerColorControlBar::~CPlayerColorControlBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerColorControlBar, CDialogBar)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CPlayerColorControlBar message handlers


int CPlayerColorControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	r.SetRectEmpty();

	csBrightLabel.Create( _T("亮度: "), WS_CHILD|WS_VISIBLE|SS_ICON, 
		r, this, IDC_STATIC);

	csConstLabel.Create( _T("对比度"), WS_CHILD|WS_VISIBLE|SS_ICON, 
		r, this, IDC_STATIC);
	Relayout();

	return 0;
}

void CPlayerColorControlBar::Relayout()
{
	CRect r, r2;
	GetClientRect(r);

	
	r2 = r;
	r2.right = r2.left + 80;
	csBrightLabel.MoveWindow(&r2);

	r2 = r;
	r2.left += (r.Width() - 50) / 2;
	r2.right = r2.left + 80;
	csConstLabel.MoveWindow(&r2);

	Invalidate();
}


BOOL CPlayerColorControlBar::OnEraseBkgnd(CDC* pDC)
{
	
	return TRUE;
}

void CPlayerColorControlBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect r;


	// Do not call CDialogBar::OnPaint() for painting messages
}

void CPlayerColorControlBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	Relayout();
}

BOOL CPlayerColorControlBar::Create(CWnd* pParentWnd)
{
	return CDialogBar::Create(pParentWnd, IDD_PLAYERCOLORCONTROLSBAR, WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM|CBRS_FLOATING, IDD_PLAYERCOLORCONTROLSBAR); //not visible default
}

BOOL CPlayerColorControlBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!CDialogBar::PreCreateWindow(cs))
		return FALSE;

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;
	cs.dwExStyle |= WS_EX_TRANSPARENT ;


	return TRUE;
}
