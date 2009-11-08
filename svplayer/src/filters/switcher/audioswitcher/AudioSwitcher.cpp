/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StdAfx.h"

#include "Shlwapi.h"
#include <math.h>
#include <atlpath.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#include "AudioSwitcher.h"
#include "Audio.h"
#include "..\..\..\DSUtil\DSUtil.h"

#include <initguid.h>
#include "..\..\..\..\include\moreuuids.h"
#include "..\..\..\svplib\svplib.h"
#include <afxtempl.h>
#include "..\..\..\apps\mplayerc\mplayerc.h"

//#define TRACE SVP_LogMsg5
#define SVP_LogMsg5 __noop

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CAudioSwitcherFilter), L"AudioSwitcher", MERIT_DO_NOT_USE, countof(sudpPins), sudpPins}
};

CFactoryTemplate g_Templates[] =
{
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CAudioSwitcherFilter>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif

//
// CAudioSwitcherFilter
//

CAudioSwitcherFilter::CAudioSwitcherFilter(LPUNKNOWN lpunk, HRESULT* phr)
	: CStreamSwitcherFilter(lpunk, phr, __uuidof(this))
	, m_fCustomChannelMapping2(-1)
	, m_fDownSampleTo441(false)
	, m_rtAudioTimeShift(0)
	, m_rtNextStart(0)
	, m_rtNextStop(1)
	, m_fNormalize(false)
	, m_fNormalizeRecover(false)
	, m_boost(1)
	, m_sample_max(0.1f)
	, m_l_number_of_channels(-1)
	, m_bNoMoreCheckConnection(0)
	, m_lTotalOutputChannel(2)
	, m_lastInputChannelCount(-1)
	, m_lastOutputChannelCount(-1)
	, m_iSimpleSwitch(-1)
	, m_fVolSuggested(0)
	, m_fEQControlOn(0)
	, m_tPlayedtime(0)
{
	//memset(m_pSpeakerToChannelMap, 0, sizeof(m_pSpeakerToChannelMap));
	memset(m_pChannelNormalize2, 0, sizeof(m_pChannelNormalize2));
	memset(m_pEQBandControlCurrent, 0, sizeof(m_pEQBandControlCurrent));

	m_tPlayedtime = time(NULL);
	if(phr)
	{
		if(FAILED(*phr)) return;
		else *phr = S_OK;
	}
}

STDMETHODIMP CAudioSwitcherFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IAudioSwitcherFilter)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAudioSwitcherFilter::CheckMediaType(const CMediaType* pmt)
{
	if(pmt->formattype == FORMAT_WaveFormatEx
	&& ((WAVEFORMATEX*)pmt->pbFormat)->nChannels > 2
	&& ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
		return VFW_E_INVALIDMEDIATYPE; // stupid iviaudio tries to fool us

	HRESULT hr = (pmt->majortype == MEDIATYPE_Audio
			&& pmt->formattype == FORMAT_WaveFormatEx
			&& (((WAVEFORMATEX*)pmt->pbFormat)->wBitsPerSample == 8
				|| ((WAVEFORMATEX*)pmt->pbFormat)->wBitsPerSample == 16
				|| ((WAVEFORMATEX*)pmt->pbFormat)->wBitsPerSample == 24
				|| ((WAVEFORMATEX*)pmt->pbFormat)->wBitsPerSample == 32)
			&& (((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_PCM
				|| ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_IEEE_FLOAT
				|| ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_DOLBY_AC3_SPDIF
				|| ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag == WAVE_FORMAT_EXTENSIBLE))
		? S_OK
		: VFW_E_TYPE_NOT_ACCEPTED;

	return hr ;
}

template<class T, class U, int Umin, int Umax> 
void mix(float mask[MAX_NORMALIZE_CHANNELS], int ch, int bps, BYTE* src, BYTE* dst)
{
	double sum = 0;
	
	BOOL setoffsetfor8bits = true;
	if(Umin < 0){
		//normal one
		setoffsetfor8bits = false;
	}else{
		// 0 -255
	}
	for(int i = 0, j = min(MAX_NORMALIZE_CHANNELS, ch); i < j; i++)
	{
		if(mask[i] > 0)
		{
			U tmpData = *(T*)&src[bps*i];
			if(setoffsetfor8bits)
			{
				tmpData -= Umax/2;
			}
			
			sum += tmpData * (double)mask[i] ;
			
		}
	}

	if(setoffsetfor8bits)
	{
		sum += Umax/2;
	}
	if(sum < Umin) sum = Umin;
	if(sum > Umax) sum = Umax;
	
	*(T*)dst = (T)sum;
}

template<> 
void mix<int, INT64, (-1<<24), (+1<<24)-1>(float mask[MAX_NORMALIZE_CHANNELS], int ch, int bps, BYTE* src, BYTE* dst)
{
	INT64 sum = 0;
	for(int i = 0, j = min(MAX_NORMALIZE_CHANNELS, ch); i < j; i++)
	{
		if(mask[i] > 0)
		{
			int tmp;
			memcpy((BYTE*)&tmp+1, &src[bps*i], 3);
			sum += (tmp >> 8) * (double)mask[i];
		}
	}

	sum = min(max(sum, (-1<<24)), (+1<<24)-1);

	memcpy(dst, (BYTE*)&sum, 3);
}


#define MAX_MUL 6
#define SMOOTH_RATE  15  //max is more smooth
//Plot[10*x/Sqrt[ (x*10)^2 + (MAX_MUL * SMOOTH_RATE)/k - (SMOOTH_RATE -4 )],  {x, -1, 1}, {k,1,MAX_MUL} ]
template<class T>
T clamp(double s, T smin, T smax, double k)
{
	//if(s < -1) s = -1;
	//else if(s > 1) s = 1;
	if(k > 1.0){
		s = 10 * s / sqrt( (s * s * 100) + (MAX_MUL * SMOOTH_RATE)/k - (SMOOTH_RATE -4 )) ;
	}
	
	
	T t = (T)( (s+1.0)/2.0 * ((double)smax - smin) + smin );

	//T t = (T)(s * smax);

	if(t < smin) t = smin;
	else if(t > smax) t = smax;
	return t;
}


HRESULT CAudioSwitcherFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
	CStreamSwitcherInputPin* pInPin = GetInputPin();
	CStreamSwitcherOutputPin* pOutPin = GetOutputPin();
	if(!pInPin || !pOutPin) 
		return __super::Transform(pIn, pOut);

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)pInPin->CurrentMediaType().pbFormat;
	WAVEFORMATEX* wfeout = (WAVEFORMATEX*)pOutPin->CurrentMediaType().pbFormat;
	WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)wfe;
	WAVEFORMATEXTENSIBLE* wfexout = (WAVEFORMATEXTENSIBLE*)wfeout;

	int bps = wfe->wBitsPerSample>>3;

	int len = pIn->GetActualDataLength() / (bps*wfe->nChannels);
	int lenout = len * wfeout->nSamplesPerSec / wfe->nSamplesPerSec;

	REFERENCE_TIME rtStart, rtStop;
	if(SUCCEEDED(pIn->GetTime(&rtStart, &rtStop)))
	{
		rtStart += m_rtAudioTimeShift;
		rtStop += m_rtAudioTimeShift;
		pOut->SetTime(&rtStart, &rtStop);

		m_rtNextStart = rtStart;
		m_rtNextStop = rtStop;
	}
	else
	{
		pOut->SetTime(&m_rtNextStart, &m_rtNextStop);
	}

	REFERENCE_TIME rtDur = 10000000i64*len/wfe->nSamplesPerSec;

	m_rtNextStart += rtDur;
	m_rtNextStop += rtDur;

	if(pIn->IsDiscontinuity() == S_OK)
	{
		m_sample_max = 0.1f;
	}

	WORD tag = wfe->wFormatTag;
	bool fPCM = tag == WAVE_FORMAT_PCM || tag == WAVE_FORMAT_EXTENSIBLE && wfex->SubFormat == KSDATAFORMAT_SUBTYPE_PCM;
	bool fFloat = tag == WAVE_FORMAT_IEEE_FLOAT || tag == WAVE_FORMAT_EXTENSIBLE && wfex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	if(!fPCM && !fFloat) return __super::Transform(pIn, pOut);

	BYTE* pDataIn = NULL;
	BYTE* pDataOut = NULL;

	HRESULT hr;
	if(FAILED(hr = pIn->GetPointer(&pDataIn))) return hr;
	if(FAILED(hr = pOut->GetPointer(&pDataOut))) return hr;

	if(!pDataIn || !pDataOut || len <= 0 || lenout <= 0) return S_FALSE;

	memset(pDataOut, 0, pOut->GetSize());
	
	int iWePCMType = WETYPE_UNKNOWN;

	if(fPCM && wfe->wBitsPerSample == 8) iWePCMType = WETYPE_PCM8;
	else if(fPCM && wfe->wBitsPerSample == 16) iWePCMType = WETYPE_PCM16;
	else if(fPCM && wfe->wBitsPerSample == 24) iWePCMType = WETYPE_PCM24;
	else if(fPCM && wfe->wBitsPerSample == 32) iWePCMType = WETYPE_PCM32;
	else if(fFloat && wfe->wBitsPerSample == 32) iWePCMType = WETYPE_FPCM32;
	else if(fFloat && wfe->wBitsPerSample == 64) iWePCMType = WETYPE_FPCM64;

	int lTotalInputChannels = wfe->nChannels;
	int lTotalOutputChannels =  wfeout->nChannels;
	
	if(m_fCustomChannelMapping2 < 0 ){
		CAutoLock dataLock(&m_csDataLock);
			//SVP_LogMsg5(L"ChanTest %d %d",lTotalInputChannels ,lTotalOutputChannels );
		m_fCustomChannelMapping2 = 0;

		if( (m_iSimpleSwitch > 2 && lTotalInputChannels > 2) || (m_iSimpleSwitch > 0 && m_iSimpleSwitch <= 2 && lTotalInputChannels > 1) ){

			m_fCustomChannelMapping2 = 1;
		}else{
			m_iSimpleSwitch = 0;
		}
		if(!m_fCustomChannelMapping2){
			for (int i = 0; i < lTotalInputChannels; i++)
			{
				if( m_pSpeakerToChannelMapOffset[lTotalInputChannels-1][i]  != 0){
					m_fCustomChannelMapping2 = 1;
					break;
				}
			}
		}
			if(!m_fCustomChannelMapping2){
				for (int j = 0; j < lTotalOutputChannels; j++){
					for (int i = 0; i < lTotalInputChannels; i++)
					{
						if( m_pChannelNormalize2[lTotalInputChannels-1][lTotalOutputChannels-1][j][i] > 0 ){
							m_fCustomChannelMapping2 = 1;
							break;
						}
					}
					if(m_fCustomChannelMapping2)
						break;
				}

			}
			
			if( !m_fCustomChannelMapping2 && lTotalInputChannels > lTotalOutputChannels){

				
				for (int i = 0; i < lTotalOutputChannels; i++)
				{
					for (int j = 0; j < lTotalInputChannels; j++)
					{
						if( i == j || j > i){
							m_pChannelNormalize2[lTotalInputChannels-1][lTotalOutputChannels-1][i][j] = 1.0/((float)lTotalInputChannels / lTotalOutputChannels);
						}
					}
				}
				m_fCustomChannelMapping2 = 1;
			}

	}

	if( m_fCustomChannelMapping2 )
	{
		if(m_fCustomChannelMapping2 == 1 || m_lastInputChannelCount != lTotalInputChannels || m_lastOutputChannelCount != lTotalOutputChannels ){

			CAutoLock dataLock(&m_csDataLock);
			memset(m_pCurrentChannelNormalize2, 0, sizeof(m_pCurrentChannelNormalize2));

			for(int iSpeakerID = 0; iSpeakerID < lTotalOutputChannels; iSpeakerID++)
			{
				float countBase = 0;
				bool bThereisOffset = FALSE;
				for(int iChannelID = 0; iChannelID < lTotalInputChannels; iChannelID++){
					if(m_pChannelNormalize2[lTotalInputChannels-1][lTotalOutputChannels-1][iSpeakerID][iChannelID] > 0){
						countBase+=0.33;
					}
					if(m_pSpeakerToChannelMapOffset[lTotalInputChannels-1][iChannelID] != 0){
						bThereisOffset = true;
					}
				}
				if(countBase > 0){
					for(int iChannelID = 0; iChannelID < lTotalInputChannels; iChannelID++){
						if(m_pSpeakerToChannelMapOffset[lTotalInputChannels-1][iChannelID] != 0){
							m_pCurrentChannelNormalize2[iSpeakerID][iChannelID] = 
								( m_pChannelNormalize2[lTotalInputChannels-1][lTotalOutputChannels-1][iSpeakerID][iChannelID] 
								* ( m_pSpeakerToChannelMapOffset[lTotalInputChannels-1][iChannelID] + 1.0))	/ countBase;
						}else{
							m_pCurrentChannelNormalize2[iSpeakerID][iChannelID] = 
								m_pChannelNormalize2[lTotalInputChannels-1][lTotalOutputChannels-1][iSpeakerID][iChannelID]	/ countBase;

						}
						
						//SVP_LogMsg5(L"%f %f" , m_pSpeakerToChannelMapOffset[lTotalInputChannels-1][iChannelID] , m_pCurrentChannelNormalize2[iSpeakerID][iChannelID]);
					}
				}

				if(countBase <= 0 && bThereisOffset && iSpeakerID < lTotalInputChannels){
					
					m_pCurrentChannelNormalize2[iSpeakerID][iSpeakerID] = 1.0 * ( m_pSpeakerToChannelMapOffset[lTotalInputChannels-1][iSpeakerID] + 1.0);
					
				}
			}
			if( (m_iSimpleSwitch > 2 && lTotalInputChannels > 2) || (m_iSimpleSwitch > 0 && m_iSimpleSwitch <= 2 && lTotalInputChannels > 1) ){

				for(int iSpeakerID = 0; iSpeakerID < lTotalOutputChannels; iSpeakerID++)
				{
					
						
						switch(m_iSimpleSwitch){
							case 1:
								//Ö»Êä³ö×óÉùµÀ
								if(m_pCurrentChannelNormalize2[iSpeakerID][1] > 0){
									m_pCurrentChannelNormalize2[iSpeakerID][0] = m_pCurrentChannelNormalize2[iSpeakerID][1];
									m_pCurrentChannelNormalize2[iSpeakerID][1] = 0.0;
								}

								break;
							case 2:
								if(m_pCurrentChannelNormalize2[iSpeakerID][0] > 0){
									m_pCurrentChannelNormalize2[iSpeakerID][1] = m_pCurrentChannelNormalize2[iSpeakerID][0];
									m_pCurrentChannelNormalize2[iSpeakerID][0] = 0.0;
								}

								break;
							case 3:
								if(m_pCurrentChannelNormalize2[iSpeakerID][2] > 0){
									m_pCurrentChannelNormalize2[iSpeakerID][0] = m_pCurrentChannelNormalize2[iSpeakerID][2];
									m_pCurrentChannelNormalize2[iSpeakerID][1] = m_pCurrentChannelNormalize2[iSpeakerID][2];
									m_pCurrentChannelNormalize2[iSpeakerID][2] = 0.0;
								}
								
								break;
							}
						}
						
					
				
			}
			m_lastInputChannelCount = lTotalInputChannels;
			m_lastOutputChannelCount = lTotalOutputChannels;
			m_fCustomChannelMapping2 = 2;
		}
		//SVP_LogMsg5(L"m_fCustomChannelMapping2 %d ",m_fCustomChannelMapping2);
			{
				for(int i = 0; i < wfeout->nChannels; i++)
				{
					//DWORD mask = m_chs[wfe->nChannels-1][i].Channel;

					BYTE* src = pDataIn;
					BYTE* dst = &pDataOut[bps*i];

					int srcstep = bps*wfe->nChannels;
					int dststep = bps*wfeout->nChannels;

					switch(iWePCMType){
						case WETYPE_PCM8:
							for(int k = 0; k < len; k++, src += srcstep, dst += dststep)
							{
								mix<unsigned char, INT64, 0, UCHAR_MAX>(m_pCurrentChannelNormalize2[i]
									, wfe->nChannels, bps, src, dst);
							}
							break;
						case WETYPE_PCM16:
							for(int k = 0; k < len; k++, src += srcstep, dst += dststep)
							{
								mix<short, INT64, SHRT_MIN, SHRT_MAX>(m_pCurrentChannelNormalize2[i],
									wfe->nChannels, bps, src, dst);
							}
							break;
						case WETYPE_PCM24:
							for(int k = 0; k < len; k++, src += srcstep, dst += dststep)
							{
								mix<int, INT64, (-1<<24), (+1<<24)-1>(m_pCurrentChannelNormalize2[i],
									 wfe->nChannels, bps, src, dst);
							}
							break;
						case WETYPE_PCM32:

							for(int k = 0; k < len; k++, src += srcstep, dst += dststep)
							{
								mix<int, __int64, INT_MIN, INT_MAX>(m_pCurrentChannelNormalize2[i],
									wfe->nChannels, bps, src, dst);
							}
							break;
						case WETYPE_FPCM32:
							for(int k = 0; k < len; k++, src += srcstep, dst += dststep)
							{
								mix<float, double, -1, 1>(m_pCurrentChannelNormalize2[i],
									wfe->nChannels, bps, src, dst);
							}
							break;
						case WETYPE_FPCM64:
							for(int k = 0; k < len; k++, src += srcstep, dst += dststep)
							{
								mix<double, double, -1, 1>(m_pCurrentChannelNormalize2[i],
									wfe->nChannels, bps, src, dst);
							}
							break;
					}

				}
			}

			

	}
	else
	{
		//SVP_LogMsg5(L"No channel maping");
		HRESULT hr;
		if(S_OK != (hr = __super::Transform(pIn, pOut))){
			TRACE(L"FAUK");
			return hr;
		}
	}
/*
	if(m_fDownSampleTo441
	&& wfe->nSamplesPerSec > 44100 && wfeout->nSamplesPerSec == 44100 
	&& wfe->wBitsPerSample <= 16 && fPCM)
	{
		if(BYTE* buff = new BYTE[len*bps])
		{
			for(int ch = 0; ch < wfeout->nChannels; ch++)
			{
				memset(buff, 0, len*bps);

				for(int i = 0; i < len; i++)
					memcpy(buff + i*bps, (char*)pDataOut + (ch + i*wfeout->nChannels)*bps, bps);

				m_pResamplers[ch]->Downsample(buff, len, buff, lenout);

				for(int i = 0; i < lenout; i++)
					memcpy((char*)pDataOut + (ch + i*wfeout->nChannels)*bps, buff + i*bps, bps);
			}

			delete [] buff;
		}
	}
*/
	
	if(m_fNormalize || m_boost > 1 || m_fEQControlOn)
	{
		int samples = lenout*wfeout->nChannels;
		
		if(double* buff = new double[samples])
		{
			
			{
				switch(iWePCMType){
						case WETYPE_PCM8:
							for(int i = 0; i < samples; i++)
								buff[i] = (((double)((BYTE*)pDataOut)[i]) - 0x7f) / 0x80;//UCHAR_MAX;
							break;
						case WETYPE_PCM16:
							for(int i = 0; i < samples; i++)
								buff[i] = (double)((short*)pDataOut)[i] / SHRT_MAX;
							break;
						case WETYPE_PCM24:
							for(int i = 0; i < samples; i++)
							{int tmp; memcpy(((BYTE*)&tmp)+1, &pDataOut[i*3], 3); buff[i] = (float)(tmp >> 8) / ((1<<23)-1);}
							break;
						case WETYPE_PCM32:
							for(int i = 0; i < samples; i++)
								buff[i] = (double)((int*)pDataOut)[i] / INT_MAX;
							break;
						case WETYPE_FPCM32:
							for(int i = 0; i < samples; i++)
								buff[i] = (double)((float*)pDataOut)[i];
							break;
						case WETYPE_FPCM64:
							for(int i = 0; i < samples; i++)
								buff[i] = ((double*)pDataOut)[i];
							break;
				}

				
			}
			
			double sample_mul = 1;


			if(m_fNormalize || m_boost > 1){

				for(int i = 0; i < samples; i++)
				{
					double s = buff[i];
					if(s < 0) s = -s;
					if(s > 1) s = 1;
					if(m_sample_max < s) m_sample_max = s;
				}

				
				//if(m_fNormalizeRecover) 
					
				if(m_fNormalize)
				{
					m_sample_max -= 1.0*rtDur/200000000; // -5%/sec
					if(m_sample_max < 0.25) m_sample_max = 0.25; // not more than 4x volume
					sample_mul = 1.0f / m_sample_max;
				}else{
					if( m_boost > 1)
					{
						sample_mul = (1+log10(m_boost));
						
						if( (time(NULL)-m_tPlayedtime) < 60 ){
							BOOL bMulChanged = FALSE;
							
							double old_sample_mul = sample_mul;
							
							while(sample_mul > 1 && clamp<double>( m_sample_max , -1, +1 , sample_mul) > 0.95){
								//m_boost = 1;

								bMulChanged = TRUE;
								sample_mul -= 0.5;

							}
							double f_suggest_sample_mul = sample_mul;
							if(!m_fVolSuggested){
								while(f_suggest_sample_mul > 1 && clamp<double>( m_sample_max , -1, +1 , f_suggest_sample_mul) > 0.85){
									//m_boost = 1;

									bMulChanged = TRUE;
									f_suggest_sample_mul -= 0.5;

								}
							}
							//float fSuggestVol = sample_mul;

							if(bMulChanged){
								
								if(!m_fVolSuggested){
									
									::SendMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_USER+32, (WPARAM)&f_suggest_sample_mul,0);
									sample_mul = 1;
								}

								m_fVolSuggested = TRUE;

								//SVP_LogMsg5(L"SafeVol1 %f %f" , old_sample_mul, m_boost);
								if(sample_mul <= 1){
									m_boost = 1;
								}else{
									m_boost = pow((double)10, (sample_mul-1));
								}
								//SVP_LogMsg5(L"SafeVol2 %f %f" , sample_mul, m_boost);
							}
						}
						
					}
				
				}

		

			}

			SVP_LogMsg5(L"EQ ON %d"  , m_fEQControlOn);
			if(m_fEQControlOn){
				CAutoLock dataLock(&m_csEQLock);
				int samplesPerChannel = lenout;
				m_EQualizer.m_rate = wfeout->nSamplesPerSec;
				m_EQualizer.EqzFilter(buff, buff, samplesPerChannel , wfeout->nChannels );
			}

			if(sample_mul <= 1)
				sample_mul = 1;
			
			if(m_fEQControlOn || sample_mul > 1 ){
				//SVP_LogMsg5(L"maul %f %f %f %f" ,m_boost , log10(m_boost) , sample_mul, sample_mul * (1+log10(m_boost)) );

				switch(iWePCMType){
								case WETYPE_PCM8:
									for(int i = 0; i < samples; i++)
										((BYTE*)pDataOut)[i] = clamp<BYTE>( buff[i] , 0, UCHAR_MAX , sample_mul);
									break;
								case WETYPE_PCM16:
									for(int i = 0; i < samples; i++)
										((short*)pDataOut)[i] = clamp<short>( buff[i] , SHRT_MIN, SHRT_MAX , sample_mul);
									break;
								case WETYPE_PCM24:
									for(int i = 0; i < samples; i++)
									{int tmp = clamp<int>( buff[i] , -1<<23, (1<<23)-1 , sample_mul); memcpy(&pDataOut[i*3], &tmp, 3);}
									break;
								case WETYPE_PCM32:
									for(int i = 0; i < samples; i++)
										((int*)pDataOut)[i] = clamp<int>( buff[i] , INT_MIN, INT_MAX , sample_mul);
									break;
								case WETYPE_FPCM32:
									for(int i = 0; i < samples; i++)
										((float*)pDataOut)[i] = clamp<float>( buff[i] , -1, +1 , sample_mul);
									break;
								case WETYPE_FPCM64:
									for(int i = 0; i < samples; i++)
										((double*)pDataOut)[i] = clamp<double>( buff[i] , -1, +1 , sample_mul);
									break;
				}


			}
			delete buff;
		}
	}

	pOut->SetActualDataLength(lenout*bps*wfeout->nChannels);

	return S_OK;
}

