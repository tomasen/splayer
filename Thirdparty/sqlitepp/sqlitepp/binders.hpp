//////////////////////////////////////////////////////////////////////////////
// $Id: binders.hpp 46 2006-09-05 06:38:56Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SQLITEPP_BINDERS_HPP_INCLUDED
#define SQLITEPP_BINDERS_HPP_INCLUDED

#include <memory>

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

class statement;

/// into binder interface
class into_binder
{
public:
	into_binder() {}
	virtual ~into_binder() {}
	// Bind value to statement st in positin pos.
	int bind(statement& st, int pos);
	// Update bound value.
	void update(statement& st);
private:
	// Noncopyable.
	into_binder(into_binder const&);
	// Nonassignable.
	into_binder& operator=(into_binder const&);

	virtual void do_bind(statement& st, int pos) = 0;
	virtual void do_update(statement& st) = 0;
};

typedef std::auto_ptr<into_binder> into_binder_ptr;

/// use binder interface
class use_binder
{
public:
	use_binder() {}
	virtual ~use_binder() {}
	/// Bind value to statement st in position pos
	int bind(statement& st, int pos);
private:
	// Noncopyable.
	use_binder(use_binder const&);
	// Nonassignable.
	use_binder& operator=(use_binder const&);

	virtual void do_bind(statement& st, int pos) = 0;
};

typedef std::auto_ptr<use_binder> use_binder_ptr;

//////////////////////////////////////////////////////////////////////////////

} //namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////

#endif // SQLITEPP_BINDERS_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
