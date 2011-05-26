#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

class CriticalSection
{
public:
  CriticalSection()
  {
#if defined(_WINDOWS_)
    memset(&m_sec, 0, sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(&m_sec);
#elif defined(_MAC_)
    pthread_mutex_init(&m_mut, NULL);
#endif
  }
  ~CriticalSection()
  {
#if defined(_WINDOWS_)
    DeleteCriticalSection(&m_sec);
#elif defined(_MAC_)
    pthread_mutex_destroy(&m_mut);
#endif
  }
  void lock()
  {
#if defined(_WINDOWS_)
    EnterCriticalSection(&m_sec);
#elif defined(_MAC_)
    pthread_mutex_lock(&m_mut);
#endif
  }
  void unlock()
  {
#if defined(_WINDOWS_)
    LeaveCriticalSection(&m_sec);
#elif defined(_MAC_)
    pthread_mutex_unlock(&m_mut);
#endif
  }
#if defined(_WINDOWS_)
  CRITICAL_SECTION m_sec;
#elif defined(_MAC_)
  pthread_mutex_t m_mut;
#endif
};

class AutoCSLock
{
public:
  AutoCSLock(CriticalSection& cs): m_cs(&cs)
  {
    m_cs->lock();
  }
  virtual ~AutoCSLock()
  {
    m_cs->unlock();
  }
private:
  CriticalSection* m_cs;
};


#endif // CRITICALSECTION_H