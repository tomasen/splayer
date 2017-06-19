#include "pch.h"
#include "postdataelem_cpptoc.h"

using namespace sinet;

SINET_DYN_API _postdataelem_t* _postdataelem_create_instance()
{
  refptr<postdataelem> impl = postdataelem::create_instance();
  if (impl.get())
    return postdataelem_cpptoc::Wrap(impl);
  return NULL;
}

void SINET_DYN_CALLBACK _set_name(struct __postdataelem_t* self, const wchar_t* fieldname)
{
  postdataelem_cpptoc::Get(self)->set_name(fieldname);
}

_string_t SINET_DYN_CALLBACK _get_name(struct __postdataelem_t* self)
{
  _string_t _name = _string_alloc(
    postdataelem_cpptoc::Get(self)->get_name().c_str());
  return _name;
}

void SINET_DYN_CALLBACK _setto_empty(struct __postdataelem_t* self)
{
  postdataelem_cpptoc::Get(self)->setto_empty();
}

void SINET_DYN_CALLBACK _setto_file(struct __postdataelem_t* self, const wchar_t* filename)
{
  postdataelem_cpptoc::Get(self)->setto_file(filename);
}

void SINET_DYN_CALLBACK _setto_buffer(struct __postdataelem_t* self, const void* bytes_in, const size_t size_in)
{
  postdataelem_cpptoc::Get(self)->setto_buffer(bytes_in, size_in);
}

void SINET_DYN_CALLBACK _setto_text(struct __postdataelem_t* self, const wchar_t* text)
{
  postdataelem_cpptoc::Get(self)->setto_text(text);
}

postdataelem_type_t SINET_DYN_CALLBACK _get_type(struct __postdataelem_t* self)
{
  return postdataelem_cpptoc::Get(self)->get_type();
}

_string_t SINET_DYN_CALLBACK _get_file(struct __postdataelem_t* self)
{
  _string_t _file = _string_alloc(
    postdataelem_cpptoc::Get(self)->get_file().c_str());
  return _file;
}

size_t SINET_DYN_CALLBACK _get_buffer_size(struct __postdataelem_t* self)
{
  return postdataelem_cpptoc::Get(self)->get_buffer_size();
}

size_t SINET_DYN_CALLBACK _copy_buffer_to(struct __postdataelem_t* self, void* bytes_inout, size_t size_in)
{
  return postdataelem_cpptoc::Get(self)->copy_buffer_to(bytes_inout, size_in);
}

_string_t SINET_DYN_CALLBACK _get_text(struct __postdataelem_t* self)
{
  _string_t _text = _string_alloc(
    postdataelem_cpptoc::Get(self)->get_text().c_str());
  return _text;
}

postdataelem_cpptoc::postdataelem_cpptoc(postdataelem* cls) :
  cpptoc<postdataelem_cpptoc, postdataelem, _postdataelem_t>(cls)
{
  struct_.struct_.set_name = _set_name;
  struct_.struct_.get_name = _get_name;
  struct_.struct_.setto_buffer = _setto_buffer;
  struct_.struct_.setto_empty = _setto_empty;
  struct_.struct_.setto_file = _setto_file;
  struct_.struct_.setto_text = _setto_text;
  struct_.struct_.get_buffer_size = _get_buffer_size;
  struct_.struct_.get_file = _get_file;
  struct_.struct_.get_type = _get_type;
  struct_.struct_.get_text = _get_text;
  struct_.struct_.copy_buffer_to = _copy_buffer_to;
}