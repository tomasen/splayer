// $Id: main.cpp 46 2006-09-05 06:38:56Z pmed $

#include <iostream>

#include <sqlitepp/string.hpp>
#include <sqlitepp/sqlitepp.hpp>

#include <tut.h>
#include <tut_reporter.h>

namespace tut
{
	test_runner_singleton runner;
}

int main(int argc, char** argv)
{
	std::cout << "SQLite++ test application." << std::endl;
	std::cout << "char_t size " << sizeof(sqlitepp::char_t) << std::endl;

	tut::reporter callback;
	tut::runner.get().set_callback(&callback);
	try
	{
		switch ( argc )
		{
		case 1:
			// run all tests
			tut::runner.get().run_tests();
			break;
		case 2: 
			// run a test group
			tut::runner.get().run_tests(argv[1]);
			break;
		case 3:
			// run particular test in a group
			tut::runner.get().run_test(argv[1], atoi(argv[2]));
			break;
		default:
			std::cout << "Usage\n" << argv[0] << " [group name] [test number]" << std::endl;
			return -1;
		}
	}
	catch( std::exception const& ex )
	{
		std::cerr << "tut raised ex: " << ex.what() << std::endl;
		return -999;
	}
	
	return 0;
}
