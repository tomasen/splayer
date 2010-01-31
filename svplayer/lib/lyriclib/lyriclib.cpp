// This is the main DLL file.

#include "stdafx.h"

#include "lyriclib.h"
#include "..\..\src\svplib\svplib.h"
#include "..\..\src\svplib\SVPToolBox.h"

CLyricLib::CLyricLib(void)
{
}

CLyricLib::~CLyricLib(void)
{
}
int CLyricLib::FindLyricFileForAudio(CString sz_audio_file_path, CStringArray* sza_results)
{
	return 0;
}


int CLyricLib::LoadLyricFile(CString sz_lyric_file_path)
{
    CSVPToolBox svptool;
    CString szBuff = svptool.fileGetContent( sz_lyric_file_path);
    if(!szBuff.IsEmpty()){
        lyrics_parser.read_lyrics(szBuff.GetBuffer());
        szBuff.ReleaseBuffer();
    }

	return -1;
}

CString CLyricLib::GetCurrentLyricLineByTime(LONGLONG rt_now, int* lasting_time_in_ms)
{

	return L"";
}
void CLyricLib::Empty()
{
	m_lyrics.RemoveAll();
	m_has_lyric = false;
	m_sz_current_lyric_file.Empty();
	
}