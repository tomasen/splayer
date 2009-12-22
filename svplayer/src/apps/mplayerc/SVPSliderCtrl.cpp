// SVPSliderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SVPSliderCtrl.h"
#include "MainFrm.h"
#include "SVPDialog.h"
#include "../../svplib/svplib.h"


// CSVPSliderCtrl

IMPLEMENT_DYNAMIC(CSVPSliderCtrl, CSliderCtrl)

CSVPSliderCtrl::CSVPSliderCtrl() :
m_bVertical(0),
m_style(0),
colorBackGround(0)
{
	AppSettings& s = AfxGetAppSettings();
	colorBackGround = s.GetColorFromTheme(_T("FloatDialogBG"), 0x00);
	m_btnVolTm = NULL;
	m_btnVolBG = NULL;
}

CSVPSliderCtrl::~CSVPSliderCtrl()
{
	if (m_btnVolTm)
		delete m_btnVolTm;
	if (m_btnVolBG)
		delete m_btnVolBG;
}


BEGIN_MESSAGE_MAP(CSVPSliderCtrl, CSliderCtrl)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CSVPSliderCtrl message handlers



void CSVPSliderCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);

	hdc.FillSolidRect(rcClient, colorBackGround);
	CRect rc;
	GetWindowRect(&rc);

	//hdc.FillSolidRect(rcClient, colorBackGround);
//	m_btnVolTm->OnSize(rc);

	CRect rcOldThumb;
	GetThumbRect(rcOldThumb);
	
	//hdc.FillSolidRect(rcOldThumb, colorBackGround);

	CRect rcLine;
	GetChannelRect( rcLine);
	//hdc.FillSolidRect(rcLine, colorBackGround);
/*
	switch(m_style){
		default:
			CRect rcLine(rcClient);
			if(m_bHOZ){
				int marg = m_btnVolTm->m_btnSize.cy;
				rcLine.DeflateRect(rcClient.Width()/2 -1,marg,rcClient.Width()/2 ,marg);
			}else{
				int marg = m_btnVolTm->m_btnSize.cx;
				rcLine.DeflateRect(marg,rcClient.Height()/2-2 ,marg,rcClient.Height()/2 +1 );
			}
			
			hdc.FillSolidRect(rcLine, 0xa5a5a5);
			break;
	}*/

	if(m_bVertical){
		rcLine.left ^= rcLine.top ^= rcLine.left ^= rcLine.top; // swap left and top values
		rcLine.right ^= rcLine.bottom ^= rcLine.right ^= rcLine.bottom; // swap right and bottom values
		rcLine.top+=4;
		rcLine.bottom-=4;
		rcLine.right--;
		rcLine.left = rcLine.right - 1;
	}else{
		rcLine.top++;
		rcLine.bottom = rcLine.top + 1;
		rcLine.left+=4;
		rcLine.right-=4;
	}
	hdc.FillSolidRect(rcLine, 0xa5a5a5);

/*
	
	m_btnVolTm->m_rcHitest.MoveToY( rcClient.Height() /2  - m_btnVolTm->m_btnSize.cy/2);
	m_btnVolTm->m_rcHitest.MoveToX( rcClient.Width() /2  - m_btnVolTm->m_btnSize.cx/2);
		if(m_bHOZ)
			m_btnVolTm->m_rcHitest.MoveToY( ( rcClient.Height() - m_btnVolTm->m_rcHitest.Height()) * GetPos() / GetRangeMax() + m_btnVolTm->m_rcHitest.Height()/2);
		else
			m_btnVolTm->m_rcHitest.MoveToX( (rcClient.Width() - m_btnVolTm->m_rcHitest.Height()) * GetPos() / GetRangeMax() + m_btnVolTm->m_rcHitest.Width()/2);
	
	//SVP_LogMsg5(_T("%d %d %d %d %d %d %d %d  Paint TM"), rc.left, rc.top, rc.Width(), rc.Height(), 
	//	 m_btnVolTm->m_rcHitest.left, m_btnVolTm->m_rcHitest.top, m_btnVolTm->m_rcHitest.Width(), m_btnVolTm->m_rcHitest.Height());
	if(m_bHOZ)
		m_btnVolTm->m_rcHitest.MoveToY( rcOldThumb.top ); //+ m_btnVolTm->m_rcHitest.Height()/2
	else
		m_btnVolTm->m_rcHitest.MoveToX( rcOldThumb.left );//+ m_btnVolTm->m_rcHitest.Width()/2
*/

	m_btnVolTm->m_rcHitest = rcOldThumb;

	m_btnVolTm->OnPaint(&hdc, CRect(0,0,0,0));

}

int CSVPSliderCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->style |= TBS_FIXEDLENGTH ;
	if (CSliderCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	if(imgTM.IsEmpty())
		imgTM = L"SLIDER_TM.BMP";

	m_bVertical = !!(lpCreateStruct->style&TBS_VERT);
	//	imgTBG = L"VOLUME_BG.BMP";
	//if(!imgTBG.IsEmpty())
	//	m_btnVolBG =  new CSUIButton(L"VOLUME.BMP" , ALIGN_TOPRIGHT, CRect(0 , 0, 0, 0)  , TRUE) ;
	//m_btnList.AddTail(m_btnVolBG);

	
	m_btnVolTm = new CSUIButton(imgTM , ALIGN_TOPRIGHT, CRect(0 , 0, 0,0)  , TRUE, 0, FALSE );
	//m_btnList.AddTail( m_btnVolTm );

	
		CSVPDialog * pDialog = dynamic_cast<CSVPDialog*>( GetParent() );
		if(pDialog)
			colorBackGround = pDialog->m_bgColor;

	return 0;
}

BOOL CSVPSliderCtrl::OnEraseBkgnd(CDC* pDC)
{

	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r, colorBackGround);
	return TRUE;
}

void CSVPSliderCtrl::OnSize(UINT nType, int cx, int cy)
{
	CSliderCtrl::OnSize(nType, cx, cy);

	//int iMin, iMax;
	//GetRange(iMin, iMax);
	if(m_bVertical){

		SetThumbLength( m_btnVolTm->m_btnSize.cy ); //* (iMax-iMin) / cy 
	}else{

		SetThumbLength( m_btnVolTm->m_btnSize.cx );// * (iMax-iMin) / cx
	}
	// TODO: Add your message handler code here
}



void CSVPSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bDragging = true;
	m_bDragChanged = false;
	SetCapture();
	SetFocus();
	if (SetThumb(point))
	{
		m_bDragChanged = true;
		PostMessageToParent(TB_THUMBTRACK);
	}
}


void CSVPSliderCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bDragging)
	{
		if (SetThumb(point))
		{
			m_bDragChanged = true;
			PostMessageToParent(TB_THUMBTRACK);
		}
	}
	else
	{
		CSliderCtrl::OnMouseMove(nFlags, point);
	}
}

void CSVPSliderCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_bDragging)
	{
		m_bDragging = false;
		::ReleaseCapture();
		if (SetThumb(point))
		{
			PostMessageToParent(TB_THUMBTRACK);
			m_bDragChanged = true;
		}
		if (m_bDragChanged)
		{
			PostMessageToParent(TB_THUMBPOSITION);
			m_bDragChanged = false;
		}
	}
	else
	{
		CSliderCtrl::OnLButtonUp(nFlags, point);
	}
}


bool CSVPSliderCtrl::SetThumb(const CPoint& point)
{
	const int nMin = GetRangeMin();
	const int nMax = GetRangeMax()+1;
	CRect rc;
	GetChannelRect(rc);
	double dPos;
	double dCorrectionFactor = 0.0;
	if (GetStyle() & TBS_VERT) 
	{
		// note: there is a bug in GetChannelRect, it gets the orientation of the rectangle mixed up
		dPos = (double)(point.y - rc.left)/(rc.right - rc.left);
	}
	else
	{
		dPos = (double)(point.x - rc.left)/(rc.right - rc.left);
	}
	// This correction factor is needed when you click inbetween tick marks
	// so that the thumb will move to the nearest one
	dCorrectionFactor = 0.5 *(1-dPos) - 0.5 *dPos;
	int nNewPos = (int)(nMin + (nMax-nMin)*dPos + dCorrectionFactor);
	const bool bChanged = (nNewPos != GetPos());
	if(bChanged)
	{
		SetPos(nNewPos);
	}
	return bChanged;
}

void CSVPSliderCtrl::PostMessageToParent(const int nTBCode) const
{
	CWnd* pWnd = GetParent();
	DWORD dwFlag = WM_HSCROLL;
	if (GetStyle() & TBS_VERT) 
		dwFlag = WM_VSCROLL;

	if(pWnd) pWnd->PostMessage(dwFlag, (WPARAM)((GetPos() << 16) | nTBCode), (LPARAM)GetSafeHwnd());
}


