#include <iostream>
#include <exception>
#include <string>

#include <sqlitepp/sqlitepp.hpp>

struct employee
{
    std::string name;
    int age;
    float salary;
};

std::ostream& operator<<(std::ostream& os, employee const& e)
{
    return os << e.name << ": " << e.age << ", earns " << e.salary << "$ in month";
}

std::istream& operator>>(std::istream& is, employee & e)
{
    return is >> e.name >> e.age >> e.salary;
}

int main()
{
    using namespace sqlitepp;
    
    try
    {
        // already known things
        session db("enterprise.db");
        db << "create table employee(id integer primary key, name text, age integer, salary real)";

        
        // Heh!
        employee e;
        statement st(db);
        
        // (1)
        st << "insert into employee values(null, :name, :age, :salary)"
                , use(e.name), use(e.age), use(e.salary);
        while ( std::cin >> e )
        {
			st.exec(); // (1.1)
        }

        // (2)
        std::cout << "\n\t-- Employees --\n";
        st << "select name, age, salary from employee", into(e.name), into(e.age), into(e.salary);
        while ( st.exec() )
        {
            std::cout << e << std::endl;
        }
    }
    catch(std::exception const& ex)
    {
        std::cerr << ex.what();
        return -1;
    }
}
