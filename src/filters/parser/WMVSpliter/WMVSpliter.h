#pragma once

#include <atlbase.h>
#include <atlcoll.h>

#include "StdAfx.h"

#include "..\BaseSplitter\BaseSplitter.h"

#include "rostream.h"

#if GDCLWMVFILTER
// classes declared here
class StreamInput;          // input pin
class WMFOutputPin;         // output pins
class WMFDemuxFilter;       // the filter

// The input pin cannot derive from CBaseInputPin as it does not use IMemInputPin.
// It pulls the data using the IAsyncReader on the output pin. The pin can return an
// object implementing the IStream interface which is used inside the filter 
// to synchronously pull data from the peer output pin
class StreamInput 
    : public CBasePin
{
public:
    StreamInput(WMFDemuxFilter* pFilter, CCritSec* pLock, HRESULT* phr);

    // base pin overrides
    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
    HRESULT CompleteConnect(IPin* pPeer);
    HRESULT BreakConnect();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();

    HRESULT GetStream(IStream** ppStream);
private:
    WMFDemuxFilter* m_pDemux;
};


// output pin, delivering compressed data from the WMF reader down to decoders
class WMFOutputPin 
    : public CBaseOutputPin,
    public IMediaSeeking
{
public:
    WMFOutputPin(WMFDemuxFilter* pParser, CCritSec* pLock, HRESULT* phr, LPCWSTR pName);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID iid, void** ppv);

    // normally, pins share the filter's lifetime, but we will release the pins
    // when the input is disconnected, so they need to have a normal COM lifetime
    STDMETHODIMP_(ULONG) NonDelegatingAddRef()
    {
        return CUnknown::NonDelegatingAddRef();
    }
    STDMETHODIMP_(ULONG) NonDelegatingRelease()
    {
        return CUnknown::NonDelegatingRelease();
    }

    // base class overrides for connection establishment
    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pprop);

    // this group of methods deal with the COutputQueue
    HRESULT Active();
    HRESULT Inactive();
    HRESULT DeliverEndOfStream();
    HRESULT DeliverBeginFlush();
    HRESULT DeliverEndFlush();
    void SendAnyway()
    { 
        if (m_pOutputQ)
        {
            m_pOutputQ->SendAnyway();
        }
    }
    HRESULT Deliver(IMediaSample* pSample);

    // called from filter
    void OfferMediaTypes(const CMediaType* pmt, long cbMax);
    HRESULT Send(IMediaSample* pSample, REFERENCE_TIME tStart, DWORD dwFlags);

    bool IsAudio()
    {
        return (*m_mtStream.Type() == MEDIATYPE_Audio) ? true : false;
    }
    // IMediaSeeking
public:
    STDMETHODIMP GetCapabilities(DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities(DWORD * pCapabilities );
    STDMETHODIMP IsFormatSupported(const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG *pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
        LONGLONG    Source, const GUID * pSourceFormat );
    STDMETHODIMP SetPositions(LONGLONG * pCurrent, DWORD dwCurrentFlags, 
        LONGLONG * pStop, DWORD dwStopFlags );
    STDMETHODIMP GetPositions(LONGLONG * pCurrent,
        LONGLONG * pStop );
    STDMETHODIMP GetAvailable(LONGLONG * pEarliest, LONGLONG * pLatest );
    STDMETHODIMP SetRate(double dRate);
    STDMETHODIMP GetRate(double * pdRate);
    STDMETHODIMP GetPreroll(LONGLONG * pllPreroll);



private:
    CMediaType m_mtStream;
    long m_cbMax;
    bool m_bDiscont;
    bool m_bFirstSample;
    WMFDemuxFilter* m_pParser;
    smart_ptr<COutputQueue> m_pOutputQ;
};
// the simplest way to keep an addref-ed pointer that we can Release is to use IPin*
typedef IPinPtr WMFOutputPinPtr;

