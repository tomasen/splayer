
#include "stdafx.h"
#include <math.h>
#include <atlbase.h>
#include "SVPSubFilter.h"


#include "..\..\..\DSUtil\DSUtil.h"
#include "..\..\..\DSUtil\MediaTypes.h"
#include "..\..\..\SubPic\MemSubPic.h"

#include <initguid.h>
#include "..\..\..\..\include\moreuuids.h"

#include "..\..\..\svplib\svplib.h"

#include "..\..\..\apps\mplayerc\DX7AllocatorPresenter.h"
#include "..\..\..\apps\mplayerc\DX9AllocatorPresenter.h"
#include "..\..\..\apps\mplayerc\EVRAllocatorPresenter.h"

#include <afxtempl.h>
#include "..\..\..\apps\mplayerc\mplayerc.h"

#define  SVP_LogMsg5  __noop
#define  SVP_LogMsg6    __noop

CSVPSubFilter::CSVPSubFilter(LPUNKNOWN lpunk, HRESULT* phr)
: CBaseVideoFilter(NAME("SPlayer 字幕滤镜"), lpunk, phr, __uuidof(this), 1) ,
m_fDoPreBuffering(1)
, m_fFlip(0)
,m_bDontUseThis(0)
,m_d_stretch_sub_hor(1.0)
,m_bEnlargeARForSub(1)
//,m_bExternalSubtitleTime(0)
{
	HRESULT hr = S_OK;

	//TODO: clear up mem etc
	if(phr) *phr = hr;
}

CSVPSubFilter::~CSVPSubFilter()
{
	
}

REFERENCE_TIME CSVPSubFilter::CalcCurrentTime()
{
	REFERENCE_TIME rt =  m_tPrev;//m_pSubClock ? m_pSubClock->GetTime() :
	return (rt - m_SubtitleDelay); //  * m_SubtitleSpeedMul / m_SubtitleSpeedDivno, it won't overflow if we use normal parameters (__int64 is enough for about 2000 hours if we multiply it by the max: 65536 as m_SubtitleSpeedMul)
}
REFERENCE_TIME CSVPSubFilter::CalcCurrentTime2()
{
	REFERENCE_TIME rt =  m_tPrev;//m_pSubClock ? m_pSubClock->GetTime() :
	return (rt - m_SubtitleDelay2); //  * m_SubtitleSpeedMul / m_SubtitleSpeedDiv no, it won't overflow if we use normal parameters (__int64 is enough for about 2000 hours if we multiply it by the max: 65536 as m_SubtitleSpeedMul)
}
HRESULT  CSVPSubFilter::GetDIB (BYTE* lpDib, DWORD* size)
{
	CheckPointer(size, E_POINTER);


	//m_pTempPicBuff
	HRESULT hr;

	CSize sub(m_w, m_h);



	BITMAPINFOHEADER bihOut;
	ExtractBIH(&m_pOutput->CurrentMediaType(), &bihOut);

	int p_w = m_w;// bihOut.biWidth;
	int p_h = m_h;//abs(bihOut.biHeight);
	DWORD required = sizeof(BITMAPINFOHEADER) + (p_w*p_h*4*2) ;
	//SVP_LogMsg5( L"GetDIB %d %d %d" , required, m_w, m_h);
	if(!lpDib) {*size = required; return S_OK;}
	if(*size < required) return E_OUTOFMEMORY;
	*size = required;


	BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)lpDib;
	memset(bih, 0, sizeof(BITMAPINFOHEADER));
	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = p_w;
	bih->biHeight = p_h;
	bih->biBitCount = 32;
	bih->biPlanes = 1;
	bih->biSizeImage = bih->biWidth*bih->biHeight*bih->biBitCount>>3;


	//BitBltFromRGBToRGB(bih->biWidth, bih->biHeight, (BYTE*)(bih + 1), bih->biWidth*bih->biBitCount>>3, bih->biBitCount,(BYTE*)m_pTempPicBuff + m_spd.pitch*(m_h-1), -(int)m_spd.pitch, 32);
	//
	const CMediaType& mt = m_pInput->CurrentMediaType();//
	BITMAPINFOHEADER bihIn;
	ExtractBIH(&mt, &bihIn);
	

	CopyBuffer((BYTE*)(bih + 1), (BYTE*)m_pTempPicBuff, p_w, p_h, m_spd.pitch, mt.subtype,false, bih);//
	

	bool fInputFlipped = bihIn.biHeight >= 0 && (bihIn.biCompression  < 3 || bihIn.biCompression == 'BGRA') ;
	bool fOutputFlipped = bihOut.biHeight >= 0  && (bihOut.biCompression  < 3 || bihOut.biCompression == 'BGRA') ;
	SVP_LogMsg5(_T("flip g %d %d %d %x %d %d %x") , m_fFlip , fInputFlipped  , bihIn.biHeight ,bihIn.biCompression  ,  fOutputFlipped ,bihOut.biHeight, bihOut.biCompression);

	if( fInputFlipped != fOutputFlipped ){
		int h = bih->biHeight;
		BYTE* rgbBuff = (BYTE*)(bih + 1);
		CAutoVectorPtr<BYTE> pTempLinBuff;
		int lineBytes = bih->biWidth * 4;
		pTempLinBuff.Allocate( lineBytes + 1);
		for(int i = 0; i < h/2 ; i++){
			memcpy( (BYTE*)pTempLinBuff, rgbBuff + (i * lineBytes ) , lineBytes);
			memcpy( (BYTE*)(rgbBuff + (i * lineBytes )), (BYTE*)(rgbBuff + ((h-i-1) * lineBytes )) , lineBytes);
			memcpy( (BYTE*)(rgbBuff + ((h-i-1) * lineBytes )),(BYTE*)pTempLinBuff , lineBytes);
		}
	
	}
	//CopyBuffer((BYTE*)(bih + 1), (BYTE*)m_spd.bits, m_spd.w, abs(m_spd.h)*(m_fFlip?-1:1), m_spd.pitch, mt.subtype);

	return S_OK;
}


