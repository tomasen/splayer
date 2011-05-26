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

#include <windows.h>
#include <streams.h>
#include <initguid.h>
#include <mmreg.h>

#include "..\wavpack\wputils.h"
#include "..\wavpacklib\wavpack_common.h"
#include "..\wavpacklib\wavpack_frame.h"
#include "..\wavpacklib\wavpack_parser.h"

#include "..\WavPackDS_GUID.h"
#include "WavPackDSSplitter.h"

#include "RegistryUtils.h"

// ----------------------------------------------------------------------------

#define constrain(x,y,z) (((y) < (x)) ? (x) : ((y) > (z)) ? (z) : (y))

#define SAFE_RELEASE(x) if(x != NULL) { x->Release(); x = NULL; }

// ----------------------------------------------------------------------------

#ifdef _DEBUG
#include <stdio.h>
#include <tchar.h>
void DebugLog(const char *pFormat,...) {
    char szInfo[2000];
    
    // Format the variable length parameter list
    va_list va;
    va_start(va, pFormat);
    
    _vstprintf(szInfo, pFormat, va);
    lstrcat(szInfo, TEXT("\r\n"));
    OutputDebugString(szInfo);
    
    va_end(va);
}
#else
#define DebugLog
#endif

// ----------------------------------------------------------------------------
//  Registration setup stuff

static const WCHAR g_wszWavPackDSName[]  = L"WavPack Audio Splitter";

// ----------------------------------------------------------------------------

CUnknown *WINAPI CWavPackDSSplitter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    //DbgSetModuleLevel(LOG_ERROR,5);
    //DbgSetModuleLevel(LOG_TRACE,5);
    //DbgSetModuleLevel(LOG_MEMORY,2);
    //DbgSetModuleLevel(LOG_LOCKING,2);
    //DbgSetModuleLevel(LOG_TIMING,5);

    CWavPackDSSplitter *pNewObject = new CWavPackDSSplitter(punk, phr);
    if (!pNewObject)
    {
        *phr = E_OUTOFMEMORY;
    }
    return pNewObject;
}

// ----------------------------------------------------------------------------

CWavPackDSSplitter::CWavPackDSSplitter(LPUNKNOWN lpunk, HRESULT *phr) :
    CBaseFilter(NAME("WavPack Splitter"), lpunk, &m_Lock, CLSID_WavPackDSSplitter),
    m_pInputPin(NULL),
    m_pOutputPin(NULL),
    m_rtStart(0),
    m_rtStop(0),
    m_rtDuration(0),
    m_dRateSeeking(1.0),
    m_bDontTryToLoadCorrectionFileAgain(FALSE)
    
{
    m_dwSeekingCaps = AM_SEEKING_CanGetDuration     
        | AM_SEEKING_CanGetStopPos
        | AM_SEEKING_CanSeekForwards
        | AM_SEEKING_CanSeekBackwards
        | AM_SEEKING_CanSeekAbsolute;

    m_pInputPin = new CWavPackDSSplitterInputPin(this, &m_Lock, phr);
    if(m_pInputPin == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }

    m_pInputPinCorr = new CWavPackDSSplitterCorrectionInputPin(this, &m_Lock, phr);
    if(m_pInputPinCorr == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }

    m_pOutputPin = new CWavPackDSSplitterOutputPin(this, &m_Lock, phr);
    if(m_pOutputPin == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }
}

// ----------------------------------------------------------------------------

CWavPackDSSplitter::~CWavPackDSSplitter()
{
    delete m_pInputPin;
    m_pInputPin = NULL;
    delete m_pOutputPin;
    m_pOutputPin = NULL;
    delete m_pInputPinCorr;
    m_pInputPinCorr = NULL;
}

// ----------------------------------------------------------------------------

int CWavPackDSSplitter::GetPinCount()
{
    CAutoLock lock(m_pLock);
    return 3;
}

// ----------------------------------------------------------------------------

CBasePin* CWavPackDSSplitter::GetPin(int n)
{
    CAutoLock lock(m_pLock);
    if(n == 0)
    {
        return m_pInputPin;
    }
    else if(n == 1)
    {
        return m_pInputPinCorr;
    }
    else if(n == 2)
    {
        return m_pOutputPin;
    }
    return NULL;
}

// ----------------------------------------------------------------------------

STDMETHODIMP CWavPackDSSplitter::Stop(void)
{
    DebugLog("CWavPackDSSplitter::Stop 0x%08X", GetCurrentThreadId());
    return CBaseFilter::Stop();
}

// ----------------------------------------------------------------------------

