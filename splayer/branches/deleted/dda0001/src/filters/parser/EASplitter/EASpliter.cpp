

#include "StdAfx.h"
#include "EASpliter.h"
#include "..\..\..\DSUtil\DSUtil.h"

#include <initguid.h>
#include "..\..\..\..\include\moreuuids.h"

#include "../../../svplib/svplib.h"


#include "PODtypes.h"
#include "avcodec.h"
#include "libavutil/intreadwrite.h"
#include "libavformat/avformat.h"

#define av_log(s,y,...) SVP_LogMsg6(__VA_ARGS__)
#define get_byte(x) (BYTE)m_pFile->BitRead(8)
#define get_be32(x) ((DWORD)m_pFile->BitRead(32))
#define get_le32(x) bswap_32((DWORD)m_pFile->BitRead(32))
#define get_le16(x) bswap_16((SHORT)m_pFile->BitRead(16))
#define url_fskip(f,o) m_pFile->Skip(o)
#define url_fseek(f,o,x); if(x==SEEK_CUR){m_pFile->Skip(o);}else{m_pFile->Seek(o);}

typedef struct EaDemuxContext {
	int big_endian;

	enum CodecID video_codec;
	AVRational time_base;
	int width, height;
	int video_stream_index;

	enum CodecID audio_codec;
	int audio_stream_index;
	int audio_frame_counter;

	int bytes;
	int sample_rate;
	int num_channels;
	int num_samples;
} EaDemuxContext;

int CEASpliterFilter::eav_get_packet( int size)
{
	AVPacket *pkt = (AVPacket *)m_pkt;
	//int ret= av_new_packet(pkt, size);

	//if(ret<0)
	//	return ret;

	pkt->pos= m_pFile->GetPos();//url_ftell(s);

	//ret= m_pFile->ByteRead( pkt->data, size);
		//get_buffer(s, pkt->data, size);
	//if(ret<=0)
	//	av_free_packet(pkt);
	//else{
		pkt->size= size;
		CAutoPtr<Packet> p;
		p.Attach(new Packet());
		p->TrackNumber = pkt->stream_index;
		p->rtStart = pkt->pts; 
		p->rtStop = p->rtStart + pkt->duration;
		p->bSyncPoint = pkt->flags;
		p->SetCount(size);
		m_pFile->ByteRead(p->GetData(), p->GetCount());
	HRESULT	hr = DeliverPacket(p);
		
	//}

	return (hr == S_OK);
}

int CEASpliterFilter::ea_read_packet()
{
	EaDemuxContext *ea = (EaDemuxContext *)m_pea;
	AVPacket *pkt = (AVPacket *)m_pkt;
	//ByteIOContext *pb = s->pb;
	int ret = 0;
	int packet_read = 0;
	unsigned int chunk_type, chunk_size;
	int key = 0;
	int av_uninit(num_samples);

	while (!packet_read) {
		chunk_type = get_le32(pb);
		chunk_size = (ea->big_endian ? get_be32(pb) : get_le32(pb)) - 8;

		switch (chunk_type) {
			/* audio data */
		case ISNh_TAG:
			/* header chunk also contains data; skip over the header portion*/
			url_fskip(pb, 32);
			chunk_size -= 32;
		case ISNd_TAG:
		case SCDl_TAG:
		case SNDC_TAG:
		case SDEN_TAG:
			if (!ea->audio_codec) {
				url_fskip(pb, chunk_size);
				break;
			} else if (ea->audio_codec == CODEC_ID_PCM_S16LE_PLANAR ||
				ea->audio_codec == CODEC_ID_MP3) {
					num_samples = get_le32(pb);
					url_fskip(pb, 8);
					chunk_size -= 12;
			}
			ret = eav_get_packet(  chunk_size);
			if (ret < 0)
				return ret;
			pkt->stream_index = ea->audio_stream_index;
			pkt->pts = 90000;
			pkt->pts *= ea->audio_frame_counter;
			pkt->pts /= ea->sample_rate;

			switch (ea->audio_codec) {
		case CODEC_ID_ADPCM_EA:
			/* 2 samples/byte, 1 or 2 samples per frame depending
			* on stereo; chunk also has 12-byte header */
			ea->audio_frame_counter += ((chunk_size - 12) * 2) /
				ea->num_channels;
			break;
		case CODEC_ID_PCM_S16LE_PLANAR:
		case CODEC_ID_MP3:
			ea->audio_frame_counter += num_samples;
			break;
		default:
			ea->audio_frame_counter += chunk_size /
				(ea->bytes * ea->num_channels);
			}

			packet_read = 1;
			break;

			/* ending tag */
		case 0:
		case ISNe_TAG:
		case SCEl_TAG:
		case SEND_TAG:
		case SEEN_TAG:
			ret = AVERROR(EIO);
			packet_read = 1;
			break;

		case MVIh_TAG:
		case kVGT_TAG:
		case pQGT_TAG:
		case TGQs_TAG:
		case MADk_TAG:
			key = PKT_FLAG_KEY;
		case MVIf_TAG:
		case fVGT_TAG:
		case MADm_TAG:
		case MADe_TAG:
			url_fseek(pb, -8, SEEK_CUR);     // include chunk preamble
			chunk_size += 8;
			goto get_video_packet;

		case mTCD_TAG:
			url_fseek(pb, 8, SEEK_CUR);  // skip ea dct header
			chunk_size -= 8;
			goto get_video_packet;

		case MV0K_TAG:
		case MPCh_TAG:
		case pIQT_TAG:
			key = PKT_FLAG_KEY;
		case MV0F_TAG:
get_video_packet:
			ret = eav_get_packet( chunk_size);
			if (ret < 0)
				return ret;
			pkt->stream_index = ea->video_stream_index;
			pkt->flags |= key;
			packet_read = 1;
			break;

		default:
			url_fseek(pb, chunk_size, SEEK_CUR);
			break;
		}
	}

	return ret;
}

