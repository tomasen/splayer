// $Id: statement_data.hpp 74 2009-02-01 11:29:27Z pmed $

#ifndef SQLITEPP_TEST_STATEMENT_DATA_HPP_INCLUDED
#define SQLITEPP_TEST_STATEMENT_DATA_HPP_INCLUDED

#include <vector>

#include <sqlitepp/string.hpp>
#include <sqlitepp/statement.hpp>

#include "session_data.hpp"

using sqlitepp::string_t;

struct statement_data : session_data
{
	sqlitepp::statement st;

	statement_data();
	~statement_data();

	struct record
	{
		int id;
		string_t name;
		double salary;
		
		typedef std::vector<char> blob_data;
		blob_data data;

		record() {}

		record(int id, string_t const& name, double salary)
			: id(id), name(name), salary(salary)
		{
		}

		void insert(sqlitepp::session& se);
	};
};

namespace tut {
void ensure_equals(statement_data::record const& r1, 
				   statement_data::record const& r2);
} // namespace tut {


template<typename T, size_t N>
inline size_t dimof( T(&)[N] ) { return N; }

#endif // SQLITEPP_TEST_STATEMENT_DATA_HPP_INCLUDED
