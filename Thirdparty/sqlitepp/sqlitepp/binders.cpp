//////////////////////////////////////////////////////////////////////////////
// $Id: binders.cpp 46 2006-09-05 06:38:56Z pmed $
//
// Copyright (c) 2004 Pavel Medvedev
// Use, modification and distribution is subject to the 
// Boost Software License, Version 1.0. (See accompanying file 
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "binders.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////
//
// into_binder
//

int into_binder::bind(statement& st, int pos)
{
	do_bind(st, pos);
	return pos + 1;
}
//----------------------------------------------------------------------------

void into_binder::update(statement& st)
{
	do_update(st);
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
//
// use_binder
//

int use_binder::bind(statement& st, int pos)
{
	do_bind(st, pos);
	return pos + 1;
}
    
//////////////////////////////////////////////////////////////////////////////

} // namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////
