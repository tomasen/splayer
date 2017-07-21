#include "pch.h"
#include "postdataelem_ctocpp.h"

using namespace sinet;

refptr<postdataelem> postdataelem::create_instance()
{
  _postdataelem_t* impl = _postdataelem_create_instance();
  if (impl)
    return postdataelem_ctocpp::Wrap(impl);
  return NULL;
}

void postdataelem_ctocpp::set_name(const wchar_t* fieldname)
{
  if (_MEMBER_MISSING(struct_, set_name))
    return;
  struct_->set_name(struct_, fieldname);
}

std::wstring postdataelem_ctocpp::get_name()
{
  if (_MEMBER_MISSING(struct_, get_name))
    return L"";
  _string_t _name = struct_->get_name(struct_);
  std::wstring name = _name;
  _string_free(_name);
  return name;
}

void postdataelem_ctocpp::setto_empty()
{
  if (_MEMBER_MISSING(struct_, setto_empty))
    return;
  struct_->setto_empty(struct_);
}

void postdataelem_ctocpp::setto_file(const wchar_t* filename)
{
  if (_MEMBER_MISSING(struct_, setto_file))
    return;
  struct_->setto_file(struct_, filename);
}

void postdataelem_ctocpp::setto_buffer(const void* bytes_in, const size_t size_in)
{
  if (_MEMBER_MISSING(struct_, setto_buffer))
    return;
  struct_->setto_buffer(struct_, bytes_in, size_in);
}

void postdataelem_ctocpp::setto_text(const wchar_t* text)
{
  if (_MEMBER_MISSING(struct_, setto_text))
    return;
  struct_->setto_text(struct_, text);
}

sinet::postdataelem_type_t postdataelem_ctocpp::get_type()
{
  if (_MEMBER_MISSING(struct_, get_type))
    return PDE_TYPE_EMPTY;
  return struct_->get_type(struct_);
}

std::wstring postdataelem_ctocpp::get_file()
{
  if (_MEMBER_MISSING(struct_, get_file))
    return L"";
  _string_t _file = struct_->get_file(struct_);
  std::wstring file = _file;
  _string_free(_file);
  return file;
}

size_t postdataelem_ctocpp::get_buffer_size()
{
  if (_MEMBER_MISSING(struct_, get_buffer_size))
    return 0;
  return struct_->get_buffer_size(struct_);
}

size_t postdataelem_ctocpp::copy_buffer_to(void* bytes_inout, size_t size_in)
{
  if (_MEMBER_MISSING(struct_, copy_buffer_to))
    return 0;
  return struct_->copy_buffer_to(struct_, bytes_inout, size_in);
}

std::wstring postdataelem_ctocpp::get_text()
{
  if (_MEMBER_MISSING(struct_, get_text))
    return L"";
  _string_t _text = struct_->get_text(struct_);
  std::wstring text = _text;
  _string_free(_text);
  return text;
}