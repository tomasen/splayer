// ----------------------------------------------------------------------------
// WavPack DirectShow Splitter
// ----------------------------------------------------------------------------
// Copyright (C) 2005 Christophe Paris (christophe.paris <at> free.fr)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// aint with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// Please see the file COPYING in this directory for full copyright
// information.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

typedef struct {
    stream_reader iocallback;
    IAsyncReader *pReader;
    LONGLONG StreamPos;
    LONGLONG StreamLen;
} IAsyncCallBackWrapper;

IAsyncCallBackWrapper* IAsyncCallBackWrapper_new(IAsyncReader *pReader);
void IAsyncCallBackWrapper_free(IAsyncCallBackWrapper* iacw);

//-----------------------------------------------------------------------------

class CWavPackDSSplitter;

enum Command {CMD_RESET, CMD_RUN, CMD_STOP, CMD_EXIT};

class CWavPackDSSplitterInputPin : public CBaseInputPin,
                               public CAMThread                             
{
    friend class CWavPackDSSplitter;

public:
    CWavPackDSSplitterInputPin(CWavPackDSSplitter *pParentFilter, CCritSec *pLock, HRESULT * phr);
    virtual ~CWavPackDSSplitterInputPin();

    HRESULT CheckMediaType(const CMediaType *pmt);
    CMediaType& CurrentMediaType() { return m_mt; };

    HRESULT CheckConnect(IPin* pPin);
    HRESULT BreakConnect(void);
    HRESULT CompleteConnect(IPin *pReceivePin); 

    HRESULT Active();
    HRESULT Inactive();

    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();

    HRESULT DoSeeking(REFERENCE_TIME rtStart);

protected:
    DWORD ThreadProc();
    HRESULT DoProcessingLoop();
    HRESULT DeliverOneFrame(WavPack_parser* wpp);

    CWavPackDSSplitter *m_pParentFilter;
    IAsyncReader *m_pReader;
    IAsyncCallBackWrapper *m_pIACBW;
    WavPack_parser *m_pWavPackParser;
    
    BOOL m_bAbort;
    BOOL m_bDiscontinuity;
};

//-----------------------------------------------------------------------------

class CWavPackDSSplitterCorrectionInputPin : public CBaseInputPin
{
    friend class CWavPackDSSplitter;
    
public:
    CWavPackDSSplitterCorrectionInputPin(CWavPackDSSplitter *pParentFilter,
        CCritSec *pLock, HRESULT * phr);
    virtual ~CWavPackDSSplitterCorrectionInputPin();
    
    HRESULT CheckMediaType(const CMediaType *pmt);
    CMediaType& CurrentMediaType() { return m_mt; };
    
    HRESULT CheckConnect(IPin* pPin);
    HRESULT BreakConnect(void);
    HRESULT CompleteConnect(IPin *pReceivePin); 
        
    HRESULT DoSeeking(REFERENCE_TIME rtStart);

    HRESULT Active();
    HRESULT Inactive();
    
protected:
    
    CWavPackDSSplitter *m_pParentFilter;
    IAsyncReader *m_pReader;
    IAsyncCallBackWrapper *m_pIACBW;
    WavPack_parser *m_pWavPackParser;
    
    BOOL m_bAbort;
    BOOL m_bDiscontinuity;
};

//-----------------------------------------------------------------------------

class CWavPackDSSplitterOutputPin : public CBaseOutputPin,
                                public IMediaSeeking
{
    friend class CWavPackDSSplitter;

public:
    CWavPackDSSplitterOutputPin(CWavPackDSSplitter *pParentFilter, CCritSec *pLock,
        HRESULT * phr);

    DECLARE_IUNKNOWN
        
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
    {
        if (riid == IID_IMediaSeeking) {
            return GetInterface((IMediaSeeking *)this, ppv);
        }
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }

    HRESULT CheckMediaType(const CMediaType *pmt);
    CMediaType& CurrentMediaType() { return m_mt; }
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES *pProp);


    // --- IMediaSeeking ---    
    STDMETHODIMP IsFormatSupported(const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID *pFormat);
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG *pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);
    STDMETHODIMP GetCapabilities(DWORD * pCapabilities);
    STDMETHODIMP CheckCapabilities(DWORD * pCapabilities);
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
        LONGLONG Source, const GUID * pSourceFormat);   
    STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags,
        LONGLONG * pStop, DWORD StopFlags);
    STDMETHODIMP GetPositions(LONGLONG * pCurrent, LONGLONG * pStop);
    STDMETHODIMP GetAvailable(LONGLONG * pEarliest, LONGLONG * pLatest);
    STDMETHODIMP SetRate(double dRate);
    STDMETHODIMP GetRate(double * pdRate);
    STDMETHODIMP GetPreroll(LONGLONG *pPreroll);

protected:
    CWavPackDSSplitter *m_pParentFilter;    
};

//-----------------------------------------------------------------------------

class __declspec(uuid("D8CF6A42-3E09-4922-A452-21DFF10BEEBA")) CWavPackDSSplitter : public CBaseFilter
{

public :
    DECLARE_IUNKNOWN
    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr); 

    CWavPackDSSplitter(LPUNKNOWN lpunk, HRESULT *phr);
    virtual ~CWavPackDSSplitter();
 
    // ----- CBaseFilter -----
    int GetPinCount();
    CBasePin *GetPin(int n);
    STDMETHODIMP Stop(void);
    STDMETHODIMP Pause(void);
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName);

    HRESULT BeginFlush();
    HRESULT EndFlush();

protected:
    CCritSec m_Lock;

    friend class CWavPackDSSplitterInputPin;
    friend class CWavPackDSSplitterOutputPin;
    friend class CWavPackDSSplitterCorrectionInputPin;

    CWavPackDSSplitterInputPin* m_pInputPin;
    CWavPackDSSplitterOutputPin* m_pOutputPin;
    CWavPackDSSplitterCorrectionInputPin* m_pInputPinCorr;

    REFERENCE_TIME m_rtStart, m_rtDuration, m_rtStop;
    DWORD m_dwSeekingCaps;
    double m_dRateSeeking;
    BOOL m_bDontTryToLoadCorrectionFileAgain;

    void SetDuration(REFERENCE_TIME rtDuration);

    HRESULT DoSeeking();

    WavPack_parser *GetWavPackParser()
	{
		return m_pInputPin->m_pWavPackParser;
	}

    WavPack_parser *GetWavPackParserCorrection()
    {
        if(m_pInputPinCorr != NULL && m_pInputPinCorr->IsConnected())
        {
            return m_pInputPinCorr->m_pWavPackParser;
        }
        else
        {
            return NULL;
        }
	}

    HRESULT TryToLoadCorrectionFile();
};

