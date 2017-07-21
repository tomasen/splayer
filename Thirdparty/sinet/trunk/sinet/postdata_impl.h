#ifndef POSTDATA_IMPL_H
#define POSTDATA_IMPL_H

#include "postdata.h"

namespace sinet
{

class postdata_impl:
  public threadsafe_base<postdata>
{
public:
  virtual void clear();
  virtual void add_elem(refptr<postdataelem> elem);
  virtual int remove_elem(refptr<postdataelem> elem);
  virtual void get_elements(std::vector<refptr<postdataelem> >& elems);
  virtual int get_element_count();

private:
  std::vector<refptr<postdataelem> > m_elems;
};

}

#endif // POSTDATA_IMPL_H