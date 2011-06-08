//////////////////////////////////////////////////////////////////////////////
// $Id: query.hpp 48 2006-12-20 07:35:49Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SQLITEPP_PROXIES_HPP_INCLUDED
#define SQLITEPP_PROXIES_HPP_INCLUDED

#include <vector>
#include <sstream>
#include <memory>

#include "string.hpp"
#include "binders.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

class session;
class statement;

// SQL query base class. Noncopyable.
class query
{
public:
	// Create an empty query.
	query();
	
	// Create a query with SQL text.
	explicit query(string_t const& sql);
	

	// Clear query on destroy.
	~query();

	// Current SQL statement.
	string_t sql() const // throw()
	{
		return sql_.str();
	}
	
	// Set new SQL statement.
	void sql(string_t const& text);

	// Clear sql text, into and use bindings.
	void clear(); // throw()
	
	// Is query empty?
	bool empty() const; // throw()

	// Into binders container type.
	typedef std::vector<into_binder*> into_binders;
	
	// Into binders.
	into_binders const& intos() const // throw()
	{
		return intos_;
	}

	// Use binders container type.
	typedef std::vector<use_binder*> use_binders;

	// Use binders.
	use_binders const& uses() const // throw()
	{
		return uses_;
	}

	// Collect SQL text.
	template<typename T>
	query& operator<<(T const& t)
	{
		sql_ << t;
		return *this;
	}

	// Add into binder.
	query& put(into_binder_ptr i);

	// Add into binder.
	query& operator,(into_binder_ptr i)
	{
		return put(i);
	}
	
	// Add use binder.
	query& put(use_binder_ptr i);
	
	// Add use binder.
	query& operator,(use_binder_ptr u)
	{
		return put(u);
	}

	// Swap queries.
	friend void swap(query& lhs, query& rhs);
protected:	
	// Noncopyable.
	query(query const& src);
	// Nonassignable.
	query& operator=(query const& src);
private:
	into_binders intos_;
	use_binders  uses_;

	std::basic_ostringstream<char_t> sql_;
};

// Statement preparing proxy.
class prepare_query : public query
{
	friend class statement; // access to ctor
public:
	// Transfer execution responsibiblty from src to this object.
	prepare_query(prepare_query& src);

	// Move query to statement on destroy.
	~prepare_query();
private:
	// Create preparing proxy for statement.
	prepare_query(statement& st);
	
	// Assignment not allowed.
	prepare_query& operator=(prepare_query const&);

	statement* st_;
};

// Immediatly executed query proxy.
class once_query : public query
{
	friend class session; // access to ctor
public:
	// Transfer execution responsibiblty from src to this object.
	once_query(once_query& src);

	// Execute statement on destroy.
	~once_query();
private:
	// Create proxy for session.
	once_query(session& s);

	// Assignment not allowed.
	once_query& operator=(once_query const&); 

	session* s_;
};

//////////////////////////////////////////////////////////////////////////////

} //namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////

#endif // SQLITEPP_PROXIES_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
