#ifndef POSTDATA_CTOCPP_H
#define POSTDATA_CTOCPP_H

#include "ctocpp.h"
#include "../sinet_dyn/sinet_capi.h"
#include "../sinet/postdata.h"

class postdata_ctocpp:
  public ctocpp<postdata_ctocpp, postdata, _postdata_t>
{
public:
  postdata_ctocpp(_postdata_t* pd)
    : ctocpp<postdata_ctocpp, postdata, _postdata_t>(pd) {}
  virtual ~postdata_ctocpp() {}

  virtual void clear();
  virtual void add_elem(refptr<postdataelem> elem);
  virtual int remove_elem(refptr<postdataelem> elem);
  virtual void get_elements(std::vector<refptr<postdataelem> >& elems);
  virtual int get_element_count();
};

#endif // POSTDATA_CTOCPP_H