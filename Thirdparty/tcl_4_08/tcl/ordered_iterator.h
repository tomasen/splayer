/*******************************************************************************
Tree Container Library: Generic container library to store data in tree-like structures.
Copyright (c) 2006  Mitchel Haas

This software is provided 'as-is', without any express or implied warranty. 
In no event will the author be held liable for any damages arising from 
the use of this software.

Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1.	The origin of this software must not be misrepresented; 
you must not claim that you wrote the original software. 
If you use this software in a product, an acknowledgment in the product 
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, 
and must not be misrepresented as being the original software.

3.	The above copyright notice and this permission notice may not be removed 
or altered from any source distribution.

For complete documentation on this library, see http://www.datasoftsolutions.net
Email questions, comments or suggestions to mhaas@datasoftsolutions.net
*******************************************************************************/
#pragma once
#include "child_iterator.h"

namespace tcl 
{
	template<typename stored_type, typename node_compare_type, typename node_order_compare_type> class unique_tree;
	template< typename stored_type, typename tree_type,  typename container_type > class associative_tree;
	template<typename tree_type, typename node_order_compare_type> struct deref_ordered_compare;
	template<typename stored_type, typename node_compare_type, typename node_order_compare_type > struct unique_tree_deref_less;

	template<typename T, typename U, typename V> class const_unique_tree_ordered_iterator;
	template<typename T, typename U, typename V> class unique_tree_ordered_iterator;
}


/************************************************************************/
/* ordered iterators (for unique_tree)                                  */
/************************************************************************/


template<typename stored_type, typename node_compare_type, typename node_order_compare_type>
class tcl::const_unique_tree_ordered_iterator : public std::iterator<std::bidirectional_iterator_tag, stored_type>
{
protected:
	// typedefs
	typedef unique_tree<stored_type, node_compare_type, node_order_compare_type> tree_type;
	typedef std::multiset<tree_type*, deref_ordered_compare<tree_type, node_order_compare_type> > ordered_container_type;

public:
	// constructors/destructor
	const_unique_tree_ordered_iterator() {}
	explicit const_unique_tree_ordered_iterator(typename ordered_container_type::const_iterator it_) : it(it_) {}

	// overloaded operators
	friend bool operator != ( const const_unique_tree_ordered_iterator& lhs, const const_unique_tree_ordered_iterator& rhs ) { return lhs.it != rhs.it; }
	friend bool operator == ( const const_unique_tree_ordered_iterator& lhs, const const_unique_tree_ordered_iterator& rhs ) { return lhs.it == rhs.it; }
	const_unique_tree_ordered_iterator& operator ++() { ++it; return *this; }
	const_unique_tree_ordered_iterator operator ++(int) { const_unique_tree_ordered_iterator old(*this); ++*this; return old; }
	const_unique_tree_ordered_iterator& operator --() { --it; return *this; }
	const_unique_tree_ordered_iterator operator --(int) { const_unique_tree_ordered_iterator old(*this); --*this; return old; }
	const stored_type& operator*() const { return  *(*it)->get(); }
	const stored_type* operator->() const { return (*it)->get(); }

	const tree_type* node() const { return *it; }

	// data
protected:
	typename ordered_container_type::const_iterator it;
};

template<typename stored_type, typename node_compare_type, typename node_order_compare_type>
class tcl::unique_tree_ordered_iterator : public tcl::const_unique_tree_ordered_iterator<stored_type, node_compare_type, node_order_compare_type>
{
	// typedefs
	typedef unique_tree<stored_type, node_compare_type, node_order_compare_type> tree_type;
	typedef std::multiset<tree_type*, deref_ordered_compare<tree_type, node_order_compare_type> > ordered_container_type;
	typedef associative_tree<stored_type, unique_tree<stored_type, node_compare_type, node_order_compare_type>,  std::set<unique_tree<stored_type, node_compare_type, node_order_compare_type>*, unique_tree_deref_less<stored_type, node_compare_type, node_order_compare_type> > > associative_tree_type;
	using const_unique_tree_ordered_iterator<stored_type, node_compare_type, node_order_compare_type>::it;

public:
	// constructors/destructor
	unique_tree_ordered_iterator() {}
	explicit unique_tree_ordered_iterator(const typename const_unique_tree_ordered_iterator<stored_type, node_compare_type, node_order_compare_type>::ordered_container_type::const_iterator it_) : const_unique_tree_ordered_iterator<stored_type, node_compare_type, node_order_compare_type>(it_) {}

	// overloaded operators
	stored_type& operator*() { return *(*it)->get(); }
	stored_type* operator->() { return (*it)->get(); }
	unique_tree_ordered_iterator& operator ++() { ++it; return *this; }
	unique_tree_ordered_iterator operator ++(int) { unique_tree_ordered_iterator old(*this); ++*this; return old; }
	unique_tree_ordered_iterator& operator --() { --it; return *this; }
	unique_tree_ordered_iterator operator --(int) { unique_tree_ordered_iterator old(*this); --*this; return old; }

	tree_type* node() { return *it; }
};