STDMETHODIMP CSVPSubFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(ISubPicAllocatorPresenter)
		QI(ISVPSubFilter)
		__super::NonDelegatingQueryInterface(riid, ppv);
}
//TODO: for sub controling
double CSVPSubFilter::GetFPS()  {
	return m_fps;
}
STDMETHODIMP CSVPSubFilter::GetSubStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
	if(m_pSubPicQueue)
	{
		//SVP_LogMsg5(L" SetTime1 %f " ,(double) rtNow- m_SubtitleDelay);
		m_pSubPicQueue->GetStats(nSubPics, rtNow, rtStart ,rtStop);
	}
	return S_OK;
}
void  CSVPSubFilter::SetTime (REFERENCE_TIME rtNow)
{
	CAutoLock cAutoLock(&m_csQueueLock);

	if(m_pSubPicQueue)
	{
		SVP_LogMsg5(L" SetTime1 %f " ,(double) rtNow- m_SubtitleDelay);
		m_pSubPicQueue->SetTime(rtNow - m_SubtitleDelay);
	}
	if(m_pSubPicQueue2)
	{
		SVP_LogMsg5(L" SetTime12 %f " ,(double) rtNow- m_SubtitleDelay2);
		m_pSubPicQueue2->SetTime(rtNow - m_SubtitleDelay2);
	}

	
	
}
void CSVPSubFilter::SetSubtitleDelay (int delay_ms) 
{
	m_SubtitleDelay = delay_ms*10000;
	
}

void CSVPSubFilter::SetSubtitleDelay2 (int delay_ms) 
{
	m_SubtitleDelay2 = delay_ms*10000;
}

int CSVPSubFilter::GetSubtitleDelay () 
{
	
	return (m_SubtitleDelay/10000);
}

int CSVPSubFilter::GetSubtitleDelay2 () 
{

	return (m_SubtitleDelay2/10000);
}
void CSVPSubFilter::SetSubPicProvider (ISubPicProvider* pSubPicProvider)
{
	if(m_pSubPicQueue){
		m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);
		SVP_LogMsg5(L"Got Sub1");
	}

	return ;
}

void CSVPSubFilter::SetSubPicProvider2 (ISubPicProvider* pSubPicProvider)
{
	if(m_pSubPicQueue2)
		m_pSubPicQueue2->SetSubPicProvider(pSubPicProvider);
	return ;
}

void CSVPSubFilter::Invalidate (REFERENCE_TIME rtInvalidate )
{
	if(m_pSubPicQueue)
		m_pSubPicQueue->Invalidate(rtInvalidate);

	if(m_pSubPicQueue2)
		m_pSubPicQueue2->Invalidate(rtInvalidate);

	return ;
}

