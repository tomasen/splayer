#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include "EAFile.h"
#include "..\BaseSplitter\BaseSplitter.h"

class __declspec(uuid("941062A7-D0F3-4588-B994-D9CBDBD52265")) CEASpliterFilter
	: public CBaseSplitterFilter
{

	int ea_read_packet();
	UINT32 read_arbitary();
	int process_audio_header_elements();
	int process_audio_header_eacs();
	int process_audio_header_sead();
	int process_video_header_mdec();
	int process_video_header_vp6();
	int process_header();
	int eav_get_packet( int size);

	LPVOID m_pea;
	LPVOID m_pkt;

	protected:
		CAutoPtr<CEAFile> m_pFile;
		HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

		bool DemuxInit();
		void DemuxSeek(REFERENCE_TIME rt);
		bool DemuxLoop();

	public:
		CEASpliterFilter(LPUNKNOWN pUnk, HRESULT* phr);
		~CEASpliterFilter();
};


class __declspec(uuid("45C74A43-C220-4096-B645-742D84DD7DC6")) CEASourceFilter : public CEASpliterFilter
{
public:
	CEASourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};