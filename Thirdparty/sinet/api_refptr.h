#ifndef API_REFPTR_H
#define API_REFPTR_H

namespace sinet
{

template <class T>
class refptr
{
public:
  refptr() : ptr_(NULL)
  {
  }

  refptr(T* p) : ptr_(p)
  {
    if (ptr_)
      ptr_->AddRef();
  }

  refptr(const refptr<T>& r) : ptr_(r.ptr_) {
    if (ptr_)
      ptr_->AddRef();
  }

  ~refptr() {
    if (ptr_)
      ptr_->Release();
  }

  T* get() const { return ptr_; }
  operator T*() const { return ptr_; }
  T* operator->() const { return ptr_; }

  refptr<T>& operator=(T* p) {
    // AddRef first so that self assignment should work
    if (p)
      p->AddRef();
    if (ptr_ )
      ptr_ ->Release();
    ptr_ = p;
    return *this;
  }

  refptr<T>& operator=(const refptr<T>& r) {
    return *this = r.ptr_;
  }

  void swap(T** pp) {
    T* p = ptr_;
    ptr_ = *pp;
    *pp = p;
  }

  void swap(refptr<T>& r) {
    swap(&r.ptr_);
  }

private:
  T* ptr_;
};

} // namespace sinet

#endif // API_REFPTR_H