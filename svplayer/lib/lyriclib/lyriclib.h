// lyriclib.h

#pragma once
#include "stdafx.h"


class CLyricLib {
	typedef struct { LONGLONG rtStart, rtStop; CString szLyricLine; } LyricLine;

private:

	CAtlList<LyricLine> m_lyrics;

public:
	CLyricLib(void);
	~CLyricLib(void);

	CString m_sz_current_lyric_file;
	BOOL m_has_lyric;

	int FindLyricFileForAudio(CString sz_audio_file_path, CStringArray* sza_results);
	int LoadLyricFile(CString sz_lyric_file_path);
	CString GetCurrentLyricLineByTime(LONGLONG rt_now, int* lasting_time_in_ms);
	void  Empty();
};