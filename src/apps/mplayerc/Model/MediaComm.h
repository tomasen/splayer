#pragma once

typedef struct 
{
    long long uniqueid;
    std::wstring path;
    std::wstring filename;
    std::wstring thumbnailpath;
    int videotime;

} MediaData;

typedef struct
{
  long long uniqueid;
  std::wstring path;
  int merit;
} MediaPath;

#define MediaDatas std::vector<MediaData>
#define MediaPaths std::vector<MediaPath>