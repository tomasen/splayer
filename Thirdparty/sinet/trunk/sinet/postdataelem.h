#ifndef POSTDATAELEM_H
#define POSTDATAELEM_H

#include "api_base.h"
#include "api_refptr.h"
#include "api_types.h"

namespace sinet
{

//////////////////////////////////////////////////////////////////////////
//
//  class postdataelem is the single entry in postdata
//
class postdataelem:
  public base
{
public:
  static refptr<postdataelem> create_instance();

  // field name for this entry
  virtual void set_name(const wchar_t* fieldname) = 0;
  virtual std::wstring get_name() = 0;

  // clear contents of postdataelem
  virtual void setto_empty() = 0;
  // set to file
  virtual void setto_file(const wchar_t* filename) = 0;
  // set to buffer
  virtual void setto_buffer(const void* bytes_in, const size_t size_in) = 0;
  // set to text (field)
  virtual void setto_text(const wchar_t* text) = 0;

  // return internal type
  virtual postdataelem_type_t get_type() = 0;

  // return file name
  virtual std::wstring get_file() = 0;
  // return buffer size
  virtual size_t get_buffer_size() = 0;
  // copy internal buffer out to |bytes_inout| with limit of |size_in| bytes
  // returning the actual size copied
  virtual size_t copy_buffer_to(void* bytes_inout, size_t size_in) = 0;
  virtual std::wstring get_text() = 0;
};

} // namespace sinet

#endif // POSTDATAELEM_H