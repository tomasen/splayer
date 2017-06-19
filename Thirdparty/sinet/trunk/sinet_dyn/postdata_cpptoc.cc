#include "pch.h"
#include "postdata_cpptoc.h"
#include "postdataelem_cpptoc.h"

using namespace sinet;

SINET_DYN_API _postdata_t* _postdata_create_instance()
{
  refptr<postdata> impl = postdata::create_instance();
  if (impl.get())
    return postdata_cpptoc::Wrap(impl);
  return NULL;
}

void SINET_DYN_CALLBACK _clear(struct __postdata_t* self)
{
  postdata_cpptoc::Get(self)->clear();
}

void SINET_DYN_CALLBACK _add_elem(struct __postdata_t* self, _postdataelem_t* elem)
{
  refptr<postdataelem> selfelemptr = postdataelem_cpptoc::Unwrap(elem);
  postdata_cpptoc::Get(self)->add_elem(selfelemptr);
}

int SINET_DYN_CALLBACK _remove_elem(struct __postdata_t* self, _postdataelem_t* elem)
{
  refptr<postdataelem> selfelemptr = postdataelem_cpptoc::Unwrap(elem);
  return postdata_cpptoc::Get(self)->remove_elem(selfelemptr);
}

//this is a special way to wrap the get_elements function
//chrome is referred to here
_postdataelem_t* SINET_DYN_CALLBACK _get_elements(struct __postdata_t* self, int elemindex)
{
  std::vector<refptr<postdataelem>> _elems;
  postdata_cpptoc::Get(self)->get_elements(_elems);
  if (elemindex < 0 || elemindex >= (int)_elems.size())
    return NULL;

  return postdataelem_cpptoc::Wrap(_elems[elemindex]);
}

int SINET_DYN_CALLBACK _get_element_count(struct __postdata_t* self)
{
  return postdata_cpptoc::Get(self)->get_element_count();
}


postdata_cpptoc::postdata_cpptoc(postdata* cls):
cpptoc<postdata_cpptoc, postdata, _postdata_t>(cls)
{
  struct_.struct_.clear = _clear;
  struct_.struct_.add_elem = _add_elem;
  struct_.struct_.remove_elem = _remove_elem;
  struct_.struct_.get_elements = _get_elements;
  struct_.struct_.get_element_count = _get_element_count;
}