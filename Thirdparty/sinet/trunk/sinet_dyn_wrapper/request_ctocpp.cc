#include "pch.h"
#include "request_ctocpp.h"
#include "postdata_ctocpp.h"

using namespace sinet;

refptr<request> request::create_instance()
{
  _request_t* impl = _request_create_instance();
  if (impl)
    return request_ctocpp::Wrap(impl);
  return NULL;
}

void request_ctocpp::set_request_method(const wchar_t* method)
{
  if (_MEMBER_MISSING(struct_, set_request_method))
    return;
  struct_->set_request_method(struct_, method);
}

std::wstring request_ctocpp::get_request_method()
{
  if (_MEMBER_MISSING(struct_, get_request_method))
    return L"";
  std::wstring method;
  _string_t _methond = struct_->get_request_method(struct_);
  if (_methond)
  {
    method = _methond;
    _string_free(_methond);
    return method;
  }
  return L"";
}

void request_ctocpp::set_request_url(const wchar_t* url)
{
  if (_MEMBER_MISSING(struct_, set_request_url))
    return;
  struct_->set_request_url(struct_, url);
}

std::wstring request_ctocpp::get_request_url()
{
  if (_MEMBER_MISSING(struct_, get_request_url))
    return L"";
  std::wstring name;
  _string_t _name = struct_->get_request_url(struct_);
  if (_name)
  {
    name = _name;
    _string_free(_name);
    return name;
  }
  return L"";
}

void request_ctocpp::set_request_header(si_stringmap& header)
{
  if (_MEMBER_MISSING(struct_, set_request_header))
    return;
  struct_->set_request_header(struct_, reinterpret_cast<_stringmap_t*>(&header));
}

sinet::si_stringmap request_ctocpp::get_request_header()
{
  if (_MEMBER_MISSING(struct_, get_request_header))
    return *(new sinet::si_stringmap);
  _stringmap_t _header = struct_->get_request_header(struct_);
  if (_header)
  {
    sinet::si_stringmap header;
    sinet::si_stringmap* _header_stl = reinterpret_cast<si_stringmap*>(_header);
    for (si_stringmap::iterator it = _header_stl->begin();
         it != _header_stl->end(); it++)
      header[it->first] = it->second;
    _stringmap_free(_header);
    return header;
  }
  return *(new sinet::si_stringmap);
}

void request_ctocpp::set_postdata(refptr<postdata> postdata)
{
  if (_MEMBER_MISSING(struct_, set_postdata))
    return;
  struct_->set_postdata(struct_, postdata_ctocpp::Unwrap(postdata));
}

refptr<postdata> request_ctocpp::get_postdata()
{
  if (_MEMBER_MISSING(struct_, get_postdata))
    return NULL;
  return postdata_ctocpp::Wrap(struct_->get_postdata(struct_));
}

void request_ctocpp::set_response_header(si_stringmap& header)
{
  if (_MEMBER_MISSING(struct_, set_response_header))
    return;
  struct_->set_response_header(struct_, reinterpret_cast<_stringmap_t*>(&header));
}

sinet::si_stringmap request_ctocpp::get_response_header()
{
  if (_MEMBER_MISSING(struct_, get_response_header))
    return *(new sinet::si_stringmap);
  _stringmap_t _header = struct_->get_response_header(struct_);
  if (_header)
  {
    sinet::si_stringmap header;
    sinet::si_stringmap* _header_stl = reinterpret_cast<si_stringmap*>(_header);
    for (si_stringmap::iterator it = _header_stl->begin();
         it != _header_stl->end(); it++)
      header[it->first] = it->second;
    _stringmap_free(_header);
    return header;
  }
  return *(new sinet::si_stringmap);
}

void request_ctocpp::set_response_buffer(si_buffer& buffer)
{
  if (_MEMBER_MISSING(struct_, set_response_buffer))
    return;
  struct_->set_response_buffer(struct_, reinterpret_cast<_buffer_t*>(&buffer));
}

sinet::si_buffer request_ctocpp::get_response_buffer()
{
  if (_MEMBER_MISSING(struct_, get_response_buffer))
    return *(new sinet::si_buffer);
  si_buffer buffer;
  _buffer_t _buffer = struct_->get_response_buffer(struct_);
  if (_buffer)
  {
    buffer.resize(_buffer_size(_buffer));
    memcpy(&buffer[0], _buffer_get(_buffer), _buffer_size(_buffer));
    _buffer_free(_buffer);
    return buffer;
  }
  return *(new sinet::si_buffer);
}

void request_ctocpp::set_response_size(size_t size_in)
{
  if (_MEMBER_MISSING(struct_, set_response_size))
    return;
  struct_->set_response_size(struct_, size_in);
}

size_t request_ctocpp::get_response_size()
{
  if (_MEMBER_MISSING(struct_, get_response_size))
    return 0;
  return struct_->get_response_size(struct_);
}

void request_ctocpp::set_retrieved_size(size_t size_in)
{
  if (_MEMBER_MISSING(struct_, set_retrieved_size))
    return;
  struct_->set_retrieved_size(struct_, size_in);
}

size_t request_ctocpp::get_retrieved_size()
{
  if (_MEMBER_MISSING(struct_, get_retrieved_size))
    return 0;
  return struct_->get_retrieved_size(struct_);
}

void request_ctocpp::set_response_errcode(int errcode)
{
  if (_MEMBER_MISSING(struct_, set_response_errcode))
    return;
  struct_->set_response_errcode(struct_, errcode);
}

int request_ctocpp::get_response_errcode()
{
  if (_MEMBER_MISSING(struct_, get_response_errcode))
    return 0;
  return struct_->get_response_errcode(struct_);
}

void request_ctocpp::set_request_outmode(int outmode)
{
  if (_MEMBER_MISSING(struct_, set_request_outmode))
    return;
  struct_->set_request_outmode(struct_, outmode);
}

int request_ctocpp::get_request_outmode()
{
  if (_MEMBER_MISSING(struct_, get_request_outmode))
    return 0;
  return struct_->get_request_outmode(struct_);
}

void request_ctocpp::set_outfile(const wchar_t* file)
{
  if (_MEMBER_MISSING(struct_, set_outfile))
    return;
  struct_->set_outfile(struct_, file);
}

std::wstring request_ctocpp::get_outfile()
{
  if (_MEMBER_MISSING(struct_, get_outfile))
    return L"";
  std::wstring outfile;
  _string_t _outfile = struct_->get_outfile(struct_);
  if (_outfile)
  {
    outfile = _outfile;
    _string_free(_outfile);
    return outfile;
  }
  return L"";
}

void request_ctocpp::close_outfile()
{
  if (_MEMBER_MISSING(struct_, close_outfile))
    return;
  struct_->close_outfile(struct_);
}

void request_ctocpp::set_appendbuffer(const void* data, size_t size)
{
  if (_MEMBER_MISSING(struct_, set_appendbuffer))
    return;
  struct_->set_appendbuffer(struct_, data, size);
}