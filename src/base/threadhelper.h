#ifndef BASE_THREADHELPER_H
#define BASE_THREADHELPER_H

/* 
 * This file include:
 *  1. class ThreadHelperImpl
 *
 * Example:
 *
 * class ThreadSample : public ThreadHelperImpl<ThreadSample>
 * {
 *   // do something
 *   void _Thread()
 *   {
 * 
 *   }
 * };
 *
 * ThreadSample mythread;
 * mythread._Stop();
 * mythread._Start();
 *
 */

template<class T>
class ThreadHelperImpl
{
public:
  ThreadHelperImpl():m_thread(NULL),
                    m_stopevent(::CreateEvent(NULL, TRUE, FALSE, NULL)) {}
  ~ThreadHelperImpl() {_Stop();}

  // Create a new thread
  void _Start()
  {
    m_thread = (HANDLE)::_beginthread(Logic, 0, (void*)this);
  }

  // Stop the thread
  void _Stop()
  {
    ::SetEvent(m_stopevent);
    while (_Is_alive())
      ::WaitForSingleObject(m_thread, 1000);

    m_thread = NULL;
    ::ResetEvent(m_stopevent);
  }

  // Use thread do something
  void _Thread() {}

  // Check thread event state.
  // The _Stop Method affecting this state.
  // true if the thread stop,otherwise false.
  bool _Exit_state(int wait_msec)
  {
    return (::WaitForSingleObject(m_stopevent, wait_msec) == WAIT_OBJECT_0)
              ? true : false;
  }

  // thread alive;
  // true if thread is alive, otherwise false.
  bool _Is_alive()
  {
    unsigned long thread_exitcode;
    return (m_thread && m_thread != INVALID_HANDLE_VALUE &&
      GetExitCodeThread(m_thread, &thread_exitcode) &&
      thread_exitcode == STILL_ACTIVE) ? true : false;
  }

private:
  static void Logic(void* t)
  {
     static_cast<T*>(t)->_Thread();
  }

private:
  HANDLE m_thread;
  HANDLE m_stopevent;
};

#endif // BASE_THREADHELPER_H