UINT32 CEASpliterFilter::read_arbitary() {
	uint8_t size, byte;
	int i;
	uint32_t word;

	
	size = get_byte(pb);

	word = 0;
	for (i = 0; i < size; i++) {
		byte = get_byte(pb);
		word <<= 8;
		word |= byte;
	}

	
	return word;
}

/*
* Process PT/GSTR sound header
* return 1 if success, 0 if invalid format, otherwise AVERROR_xxx
*/
int CEASpliterFilter::process_audio_header_elements()
{
	EaDemuxContext *ea = (EaDemuxContext *)m_pea;
	int inHeader = 1;
	
	int compression_type = -1, revision = -1, revision2 = -1;

	ea->bytes = 2;
	ea->sample_rate = -1;
	ea->num_channels = 1;

	while (inHeader) {
		int inSubheader;
		uint8_t byte;
		byte = get_byte(pb);

		switch (byte) {
		case 0xFD:
			av_log (s, AV_LOG_DEBUG, "entered audio subheader\n");
			inSubheader = 1;
			while (inSubheader) {
				uint8_t subbyte;
				subbyte = get_byte(pb);

				switch (subbyte) {
		case 0x80:
			revision = read_arbitary();
			av_log (s, AV_LOG_DEBUG, "revision (element 0x80) set to 0x%08x\n", revision);
			break;
		case 0x82:
			ea->num_channels = read_arbitary();
			av_log (s, AV_LOG_DEBUG, "num_channels (element 0x82) set to 0x%08x\n", ea->num_channels);
			break;
		case 0x83:
			compression_type = read_arbitary();
			av_log (s, AV_LOG_DEBUG, "compression_type (element 0x83) set to 0x%08x\n", compression_type);
			break;
		case 0x84:
			ea->sample_rate = read_arbitary();
			av_log (s, AV_LOG_DEBUG, "sample_rate (element 0x84) set to %i\n", ea->sample_rate);
			break;
		case 0x85:
			ea->num_samples = read_arbitary();
			av_log (s, AV_LOG_DEBUG, "num_samples (element 0x85) set to 0x%08x\n", ea->num_samples);
			break;
		case 0x8A:
			av_log (s, AV_LOG_DEBUG, "element 0x%02x set to 0x%08x\n", subbyte, read_arbitary());
			av_log (s, AV_LOG_DEBUG, "exited audio subheader\n");
			inSubheader = 0;
			break;
		case 0xA0:
			revision2 = read_arbitary();
			av_log (s, AV_LOG_DEBUG, "revision2 (element 0xA0) set to 0x%08x\n", revision2);
			break;
		case 0xFF:
			av_log (s, AV_LOG_DEBUG, "end of header block reached (within audio subheader)\n");
			inSubheader = 0;
			inHeader = 0;
			break;
		default:
			av_log (s, AV_LOG_DEBUG, "element 0x%02x set to 0x%08x\n", subbyte, read_arbitary());
			break;
				}
			}
			break;
		case 0xFF:
			av_log (s, AV_LOG_DEBUG, "end of header block reached\n");
			inHeader = 0;
			break;
		default:
			av_log (s, AV_LOG_DEBUG, "header element 0x%02x set to 0x%08x\n", byte, read_arbitary());
			break;
		}
	}

	switch (compression_type) {
	case  0: ea->audio_codec = CODEC_ID_PCM_S16LE; break;
	case  7: ea->audio_codec = CODEC_ID_ADPCM_EA; break;
	case -1:
		switch (revision) {
	case  1: ea->audio_codec = CODEC_ID_ADPCM_EA_R1; break;
	case  2: ea->audio_codec = CODEC_ID_ADPCM_EA_R2; break;
	case  3: ea->audio_codec = CODEC_ID_ADPCM_EA_R3; break;
	case -1: break;
	default:
		av_log(s, AV_LOG_ERROR, "unsupported stream type; revision=%i\n", revision);
		return 0;
		}
		switch (revision2) {
	case  8: ea->audio_codec = CODEC_ID_PCM_S16LE_PLANAR; break;
	case 10: ea->audio_codec = CODEC_ID_ADPCM_EA_R2; break;
	case 16: ea->audio_codec = CODEC_ID_MP3; break;
	case -1: break;
	default:
		av_log(s, AV_LOG_ERROR, "unsupported stream type; revision2=%i\n", revision2);
		return 0;
		}
		break;
	default:
		av_log(s, AV_LOG_ERROR, "unsupported stream type; compression_type=%i\n", compression_type);
		return 0;
	}

	if (ea->sample_rate == -1)
		ea->sample_rate = revision==3 ? 48000 : 22050;

	return 1;
}

