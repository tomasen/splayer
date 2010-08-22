#pragma once

#include "svplib.h"

class CSVPSubfilterLib
{
public:
	HRESULT Copy(BYTE* pSub, BYTE* pIn, CSize sub, CSize in, int bpp, const GUID& subtype, DWORD black);
	HRESULT CalcDualSubPosisiton(BOOL bltSub1, BOOL bltSub2, CRect& rcDest1, CRect& rcDest2, CSize size, BOOL pSubPic, BOOL pSubPic2);


	int m_last_2ndSubBaseLineUp;
	int m_last_2ndSubBaseLineDown;
	int m_last_2ndSubBaseLineUp2;
	int m_last_2ndSubBaseLineDown2;
	int m_last_2sub_relative;
	int m_force_pos_counter;
	void ResSetForcePos(int s = 0);
	BOOL m_last_pSubPic;
	BOOL m_last_pSubPic2;


	CSVPSubfilterLib(void);
	~CSVPSubfilterLib(void);
};
