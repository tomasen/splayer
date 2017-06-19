#ifndef SINET_REQUEST_H
#define SINET_REQUEST_H

#include "api_base.h"
#include "api_refptr.h"
#include "postdata.h"

namespace sinet
{

//////////////////////////////////////////////////////////////////////////
//
//  request class
//
//    A request contains the parameter of this network transaction, as
//    well as the returning buffer set by the pool.
//
//    The current design of request emphasizes on HTTP only.
//

// be used for response output
#define   REQ_OUTFILE         1
#define   REQ_OUTBUFFER       2

class request:
  public base
{
public:
  static refptr<request> create_instance();

  // request method
  virtual void set_request_method(const wchar_t* method) = 0;
  virtual std::wstring get_request_method() = 0;

  // request url
  virtual void set_request_url(const wchar_t* url) = 0;
  virtual std::wstring get_request_url() = 0;

  // request header
  virtual void set_request_header(si_stringmap& header) = 0;
  virtual si_stringmap get_request_header() = 0;

  // request postdata
  virtual void set_postdata(refptr<postdata> postdata) = 0;
  virtual refptr<postdata> get_postdata() = 0;

  // response header
  virtual void set_response_header(si_stringmap& header) = 0;
  virtual si_stringmap get_response_header() = 0;

  // response content buffer
  virtual void set_response_buffer(si_buffer& buffer) = 0;
  virtual si_buffer get_response_buffer() = 0;

  // response content buffer size only
  virtual void set_response_size(size_t size_in) = 0;
  virtual size_t get_response_size() = 0;

  // response content buffer retrieved size
  virtual void set_retrieved_size(size_t size_in) = 0;
  virtual size_t get_retrieved_size() = 0;

  // response error code
  virtual void set_response_errcode(int errcode) = 0;
  virtual int get_response_errcode() = 0;

  virtual void set_request_outmode(int outmode) = 0;
  virtual int get_request_outmode() = 0;

  virtual void set_outfile(const wchar_t* file) = 0;
  virtual std::wstring get_outfile() = 0;
  virtual void close_outfile() = 0;

  virtual void set_appendbuffer(const void* data, size_t size) = 0;
};

} // namespace sinet

#endif // SINET_REQUEST_H