/*
* Process EACS sound header
* return 1 if success, 0 if invalid format, otherwise AVERROR_xxx
*/
int CEASpliterFilter::process_audio_header_eacs()
{
	EaDemuxContext *ea = (EaDemuxContext *)m_pea;
	int compression_type;

	ea->sample_rate  = ea->big_endian ? get_be32(pb) : get_le32(pb);
	ea->bytes        = get_byte(pb);   /* 1=8-bit, 2=16-bit */
	ea->num_channels = get_byte(pb);
	compression_type = get_byte(pb);
	url_fskip(pb, 13);

	switch (compression_type) {
	case 0:
		switch (ea->bytes) {
	case 1: ea->audio_codec = CODEC_ID_PCM_S8;    break;
	case 2: ea->audio_codec = CODEC_ID_PCM_S16LE; break;
		}
		break;
	case 1: ea->audio_codec = CODEC_ID_PCM_MULAW; ea->bytes = 1; break;
	case 2: ea->audio_codec = CODEC_ID_ADPCM_IMA_EA_EACS; break;
	default:
		av_log (s, AV_LOG_ERROR, "unsupported stream type; audio compression_type=%i\n", compression_type);
	}

	return 1;
}

/*
* Process SEAD sound header
* return 1 if success, 0 if invalid format, otherwise AVERROR_xxx
*/
int CEASpliterFilter::process_audio_header_sead()
{
	EaDemuxContext *ea = (EaDemuxContext *)m_pea;
	ea->sample_rate  = get_le32(pb);
	ea->bytes        = get_le32(pb);  /* 1=8-bit, 2=16-bit */
	ea->num_channels = get_le32(pb);
	ea->audio_codec  = CODEC_ID_ADPCM_IMA_EA_SEAD;

	return 1;
}

int CEASpliterFilter::process_video_header_mdec()
{
	EaDemuxContext *ea = (EaDemuxContext *)m_pea;
	url_fskip(pb, 4);
	ea->width  = get_le16(pb);
	ea->height = get_le16(pb);
	ea->time_base.num = 1;
	ea->time_base.den = 15;
	ea->video_codec = CODEC_ID_MDEC;
	return 1;
}

int CEASpliterFilter::process_video_header_vp6()
{
	
	EaDemuxContext *ea = (EaDemuxContext *)m_pea;
	url_fskip(pb, 16);
	ea->time_base.den = get_le32(pb);
	ea->time_base.num = get_le32(pb);
	ea->video_codec = CODEC_ID_VP6;

	return 1;
}

