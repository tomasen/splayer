// lyriclib.h

#pragma once
#include "stdafx.h"
#include "parse_lyrics.h"
#include "lyrics_db.h"
#include "../../src/filters/BaseClasses/streams.h"

typedef struct {CString fn; /*exttype ext;*/} LrcFile;

class CLyricLib {
	typedef struct { LONGLONG rtStart, rtStop; CString szLyricLine; } LyricLine;

private:

    window_config wcfg;
    std::vector<lyric_data> availableLyrics;
	CAtlList<LyricLine> m_lyrics;

    parse_lyrics lyrics_parser;
public:
	CLyricLib(void);
	~CLyricLib(void);

	CString m_sz_current_lyric_file;
    CString m_sz_current_music_file;
	BOOL m_has_lyric;
    BOOL m_stop_downloading;

     tstring title;
     tstring artist;
     tstring album;

    CCritSec m_fetch_lock;
    void fetch_lyric_from_internet();
    bool try_download_lyric(CString szDownloadingForFile);
    
    BOOL isLyricFile(CString szFn);
    void GetLrcFileNames(CString fn, CAtlArray<CString>& paths, CAtlArray<LrcFile>& ret, BOOL byDir = 0);
	int LoadLyricFile(CString sz_lyric_file_path);
	CString GetCurrentLyricLineByTime(LONGLONG rt_now, int* lasting_time_in_ms);
	void  Empty();
};