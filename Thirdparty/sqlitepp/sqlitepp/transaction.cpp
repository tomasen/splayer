//////////////////////////////////////////////////////////////////////////////
// $Id: transaction.cpp 53 2007-07-12 13:25:37Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "transaction.hpp"
#include "exception.hpp"
#include "session.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

transaction::transaction(session& s) : s_(s), do_rollback_(false)
{
	if ( s_.active_txn() )
	{
		throw nested_txn_not_supported();
	}
	s_ << utf(L"begin");
	s_.active_txn_ = this;
	do_rollback_ = true;
}
//----------------------------------------------------------------------------

transaction::~transaction() 
{
	if ( do_rollback_ )
	{
		s_ << utf(L"rollback");
	}
	s_.active_txn_ = 0;
}
//----------------------------------------------------------------------------

void transaction::commit()
{
	s_ << utf(L"commit");
    do_rollback_ = false;
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////

} // namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////
