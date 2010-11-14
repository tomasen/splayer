#ifndef LAZYINSTANCE_H
#define LAZYINSTANCE_H

template<class T>
class LazyInstanceImpl
{
public:
  static std::auto_ptr<T> m_instance;
  static T* GetInstance()
  {
    if (m_instance.get())
      return m_instance.get();
    else
    {
      m_instance.reset(new T());
      return m_instance.get();
    }
  }
};

#define DECLARE_LAZYINSTANCE(classname) \
  std::auto_ptr<classname> LazyInstanceImpl<classname>::m_instance;

#endif // LAZYINSTANCE_H
