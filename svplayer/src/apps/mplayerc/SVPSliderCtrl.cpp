// SVPSliderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SVPSliderCtrl.h"
#include "MainFrm.h"
#include "../../svplib/svplib.h"


// CSVPSliderCtrl

IMPLEMENT_DYNAMIC(CSVPSliderCtrl, CSliderCtrl)

CSVPSliderCtrl::CSVPSliderCtrl() :
m_bHOZ(0),
m_style(0)
{

}

CSVPSliderCtrl::~CSVPSliderCtrl()
{
}


BEGIN_MESSAGE_MAP(CSVPSliderCtrl, CSliderCtrl)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CSVPSliderCtrl message handlers



void CSVPSliderCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);

	//hdc.FillSolidRect(rcClient, colorBackGround);
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

	if(m_bHOZ){
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
	if (CSliderCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(imgTM.IsEmpty())
		imgTM = L"VOLUME_TM.BMP";

	
	//	imgTBG = L"VOLUME_BG.BMP";
	//if(!imgTBG.IsEmpty())
	//	m_btnVolBG =  new CSUIButton(L"VOLUME.BMP" , ALIGN_TOPRIGHT, CRect(0 , 0, 0, 0)  , TRUE) ;
	//m_btnList.AddTail(m_btnVolBG);

	
	m_btnVolTm = new CSUIButton(imgTM , ALIGN_TOPRIGHT, CRect(0 , 0, 0,0)  , FALSE, 0, FALSE );
	//m_btnList.AddTail( m_btnVolTm );


	return 0;
}

BOOL CSVPSliderCtrl::OnEraseBkgnd(CDC* pDC)
{

	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r, colorBackGround);
	return TRUE;
}
