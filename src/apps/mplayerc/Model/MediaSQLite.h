#pragma once

#include <sqlitepp/sqlitepp.hpp>
#include "../../../base/CriticalSection.h"

extern sqlitepp::session g_dbMediaSQLite;
extern CriticalSection g_csMediaSQLite;

// Note:
// this class is used for read and write sqlite media database
template<typename T1 = int, typename T2 = int, typename T3 = int, typename T4 = int, typename T5 = int >
class MediaSQLite
{
public:
  static void exec(const std::wstring &sql
                  , T1 *pRet1 = 0
                  , T2 *pRet2 = 0
                  , T3 *pRet3 = 0
                  , T4 *pRet4 = 0
                  , T5 *pRet5 = 0)
  {
    g_csMediaSQLite.lock();

    try
    {
      init();
      sqlitepp::statement st(g_dbMediaSQLite);

      if (pRet1 == 0)
      {
        g_dbMediaSQLite << sql;
      }
      else if (pRet2 == 0)
      {
        st << sql
          , sqlitepp::into(*pRet1);
        st.exec();
      }
      else if (pRet3 == 0)
      {
        st << sql
          , sqlitepp::into(*pRet1)
          , sqlitepp::into(*pRet2);
        st.exec();
      }
      else if (pRet4 == 0)
      {
        st << sql
          , sqlitepp::into(*pRet1)
          , sqlitepp::into(*pRet2)
          , sqlitepp::into(*pRet3);
        st.exec();
      }
      else if (pRet5 == 0)
      {
        st << sql
          , sqlitepp::into(*pRet1)
          , sqlitepp::into(*pRet2)
          , sqlitepp::into(*pRet3)
          , sqlitepp::into(*pRet4);
        st.exec();
      }
      else
      {
        st << sql
          , sqlitepp::into(*pRet1)
          , sqlitepp::into(*pRet2)
          , sqlitepp::into(*pRet3)
          , sqlitepp::into(*pRet4)
          , sqlitepp::into(*pRet5);
        st.exec();
      }
    }
    catch (std::runtime_error const& err)
    {
      Logging(err.what());
    }
    catch (...)
    {
      g_csMediaSQLite.unlock();
      throw;
    }

    g_csMediaSQLite.unlock();
  }

  static void exec(const std::wstring &sql
                  , std::vector<T1 > *pRet1
                  , std::vector<T2 > *pRet2 = 0
                  , std::vector<T3 > *pRet3 = 0
                  , std::vector<T4 > *pRet4 = 0
                  , std::vector<T5 > *pRet5 = 0)
  {
    g_csMediaSQLite.lock();

    try
    {
      init();
      sqlitepp::statement st(g_dbMediaSQLite);

      if (pRet2 == 0)
      {
        T1 t1;
        st << sql, sqlitepp::into(t1);
        while (st.exec())
          pRet1->push_back(t1);
      }
      else if (pRet3 == 0)
      {
        T1 t1;
        T2 t2;
        st << sql, sqlitepp::into(t1), sqlitepp::into(t2);
        while (st.exec())
        {
          pRet1->push_back(t1);
          pRet2->push_back(t2);
        }
      }
      else if (pRet4 == 0)
      {
        T1 t1;
        T2 t2;
        T3 t3;
        st << sql, sqlitepp::into(t1), sqlitepp::into(t2), sqlitepp::into(t3);
        while (st.exec())
        {
          pRet1->push_back(t1);
          pRet2->push_back(t2);
          pRet3->push_back(t3);
        }
      }
      else if (pRet5 == 0)
      {
        T1 t1;
        T2 t2;
        T3 t3;
        T4 t4;
        st << sql, sqlitepp::into(t1), sqlitepp::into(t2), sqlitepp::into(t3), sqlitepp::into(t4);
        while (st.exec())
        {
          pRet1->push_back(t1);
          pRet2->push_back(t2);
          pRet3->push_back(t3);
          pRet4->push_back(t4);
        }
      }
      else
      {
        T1 t1;
        T2 t2;
        T3 t3;
        T4 t4;
        T5 t5;
        st << sql, sqlitepp::into(t1), sqlitepp::into(t2), sqlitepp::into(t3), sqlitepp::into(t4), sqlitepp::into(t5);
        while (st.exec())
        {
          pRet1->push_back(t1);
          pRet2->push_back(t2);
          pRet3->push_back(t3);
          pRet4->push_back(t4);
          pRet5->push_back(t5);
        }
      }
    }
    catch (std::runtime_error const& err)
    {
      Logging(err.what());
    }
    catch (...)
    {
      g_csMediaSQLite.unlock();
      throw;
    }

    g_csMediaSQLite.unlock();
  }

  static void last_insert_rowid(long long &llRet)
  {
    g_csMediaSQLite.lock();
  
    llRet = g_dbMediaSQLite.last_insert_rowid();
  
    g_csMediaSQLite.unlock();
  }

protected:
  static void init()
  {
    if (!g_dbMediaSQLite.is_open())
    {
      try
      {
        g_dbMediaSQLite.open(GetModuleFolder() + L"media.db");

        g_dbMediaSQLite << L"CREATE TABLE IF NOT EXISTS media_data ("
          L"uniqueid integer PRIMARY KEY, path text, filename text,"
          L"thumbnailpath text, videotime integer)";
        g_dbMediaSQLite << L"CREATE TABLE IF NOT EXISTS detect_path (" \
          L"uniqueid integer PRIMARY KEY, path text, merit integer)";
      }
      catch (std::runtime_error const& err)
      {
        Logging(err.what());
      }
      catch (...)
      {
        g_csMediaSQLite.unlock();
        throw;
      }
    }
  }

  // helper functions
  static std::wstring GetModuleFolder(HMODULE hModuleHandle = 0)
  {
    TCHAR szModuleFullPath[MAX_PATH] = {0};
    ::GetModuleFileName(hModuleHandle, szModuleFullPath, MAX_PATH);

    TCHAR szDrive[10] = {0};
    TCHAR szDir[MAX_PATH] = {0};

    ::_wsplitpath(szModuleFullPath, szDrive, szDir, 0, 0);

    std::wstring sResult;
    sResult += szDrive;
    sResult += szDir;

    return sResult;
  }
};