CMediaType CAudioSwitcherFilter::CreateNewOutputMediaType(CMediaType mt, long& cbBuffer)
{
	CStreamSwitcherInputPin* pInPin = GetInputPin();
	CStreamSwitcherOutputPin* pOutPin = GetOutputPin();
	if(!pInPin || !pOutPin || ((WAVEFORMATEX*)mt.pbFormat)->wFormatTag == WAVE_FORMAT_DOLBY_AC3_SPDIF) 
		return __super::CreateNewOutputMediaType(mt, cbBuffer);

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)pInPin->CurrentMediaType().pbFormat;

	/*
	
	
		if(!m_bNoMoreCheckConnection){
			CComPtr<IEnumFilters> pEnumFilters; 
			if(m_pGraph && SUCCEEDED(m_pGraph->EnumFilters(&pEnumFilters))) 
			{ 
				for(CComPtr<IBaseFilter> pBaseFilter; S_OK == pEnumFilters->Next(1, &pBaseFilter, 0); pBaseFilter = NULL) 
				{ 
					CLSID clsid;
					memcpy(&clsid, &GUID_NULL, sizeof(clsid));
					pBaseFilter->GetClassID(&clsid);
	
					if( clsid == CLSID_DSoundRender || clsid == CLSID_AudioRender ){
	
						{
							int nIn, nOut, nInC, nOutC;
							CountPins(pBaseFilter, nIn, nOut, nInC, nOutC);
	
							if(nInC > 0 && nOut == 0 && CComQIPtr<IBasicAudio>(pBaseFilter))
							{
								BeginEnumPins(pBaseFilter, pEP, pPin)
								{
	
									
									{
										AM_MEDIA_TYPE mt;
										if(S_OK != pPin->ConnectionMediaType(&mt))
											continue;
										if(mt.majortype == MEDIATYPE_Audio )
										{
	
											SVP_LogMsg5(L"wfe->nChannels %d %d %s", wfe->nChannels , ((WAVEFORMATEX*)mt.pbFormat)->nChannels , CStringFromGUID(clsid));//&& !mt.bTemporalCompression
										}
										FreeMediaType(mt);
	
										continue;;
									}
									
								}
								EndEnumPins
							}
						}
						
	
						m_bNoMoreCheckConnection = true;
						
						break;
					}
	
				}
			}
		}

		if(m_l_number_of_channels < 0){
		m_l_number_of_channels = AfxGetMyApp()->GetNumberOfSpeakers();
		}
		*/
	if(m_lTotalOutputChannel > wfe->nChannels)
		m_lTotalOutputChannel = wfe->nChannels;
	
	if(m_lTotalOutputChannel < wfe->nChannels )
	{
		//m_chs[wfe->nChannels-1].RemoveAll();
		/*
		DWORD mask = DWORD((__int64(1)<<wfe->nChannels)-1);
		for(int i = 0; i < 18; i++)
		{
			if(m_pSpeakerToChannelMap[wfe->nChannels-1][i]&mask)
			{
				ChMap cm = {1<<i, m_pSpeakerToChannelMap[wfe->nChannels-1][i]};
				m_chs[wfe->nChannels-1].Add(cm);
			}
		}
		*/

		if(m_lTotalOutputChannel > 0)
		{
			mt.ReallocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));
			WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)mt.pbFormat;
			wfex->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
			wfex->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			wfex->Samples.wValidBitsPerSample = wfe->wBitsPerSample;
			wfex->SubFormat = 
				wfe->wFormatTag == WAVE_FORMAT_PCM ? KSDATAFORMAT_SUBTYPE_PCM :
				wfe->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT :
				wfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? ((WAVEFORMATEXTENSIBLE*)wfe)->SubFormat :
				KSDATAFORMAT_SUBTYPE_PCM; // can't happen

			wfex->dwChannelMask = 0;
			for(int i = 0; i < m_lTotalOutputChannel; i++)
				wfex->dwChannelMask |= (1 << i);

			wfex->Format.nChannels = (WORD)m_lTotalOutputChannel;
			wfex->Format.nBlockAlign = wfex->Format.nChannels*wfex->Format.wBitsPerSample>>3;
			wfex->Format.nAvgBytesPerSec = wfex->Format.nBlockAlign*wfex->Format.nSamplesPerSec;

		}
	}

	WAVEFORMATEX* wfeout = (WAVEFORMATEX*)mt.pbFormat;

	if(m_fDownSampleTo441)
	{
		if(wfeout->nSamplesPerSec > 44100 && wfeout->wBitsPerSample <= 16)
		{
			wfeout->nSamplesPerSec = 44100;
			wfeout->nAvgBytesPerSec = wfeout->nBlockAlign*wfeout->nSamplesPerSec;
		}
	}

	int bps = wfe->wBitsPerSample>>3;
	int len = cbBuffer / (bps*wfe->nChannels);
	int lenout = len * wfeout->nSamplesPerSec / wfe->nSamplesPerSec;
	cbBuffer = lenout*bps*wfeout->nChannels;

