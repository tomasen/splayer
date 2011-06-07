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
#include <set>
#include <stack>
#include <queue>
#include <algorithm>

namespace tcl 
{
	template<typename T, typename U, typename V, typename W> class const_pre_order_descendant_node_iterator;
	template<typename T, typename U, typename V, typename W> class pre_order_descendant_node_iterator;
	template<typename T, typename U, typename V, typename W> class const_post_order_descendant_node_iterator;
	template<typename T, typename U, typename V, typename W> class post_order_descendant_node_iterator;
	template<typename T, typename U, typename V, typename W> class const_level_order_descendant_node_iterator;
	template<typename T, typename U, typename V, typename W> class level_order_descendant_node_iterator;
}

/************************************************************************/
/* descendant iterators                                                 */
/************************************************************************/

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
class tcl::const_pre_order_descendant_node_iterator : public std::iterator<std::bidirectional_iterator_tag, tree_type>
{
public:
	// constructors/destructor
	const_pre_order_descendant_node_iterator() : pTop_node(0) {}
	// destructor, copy constructor, and assignment operator will be compiler generated correctly
protected:
	explicit const_pre_order_descendant_node_iterator(typename tree_category_type::const_node_iterator& it_, const tree_category_type* pTop_node_) : it(it_), pTop_node(pTop_node_) {}

public:
	// overloaded operators
	bool operator != ( const const_pre_order_descendant_node_iterator& rhs ) const { return it != rhs.it; }
	bool operator == ( const const_pre_order_descendant_node_iterator& rhs ) const { return it == rhs.it; }
	const_pre_order_descendant_node_iterator& operator ++(); 
	const_pre_order_descendant_node_iterator operator ++(int) { const_pre_order_descendant_node_iterator old(*this); ++*this; return old; }
	const_pre_order_descendant_node_iterator& operator --();
	const_pre_order_descendant_node_iterator operator --(int) { const_pre_order_descendant_node_iterator old(*this); --*this; return old; }

	// public interface
	const tree_type& operator*() const { return  it.operator *(); }
	const tree_type* operator->() const { return it.operator ->(); }
	friend class associative_tree<stored_type, tree_type, container_type>;
	friend class sequential_tree<stored_type>;

	// data
protected:
	typename tree_category_type::const_node_iterator it;
	std::stack<typename tree_category_type::const_node_iterator> node_stack;   
	const tree_category_type* pTop_node;
	typename container_type::const_reverse_iterator rit;
};

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
class tcl::pre_order_descendant_node_iterator : public tcl::const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>
{
public:
	using const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::it;
	// constructors/destructor
	pre_order_descendant_node_iterator() : const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>() {}
	// destructor, copy constructor, and assignment operator will be compiler generated correctly
protected:
	explicit pre_order_descendant_node_iterator(typename tree_category_type::node_iterator& it_, const tree_category_type* const pTop_node_) : const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>(it_, pTop_node_) {}

public:
	// overloaded operators
	pre_order_descendant_node_iterator& operator ++() { ++(*static_cast<const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>*>(this)); return *this; }
	pre_order_descendant_node_iterator operator ++(int) { pre_order_descendant_node_iterator old(*this); ++*this; return old; }
	pre_order_descendant_node_iterator& operator --() { --(*static_cast<const_pre_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>*>(this)); return *this; }
	pre_order_descendant_node_iterator operator --(int) { pre_order_descendant_node_iterator old(*this); --*this; return old; }

	// public interface
	tree_type& operator*() { return  const_cast<tree_type&>(it.operator *()); }
	tree_type* operator->() const { return const_cast<tree_type*>(it.operator ->()); }
	friend class associative_tree<stored_type, tree_type, container_type>;
	friend class sequential_tree<stored_type>;
};


template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
class tcl::const_post_order_descendant_node_iterator : public std::iterator<std::bidirectional_iterator_tag, tree_type>
{
public:
	// constructors/destructor
	const_post_order_descendant_node_iterator() : pTop_node(0) {}
	// destructor, copy constructor, and assignment operator will be compiler generated correctly
protected:
	explicit const_post_order_descendant_node_iterator(const tree_category_type* pTop_node_); 
	explicit const_post_order_descendant_node_iterator(typename tree_category_type::const_node_iterator& it_, const tree_category_type* pTop_node_) : it(it_), pTop_node(pTop_node_) {}

public:
	// overloaded operators
	bool operator != ( const const_post_order_descendant_node_iterator& rhs ) const { return it != rhs.it; }
	bool operator == ( const const_post_order_descendant_node_iterator& rhs ) const { return it == rhs.it; }
	const_post_order_descendant_node_iterator& operator ++(); 
	const_post_order_descendant_node_iterator operator ++(int) { const_post_order_descendant_node_iterator old(*this); ++*this; return old; }
	const_post_order_descendant_node_iterator& operator --(); 
	const_post_order_descendant_node_iterator operator --(int) { const_post_order_descendant_node_iterator old(*this); --*this; return old; }

