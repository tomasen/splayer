//////////////////////////////////////////////////////////////////////////////
// $Id: transaction.hpp 46 2006-09-05 06:38:56Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SQLITEPP_TRANSACTION_HPP_INCLUDED
#define SQLITEPP_TRANSACTION_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

class session;

// Transaction. Noncopyable.
class transaction
{
public:
	// Begin transaction in context of session.
	transaction(session& s);

	// End transaction with rollback if it is not commited.
	~transaction();

	// Commit transaction.
	void commit();
private:
	transaction(transaction const&);
	transaction& operator=(transaction const&);

	session& s_;
	bool do_rollback_;
};

//////////////////////////////////////////////////////////////////////////////

} // namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////

#endif // SQLITEPP_TRANSACTION_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
