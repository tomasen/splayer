#pragma once

typedef struct 
{
    int uniqueid;
    std::wstring path;
    std::wstring filename;
    std::wstring thumbnailpath;
    int videotime;

} MediaData;

#define MediaDatas std::vector<MediaData>