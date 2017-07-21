#ifndef REQUEST_CTOCPP_H
#define REQUEST_CTOCPP_H

#include "ctocpp.h"
#include "../sinet_dyn/sinet_capi.h"
#include "../sinet/request.h"

class request_ctocpp:
  public ctocpp<request_ctocpp, request, _request_t>
{
public:
  request_ctocpp(_request_t* rqst)
    : ctocpp<request_ctocpp, request, _request_t>(rqst) {}
  virtual ~request_ctocpp() {}

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
};

#endif // REQUEST_CTOCPP_H