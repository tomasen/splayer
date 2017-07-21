~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SQLite++
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------
C++ wrapper to SQLite library
------------------------------

:Author: Pavel Medvedev
:Contact: pmedvedev@gmail.com
:date: $Date: 2008-01-24 20:10:59 +0800 (Thu, 24 Jan 2008) $
:revision: $Rev: 65 $
:copyright: Copyright (c) 2004-2008 Pavel Medvedev

.. contents:: Table of Contents
.. section-numbering::


Overview
========

SQLite++ is an object-oriented wrapper library of SQLite_ version 3. 
The latest sources are available from `Subversion repository`_

The main idea is to use simple variable binding with SQL statements::

    int count;
    db << "select count(*) from employee", into(count);


The library is distributed under Boost Software License (see accompanying 
file LICENSE_1_0.txt_ in the project root). Additional information about 
Boost Software License is available at `Boost site`_

Approaches with binding variables to SQL queries were inspired by 
Maciej Sobczak' `SOCI wrapper`_.

.. _SQLite:                  http://sqlite.org
.. _`SOCI wrapper`:          http://www.msobczak.com/prog/soci/
.. _LICENSE_1_0.txt:         ../LICENSE_1_0.txt
.. _`Boost site`:            http://boost.org/more/license_info.html
.. _`Subversion repository`: https://svn.berlios.de/svnroot/repos/sqlitepp/trunk


Installation
============

This section describes how to build and install SQLitepp from released 
source distribution.

SQLite++ uses `Boost Build v2`_. Refer to the documentation_ how to install 
Boost Build v2.

To build SQLite++ change working dir to the root of this distribution 
and invoke

    ``bjam [build-variant]``

Where build-variant is an optional predefined build variant name.
There are several predefined variants:

.. table:: Build variants

    +-------------+-----------------------------------------+
    |  ``utf8``   | UTF-8 encoding support. Defaul variant. |
    +-------------+-----------------------------------------+
    |  ``utf8d``  | UTF-8 encoding support, debug version.  |
    +-------------+-----------------------------------------+
    |  ``utf16``  | UTF-16 encoding support.                |
    +-------------+-----------------------------------------+
    |  ``utf16d`` | UTF-16 encoding support, debug version. |
    +-------------+-----------------------------------------+

After successful building two subdirectories will be created:

  - ``lib``      - with binary library files. 
  - ``include``  -  with header files.

Make them available for a compiler in your favorite way and have fun ;-)

Testing
-------

You might want to run unit-tests of SQLite++. It's simple with bjam:

    ``bjam test [build-variant]``

Test application uses excellent unit-testing library TUT_

.. _`Boost Build v2`: http://sourceforge.net/project/showfiles.php?group_id=7586&package_id=72941
.. _documentation: http://boost.org/tools/build/v2/index.html
.. _TUT: http://tut-framework.sourceforge.net/

Short Tutorial
==============

Let's make simple database application. It creates database with table
``employee`` and adds a record.

::

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

            db << "create table employee(id integer primary key, name text,"
                  " age integer, salary real)";
            db << "insert into employee values(null, 'Bob', 31, 5500)";

            employee const employees[] = { {"Alice", 23, 3220}, 
                                           {"Fred", 54, 4870} };
            for (int i = 0; i < 2; ++i)
            {
                db << "insert into employee values(null, '" 
                   << employees[i].name << "' ," << employees[i].age << ", "
                   << employees[i].salary << ")";
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

All SQLite-related errors manifest themselves as exceptions of type
``sqlitepp::exception`` derived from ``std::runtime_error``. This allows 
handling errors within the standard library exception hierarchy.

Session class encapsulates SQLite database. The session constructor accepts
database file name. As it should be destructor closes database. SQL statements
may be executed immediately by session. In this case implicit statement will
be created and executed. Statement has an ostream-like interface, so an SQL
text can be composed of many parts, involving any object that supports output
stream shifting (i.e. custom classes with ``operator<<``).

Ok, while it seems like just yet another object-oriented database library.
Next example.

.. parsed-literal::

    #include <iostream>
    #include <stdexcept>
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
        return os << e.name << ": " << e.age << ", earns " 
                  << e.salary << "$ in month";
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
            db << "create table employee(id integer primary key, name text,"
                  " age integer, salary real)";

            // Heh!
            employee e;
            statement st(db);

            // **(1)**
            st << "insert into employee values(null, :name, :age, :salary)",
               use(e.name), use(e.age), use(e.salary);
            while ( std::cin >> e )
            {
                st.exec(); // **(1.1)**
            }

            // **(2)**
            std::cout << "\n\t-- Employees --\n";
            st << "select name, age, salary from employee", 
                into(e.name), into(e.age), into(e.salary);
            
            while ( st.exec() )
            {
                std::cout << e << std::endl;
            }
        }
        catch(std::runtime_error const& err)
        {
            std::cerr << err.what();
            return -1;
        }
    }