STDMETHODIMP CWavPackDSSplitter::Pause(void)
{
    DebugLog("CWavPackDSSplitter::Pause 0x%08X", GetCurrentThreadId());

    CAutoLock cObjectLock(m_pLock); 
    
    // notify all pins of the change to active state

    if (m_State == State_Stopped) {
        // Order is important, the output pin allocator need to be commited
        // when we activate the input pin

        // First the output pin
        if (m_pOutputPin->IsConnected()) {
            HRESULT hr = m_pOutputPin->Active();
            if (FAILED(hr)) {
                return hr;
            }
        }
        // Then the input pin
        if (m_pInputPin->IsConnected()) {
            HRESULT hr = m_pInputPin->Active();
            if (FAILED(hr)) {
                return hr;
            }
        }
        // Then the correction input pin
        if ((m_pInputPinCorr != NULL) && m_pInputPinCorr->IsConnected()) {
            HRESULT hr = m_pInputPinCorr->Active();
            if (FAILED(hr)) {
                return hr;
            }
        }
    }
    
    m_State = State_Paused;
    return S_OK;
}

// ----------------------------------------------------------------------------

STDMETHODIMP CWavPackDSSplitter::Run(REFERENCE_TIME tStart)
{
    DebugLog("CWavPackDSSplitter::Run 0x%08X", GetCurrentThreadId());
    return CBaseFilter::Run(tStart);
}

// ----------------------------------------------------------------------------

