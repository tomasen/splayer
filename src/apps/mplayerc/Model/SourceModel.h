#pragma once

// Source Model, an abstract class used for add, find and delete media data
template<class TDATA, class TCONDITION>
class SourceModel
{
public:
    int GetCount() {}
    
    void Add(const std::vector<TDATA>& data) {}

    void Find(std::vector<TDATA>& data, const TCONDITION& condition, int limit_start, int limit_end) {}
    void FindOne(TDATA& data, const TCONDITION& condition) {}
    void FindAll(TDATA& data) {}
    void Delete(const TCONDITION& condition) {}
    void DeleteAll() {}
};