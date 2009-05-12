
#include "SUIButton.h"

CSUIButton::CSUIButton(LPCTSTR szBmpName , CRect marginTownd , CSUIButton * relativeto ) : 
m_stat(0)
{
	m_marginTownd  = marginTownd;
	m_relativeto = relativeto;
	this->LoadImage(szBmpName);
}

void CSUIButton::OnSize(CRect WndRect)
{
	m_rcHitest = CRect ( WndRect.right - m_btnSize.cx - m_marginTownd.right, WndRect.top + m_marginTownd.top,
		WndRect.right-m_marginTownd.right, WndRect.top+ m_marginTownd.top+m_btnSize.cy);


}
void CSUIButton::LoadImage(LPCTSTR szBmpName){

	m_bitmap.Attach((HBITMAP)::LoadImage(GetModuleHandle(NULL), szBmpName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_CREATEDIBSECTION));
	PreMultiplyBitmap(m_bitmap);
	m_btnSize = m_bitmap.GetBitmapDimension();
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

