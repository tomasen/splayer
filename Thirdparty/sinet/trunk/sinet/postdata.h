#ifndef POSTDATA_H
#define POSTDATA_H

#include "postdataelem.h"

namespace sinet
{

//////////////////////////////////////////////////////////////////////////
//
//  class postdata to wrap postdata section in an http request
//
class postdata:
  public base
{
public:
  static refptr<postdata> create_instance();

  // clear all elements
  virtual void clear() = 0;
  // add an element, return non-zero if succeeds.
  virtual void add_elem(refptr<postdataelem> elem) = 0;
  // remove an element, return non-zero if succeeds.
  virtual int remove_elem(refptr<postdataelem> elem) = 0;
  // retrieve all elements, returning count
  virtual void get_elements(std::vector<refptr<postdataelem> >& elems) = 0;
  virtual int get_element_count() = 0;
};

} // namespace sinet

#endif // POSTDATA_H