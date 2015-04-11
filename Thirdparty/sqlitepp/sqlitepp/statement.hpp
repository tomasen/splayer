//////////////////////////////////////////////////////////////////////////////
// $Id: statement.hpp 77 2010-08-02 11:13:27Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SQLITEPP_STATEMENT_HPP_INCLUDED
#define SQLITEPP_STATEMENT_HPP_INCLUDED

#include <vector>

#include "string.hpp"
#include "query.hpp" 
#include "converters.hpp" 

struct sqlite3_stmt;

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

class session;

// Database statement, noncopyable
class statement
{
public:
	// Create an empty statement
	explicit statement(session& s);
	
	// Create statement with SQL query text.
	statement(session& s, string_t const& sql);

	// Finalize statement on destroy.
	~statement();

	// Execute statement. Return true if result exists.
	bool exec();

	// Prepare statement.
	void prepare();

	// Finalize statement.
	void finalize(bool check_error = true);

	// Is statement prepared.
	bool is_prepared() const // throw() 
	{
		return impl_ != 0; 
	}

	// Reset statement. Return to prepared state. Optionally rebind values
	void reset(bool rebind = false);

	// Start statement preparing.
	template<typename T>
	prepare_query operator<<(T const& t)
	{
		prepare_query pq(*this);
		pq << t;
		return pq;
	}

	// Statement query.
	query const& q() const // throw()
	{
		return q_;
	}

	// Statement query.
	query& q() // throw()
	{
		return q_;
	}

	// Number of columns in result set of prepared statement.
	int column_count() const;
	// Column name in result set.
	string_t column_name(int column) const;
	// Column index in result set.
	int column_index(string_t const& name) const;
	// Colmn type of result set in prepared statement.
	enum col_type { COL_INT = 1, COL_FLOAT = 2, COL_TEXT = 3, COL_BLOB = 4, COL_NULL = 5 };
	// Column type in result set.
	col_type column_type(int column) const;

	// Column value as int.
	void column_value(int column, int& value) const;
	// Column value as 64-bit int.
	void column_value(int column, long long& value) const;
	// Column value as double.
	void column_value(int column, double& value) const;
	// Column value as string.
	void column_value(int column, string_t& value) const;
	// Column value as BLOB.
	void column_value(int column, blob& value) const;

	// Get column value as type T.
	template<typename T>
	T get(int column) const
	{
		typename converter<T>::base_type t;
		column_value(column, t);
		return converter<T>::to(t);
	}

	// Use int value in query.
	void use_value(int pos, int value);
	// Use 64-bit int value in query.
	void use_value(int pos, long long value);
	// Use double value in query.
	void use_value(int pos, double value);
	// Use string value in query.
	void use_value(int pos, string_t const& value, bool make_copy = true);
	// Use BLOB value in query.
	void use_value(int pos, blob const& value, bool make_copy = true);

	// Get use position by name in query.
	int use_pos(string_t const& name) const;
private:
	// Copy not allowed.
	statement(statement const&);
	// Assignment not allowed.
	statement& operator=(statement const&);
	
	session& s_;
	query q_;
	sqlite3_stmt* impl_;
};

//////////////////////////////////////////////////////////////////////////////

} // namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////

#endif // SQLITEPP_STATEMENT_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
