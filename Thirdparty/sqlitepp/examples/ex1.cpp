#include <iostream>
#include <stdexcept>
#include <string>

#include <sqlitepp/session.hpp>

struct employee
{
    std::string name;
    int age;
    float salary;
};

int main()
{
    using namespace sqlitepp;
    
    try
    {
        session db("enterprise.db");

        db << "create table employee(id integer primary key, name text, age integer, salary real)";
        db << "insert into employee values(null, 'Bob', 31, 5500)";

        employee const employees[] = { {"Alice", 23, 3220}, {"Fred", 54, 4870} };
        for (int i = 0; i < 2; ++i)
        {
            db << "insert into employee values(null, '" 
                << employees[i].name << "' ," << employees[i].age << ", " << employees[i].salary << ")";
        }
        
        std::string const table_name = "employee";
        db << "drop table " << table_name;
    }
    catch(std::runtime_error const& err)
    {
        std::cerr << err.what();
        return -1;
    }
}
