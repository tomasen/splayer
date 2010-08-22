#ifndef __DB_AILRC_H
#define __DB_AILRC_H

//#include "global.h"
#include "lyrics_db.h"

class db_ailrc : lyrics_db
{

  public:
    db_ailrc() {}

    void GetResults(window_config *wcfg, std::vector<lyric_data>* results, tstring title, tstring artist, tstring album);
    void DownloadLyrics(window_config *wcfg, lyric_data* data);
    tchar* GetName();
    LYRICS_DATABASES GetID();

};


#endif