//	mt.lSampleSize = (ULONG)max(mt.lSampleSize, wfe->nAvgBytesPerSec * rtLen / 10000000i64);
//	mt.lSampleSize = (mt.lSampleSize + (wfe->nBlockAlign-1)) & ~(wfe->nBlockAlign-1);
	//mt.lSampleSize  = 14880;
	//mt.bTemporalCompression = 0;
	return mt;
}
STDMETHODIMP CAudioSwitcherFilter::ResetAudioSwitch(){

	
	return S_OK;
}
void CAudioSwitcherFilter::OnNewOutputMediaType(const CMediaType& mtIn, const CMediaType& mtOut)
{
	const WAVEFORMATEX* wfe = (WAVEFORMATEX*)mtIn.pbFormat;
	const WAVEFORMATEX* wfeout = (WAVEFORMATEX*)mtOut.pbFormat;

	m_pResamplers.RemoveAll();
	for(int i = 0; i < wfeout->nChannels; i++)
	{
		CAutoPtr<AudioStreamResampler> pResampler;
		pResampler.Attach(new AudioStreamResampler(wfeout->wBitsPerSample>>3, wfe->nSamplesPerSec, wfeout->nSamplesPerSec, true));
		m_pResamplers.Add(pResampler);
	}

	TRACE(_T("CAudioSwitcherFilter::OnNewOutputMediaType\n"));
	m_sample_max = 0.1f;
}

