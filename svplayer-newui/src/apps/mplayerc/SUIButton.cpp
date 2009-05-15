
#include "SUIButton.h"

CSUIButton::CSUIButton(LPCTSTR szBmpName , int iAlign, CRect marginTownd , CSUIButton * relativeto ) : 
m_stat(0)
{
	m_marginTownd  = marginTownd;
	m_relativeto = relativeto;
	m_iAlign = iAlign;
	this->LoadImage(szBmpName);
}

LONG CSUIButton::CalcRealMargin(LONG Mlen, LONG bW, LONG wW)
{
	if(Mlen >= 0){
		return Mlen;
	}
	else
	{
		MLen = -MLen;
		return ( wW * MLen / 100) - bW / 2;
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
void CSUIButton::LoadImage(LPCTSTR szBmpName){

	m_bitmap.Attach((HBITMAP)::LoadImage(GetModuleHandle(NULL), szBmpName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_CREATEDIBSECTION));
	PreMultiplyBitmap(m_bitmap);
	m_btnSize = m_bitmap.GetBitmapDimension();
	m_btnSize.cy = m_btnSize.cy / 4;
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

