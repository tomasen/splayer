// $Id: session_data.hpp 46 2006-09-05 06:38:56Z pmed $

#ifndef SQLITEPP_TEST_SESSION_DATA_HPP_INCLUDED
#define SQLITEPP_TEST_SESSION_DATA_HPP_INCLUDED

#include <sqlitepp/string.hpp>
#include <sqlitepp/session.hpp>

struct session_data
{
	session_data(sqlitepp::string_t const& name = sqlitepp::utf(L"test.db"));
	~session_data();

	sqlitepp::string_t name_;
	sqlitepp::session se;
};

#endif // SQLITEPP_TEST_SESSION_DATA_HPP_INCLUDED
