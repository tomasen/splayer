// lyriclib.h

#pragma once
#include "stdafx.h"
#include "parse_lyrics.h"

typedef struct {CString fn; /*exttype ext;*/} LrcFile;

class CLyricLib {
	typedef struct { LONGLONG rtStart, rtStop; CString szLyricLine; } LyricLine;

private:

	CAtlList<LyricLine> m_lyrics;

    parse_lyrics lyrics_parser;
public:
	CLyricLib(void);
	~CLyricLib(void);

	CString m_sz_current_lyric_file;
	BOOL m_has_lyric;

    void GetLrcFileNames(CString fn, CAtlArray<CString>& paths, CAtlArray<LrcFile>& ret, BOOL byDir = 0);
	int LoadLyricFile(CString sz_lyric_file_path);
	CString GetCurrentLyricLineByTime(LONGLONG rt_now, int* lasting_time_in_ms);
	void  Empty();
};