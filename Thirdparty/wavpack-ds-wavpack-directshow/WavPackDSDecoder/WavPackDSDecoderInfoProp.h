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
#ifndef _WAVPACKDSDECODERINFOPROP_H_
#define _WAVPACKDSDECODERINFOPROP_H_
// ----------------------------------------------------------------------------

class CWavPackDSDecoderInfoProp : public CBasePropertyPage
{
public:
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);	
	CWavPackDSDecoderInfoProp(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CWavPackDSDecoderInfoProp();
	HRESULT OnConnect(IUnknown *pUnknown);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	BOOL OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT OnApplyChanges();

private:
	void RefreshDisplay(HWND hwnd);
	void SetDirty(void);

	int m_SampleRate;
	int m_Channels;
	int m_BitsPerSample;
	int m_FramesDecoded;
	int m_CrcErrors;
	int m_DecodingMode;

	IWavPackDSDecoder* m_pIWavPackDSDecoder;
	BOOL    m_fWindowInActive;          // TRUE ==> dialog is being destroyed
};

// ----------------------------------------------------------------------------
#endif // _WAVPACKDSDECODERINFOPROP_H_
// ----------------------------------------------------------------------------