Again usual things happen - we create session and table. Then we create
statement object ``st``. It is executing in context of database ``db``. In the
code block marked as (1) we prepare SQL query and bind variable e members to
SQL values(:name, :age, :salary) of the same name. Next, in loop we ask user
to enter employee data. In line marked (1.1) the statement is executed and
data of members``e`` are inserted into the table ``employee``.

But let's check contents of the ``employee``. The code block marked as (2)
demonstrates it. Statement ``st`` is preparing with new SQL select query. Note
the members of ``e`` are bound to SQL result columns again. Loop counts until
statement select has data.

Reference
=========

This section provides the information how to use SQLite++.

String. Aha, Yet Another One
----------------------------

Internationalization, code pages, all these things may cause a big headache.
Fortunately, SQLite stores text fields in UTF-8 or UTF-16 encoded. So,
actually SQLite++ has to support either one or another. There are 3 types
of char type:

.. table:: Character types

    +------------------+----------+
    |  ``utf8_char``   |  1 byte  |
    +------------------+----------+
    |  ``utf16_char``  |  2 byte  |
    +------------------+----------+
    |  ``utf32_char``  |  4 byte  |
    +------------------+----------+

and 3 string types::

  typedef std::basic_string<utf8_char>    utf8_string;
  typedef std::basic_string<utf16_char>   utf16_string;
  typedef std::basic_string<utf32_char>   utf32_string;

So, there is a bunch of string encoding routines.

  * For UTF-8 encoding::

        utf8_string utf8(utf16_char const* str, size_t size = npos);
        utf8_string utf8(utf16_string const& str);
        utf8_string utf8(utf32_char const* str, size_t size = npos);
        utf8_string utf8(utf32_string const& str);

  * For UTF-16 encoding::

        utf16_string utf16(utf8_char const* str, size_t size = npos);
        utf16_string utf16(utf8_string const& str);
        utf16_string utf16(utf32_char const* str, size_t size = npos);
        utf16_string utf16(utf32_string const& str);

  * For UTF-32 encoding::

        utf32_string utf32(utf8_char const* str, size_t size = npos);
        utf32_string utf32(utf8_string const& str);
        utf32_string utf32(utf16_char const* str, size_t size = npos);
        utf32_string utf32(utf16_string const& str);

To turn on UTF-16 encoding support define preprocessor symbol
``SQLITEPP_UTF16``. Otherwise UTF-8 will be used. The common way to abstract
from encoding used is to define generalized character type. In SQLite++ is a
``char_t``::

    #ifdef SQLITEPP_UTF16
        typedef utf16_char   char_t;
        typedef utf16_string string_t;
    #else
        typedef utf8_char    char_t;
        typedef utf8_string  string_t;
    #endif // SQLITEPP_UTF16 


Also generalized conversion routines are used::

    string_t utf(utf8_char const* str, size_t size = npos);
    string_t utf(utf8_string const& str);
    string_t utf(utf16_char const* str, size_t size = npos);
    string_t utf(utf16_string const& str);
    string_t utf(utf32_char const* str, size_t size = npos);
    string_t utf(utf32_string const& str);

