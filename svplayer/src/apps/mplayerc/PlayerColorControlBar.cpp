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


void CPlayerColorControlBar::Relayout()
{
	
	Invalidate();
}


BOOL CPlayerColorControlBar::OnEraseBkgnd(CDC* pDC)
{
	for(CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow())
	{
		if(!pChild->IsWindowVisible()) continue;

		CRect r;
		pChild->GetClientRect(&r);
		pChild->MapWindowPoints(this, &r);
		pDC->ExcludeClipRect(&r);
	}

	CRect r;
	GetClientRect(&r);

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());


	if(pFrame->m_pLastBar != this || pFrame->m_fFullScreen)
		r.InflateRect(0, 0, 0, 1);


	r.InflateRect(1, 1, 1, 0);

	pDC->FillSolidRect(&r, 0);

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
	return CDialogBar::Create(pParentWnd, IDD_PLAYERCOLORCONTROLSBAR, WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM|CBRS_FLOATING, IDD_PLAYERCOLORCONTROLSBAR);
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

int CPlayerColorControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	r.SetRectEmpty();


	Relayout();

	return 0;
}