/*
bool CSVPSubFilter::IsVideoInterlaced()
{
    // NOT A BUG : always tell DirectShow it's interlaced (progressive flags set in 
    // SetTypeSpecificFlags function)
    return true;
};
*/
//End of sub control func




// CTransformFilter

bool CSVPSubFilter::AdjustFrameSize(CSize& s){
	s.cx <<= 1; 
	s.cy <<= 1;
	return TRUE;
}

HRESULT CSVPSubFilter::Transform(IMediaSample* pIn)
{

	HRESULT hr;
//SVP_LogMsg5(L"CSVPSubFilter::Transform");

	REFERENCE_TIME rtStart, rtStop;
    double fps_that_we_using = m_fps;
	if(SUCCEEDED(pIn->GetTime(&rtStart, &rtStop)))
	{
		double dRate = m_pInput->CurrentRate();

		m_tPrev = m_pInput->CurrentStartTime() + dRate*rtStart;

		REFERENCE_TIME rtAvgTimePerFrame = rtStop - rtStart;

		/*
		if(CComQIPtr<ISubClock2> pSC2 = m_pSubClock)
		{
		REFERENCE_TIME rt;
		if(S_OK == pSC2->GetAvgTimePerFrame(&rt))
		rtAvgTimePerFrame = rt;
		}
		*/

        double accu_fps = 10000000.0/rtAvgTimePerFrame;
        m_fps = max(m_fps , accu_fps);
		fps_that_we_using = m_fps / dRate;
	}

	//
    REFERENCE_TIME rt_sub1 = CalcCurrentTime();
    REFERENCE_TIME rt_sub2 = CalcCurrentTime2();

	{
		CAutoLock cAutoLock(&m_csQueueLock);
		
		if(m_pSubPicQueue)
		{
			
			m_pSubPicQueue->SetFPS(fps_that_we_using);
		}

		if(m_pSubPicQueue2)
		{
			
			m_pSubPicQueue2->SetFPS(fps_that_we_using);
		}
	
		SVP_LogMsg5(L" SetFPS fps %f " , m_fps);
		if(!AfxGetAppSettings().bExternalSubtitleTime){

			
			if(m_pSubPicQueue)
			{
				SVP_LogMsg6(" SetTime CalcCurrentTime1 %f " ,(double) rt_sub1);
				m_pSubPicQueue->SetTime( rt_sub1);
				
			}

			if(m_pSubPicQueue2)
			{
				SVP_LogMsg6(" SetTime CalcCurrentTime2 %f " ,(double) rt_sub2);
				m_pSubPicQueue2->SetTime(rt_sub2);
				
			}
		}
	}

	//

	BYTE* pDataIn = NULL;
	if(FAILED(pIn->GetPointer(&pDataIn)) || !pDataIn){
		SVP_LogMsg5(L"FAiL 1");
		return S_FALSE;
	}

	const CMediaType& mt = m_pInput->CurrentMediaType();

	BITMAPINFOHEADER bihIn;
	ExtractBIH(&mt, &bihIn);

	bool fYV12 = (mt.subtype == MEDIASUBTYPE_YV12 || mt.subtype == MEDIASUBTYPE_I420 || mt.subtype == MEDIASUBTYPE_IYUV);
	int bpp = fYV12 ? 8 : bihIn.biBitCount;
	DWORD black = fYV12 ? 0x10101010 : (bihIn.biCompression == '2YUY') ? 0x80108010 : 0;

	CSize sub(m_w, m_h);
	CSize in(bihIn.biWidth, bihIn.biHeight);

	if(FAILED(m_sublib.Copy((BYTE*)m_pTempPicBuff, pDataIn, sub, in, bpp, mt.subtype, black))) {
		SVP_LogMsg5(L"FAiL 2");
		return E_FAIL;
	}

	if(fYV12)
	{
		BYTE* pSubV = (BYTE*)m_pTempPicBuff + (sub.cx*bpp>>3)*sub.cy;
		BYTE* pInV = pDataIn + (in.cx*bpp>>3)*in.cy;
		sub.cx >>= 1; sub.cy >>= 1; in.cx >>= 1; in.cy >>= 1;
		BYTE* pSubU = pSubV + (sub.cx*bpp>>3)*sub.cy;
		BYTE* pInU = pInV + (in.cx*bpp>>3)*in.cy;
		if(FAILED(m_sublib.Copy(pSubV, pInV, sub, in, bpp, mt.subtype, 0x80808080))){
			SVP_LogMsg5(L"FAiL 3");
			return E_FAIL;
		}
		if(FAILED(m_sublib.Copy(pSubU, pInU, sub, in, bpp, mt.subtype, 0x80808080))){
			SVP_LogMsg5(L"FAiL 4");
			return E_FAIL;
		}
	}

	//

	SubPicDesc spd = m_spd;

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if(FAILED(hr = GetDeliveryBuffer(spd.w, spd.h, &pOut))){
		SVP_LogMsg5(L"FAiL 5 %d %d" , spd.w, spd.h);
		return hr;
	}
	if(FAILED(hr = pOut->GetPointer(&pDataOut))){
		SVP_LogMsg5(L"FAiL 6 %d %d" );
		return hr;
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetDiscontinuity(pIn->IsDiscontinuity() == S_OK);
	pOut->SetSyncPoint(pIn->IsSyncPoint() == S_OK);
	pOut->SetPreroll(pIn->IsPreroll() == S_OK);

	// 

	BITMAPINFOHEADER bihOut;
	ExtractBIH(&m_pOutput->CurrentMediaType(), &bihOut);

	bool fInputFlipped = bihIn.biHeight >= 0 && bihIn.biCompression <= 3;
	bool fOutputFlipped = bihOut.biHeight >= 0 && bihOut.biCompression <= 3;

	SVP_LogMsg5(L"flip info %d %d" , bihIn.biHeight , bihOut.biHeight);

	bool m_fFlip = fInputFlipped != fOutputFlipped;
	//	if(m_fFlipPicture) fFlip = !fFlip;
	//	if(m_fMSMpeg4Fix) fFlip = !fFlip;

	bool fFlipSub = fOutputFlipped;
	//if(m_fFlipSubtitles) fFlipSub = !fFlipSub;

	//

	{
		CAutoLock cAutoLock(&m_csQueueLock);

		//size.cy -= AfxGetMyApp()->GetBottomSubOffset();

		CComPtr<ISubPic> pSubPic;
		CComPtr<ISubPic> pSubPic2;
		BOOL bltSub1 = false, bltSub2 = false;
		CRect rcSource1, rcSource2, rcDest1, rcDest2;
		CSize size(spd.w, spd.h);
       
		if(m_pSubPicQueue)
		{
			if(SUCCEEDED(m_pSubPicQueue->LookupSubPic(rt_sub1, pSubPic)) && pSubPic)
			{
				pSubPic->GetDirtyRect(rcSource1);

				if(m_fFlip ^ fFlipSub)
					spd.h = -spd.h;

				bltSub1 = true;

				rcDest1 = rcSource1;

				//Mempic 不支持变形
				//rcDest1.left+=50;
				//rcDest1.right-=50;
				//rcDest1.DeflateRect(20 , 0) ;
				
			}
		}

		if(m_pSubPicQueue2)
		{
			if(SUCCEEDED(m_pSubPicQueue2->LookupSubPic(rt_sub2, pSubPic2)) && pSubPic2)
			{
				pSubPic2->GetDirtyRect(rcSource2);

				rcDest2 = rcSource2;
				if(m_fFlip ^ fFlipSub)
					spd.h = -spd.h;

				bltSub2 = true;

				//rcDest2.DeflateRect( (rcDest2.Width() - rcDest2.Width() * m_d_stretch_sub_hor)/2 , 0) ;

			}
		}
         SVP_LogMsg6("hh %f %f %f %d %d",m_fps , double(rt_sub1), double(rt_sub2) ,bltSub1 , bltSub2 );
		m_sublib.CalcDualSubPosisiton(bltSub1 , bltSub2 , rcDest1 , rcDest2 , size ,!!pSubPic, !!pSubPic2);
		
		if(bltSub1)
			pSubPic->AlphaBlt(rcSource1, rcDest1, &spd);
		if(bltSub2){
			pSubPic2->AlphaBlt(rcSource2, rcDest2, &spd);
		}

	}
    /*
    if( 0 )//interlaced
    {
        int pitchIn =  spd.pitch;
        int abs_h = abs(spd.h);
        BYTE* pInBuff =  (BYTE*)m_spd.bits;
        BYTE* pI420[3] = {pInBuff, pInBuff + pitchIn*abs_h, pInBuff + pitchIn*abs_h + (pitchIn>>1)*(abs_h>>1)};
        int size = m_w*m_h;
        DeinterlaceBlend(pI420[1], pI420[0], m_w, m_h, m_w, m_w);
        DeinterlaceBlend(pI420[1]+size, pI420[0]+size, m_w/2, m_h/2, m_w/2, m_w/2);
        DeinterlaceBlend(pI420[1]+size*5/4, pI420[0]+size*5/4, m_w/2, m_h/2, m_w/2, m_w/2);
        pI420[2] = pI420[1], pI420[1] = pI420[0], pI420[0] = pI420[2];
    }*/
	//SVP_LogMsg5(_T("flip t %d ") , m_fFlip);
	CopyBuffer(pDataOut, (BYTE*)m_spd.bits, spd.w, abs(spd.h)*(m_fFlip?-1:1), spd.pitch, mt.subtype);

    /*
	//	PrintMessages(pDataOut);
    if(CComQIPtr<IMediaSample2> pMSIn = pIn)
    {
        if( CComQIPtr<IMediaSample2> pMS2 = pOut ){
            AM_SAMPLE2_PROPERTIES props;
            AM_SAMPLE2_PROPERTIES props_in;
            if(SUCCEEDED(pMSIn->GetProperties(sizeof(props_in), (BYTE*)&props_in)) && SUCCEEDED(pMS2->GetProperties(sizeof(props), (BYTE*)&props)))
            {
                
                props.dwTypeSpecificFlags &= ~0x7f;

                props.dwTypeSpecificFlags |= props_in.dwTypeSpecificFlags;
               

                SVP_LogMsg5(_T("pMS2 %x %x") , props.dwTypeSpecificFlags , props_in.dwTypeSpecificFlags);
                pMS2->SetProperties(sizeof(props), (BYTE*)&props);
            }
        }
    }
    */

	hr = m_pOutput->Deliver(pOut);

//	SVP_LogMsg5(L"transform %d", hr);
	return hr;

}
static BOOL CALLBACK MonitorEnumProcARDetect(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	double* ms = (double*)dwData;
	MONITORINFOEX  mi;
	mi.cbSize = sizeof(MONITORINFOEX );
	if( GetMonitorInfo(hMonitor, &mi) ){
		//SVP_LogMsg5(_T("Monitors %s %d %d %d"), mi.szDevice , mi.dwFlags, mi.rcMonitor.right - mi.rcMonitor.left ,   mi.rcMonitor.bottom - mi.rcMonitor.top);

		double ar = (double)( mi.rcMonitor.right - mi.rcMonitor.left )/(mi.rcMonitor.bottom - mi.rcMonitor.top) ;

		if(ar > *ms){
			*ms = ar;
		}

	}
	return TRUE;
}


STDMETHODIMP CSVPSubFilter::Set_AutoEnlargeARForSub( BOOL bEnlarge ){
	/*
	if(m_bEnlargeARForSub != bEnlarge){
			m_bEnlargeARForSub = bEnlarge;
			//SVP_LogMsg5(L"Set_AutoEnlargeARForSub");
			if(m_pOutput && m_pOutput->IsConnected())
				m_pInput->SetMediaType(&m_pInput->CurrentMediaType());
		}
		*/
	return S_OK;
}

void CSVPSubFilter::InitSubPicQueue()
{
	CAutoLock cAutoLock(&m_csQueueLock);


	
	const GUID& subtype = m_pInput->CurrentMediaType().subtype;

	BITMAPINFOHEADER bihIn;
	ExtractBIH(&m_pInput->CurrentMediaType(), &bihIn);

	m_spd.type = -1;
	if(subtype == MEDIASUBTYPE_YV12) m_spd.type = MSP_YV12;
	else if(subtype == MEDIASUBTYPE_I420 || subtype == MEDIASUBTYPE_IYUV) m_spd.type = MSP_IYUV;
	else if(subtype == MEDIASUBTYPE_YUY2) m_spd.type = MSP_YUY2;
	else if(subtype == MEDIASUBTYPE_RGB32) m_spd.type = MSP_RGB32;
	else if(subtype == MEDIASUBTYPE_RGB24) m_spd.type = MSP_RGB24;
	else if(subtype == MEDIASUBTYPE_RGB565) m_spd.type = MSP_RGB16;
	else if(subtype == MEDIASUBTYPE_RGB555) m_spd.type = MSP_RGB15;
	m_l_add_more_height = 0;
	//Get The right AR
	if(0 && m_bEnlargeARForSub){
		double d_current_ar = (double)m_arx/m_ary;
		double d_the_max_ar = 4.0/3.0;
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProcARDetect, (LPARAM)&d_the_max_ar);

		double d_the_better_ar = d_current_ar;

		for(int i = 9; i < 9.0*d_the_max_ar ; i++){
			//SVP_LogMsg5(L"Comp AR %f %f %d ", d_current_ar*0.8 , 16.0/i, i);
			d_the_better_ar = 16.0/i;
			if( d_current_ar*0.8 >= d_the_better_ar){
				break;
			}	
		}
		if(d_the_max_ar < d_the_better_ar && d_the_better_ar < d_current_ar){
			d_the_better_ar = d_the_max_ar;
			double d_real_ar = (double)m_w/m_h;
			m_l_add_more_height = max(0 ,  m_w / d_the_better_ar * d_real_ar / d_current_ar  - m_h );
			if(m_l_add_more_height > 0){
				m_arx = 16;
				m_ary = 16/d_the_better_ar;
				SVP_LogMsg5(L"Max AR %f New AR %d %d %d %f %f ",d_the_max_ar, m_arx,m_ary, m_l_add_more_height, 
					d_current_ar,d_real_ar );
			}
			
		}
		int margin = (m_l_add_more_height) % 8;
		if(margin){
			m_l_add_more_height -= margin;
		}

	}
	m_d_stretch_sub_hor = ((double)m_arx/m_ary) / ( (double) m_w/m_h);
	//
	m_spd.w = m_w;
	m_spd.h = m_h+m_l_add_more_height;

	m_spd.bpp = (m_spd.type == MSP_YV12 || m_spd.type == MSP_IYUV) ? 8 : bihIn.biBitCount;
	m_spd.pitch = m_spd.w*m_spd.bpp>>3;
	

	CString szInitSubqueue;
	szInitSubqueue.Format(L"InitSubPicQueue %d %d %d %d %d %d %d %d %d " ,
		m_spd.type, m_spd.w, m_spd.h ,bihIn.biWidth, bihIn.biHeight , m_spd.bpp , m_spd.pitch , m_w, m_h );
	if(szInitSubqueue == szLastInitSubqueue){
		return;
	}
	szLastInitSubqueue = szInitSubqueue;
	SVP_LogMsg5(szInitSubqueue);
	

	CComPtr<ISubPicProvider> pSubPicProvider;
	if(m_pSubPicQueue) m_pSubPicQueue->GetSubPicProvider(&pSubPicProvider);

	CComPtr<ISubPicProvider> pSubPicProvider2;
	if(m_pSubPicQueue2) m_pSubPicQueue2->GetSubPicProvider(&pSubPicProvider2);


	m_pSubPicQueue = NULL;
	m_pSubPicQueue2 = NULL;

	m_pTempPicBuff.Free();
	m_pTempPicBuff.Allocate(4*m_w*(m_h+m_l_add_more_height)*2);

	m_spd.bits = (void*)m_pTempPicBuff;

	//SVP_LogMsg5(L"Sub_ %d %d ",m_w , m_h);
	CComPtr<ISubPicAllocator> pSubPicAllocator = new CMemSubPicAllocator(m_spd.type, CSize(m_w, m_h+m_l_add_more_height));

	CSize video(bihIn.biWidth, bihIn.biHeight), window = video;
	//if(AdjustFrameSize(window)) video += video; //TODO: not sure why we doing this
	//window.cx -= 100y;
	ASSERT(window == CSize(m_w, m_h));
	window.cy += m_l_add_more_height;

	pSubPicAllocator->SetCurSize(window);
	pSubPicAllocator->SetCurVidRect(CRect(CPoint((window.cx - video.cx)/2, (window.cy - video.cy)/2), video));

	HRESULT hr = S_OK;
	m_pSubPicQueue = m_fDoPreBuffering 
		? (ISubPicQueue*)new CSubPicQueue(10, pSubPicAllocator, &hr)
		: (ISubPicQueue*)new CSubPicQueueNoThread(pSubPicAllocator, &hr);

	if(FAILED(hr)){
		SVP_LogMsg5(L"Create m_pSubPicQueue 1 Fail");
		m_pSubPicQueue = NULL;
	}

	HRESULT hr2 = S_OK;
	m_pSubPicQueue2 = m_fDoPreBuffering 
		? (ISubPicQueue*)new CSubPicQueue(10, pSubPicAllocator, &hr2)
		: (ISubPicQueue*)new CSubPicQueueNoThread(pSubPicAllocator, &hr2);

	if(FAILED(hr2)){
		m_pSubPicQueue2 = NULL;
		SVP_LogMsg5(L"Create m_pSubPicQueue 2 Fail");
	}

	

	if(m_hbm) {DeleteObject(m_hbm); m_hbm = NULL;}
	if(m_hdc) {DeleteDC(m_hdc); m_hdc = NULL;}

	struct {BITMAPINFOHEADER bih; DWORD mask[3];} b = {{sizeof(BITMAPINFOHEADER), m_w, -(int)m_h, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}, 0xFF0000, 0x00FF00, 0x0000FF};
	m_hdc = CreateCompatibleDC(NULL);
	m_hbm = CreateDIBSection(m_hdc, (BITMAPINFO*)&b, DIB_RGB_COLORS, NULL, NULL, 0);

	CString szName(this->m_pName);
	if( (szName.Find(_T("SPlayer")) < 0) && (szName.Find(_T("射手")) < 0) ){
		//AfxMessageBox(szName);
		m_pSubPicQueue = NULL;
		m_pSubPicQueue2 = NULL;
		pSubPicAllocator = NULL;
	}

	BITMAP bm;
	GetObject(m_hbm, sizeof(bm), &bm);
	memsetd(bm.bmBits, 0xFF000000, bm.bmHeight*bm.bmWidthBytes);

	
	if(m_pSubPicQueue && pSubPicProvider) m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);

	if(m_pSubPicQueue2 && pSubPicProvider2) m_pSubPicQueue2->SetSubPicProvider(pSubPicProvider2);

	//SVP_LogMsg5(L"Init Done")	;
}

