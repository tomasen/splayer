#ifndef POSTDATAELEM_IMPL_H
#define POSTDATAELEM_IMPL_H

#include "postdataelem.h"

namespace sinet
{

class postdataelem_impl:
  public threadsafe_base<postdataelem>
{
public:
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

private:
  std::wstring m_name;
  std::wstring m_filename;
  std::wstring m_text;
  std::vector<unsigned char> m_buffer;
  postdataelem_type_t  m_type;
};

} // namespace sinet

#endif // POSTDATAELEM_IMPL_H