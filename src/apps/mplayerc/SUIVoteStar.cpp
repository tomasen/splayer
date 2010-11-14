
#include "SUIVoteStar.h"

SUIVoteStar::SUIVoteStar(LPCTSTR szStarBmpYellow,LPCTSTR szStarBmpNull,LPCTSTR szStarBmpRed, int iStars , int defaultRate)
{
	m_iStars = iStars;
	m_Rating = defaultRate;

	m_StarBmpYellow.Attach(CSUIButton::SUILoadImage(szStarBmpYellow));
	CSUIButton::PreMultiplyBitmap(m_StarBmpYellow,m_StarBmpYellowSize , true);

	m_StarBmpNull.Attach(CSUIButton::SUILoadImage(szStarBmpNull));
	CSUIButton::PreMultiplyBitmap(m_StarBmpNull,m_StarBmpNullSize , true);

	m_StarBmpRed.Attach(CSUIButton::SUILoadImage(szStarBmpRed));
	CSUIButton::PreMultiplyBitmap(m_StarBmpRed,m_StarBmpRedSize , true);

	CountDPI();
}

SUIVoteStar::~SUIVoteStar(void)
{
}


void SUIVoteStar::CountDPI(){
	if(!nLogDPIX){
		CDC ScreenDC;
		ScreenDC.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
		nLogDPIX = ScreenDC.GetDeviceCaps(LOGPIXELSX), nLogDPIY = ScreenDC.GetDeviceCaps(LOGPIXELSY);
	}

	m_StarBmpYellowSizeOrg = m_StarBmpYellowSize;
	m_StarBmpYellowSize.cx = nLogDPIX  * m_StarBmpYellowSizeOrg.cx / 96;
	m_StarBmpYellowSize.cy = nLogDPIY * m_StarBmpYellowSizeOrg.cy / 96;

	m_StarBmpNullSizeOrg = m_StarBmpNullSize;
	m_StarBmpNullSize.cx = nLogDPIX  * m_StarBmpNullSizeOrg.cx / 96;
	m_StarBmpNullSize.cy = nLogDPIY * m_StarBmpNullSizeOrg.cy / 96;

	m_StarBmpRedSizeOrg = m_StarBmpRedSize;
	m_StarBmpRedSize.cx = nLogDPIX  * m_StarBmpRedSizeOrg.cx / 96;
	m_StarBmpRedSize.cy = nLogDPIY * m_StarBmpRedSizeOrg.cy / 96;
}

void SUIVoteStar::OnPaint(CMemoryDC *hDC, CRect rc)
{
	
	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};//
	CDC dcBmp;
	dcBmp.CreateCompatibleDC(hDC);
	for(int i = 1; i <= m_iStars; i++){
		if(i > m_Rating){
			HBITMAP holdBmp = (HBITMAP)dcBmp.SelectObject(m_StarBmpNull);
			BOOL ret = hDC->AlphaBlend(m_StarBmpNullSize.cx * (i - 1) , 0, m_StarBmpNullSize.cx, m_StarBmpNullSize.cy,
				&dcBmp, 0, 0, m_StarBmpNullSizeOrg.cx, m_StarBmpNullSizeOrg.cy, bf);
			dcBmp.SelectObject(holdBmp);
		}else if(i <= (m_iStars/2)){
			HBITMAP holdBmp = (HBITMAP)dcBmp.SelectObject(m_StarBmpRed);
			BOOL ret = hDC->AlphaBlend(m_StarBmpRedSize.cx * (i - 1) , 0, m_StarBmpRedSize.cx, m_StarBmpRedSize.cy,
				&dcBmp, 0, 0, m_StarBmpRedSizeOrg.cx, m_StarBmpRedSizeOrg.cy, bf);
			dcBmp.SelectObject(holdBmp);
		}else{
			HBITMAP holdBmp = (HBITMAP)dcBmp.SelectObject(m_StarBmpYellow);
			BOOL ret = hDC->AlphaBlend(m_StarBmpYellowSize.cx * (i - 1) , 0, m_StarBmpYellowSize.cx, m_StarBmpYellowSize.cy,
				&dcBmp, 0, 0, m_StarBmpYellowSizeOrg.cx, m_StarBmpYellowSizeOrg.cy, bf);
			dcBmp.SelectObject(holdBmp);
		}
	}
	dcBmp.DeleteDC();

}

int SUIVoteStar::OnHitTest(CPoint pt , BOOL bLBtnDown){
	
	int iRating = 0;
	if( pt.x < 0 || pt.x >= m_iStars*m_StarBmpNullSize.cx
		|| pt.y  < 0 || pt.y > m_StarBmpNullSize.cy)
		return -1;

	iRating = (pt.x / m_StarBmpNullSize.cx) + 1;

	if(m_Rating == iRating){
		return -1;
	}else{
		m_Rating = iRating;
		return 1; //require redraw
	}

}