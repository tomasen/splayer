//////////////////////////////////////////////////////////////////////////////
// $Id: exception.cpp 53 2007-07-12 13:25:37Z pmed $
//
// Copyright (c) 2004 Pavel Medvedev
// Use, modification and distribution is subject to the 
// Boost Software License, Version 1.0. (See accompanying file 
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../sqlite/sqlite3.h"

#include "exception.hpp"
#include "session.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

exception::exception(int code, string_t const& msg)
    : std::runtime_error(reinterpret_cast<char const*>(utf8(msg).c_str()))
    , code_(code)
{
}

nested_txn_not_supported::nested_txn_not_supported()
	: exception(-1, utf(L"nested transactions are not supported"))
{
}

no_such_column::no_such_column(string_t const& col_name)
	: exception(-2, utf(L"no such column '") + col_name + utf(L"'"))
{
}

session_not_open::session_not_open()
	: exception(-3, utf(L"session not open"))
{
}

multi_stmt_not_supported::multi_stmt_not_supported()
	: exception(-4, utf(L"only one statement is supported"))
{
}

//////////////////////////////////////////////////////////////////////////////

} // namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////
