// $Id: test_convert.cpp 77 2010-08-02 11:13:27Z pmed $

#include <tut.h>
#include <time.h>

#include <sqlitepp/string.hpp>
#include <sqlitepp/transaction.hpp>
#include <sqlitepp/use.hpp>
#include <sqlitepp/into.hpp>

#include "statement_data.hpp"

using namespace sqlitepp;

struct my_data
{
	int value;
};

namespace sqlitepp {

// specialize convert for the tm struct
template<>
struct converter<tm>
{
	typedef long long base_type;
	static long long from(tm& src)
	{
		return mktime(&src);
	}
	static tm to(long long src)
	{
		time_t tt = src;
		return *localtime(&tt);
	}
};

template<>
struct converter<my_data>
{
	typedef string_t base_type;
	
	static string_t from(my_data const& src)
	{
		char buf[20];
		itoa(src.value, buf, 10);
		return utf(buf);
	}
	
	static my_data to(string_t const& src)
	{
		my_data result;
		result.value = atoi(utf8(src).c_str());
		return result;
	}
};

} // namespace sqlitepp 

namespace tut {

struct conv_data : statement_data
{
	conv_data()
	{
		se << utf(L"insert into some_table(id, name, salary) values(100, 'qaqa', 0.1)");
	}

	template<typename T>
	void select_id()
	{
		T id;
		se << utf(L"select id from some_table"), into(id);
		ensure_equals("id", id, T(100));
	}

	template<typename T>
	void select_salary(T const& expected, T const& dist)
	{
		T salary;
		se << utf(L"select salary from some_table"), into(salary);
		ensure_distance("salary", salary, expected, dist);
	}
};

typedef test_group<conv_data> conv_test_group;
typedef conv_test_group::object object;

conv_test_group conv_tests("9. conversion");

template<>template<>
void object::test<1>()
{
	select_id<char>();
	select_id<unsigned char>();
	select_id<signed char>();
	select_id<short>();
	select_id<short>();
	select_id<unsigned short>();
	select_id<int>();
	select_id<unsigned int>();
}

template<>template<>
void object::test<2>()
{
	select_salary<float>(0.1f, 0.01f);
	select_salary<double>(0.1, 0.01);
}

template<>template<>
void object::test<3>()
{
	transaction txn(se);

	se << utf(L"create table timer(t integer)");
	
	time_t now = time(0);
	tm t1 = *localtime(&now);
	se << utf(L"insert into timer(t) values(:t)"), use(t1);
	
	tm t2;
	se << utf(L"select t from timer"), into(t2);

	ensure("tm equal", memcmp(&t1, &t2, sizeof(tm)) == 0);
}

template<>template<>
void object::test<4>()
{
#ifdef SQLITEPP_ENUM_CONVERTER
	enum enum_type { VAL1, VAL2, VAL3 } val;
	transaction txn(se);

	se << utf(L"create table enum_test(val integer)");
	
	val = VAL2;
	se << utf(L"insert into enum_test(val) values(:val)"), use(val);
	se << utf(L"select val from enum_test"), into(val);
	ensure_equals("enum", val, VAL2);
	
	se << utf(L"delete from enum_test");
	se << utf(L"insert into enum_test(val) values(:val)"), use(1000);
	se << utf(L"select val from enum_test"), into(val);
	ensure_equals("out of enum", val, 1000);
#endif
}

template<>template<>
void object::test<5>()
{
	transaction txn(se);

	se << utf(L"create table my_data(val text)");
	
	my_data data, data2;
	data.value = 99;
	se << utf(L"insert into my_data(val) values(:val)"), use(data);
	se << utf(L"select val from my_data"), into(data2);
	ensure_equals("my_data", data2.value, data.value);
}

} // namespace tut {
