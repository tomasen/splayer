#pragma once

#include "MediaSQLite.h"

extern const std::wstring g_sDBType;

// Note:
// this class is like a factory class
template<typename T1 = int, typename T2 = int, typename T3 = int, typename T4 = int, typename T5 = int >
class MediaDB
{
public:
  static void exec(const std::wstring &sql
                  , T1 *pRet1 = 0
                  , T2 *pRet2 = 0
                  , T3 *pRet3 = 0
                  , T4 *pRet4 = 0
                  , T5 *pRet5 = 0)
  {
    if (g_sDBType == L"sqlite")
    {
      MediaSQLite<T1, T2, T3, T4, T5 >::exec(sql, pRet1, pRet2, pRet3, pRet4, pRet5);
    }
  }

  static void exec(const std::wstring &sql
                , std::vector<T1 > *pRet1
                , std::vector<T2 > *pRet2 = 0
                , std::vector<T3 > *pRet3 = 0
                , std::vector<T4 > *pRet4 = 0
                , std::vector<T5 > *pRet5 = 0)
  {
    if (g_sDBType == L"sqlite")
    {
      MediaSQLite<T1, T2, T3, T4, T5 >::exec(sql, pRet1, pRet2, pRet3, pRet4, pRet5);
    }
  }

  static void last_insert_rowid(long long &llRet)
  {
    if (g_sDBType == L"sqlite")
    {
      MediaSQLite<T1, T2, T3, T4, T5 >::last_insert_rowid(llRet);
    }
  }

protected:
  static void init(const std::wstring &DBType = L"sqlite")
  {
    g_sDBType = DBType;

    if (DBType == L"sqlite")
    {
      MediaSQLite<T1, T2, T3, T4, T5 >::init();
    }
  }
};