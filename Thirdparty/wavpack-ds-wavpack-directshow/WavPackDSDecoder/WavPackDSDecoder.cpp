// ----------------------------------------------------------------------------
// WavPack DirectShow Audio Decoder
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

#include <streams.h>
#include <initguid.h>
#include <mmreg.h>

#include "..\wavpack\wputils.h"
#include "..\wavpacklib\wavpack_common.h"
#include "..\wavpacklib\wavpack_frame.h"
#include "..\wavpacklib\wavpack_buffer_decoder.h"

#include "IWavPackDSDecoder.h"
#include "..\WavPackDS_GUID.h"
#include "WavPackDSDecoder.h"

#include <ks.h>
#include <ksmedia.h>

#include "WavPackDSDecoderAboutProp.h"
#include "WavPackDSDecoderInfoProp.h"


// Returns the sample rate of the specified WavPack file

uint32_t WavpackGetSampleRate (WavpackContext *wpc)
{
  return wpc ? wpc->config.sample_rate : 44100;
}

// Returns the number of channels of the specified WavPack file. Note that
// this is the actual number of channels contained in the file even if the
// OPEN_2CH_MAX flag was specified when the file was opened.

int WavpackGetNumChannels (WavpackContext *wpc)
{
  return wpc ? wpc->config.num_channels : 2;
}


// Get the number of errors encountered so far

int WavpackGetNumErrors (WavpackContext *wpc)
{
  return wpc ? wpc->crc_errors : 0;
}


// Returns the number of bytes used for each sample (1 to 4) in the original
// file. This is required information for the user of this module because the
// audio data is returned in the LOWER bytes of the long buffer and must be
// left-shifted 8, 16, or 24 bits if normalized longs are required.

int WavpackGetBytesPerSample (WavpackContext *wpc)
{
  return wpc ? wpc->config.bytes_per_sample : 2;
}
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

#define WAVPACK_DECODER_NAME L"WavPack Audio Decoder"

static const WCHAR g_wszWavPackDSName[]  = WAVPACK_DECODER_NAME;

// ----------------------------------------------------------------------------

CUnknown *WINAPI CWavPackDSDecoder::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CWavPackDSDecoder *pNewObject = new CWavPackDSDecoder(punk, phr);
    if (!pNewObject)
    {
        *phr = E_OUTOFMEMORY;
    }
    return pNewObject;
}

// ----------------------------------------------------------------------------

CWavPackDSDecoder::CWavPackDSDecoder(LPUNKNOWN lpunk, HRESULT *phr) :
    CTransformFilter(NAME("CWavPackDSDecoder"), lpunk, CLSID_WavPackDSDecoder)
    ,m_Codec(NULL)
    ,m_rtFrameStart(0)
    ,m_TotalSamples(0)
    ,m_SamplesPerBuffer(0)
    ,m_HybridMode(FALSE)
    ,m_MainFrame(NULL)
    ,m_CorrectionFrame(NULL)
    ,m_MainBlockDiscontinuity(TRUE)
    ,m_DecodedFrames(0)
    ,m_CrcError(0)
    ,m_DecodingMode(0)

{

}

// ----------------------------------------------------------------------------

CWavPackDSDecoder::~CWavPackDSDecoder()
{
    if(m_Codec)
    {
        wavpack_buffer_decoder_free(m_Codec);
        m_Codec = NULL;
    }
}

// ----------------------------------------------------------------------------

