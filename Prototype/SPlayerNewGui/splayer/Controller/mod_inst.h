#ifndef MOD_INST_H
#define MOD_INST_H

// ModuleInstanceImpl is a per-module singleton instance implementation
template<class T>
class ModuleInstanceImpl
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

#define DECLARE_MODULEINSTANCE(classname) \
  std::auto_ptr<classname> ModuleInstanceImpl<classname>::m_instance;

#endif // MOD_INST_H
