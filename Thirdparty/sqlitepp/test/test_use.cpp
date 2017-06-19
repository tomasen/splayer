// $Id: test_use.cpp 77 2010-08-02 11:13:27Z pmed $

#include <tut.h>

#include <sqlitepp/exception.hpp>
#include <sqlitepp/into.hpp>
#include <sqlitepp/use.hpp>

#include "statement_data.hpp"

using namespace sqlitepp;

namespace tut {
	
struct use_data : statement_data
{
	use_data()
	{
	}

	string_t const& str() const
	{
		static string_t const result(utf(L"string"));
		return result;
	}
};

typedef tut::test_group<use_data> use_test_group;
typedef use_test_group::object object;

use_test_group u_g("8. use");

// use by pos binding
template<>template<>
void object::test<1>()
{
	record recs[2] = { record(1, utf(L"Petya"), 123.45), record(2, utf(L"Vasya"), 678.90) };

	// insert records

	for (size_t i = 0; i < dimof(recs); ++i)
	{
		st << utf(L"insert into some_table values( :id, :name, :salary, NULL)"),
			use(recs[i].id), use(recs[i].name), use(recs[i].salary);
		ensure("row inserted", !st.exec() );
		ensure("no data", !se.last_exec());
	}

	// count inserted record
	int count;
	se << utf(L"select count(*) from some_table"), into(count);
	ensure("row", se.last_exec());
	ensure_equals("row count", count, static_cast<int>(dimof(recs)) ); 

	// check inserted records
	record r;
	st << utf(L"select id, name, salary from some_table"),
		into(r.id, utf(L"id")), into(r.name, utf(L"name")), into(r.salary, utf(L"salary"));
	for (count = 0; st.exec(); ++count)
	{
		ensure("row", se.last_exec());
		ensure_equals(r, recs[count]);
	}
	ensure_equals("row count", count, static_cast<int>(dimof(recs))); 
}

// use by name binding
template<>template<>
void object::test<2>()
{
	record recs[2] = { record(1, utf(L"Vova"), 123.45), record(2, utf(L"Serega"), 678.90) };

	// insert records
	for (size_t i = 0; i < dimof(recs); ++i)
	{
		st << utf(L"insert into some_table(id, name, salary) values( :id, :name, :salary)"),
			use(recs[i].id, utf(L":id")),
			use(recs[i].name, utf(L":name")),
			use(recs[i].salary, utf(L":salary"));
		ensure("row inserted", !st.exec() );
		ensure("no row", !se.last_exec());
	}

	// count inserted record
	int count;
	se << utf(L"select count(*) from some_table"), into(count);
	ensure("row", se.last_exec());
	ensure_equals("row count", count, static_cast<int>(dimof(recs)) ); 

	// check inserted records
	record r;
	st << utf(L"select id, name, salary from some_table"),
		into(r.id, utf(L"id")), into(r.name, utf(L"name")), into(r.salary, utf(L"salary"));
	for (count = 0; st.exec(); ++count)
	{
		ensure("row", se.last_exec());
		ensure_equals(r, recs[count]);
	}
	ensure_equals(count, static_cast<int>(dimof(recs))); 
}

// use by invalid name binding
template<>template<>
void object::test<3>()
{
	try
	{
		int id;
		st << utf(L"insert into some_table(id, name, salary) values( :id, '', 0)"),
			use(id, utf(L"id_ZZZ"));
		st.exec();
		fail( "exception expected" );
	}
	catch(sqlitepp::no_such_column const&)
	{
		ensure( "statement not prepared", !st.is_prepared() );
	}
}

// use BLOB
template<>template<>
void object::test<4>()
{
	const char BLOB_data[] = { "very big data" };

	record r1(1, utf(L"Vera"), 545.6);
	r1.data.assign(BLOB_data, BLOB_data + sizeof(BLOB_data));
	
	st << utf(L"insert into some_table(id, name, salary, data) values( :id, :name, :salary, :data)"),
		use(r1.data, utf(L":data")), use(r1.id, utf(L":id")),
		use(r1.name, utf(L":name")), use(r1.salary, utf(L":salary"));
	ensure("row inserted", !st.exec() );
	ensure("no row", !se.last_exec());

	record r2;
	st << utf(L"select * from some_table where id = :1"),
		into(r2.id), into(r2.name), into(r2.salary), into(r2.data), use(r1.id);

	ensure("select completed", st.exec());
	ensure("row", se.last_exec());
	ensure_equals(r2, r1);
	ensure( "single row", !st.exec() );
	ensure("no row", !se.last_exec());
}

// insert loop
template<>template<>
void object::test<5>()
{
	int id = 1;
	int MAX_RECORD = 100;

	st << utf("insert into some_table(id) values(:id)"), use(id);
	ensure("no row", !se.last_exec());
	
	se << utf(L"begin");
	for (; id <= MAX_RECORD; ++id)
	{
		st.reset(true);
		st.exec();
		ensure("row", !se.last_exec());
	}
	se << utf(L"commit");

	int id2;
	st << utf(L"select id from some_table"), into(id2);

	for (id = 1; st.exec(); ++id)
	{
		ensure("row", se.last_exec());
		ensure_equals("id", id2, id);
	}
	ensure_equals("MAX_RECORD", id - 1, MAX_RECORD);
	ensure_equals("MAX_RECORD", id2, MAX_RECORD);
}

} // namespace
