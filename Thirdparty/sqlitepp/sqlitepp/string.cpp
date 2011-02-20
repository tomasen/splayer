//////////////////////////////////////////////////////////////////////////////
// $Id: string.cpp 46 2006-09-05 06:38:56Z pmed $
//
// Copyright (c) 2004 Pavel Medvedev
// Use, modification and distribution is subject to the 
// Boost Software License, Version 1.0. (See accompanying file 
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iterator>

#include "string.hpp"
#include "unicode.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////
namespace {
//////////////////////////////////////////////////////////////////////////////
template<typename T>
inline size_t str_len(T const* str)
{
	size_t len = 0;
	for ( ; str && *str; ++str )
	{
		++len;
	}
	return len;
}


//////////////////////////////////////////////////////////////////////////////
} // namespace {
//////////////////////////////////////////////////////////////////////////////

utf8_string utf16_to_utf8(utf16_char const* str, size_t size)
{
	utf8_string result;
	adobe::to_utf8(str, str + (size == npos? str_len(str) : size), std::back_inserter(result));
	return result;
}

utf8_string utf32_to_utf8(utf32_char const* str, size_t size)
{
	utf8_string result;
	adobe::to_utf8(str, str + (size == npos? str_len(str) : size), std::back_inserter(result));
	return result;
}

utf16_string utf8_to_utf16(utf8_char const* str, size_t size)
{
	utf16_string result;
	adobe::to_utf16(str, str + (size == npos? str_len(str) : size), std::back_inserter(result));
	return result;
}

utf16_string utf32_to_utf16(utf32_char const* str, size_t size)
{
	utf16_string result;
	adobe::to_utf16(str, str + (size == npos? str_len(str) : size), std::back_inserter(result));
	return result;
}

utf32_string utf8_to_utf32(utf8_char const* str, size_t size)
{
	utf32_string result;
	adobe::to_utf32(str, str + (size == npos? str_len(str) : size), std::back_inserter(result));
	return result;
}

utf32_string utf16_to_utf32(utf16_char const* str, size_t size)
{
	utf32_string result;
	adobe::to_utf32(str, str + (size == npos? str_len(str) : size), std::back_inserter(result));
	return result;
}

//////////////////////////////////////////////////////////////////////////////

} // namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////