.. Note:: ``utf8_char`` is the C++ char type, therefore it's possible don't
          use UTF-8 encoding at all. SQLite and SQLite++ make no checks and
          interpret a string as a raw byte sequence.

Exceptions
----------

All SQLite errors mapped to SQLite++ exceptions.
Base class is a ``sqlitepp::exception``::

    class exception : public std::runtime_error
    {
    public:
        // Create exception with UTF encoded message
        exception(int code, string_t const& msg);

        // SQLite library error code.
        int code() const; // throw()
    };

.. Note:: ``sqlitepp::exception::what()`` returns UTF-8 encoded SQLite error
          message (see sqlite3_errmsg_)

.. _sqlite3_errmsg: http://sqlite.org/capi3ref.html#sqlite3_errmsg

There are some custom SQLite++ exceptions:

+--------------------------------+------------------------------------------+
|                                |  This exception is thrown when the user  |
|                                |  tries to run nested transaction. This   |
|                                |  is a SQLite  limitation -  lack of      |
|                                |  nested transactions. ::                 |
|                                |                                          |
|                                |    // start a transaction                |
|  ``nested_txn_not_supported``  |    transaction txn(db);                  |
|                                |    {                                     |
|                                |      // try start another one            |
|                                |      // will throw exception             |
|                                |      // ``nested_txn_not_supported``     |
|                                |      transaction nested(db);             |
|                                |    }                                     |
+--------------------------------+------------------------------------------+
|                                |  This exception is thrown when the user  |
|                                |  tries to get statement column by name   |
|  ``no_such_column``            |  (see statement::column_index)::         |
|                                |                                          |
|                                |    // table zz(int id, name text)        |
|                                |    statement::column_index("qqq");       |
+--------------------------------+------------------------------------------+
|                                |  This exception is thrown when the user  |
|                                |  tries to excecute SQL statement in not  |
|  ``session_not_open``          |  opened session::                        |
|                                |                                          |
|                                |    session s;                            |
|                                |    s << "drop table q";                  |
+--------------------------------+------------------------------------------+
|                                |  This excpetion is thrown when the user  |
|                                |  tries to excecute multiple SQL queries  |
|  ``multi_stmt_not_supported``  |  in the one statement::                  |
|                                |                                          |
|                                |    statement st(se, "select * from t1;"  |
|                                |                     " select * from t2"; |
+--------------------------------+------------------------------------------+


Session
-------

Session is a SQLite database abstraction::

    // Database session. Noncopyable.
    class session
    {
    public:
        // Create a session.
        session();

        // Create and open session.
        explicit session(string_t const& file_name);
        
        // Close session on destroy.
        ~session();

        // Open database session. Previous one will be closed.
        void open(string_t const& file_name);

        // Close database session.
        void close();

        // Is session opened?
        bool is_open() const; // throw()

        // Is there an active transaction?
        // Currently SQLite 3 doesn't support nested transactions.
        // So we can test, is there any transaction in session.
        // If we have the transaction, we get it or null otherwise.
        transaction* active_txn() const; // throw()

        // Last insert row ID
        long long last_insert_rowid() const;
        
        // The number of rows that were changed (or inserted or deleted)
        // by the most recent SQL statement
        size_t last_changes() const;
    
        // The number of rows that were changed (or inserted or deleted)
        // since the database was opened
        size_t total_changes() const;
        
        // Execute SQL query immediately.
        // It might be useful for resultless statements like INSERT, UPDATE etc.
        // T is any output-stream-shiftable type.
        template<typename T>
        once_query operator<<(T const& t);
        
        // Swap session instances
        friend void swap(session& lhs, session& rhs);
    };

Statement
---------

Database statement::

    // Database statement, noncopyable
    class statement
    {
    public:
        // Create an empty statement. s is a owner session.
        explicit statement(session& s);
        
        // Create statement with SQL query text.
        statement(session& s, string_t const& sql);
    
        // Finalize statement on destroy.
        ~statement();
    
        // Execute statement. Return is true if result exists.
        bool exec();
    
        // Prepare statement before execution.
        void prepare();
    
        // Finalize statement.
        void finalize();
    
        // Is statement prepared.
        bool is_prepared() const; // throw() 
    
        // Reset statement. Return to prepared state. Optionally rebind values
        void reset(bool rebind = false);
    
        // Start statement preparing by collection query data.
        // T is any output-stream-shiftable type.
        template<typename T>
        prepare_query operator<<(T const& t);
    
        // Statement query.
        query const& q() const; // throw()

        // Statement query reference.
        query& q(); // throw()
    
        // Number of columns in result set of prepared statement.
        int column_count() const;

        // Column name in result set.
        string_t column_name(int column) const;

        // Column index in result set.
        int column_index(string_t const& name) const;

        // Colmn type of result set in prepared statement.
        enum col_type { COL_INT = 1, COL_FLOAT = 2, COL_TEXT = 3,
                        COL_BLOB = 4, COL_NULL = 5 };

        // Column type in result set.
        col_type column_type(int column) const;
    
        // Get column value as int.
        void column_value(int column, int& value) const;
        
        // Get column value as 64-bit int.
        void column_value(int column, long long& value) const;
        
        // Get column value as double.
        void column_value(int column, double& value) const;
        
        // Get column value as string.
        void column_value(int column, string_t& value) const;
        
        // Get column value as BLOB.
        void column_value(int column, blob& value) const;
    
        // Get column value as type T.
        template<typename T>
        T get(int column) const;
    
        // Use int value in query.
        void use_value(int pos, int value);
        
        // Use 64-bit int value in query.
        void use_value(int pos, long long value);
        
        // Use double value in query.
        void use_value(int pos, double value);
        
        // Use string value in query.
        void use_value(int pos, string_t const& value);
        
        // Use BLOB value in query.
        void use_value(int pos, blob const& value);
    
        // Get use position by name in query.
        int use_pos(string_t const& name) const;
    };

Transaction
-----------

Transaction is a RAII class that starts transaction in constructor and ends
one in destructor. By default, the transaction is finished with rollback.
You should call ``transaction::commit`` to explicitly make a commit. ::

    // Transaction. Noncopyable.
    class transaction
    {
    public:
        // Begin transaction in context of the session s.
        transaction(session& s);

        // End transaction with rollback if it is not commited.
        ~transaction();

        // Commit transaction.
        void commit();
    };
    
.. note:: Currently SQLite doesn't support nested transactions.

BLOB
----

SQLite tables can contain BLOB columns. BLOB is a simple struct::

    struct blob
    {
        void const* data; // raw data pointer
        size_t size;      // data size in bytes
    };
    
SQLite++ supports conversion between template ``std::vector<T>`` and blob value.
See "Data Conversion" section below.


Binders
-------

Binders are used to bind value into the statement query. There are two types
of binders:
  
  * into binders
  * use binders
  
Into binders
~~~~~~~~~~~~

Into binders are used for binding result set values *into* variables. In the
following example selected value of column ``name`` from table ``employees``
will be stored in variable ``name``::

    session db("test.db");
    statement st(db);
    
    string_t name;
    
    st << "select name from employees where id = 3", into(name);


Into binders are different classes with following interface::

    /// Into binder interface
    class into_binder
    {
    public:
        virtual ~into_binder();

        // Bind variable to statement st in position pos.
        int bind(statement& st, int pos);

        // Update bound variable after statement execution.
        void update(statement& st);
    protected:
        // Protect from improper using
        into_binder();
        into_binder(into_binder const&);
        into_binder& operator=(into_binder const&);
    private:
        // Should implement real binding.
        virtual void do_bind(statement& st, int pos) = 0;

        // Should implement real update.
        virtual void do_update(statement& st) = 0;
    };

    typedef std::auto_ptr<into_binder> into_binder_ptr;

    // Create position into binder for variable reference t.
    template<typename T>
    into_binder_ptr into(T& t);

    // Create named into binder for variable reference t.
    template<typename T>
    inline into_binder_ptr into(T& t, string_t const& name);

Use binders
~~~~~~~~~~~

This binders *use* variables as parameters for SQL queries. Following code
uses variable id to select column ``name`` from table ``employees``::

    session db("test.db");
    statement st(db);
    
    string_t name;
    int id = 3;
    
    st << "select name from employees where id = :id", into(name), use(id);

Use binders are different classes with following interface::

    /// Use binder interface
    class use_binder
    {
    public:
        virtual ~use_binder();

        // Bind variable to statement st in position pos.
        int bind(statement& st, int pos);
    protected:
        // Protect from improper using
        use_binder();
        use_binder(use_binder const&);
        use_binder& operator=(use_binder const&);
    private:
        // Should implement real binding.
        virtual void do_bind(statement& st, int pos) = 0;
    };

    typedef std::auto_ptr<use_binder> use_binder_ptr;

    // Create position use binder for reference t.
    template<typename T>
    inline use_binder_ptr use(T& t);

    // Create named use binder for reference t.
    template<typename T>
    inline use_binder_ptr use(T& t, string_t const& name);


Data Conversion
~~~~~~~~~~~~~~~

To convert bound variable from type T into the supported SQLite column type
and vice versa a set of specialized templates is used::

    template<T>
    struct converter<T>
    {
        typedef SQLite_column_type base_type; // concrete SQLite column type


        // Convert from SQLite column type into the T.
        static T to(base_type const& b) { return b; }

        // Convert from T into the SQLite column type.
        static base_type from(T const& t) { return t; }
    };

There are all arithmetic C++ types, ``string_t`` and ``blob`` specializations exist.
The ``blob`` converter is partially specialized template for the ``std::vector<T>``
so it is possible to use ``std::vector`` in statement binders::

    db << "create table employee(id integer primary key, name text,"
                  " age integer, salary real, photo blob)";

    statement st(db);
    employee e;
    std::vector<char> image = e.photo.pixels;
    st << "insert into employee values(null, :name, :age, :salary :photo)",
                use(e.name), use(e.age), use(e.salary), use(image);
    // ...
    
    st << "select name, age, salary, photo from employee", 
                into(e.name), into(e.age), into(e.salary), into(image);


You can define convert for some custom type, if it fits to SQLite column type
(``int``, ``long long``, ``double``, ``string_t``, ``blob``). For example::

    namespace sqlitepp {
        // specialize converter for the tm struct
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
    } // namespace sqlitepp 

It is possible to convert C++ enumeration types. You should have boost_ library installed and 
``#define SQLITEPP_ENUM_CONVERTER`` macro in your project.

.. _boost: http://boost.org


Query
-----

Query is a noncopyable collection of SQL text and variable binders::

    // Query class. Noncopyable
    class query
    {
    public:
        // Create an empty query.
        query();
        
        // Create a query with SQL text.
        explicit query(string_t const& sql);
        
    
        // Clear query on destroy.
        ~query();
    
        // Current SQL statement.
        string_t sql() const; // throw()
        
        // Set a new SQL statement.
        void sql(string_t const& text);
    
        // Clear sql text, into and use bindings.
        void clear(); // throw()
        
        // Is query empty?
        bool empty() const; // throw()
    
        // Into binders container type.
        typedef std::vector<into_binder*> into_binders;
        
        // Into binders.
        into_binders const& intos() const; // throw()
    
        // Use binders container type.
        typedef std::vector<use_binder*> use_binders;
    
        // Use binders.
        use_binders const& uses() const; // throw()
    
        // Collect SQL text.
        template<typename T>
        query& operator<<(T const& t);
    
        // Add into binder.
        query& put(into_binder_ptr i);
    
        // Add into binder.
        query& operator,(into_binder_ptr i);
        
        // Add use binder.
        query& put(use_binder_ptr i);
        
        // Add use binder.
        query& operator,(use_binder_ptr u);
    
        // Swap queries.
        friend void swap(query& lhs, query& rhs);
    };
