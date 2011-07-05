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
#ifndef _IWAVPACKDSDECODER_H_
#define _IWAVPACKDSDECODER_H_
//-----------------------------------------------------------------------------

enum DecodingModeEnum {
    DECODING_MODE_LOSSLESS,
    DECODING_MODE_HYBRID,
    DECODING_MODE_LOSSY,
    DECODING_MODE_LOSSLESS_OR_LOSSY,
    DECODING_MODE_LAST
};

#ifdef __cplusplus
extern "C" {
#endif
	
	// IWavPackDSDecoder GUID
	// {597EC162-B5F8-4baf-A33D-4F55354E9A5A}
	DEFINE_GUID(IID_IWavPackDSDecoder,
		0x597ec162, 0xb5f8, 0x4baf, 0xa3, 0x3d, 0x4f, 0x55, 0x35, 0x4e, 0x9a, 0x5a);
		
	//
	// IWavPackDSDecoder
	//
	DECLARE_INTERFACE_(IWavPackDSDecoder, IUnknown)
	{
		STDMETHOD(get_SampleRate)(THIS_ int* sample_rate) PURE;
		STDMETHOD(get_Channels)(THIS_ int *channels) PURE;
		STDMETHOD(get_BitsPerSample)(THIS_ int *bits_per_sample) PURE;
		STDMETHOD(get_FramesDecoded)(THIS_ int *frames_decoded) PURE;
		STDMETHOD(get_CrcErrors)(THIS_ int *crc_errors) PURE;
		STDMETHOD(get_DecodingMode)(THIS_ int *decoding_mode) PURE;
	};
	
#ifdef __cplusplus
}
#endif

//-----------------------------------------------------------------------------
#endif // _IWAVPACKDSDECODER_H_
//-----------------------------------------------------------------------------
