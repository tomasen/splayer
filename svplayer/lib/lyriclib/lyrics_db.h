#ifndef __LYRICS_DB_H
#define __LYRICS_DB_H

//#include "global.h"
#include "stdafx.h"
#include "main.h"
//#include "lyriclib.h"
#include "URLEncode.h"
//#include "window_config.h"
class lyrics_db;

enum LYRIC_DATA_LOAD_STATE
{
    LS_NONE, LS_CACHE, LS_TAG, LS_ASSOC, LS_FILE, LS_INTERNET, LS_ALL
};

enum LYRIC_SOURCE_TYPE
{
    ST_TAG, ST_ASSOC, ST_FILE, ST_INTERNET, ST_UNKNOWN
};

enum LYRIC_RESULT_QUALITY
{
    Q_BAD, Q_MEDIUM, Q_GOOD
};

struct lyric_data
{

public:
    LYRIC_SOURCE_TYPE SourceType;
    tstring Source;

    bool IsTimestamped;
    bool IsLoaded;

    tstring Text;

    tstring OnlineService;
    LYRIC_RESULT_QUALITY Quality;
    tstring matchTitle;
    tstring matchArtist;
    tstring matchAlbum;
    lyrics_db *db;

    window_config *wcfg;

    lyric_data(window_config *cfg)
    {
        SourceType = ST_UNKNOWN;
        Source = L"";
        IsTimestamped = false;
        IsLoaded = false;
        Text = L"";
        OnlineService = L"";
        Quality = Q_BAD;
        matchTitle = L"";
        matchArtist = L"";
        matchAlbum = L"";
        db = NULL;
        wcfg = cfg;
    }

    virtual inline bool operator<(const lyric_data& rhs)
    {

      
        {
            if(this->IsTimestamped && !rhs.IsTimestamped)
                return true;
            else if(!this->IsTimestamped && rhs.IsTimestamped)
                return false;
        }

        if (rhs.SourceType != ST_INTERNET)
        {
            if (this->SourceType == ST_INTERNET && rhs.SourceType != ST_INTERNET)
                return true;

            if (this->SourceType != ST_INTERNET && rhs.SourceType == ST_INTERNET)
                return false;

            return (this->SourceType < rhs.SourceType);
        }

        return (this->Quality > rhs.Quality);
    }
};

enum LYRICS_DATABASES
{
  LD_UNKNOWN, LD_VIEWLYRICS, LD_LRCDB, LD_LYRDB, LD_LEOS, LD_AILRC, LD_LYRICSFLY
};


class lyrics_db
{

  public:
    virtual void GetResults(window_config *wcfg, std::vector<lyric_data> *results, tstring title, tstring artist, tstring album) {}

    virtual void DownloadLyrics(window_config *wcfg, lyric_data *data) {}

    virtual tchar* GetName() { return _T("NULL"); }

    virtual LYRICS_DATABASES GetID() { return LD_UNKNOWN; }
};




#endif