HRESULT CAudioSwitcherFilter::DeliverEndFlush()
{
	TRACE(_T("CAudioSwitcherFilter::DeliverEndFlush\n"));
	m_sample_max = 0.1f;
	return __super::DeliverEndFlush();
}

HRESULT CAudioSwitcherFilter::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	TRACE(_T("CAudioSwitcherFilter::DeliverNewSegment\n"));
	
	m_sample_max = 0.1f;
	return __super::DeliverNewSegment(tStart, tStop, dRate);
}

// IAudioSwitcherFilter

STDMETHODIMP CAudioSwitcherFilter::GetInputSpeakerConfig(DWORD* pdwChannelMask)
{
	if(!pdwChannelMask) 
		return E_POINTER;

	*pdwChannelMask = 0;

	CStreamSwitcherInputPin* pInPin = GetInputPin();
	if(!pInPin || !pInPin->IsConnected())
		return E_UNEXPECTED;

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)pInPin->CurrentMediaType().pbFormat;

	if(wfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)wfe;
		*pdwChannelMask = wfex->dwChannelMask;
	}
	else
	{
		*pdwChannelMask = 0/*wfe->nChannels == 1 ? 4 : wfe->nChannels == 2 ? 3 : 0*/;
	}

	return S_OK;
}
STDMETHODIMP CAudioSwitcherFilter::SetSpeakerChannelConfig (int lTotalOutputChannel , 
															float pChannelNormalize[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS]
															,float pSpeakerToChannelMapOffset[MAX_INPUT_CHANNELS][MAX_NORMALIZE_CHANNELS], int iSimpleSwitch )
{
	if(m_State == State_Stopped || m_pChannelNormalize2 != pChannelNormalize
		|| memcmp(m_pChannelNormalize2, pChannelNormalize, sizeof(m_pChannelNormalize2)) ||
		memcmp(m_pSpeakerToChannelMapOffset, pSpeakerToChannelMapOffset, sizeof(m_pSpeakerToChannelMapOffset)) )
	{
		//PauseGraph;

		//CStreamSwitcherInputPin* pInput = GetInputPin();

		//SelectInput(NULL);
		CAutoLock dataLock(&m_csDataLock);

		//SVP_LogMsg5(L"Set channel maping");
		memcpy(m_pChannelNormalize2, pChannelNormalize, sizeof(m_pChannelNormalize2));
		memcpy(m_pSpeakerToChannelMapOffset, pSpeakerToChannelMapOffset, sizeof(m_pSpeakerToChannelMapOffset));
		if(lTotalOutputChannel > 0)
			m_lTotalOutputChannel = lTotalOutputChannel;

		m_fCustomChannelMapping2 = -1;

		if(iSimpleSwitch >= 0)
			m_iSimpleSwitch = iSimpleSwitch;

		//SVP_LogMsg5(L"seted %fd %f" ,  m_pChannelNormalize2[5][1][0][1] , pChannelNormalize[5][1][0][1] );

		//SelectInput(pInput);

		//ResumeGraph;
	}

	return S_OK;

}
STDMETHODIMP CAudioSwitcherFilter::GetSpeakerChannelConfig (int *plTotalOutputChannel , 
															float pChannelNormalize[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS])
{
	memcpy(pChannelNormalize, m_pChannelNormalize2, sizeof(m_pChannelNormalize2));
	if(plTotalOutputChannel)
	{
		*plTotalOutputChannel = m_lTotalOutputChannel;
	}
	return S_OK;
}
/*
STDMETHODIMP CAudioSwitcherFilter::GetSpeakerConfig(bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18])
{
	if(pfCustomChannelMapping) *pfCustomChannelMapping = m_fCustomChannelMapping;
	memcpy(pSpeakerToChannelMap, m_pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap));

	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::SetSpeakerConfig(bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18])
{
	fCustomChannelMapping = true;
	if(m_State == State_Stopped || m_fCustomChannelMapping != fCustomChannelMapping
	|| memcmp(m_pSpeakerToChannelMap, pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap)))
	{
		PauseGraph;
		
		CStreamSwitcherInputPin* pInput = GetInputPin();

		SelectInput(NULL);

		m_fCustomChannelMapping = fCustomChannelMapping;
		memcpy(m_pSpeakerToChannelMap, pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap));

		SelectInput(pInput);

		ResumeGraph;
	}

	return S_OK;
}
*/
STDMETHODIMP_(int) CAudioSwitcherFilter::GetNumberOfInputChannels()
{
	CStreamSwitcherInputPin* pInPin = GetInputPin();
	return pInPin ? ((WAVEFORMATEX*)pInPin->CurrentMediaType().pbFormat)->nChannels : 0;
}

