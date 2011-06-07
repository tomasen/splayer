#ifndef LOCK_H
#define LOCK_H

namespace lock
{

class Mutex
{
public:
  Mutex()
  {
#if defined(_WINDOWS_)
    memset(&m_sec, 0, sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(&m_sec);
#elif defined(_MAC_)
    pthread_mutex_init(&m_mut, NULL);
#endif
  }
  ~Mutex()
  {
#if defined(_WINDOWS_)
    DeleteCriticalSection(&m_sec);
#elif defined(_MAC_)
    pthread_mutex_destroy(&m_mut);
#endif
  }
  void Lock()
  {
#if defined(_WINDOWS_)
    EnterCriticalSection(&m_sec);
#elif defined(_MAC_)
    pthread_mutex_lock(&m_mut);
#endif
  }
  bool TryLock()
  {
#if defined(_WINDOWS_)
    if (TryEnterCriticalSection(&m_sec))
      return true;
    return false;
#endif
  }
  void Unlock()
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

class AutoLock
{
public:
  AutoLock(Mutex& cs): m_cs(&cs)
  {
    m_cs->Lock();
  }
  virtual ~AutoLock()
  {
    m_cs->Unlock();
  }
private:
  Mutex* m_cs;
};

} // namespace lock

#endif // LOCK_H