// when allocating buffers for WMF, we need to return an
// INSSBuffer interface as a wrapper around an IMediaSample object.
// This interface allows us to recover the encapsulated sample object 
// without an ugly cast
MIDL_INTERFACE("D6839DF3-34B3-42a7-9288-D94EEE378822") IEncapsulatedAccess;
DECLARE_INTERFACE_(IEncapsulatedAccess, IUnknown)
{
    STDMETHOD(GetEncapsulatedObject)(THIS_ 
        IUnknown** ppUnknown) PURE;
};
_COM_SMARTPTR_TYPEDEF(IEncapsulatedAccess, __uuidof(IEncapsulatedAccess));

class EncapsulateSample 
    : public CUnknown,
    public INSSBuffer,
    public IEncapsulatedAccess
{
public:
    EncapsulateSample(IMediaSample* pSample);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID iid, void** ppv);

    // INSBuffer methods
    STDMETHOD(GetLength)(DWORD *pdwLength);
    STDMETHOD(SetLength)(DWORD dwLength);
    STDMETHOD(GetMaxLength)(DWORD* pdwLength);
    STDMETHOD(GetBuffer)(BYTE** ppdwBuffer);
    STDMETHOD(GetBufferAndLength)(BYTE** ppdwBuffer, DWORD* pdwLength);

    // IEncapsulatedAccess methods
    STDMETHOD(GetEncapsulatedObject)(IUnknown** ppUnknown);
private:
    IMediaSamplePtr m_pSample;
};