	// public interface
	const tree_type& operator*() const { return  it.operator *(); }
	const tree_type* operator->() const { return it.operator ->(); }
	friend class associative_tree<stored_type, tree_type, container_type>;
	friend class sequential_tree<stored_type>;

	// data
protected:
	std::stack<typename tree_category_type::const_node_iterator > node_stack;   
	typename tree_category_type::const_node_iterator it;
	const tree_category_type* pTop_node;
	typename container_type::const_reverse_iterator rit;
};

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
class tcl::post_order_descendant_node_iterator : public tcl::const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>
{
public:
	using const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::it;
	// constructors/destructor
	post_order_descendant_node_iterator() : const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>() {}
	// destructor, copy constructor, and assignment operator will be compiler generated correctly
protected:
	explicit post_order_descendant_node_iterator(const tree_category_type* pTop_node_) : const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>(pTop_node_) { }
	explicit post_order_descendant_node_iterator(typename tree_category_type::node_iterator& it_, const tree_category_type* pTop_node_) : const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>(it_, pTop_node_) {}

public:
	// overloaded operators
	post_order_descendant_node_iterator& operator ++() { ++(*static_cast<const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>*>(this)); return *this; }
	post_order_descendant_node_iterator operator ++(int) { post_order_descendant_node_iterator old(*this); ++*this; return old; }
	post_order_descendant_node_iterator& operator --() { --(*static_cast<const_post_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>*>(this)); return *this; }
	post_order_descendant_node_iterator operator --(int) { post_order_descendant_node_iterator old(*this); --*this; return old; }

	// public interface
	tree_type& operator*() { return  const_cast<tree_type&>(it.operator *()); }
	tree_type* operator->() { return const_cast<tree_type*>(it.operator ->()); }
	friend class associative_tree<stored_type, tree_type, container_type>;
	friend class sequential_tree<stored_type>;
};


template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
class tcl::const_level_order_descendant_node_iterator : public std::iterator<std::forward_iterator_tag, tree_type>
{
public:
	// constructors/destructor
	const_level_order_descendant_node_iterator() : pTop_node(0) {}
	// destructor, copy constructor, and assignment operator will be compiler generated correctly
protected:
	explicit const_level_order_descendant_node_iterator(typename tree_category_type::const_node_iterator& it_, const tree_category_type* pTop_node_) : it(it_), pTop_node(pTop_node_) {}

public:
	// overloaded operators
	bool operator != (const const_level_order_descendant_node_iterator& rhs) const { return it != rhs.it; }
	bool operator == (const const_level_order_descendant_node_iterator& rhs) const { return it == rhs.it; }
	const_level_order_descendant_node_iterator& operator ++();
	const_level_order_descendant_node_iterator operator ++(int) { const_level_order_descendant_node_iterator old(*this); ++*this; return old; }

	// public interface
	const tree_type& operator*() const { return  it.operator *(); }
	const tree_type* operator->() const { return it.operator ->(); }
	friend class associative_tree<stored_type, tree_type, container_type>;
	friend class sequential_tree<stored_type>;

	// data
protected:
	typename tree_category_type::const_node_iterator it;
	std::queue<typename tree_category_type::const_node_iterator> node_queue;
	const tree_category_type* pTop_node;
};

template<typename stored_type, typename tree_type, typename container_type, typename tree_category_type>
class tcl::level_order_descendant_node_iterator : public tcl::const_level_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>
{
public:
	using const_level_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>::it;
	// constructors/destructor
	level_order_descendant_node_iterator() : const_level_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>() {}
	// destructor, copy constructor, and assignment operator will be compiler generated correctly
protected:
	explicit level_order_descendant_node_iterator(typename tree_category_type::node_iterator& it_, const tree_category_type* pTop_node_) : const_level_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>(it_, pTop_node_) {}

public:
	// overloaded operators
	level_order_descendant_node_iterator& operator ++() { ++(*static_cast<const_level_order_descendant_node_iterator<stored_type, tree_type, container_type, tree_category_type>*>(this)); return *this; }
	level_order_descendant_node_iterator operator ++(int) { level_order_descendant_node_iterator old(*this); ++*this; return old; }

	// public interface
	tree_type& operator*() { return  const_cast<tree_type&>(it.operator *()); }
	tree_type* operator->() { return const_cast<tree_type*>(it.operator ->()); }
	friend class associative_tree<stored_type, tree_type, container_type>;
	friend class sequential_tree<stored_type>;
};


#include "descendant_node_iterator.inl"
