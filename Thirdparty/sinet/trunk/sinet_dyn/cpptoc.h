#ifndef CPPTOC
#define CPPTOC

#include "../sinet/api_base.h"
#include "../sinet/api_refptr.h"
#include "../sinet_dyn/sinet_capi.h"

template <class ClassName, class BaseName, class StructName>
class cpptoc : public threadsafe_base<base>
{
public:
  // Use this method to retrieve the underlying class instance from our
  // own structure when the structure is passed as the required first
  // parameter of a C API function call. No explicit reference counting
  // is done in this case.
  static refptr<BaseName> Get(StructName* s)
  {
    // Cast our structure to the wrapper structure type.
    ClassName::Struct* wrapperStruct =
      reinterpret_cast<ClassName::Struct*>(s);
    // Return the underlying object instance.
    return wrapperStruct->class_->GetClass();
  }

  // Use this method to create a wrapper structure for passing our class
  // instance to the other side.
  static StructName* Wrap(refptr<BaseName> c)
  {
    // Wrap our object with the cpptoc class.
    ClassName* wrapper = new ClassName(c);
    // Add a reference to our wrapper object that will be released once our
    // structure arrives on the other side.
    wrapper->AddRef();
    // Return the structure pointer that can now be passed to the other side.
    return wrapper->GetStruct();
  }

  // Use this method to retrieve the underlying class instance when receiving
  // our wrapper structure back from the other side.
  static refptr<BaseName> Unwrap(StructName* s)
  {
    // Cast our structure to the wrapper structure type.
    ClassName::Struct* wrapperStruct =
      reinterpret_cast<ClassName::Struct*>(s);
    // Add the underlying object instance to a smart pointer.
    refptr<BaseName> objectPtr(wrapperStruct->class_->GetClass());
    // Release the reference to our wrapper object that was added before the
    // structure was passed back to us.
    wrapperStruct->class_->Release();
    // Return the underlying object instance.
    return objectPtr;
  }

  // Structure representation with pointer to the C++ class.
  struct Struct
  {
    StructName struct_;
    cpptoc<ClassName,BaseName,StructName>* class_;
  };

  cpptoc(BaseName* cls)
    : class_(cls)
  {
    struct_.class_ = this;

    // zero the underlying structure and set base members
    memset(&struct_.struct_, 0, sizeof(StructName));
    struct_.struct_.base.size = sizeof(StructName);
    struct_.struct_.base.add_ref = struct_add_ref;
    struct_.struct_.base.release = struct_release;
    struct_.struct_.base.get_refct = struct_get_refct;

  }
  virtual ~cpptoc()
  {
  }

  BaseName* GetClass() { return class_; }

  // If returning the structure across the DLL boundary you should call
  // AddRef() on this cpptoc object.  On the other side of the DLL boundary,
  // call UnderlyingRelease() on the wrapping CefCToCpp object.
  StructName* GetStruct() { return &struct_.struct_; }

  // CefBase methods increment/decrement reference counts on both this object
  // and the underlying wrapper class.
  virtual int AddRef()
  {
    UnderlyingAddRef();
    return threadsafe_base<base>::AddRef();
  }
  virtual int Release()
  {
    UnderlyingRelease();
    return threadsafe_base<base>::Release();
  }

  // Increment/decrement reference counts on only the underlying class.
  int UnderlyingAddRef() { return class_->AddRef(); }
  int UnderlyingRelease() { return class_->Release(); }
  int UnderlyingGetRefCt() { return class_->GetRefCt(); }

private:
  static int SINET_DYN_CALLBACK struct_add_ref(struct __base_t* base)
  {
    if(!base)
      return 0;

    Struct* impl = reinterpret_cast<Struct*>(base);
    return impl->class_->AddRef();
  }

  static int SINET_DYN_CALLBACK struct_release(struct __base_t* base)
  {
    if(!base)
      return 0;

    Struct* impl = reinterpret_cast<Struct*>(base);
    return impl->class_->Release();
  }

  static int SINET_DYN_CALLBACK struct_get_refct(struct __base_t* base)
  {
    if(!base)
      return 0;

    Struct* impl = reinterpret_cast<Struct*>(base);
    return impl->class_->GetRefCt();
  }

protected:
  Struct struct_;
  BaseName* class_;
};


#endif // CPPTOC