/*
* Process EA file header
* Returns 1 if the EA file is valid and successfully opened, 0 otherwise
*/
int CEASpliterFilter::process_header() {
	uint32_t blockid, size = 0;
	int i;
	EaDemuxContext *ea = (EaDemuxContext *)m_pea;
	
	for (i=0; i<5 && (!ea->audio_codec || !ea->video_codec); i++) {
		unsigned int startpos = m_pFile->GetPos();
		int err = 0;

		blockid = get_le32(pb);
		av_log(s, AV_LOG_ERROR, " %x\n", blockid);

		size = get_le32(pb);
		if (i == 0)
			ea->big_endian = size > 0x000FFFFF;
		if (ea->big_endian)
			size = bswap_32(size);

		switch (blockid) {
			case ISNh_TAG:
				if (m_pFile->BitRead(32) != EACS_TAG) {
					av_log (s, AV_LOG_ERROR, "unknown 1SNh headerid\n");
					return 0;
				}
				err = process_audio_header_eacs();
				break;

			case SCHl_TAG :
			case SHEN_TAG :
				blockid = get_le32(pb);
				if (blockid == GSTR_TAG) {
					url_fskip(pb, 4);
				} else if ((blockid & 0xFFFF)!=PT00_TAG) {
					av_log (s, AV_LOG_ERROR, "unknown SCHl headerid %x\n", blockid);
					return 0;
				}
				err = process_audio_header_elements();
				break;

			case SEAD_TAG:
				err = process_audio_header_sead();
				break;

			case MVIh_TAG :
				ea->video_codec = CODEC_ID_CMV;
				ea->time_base.num = 0;
				ea->time_base.den = 0;

				break;

			case kVGT_TAG:
				ea->video_codec = CODEC_ID_TGV;
				ea->time_base.num = 0;
				ea->time_base.den = 0;

				break;

			case mTCD_TAG :
				err = process_video_header_mdec();
				break;

			case MPCh_TAG:
				ea->video_codec = CODEC_ID_MPEG2VIDEO;
				break;

			case pQGT_TAG:
			case TGQs_TAG:
				ea->video_codec = CODEC_ID_TGQ;
				break;

			case pIQT_TAG:
				ea->video_codec = CODEC_ID_TQI;
				break;

			case MADk_TAG :
				ea->video_codec = CODEC_ID_MAD;
				break;

			case MVhd_TAG :
				err = process_video_header_vp6();
				break;
			default:
				av_log(s, AV_LOG_ERROR, "error parsing header: %x\n", blockid);
				break;
		}

		if (err < 0) {
			av_log(s, AV_LOG_ERROR, "error parsing header: %i\n", err);
			return err;
		}

		//url_fseek(pb, startpos + size, SEEK_SET);
		m_pFile->Seek( startpos + size);
	}

	//url_fseek(pb, 0, SEEK_SET);
	m_pFile->Seek(0);

	return 1;
}



CEASpliterFilter::CEASpliterFilter(LPUNKNOWN pUnk, HRESULT* phr)
: CBaseSplitterFilter(NAME("CEASplitterFilter"), pUnk, phr, __uuidof(this))
{
	
	m_pea = malloc(sizeof(EaDemuxContext)) ;
	memset(m_pea,0, sizeof(EaDemuxContext));

	m_pkt = malloc(sizeof(AVPacket)) ;
	memset(m_pkt,0, sizeof(AVPacket));
}

CEASpliterFilter::~CEASpliterFilter()
{
	free(m_pea);
	free(m_pkt);
}

