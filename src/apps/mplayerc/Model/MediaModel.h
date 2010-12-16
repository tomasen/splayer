#pragma once

#include "MediaComm.h"
#include "SourceModel.h"
#include <sqlitepp/sqlitepp.hpp>

typedef struct
{
    int uniqueid;
    std::wstring filename;
} MediaFindCondition;

class MediaModel :
    public SourceModel<MediaData, MediaFindCondition>
{
public:
    MediaModel();
    ~MediaModel();
    
    int GetCount();

    void Add(const MediaDatas& data);

    void FindOne(MediaData& data, const MediaFindCondition& condition);
    void Find(MediaDatas& data,
                const MediaFindCondition& condition,
                int limit_start, int limit_end);

    void Delete(const MediaFindCondition& condition);
    void DeleteAll();

};