HRESULT CSVPSubFilter::SetMediaType(PIN_DIRECTION dir, const CMediaType* pmt)
{
	//seems a good place to init subqueue
	SVP_LogMsg5(L"SetMediaType");
	HRESULT hr = __super::SetMediaType(dir, pmt);
	if(FAILED(hr)) return hr;

	if(dir == PINDIR_INPUT)
	{
		CAutoLock cAutoLock(&m_csReceive);

		REFERENCE_TIME atpf = 
			pmt->formattype == FORMAT_VideoInfo ? ((VIDEOINFOHEADER*)pmt->Format())->AvgTimePerFrame :
			pmt->formattype == FORMAT_VideoInfo2 ? ((VIDEOINFOHEADER2*)pmt->Format())->AvgTimePerFrame :
			0;

		m_fps = atpf ? 10000000.0 / atpf : 25;

		if (pmt->formattype == FORMAT_VideoInfo2)
			m_CurrentVIH2 = *(VIDEOINFOHEADER2*)pmt->Format();

		
		InitSubPicQueue();
	}
	else if(dir == PINDIR_OUTPUT)
	{
		
	}
	return hr;
}
HRESULT CSVPSubFilter::CheckConnect(PIN_DIRECTION dir, IPin* pPin)
{
	SVP_LogMsg5(L"CheckConnect");
	

	
	if(dir == PINDIR_OUTPUT)
	{
		//CLSID clsid = GetCLSID(pPin);
			// one of these needed for dynamic format changes
/*
			
			*/
		
	}else if(dir == PINDIR_INPUT){
	/*	CLSID clsid = GetCLSID(pPin);
		if(clsid == GUIDFromCString(L"{FEB50740-7BEF-11CE-9BD9-0000E202599C}")){
			//SVP_LogMsg5(L"CSVPSubFilter CheckConnect Filter %s ", CStringFromGUID(clsid));

			 AfxGetAppSettings().bDontNeedSVPSubFilter = true;
			
			 CComPtr<IBaseFilter> pBFX;
			 if( SUCCEEDED( m_pGraph->FindFilterByName(m_pName, &pBFX) )){
				  m_pGraph->RemoveFilter(pBFX);
			 }

			return VFW_E_INVALIDMEDIATYPE;
		}
		
		*/
	}

	return __super::CheckConnect(dir, pPin);
}
HRESULT CSVPSubFilter::CheckInputType(const CMediaType* mtIn)
{
	HRESULT hr  = __super::CheckInputType( mtIn);
	SVP_LogMsg5(L"CheckInputType %x", hr);
	
	return hr;
}
HRESULT CSVPSubFilter::CheckOutputType(const CMediaType& mtOut)
{
	HRESULT hr  = __super::CheckOutputType( mtOut);
	SVP_LogMsg5(L"CheckOutputType %x", hr);
	return hr;

}