// parser filter itself
class DECLSPEC_UUID("1932C124-77DA-4151-99AA-234FEA09F463")
    WMFDemuxFilter 
    : public CBaseFilter,
     public IWMReaderCallback,
    public IWMReaderCallbackAdvanced
{
public:
    // constructor method used by class factory
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

    // filter registration tables
    static const AMOVIESETUP_MEDIATYPE m_sudType[];
    static const AMOVIESETUP_PIN m_sudPin[];
    static const AMOVIESETUP_FILTER m_sudFilter;

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);


    // CBaseFilter methods
    int GetPinCount();
    CBasePin *GetPin(int n);

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();

    // called from input pin
    HRESULT CompleteConnect();
    HRESULT BreakConnect();

    // called from output pin
    bool SelectSeekingPin(WMFOutputPin* pPin);
    void DeselectSeekingPin(WMFOutputPin* pPin);
    void GetSeekingParams(REFERENCE_TIME* ptStart, REFERENCE_TIME* ptStop, double* pdRate);
    HRESULT Seek(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    HRESULT SetRate(double dRate);
    HRESULT SetStopTime(REFERENCE_TIME tStop);
    REFERENCE_TIME GetDuration()
    {
        return m_duration;
    }

    // WMReader callback methods
public:
    STDMETHOD(OnStatus)( 
        /* [in] */ WMT_STATUS Status,
        /* [in] */ HRESULT hr,
        /* [in] */ WMT_ATTR_DATATYPE dwType,
        /* [in] */ BYTE __RPC_FAR *pValue,
        /* [in] */ void __RPC_FAR *pvContext);
    STDMETHOD(OnSample)( 
        /* [in] */ DWORD dwOutputNum,
        /* [in] */ QWORD cnsSampleTime,
        /* [in] */ QWORD cnsSampleDuration,
        /* [in] */ DWORD dwFlags,
        /* [in] */ INSSBuffer __RPC_FAR *pSample,
        /* [in] */ void __RPC_FAR *pvContext);
    STDMETHOD(OnStreamSample)( 
        /* [in] */ WORD wStreamNum,
        /* [in] */ QWORD cnsSampleTime,
        /* [in] */ QWORD cnsSampleDuration,
        /* [in] */ DWORD dwFlags,
        /* [in] */ INSSBuffer __RPC_FAR *pSample,
        /* [in] */ void __RPC_FAR *pvContext);
    STDMETHOD(OnTime)( 
        /* [in] */ QWORD cnsCurrentTime,
        /* [in] */ void __RPC_FAR *pvContext);
    STDMETHOD(OnStreamSelection)( 
        /* [in] */ WORD wStreamCount,
        /* [in] */ WORD __RPC_FAR *pStreamNumbers,
        /* [in] */ WMT_STREAM_SELECTION __RPC_FAR *pSelections,
        /* [in] */ void __RPC_FAR *pvContext);
    STDMETHOD(OnOutputPropsChanged)( 
        /* [in] */ DWORD dwOutputNum,
        /* [in] */ WM_MEDIA_TYPE __RPC_FAR *pMediaType,
        /* [in] */ void __RPC_FAR *pvContext);
    STDMETHOD(AllocateForStream)( 
        /* [in] */ WORD wStreamNum,
        /* [in] */ DWORD cbBuffer,
        /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer,
        /* [in] */ void __RPC_FAR *pvContext);
    STDMETHOD(AllocateForOutput)( 
        /* [in] */ DWORD dwOutputNum,
        /* [in] */ DWORD cbBuffer,
        /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR *ppBuffer,
        /* [in] */ void __RPC_FAR *pvContext);
public:
    // construct only via class factory
    WMFDemuxFilter(LPUNKNOWN pUnk, HRESULT* phr);
    ~WMFDemuxFilter(void) {}

private:
    
    HRESULT StartWMF();
    HRESULT StopWMF();


private:
    CCritSec m_csFilter;
    smart_ptr<StreamInput> m_pInput;
    IWMReaderPtr m_pWMFReader;

    // not all streams are enabled. To make the wStreamNum to pin
    // mapping simple, the array is sized to the number of streams, but
    // disabled entries are null.
    typedef map<WORD, WMFOutputPinPtr> PinMap;
    PinMap m_Outputs;
    int m_cPins;            // only this many entries are non-null
    WMFOutputPin* PinFromMap(PinMap::iterator it)
    {
        // the simplest way to keep an addrefed pointer is to cast to IPin*
        // but it is, sadly, rather ugly
        return static_cast<WMFOutputPin*>(static_cast<IPin*>(it->second));
    }
    WMFOutputPin* Pin(int n)
    {
        for (PinMap::iterator it = m_Outputs.begin(); it != m_Outputs.end(); it++)
        {
            if (n-- <= 0)
            {
                return PinFromMap(it);
            }
        }
        return NULL;
    }

    // for async open
    CCritSec m_csStatus;
    CAMEvent m_evStatus;
    HRESULT m_hrAsync;
    enum eOp {
        eNoop,
        eOpen,
        eStop,
        eClose,
    };
    eOp m_Op;
    bool m_bEOF;

    // clock time given to WMF Reader to ensure asap delivery
    // -always 1 second ahead
    REFERENCE_TIME m_tDelivery;

    // seeking support
    CCritSec m_csSeeking;
    LONGLONG m_duration;
    REFERENCE_TIME m_tStart;
    REFERENCE_TIME m_tStop;
    double m_dRate;
    WMFOutputPin* m_pSeekingPin;
};

#else

class __declspec(uuid("AA3914AF-9EDB-436e-A0C4-9863AF256D58")) CWMVSpliterFilter
	: public CBaseSplitterFilter
{

	protected:
		HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

		bool DemuxInit();
		void DemuxSeek(REFERENCE_TIME rt);
		bool DemuxLoop();

	public:
		CWMVSpliterFilter(LPUNKNOWN pUnk, HRESULT* phr);
		~CWMVSpliterFilter();


private :
    IWMSyncReader*  m_pReader;
    WORD            m_wAudioStreamNum;
    WORD            m_wVideoStreamNum;
    QWORD           m_cnsStart;
    QWORD           m_cnsEnd;

    BOOL            m_fCompressed;
    BOOL            m_fAudioStream;
    BOOL            m_fVideoStream;
    BOOL            m_fRangeInFrames;
    CROStream*      m_pStream;

};


class __declspec(uuid("70FAB121-8662-453a-B75F-FEBCA575285E")) CWMVSourceFilter : public CWMVSpliterFilter
{
public:
	CWMVSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

#endif
