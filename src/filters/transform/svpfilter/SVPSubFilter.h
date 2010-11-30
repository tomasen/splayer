#pragma once

#include <atlcoll.h>
#include <Videoacc.h>
#include "ISVPSubFilter.h"
#include "..\BaseVideoFilter\BaseVideoFilter.h"
#include "..\..\..\subpic\ISubPic.h"
#include "..\..\..\subtitles\VobSubFile.h"
#include "..\..\..\subtitles\RTS.h"
#include "..\..\..\subtitles\SSF.h"

#include "..\..\..\svplib\SVPSubfilterLib.h"


interface __declspec(uuid("FE6EC6A0-21CA-4970-9EF0-B296F7F38AF0")) ISubClock : public IUnknown
{
	STDMETHOD(SetTime)(REFERENCE_TIME rt) PURE;
	STDMETHOD_(REFERENCE_TIME, GetTime)() PURE;
};


interface __declspec(uuid("0665B760-FBC1-46C3-A35F-E471527C96A4")) ISubClock2 : public ISubClock
{
	STDMETHOD(SetAvgTimePerFrame)(REFERENCE_TIME rt) PURE;
	STDMETHOD(GetAvgTimePerFrame)(REFERENCE_TIME* prt) PURE; // return S_OK only if *prt was set and is valid
};



class __declspec(uuid("E8D381DD-8C7D-4a6f-96ED-92BBB64064CF")) CSVPSubFilter
	 : public CBaseVideoFilter
	 , public ISubPicAllocatorPresenter
	 , public ISVPSubFilter
{
	CSVPSubfilterLib m_sublib;
protected:
	// segment start time, absolute time
	CRefTime m_tPrev;
	REFERENCE_TIME CalcCurrentTime();
	REFERENCE_TIME CalcCurrentTime2();
	//CComPtr<ISubClock> m_pSubClock;
	bool  m_pSubClock;

	HRESULT Transform(IMediaSample* pIn);

	HDC m_hdc;
	HBITMAP m_hbm;
	HFONT m_hfont;
	double m_fps;
	bool m_bDontUseThis;
	bool m_fFlip;
	bool m_bEnlargeARForSub;
	int m_SubtitleDelay, m_SubtitleDelay2, m_SubtitleSpeedMul, m_SubtitleSpeedDiv;
	//bool m_bExternalSubtitleTime;

	//REFERENCE_TIME m_rtNow ,m_lSubtitleDelay , m_rtNow2 , m_lSubtitleDelay2;

public:
	CSVPSubFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CSVPSubFilter();

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);


	STDMETHODIMP Set_AutoEnlargeARForSub( BOOL bEnlarge );

	STDMETHODIMP Put_OSD (LPVOID* lpInput ) {return E_NOTIMPL;};
	//TODO: MORE FUN
	//STDMETHODIMP_(void) SetPosition (RECT w, RECT v)  {return ;};
	//STDMETHODIMP SetVideoAngle (Vector v, bool fRepaint = true)  {return S_OK;};
	//STDMETHODIMP_(SIZE) GetVideoSize (bool fCorrectAR = true) {return CSize(0,0);};

	STDMETHODIMP GetSubStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);

	// Must
	STDMETHODIMP_(double) GetFPS()  ;

	// SubFilter
	STDMETHODIMP_(void) SetTime (REFERENCE_TIME rtNow) ; //TODO ??
	
	STDMETHODIMP_(void) SetSubtitleDelay (int delay_ms) ;
	STDMETHODIMP_(int) GetSubtitleDelay() ;

	STDMETHODIMP_(void) SetSubtitleDelay2 (int delay_ms) ;
	STDMETHODIMP_(int) GetSubtitleDelay2() ;

	STDMETHODIMP_(void) SetSubPicProvider (ISubPicProvider* pSubPicProvider) ;
	STDMETHODIMP_(void) SetSubPicProvider2 (ISubPicProvider* pSubPicProvider) ;
	STDMETHODIMP_(void) Invalidate (REFERENCE_TIME rtInvalidate = -1) ;
	STDMETHODIMP GetDIB (BYTE* lpDib, DWORD* size) ;

	
	HRESULT SetMediaType(PIN_DIRECTION dir, const CMediaType* pMediaType);
	
	// HRESULT CheckInputType(const CMediaType* mtIn); seem useless
	HRESULT 	CheckConnect(PIN_DIRECTION dir, IPin* pPin);
	HRESULT	CompleteConnect(PIN_DIRECTION dir, IPin* pReceivePin);
	
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckOutputType(const CMediaType& mtOut);
	//	BreakConnect(PIN_DIRECTION dir), may be can free some mem as vsfilter
	HRESULT	StartStreaming();
	HRESULT	StopStreaming();
	HRESULT	NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	void  GetOutputFormats (int& nNumber, VIDEO_OUTPUT_FORMATS** ppFormats);

	VIDEO_OUTPUT_FORMATS*					m_pVideoOutputFormat;
	int										m_nVideoOutputCount;

    //virtual bool			IsVideoInterlaced();
	//HRESULT Receive(IMediaSample* pSample);
	//HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	//HRESULT DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	//HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

private:
	void InitSubPicQueue();
	bool AdjustFrameSize(CSize& s);
	int m_l_add_more_height;
	double m_d_stretch_sub_hor;
	CComPtr<ISubPicQueue> m_pSubPicQueue;
	CComPtr<ISubPicQueue> m_pSubPicQueue2;
	CCritSec m_csQueueLock;
	SubPicDesc m_spd;
	CString szLastInitSubqueue;
	CAutoVectorPtr<BYTE> m_pTempPicBuff;
	
	BOOL m_fDoPreBuffering;

	// don't set the "hide subtitles" stream until we are finished with loading
	bool m_fLoading;

	VIDEOINFOHEADER2 m_CurrentVIH2;
};

