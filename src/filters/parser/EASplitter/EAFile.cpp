#include "StdAfx.h"
#include "EAFile.h"

#include "../../../svplib/svplib.h"

CEAFile::CEAFile(IAsyncReader* pAsyncReader, HRESULT& hr)
: CBaseSplitterFile(pAsyncReader, hr, DEFAULT_CACHE_LENGTH, false)
{
	if(FAILED(hr)) return;
	hr = Init();
}

HRESULT CEAFile::Init()
{
	if(!Probe()) return E_FAIL;

	return S_OK;
}

bool CEAFile::Probe()
{
	Seek(0);
	DWORD probeTag = bswap_32((DWORD)BitRead(32));

	SVP_LogMsg5(L"EA Tag %x %x", probeTag , MVhd_TAG);
	switch(probeTag)
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
		return false;
	}

	return true;
}