HRESULT CEASpliterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	SVP_LogMsg5(L" CEASpliterFilter::CreateOutputs");
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();

	m_pFile.Attach(new CEAFile(pAsyncReader, hr));
	if(!m_pFile) return E_OUTOFMEMORY;
	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	m_pFile->Seek(0);
	DWORD EAFileTag = bswap_32((DWORD)m_pFile->BitRead(32));

	switch(EAFileTag)
	{
		case ISNh_TAG:
		case SCHl_TAG:
		case SEAD_TAG:
		case SHEN_TAG:
		case kVGT_TAG:
		case MADk_TAG:
		case MPCh_TAG:
		case MVhd_TAG:
		case MVIh_TAG:
			SVP_LogMsg5(L"GotEA Tag");
			break;
		default:
			return hr;
	}

	m_pFile->Seek(0);
	EaDemuxContext *ea = (EaDemuxContext *)m_pea;
	SVP_LogMsg5(L"EABefore Info %x %x", ea->audio_codec, ea->video_codec);;
	process_header();
	SVP_LogMsg5(L"GotEA Info  %d %d", ea->audio_codec, ea->video_codec);;
	int idPin = 0;
	while(ea->video_codec){
		CMediaType mt;
		mt.SetSampleSize(1);
		mt.subtype = GUID_NULL;
		switch(ea->video_codec){

			case CODEC_ID_VP6:
				{
					mt.majortype = MEDIATYPE_Video;
					mt.formattype = FORMAT_VideoInfo;
					VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
					memset(vih, 0, sizeof(VIDEOINFOHEADER));
					BITMAPINFOHEADER* bih = &vih->bmiHeader;
					vih->bmiHeader.biWidth = ea->width;
					vih->bmiHeader.biHeight = ea->height;
					bih->biCompression = '26PV';
					mt.subtype = MEDIASUBTYPE_VP62;
				}
				break;
			case CODEC_ID_MPEG2VIDEO:
				//TODO: handle MPEG2 in vp6
				/*mt.formattype = FORMAT_MPEG2Video;
				MPEG2VIDEOINFO* vih = (MPEG2VIDEOINFO*)mt.AllocFormatBuffer(FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + headerSize);
				memset(vih, 0, mt.FormatLength());
				vih->hdr.bmiHeader.biSize = sizeof(vih->hdr.bmiHeader);
				vih->hdr.bmiHeader.biPlanes = 1;
				vih->hdr.bmiHeader.biBitCount = 24;
				vih->hdr.bmiHeader.biWidth =  ea->width;
				vih->hdr.bmiHeader.biHeight = ea->height;
				*/
			case CODEC_ID_MDEC:
			case CODEC_ID_CMV:
			case CODEC_ID_TGV:
			case CODEC_ID_TGQ:
			case CODEC_ID_TQI:
			case CODEC_ID_MAD:
				SVP_LogMsg5(L"sorry we cant handle this now");
				break;
		}
		
		if(mt.subtype == GUID_NULL)
			break;
		CAtlArray<CMediaType> mts;
		mts.Add(mt);
		CAutoPtr<CBaseSplitterOutputPin> pPinOut(new CBaseSplitterOutputPin(mts, L"Video", this, this, &hr));
		EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(idPin, pPinOut)));
		
		idPin++;
		break;
	}
	if(ea->audio_codec){
		CMediaType mt;
		mt.SetSampleSize(1);
		

		mt.majortype = MEDIATYPE_Audio;
		mt.formattype = FORMAT_WaveFormatEx;
		
		switch(ea->audio_codec){
			case CODEC_ID_MP3:
				mt.subtype = MEDIASUBTYPE_MP3;
				{
					WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(sizeof(WAVEFORMATEX));
					memset(wfe, 0, sizeof(WAVEFORMATEX));
					wfe->nSamplesPerSec = ea->sample_rate;
					wfe->wBitsPerSample = ea->bytes * 8;
					wfe->nChannels = ea->num_channels;
				}
				break;
			case CODEC_ID_ADPCM_EA:
			case CODEC_ID_ADPCM_EA_R1:
			case CODEC_ID_ADPCM_EA_R2:
			case CODEC_ID_ADPCM_EA_R3:
			case CODEC_ID_PCM_S16LE_PLANAR:
			case CODEC_ID_PCM_S8:
			case CODEC_ID_PCM_S16LE:
			case CODEC_ID_PCM_MULAW:
			case CODEC_ID_ADPCM_IMA_EA_EACS:
			case CODEC_ID_ADPCM_IMA_EA_SEAD:
				break;
		}
		
		if(mt.subtype != GUID_NULL){

			CAtlArray<CMediaType> mts;
			mts.Add(mt);
			CAutoPtr<CBaseSplitterOutputPin> pPinOut(new CBaseSplitterOutputPin(mts, L"Audio", this, this, &hr));
			EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(idPin, pPinOut)));
		}

	}
	
	SVP_LogMsg5(L"EA Out %d", m_pOutputs.GetCount());
	m_rtNewStop = m_rtStop = m_rtDuration ;

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CEASpliterFilter::DemuxInit()
{
	SVP_LogMsg5(L" CEASpliterFilter::DemuxInit");
	if(!m_pFile) return(false);

	return(true);
}

void CEASpliterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	SVP_LogMsg5(L" CEASpliterFilter::DemuxSeek");
	if(rt <= 0 )
	{
		m_pFile->Seek(0);
	}
	
}

bool CEASpliterFilter::DemuxLoop()
{
	SVP_LogMsg5(L" CEASpliterFilter::DemuxLoop");
	HRESULT hr = S_OK;

	ea_read_packet();

	return(true);
}


CEASourceFilter::CEASourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
: CEASpliterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}
