#pragma once


#include "stdafx.h"

#include <atlbase.h>
#include "SUIButton.h"

class SUIVoteStar
{
	int m_iStars;

	CBitmap m_StarBmpYellow;
	CSize m_StarBmpYellowSize, m_StarBmpYellowSizeOrg;
	CBitmap m_StarBmpNull;
	CSize m_StarBmpNullSize, m_StarBmpNullSizeOrg;
	CBitmap m_StarBmpRed;
	CSize m_StarBmpRedSize, m_StarBmpRedSizeOrg;
public:
	int m_Rating;
	
	SUIVoteStar(LPCTSTR szStarBmpYellow,LPCTSTR szStarBmpNull,LPCTSTR szStarBmpRed, int iStars = 5, int defaultRate = 2);
	~SUIVoteStar(void);

	void OnPaint(CMemoryDC *hDC, CRect rc);

	int OnHitTest(CPoint pt , BOOL bLBtnDown);

	void CountDPI();	


};
