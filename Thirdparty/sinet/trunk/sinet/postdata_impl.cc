#include "pch.h"
#include "postdata_impl.h"

using namespace sinet;

refptr<postdata> postdata::create_instance()
{
  refptr<postdata> _postdata(new postdata_impl());
  return _postdata;
}

void postdata_impl::clear()
{
  m_elems.clear();
}

void postdata_impl::add_elem(refptr<postdataelem> elem)
{
  m_elems.push_back(elem);
}

int postdata_impl::remove_elem(refptr<postdataelem> elem)
{
  for (std::vector<refptr<postdataelem> >::iterator it = m_elems.begin();
       it != m_elems.end(); it++)
  {
    if (*it == elem)
    {
      m_elems.erase(it);
      return 1;
    }
  }
  return 0;
}

void postdata_impl::get_elements(std::vector<refptr<postdataelem> >& elems)
{
  elems = m_elems;
}

int postdata_impl::get_element_count()
{
  return m_elems.size();
}
