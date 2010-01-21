
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components

// TODO: reference additional headers your program requires here

#include <streams.h>
#include <dvdmedia.h>
#include <amvideo.h>

#include <atlcoll.h>
#include <Videoacc.h>

//#include "..\BaseVideoFilter\BaseVideoFilter.h"

#include "SVPSubfilterLib.h"


#include "..\..\..\DSUtil\DSUtil.h"
//#include "..\..\..\DSUtil\MediaTypes.h"


#include <initguid.h>
#include "..\..\..\..\include\moreuuids.h"

//#define  SVP_LogMsg5  __noop

extern int c2y_yb[256];
extern int c2y_yg[256];
extern int c2y_yr[256];
extern void ColorConvInit();

void BltLineRGB32(DWORD* d, BYTE* sub, int w, const GUID& subtype)
{
	if(subtype == MEDIASUBTYPE_YV12 || subtype == MEDIASUBTYPE_I420 || subtype == MEDIASUBTYPE_IYUV)
	{
		BYTE* db = (BYTE*)d;
		BYTE* dbtend = db + w;

		for(; db < dbtend; sub+=4, db++)
		{
			if(sub[3] < 0xff)
			{
				int y = (c2y_yb[sub[0]] + c2y_yg[sub[1]] + c2y_yr[sub[2]] + 0x108000) >> 16; 
				*db = y; // w/o colors 
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_YUY2)
	{
		WORD* ds = (WORD*)d;
		WORD* dstend = ds + w;

		for(; ds < dstend; sub+=4, ds++)
		{
			if(sub[3] < 0xff)
			{
				int y = (c2y_yb[sub[0]] + c2y_yg[sub[1]] + c2y_yr[sub[2]] + 0x108000) >> 16; 
				*ds = 0x8000|y; // w/o colors 
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_RGB555)
	{
		WORD* ds = (WORD*)d;
		WORD* dstend = ds + w;

		for(; ds < dstend; sub+=4, ds++)
		{
			if(sub[3] < 0xff)
			{
				*ds = ((*((DWORD*)sub)>>9)&0x7c00)|((*((DWORD*)sub)>>6)&0x03e0)|((*((DWORD*)sub)>>3)&0x001f);
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_RGB565)
	{
		WORD* ds = (WORD*)d;
		WORD* dstend = ds + w;

		for(; ds < dstend; sub+=4, ds++)
		{
			if(sub[3] < 0xff)
			{
				*ds = ((*((DWORD*)sub)>>8)&0xf800)|((*((DWORD*)sub)>>5)&0x07e0)|((*((DWORD*)sub)>>3)&0x001f);
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_RGB24)
	{
		BYTE* dt = (BYTE*)d;
		BYTE* dstend = dt + w*3;

		for(; dt < dstend; sub+=4, dt+=3)
		{
			if(sub[3] < 0xff)
			{
				dt[0] = sub[0];
				dt[1] = sub[1];
				dt[2] = sub[2];
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_RGB32 || subtype == MEDIASUBTYPE_ARGB32)
	{
		DWORD* dstend = d + w;

		for(; d < dstend; sub+=4, d++)
		{
			if(sub[3] < 0xff) *d = *((DWORD*)sub)&0xffffff;
		}
	}
}

/* ResX2 */
void Scale2x(const GUID& subtype, BYTE* d, int dpitch, BYTE* s, int spitch, int w, int h)
{
	if(subtype == MEDIASUBTYPE_YV12 || subtype == MEDIASUBTYPE_I420 || subtype == MEDIASUBTYPE_IYUV)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch) // TODO: replace this mess with mmx code
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			for(BYTE* s3 = s1 + (w-1); s1 < s3; s1 += 1, d1 += 2)
			{
				d1[0] = s1[0]; 
				d1[1] = (s1[0]+s1[1])>>1;
			}

			d1[0] = d1[1] = s1[0]; 

			s1 += 1;
			d1 += 2;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines8(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_YUY2)
	{
		unsigned __int64 __0xffffffff00000000 = 0xffffffff00000000;
		unsigned __int64 __0x00000000ffffffff = 0x00000000ffffffff;
		unsigned __int64 __0x00ff00ff00ff00ff = 0x00ff00ff00ff00ff;

		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch)
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			// row0, 4 pixels: y1|u1|y2|v1|y3|u2|y4|v2
			// ->
			// row0, 8 pixels: y1|u1|(y1+y2)/2|v1|y2|(u1+u2)/2|(y2+y3)/2|(v1+v2)/2

			__asm
			{
				mov		esi, s1
					mov		edi, d1

					mov		ecx, w
					shr		ecx, 1
					dec		ecx

					movq	mm4, __0x00ff00ff00ff00ff
					movq	mm5, __0x00000000ffffffff
					movq	mm6, __0xffffffff00000000
row_loop1:
				movq	mm0, [esi]
				movq	mm2, mm0

					pand	mm0, mm4	// mm0 = 00y400y300y200y1
					psrlw	mm2, 8		// mm2 = 00u200v200u100v1


					movq	mm1, mm0

					pand	mm0, mm5	// mm0 = 0000000000y200y1

					psllq	mm1, 16
					pand	mm1, mm6	// mm1 = 00y300y200000000

					por		mm1, mm0	// mm1 = 00y300y200y200y1

					punpcklwd mm0, mm0	// mm0 = 00y200y200y100y1

					paddw	mm0, mm1
					psrlw	mm0, 1		// mm0 = (mm0 + mm1) / 2


					movq	mm1, mm2
					punpckldq	mm1, mm1 // mm1 = 00u100v100u100v1

					paddw	mm1, mm2
					psrlw	mm1, 1		// mm1 = (mm1 + mm2) / 2


					psllw	mm1, 8
					por		mm0, mm1	// mm0 = (v1+v2)/2|(y2+y3)/2|(u1+u2)/2|y2|v1|(y1+y2)/2|u1|y1

					movq	[edi], mm0

					lea		esi, [esi+4]
				lea		edi, [edi+8]

				dec		ecx
					jnz		row_loop1

					mov		s1, esi
					mov		d1, edi
			};

			*d1++ = s1[0];
			*d1++ = s1[1];
			*d1++ =(s1[0]+s1[2])>>1;
			*d1++ = s1[3];

			*d1++ = s1[2];
			*d1++ = s1[1];
			*d1++ = s1[2];
			*d1++ = s1[3];

			s1 += 4;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines8(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_RGB555)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch) // TODO: replace this mess with mmx code
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			for(BYTE* s3 = s1 + (w-1)*2; s1 < s3; s1 += 2, d1 += 4)
			{
				*((WORD*)d1) = *((WORD*)s1);
				*((WORD*)d1+1) = 
					((((*((WORD*)s1)&0x7c00) + (*((WORD*)s1+1)&0x7c00)) >> 1)&0x7c00)|
					((((*((WORD*)s1)&0x03e0) + (*((WORD*)s1+1)&0x03e0)) >> 1)&0x03e0)|
					((((*((WORD*)s1)&0x001f) + (*((WORD*)s1+1)&0x001f)) >> 1)&0x001f);
			}

			*((WORD*)d1) = *((WORD*)s1);
			*((WORD*)d1+1) = *((WORD*)s1);

			s1 += 2;
			d1 += 4;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines555(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_RGB565)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch) // TODO: replace this mess with mmx code
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			for(BYTE* s3 = s1 + (w-1)*2; s1 < s3; s1 += 2, d1 += 4)
			{
				*((WORD*)d1) = *((WORD*)s1);
				*((WORD*)d1+1) = 
					((((*((WORD*)s1)&0xf800) + (*((WORD*)s1+1)&0xf800)) >> 1)&0xf800)|
					((((*((WORD*)s1)&0x07e0) + (*((WORD*)s1+1)&0x07e0)) >> 1)&0x07e0)|
					((((*((WORD*)s1)&0x001f) + (*((WORD*)s1+1)&0x001f)) >> 1)&0x001f);
			}

			*((WORD*)d1) = *((WORD*)s1);
			*((WORD*)d1+1) = *((WORD*)s1);

			s1 += 2;
			d1 += 4;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines565(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_RGB24)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch) // TODO: replace this mess with mmx code
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			for(BYTE* s3 = s1 + (w-1)*3; s1 < s3; s1 += 3, d1 += 6)
			{
				d1[0] = s1[0]; 
				d1[1] = s1[1]; 
				d1[2] = s1[2];
				d1[3] = (s1[0]+s1[3])>>1;
				d1[4] = (s1[1]+s1[4])>>1;
				d1[5] = (s1[2]+s1[5])>>1;
			}

			d1[0] = d1[3] = s1[0]; 
			d1[1] = d1[4] = s1[1]; 
			d1[2] = d1[5] = s1[2];

			s1 += 3;
			d1 += 6;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines8(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_RGB32 || subtype == MEDIASUBTYPE_ARGB32)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch)
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			__asm
			{
				mov		esi, s1
					mov		edi, d1

					mov		ecx, w
					dec		ecx

					pxor	mm0, mm0
row_loop3:
				movq	mm1, [esi]
				movq	mm2, mm1

					punpcklbw mm1, mm0	// mm1 = 00xx00r100g100b1
					punpckhbw mm2, mm0	// mm2 = 00xx00r200g200b2

					paddw	mm2, mm1
					psrlw	mm2, 1		// mm2 = (mm1 + mm2) / 2

					packuswb	mm1, mm2

					movq	[edi], mm1

					lea		esi, [esi+4]
				lea		edi, [edi+8]

				dec		ecx
					jnz		row_loop3

					mov		s1, esi
					mov		d1, edi
			};

			*((DWORD*)d1) = *((DWORD*)s1);
			*((DWORD*)d1+1) = *((DWORD*)s1);

			s1 += 4;
			d1 += 8;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines8(d, h*2, dpitch);
	}

	__asm emms;
}
static bool b_fake_player = 1;

struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
} *lpTranslateT;

CSVPSubfilterLib::CSVPSubfilterLib(void)
: m_last_2ndSubBaseLineUp(-1), m_last_2ndSubBaseLineDown(1000000)
, m_last_2ndSubBaseLineUp2(-1), m_last_2ndSubBaseLineDown2(1000000)
, m_last_2sub_relative(-1)
, m_force_pos_counter(0)
{
	if(!b_fake_player)
		return;

	CString path;
	GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();
	int Ret = -1;
	path.MakeLower();
	//SVP_LogMsg5(L"got splayer path %s" ,path);
	if( path.Find(_T("splayer")) >= 0 || path.Find(_T("svplayer")) >= 0 || path.Find(_T("mplayerc")) >= 0  ){
		DWORD             dwHandle;
		UINT              dwLen;
		UINT              uLen;
		UINT              cbTranslate;
		LPVOID            lpBuffer;

		dwLen  = GetFileVersionInfoSize(path, &dwHandle);

		TCHAR * lpData = (TCHAR*) malloc(dwLen);
		if(!lpData)
			return ;
		memset((char*)lpData, 0 , dwLen);


		/* GetFileVersionInfo() requires a char *, but the api doesn't
		* indicate that it will modify it */
		if(GetFileVersionInfo(path, dwHandle, dwLen, lpData) != 0)
		{
			
				CString szParm( _T("\\StringFileInfo\\000004b0\\FileDescription"));

				if(VerQueryValue(lpData, szParm, &lpBuffer, &uLen) != 0)
				{
		

					CString szProductName((TCHAR*)lpBuffer);
					//SVP_LogMsg5(L"szProductName %s", szProductName);
					szProductName.MakeLower();

					if(szProductName.Find(_T("ÉäÊÖ")) >= 0 || szProductName.Find(_T("splayer")) >= 0  ){
						Ret = 25;
					
					}
				}
		}
	}
	if(Ret == 25){
		//SVP_LogMsg5(L" splayer" );
		b_fake_player = 0;
	}

}

CSVPSubfilterLib::~CSVPSubfilterLib(void)
{
}

HRESULT CSVPSubfilterLib::Copy(BYTE* pSub, BYTE* pIn, CSize sub, CSize in, int bpp, const GUID& subtype, DWORD black)
{
	int wIn = in.cx, hIn = in.cy, pitchIn = wIn*bpp>>3;
	int wSub = sub.cx, hSub = sub.cy, pitchSub = wSub*bpp>>3;
	bool fScale2x = wIn*2 <= wSub;

	if(fScale2x) wIn <<= 1, hIn <<= 1;

	int left = ((wSub - wIn)>>1)&~1;
	int mid = wIn;
	int right = left + ((wSub - wIn)&1);

	int dpLeft = left*bpp>>3;
	int dpMid = mid*bpp>>3;
	int dpRight = right*bpp>>3;

	if(b_fake_player)
		return S_FALSE;
	ASSERT(wSub >= wIn);

	{
		int i = 0, j = 0;

		j += (hSub - hIn) >> 1;

		for(; i < j; i++, pSub += pitchSub)
		{
			memsetd(pSub, black, dpLeft+dpMid+dpRight);
		}

		j += hIn;

		if(hIn > hSub)
			pIn += pitchIn * ((hIn - hSub) >> (fScale2x?2:1));

		if(fScale2x)
		{
			Scale2x(subtype, 
				pSub + dpLeft, pitchSub, pIn, pitchIn, 
				in.cx, (min(j, hSub) - i) >> 1);

			for(int k = min(j, hSub); i < k; i++, pIn += pitchIn, pSub += pitchSub)
			{
				memsetd(pSub, black, dpLeft);
				memsetd(pSub + dpLeft+dpMid, black, dpRight);
			}
		}
		else
		{
			for(int k = min(j, hSub); i < k; i++, pIn += pitchIn, pSub += pitchSub)
			{
				memsetd(pSub, black, dpLeft);
				memcpy(pSub + dpLeft, pIn, dpMid);
				memsetd(pSub + dpLeft+dpMid, black, dpRight);
			}
		}

		j = hSub;

		for(; i < j; i++, pSub += pitchSub)
		{
			memsetd(pSub, black, dpLeft+dpMid+dpRight);
		}
	}

	return NOERROR;
}

HRESULT CSVPSubfilterLib::CalcDualSubPosisiton(BOOL bltSub1, BOOL bltSub2, CRect& rcDest1, CRect& rcDest2, CSize size, BOOL pSubPic, BOOL pSubPic2)
{
	if(b_fake_player)
		return E_FAIL;

	if(bltSub2 && rcDest2.top < 0)
		rcDest2.MoveToY(0);
	if(bltSub1 && rcDest1.top < 0)
		rcDest1.MoveToY(0);


	BOOL bForcePosBy2 =  (m_last_2ndSubBaseLineDown2 < 1000000) || (m_last_2ndSubBaseLineUp2 >= 0) ;
	BOOL bForcePosBy1 =  (m_last_2ndSubBaseLineDown < 1000000) || (m_last_2ndSubBaseLineUp >= 0);
	BOOL bForcePos =  bForcePosBy1 || bForcePosBy2 ;
	if(bltSub1 && bltSub2 || (bForcePos && (bltSub1 || bltSub2 ) )){
		//avoid overlap
		CRect rectInter;
		if(rectInter.IntersectRect(rcDest1, rcDest2) || bForcePos ){

			//there is overlap
			CPoint cent1 = rcDest1.CenterPoint(); // sub1 center
			CPoint cent2 = rcDest2.CenterPoint(); // sub2 center
			CPoint vcent( size.cx /2 , size.cy /2)  ; //video center
			int i_targetY = -1;

			//which one is closer to border?
			if  ( abs(cent1.y - vcent.y) > abs(cent2.y - vcent.y) && pSubPic2 ){ 
				//rcDest1 is outer(fixed) , move rcDest2

				if(cent2.y > cent1.y && pSubPic || (!pSubPic && m_last_2sub_relative == 1 ) ){ //sub2 is under sub1 and there are on top area of sreen
					m_last_2ndSubBaseLineUp =  max(m_last_2ndSubBaseLineUp ,  rcDest1.bottom);
					i_targetY = m_last_2ndSubBaseLineUp;
					m_last_2sub_relative = 1;
					//moving down
				}else{
					if(rcDest1.top)
					{
						m_last_2ndSubBaseLineDown = min(m_last_2ndSubBaseLineDown, rcDest1.top );
						m_last_2sub_relative = 2;
					}

					i_targetY = m_last_2ndSubBaseLineDown  - rcDest2.Height()  ;

					//moving up
				}
				if(i_targetY >= 0 && i_targetY < 1000000){
					rcDest2.MoveToY(i_targetY);
					if(pSubPic ){
						if(m_last_2sub_relative == 1)
						{
							rcDest1.MoveToY(max(0, i_targetY - rcDest1.Height()));
						}else{
							rcDest1.MoveToY(i_targetY + rcDest2.Height());
						}
					}
				}
			}else{
				//rcDest2 is outer(fixed) , move rcDest1
				if(cent1.y > cent2.y && pSubPic2 || (!pSubPic2 && m_last_2sub_relative == 3)){ //sub1 is under sub2 and there are on top area of sreen
					if(pSubPic2 ){
						m_last_2sub_relative = 3;	
						m_last_2ndSubBaseLineUp2 =  max( m_last_2ndSubBaseLineUp2 , rcDest2.bottom);  //moving down
					}
					i_targetY = m_last_2ndSubBaseLineUp2 ;

				}else{
					if(pSubPic2){
						m_last_2ndSubBaseLineDown2 = min(m_last_2ndSubBaseLineDown2, rcDest2.top );
						m_last_2sub_relative = 4;	
					}
					i_targetY = m_last_2ndSubBaseLineDown2  - rcDest1.Height() ; //moving up

				}
				if(i_targetY >= 0 && i_targetY < 1000000){
					rcDest1.MoveToY(i_targetY);
					if(pSubPic2){
						if(m_last_2sub_relative == 3)
						{
							rcDest2.MoveToY(max(0, i_targetY - rcDest2.Height()));
						}else{
							rcDest2.MoveToY(i_targetY + rcDest1.Height());
						}
					}
				}
			}

			m_last_pSubPic = pSubPic;
			m_last_pSubPic2 = pSubPic2;

		}
	}


	if( !bltSub1 && !bltSub2 && bForcePos){
		m_force_pos_counter++;
		if(m_force_pos_counter > 10){
			ResSetForcePos();
		}

	}

	/*if(bltSub1 && bltSub2){

		//avoid overlap
		CRect rectInter;
		if(rectInter.IntersectRect(rcDest1, rcDest2) ){
			//there is overlap
			CPoint cent1 = rcDest1.CenterPoint(); // sub1 center
			CPoint cent2 = rcDest2.CenterPoint(); // sub2 center
			CPoint vcent( size.cx /2 , size.cy /2)  ; //video center

			//which one is closer to border?
			if  ( abs(cent1.y - vcent.y) > abs(cent2.y - vcent.y) ){ 
				//rcDest1 is outer(fixed) , move rcDest2
				if(cent2.y > cent1.y ){ //sub2 is under sub1 and there are on top area of sreen
					rcDest2.MoveToY(rcDest1.bottom);  //moving down
				}else{
					rcDest2.MoveToY(rcDest1.top - rcDest2.Height()); //moving up
				}
			}else{
				//rcDest2 is outer(fixed) , move rcDest1
				if(cent1.y > cent2.y ){ //sub1 is under sub2 and there are on top area of sreen
					rcDest1.MoveToY(rcDest2.bottom);  //moving down
				}else{
					rcDest1.MoveToY(rcDest2.top - rcDest1.Height()); //moving up
				}
			}

		}
	}*/

	return S_OK;
}

void CSVPSubfilterLib::ResSetForcePos(int s)
{
	m_last_2ndSubBaseLineUp = (-1); 
	m_last_2ndSubBaseLineDown = (1000000);
	m_last_2ndSubBaseLineUp2 = (-1); 
	m_last_2ndSubBaseLineDown2 = (1000000);
	m_last_2sub_relative = (-1);
	m_force_pos_counter = 0;

}