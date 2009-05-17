
#include "SUIButton.h"

CSUIButton::CSUIButton(LPCTSTR szBmpName , int iAlign, CRect marginTownd , CSUIButton * relativeto 
					   , BOOL bNotButton, UINT htMsgID, BOOL bHide) : 
m_stat(0)
{
	m_NotButton = bNotButton;
	m_marginTownd  = marginTownd;
	m_relativeto = relativeto;
	m_iAlign = iAlign;
	m_htMsgID = htMsgID;
	this->LoadImage(szBmpName);
	m_szBmpName = szBmpName;
	m_hide = bHide;
}

LONG CSUIButton::CalcRealMargin(LONG Mlen, LONG bW, LONG wW)
{
	if(Mlen >= 0){
		return Mlen;
	}
	else
	{
		Mlen = -Mlen;
		return ( wW * Mlen / 100) - bW / 2;
	}
}
BOOL CSUIButton::OnHitTest(CPoint pt , BOOL bLBtnDown){
	if(m_hide || m_NotButton){
		return FALSE;
	}
	int old_stat = m_stat;
	if (m_rcHitest.PtInRect(pt) && bLBtnDown) m_stat = 2; else if (m_rcHitest.PtInRect(pt)) m_stat = 1; else m_stat = 0;
	if(m_stat == old_stat){
		return FALSE;
	}else{
		return TRUE; //require redraw
	}
	
}
void CSUIButton::OnSize(CRect WndRect)
{

	LONG left = CalcRealMargin(m_marginTownd.left , m_btnSize.cx , WndRect.Width());
	LONG top = CalcRealMargin(m_marginTownd.top , m_btnSize.cy , WndRect.Height());
	LONG right = CalcRealMargin(m_marginTownd.right , m_btnSize.cx , WndRect.Width());
	LONG bottom =  CalcRealMargin(m_marginTownd.bottom , m_btnSize.cy , WndRect.Height());
	
	switch (m_iAlign){
		case ALIGN_TOPLEFT:
			m_rcHitest = CRect ( WndRect.left + left,
								WndRect.top + top,
								WndRect.left + m_btnSize.cx + left,
								WndRect.top+ top+m_btnSize.cy);
			
			break;
		case ALIGN_TOPRIGHT:
			m_rcHitest = CRect ( WndRect.right - m_btnSize.cx - right,
								WndRect.top + top,
								WndRect.right-right,
								WndRect.top+ top+m_btnSize.cy);

			break;
		case ALIGN_BOTTOMLEFT:
			m_rcHitest = CRect ( WndRect.left + left,
								WndRect.bottom - m_btnSize.cy - bottom,
								WndRect.left + m_btnSize.cx + left,
								WndRect.bottom - bottom);
			
			break;
		case ALIGN_BOTTOMRIGHT:
			m_rcHitest = CRect ( WndRect.right - m_btnSize.cx - right,
								WndRect.bottom - m_btnSize.cy - bottom,
								WndRect.right-right,
								WndRect.bottom - bottom);
			
			break;
	}
	

}
void CSUIButton::OnPaint(CMemoryDC *hDC){
	if(m_hide) return;
	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};//
	CDC dcBmp;
	dcBmp.CreateCompatibleDC(hDC);
	dcBmp.SelectObject(m_bitmap);
	hDC->AlphaBlend(m_rcHitest.left, m_rcHitest.top, m_rcHitest.Width(), m_rcHitest.Height(),
		&dcBmp, 0, m_btnSize.cy * m_stat, m_btnSize.cx, m_btnSize.cy, bf);
	
}
void CSUIButton::LoadImage(LPCTSTR szBmpName){

	m_bitmap.Attach((HBITMAP)::LoadImage(GetModuleHandle(NULL), szBmpName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_CREATEDIBSECTION));
	PreMultiplyBitmap(m_bitmap);
	m_btnSize = m_bitmap.GetBitmapDimension();
	if(!m_NotButton){
		m_btnSize.cy = m_btnSize.cy / 4;
	}
}

void CSUIButton::PreMultiplyBitmap( CBitmap& bmp )
{

	BITMAP bm;
	bmp.GetBitmap(&bm);
	for (int y=0; y<bm.bmHeight; y++)
	{
		BYTE * pPixel = (BYTE *) bm.bmBits + bm.bmWidth * 4 * y;
		for (int x=0; x<bm.bmWidth; x++)
		{
			pPixel[0] = pPixel[0] * pPixel[3] / 255; 
			pPixel[1] = pPixel[1] * pPixel[3] / 255; 
			pPixel[2] = pPixel[2] * pPixel[3] / 255; 
			pPixel += 4;
		}
	}
}


/*CSUIBtnList*/
CSUIBtnList::CSUIBtnList()
{
}

CSUIBtnList::~CSUIBtnList()
{
}
void CSUIBtnList::SetHideStat(POSITION pos, BOOL bHide){
	CSUIButton* cBtn = GetAt(pos);
	if(cBtn)
		cBtn->m_hide = bHide;
}
void CSUIBtnList::SetHideStat(UINT iMsgID, BOOL bHide){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( iMsgID == cBtn->m_htMsgID ){
			SetHideStat(pos,  bHide);
			break;
		}
	}

}
void CSUIBtnList::SetHideStat(LPCTSTR szBmpName, BOOL bHide){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( cBtn->m_szBmpName.Compare(szBmpName) == 0 ){
			SetHideStat(pos,  bHide);
			break;
		}
	}
}

UINT CSUIBtnList::OnHitTest(CPoint pt ){
	SHORT bLBtnDown = GetAsyncKeyState(VK_LBUTTON);  
	POSITION pos = GetHeadPosition();
	UINT iMsg = 0;
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( cBtn->OnHitTest(pt , bLBtnDown) ){
			iMsg = cBtn->m_htMsgID;
		}
	}
	return iMsg;
}
void CSUIBtnList::OnSize(CRect WndRect){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		cBtn->OnSize(WndRect);
	}
}	
void CSUIBtnList::PaintAll(CMemoryDC *hDC){

	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		cBtn->OnPaint(hDC);
	}
}
