// $Id: test_query.cpp 46 2006-09-05 06:38:56Z pmed $

#include <tut.h>

#include <locale>

#include <sqlitepp/query.hpp>
#include <sqlitepp/binders.hpp>

using namespace sqlitepp;

namespace tut {

struct query_data
{
	query q;
};

typedef tut::test_group<query_data> query_test_group;
typedef query_test_group::object object;

query_test_group qg("6. query");

template<>template<>
void object::test<1>()
{
	ensure("empty query", q.empty() );

	q.sql(utf(L"qaz"));
	ensure_equals("q.sql() == qaz", q.sql(), utf(L"qaz") );
}

template<>template<>
void object::test<2>()
{
	int z = 100;
	q << utf(L"qaz") << z;
	ensure_equals("q.sql() == qaz100", q.sql(), utf(L"qaz100"));
}

template<>template<>
void object::test<6>()
{
	try
	{
		into_binder_ptr i;
		q.put(i);
		fail("exception expected");
	}
	catch(std::invalid_argument const&)
	{
	}
	ensure("intos empty", q.intos().empty());
	try
	{
		use_binder_ptr u;
		q.put(u);
		fail("exception expected");
	}
	catch(std::invalid_argument const&)
	{
	}
	ensure("uses empty", q.uses().empty());
}

} // namespace tut {
