#include "pch.h"
#include "postdataelem_impl.h"

using namespace sinet;

refptr<postdataelem> postdataelem::create_instance()
{
  refptr<postdataelem> _postdataelem(new postdataelem_impl());
  return _postdataelem;
}

void postdataelem_impl::set_name(const wchar_t* fieldname)
{
  m_name = fieldname;
}

std::wstring postdataelem_impl::get_name()
{
  return m_name;
}

void postdataelem_impl::setto_empty()
{
  m_type = PDE_TYPE_EMPTY;
}

void postdataelem_impl::setto_file(const wchar_t* filename)
{
  m_type = PDE_TYPE_FILE;
  m_filename = filename;
}

void postdataelem_impl::setto_buffer(const void* bytes_in, const size_t size_in)
{
  if (size_in == 0)
    return;
  m_type = PDE_TYPE_BYTES;
  m_buffer.resize(size_in);
  memcpy(&m_buffer[0], bytes_in, size_in);
}

void postdataelem_impl::setto_text(const wchar_t* text)
{
  m_type = PDE_TYPE_TEXT;
  m_text = text;
}

postdataelem_type_t postdataelem_impl::get_type()
{
  return m_type;
}

std::wstring postdataelem_impl::get_file()
{
  return m_filename;
}

size_t postdataelem_impl::get_buffer_size()
{
  return m_buffer.size();
}

size_t postdataelem_impl::copy_buffer_to(void* bytes_inout, size_t size_in)
{
  if (size_in > 0 && !m_buffer.empty())
    memcpy(bytes_inout, &m_buffer[0], size_in);
  return size_in;
}

std::wstring postdataelem_impl::get_text()
{
  return m_text;
}