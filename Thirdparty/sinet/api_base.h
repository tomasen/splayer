#ifndef API_BASE_H
#define API_BASE_H

namespace sinet
{

class base
{
public:
  virtual int AddRef() =0;
  virtual int Release() =0;
  virtual int GetRefCt() = 0;
};

#if defined(_WINDOWS_)

#define atomic_increment(p) InterlockedIncrement(p)
#define atomic_decrement(p) InterlockedDecrement(p)
  
#elif defined(_MAC_) || defined(__linux__)

#define atomic_increment(p) __sync_fetch_and_add(p, 1)
#define atomic_decrement(p) __sync_fetch_and_sub(p, 1)
  
#endif

  
class critical_section
{
public:
  critical_section()
  {
#if defined(_WINDOWS_)
    memset(&m_sec, 0, sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(&m_sec);
#elif defined(_MAC_) || defined(__linux__)
    pthread_mutex_init(&m_mut, NULL);   
#endif
  }
  ~critical_section()
  {
#if defined(_WINDOWS_)
    DeleteCriticalSection(&m_sec);
#elif defined(_MAC_) || defined(__linux__)
    pthread_mutex_destroy(&m_mut);
#endif
  }
  void lock()
  {
#if defined(_WINDOWS_)
    EnterCriticalSection(&m_sec);
#elif defined(_MAC_) || defined(__linux__)
    pthread_mutex_lock(&m_mut);
#endif
  }
  void unlock()
  {
#if defined(_WINDOWS_)
    LeaveCriticalSection(&m_sec);
#elif defined(_MAC_) || defined(__linux__)
    pthread_mutex_unlock(&m_mut);
#endif
  }
#if defined(_WINDOWS_)
  CRITICAL_SECTION m_sec;
#elif defined(_MAC_) || defined(__linux__)
  pthread_mutex_t m_mut;
#endif
};

class auto_criticalsection
{
public:
  auto_criticalsection(critical_section& cs): m_cs(&cs)
  {
    m_cs->lock();
  }
  virtual ~auto_criticalsection()
  {
    m_cs->unlock();
  }
private:
  critical_section* m_cs;
};

template <class ClassName>
class threadsafe_base : public ClassName
{
public:
  threadsafe_base()
  {
    m_dwRef = 0L;
  }
  virtual ~threadsafe_base()
  {
  }

  virtual int AddRef()
  {
    return atomic_increment(&m_dwRef);
  }

  virtual int Release()
  {
    int retval = atomic_decrement(&m_dwRef);
    if(retval == 0)
      delete this;
    return retval;
  }

  virtual int GetRefCt() { return m_dwRef; }

  void Lock() { m_critsec.lock(); }
  void Unlock() { m_critsec.unlock(); }

protected:
  long m_dwRef;
  critical_section m_critsec;
};

} // namespace sinet

#endif // API_BASE_H
