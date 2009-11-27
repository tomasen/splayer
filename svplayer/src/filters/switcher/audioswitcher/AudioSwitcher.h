// Copyright 2003 Gabest.
// http://www.gabest.org
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html

#pragma once

#include "StreamSwitcher.h"
#include "..\..\..\svplib\SVPEqualizer.h"


#define MAX_OUTPUT_CHANNELS 18
#define MAX_INPUT_CHANNELS 18
#define MAX_NORMALIZE_CHANNELS 18

interface __declspec(uuid("CEDB2890-53AE-4231-91A3-B0AAFCD1DBDE")) IAudioSwitcherFilter : public IUnknown
{
	STDMETHOD(GetInputSpeakerConfig) (DWORD* pdwChannelMask) = 0;
	STDMETHOD(ResetAudioSwitch) () = 0;
    //STDMETHOD(GetSpeakerConfig) (bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]) = 0;
    //STDMETHOD(SetSpeakerConfig) (bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]) = 0;
    STDMETHOD_(int, GetNumberOfInputChannels) () = 0;
	STDMETHOD_(bool, IsDownSamplingTo441Enabled) () = 0;
	STDMETHOD(EnableDownSamplingTo441) (bool fEnable) = 0;
	STDMETHOD_(REFERENCE_TIME, GetAudioTimeShift) () = 0;
	STDMETHOD(SetAudioTimeShift) (REFERENCE_TIME rtAudioTimeShift) = 0;
	STDMETHOD(GetNormalizeBoost) (bool& fNormalize, bool& fNormalizeRecover, float& boost) = 0;
	STDMETHOD(SetNormalizeBoost) (bool fNormalize, bool fNormalizeRecover, float boost) = 0;
	STDMETHOD(GetSpeakerChannelConfig) (int *plTotalOutputChannel , float pChannelNormalize[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS]) = 0;
	STDMETHOD(SetSpeakerChannelConfig) (int lTotalOutputChannel , float pChannelNormalize[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS]
			,float pSpeakerToChannelMapOffset[MAX_INPUT_CHANNELS][MAX_NORMALIZE_CHANNELS], int iSimpleSwitch, int iSS) = 0;
	STDMETHOD(SetEQControl) ( int lEQBandControlPreset, float pEQBandControl[MAX_EQ_BAND]) = 0;

	STDMETHOD (SetRate)(double dRate) = 0;
};

class AudioStreamResampler;


class __declspec(uuid("18C16B08-6497-420e-AD14-22D21C2CEAB7")) CAudioSwitcherFilter : public CStreamSwitcherFilter, public IAudioSwitcherFilter
{
	//typedef struct {DWORD Speaker, Channel;} ChMap;
	//CAtlArray<ChMap> m_chs[18];

	int m_fCustomChannelMapping2;
	int m_lastInputChannelCount;
	int m_lastOutputChannelCount;
	int m_lastInputChannelCount2;
	int m_lastOutputChannelCount2;
	time_t m_tPlayedtime;
	int m_iSS;

	float m_pCurrentChannelNormalize2[MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS];
	// -- DWORD m_pSpeakerToChannelMap[18][18];
	// -- TotalOutputChannel set by function
	int m_lTotalOutputChannel;
	// TotalInputChannel-1 | TotalOutputChannel - 1 | OutputChannelID | LevelOfEachChannel  range : -0.0 - 2.0f+/-
	float m_pChannelNormalize2[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS];

	// 0 - 2.0
	float m_pSpeakerToChannelMapOffset[MAX_INPUT_CHANNELS][MAX_NORMALIZE_CHANNELS];
	int m_iSimpleSwitch;

	int m_fEQControlOn;
	float m_pEQBandControlCurrent[MAX_EQ_BAND];
	
	bool m_fDownSampleTo441;
	int m_fUpSampleTo;
	REFERENCE_TIME m_rtAudioTimeShift;
	CAutoPtrArray<AudioStreamResampler> m_pResamplers;
	double m_sample_max;
	bool m_fNormalize, m_fNormalizeRecover;
	bool m_fVolSuggested;
	float m_boost;
	float m_dRate;

	REFERENCE_TIME m_rtNextStart, m_rtNextStop;

	bool m_bNoMoreCheckConnection;
	int m_l_number_of_channels;

	CSVPEqualizer m_EQualizer;

public:
	CAudioSwitcherFilter(LPUNKNOWN lpunk, HRESULT* phr);

	enum
	{
		WETYPE_UNKNOWN,
		WETYPE_PCM8,
		WETYPE_PCM16,
		WETYPE_PCM24,
		WETYPE_PCM32,
		WETYPE_FPCM32,
		WETYPE_FPCM64
	};
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	CCritSec m_csDataLock;
	CCritSec m_csEQLock;

	HRESULT CheckMediaType(const CMediaType* pmt);
	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	CMediaType CreateNewOutputMediaType(CMediaType mt, long& cbBuffer);
	void OnNewOutputMediaType(const CMediaType& mtIn, const CMediaType& mtOut);

	HRESULT DeliverEndFlush();
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	// IAudioSwitcherFilter
	STDMETHODIMP GetInputSpeakerConfig(DWORD* pdwChannelMask);
    //STDMETHODIMP GetSpeakerConfig(bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]);
    //STDMETHODIMP SetSpeakerConfig(bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[18][18]);
    STDMETHODIMP_(int) GetNumberOfInputChannels();
	STDMETHODIMP_(bool) IsDownSamplingTo441Enabled();
	STDMETHODIMP EnableDownSamplingTo441(bool fEnable);
	STDMETHODIMP_(REFERENCE_TIME) GetAudioTimeShift();
	STDMETHODIMP SetAudioTimeShift(REFERENCE_TIME rtAudioTimeShift);
	STDMETHODIMP GetNormalizeBoost(bool& fNormalize, bool& fNormalizeRecover, float& boost);
	STDMETHODIMP SetNormalizeBoost(bool fNormalize, bool fNormalizeRecover, float boost);
	STDMETHODIMP ResetAudioSwitch();

	STDMETHODIMP SetRate(double dRate);

	STDMETHODIMP GetSpeakerChannelConfig (int *plTotalOutputChannel , float pChannelNormalize[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS]);
	STDMETHODIMP SetSpeakerChannelConfig (int lTotalOutputChannel , float pChannelNormalize[MAX_INPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_OUTPUT_CHANNELS][MAX_NORMALIZE_CHANNELS]
			,float pSpeakerToChannelMapOffset[MAX_INPUT_CHANNELS][MAX_NORMALIZE_CHANNELS], int iSimpleSwitch , int iSS );

	STDMETHODIMP SetEQControl ( int lEQBandControlPreset, float pEQBandControl[MAX_EQ_BAND]);
	// IAMStreamSelect
	STDMETHODIMP Enable(long lIndex, DWORD dwFlags);

};