void CWavPackDSSplitter::SetDuration(REFERENCE_TIME rtDuration)
{
    m_rtStart = 0; 
    m_rtStop = rtDuration; 
    m_rtDuration = rtDuration;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitter::BeginFlush()
{
    CAutoLock lock(m_pLock);
    // Call DeliverBeginFlush on output pin first so GetDeliveryBuffer doesn't lock
    // in input pin DoProcessingLoop
    m_pOutputPin->DeliverBeginFlush();
    // Suspend DoProcessingLoop
    m_pInputPin->BeginFlush();

    if ((m_pInputPinCorr != NULL) && m_pInputPinCorr->IsConnected())
    {
        m_pInputPinCorr->BeginFlush();
    }    
    return NOERROR;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitter::EndFlush()
{
    CAutoLock lock(m_pLock);
    m_pOutputPin->DeliverEndFlush();
    m_pInputPin->EndFlush(); 
    if ((m_pInputPinCorr != NULL) && m_pInputPinCorr->IsConnected())
    {
        m_pInputPinCorr->EndFlush();
    }    
    return NOERROR;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitter::DoSeeking()
{
    if ((m_pInputPinCorr != NULL) && m_pInputPinCorr->IsConnected())
    {
        m_pInputPinCorr->DoSeeking(m_rtStart);
    }    
    return m_pInputPin->DoSeeking(m_rtStart);
}

// ----------------------------------------------------------------------------

STDMETHODIMP CWavPackDSSplitter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
    return CBaseFilter::JoinFilterGraph(pGraph,pName);
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitter::TryToLoadCorrectionFile()
{
    // Here is the nasty hacky stuff :>

    HRESULT hr = S_FALSE;
    IPin *pPinOutSrc = NULL;
    IFileSourceFilter *pFSF = NULL;
    LPOLESTR pszFileName = NULL;
    IBaseFilter* pSrcFilterCorr = NULL;
    IFileSourceFilter*	pFSFCorr = NULL;
    IEnumPins *pEnum = NULL;
    IPin *pPinNew = NULL;
    BOOL bCorrectionFileLoaded = FALSE;

    if((m_bDontTryToLoadCorrectionFileAgain == TRUE) ||
       (m_pInputPinCorr == NULL) ||
       (m_pInputPinCorr->IsConnected() == TRUE))
    {
        return hr;
    }

    if((m_pInputPin->m_pWavPackParser->first_wphdr.flags & WV_HYBRID_FLAG) != WV_HYBRID_FLAG)
    {
        // Not an hybrid file, don't even try
        m_bDontTryToLoadCorrectionFileAgain = TRUE;
        return hr;
    }
    
#define IF_FAIL_BREAK(x) if(FAILED(x)) { break; }
    
    do {
        hr = m_pInputPin->ConnectedTo(&pPinOutSrc);
        IF_FAIL_BREAK(hr);

        // Get a pointer on the source filter
        PIN_INFO pi;
        pi.pFilter = NULL;
        hr = pPinOutSrc->QueryPinInfo(&pi);
        IF_FAIL_BREAK(hr);
        
        // Get source filter IFileSourceFilter interface    
        hr = pi.pFilter->QueryInterface(IID_IFileSourceFilter, (void **)&pFSF);
        IF_FAIL_BREAK(hr);
        
        // Get filename
        hr = pFSF->GetCurFile(&pszFileName, NULL);
        IF_FAIL_BREAK(hr);

        // Create correction file filename
        WCHAR pszFileNameC[MAX_PATH];
        ZeroMemory(pszFileNameC, sizeof(WCHAR)*MAX_PATH);
        int cch = lstrlenW(pszFileName);
        CopyMemory(pszFileNameC, pszFileName, cch*sizeof(WCHAR));
        pszFileNameC[cch] = 'c';
        
        // Create new file source filter
        hr = CoCreateInstance(CLSID_AsyncReader,
            NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter,
            (void**)&pSrcFilterCorr);
        IF_FAIL_BREAK(hr);

        IFilterGraph* pFG = GetFilterGraph();
        hr = pFG->AddFilter(pSrcFilterCorr, pszFileNameC);
        IF_FAIL_BREAK(hr);
        
        hr = pSrcFilterCorr->QueryInterface(IID_IFileSourceFilter, (void**)&pFSFCorr);
        IF_FAIL_BREAK(hr);
                
        hr = pFSFCorr->Load(pszFileNameC, NULL);
        IF_FAIL_BREAK(hr);
        
        // Get first pin and connect it
        HRESULT hr = pSrcFilterCorr->EnumPins(&pEnum);
        IF_FAIL_BREAK(hr);
        if(pEnum->Next(1, &pPinNew, 0) == S_OK)
        {            
            hr = pFG->ConnectDirect(pPinNew, m_pInputPinCorr, NULL);
            bCorrectionFileLoaded = SUCCEEDED(hr);
        }
        
    } while(0);
    
    if((bCorrectionFileLoaded == FALSE) && (pSrcFilterCorr != NULL))
    {
        IFilterGraph* pFG = GetFilterGraph();
        pFG->RemoveFilter(pSrcFilterCorr);
    }

    // Cleanup
    SAFE_RELEASE(pPinNew);
    SAFE_RELEASE(pEnum);
    SAFE_RELEASE(pFSFCorr);
    SAFE_RELEASE(pSrcFilterCorr);
    if(pszFileName != NULL)
    {
        CoTaskMemFree(pszFileName);
    }
    SAFE_RELEASE(pFSF);
    SAFE_RELEASE(pPinOutSrc);
    
#undef IF_FAIL_BREAK
    
    m_bDontTryToLoadCorrectionFileAgain = TRUE;

    return hr;
}

// ============================================================================

CWavPackDSSplitterInputPin::CWavPackDSSplitterInputPin(
    CWavPackDSSplitter *pParentFilter, CCritSec *pLock, HRESULT * phr) :
    CBaseInputPin(NAME("CWavPackDSSplitterInputPin"),
        (CBaseFilter *) pParentFilter,
        pLock,
        phr,
        L"Input"),      
    m_pParentFilter(pParentFilter),
    m_pReader(NULL),
    m_bDiscontinuity(TRUE),
    m_pIACBW(NULL),
    m_pWavPackParser(NULL)
{
    
}

// ----------------------------------------------------------------------------

CWavPackDSSplitterInputPin::~CWavPackDSSplitterInputPin()
{
    if(m_pWavPackParser)
	{
        wavpack_parser_free(m_pWavPackParser);
		m_pWavPackParser = NULL;
	}
}
    
// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::CheckMediaType(const CMediaType *pmt)
{
    if ((*pmt->Type() != MEDIATYPE_Stream) ||
        (*pmt->Subtype() != MEDIASUBTYPE_WAVPACK_Stream))
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::CheckConnect(IPin* pPin)
{
    HRESULT hr = CBaseInputPin::CheckConnect(pPin);
    if (FAILED(hr))
        return hr;

    hr = pPin->QueryInterface(IID_IAsyncReader, (void**)&m_pReader); 
    if (FAILED(hr))
        return S_FALSE;

    if(m_pIACBW)
	{
        IAsyncCallBackWrapper_free(m_pIACBW);
		m_pIACBW = NULL;
	}
    m_pIACBW = IAsyncCallBackWrapper_new(m_pReader);        

    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::BreakConnect(void)
{
    HRESULT hr = CBaseInputPin::BreakConnect();
    if (FAILED(hr))
        return hr;
    
    if(m_pIACBW)
	{
        IAsyncCallBackWrapper_free(m_pIACBW);
		m_pIACBW = NULL;
	}

    if (m_pReader) 
    { 
        m_pReader->Release(); 
        m_pReader = NULL; 
    }
    
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
        return hr;
    
    if(m_pWavPackParser)
	{
        wavpack_parser_free(m_pWavPackParser);
		m_pWavPackParser = NULL;
	}
    m_pWavPackParser = wavpack_parser_new((stream_reader*)m_pIACBW, FALSE);
    if(!m_pWavPackParser)
    {
        return E_FAIL;
    }
    
    // Compute total duration
    REFERENCE_TIME rtDuration;
    rtDuration = m_pWavPackParser->first_wphdr.total_samples;
    rtDuration = (rtDuration * 10000000) / m_pWavPackParser->sample_rate;
    m_pParentFilter->SetDuration(rtDuration);

    return S_OK;
}

// ----------------------------------------------------------------------------

DWORD CWavPackDSSplitterInputPin::ThreadProc()
{
    Command com; 
    
    DebugLog("===> Entering CWavPackDSSplitterInputPin::ThreadProc... 0x%08X", GetCurrentThreadId());

    do 
    { 
        DebugLog("===> ThreadProc waiting command... 0x%08X", GetCurrentThreadId());
        com = (Command)GetRequest();
        switch (com) 
        { 
        case CMD_EXIT:
            DebugLog("===> ThreadProc CMD_EXIT 0x%08X", GetCurrentThreadId());
            Reply(NOERROR); 
            break; 
            
        case CMD_STOP: 
            DebugLog("===> ThreadProc CMD_STOP 0x%08X", GetCurrentThreadId());
            Reply(NOERROR); 
            break; 
            
        case CMD_RUN:
            DebugLog("===> ThreadProc CMD_RUN 0x%08X", GetCurrentThreadId());
            DebugLog("===> Entering DoProcessingLoop... 0x%08X", GetCurrentThreadId());
            DoProcessingLoop(); 
            DebugLog("<=== Leaving DoProcessingLoop 0x%08X", GetCurrentThreadId());
            break; 
        } 
    } while (com != CMD_EXIT);

    DebugLog("<=== Leaving CWavPackDSSplitterInputPin::ThreadProc 0x%08X", GetCurrentThreadId());
    
    return NOERROR; 
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::DeliverOneFrame(WavPack_parser* wpp)
{
    IMediaSample *pSample;
    BYTE *Buffer = NULL;
    HRESULT hr;
    unsigned long FrameLenBytes = 0, FrameLenSamples = 0, FrameIndex = 0;

    // Get a new media sample
    hr = m_pParentFilter->m_pOutputPin->GetDeliveryBuffer(&pSample, NULL, NULL, 0); 
    if (FAILED(hr))
    {
        DebugLog("CWavPackDSSplitterInputPin::DoProcessingLoop GetDeliveryBuffer failed 0x%08X",hr);
        return hr;
    }
    
    hr = pSample->GetPointer(&Buffer);
    if (FAILED(hr))
    {
        DebugLog("CWavPackDSSplitterInputPin::DoProcessingLoop GetPointer failed 0x%08X",hr);
        pSample->Release();
        return hr;
    }
    
    FrameLenBytes = wavpack_parser_read_frame(wpp, Buffer,
        &FrameIndex, &FrameLenSamples);
    if(!FrameLenBytes)
    {
        // Something bad happened, let's end here
        pSample->Release();
        m_pParentFilter->m_pOutputPin->DeliverEndOfStream();
        // TODO : check if we need to stop the thread
        DebugLog("CWavPackDSSplitterInputPin::DoProcessingLoop wavpack_parser_read_frame error");
        return hr;
    }
    pSample->SetActualDataLength(FrameLenBytes);
    
    if(wpp->is_correction == TRUE)
    {    
        IMediaSample2 *pSample2;
        if (SUCCEEDED(pSample->QueryInterface(IID_IMediaSample2, (void **)&pSample2)))
        {
            AM_SAMPLE2_PROPERTIES ams2p;
            ZeroMemory(&ams2p, sizeof(AM_SAMPLE2_PROPERTIES));
            hr = pSample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (PBYTE)&ams2p);
            if(SUCCEEDED(hr))
            {            
                ams2p.dwStreamId = AM_STREAM_BLOCK_ADDITIONNAL;
                pSample2->SetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (PBYTE)&ams2p);
            }
            pSample2->Release();
            pSample2 = NULL;
        }
    }
    
    REFERENCE_TIME rtStart, rtStop;
    rtStart = FrameIndex;
    rtStop = rtStart + FrameLenSamples;
    rtStart = (rtStart * 10000000) / wpp->sample_rate;
    rtStop = (rtStop * 10000000) / wpp->sample_rate;
    
    rtStart -= m_pParentFilter->m_rtStart;
    rtStop  -= m_pParentFilter->m_rtStart;
    
    pSample->SetTime(&rtStart, &rtStop);
    pSample->SetPreroll(FALSE);
    pSample->SetDiscontinuity(m_bDiscontinuity);
    if(m_bDiscontinuity)
    {
        m_bDiscontinuity = FALSE;
    }
    pSample->SetSyncPoint(TRUE);
    
    // Deliver the sample
    hr = m_pParentFilter->m_pOutputPin->Deliver(pSample);
    pSample->Release();
    pSample = NULL;
    if (FAILED(hr))
    {
        DebugLog("CWavPackDSSplitterInputPin::DoProcessingLoop Deliver failed 0x%08X",hr);
        return hr;
    }

    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::DoProcessingLoop(void)
{
    Command com;
    HRESULT hr;

    Reply(NOERROR);
    m_bAbort = FALSE;
    
    m_pParentFilter->m_pOutputPin->DeliverNewSegment(0,
        m_pParentFilter->m_rtStop - m_pParentFilter->m_rtStart,
        m_pParentFilter->m_dRateSeeking);

    do
    {
        if(m_pIACBW->StreamPos >= m_pIACBW->StreamLen || wavpack_parser_eof(m_pWavPackParser))
        { 
            // EOF
            m_pParentFilter->m_pOutputPin->DeliverEndOfStream();
            // TODO : check if we need to stop the thread
            return NOERROR;
        }

        hr = DeliverOneFrame(m_pWavPackParser);
        if(FAILED(hr))
        {
            return hr;
        }

        // Deliver correction data
        WavPack_parser* wppc = m_pParentFilter->GetWavPackParserCorrection();
        if(wppc != NULL)
        {
            hr = DeliverOneFrame(wppc);
            if(FAILED(hr))
            {
                return hr;
            }
        }
        
    } while (!CheckRequest((DWORD*)&com) && !m_bAbort);

    return NOERROR;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::Active()
{
    HRESULT hr;

    DebugLog("CWavPackDSSplitterInputPin::Active 0x%08X", GetCurrentThreadId());
    
    if(m_pParentFilter->IsActive())
        return S_FALSE;
    if (!IsConnected())
        return NOERROR;
    hr = CBaseInputPin::Active();
    if (FAILED(hr))
        return hr;
    
    // Create and start the thread

    if (!Create())
        return E_FAIL;
    CallWorker(CMD_RUN);

    return NOERROR;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::Inactive()
{
    DebugLog("CWavPackDSSplitterInputPin::Inactive 0x%08X", GetCurrentThreadId());

    // Stop the thread
    if (ThreadExists())
    { 
        m_bAbort = TRUE;
        CallWorker(CMD_EXIT);
        Close();
    }

    return CBasePin::Inactive();
}

// ----------------------------------------------------------------------------

STDMETHODIMP CWavPackDSSplitterInputPin::BeginFlush()
{
    HRESULT hr = CBaseInputPin::BeginFlush();
    if (FAILED(hr))
        return hr;

    DebugLog("CWavPackDSSplitterInputPin::BeginFlush 0x%08X", GetCurrentThreadId());

    if (!ThreadExists())
        return NOERROR;     
    m_bAbort = TRUE;
    CallWorker(CMD_STOP);
    return  NOERROR;
}

// ----------------------------------------------------------------------------

STDMETHODIMP CWavPackDSSplitterInputPin::EndFlush()
{
    HRESULT hr = CBaseInputPin::EndFlush();
    if (FAILED(hr))
        return hr;

    DebugLog("CWavPackDSSplitterInputPin::EndFlush 0x%08X", GetCurrentThreadId());

    if (ThreadExists())
        CallWorker(CMD_RUN);

    return NOERROR; 
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterInputPin::DoSeeking(REFERENCE_TIME rtStart)
{
    DebugLog("CWavPackDSSplitterInputPin::DoSeeking to %I64d ms, 0x%08X", rtStart/10000, GetCurrentThreadId());
    wavpack_parser_seek(m_pWavPackParser, rtStart);
    m_bDiscontinuity = TRUE;
    return S_OK;
}

// ============================================================================

CWavPackDSSplitterOutputPin::CWavPackDSSplitterOutputPin(
    CWavPackDSSplitter *pParentFilter, CCritSec *pLock, HRESULT * phr) :
    CBaseOutputPin(NAME("CWavPackDSSplitterOutputPin"),
              (CBaseFilter *) pParentFilter,
              pLock,
              phr,
              L"Output"),
    m_pParentFilter(pParentFilter)
{
    
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::CheckMediaType(const CMediaType *pmt)
{
    // must have selected input first
    ASSERT(m_pParentFilter->m_pInputPin != NULL);
    if ((m_pParentFilter->m_pInputPin->IsConnected() == FALSE)) {
        return E_INVALIDARG;
    }

    m_pParentFilter->TryToLoadCorrectionFile();

    if ((*pmt->Type() != MEDIATYPE_Audio) ||
        ((*pmt->Subtype() != MEDIASUBTYPE_WAVPACK) &&
		 (*pmt->Subtype() != MEDIASUBTYPE_WavpackHybrid)) &&
        (*pmt->FormatType() != FORMAT_WaveFormatEx))
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return m_pParentFilter->m_pInputPin->CheckMediaType(&m_pParentFilter->m_pInputPin->CurrentMediaType());
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if(!m_pParentFilter->m_pInputPin->IsConnected())
    {
        return E_UNEXPECTED;
    }
    
    if (iPosition < 0)
    {
        return E_INVALIDARG;
    }
    
    if (iPosition > 0)
    {
        return VFW_S_NO_MORE_ITEMS;
    }
    
    WavPack_parser* wpp = m_pParentFilter->GetWavPackParser();
    WavPack_parser* wppc = m_pParentFilter->GetWavPackParserCorrection();
    if(wpp == NULL)
    {
        return E_UNEXPECTED;
	}

    pMediaType->InitMediaType();
    pMediaType->SetType(&MEDIATYPE_Audio);
    if(wppc == NULL)
    {
        pMediaType->SetSubtype(&MEDIASUBTYPE_WAVPACK);
    }
    else
    {
        pMediaType->SetSubtype(&MEDIASUBTYPE_WavpackHybrid);
    }
    pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
    pMediaType->SetVariableSize();
    
    WAVEFORMATEX *pwfxout = (WAVEFORMATEX*)pMediaType->AllocFormatBuffer(
        sizeof(WAVEFORMATEX) + sizeof(wavpack_codec_private_data));
    ZeroMemory(pwfxout, sizeof(WAVEFORMATEX) + sizeof(wavpack_codec_private_data));
    pwfxout->wFormatTag = WAVE_FORMAT_WAVPACK;
	pwfxout->cbSize = sizeof(wavpack_codec_private_data);
	
    pwfxout->wBitsPerSample = wpp->bits_per_sample;
    pwfxout->nChannels = wpp->channel_count;
    pwfxout->nSamplesPerSec = wpp->sample_rate;    

    wavpack_codec_private_data* pd = (wavpack_codec_private_data*)(pwfxout + 1);
    pd->version = wpp->wphdr.version;    
    
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES *pProp)
{

    if(!m_pParentFilter->m_pInputPin->IsConnected())
    {
        return E_UNEXPECTED;
    }

	WavPack_parser* wpp = m_pParentFilter->GetWavPackParser();
    pProp->cBuffers = 2;
	pProp->cbBuffer = wpp->suggested_buffer_size;

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAlloc->SetProperties(pProp, &Actual);
    if(FAILED(hr))
    {
        return hr;
    }
    
    if (Actual.cbBuffer < pProp->cbBuffer ||
        Actual.cBuffers < pProp->cBuffers)
    {
        return E_INVALIDARG;
    }   
    
    return S_OK;
}

// ----------------------------------------------------------------------------
// IMediaSeeking
// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::IsFormatSupported(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    // only seeking in time (REFERENCE_TIME units) is supported
    return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::SetTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    // nothing to set; just check that it's TIME_FORMAT_TIME
    return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : E_INVALIDARG;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::IsUsingTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetTimeFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetDuration(LONGLONG *pDuration)
{
    CheckPointer(pDuration, E_POINTER);
    CAutoLock lock(m_pParentFilter->m_pLock);
    *pDuration = m_pParentFilter->m_rtDuration;
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetStopPosition(LONGLONG *pStop)
{
    CheckPointer(pStop, E_POINTER);
    CAutoLock lock(m_pParentFilter->m_pLock);
    *pStop = m_pParentFilter->m_rtStop;
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    // GetCurrentPosition is typically supported only in renderers and
    // not in source filters.
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetCapabilities(DWORD * pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);
    *pCapabilities = m_pParentFilter->m_dwSeekingCaps;
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::CheckCapabilities(DWORD * pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);
    // make sure all requested capabilities are in our mask
    return (~m_pParentFilter->m_dwSeekingCaps & *pCapabilities) ? S_FALSE : S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::ConvertTimeFormat(LONGLONG * pTarget,
    const GUID * pTargetFormat, LONGLONG Source, const GUID * pSourceFormat)
{
    CheckPointer(pTarget, E_POINTER);
    // format guids can be null to indicate current format

    // since we only support TIME_FORMAT_MEDIA_TIME, we don't really
    // offer any conversions.
    if(pTargetFormat == 0 || *pTargetFormat == TIME_FORMAT_MEDIA_TIME)
    {
        if(pSourceFormat == 0 || *pSourceFormat == TIME_FORMAT_MEDIA_TIME)
        {
            *pTarget = Source;
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::SetPositions(LONGLONG * pCurrent,
    DWORD CurrentFlags, LONGLONG * pStop, DWORD StopFlags)
{

    DWORD StopPosBits = StopFlags & AM_SEEKING_PositioningBitsMask;
    DWORD StartPosBits = CurrentFlags & AM_SEEKING_PositioningBitsMask;

    if(StopFlags) {
        CheckPointer(pStop, E_POINTER);
        // accept only relative, incremental, or absolute positioning
        if(StopPosBits != StopFlags) {
            return E_INVALIDARG;
        }
    }

    if(CurrentFlags) {
        CheckPointer(pCurrent, E_POINTER);
        if(StartPosBits != AM_SEEKING_AbsolutePositioning &&
           StartPosBits != AM_SEEKING_RelativePositioning) {
            return E_INVALIDARG;
        }
    }


    // scope for autolock
    {
        CAutoLock lock(m_pParentFilter->m_pLock);

        // set start position
        if(StartPosBits == AM_SEEKING_AbsolutePositioning)
        {
            m_pParentFilter->m_rtStart = *pCurrent;
        }
        else if(StartPosBits == AM_SEEKING_RelativePositioning)
        {
            m_pParentFilter->m_rtStart += *pCurrent;
        }

        // set stop position
        if(StopPosBits == AM_SEEKING_AbsolutePositioning)
        {
            m_pParentFilter->m_rtStop = *pStop;
        }
        else if(StopPosBits == AM_SEEKING_IncrementalPositioning)
        {
            m_pParentFilter->m_rtStop = m_pParentFilter->m_rtStart + *pStop;
        }
        else if(StopPosBits == AM_SEEKING_RelativePositioning)
        {
            m_pParentFilter->m_rtStop = m_pParentFilter->m_rtStop + *pStop;
        }
    }

    HRESULT hr = S_OK;
    if(StartPosBits) {      
        m_pParentFilter->BeginFlush();
        m_pParentFilter->DoSeeking();
        m_pParentFilter->EndFlush();
    } else if(StopPosBits) {
        // Output a new segment
        m_pParentFilter->BeginFlush();
        m_pParentFilter->EndFlush();
    }
    
    return hr;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetPositions(LONGLONG * pCurrent, LONGLONG * pStop)
{
    if(pCurrent) {
        *pCurrent = m_pParentFilter->m_rtStart;
    }
    if(pStop) {
        *pStop = m_pParentFilter->m_rtStop;
    }
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetAvailable(LONGLONG * pEarliest, LONGLONG * pLatest)
{
    if(pEarliest) {
        *pEarliest = 0;
    }
    if(pLatest) {
        CAutoLock lock(m_pParentFilter->m_pLock);
        *pLatest = m_pParentFilter->m_rtDuration;
    }
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::SetRate(double dRate)
{
    {
        CAutoLock lock(m_pParentFilter->m_pLock);
        m_pParentFilter->m_dRateSeeking = dRate;
    }
    // Output a new segment
    m_pParentFilter->BeginFlush();
    m_pParentFilter->EndFlush();
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetRate(double * pdRate)
{
    CheckPointer(pdRate, E_POINTER);
    CAutoLock lock(m_pParentFilter->m_pLock);
    *pdRate = m_pParentFilter->m_dRateSeeking;
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterOutputPin::GetPreroll(LONGLONG *pPreroll)
{
    CheckPointer(pPreroll, E_POINTER);
    *pPreroll = 0;
    return S_OK;
}

// ============================================================================
// IAsyncCallBackWrapper
// ============================================================================

int32_t IAsyncCallBackWrapper_read_bytes(void *id, void *data, int32_t bcount)
{
    
    IAsyncCallBackWrapper* iacbw = (IAsyncCallBackWrapper*)id;
    HRESULT hr = iacbw->pReader->SyncRead(iacbw->StreamPos, bcount, (BYTE*)data);
    if(hr == S_OK)
	{
        iacbw->StreamPos += bcount;
		return bcount;
	}
    else
    {
		return -1;
	}
}

// ----------------------------------------------------------------------------

uint32_t IAsyncCallBackWrapper_get_pos(void *id)
{
	IAsyncCallBackWrapper* iacbw = (IAsyncCallBackWrapper*)id;
	return (uint32_t)iacbw->StreamPos;
}

// ----------------------------------------------------------------------------

int IAsyncCallBackWrapper_set_pos_abs(void *id, uint32_t pos)
{
	IAsyncCallBackWrapper* iacbw = (IAsyncCallBackWrapper*)id;
	iacbw->StreamPos = min(pos, iacbw->StreamLen);
    if(pos > iacbw->StreamLen)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

// ----------------------------------------------------------------------------

int IAsyncCallBackWrapper_set_pos_rel(void *id, int32_t delta, int mode)
{
    IAsyncCallBackWrapper* iacbw = (IAsyncCallBackWrapper*)id;
    LONGLONG newPos = 0;
    switch(mode)
    {
    case SEEK_SET:
        newPos = delta;
        break;
    case SEEK_CUR:
        newPos = iacbw->StreamPos + delta;
        break;
    case SEEK_END:
        newPos = iacbw->StreamLen + delta;
        break;
    }
    iacbw->StreamPos = constrain(0, newPos, iacbw->StreamLen);

    if((newPos < 0) || (newPos > iacbw->StreamLen))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

// ----------------------------------------------------------------------------

int IAsyncCallBackWrapper_push_back_byte(void *id, int c)
{
	IAsyncCallBackWrapper* iacbw = (IAsyncCallBackWrapper*)id;
	iacbw->StreamPos = constrain(0, iacbw->StreamPos - 1, iacbw->StreamLen);
	return c;
}

// ----------------------------------------------------------------------------

uint32_t IAsyncCallBackWrapper_get_length(void *id)
{
	IAsyncCallBackWrapper* iacbw = (IAsyncCallBackWrapper*)id;
	return (uint32_t)iacbw->StreamLen;
}

// ----------------------------------------------------------------------------

int IAsyncCallBackWrapper_can_seek(void *id)
{
	return 1;
}

// ----------------------------------------------------------------------------

IAsyncCallBackWrapper* IAsyncCallBackWrapper_new(IAsyncReader *pReader)
{
    IAsyncCallBackWrapper* iacbw = new IAsyncCallBackWrapper;
    if(!iacbw)
    {
        return NULL;
    }

    iacbw->iocallback.read_bytes = IAsyncCallBackWrapper_read_bytes;
	iacbw->iocallback.get_pos = IAsyncCallBackWrapper_get_pos;
    iacbw->iocallback.set_pos_abs = IAsyncCallBackWrapper_set_pos_abs;
    iacbw->iocallback.set_pos_rel = IAsyncCallBackWrapper_set_pos_rel;
    iacbw->iocallback.push_back_byte = IAsyncCallBackWrapper_push_back_byte;
    iacbw->iocallback.get_length = IAsyncCallBackWrapper_get_length;
    iacbw->iocallback.can_seek = IAsyncCallBackWrapper_can_seek;
    iacbw->pReader = pReader;
    iacbw->StreamPos = 0;

    // Get total size
    LONGLONG Available;
    pReader->Length(&iacbw->StreamLen, &Available);

    return iacbw;
}

// ----------------------------------------------------------------------------

void IAsyncCallBackWrapper_free(IAsyncCallBackWrapper* iacbw)
{
    delete iacbw;
}

// ============================================================================

CWavPackDSSplitterCorrectionInputPin::CWavPackDSSplitterCorrectionInputPin(
    CWavPackDSSplitter *pParentFilter, CCritSec *pLock, HRESULT * phr) :
    CBaseInputPin(NAME("CWavPackDSSplitterCorrectionInputPin"),
        (CBaseFilter *) pParentFilter,
        pLock,
        phr,
        L"Input"),      
    m_pParentFilter(pParentFilter),
    m_pReader(NULL),
    m_bDiscontinuity(TRUE),
    m_pIACBW(NULL),
    m_pWavPackParser(NULL)
{
    
}

// ----------------------------------------------------------------------------

CWavPackDSSplitterCorrectionInputPin::~CWavPackDSSplitterCorrectionInputPin()
{
    if(m_pWavPackParser)
	{
        wavpack_parser_free(m_pWavPackParser);
		m_pWavPackParser = NULL;
	}
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterCorrectionInputPin::CheckMediaType(const CMediaType *pmt)
{
    if ((*pmt->Type() != MEDIATYPE_Stream) ||
        (*pmt->Subtype() != MEDIASUBTYPE_WAVPACK_CORRECTION_Stream))
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterCorrectionInputPin::CheckConnect(IPin* pPin)
{
    HRESULT hr = CBaseInputPin::CheckConnect(pPin);
    if (FAILED(hr))
        return hr;

    hr = pPin->QueryInterface(IID_IAsyncReader, (void**)&m_pReader); 
    if (FAILED(hr))
        return S_FALSE;

    if(m_pIACBW)
	{
        IAsyncCallBackWrapper_free(m_pIACBW);
		m_pIACBW = NULL;
	}
    m_pIACBW = IAsyncCallBackWrapper_new(m_pReader);        

    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterCorrectionInputPin::BreakConnect(void)
{
    HRESULT hr = CBaseInputPin::BreakConnect();
    if (FAILED(hr))
        return hr;
    
    if(m_pIACBW)
	{
        IAsyncCallBackWrapper_free(m_pIACBW);
		m_pIACBW = NULL;
	}

    if (m_pReader) 
    { 
        m_pReader->Release(); 
        m_pReader = NULL; 
    }
    
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterCorrectionInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
        return hr;
    
    if(m_pWavPackParser)
	{
        wavpack_parser_free(m_pWavPackParser);
		m_pWavPackParser = NULL;
	}
    m_pWavPackParser = wavpack_parser_new((stream_reader*)m_pIACBW, TRUE);
    if(!m_pWavPackParser)
    {
        return E_FAIL;
    }
    
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterCorrectionInputPin::DoSeeking(REFERENCE_TIME rtStart)
{
    DebugLog("CWavPackDSSplitterCorrectionInputPin::DoSeeking to %I64d ms, 0x%08X",
        rtStart/10000, GetCurrentThreadId());
    wavpack_parser_seek(m_pWavPackParser, rtStart);
    m_bDiscontinuity = TRUE;
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterCorrectionInputPin::Active()
{   
    DebugLog("CWavPackDSSplitterCorrectionInputPin::Active 0x%08X", GetCurrentThreadId());
    
    if(m_pParentFilter->IsActive())
        return S_FALSE;
    if (!IsConnected())
        return NOERROR;
        
    return CBaseInputPin::Active();
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSSplitterCorrectionInputPin::Inactive()
{
    DebugLog("CWavPackDSSplitterCorrectionInputPin::Inactive 0x%08X", GetCurrentThreadId());
        
    return CBasePin::Inactive();
}

// ----------------------------------------------------------------------------