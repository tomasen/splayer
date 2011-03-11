#pragma once

#include <sqlitepp/sqlitepp.hpp>
#include "../Controller/LazyInstance.h"
#include "../../../base/CriticalSection.h"

// Note:
// this class is used for read and write media database
class MediaSQLite : public LazyInstanceImpl<MediaSQLite>
{
public:
  MediaSQLite();
  ~MediaSQLite();

public:
  void exec(const std::wstring &sql);  // just run the sql

  template<typename T >
  void exec(const std::wstring &sql, T &ret)  // get one result(the first result, also only one column)
  {
    m_cs.lock();

    try
    {
      sqlitepp::statement st(m_db);
      st << sql, sqlitepp::into(ret);
      st.exec();
    }
    catch (std::runtime_error const& err)
    {
      Logging(err.what());
    }

    m_cs.unlock();
  }

  template<typename T >
  void exec(const std::wstring &sql, std::vector<T > &ret)  // get one result(also only one column)
  {
    m_cs.lock();

    try
    {
      sqlitepp::statement st(m_db);
      T t;
      st << sql, sqlitepp::into(t);
      while (st.exec())
        ret.push_back(t);
    }
    catch (std::runtime_error const& err)
    {
      Logging(err.what());
    }

    m_cs.unlock();
  }

  long long last_insert_rowid();      // Last insert row ID

private:
  sqlitepp::session m_db;
  CriticalSection m_cs;
};