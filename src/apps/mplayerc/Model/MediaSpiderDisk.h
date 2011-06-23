#pragma once

#include "MediaModel.h"
#include "MediaSpiderAbstract.h"

class MediaSpiderDisk :
    public MediaSpiderAbstract<MediaSpiderDisk>
{
public:
    MediaSpiderDisk();
    ~MediaSpiderDisk();

public:
    // The main thread and main function that do the spider job
    void _Thread();
  
    void SearchDisk(const std::wstring& sPath);
    void SetSearchPath(const std::wstring& sPath);

private:
  std::wstring m_sLastSearchPath;
};