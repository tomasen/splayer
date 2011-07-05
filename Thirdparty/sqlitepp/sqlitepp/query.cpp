//////////////////////////////////////////////////////////////////////////////
// $Id: query.cpp 72 2008-12-09 07:54:08Z pmed $
//
// Copyright (c) 2004 Pavel Medvedev
// Use, modification and distribution is subject to the 
// Boost Software License, Version 1.0. (See accompanying file 
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <stdexcept>

#include "../sqlite/sqlite3.h"

#include "query.hpp"
#include "binders.hpp"
#include "exception.hpp"
#include "statement.hpp"
#include "session.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

namespace { // implementation details

//////////////////////////////////////////////////////////////////////////////

template<typename T>
inline void delete_object(T* obj)
{
	delete obj;
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////

} // namespace { // implementation details

//////////////////////////////////////////////////////////////////////////////
// 
// query 
//

query::query()
{
}
//----------------------------------------------------------------------------

query::query(string_t const& sql)
{
	sql_ << sql;
}
//----------------------------------------------------------------------------

query::~query()
{
	clear();
}
//----------------------------------------------------------------------------

void query::sql(string_t const& text)
{
	sql_.str(text);
	sql_.seekp(0, std::ios_base::end).clear();
}
//----------------------------------------------------------------------------

void query::clear() // throw()
{
	// clear binders
	std::for_each(intos_.begin(), intos_.end(), delete_object<into_binder>);
	intos_.clear();
	std::for_each(uses_.begin(), uses_.end(), delete_object<use_binder>);
	uses_.clear();
	// clear sql
	sql(string_t());
}
//----------------------------------------------------------------------------

bool query::empty() const // throw()
{
	return sql_.str().empty() && intos_.empty() && uses_.empty();
}
//----------------------------------------------------------------------------

query& query::put(into_binder_ptr i)
{
	if ( !i.get() )
	{
		throw std::invalid_argument("null into binder");
	}
	intos_.push_back(i.release());
	return *this;
}
//----------------------------------------------------------------------------

query& query::put(use_binder_ptr u)
{
	if ( !u.get() )
	{
		throw std::invalid_argument("null use binder");
	}
	uses_.push_back(u.release());
	return *this;
}
//----------------------------------------------------------------------------

void swap(query& q1, query& q2)
{
	// swap binders
	swap(q1.intos_, q2.intos_);
	swap(q1.uses_, q2.uses_);
	// swap sql streams
	string_t tmp = q1.sql();
	q1.sql(q2.sql());
	q2.sql(tmp);
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// 
// prepare_query 
//

prepare_query::prepare_query(statement& st)
	: st_(&st)
{
}
//----------------------------------------------------------------------------

prepare_query::prepare_query(prepare_query& src)
{
	swap(*this, src);
	st_ = src.st_; src.st_ = 0;
}
//----------------------------------------------------------------------------

prepare_query::~prepare_query()
{
	if ( st_ )
	{
		// move query to statement.
		swap(st_->q(), *this); 
		st_->finalize();
	}
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// 
// once_query
//

once_query::once_query(session& s)
	: s_(&s)
{
}
//----------------------------------------------------------------------------

once_query::once_query(once_query& src)
{
	swap(*this, src);
	s_ = src.s_; src.s_ = 0;
}
//----------------------------------------------------------------------------

once_query::~once_query()
{
	if ( s_ )
	{
		if ( !s_->is_open() )
		{
			throw session_not_open();
		}
		// execute statement in session.
		statement st(*s_);
		swap(st.q(), *this);
		st.exec();
	}
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////

} // namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////
