#pragma once

#include "MediaModel.h"
#include <threadhelper.h>

class MediaSpiderDisk :
    public ThreadHelperImpl<MediaSpiderDisk>
{
public:
    MediaSpiderDisk();
    ~MediaSpiderDisk();

    void _Thread();

private:
    MediaModel m_model;
};