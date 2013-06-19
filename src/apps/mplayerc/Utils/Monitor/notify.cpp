#include "stdafx.h"
#include "notify.h"

// add listeners
void MT::notifier::add_listener_direct(MT::notifier::direct_type func)
{
  m_sig.connect(func);
}

void MT::notifier::add_listener_sendmsg(HWND hwnd)
{
  m_sendmsg.push_back(hwnd);
}

void MT::notifier::add_listener_postmsg(HWND hwnd)
{
  m_postmsg.push_back(hwnd);
}

// delete listeners
void MT::notifier::del_listener_direct(direct_type func)
{
  m_sig.disconnect(func);
}

void MT::notifier::del_listener_sendmsg(HWND hwnd)
{
  std::vector<HWND>::iterator it;
  it = std::find(m_sendmsg.begin(), m_sendmsg.end(), hwnd);
  if (it != m_sendmsg.end())
    m_sendmsg.erase(it);
}

void MT::notifier::del_listener_postmsg(HWND hwnd)
{
  std::vector<HWND>::iterator it;
  it = std::find(m_postmsg.begin(), m_postmsg.end(), hwnd);
  if (it != m_postmsg.end())
    m_postmsg.erase(it);
}

// notify the observer
void MT::notifier::notify()
{
  if (!m_sig.empty())
    m_sig();

  for (size_t i = 0; i < m_sendmsg.size(); ++i)
    ::SendMessage(m_sendmsg[i], WM_COMMAND, (WPARAM)ID_MONITOR_CHANGED, 0);

  for (size_t i = 0; i < m_postmsg.size(); ++i)
    ::PostMessage(m_sendmsg[i], WM_COMMAND, (WPARAM)ID_MONITOR_CHANGED, 0);
}