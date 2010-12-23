#pragma once

#include "MediaModel.h"
#include "MediaSpiderAbstract.h"

class MediaSpiderDisk :
    public MediaSpiderAbstract
{
public:
    MediaSpiderDisk();
    ~MediaSpiderDisk();

public:
    // The condition to judge if should pass the folder or file
    bool PassThisItem(const std::wstring& sItemPath);

    // Judge if it's a media file
    bool IsThisMediaFile(const std::wstring& sFilePath);

    // The main thread and main function that do the spider job
    void _Thread();
    void SearchDisk(const std::wstring& sPath);

protected:
  void SetCurSearchPath(const std::wstring& sPath);

private:
  std::wstring m_sLastSearchPath;

private:
    MediaModel m_model;
};