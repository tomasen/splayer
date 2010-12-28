#include "StdAfx.h"
#include "MediaModel.h"

MediaModel::MediaModel()
{
}

MediaModel::~MediaModel()
{
}

void MediaModel::Find(MediaDatas &data, const MediaFindCondition &condition,
                      int limit_start, int limit_end)
{
    for (int i=0;i<limit_end;i++)
    {
        wchar_t filename[80], imgpath[80];
        wsprintf(filename, L"filename_%d", i);
        wsprintf(imgpath, L"thumbnailpath_%d", i);

        MediaData rs;
        rs.filename = filename;
        rs.thumbnailpath = imgpath;

        data.push_back(rs);
    }

}