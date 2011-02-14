#pragma once

#include "MediaComm.h"
#include "SourceModel.h"
#include <sqlitepp/sqlitepp.hpp>

// When search the media, use this to judge if the media should be searched
typedef struct
{
    int uniqueid;
    std::wstring filename;
} MediaFindCondition;

// An implement class to add, find and delete media data in the sqlite database
class MediaModel :
    public SourceModel<MediaData, MediaFindCondition>
{
public:
    MediaModel();
    ~MediaModel();
    
public:
    int GetCount();

    void Add(const MediaData& mdData);
    void Add(const MediaDatas& data);

    void FindOne(MediaData& data, const MediaFindCondition& condition);
    void Find(MediaDatas& data,
                const MediaFindCondition& condition,
                int limit_start, int limit_end);

    void Delete(const MediaFindCondition& condition);
    void DeleteAll();

private:
  sqlitepp::session m_db;
};