VIDEO_OUTPUT_FORMATS DefaultFormatsRGB565[] =
{
	{&MEDIASUBTYPE_RGB565, 1, 16, BI_BITFIELDS},
	{&MEDIASUBTYPE_RGB555, 1, 16, BI_BITFIELDS},
	{&MEDIASUBTYPE_RGB565, 1, 16, BI_RGB},
	{&MEDIASUBTYPE_RGB555, 1, 16, BI_RGB}
};

void CSVPSubFilter::GetOutputFormats (int& nNumber, VIDEO_OUTPUT_FORMATS** ppFormats)
{
	__super::GetOutputFormats(nNumber, ppFormats);
	//SVP_LogMsg5(L"GetOutputFormats ");
	//nNumber		= countof(DefaultFormatsRGB565);
	//*ppFormats	= DefaultFormatsRGB565;
}

HRESULT CSVPSubFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut){
	//if(mtIn->majortype == MEDIATYPE_Video &&
	//	mtIn->subtype == MEDIASUBTYPE_YUY2 && 
	//	(mtOut->subtype == MEDIASUBTYPE_RGB565 || mtOut->subtype == MEDIASUBTYPE_RGB555)){
			//BitBltFromYUY2ToRGB to 16dbpp not yet done

			//return VFW_E_TYPE_NOT_ACCEPTED;
	//}
	HRESULT hr  = __super::CheckTransform( mtIn, mtOut);
	SVP_LogMsg5(L"CheckTransform %x", hr);
	return hr;
	
}
HRESULT CSVPSubFilter::CompleteConnect(PIN_DIRECTION dir, IPin* pReceivePin)
{

	//if(m_bDontUseThis){
	//	return VFW_E_INVALIDMEDIATYPE;
	//}

	if(dir == PINDIR_INPUT)
	{
		
	
	}
	else if(dir == PINDIR_OUTPUT)
	{
		//CLSID clsid = GetCLSID(pReceivePin);

		//TODO: Check What connect to this and disable sub if VMR/EVR
		
/*

		if(clsid == CLSID_EVRAllocatorPresenter || clsid == CLSID_VMR9AllocatorPresenter || clsid == CLSID_VMR7AllocatorPresenter) {
				m_bDontUseThis = true;
				return VFW_E_INVALIDMEDIATYPE;
		} 
*/
		
		// HACK: triggers CBaseVideoFilter::SetMediaType to adjust m_w/m_h/.. and InitSubPicQueue() to realloc buffers
		m_pInput->SetMediaType(&m_pInput->CurrentMediaType());
	}

	return __super::CompleteConnect(dir, pReceivePin);
}

HRESULT CSVPSubFilter::StartStreaming()
{
	/* WARNING: calls to m_pGraph member functions from within this function will generate deadlock with Haali
	* Video Renderer in MPC. Reason is that CAutoLock's variables in IFilterGraph functions are overriden by 
	* CFGManager class.
	*/

	m_fLoading = false;

	InitSubPicQueue();


	//TODO  put_MediaFPS(m_fMediaFPSEnabled, m_MediaFPS);

	return __super::StartStreaming();
}

HRESULT CSVPSubFilter::StopStreaming()
{
	//TODO InvalidateSubtitle();
	//this->
	//Invalidate();
	
	return __super::StopStreaming();
}

HRESULT CSVPSubFilter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	m_tPrev = tStart; //TODO: what m_tPrev is for?
	return __super::NewSegment(tStart, tStop, dRate);
}
