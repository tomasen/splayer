#include "pch.h"
#include "request_cpptoc.h"
#include "postdata_cpptoc.h"

using namespace sinet;

SINET_DYN_API _request_t* _request_create_instance()
{
  refptr<request> impl = request::create_instance();
  if (impl.get())
    return request_cpptoc::Wrap(impl);
  return NULL;
}

void SINET_DYN_CALLBACK _set_request_method(struct __request_t* self, const wchar_t* method)
{
  request_cpptoc::Get(self)->set_request_method(method);
}

_string_t SINET_DYN_CALLBACK _get_request_method(struct __request_t* self)
{
  std::wstring _method = request_cpptoc::Get(self)->get_request_method();
  return _string_alloc(_method.c_str());
}

void SINET_DYN_CALLBACK _set_request_url(struct __request_t* self, const wchar_t* url)
{
  request_cpptoc::Get(self)->set_request_url(url);
}

_string_t SINET_DYN_CALLBACK _get_request_url(struct __request_t* self)
{
  std::wstring _url = request_cpptoc::Get(self)->get_request_url();
  return _string_alloc(_url.c_str());
}

void SINET_DYN_CALLBACK _set_request_header(struct __request_t* self, _stringmap_t* header)
{
  request_cpptoc::Get(self)->set_request_header(
    *(reinterpret_cast<std::map<std::wstring, std::wstring>*>(header)));
}

_stringmap_t SINET_DYN_CALLBACK _get_request_header(struct __request_t* self)
{
  _stringmap_t header = _stringmap_alloc();
  std::map<std::wstring, std::wstring> _header = request_cpptoc::Get(self)->get_request_header();
  for (std::map<std::wstring, std::wstring>::iterator it = _header.begin();
       it != _header.end(); it++)
    (*reinterpret_cast<std::map<std::wstring, std::wstring>*>(header))[it->first] = it->second;
  return header;
}

void SINET_DYN_CALLBACK _set_postdata(struct __request_t* self, _postdata_t* postdata)
{
  refptr<sinet::postdata> pdptr = postdata_cpptoc::Unwrap(postdata);
  request_cpptoc::Get(self)->set_postdata(pdptr);
}

_postdata_t* SINET_DYN_CALLBACK _get_postdata(struct __request_t* self)
{
  return postdata_cpptoc::Wrap(request_cpptoc::Get(self)->get_postdata());
}

void SINET_DYN_CALLBACK _set_response_header(struct __request_t* self, _stringmap_t* header)
{
  request_cpptoc::Get(self)->set_response_header(
    *(reinterpret_cast<std::map<std::wstring, std::wstring>*>(header)));
}

_stringmap_t SINET_DYN_CALLBACK _get_response_header(struct __request_t* self)
{
  _stringmap_t header = _stringmap_alloc();
  std::map<std::wstring, std::wstring> _header = request_cpptoc::Get(self)->get_response_header();
  for (std::map<std::wstring, std::wstring>::iterator it = _header.begin();
    it != _header.end(); it++)
    (*reinterpret_cast<std::map<std::wstring, std::wstring>*>(header))[it->first] = it->second;
  return header;
}

void SINET_DYN_CALLBACK _set_response_buffer(struct __request_t* self, _buffer_t* buffer)
{
  request_cpptoc::Get(self)->set_response_buffer(
    *(reinterpret_cast<std::vector<unsigned char>*>(buffer)));
}

_buffer_t SINET_DYN_CALLBACK _get_response_buffer(struct __request_t* self)
{
  std::vector<unsigned char> _buffer = request_cpptoc::Get(self)->get_response_buffer();
  if (_buffer.empty())
    return NULL;
  return _buffer_alloc(&_buffer[0], _buffer.size());
}

void SINET_DYN_CALLBACK _set_response_size(struct __request_t* self, size_t size_in)
{
  request_cpptoc::Get(self)->set_response_size(size_in);
}

size_t SINET_DYN_CALLBACK _get_response_size(struct __request_t* self)
{
  return request_cpptoc::Get(self)->get_response_size();
}

void SINET_DYN_CALLBACK _set_retrieved_size(struct __request_t* self, size_t size_in)
{
  request_cpptoc::Get(self)->set_retrieved_size(size_in);
}

size_t SINET_DYN_CALLBACK _get_retrieved_size(struct __request_t* self)
{
  return request_cpptoc::Get(self)->get_retrieved_size();
}

void SINET_DYN_CALLBACK _set_response_errcode(struct __request_t* self, int errcode)
{
  request_cpptoc::Get(self)->set_response_errcode(errcode);
}

int SINET_DYN_CALLBACK _get_response_errcode(struct __request_t* self)
{
  return request_cpptoc::Get(self)->get_response_errcode();
}

void SINET_DYN_CALLBACK _set_request_outmode(struct __request_t* self, int outmode)
{
  request_cpptoc::Get(self)->set_request_outmode(outmode);
}

int SINET_DYN_CALLBACK _get_request_outmode(struct __request_t* self)
{
  return request_cpptoc::Get(self)->get_request_outmode();
}

void SINET_DYN_CALLBACK _set_outfile(struct __request_t* self, const wchar_t* file)
{
  request_cpptoc::Get(self)->set_outfile(file);
}

_string_t SINET_DYN_CALLBACK _get_outfile(struct __request_t* self)
{
  return _string_alloc(request_cpptoc::Get(self)->get_outfile().c_str());
}

void SINET_DYN_CALLBACK _close_outfile(struct __request_t* self)
{
  request_cpptoc::Get(self)->close_outfile();
}

void SINET_DYN_CALLBACK _set_appendbuffer(struct __request_t* self, const void* data, size_t size)
{
  request_cpptoc::Get(self)->set_appendbuffer(data, size);
}

request_cpptoc::request_cpptoc(request* cls):
cpptoc<request_cpptoc, request, _request_t>(cls)
{
  struct_.struct_.get_outfile           = _get_outfile;
  struct_.struct_.get_postdata          = _get_postdata;
  struct_.struct_.get_request_header    = _get_request_header;
  struct_.struct_.get_request_method    = _get_request_method;
  struct_.struct_.get_request_outmode   = _get_request_outmode;
  struct_.struct_.get_request_url       = _get_request_url;
  struct_.struct_.get_response_buffer   = _get_response_buffer;
  struct_.struct_.get_response_errcode  = _get_response_errcode;
  struct_.struct_.get_response_header   = _get_response_header;
  struct_.struct_.get_response_size     = _get_response_size;
  struct_.struct_.get_retrieved_size    = _get_retrieved_size;
  struct_.struct_.set_appendbuffer      = _set_appendbuffer;
  struct_.struct_.set_outfile           = _set_outfile;
  struct_.struct_.set_postdata          = _set_postdata;
  struct_.struct_.set_request_header    = _set_request_header;
  struct_.struct_.set_request_method    = _set_request_method;
  struct_.struct_.set_request_outmode   = _set_request_outmode;
  struct_.struct_.set_request_url       = _set_request_url;
  struct_.struct_.set_response_buffer   = _set_response_buffer;
  struct_.struct_.set_response_errcode  = _set_response_errcode;
  struct_.struct_.set_response_header   = _set_response_header;
  struct_.struct_.set_response_size     = _set_response_size;
  struct_.struct_.set_retrieved_size    = _set_retrieved_size;
  struct_.struct_.close_outfile         = _close_outfile;
}