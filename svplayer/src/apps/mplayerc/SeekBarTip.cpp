// SeekBarTip.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SeekBarTip.h"


// CSeekBarTip

IMPLEMENT_DYNAMIC(CSeekBarTip, CWnd)

CSeekBarTip::CSeekBarTip()
{

}

CSeekBarTip::~CSeekBarTip()
{
}


BEGIN_MESSAGE_MAP(CSeekBarTip, CWnd)
	ON_WM_ENABLE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()



// CSeekBarTip message handlers



void CSeekBarTip::OnEnable(BOOL bEnable)
{
	return ;
	//CWnd::OnEnable(bEnable);

	// TODO: Add your message handler code here
}
CSize CSeekBarTip::CountSize(){
	CSize mSize ;
	
	if(m_text.IsEmpty()){
		mSize.cx = 0;
		mSize.cy = 0;
	}else{
		if(CDC* pDC = GetDC())
		{
			CDC dcMemory;
			dcMemory.CreateCompatibleDC(pDC);
			HFONT holdft = (HFONT)dcMemory.SelectObject(m_statft); 
			mSize = dcMemory.GetTextExtent(m_text); 
			dcMemory.SelectObject(holdft);

			ReleaseDC(pDC);
			mSize.cx += 8;
			mSize.cy += 6;

		}
		else
		{
			mSize.cx = 0;
			mSize.cy = 0;
			m_text.Empty();
			ShowWindow(SW_HIDE);
		}

	}
	return mSize;
}
void CSeekBarTip::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect r,cr;

	{ //New UI
		CRect rc;
		GetWindowRect(&rc);
		rc-=rc.TopLeft();



		// destroy old region
		if((HRGN)m_rgn)
		{
			m_rgn.DeleteObject();
		}

		m_rgn.CreateRoundRectRgn(0,0,rc.Width()-1,rc.Height()-1, 3,3);                 // rounded rect w/50 pixel corners


		SetWindowRgn(m_rgn,TRUE);

	}
}

void CSeekBarTip::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);
	hdc.FillSolidRect(rcClient, RGB(0xcf,0xcf,0xcf) );

	HFONT holdft = (HFONT)hdc.SelectObject(m_statft);
	hdc.SetTextColor(0x121212);
	if(!m_text.IsEmpty()){
		//rcClient.left += 2;
		::DrawText(hdc, m_text, m_text.GetLength(), rcClient,  DT_CENTER|DT_SINGLELINE| DT_VCENTER);

	}
	hdc.SelectObject(holdft);

}

int CSeekBarTip::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetSystemFontWithScale(&m_statft, 14.0);

	return 0;
}

void CSeekBarTip::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	//CWnd::OnClose();
}

void CSeekBarTip::OnRealClose(){
	CWnd::OnClose();
}