// $Id: test_exception.cpp 46 2006-09-05 06:38:56Z pmed $

#include <tut.h>

#include <sqlitepp/exception.hpp>

#include "session_data.hpp"

using namespace sqlitepp;

namespace tut {

struct exception_data : session_data
{
	exception ex;
	exception_data() : ex(2343, utf(L"exception message")) {}
};

typedef test_group<exception_data> ex_test_group;
typedef ex_test_group::object object;

ex_test_group ex_g("2. exception");

template<>template<>
void object::test<1>()
{
	ensure_equals("code", ex.code(), 2343);
	ensure_equals("what", ex.what(), utf8(L"exception message"));
}

template<>template<>
void object::test<2>()
{
	exception ex_copy(ex);
	ensure_equals("code copy", ex_copy.code(), 2343);
	ensure_equals("what copy", ex_copy.what(), utf8(L"exception message"));
}

template<>template<>
void object::test<3>()
{
	try
	{
		try
		{
			throw ex;
		}
		catch(std::runtime_error const& rt_err)
		{
			ensure_equals("catched what", rt_err.what(), utf8(L"exception message"));
			throw;
		}
	}
	catch(sqlitepp::exception const& e)
	{
		ensure_equals("catched code", e.code(), 2343);
		ensure_equals("catched what", e.what(), utf8(L"exception message"));
	}
}

template<>template<>
void object::test<4>()
{
	try
	{
		se << utf(L"qaz");
		fail( "exception expected");
	}
	catch(sqlitepp::exception const& ex)
	{
		ensure( "error code", ex.code() != 0 );
		ensure( "what", ex.what() != 0 );
	}
}

} // namespace tut {

