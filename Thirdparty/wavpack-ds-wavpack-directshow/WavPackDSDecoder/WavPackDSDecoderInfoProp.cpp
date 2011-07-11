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
#include "IWavPackDSDecoder.h"
#include "WavPackDSDecoderInfoProp.h"
#include "resource.h"
#include <stdio.h>


// ----------------------------------------------------------------------------

static char* strDecodingMode[DECODING_MODE_LAST+1] = {
    "Lossless",
    "Hybrid",
    "Lossy",
    "Lossless or lossy",
    "Unknown"
};

char* getDecodingModeName(int decoding_mode)
{
    if(decoding_mode >= 0 && decoding_mode < DECODING_MODE_LAST)
    {
        return strDecodingMode[decoding_mode];
    }
    else
    {  
        return strDecodingMode[DECODING_MODE_LAST];
    }
}

// ----------------------------------------------------------------------------

CUnknown *WINAPI CWavPackDSDecoderInfoProp::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
	CWavPackDSDecoderInfoProp *pNewObject = new CWavPackDSDecoderInfoProp(punk, phr);
	if (!pNewObject)
		*phr = E_OUTOFMEMORY;
	return pNewObject;
}

// ----------------------------------------------------------------------------

CWavPackDSDecoderInfoProp::CWavPackDSDecoderInfoProp(LPUNKNOWN pUnk, HRESULT *phr) :
	CBasePropertyPage(NAME("Info"), pUnk, IDD_DIALOG_INFO, IDS_INFO),
	m_pIWavPackDSDecoder(NULL),
	m_fWindowInActive(TRUE)
{
	
}

// ----------------------------------------------------------------------------

CWavPackDSDecoderInfoProp::~CWavPackDSDecoderInfoProp()
{

}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoderInfoProp::OnConnect(IUnknown *pUnknown)
{
	if (pUnknown == NULL)
	{
		return E_POINTER;
	}

	ASSERT(m_pIWavPackDSDecoder == NULL);
	
	// Ask the filter for it's control interface		
	HRESULT hr = pUnknown->QueryInterface(IID_IWavPackDSDecoder,
		reinterpret_cast<void**>(&m_pIWavPackDSDecoder));
	if(FAILED(hr))
	{
		return hr;
	}

	ASSERT(m_pIWavPackDSDecoder);

    m_pIWavPackDSDecoder->get_SampleRate(&m_SampleRate);
    m_pIWavPackDSDecoder->get_Channels(&m_Channels);
    m_pIWavPackDSDecoder->get_BitsPerSample(&m_BitsPerSample);
    m_pIWavPackDSDecoder->get_FramesDecoded(&m_FramesDecoded);
    m_pIWavPackDSDecoder->get_CrcErrors(&m_CrcErrors);
    m_pIWavPackDSDecoder->get_DecodingMode(&m_DecodingMode);
	
	return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoderInfoProp::OnDisconnect()
{
	// Release the interface
	if (m_pIWavPackDSDecoder == NULL) {
		return E_UNEXPECTED;
	}
	m_pIWavPackDSDecoder->Release();
	m_pIWavPackDSDecoder = NULL;
	return NOERROR;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoderInfoProp::OnActivate()
{		
	char msgFormat[255];
	m_fWindowInActive = FALSE;

	wsprintf(msgFormat, "%d Hz", m_SampleRate);
	SetDlgItemText(m_hwnd, IDC_LABEL_SAMPLERATE, msgFormat);

	wsprintf(msgFormat, "%d", m_Channels);
	SetDlgItemText(m_hwnd, IDC_LABEL_CHANNELS, msgFormat);

	wsprintf(msgFormat, "%d", m_BitsPerSample);
	SetDlgItemText(m_hwnd, IDC_LABEL_BPS, msgFormat);

    wsprintf(msgFormat, "%d", m_FramesDecoded);
    SetDlgItemText(m_hwnd, IDC_LABEL_FRAMES, msgFormat);

    wsprintf(msgFormat, "%d", m_CrcErrors);
    SetDlgItemText(m_hwnd, IDC_LABEL_CRC, msgFormat);

    SetDlgItemText(m_hwnd, IDC_LABEL_MODE, getDecodingModeName(m_DecodingMode));

	RefreshDisplay(m_hwnd);
	SetTimer(m_hwnd, 0, 1000, NULL);

	return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoderInfoProp::OnDeactivate()
{
	KillTimer(m_hwnd,0);
	m_fWindowInActive = TRUE;
	return S_OK;
}

// ----------------------------------------------------------------------------

HRESULT CWavPackDSDecoderInfoProp::OnApplyChanges(void)
{
    // Apply change here
	//m_pIWavPackDSDecoder->set_Something(m_Something);
	return S_OK;
}

// ----------------------------------------------------------------------------

BOOL CWavPackDSDecoderInfoProp::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_fWindowInActive)
		return FALSE;

	switch(uMsg)
	{
//	case WM_COMMAND:
//		switch (LOWORD(wParam))
//		{
//		case IDC_CHECK_POSTGAIN:
//			m_bUsePostGain = (IsDlgButtonChecked(hwnd,IDC_CHECK_POSTGAIN) == BST_CHECKED) ? true : false;
//			SetDirty();
//			break;
//		case IDC_CHECK_16DECODING:
//			m_bDecodeTo16Bits = (IsDlgButtonChecked(hwnd,IDC_CHECK_16DECODING) == BST_CHECKED) ? true : false;
//			SetDirty();				
//			break;
//		}
//		break;

	case WM_TIMER:
		RefreshDisplay(hwnd);
		return (LRESULT) 1;
	}
	
	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);		
}

// ----------------------------------------------------------------------------

void CWavPackDSDecoderInfoProp::RefreshDisplay(HWND hwnd)
{
	static char msgFormat[16];

    m_pIWavPackDSDecoder->get_FramesDecoded(&m_FramesDecoded);
    m_pIWavPackDSDecoder->get_CrcErrors(&m_CrcErrors);
    m_pIWavPackDSDecoder->get_DecodingMode(&m_DecodingMode);

	wsprintf(msgFormat, "%d", m_FramesDecoded);
	SetDlgItemText(hwnd, IDC_LABEL_FRAMES, msgFormat);

    wsprintf(msgFormat, "%d", m_CrcErrors);
    SetDlgItemText(hwnd, IDC_LABEL_CRC, msgFormat);

    SetDlgItemText(m_hwnd, IDC_LABEL_MODE, getDecodingModeName(m_DecodingMode));
}

// ----------------------------------------------------------------------------

void CWavPackDSDecoderInfoProp::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }
}

// ----------------------------------------------------------------------------