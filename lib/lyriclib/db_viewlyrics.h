#ifndef __VIEWLYRICS_H
#define __VIEWLYRICS_H

//#include "global.h"
#include "stdafx.h"
#include "lyrics_db.h"

class db_viewlyrics : lyrics_db
{

  public:
    db_viewlyrics() {}

    void GetResults(window_config *wcfg,std::vector<lyric_data>* results, tstring title, tstring artist, tstring album);
    void DownloadLyrics(window_config *wcfg,lyric_data* data);
    tchar* GetName();
    LYRICS_DATABASES GetID();
};


#endif
