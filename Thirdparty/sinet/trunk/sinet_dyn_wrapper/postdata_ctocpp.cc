#include "pch.h"
#include "postdata_ctocpp.h"
#include "postdataelem_ctocpp.h"

using namespace sinet;

refptr<postdata> postdata::create_instance()
{
  _postdata_t* impl = _postdata_create_instance();
  if (impl)
    return postdata_ctocpp::Wrap(impl);
  return NULL;
}

void postdata_ctocpp::clear()
{
  if (_MEMBER_MISSING(struct_, clear))
    return;
  struct_->clear(struct_);
}

void postdata_ctocpp::add_elem(refptr<postdataelem> elem)
{
  if (_MEMBER_MISSING(struct_, add_elem))
    return;
  struct_->add_elem(struct_, postdataelem_ctocpp::Unwrap(elem));
}

int postdata_ctocpp::remove_elem(refptr<postdataelem> elem)
{
  if (_MEMBER_MISSING(struct_, remove_elem))
    return 0;
  return struct_->remove_elem(struct_, postdataelem_ctocpp::Unwrap(elem));
}

//use a special wrapped function of dll in this function
//chrome is referred to here
void postdata_ctocpp::get_elements(std::vector<refptr<postdataelem> >& elems)
{
  elems.clear();
  if (_MEMBER_MISSING(struct_, get_elements))
    return;
  int count = (int)get_element_count();
  _postdataelem_t* pdeptr;
  for (int index = 0; index < count; index++)
  {
    //get elements one by one by this special function
    pdeptr = struct_->get_elements(struct_, index);
    if (pdeptr)
      elems.push_back(postdataelem_ctocpp::Wrap(pdeptr));
  }
}

int postdata_ctocpp::get_element_count()
{
  if (_MEMBER_MISSING(struct_, get_element_count))
    return 0;
  return struct_->get_element_count(struct_);
}