STDMETHODIMP CWavPackDSDecoder::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if(riid == IID_IWavPackDSDecoder)
        return GetInterface((IWavPackDSDecoder *)this, ppv);
    else if (riid == IID_ISpecifyPropertyPages)
        return GetInterface((ISpecifyPropertyPages *)this, ppv);
    else
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoder::CheckInputType(const CMediaType *mtIn)
{
    if ((*mtIn->Type() != MEDIATYPE_Audio) ||
        ((*mtIn->Subtype() != MEDIASUBTYPE_WAVPACK) &&
         (*mtIn->Subtype() != MEDIASUBTYPE_WavpackHybrid)) ||
        (*mtIn->FormatType() != FORMAT_WaveFormatEx))
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    WAVEFORMATEX *pwfxin = (WAVEFORMATEX*)mtIn->Format();    
    if((pwfxin->wBitsPerSample == 8) ||
        (pwfxin->wBitsPerSample == 16) ||
        (pwfxin->wBitsPerSample == 24) ||
        (pwfxin->wBitsPerSample == 32))
    {
        return S_OK;
    }

    return VFW_E_TYPE_NOT_ACCEPTED;    
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoder::GetMediaType(int iPosition, CMediaType *mtOut)
{
    if (!m_pInput->IsConnected())
    {
        return E_UNEXPECTED;
    }
    
    if (iPosition < 0)
    {
        return E_INVALIDARG;
    }
    
    if (iPosition > 1)
    {
        return VFW_S_NO_MORE_ITEMS;
    }

    WAVEFORMATEX *pwfxin = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
    
    // Some drivers don't like WAVEFORMATEXTENSIBLE when channels are <= 2 so
    // we fall back to a classic WAVEFORMATEX struct in this case 
    
    WAVEFORMATEXTENSIBLE wfex;
    ZeroMemory(&wfex, sizeof(WAVEFORMATEXTENSIBLE));

    bool bUseWavExtensible = (pwfxin->nChannels > 2) ||
        (pwfxin->wBitsPerSample == 24) ||
        (pwfxin->wBitsPerSample == 32);
    
    if(pwfxin->wBitsPerSample == 32)
    {
        wfex.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        wfex.Format.wFormatTag = bUseWavExtensible ?
            WAVE_FORMAT_EXTENSIBLE :
            WAVE_FORMAT_IEEE_FLOAT;
    }
    else
    {
        wfex.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        wfex.Format.wFormatTag = bUseWavExtensible ?
            WAVE_FORMAT_EXTENSIBLE :
            WAVE_FORMAT_PCM;
    }

    wfex.Format.cbSize = bUseWavExtensible ? sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX) : 0;
    wfex.Format.nChannels = pwfxin->nChannels;
    wfex.Format.nSamplesPerSec = pwfxin->nSamplesPerSec;
    wfex.Format.wBitsPerSample = pwfxin->wBitsPerSample;
    wfex.Format.nBlockAlign = (unsigned short)((wfex.Format.nChannels * wfex.Format.wBitsPerSample) / 8);
    wfex.Format.nAvgBytesPerSec = wfex.Format.nSamplesPerSec * wfex.Format.nBlockAlign;
    switch(pwfxin->nChannels)
    {
    case 1:
        wfex.dwChannelMask = KSAUDIO_SPEAKER_MONO;      
        break;
    case 2:
        wfex.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
        break;
    case 3:
        wfex.dwChannelMask = KSAUDIO_SPEAKER_STEREO | SPEAKER_FRONT_CENTER;
        break;
    case 4:
        wfex.dwChannelMask = KSAUDIO_SPEAKER_QUAD;
        break;
    case 5:
        wfex.dwChannelMask = KSAUDIO_SPEAKER_QUAD | SPEAKER_FRONT_CENTER;
        break;
    case 6:
        wfex.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
        break;
    default:
        wfex.dwChannelMask = KSAUDIO_SPEAKER_DIRECTOUT; // XXX : or SPEAKER_ALL ??
        break;
    }
    wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;  
    
    mtOut->SetType(&MEDIATYPE_Audio);
    //if(pwfxin->wBitsPerSample == 32)
    //{
    //    mtOut->SetSubtype(&MEDIASUBTYPE_IEEE_FLOAT);
    //}
    //else
    {
        mtOut->SetSubtype(&MEDIASUBTYPE_PCM);
    }
    
    mtOut->SetFormatType(&FORMAT_WaveFormatEx);
    mtOut->SetFormat( (BYTE*) &wfex, sizeof(WAVEFORMATEX) + wfex.Format.cbSize);
    mtOut->SetTemporalCompression(FALSE);
    
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoder::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    if ((*mtOut->Type() != MEDIATYPE_Audio) ||
        (*mtOut->Subtype() != MEDIASUBTYPE_PCM) ||
        (*mtOut->FormatType() != FORMAT_WaveFormatEx))
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    
    WAVEFORMATEX *pwfxin = (WAVEFORMATEX *)mtIn->Format();
    WAVEFORMATEX *pwfxout = (WAVEFORMATEX *)mtOut->Format();
    if ((pwfxin->nSamplesPerSec != pwfxout->nSamplesPerSec) ||
        (pwfxin->nChannels != pwfxout->nChannels) ||
        (pwfxin->wBitsPerSample != pwfxout->wBitsPerSample))
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return CheckInputType(mtIn);
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoder::DecideBufferSize(IMemAllocator *pAllocator, ALLOCATOR_PROPERTIES *pProperties)
{   
    if (!m_pInput->IsConnected())
    {
        return E_UNEXPECTED;
    }

    WAVEFORMATEX *pwfxin = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();

    // Place for ~100ms
    long OutBuffSize = 0;
    m_SamplesPerBuffer = (pwfxin->nSamplesPerSec / 10);
    // required memory for output "buffer" is 4 * samples * num_channels bytes
    OutBuffSize = 4 * m_SamplesPerBuffer * pwfxin->nChannels;
    //OutBuffSize &= 0xFFFFFFF8;
    
    pProperties->cBuffers = 2;
    pProperties->cbBuffer = OutBuffSize;

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties, &Actual);
    if(FAILED(hr))
    {
        return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer ||
        Actual.cBuffers < pProperties->cBuffers)
    {
        return E_INVALIDARG;
    }
    return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoder::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
    HRESULT hr = CTransformFilter::SetMediaType(direction, pmt);

    if(SUCCEEDED(hr) && (direction == PINDIR_INPUT))
    {        
        WAVEFORMATEX* pwfx = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();      
        if(pwfx->cbSize > 0)
        {
            memcpy(&m_PrivateData,
                (char*)(pwfx+1),
                min(sizeof(wavpack_codec_private_data),pwfx->cbSize));
        }

        m_SamplesPerSec = pwfx->nSamplesPerSec;
        m_Channels = pwfx->nChannels;
        m_BitsPerSample = pwfx->wBitsPerSample;

        m_HybridMode = (*m_pInput->CurrentMediaType().Subtype() == MEDIASUBTYPE_WavpackHybrid);

        
        if(m_HybridMode == TRUE)
        {
            m_DecodingMode = DECODING_MODE_HYBRID;
        }
        else
        {
            m_DecodingMode = DECODING_MODE_LOSSLESS_OR_LOSSY;
        }
    }

    return hr;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoder::StartStreaming()
{
    HRESULT hr = CTransformFilter::StartStreaming();

    m_DecodedFrames = 0;
    m_CrcError = 0;

    if(m_Codec)
    {
        wavpack_buffer_decoder_free(m_Codec);
        m_Codec = NULL;
    }
    
    if(m_MainFrame)
    {
        frame_buffer_free(m_MainFrame);
        m_MainFrame = NULL;
    }
    
    if(m_CorrectionFrame)
    {
        frame_buffer_free(m_CorrectionFrame);
        m_CorrectionFrame = NULL;
    }

    m_Codec = wavpack_buffer_decoder_new();
    m_MainFrame = frame_buffer_new();
    m_CorrectionFrame = frame_buffer_new();
    
    if(m_Codec == NULL || m_MainFrame == NULL || m_CorrectionFrame == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoder::StopStreaming()
{
    HRESULT hr = CTransformFilter::StopStreaming();

    if(m_Codec)
    {
        wavpack_buffer_decoder_free(m_Codec);
        m_Codec = NULL;
    }

    if(m_MainFrame)
    {
        frame_buffer_free(m_MainFrame);
        m_MainFrame = NULL;
    }

    if(m_CorrectionFrame)
    {
        frame_buffer_free(m_CorrectionFrame);
        m_CorrectionFrame = NULL;
    }

    return hr;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoder::Receive(IMediaSample *pSample)
{
    //  Check for other streams and pass them on 
    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps(); 
    if ((pProps->dwStreamId != AM_STREAM_MEDIA) &&
        (pProps->dwStreamId != AM_STREAM_BLOCK_ADDITIONNAL))
    {
        return m_pOutput->Deliver(pSample);
    }
    
    ASSERT(pSample);
    // If no output to deliver to then no point sending us data 
    ASSERT(m_pOutput != NULL);

    HRESULT hr = S_OK;
    BYTE *pSrc, *pDst;
    DWORD SrcLength = pSample->GetActualDataLength();
    hr = pSample->GetPointer(&pSrc);
    if(FAILED(hr))
        return hr;
    
     // Check for minimal block size
    if(SrcLength < (3 * sizeof(uint32_t)))
    {
        return S_OK;
    }

    WAVEFORMATEX* pwfx = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
    BOOL bSeveralBlocks = (pwfx->nChannels > 2);
 
    if(pProps->dwStreamId == AM_STREAM_MEDIA)
    {
        REFERENCE_TIME rtStop;
        if(pSample->IsSyncPoint() == S_OK)
        {
            pSample->GetTime(&m_rtFrameStart, &rtStop);
            m_TotalSamples = 0;
        }

        m_MainBlockDiscontinuity = (pSample->IsDiscontinuity() == S_OK);

        reconstruct_wavpack_frame(
            m_MainFrame,
            &m_CommonFrameData,
            (char*)pSrc,
            SrcLength,            
            TRUE,
            bSeveralBlocks,
            m_PrivateData.version);

        if(m_HybridMode == TRUE)
        {
            // Stop here and wait for correction data
            return S_OK;
        }
    }
    
    if((m_HybridMode == TRUE) && 
       (pProps->dwStreamId == AM_STREAM_BLOCK_ADDITIONNAL))
    {
        // rebuild correction data block
        reconstruct_wavpack_frame(
            m_CorrectionFrame,
            &m_CommonFrameData,
            (char*)pSrc,
            SrcLength,
            FALSE,
            bSeveralBlocks,
            m_PrivateData.version);
    }

    if(wavpack_buffer_decoder_load_frame(m_Codec, m_MainFrame->data, m_MainFrame->len,
        m_HybridMode ? m_CorrectionFrame->data : NULL, m_CorrectionFrame->len) == 0)
    {
        // Something is wrong
        return S_FALSE;
    }
   
    // We can precise the decoding mode now
    if(m_HybridMode == FALSE)
    {
        if(m_CommonFrameData.array_flags[0] & WV_HYBRID_FLAG)
        {
            m_DecodingMode = DECODING_MODE_LOSSY;
        }
        else
        {
            m_DecodingMode = DECODING_MODE_LOSSLESS;
        }
    }

    uint32_t samplesLeft = m_CommonFrameData.block_samples;
    while(samplesLeft > 0)
    {
        // Set up the output sample
        IMediaSample *pOutSample;
        hr = InitializeOutputSample(pSample, &pOutSample);
        if(FAILED(hr))
        {
            break;
        }
    
        DWORD DstLength = pOutSample->GetSize();
        hr = pOutSample->GetPointer(&pDst);
        if(FAILED(hr))
        {
            pOutSample->Release();
            break;
        }

        DstLength &= 0xFFFFFFF8;
    
        long samples = wavpack_buffer_decoder_unpack(m_Codec,(int32_t *)pDst, m_SamplesPerBuffer);
        if(samples)
        {
            wavpack_buffer_format_samples(m_Codec,
                (uchar *) pDst,
                (long*) pDst,
                samples);
            
            DstLength = samples *
                WavpackGetBytesPerSample(m_Codec->wpc) *
                WavpackGetNumChannels (m_Codec->wpc);

            pOutSample->SetActualDataLength(DstLength);
            
            REFERENCE_TIME rtStart, rtStop;
            rtStart = m_rtFrameStart + (REFERENCE_TIME)(((double)m_TotalSamples / WavpackGetSampleRate(m_Codec->wpc)) * 10000000);
            m_TotalSamples += samples;
            rtStop = m_rtFrameStart + (REFERENCE_TIME)(((double)m_TotalSamples / WavpackGetSampleRate(m_Codec->wpc)) * 10000000);

            if(rtStart < 0 && rtStop < 0)
            {
                // No need to deliver this sample it will be skipped
                pOutSample->Release();
                continue;
            }
            pOutSample->SetTime(&rtStart, &rtStop);
            pOutSample->SetSyncPoint(TRUE);
            pOutSample->SetDiscontinuity(m_MainBlockDiscontinuity);
            if(m_MainBlockDiscontinuity == TRUE)
            {
                m_MainBlockDiscontinuity = FALSE;
            }

            hr = m_pOutput->Deliver(pOutSample);
            if(FAILED(hr))
            {
                pOutSample->Release();
                break;
            }
            pOutSample->Release();
        }
        else
        {
            pOutSample->Release();
            break;
        }
        samplesLeft -= samples;
    }
    
    m_DecodedFrames++;
    m_CrcError = WavpackGetNumErrors(m_Codec->wpc);
    
    return S_OK;
}

// ============================================================================

// ISpecifyPropertyPages

STDMETHODIMP CWavPackDSDecoder::GetPages(CAUUID *pPages)
{
    pPages->cElems = 2;
    pPages->pElems = (GUID *)CoTaskMemAlloc(pPages->cElems * sizeof(GUID));
    if (!pPages->pElems)
        return E_OUTOFMEMORY;
    
    pPages->pElems[0] = CLSID_WavPackDSDecoderInfoProp;
    pPages->pElems[1] = CLSID_WavPackDSDecoderAboutProp;
    
    return S_OK;
}

// ============================================================================

STDMETHODIMP CWavPackDSDecoder::get_SampleRate(int* sample_rate)
{
    CheckPointer(sample_rate, E_POINTER);
    CAutoLock lock(&m_WPDSLock);
    *sample_rate = m_SamplesPerSec;
    return S_OK;
}

STDMETHODIMP CWavPackDSDecoder::get_Channels(int *channels)
{
    CheckPointer(channels, E_POINTER);
    CAutoLock lock(&m_WPDSLock);
    *channels = m_Channels;
    return S_OK;
}

STDMETHODIMP CWavPackDSDecoder::get_BitsPerSample(int *bits_per_sample)
{
    CheckPointer(bits_per_sample, E_POINTER);
    CAutoLock lock(&m_WPDSLock);
    *bits_per_sample = m_BitsPerSample;
    return S_OK;
}

STDMETHODIMP CWavPackDSDecoder::get_FramesDecoded(int *frames_decoded)
{
    CheckPointer(frames_decoded,E_POINTER);
    CAutoLock lock(&m_WPDSLock);
    *frames_decoded = m_DecodedFrames;
    return S_OK;
}

STDMETHODIMP CWavPackDSDecoder::get_CrcErrors(int *crc_errors)
{
    CheckPointer(crc_errors, E_POINTER);
    CAutoLock lock(&m_WPDSLock);
    *crc_errors = m_CrcError;
    return S_OK;
}

STDMETHODIMP CWavPackDSDecoder::get_DecodingMode(int *decoding_mode)
{
    CheckPointer(decoding_mode, E_POINTER);
    CAutoLock lock(&m_WPDSLock);
    *decoding_mode = m_DecodingMode;
    return S_OK;
}

// ============================================================================