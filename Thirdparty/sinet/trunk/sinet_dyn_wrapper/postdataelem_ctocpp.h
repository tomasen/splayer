#ifndef POSTDATAELEM_CTOCPP_H
#define POSTDATAELEM_CTOCPP_H

#include "ctocpp.h"
#include "../sinet_dyn/sinet_capi.h"
#include "../sinet/postdataelem.h"

class postdataelem_ctocpp:
  public ctocpp<postdataelem_ctocpp, postdataelem, _postdataelem_t>
{
public:
  postdataelem_ctocpp(_postdataelem_t* pde)
    : ctocpp<postdataelem_ctocpp, postdataelem, _postdataelem_t>(pde) {}
  virtual ~postdataelem_ctocpp() {}

  virtual void set_name(const wchar_t* fieldname);
  virtual std::wstring get_name();

  virtual void setto_empty();
  virtual void setto_file(const wchar_t* filename);
  virtual void setto_buffer(const void* bytes_in, const size_t size_in);
  virtual void setto_text(const wchar_t* text);

  virtual postdataelem_type_t get_type();

  virtual std::wstring get_file();
  virtual size_t get_buffer_size();
  virtual size_t copy_buffer_to(void* bytes_inout, size_t size_in);
  virtual std::wstring get_text();
};

#endif // POSTDATAELEM_CTOCPP_H