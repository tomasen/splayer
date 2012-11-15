#pragma once

#include <Pdh.h>
#include <PdhMsg.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "notify.h"

namespace MT
{
  // base is the base class for all the monitor classes
  // base has the ability to notify the listener if a status is changed
  class monitor_base
  {
  public:
    // constructor/destructor
    monitor_base();
    virtual ~monitor_base();

    // get the current value
    double get_cur_value() const { return m_cur_value; }

    // get notifier to add and remove listeners
    boost::shared_ptr<notifier> get_notifier() { return m_notifier; }

    // query monitor status
    virtual bool is_running() const { return m_is_running; }

    // start and stop monitoring
    virtual void start_monitoring();
    virtual void stop_monitoring();

    // set query interval, default is only 100 milliseconds
    virtual void set_interval(int milliseconds) { m_interval = milliseconds; }

    // pdh counter
    virtual std::wstring get_counter() = 0;

  protected:
    void clean_pdh();    // clean pdh
    void stop_thread();  // stop thread

  private:
    void thread_worker();   // the working thread

  protected:
    // notifier
    boost::shared_ptr<notifier> m_notifier;

    // control the thread
    boost::shared_ptr<boost::thread> m_thread;
    bool m_is_running;
    int m_interval;
    HQUERY m_hQuery;

    // result
    double m_cur_value;
  };
}

namespace MT
{
  // monitor the cpu status
  class cpu : public monitor_base
  {
  public:
    cpu(const std::wstring &which_cpu, const std::wstring &counter_string);
    ~cpu() { stop_thread(); }
    // pdh counter
    virtual std::wstring get_counter();

  private:
    std::wstring m_which_cpu;
    std::wstring m_counter_string;
  };
}

namespace MT
{
  // monitor the disk status
  class disk : public monitor_base
  {
  public:
    disk(const std::wstring &which_disk, const std::wstring &counter_string);
    ~disk() { stop_thread(); }
    // pdh counter
    virtual std::wstring get_counter();

  private:
    std::wstring m_which_disk;
    std::wstring m_counter_string;
  };
}