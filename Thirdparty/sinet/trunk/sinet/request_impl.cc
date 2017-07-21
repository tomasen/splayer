#include "pch.h"
#include "request_impl.h"
#define _min(x,y) x<y?x:y
using namespace sinet;

refptr<request> request::create_instance()
{
  refptr<request> _request(new request_impl());
  return _request;
}

request_impl::request_impl(void):
  m_response_size(0),
  m_retrieved_size(0),
  m_request_outmode(REQ_OUTBUFFER)
{

}

request_impl::~request_impl(void)
{

}

void request_impl::set_request_method(const wchar_t* method)
{
  m_method = method;
}

std::wstring request_impl::get_request_method()
{
  return m_method;
}

void request_impl::set_request_url(const wchar_t* url)
{
  m_url = url;
}

std::wstring request_impl::get_request_url()
{
  return m_url;
}

void request_impl::set_request_header(si_stringmap& header)
{
  m_header = header;
}

si_stringmap request_impl::get_request_header()
{
  return m_header;
}

void request_impl::set_postdata(refptr<postdata> postdata)
{
  m_postdata = postdata;
}

refptr<postdata> request_impl::get_postdata()
{
  return m_postdata;
}

void request_impl::set_response_header(si_stringmap& header)
{
  m_response_header = header;
}

si_stringmap request_impl::get_response_header()
{
  return m_response_header;
}

void request_impl::set_response_buffer(si_buffer& buffer)
{
  size_t rsz = _min(buffer.size(), m_response_size);
  if (rsz == 0)
    return;
  m_response_buffer.resize(rsz);
  memcpy(&m_response_buffer[0], &buffer[0], rsz);
}

si_buffer request_impl::get_response_buffer()
{
  return m_response_buffer;
}

void request_impl::set_response_size(size_t size_in)
{
  m_response_size = size_in;
}

size_t request_impl::get_response_size()
{
  return m_response_size;
}

void request_impl::set_retrieved_size(size_t size_in)
{
  m_retrieved_size = size_in;
}

size_t request_impl::get_retrieved_size()
{
  return m_retrieved_size;
}

void request_impl::set_response_errcode(int errcode)
{
  m_response_errcode = errcode;
}

// if HTTP request success, return 0, otherwise return it response status
int request_impl::get_response_errcode()
{
  return 200==m_response_errcode?0:m_response_errcode;
}

void request_impl::set_request_outmode(int outmode)
{
  m_request_outmode = outmode;
}

int request_impl::get_request_outmode()
{
  return m_request_outmode;
}

void request_impl::set_outfile(const wchar_t *file)
{
  m_outfile = file;
#ifdef WIN32
  _wremove(m_outfile.c_str());
#elif defined(_MAC_)
	unlink(Utf8(m_outfile.c_str()));
#elif defined(__linux__)
  unlink((wchar_utf8(m_outfile)).c_str());
#endif
}

std::wstring request_impl::get_outfile()
{
  return m_outfile;
}

void request_impl::close_outfile()
{
  if (m_outstream.is_open())
    m_outstream.close();
}

void request_impl::set_appendbuffer(const void* data, size_t size)
{
  if (size == 0)
    return;

  m_retrieved_size += size;

  switch (m_request_outmode)
  {
  // save data to buffer
  case REQ_OUTBUFFER:
    {
    size_t lastsize = m_response_buffer.size();
    m_response_buffer.resize(lastsize + size);
    memcpy(&m_response_buffer[lastsize], data, size);
    break;
    }
  // save data to file
  case REQ_OUTFILE:
    if (!m_outstream.is_open())
    {
#ifdef _MAC_ 
      m_outstream.open(Utf8(m_outfile.c_str()), std::ios::out|std::ios::binary);
#elif  defined(__linux__)
      m_outstream.open((wchar_utf8(m_outfile)).c_str(), std::ios::out|std::ios::binary);
#else
			m_outstream.open(m_outfile.c_str(), std::ios::out|std::ios::binary);
#endif
			if (!m_outstream.is_open())
        return;
    }
    m_outstream.write((char*)data, size);
    break;
  }
}
