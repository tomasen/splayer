//////////////////////////////////////////////////////////////////////////////
// $Id: session.hpp 77 2010-08-02 11:13:27Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SQLITEPP_SESSION_HPP_INCLUDED
#define SQLITEPP_SESSION_HPP_INCLUDED

#include "string.hpp"
#include "query.hpp"

struct sqlite3;

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

class transaction;

// Database session. Noncopyable.
class session
{
	friend class transaction; // access to active_txn_ and impl_
	friend class statement;   // access to impl_ and check_error
public:
	// Create a session.
	session();
	
	// Create and open session.
	explicit session(string_t const& file_name);
	
	// Close session on destroy.
	~session();

	// Open database session. Previous one will be closed.
	void open(string_t const& file_name);
	
	// Close database session.
	void close();

	// Is session opened?
	bool is_open() const // throw()
	{
		return impl_ != 0;
	}

	// Is there an active transaction?
	// Currently SQLite 3 doesn't support nested transactions.
	// So we can test, is there any transaction in session.
	// If we have the transaction, we get it or null otherwise.
	transaction* active_txn() const // throw()
	{
		return active_txn_; 
	}

	// Last session error
	int last_error() const;

	// Last statement::exec result
	bool last_exec() const { return last_exec_; }

	// Last insert row ID
	long long last_insert_rowid() const;
	
	// The number of rows that were changed (or inserted or deleted)
	// by the most recent SQL statement
	size_t last_changes() const;

	// The number of rows that were changed (or inserted or deleted)
	// since the database was opened
	size_t total_changes() const;

	// Execute SQL query immediately.
	// It might be useful for resultless statements like INSERT, UPDATE etc.
	// T is any output-stream-shiftable type.
	template<typename T>
	once_query operator<<(T const& t)
	{
		once_query q(*this);
		q << t;
		return q;
	}

	// Swap session instances
	friend void swap(session& lhs, session& rhs)
	{
		std::swap(lhs.impl_, rhs.impl_);
		std::swap(lhs.active_txn_, rhs.active_txn_);
	}
private:
	// Noncopyable.
	session(session const&);
	// Nonassignable.
	session& operator=(session const&);

	/// Check error code. If code is not ok, throws exception.
	void check_error(int code) const;
	void check_last_error() const;

	sqlite3* impl_;
	transaction* active_txn_;
	bool last_exec_;
};

//////////////////////////////////////////////////////////////////////////////

} // end of namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////

#endif // SQLITEPP_SESSION_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
