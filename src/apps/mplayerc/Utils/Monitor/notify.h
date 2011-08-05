#pragma once

#include <Windows.h>
#include <boost/signals2.hpp>

#define ID_MONITOR_CHANGED 100

namespace MT
{
  // notifier is used for notify the observers when an event occur
  class notifier
  {
  public:
    typedef void (*direct_type)();
    typedef boost::signals2::signal<void ()> type_direct_sig;

  public:
    // add listeners, 3 ways
    void add_listener_direct(direct_type func); // direct call
    void add_listener_sendmsg(HWND hwnd);       // send message to window
    void add_listener_postmsg(HWND hwnd);       // post message to window

    // delete listeners
    void del_listener_direct(direct_type func);
    void del_listener_sendmsg(HWND hwnd);
    void del_listener_postmsg(HWND hwnd);

    // notify the observer
    void notify();

  private:
    type_direct_sig m_sig;
    std::vector<HWND> m_sendmsg;
    std::vector<HWND> m_postmsg;
  };
}