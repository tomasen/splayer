//////////////////////////////////////////////////////////////////////////////
// $Id: use.hpp 76 2010-08-02 11:08:26Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SQLITEPP_USE_HPP_INCLUDED
#define SQLITEPP_USE_HPP_INCLUDED

#include "binders.hpp"
#include "converters.hpp"
#include "string.hpp"
#include "statement.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

/// Positional use binder.
template<typename T>
class use_pos_binder : public use_binder
{
public:
	use_pos_binder(T& value)
		: value_(value)
	{
	}
protected:
	T& value_;
private:
	void do_bind(statement& st, int pos)
	{
		st.use_value(pos, converter<T>::from(this->value_));
	}
};

template<typename T>
class use_pos_binder_const : public use_binder
{
public:
	use_pos_binder_const(T const& value)
		: value_(value)
	{
	}
protected:
	T const value_;
private:
	void do_bind(statement& st, int pos)
	{
		st.use_value(pos, converter<T>::from(this->value_));
	}
};

/// Named use binder.
template<typename T>
class use_name_binder : public use_pos_binder<T>
{
public:
	use_name_binder(T& value, string_t const& name)
		: use_pos_binder<T>(value)
		, name_(name)
	{
	}
private:
	void do_bind(statement& st, int)
	{
		st.use_value(st.use_pos(this->name_), converter<T>::from(this->value_));
	}

	string_t name_;
};
template<typename T>
class use_name_binder_const : public use_pos_binder_const<T>
{
public:
	use_name_binder_const(T const& value, string_t const& name)
		: use_pos_binder_const<T>(value)
		, name_(name)
	{
	}
private:
	void do_bind(statement& st, int)
	{
		st.use_value(st.use_pos(this->name_), converter<T>::from(this->value_));
	}

	string_t name_;
};

// Create position use binding for reference t.
template<typename T>
inline use_binder_ptr use(T& t)
{
	return use_binder_ptr(new use_pos_binder<T>(t));
}

template<typename T>
inline use_binder_ptr use(T const& t)
{
	return use_binder_ptr(new use_pos_binder_const<T>(t));
}
//----------------------------------------------------------------------------

// Create named use binding for reference t.
template<typename T>
inline use_binder_ptr use(T& t, string_t const& name)
{
	return use_binder_ptr(new use_name_binder<T>(t, name));
}

template<typename T>
inline use_binder_ptr use(T const& t, string_t const& name)
{
	return use_binder_ptr(new use_name_binder_const<T>(t, name));
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////

} //namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////

#endif // SQLITEPP_USE_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
