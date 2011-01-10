#ifndef __LYRDB_H
#define __LYRDB_H

//#include "global.h"
#include "lyrics_db.h"

class db_lyrdb : lyrics_db
{

  public:
    db_lyrdb() {}

    void GetResults(window_config *wcfg, std::vector<lyric_data>* results, tstring title, tstring artist, tstring album);
    void DownloadLyrics(window_config *wcfg, lyric_data* data);
    tchar* GetName();
    LYRICS_DATABASES GetID();
};


#endif
