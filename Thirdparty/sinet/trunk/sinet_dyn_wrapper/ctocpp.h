#ifndef CTOCPP_H
#define CTOCPP_H

#include "../sinet/api_base.h"
#include "../sinet/api_refptr.h"
#include "../sinet_dyn/sinet_capi.h"

template <class ClassName, class BaseName, class StructName>
class ctocpp : public threadsafe_base<BaseName>
{
public:
  // Use this method to create a wrapper class instance for a structure
  // received from the other side.
  static refptr<BaseName> Wrap(StructName* s)
  {
    // Wrap their structure with the ctocpp object.
    ClassName* wrapper = new ClassName(s);
    // Put the wrapper object in a smart pointer.
    refptr<BaseName> wrapperPtr(wrapper);
    // Release the reference that was added to the CefCppToC wrapper object on
    // the other side before their structure was passed to us.
    wrapper->UnderlyingRelease();
    // Return the smart pointer.
    return wrapperPtr;
  }

  // Use this method to retrieve the underlying structure from a wrapper class
  // instance for return back to the other side.
  static StructName* Unwrap(refptr<BaseName> c)
  {
    // Cast the object to our wrapper class type.
    ClassName* wrapper = static_cast<ClassName*>(c.get());
    // Add a reference to the CefCppToC wrapper object on the other side that
    // will be released once the structure is received.
    wrapper->UnderlyingAddRef();
    // Return their original structure.
    return wrapper->GetStruct();
  }

  ctocpp(StructName* str)
    : struct_(str)
  {
  }
  virtual ~ctocpp()
  {
  }

  // If returning the structure across the DLL boundary you should call
  // UnderlyingAddRef() on this wrapping ctocpp object.  On the other side of
  // the DLL  boundary, call Release() on the CefCppToC object.
  StructName* GetStruct() { return struct_; }

  // CefBase methods increment/decrement reference counts on both this object
  // and the underlying wrapped structure.
  virtual int AddRef()
  {
    UnderlyingAddRef();
    return threadsafe_base<BaseName>::AddRef();
  }
  virtual int Release()
  {
    UnderlyingRelease();
    return threadsafe_base<BaseName>::Release();
  }

  // Increment/decrement reference counts on only the underlying class.
  int UnderlyingAddRef()
  {
    if(!struct_->base.add_ref)
      return 0;
    return struct_->base.add_ref(&struct_->base);
  }
  int UnderlyingRelease()
  {
    if(!struct_->base.release)
      return 0;
    return struct_->base.release(&struct_->base);
  }
  int UnderlyingGetRefCt()
  {
    if(!struct_->base.get_refct)
      return 0;
    return struct_->base.get_refct(&struct_->base);
  }

protected:
  StructName* struct_;
};

#endif // CTOCPP_H