STDMETHODIMP_(bool) CAudioSwitcherFilter::IsDownSamplingTo441Enabled()
{
	return m_fDownSampleTo441;
}

STDMETHODIMP CAudioSwitcherFilter::EnableDownSamplingTo441(bool fEnable)
{
	if(m_fDownSampleTo441 != fEnable)
	{
		PauseGraph;
		m_fDownSampleTo441 = fEnable;
		ResumeGraph;
	}

	return S_OK;
}

STDMETHODIMP_(REFERENCE_TIME) CAudioSwitcherFilter::GetAudioTimeShift()
{
	return m_rtAudioTimeShift;
}

STDMETHODIMP CAudioSwitcherFilter::SetAudioTimeShift(REFERENCE_TIME rtAudioTimeShift)
{
	m_rtAudioTimeShift = rtAudioTimeShift;
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::GetNormalizeBoost(bool& fNormalize, bool& fNormalizeRecover, float& boost)
{
	fNormalize = m_fNormalize;
	fNormalizeRecover = m_fNormalizeRecover;
	boost = m_boost;
	return S_OK;
}

STDMETHODIMP CAudioSwitcherFilter::SetNormalizeBoost(bool fNormalize, bool fNormalizeRecover, float boost)
{
	if(m_fNormalize != fNormalize) m_sample_max = 0.1f;
	m_fNormalize = fNormalize;
	m_fNormalizeRecover = fNormalizeRecover;
	m_boost = boost;
	return S_OK;
}

// IAMStreamSelect

STDMETHODIMP CAudioSwitcherFilter::Enable(long lIndex, DWORD dwFlags)
{
	HRESULT hr = __super::Enable(lIndex, dwFlags);
	if(S_OK == hr){
		m_sample_max = 0.1f;
		m_fCustomChannelMapping2 = -1;
		SVP_LogMsg5(L"switched");
	}
	SVP_LogMsg5(L"switched2");
	return hr;
}


STDMETHODIMP CAudioSwitcherFilter::SetEQControl ( int lEQBandControlPreset, float pEQBandControl[MAX_EQ_BAND])
{
	if(m_State == State_Stopped || m_pEQBandControlCurrent != pEQBandControl
		|| memcmp(m_pEQBandControlCurrent, pEQBandControl, sizeof(m_pEQBandControlCurrent))  )
	{
		
		if(pEQBandControl)
			memcpy(m_pEQBandControlCurrent, pEQBandControl, sizeof(m_pEQBandControlCurrent));
		else if(lEQBandControlPreset > 0){
			//TODO set according to preset
		}
		m_fEQControlOn = 0;
		for(int i = 0 ; i < MAX_EQ_BAND; i++){
			if(m_pEQBandControlCurrent[i] != 0){
				m_fEQControlOn = 1;
				break;
			}
		}
		SVP_LogMsg5(L"Set EQ  %d"  , m_fEQControlOn);
		if(m_fEQControlOn){
			CAutoLock dataLock(&m_csEQLock);
			if( m_EQualizer.EqzInitBoth(m_pEQBandControlCurrent) < 0){
				m_fEQControlOn = 0;
			}
		}
	}

	return S_OK;

}