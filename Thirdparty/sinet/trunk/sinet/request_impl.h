#ifndef SINET_REQUEST_IMPL_H
#define SINET_REQUEST_IMPL_H

#include "request.h"
#include <fstream>

namespace sinet
{

class request_impl:
  public threadsafe_base<request>
{
public:
  request_impl(void);
  ~request_impl(void);

  virtual void set_request_method(const wchar_t* method);
  virtual std::wstring get_request_method();

  virtual void set_request_url(const wchar_t* url);
  virtual std::wstring get_request_url();

  virtual void set_request_header(si_stringmap& header);
  virtual si_stringmap get_request_header();

  virtual void set_postdata(refptr<postdata> postdata);
  virtual refptr<postdata> get_postdata();

  virtual void set_response_header(si_stringmap& header);
  virtual si_stringmap get_response_header();

  virtual void set_response_buffer(si_buffer& buffer);
  virtual si_buffer get_response_buffer();

  virtual void set_response_size(size_t size_in);
  virtual size_t get_response_size();

  virtual void set_retrieved_size(size_t size_in);
  virtual size_t get_retrieved_size();

  virtual void set_response_errcode(int errcode);
  virtual int get_response_errcode();

  virtual void set_request_outmode(int outmode);
  virtual int get_request_outmode();

  virtual void set_outfile(const wchar_t* file);
  virtual std::wstring get_outfile();

  virtual void close_outfile();

  virtual void set_appendbuffer(const void* data, size_t size);

private:
  std::wstring  m_url;
  std::wstring  m_method;
  si_buffer     m_response_buffer;
  size_t        m_response_size;
  size_t        m_retrieved_size;
  si_stringmap  m_header;
  si_stringmap  m_response_header;
  int           m_response_errcode;
  
  int           m_request_outmode;
  std::wstring  m_outfile;

  std::ofstream m_outstream;

  refptr<postdata> m_postdata;
};

} // namespace sinet

#endif // SINET_REQUEST_IMPL_H