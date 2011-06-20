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
#include "associative_tree.h"
#include <set>

namespace tcl
{
// forward declaration for deref comparison functor
template<typename stored_type, typename node_compare_type > class tree;

	// deref comparison functor, derive from binary function per Scott Meyer
	template<typename stored_type, typename node_compare_type >
	struct tree_deref_less : public std::binary_function<const tree<stored_type, node_compare_type>*, const tree<stored_type, node_compare_type>*, bool>
	{
		bool operator () (const tree<stored_type, node_compare_type>* lhs, const tree<stored_type, node_compare_type>* rhs) const 
		{
			// call < on actual object
			return node_compare_type()(*lhs->get(), *rhs->get());
		}
	};
}




// node object type.  forwards most operations to base_tree_type, 
// instanciates base_tree_type with type of container (set of unique_tree ptrs) to use for node and key comparisons
template<typename stored_type, typename node_compare_type = std::less<stored_type> >
class tcl::tree : public tcl::associative_tree<stored_type, tcl::tree<stored_type, node_compare_type>,  std::set<tcl::tree<stored_type, node_compare_type>*, tcl::tree_deref_less<stored_type, node_compare_type> > >
{
public:
	// typedefs
	typedef tree<stored_type, node_compare_type> tree_type;
	typedef tree_deref_less<stored_type, node_compare_type> key_compare;
	typedef tree_deref_less<stored_type, node_compare_type> value_compare;
	typedef basic_tree<stored_type, tree<stored_type, node_compare_type>,  std::set<tree<stored_type, node_compare_type>*, tree_deref_less<stored_type, node_compare_type> > > basic_tree_type;
	typedef associative_tree<stored_type, tree<stored_type, node_compare_type>,  std::set<tree<stored_type, node_compare_type>*, tree_deref_less<stored_type, node_compare_type> > > associative_tree_type;
	typedef std::set<tree<stored_type, node_compare_type>*, tree_deref_less<stored_type, node_compare_type> > container_type;
	friend class basic_tree<stored_type, tree<stored_type, node_compare_type>,  std::set<tree<stored_type, node_compare_type>*, tree_deref_less<stored_type, node_compare_type> > >;

	// constructors/destructor
	explicit tree( const stored_type& value = stored_type() ) : associative_tree_type(value) {}
	template<typename iterator_type> tree(iterator_type it_beg, iterator_type it_end, const stored_type& value = stored_type()) : associative_tree_type(value) { while (it_beg != it_end) { insert(*it_beg); ++it_beg; } }
	tree( const tree_type& rhs ); // copy constructor
	~tree() { associative_tree_type::clear(); }

	// assignment operator
	tree_type& operator = (const tree_type& rhs);

	// public interface
public:
	typename associative_tree_type::iterator insert(const stored_type& value) { return associative_tree_type::insert(value, this); }
	typename associative_tree_type::iterator insert(const typename associative_tree_type::const_iterator pos, const stored_type& value) { return associative_tree_type::insert(pos, value, this); }
	typename associative_tree_type::iterator insert(const tree_type& tree_obj ) { return associative_tree_type::insert(tree_obj, this); }
	typename associative_tree_type::iterator insert(const typename associative_tree_type::const_iterator pos, const tree_type& tree_obj) { return associative_tree_type::insert(pos, tree_obj, this); }
	#if !defined(_MSC_VER) || _MSC_VER >= 1300 // insert range not available for VC6
	template<typename iterator_type> void insert(iterator_type it_beg, iterator_type it_end) { while ( it_beg != it_end ) insert(*it_beg++); }
	#endif
	void swap(tree_type& rhs);
};

#include "tree.inl"
