//////////////////////////////////////////////////////////////////////////////
// $Id: string.hpp 48 2006-12-20 07:35:49Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SQLITEPP_STRING_HPP_INCLUDED
#define SQLITEPP_STRING_HPP_INCLUDED

#include <string>
//#include <vector>
//#include <iostream>

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

namespace meta {

//////////////////////////////////////////////////////////////////////////////

// Meta if
template<bool C, typename T1, typename T2>
struct if_ { typedef T1 type; };
template<typename T1, typename T2>
struct if_<false, T1, T2> { typedef T2 type; };

template<size_t Size>
struct utf_char_selector
{
	class unknown_char_type;
	typedef 
		typename if_<sizeof(wchar_t) == Size, wchar_t,
		typename if_<sizeof(unsigned short) == Size, unsigned short,
		typename if_<sizeof(unsigned int) == Size, unsigned int, 
			unknown_char_type>::type>::type>::type type;
};

//////////////////////////////////////////////////////////////////////////////

} // namespace meta

//////////////////////////////////////////////////////////////////////////////

using std::size_t;

struct blob
{
    void const* data;
    size_t size;
};

//typedef unsigned char                   utf8_char;
typedef char							utf8_char;
typedef meta::utf_char_selector<2>::type utf16_char;
typedef meta::utf_char_selector<4>::type utf32_char;

typedef std::basic_string<utf8_char>    utf8_string;
typedef std::basic_string<utf16_char>   utf16_string;
typedef std::basic_string<utf32_char>   utf32_string;

#ifdef SQLITEPP_UTF16
	typedef utf16_char   char_t;
	typedef utf16_string string_t;
#else
	typedef utf8_char    char_t;
	typedef utf8_string  string_t;
#endif // SQLITEPP_UTF16

size_t const npos = (size_t)-1;

utf8_string  utf16_to_utf8(utf16_char const* str, size_t size = npos);
utf8_string  utf32_to_utf8(utf32_char const* str, size_t size = npos);

utf16_string utf8_to_utf16(utf8_char const* str, size_t size = npos);
utf16_string utf32_to_utf16(utf32_char const* str, size_t size = npos);

utf32_string utf8_to_utf32(utf8_char const* str, size_t size = npos);
utf32_string utf16_to_utf32(utf16_char const* str, size_t size = npos);

//////////////////////////////////////////////////////////////////////////////

namespace aux {

//////////////////////////////////////////////////////////////////////////////

template<typename C, typename T1, typename T2>
struct selector;

template<typename T1, typename T2>
struct selector<utf8_char, T1, T2>
{
	selector(T1 t1, T2) : result(t1) {}
	typedef T1 type;
	type result;
};

template<typename T1, typename T2>
struct selector<utf16_char, T1, T2>
{
	selector(T1, T2 t2) : result(t2) {}
	typedef T2 type;
	type result;
};

template<typename T1, typename T2>
inline typename selector<char_t, T1, T2>::type select(T1 t1, T2 t2)
{
	return selector<char_t, T1, T2>(t1, t2).result;
}

template<typename C>
struct converter;

template<>
struct converter<utf8_char>
{
	static utf8_string utf(utf8_char const* str, size_t)
	{
		return str;
	}
	static utf8_string utf(utf16_char const* str, size_t size)
	{
		return utf16_to_utf8(str, size);
	}
	static utf8_string utf(utf32_char const* str, size_t size)
	{
		return utf32_to_utf8(str, size);
	}
};

template<>
struct converter<utf16_char>
{
	static utf16_string utf(utf8_char const* str, size_t size)
	{
		return utf8_to_utf16(str, size);
	}
	static utf16_string utf(utf16_char const* str, size_t)
	{
		return str;
	}
	static utf16_string utf(utf32_char const* str, size_t size)
	{
		return utf32_to_utf16(str, size);
	}
};

template<>
struct converter<utf32_char>
{
	static utf32_string utf(utf8_char const* str, size_t size)
	{
		return utf8_to_utf32(str, size);
	}
	static utf32_string utf(utf16_char const* str, size_t size)
	{
		return utf16_to_utf32(str, size);
	}
	static utf32_string utf(utf32_char const* str, size_t)
	{
		return str;
	}
};

//////////////////////////////////////////////////////////////////////////////

} // namespace aux

//////////////////////////////////////////////////////////////////////////////

inline utf8_string utf8(utf8_char const* str, size_t)
{
	return str;
}

inline utf8_string utf8(utf8_string const& str)
{
	return str;
}

inline utf8_string utf8(utf16_char const* str, size_t size = npos)
{
	return aux::converter<utf8_char>::utf(str, size);
}

inline utf8_string utf8(utf16_string const& str)
{
	return aux::converter<utf8_char>::utf(str.c_str(), str.size());
}

inline utf8_string utf8(utf32_char const* str, size_t size = npos)
{
	return aux::converter<utf8_char>::utf(str, size);
}

inline utf8_string utf8(utf32_string const& str)
{
	return aux::converter<utf8_char>::utf(str.c_str(), str.size());
}

inline utf16_string utf16(utf8_char const* str, size_t size = npos)
{
	return aux::converter<utf16_char>::utf(str, size);
}

inline utf16_string utf16(utf8_string const& str)
{
	return aux::converter<utf16_char>::utf(str.c_str(), str.size());
}

inline utf16_string utf16(utf16_char const* str, size_t)
{
	return str;
}

inline utf16_string utf16(utf16_string const& str)
{
	return str;
}

inline utf16_string utf16(utf32_char const* str, size_t size = npos)
{
	return aux::converter<utf16_char>::utf(str, size);
}

inline utf16_string utf16(utf32_string const& str)
{
	return aux::converter<utf16_char>::utf(str.c_str(), str.size());
}

inline utf32_string utf32(utf8_char const* str, size_t size = npos)
{
	return aux::converter<utf32_char>::utf(str, size);
}

inline utf32_string utf32(utf8_string const& str)
{
	return aux::converter<utf32_char>::utf(str.c_str(), str.size());
}

inline utf32_string utf32(utf16_char const* str, size_t size = npos)
{
	return aux::converter<utf32_char>::utf(str, size);
}

inline utf32_string utf32(utf16_string const& str)
{
	return aux::converter<utf32_char>::utf(str.c_str(), str.size());
}

inline utf32_string utf32(utf32_char const* str, size_t)
{
	return str;
}

inline utf32_string utf32(utf32_string const& str)
{
	return str;
}

inline string_t utf(utf8_char const* str, size_t size = npos)
{
	return aux::converter<char_t>::utf(str, size);
}

inline string_t utf(utf8_string const& str)
{
	return aux::converter<char_t>::utf(str.c_str(), str.size());
}

inline string_t utf(utf16_char const* str, size_t size = npos)
{
	return aux::converter<char_t>::utf(str, size);
}

inline string_t utf(utf16_string const& str)
{
	return aux::converter<char_t>::utf(str.c_str(), str.size());
}

inline string_t utf(utf32_char const* str, size_t size = npos)
{
	return aux::converter<char_t>::utf(str, size);
}

inline string_t utf(utf32_string const& str)
{
	return aux::converter<char_t>::utf(str.c_str(), str.size());
}

//////////////////////////////////////////////////////////////////////////////

} // namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////


#if defined(SQLITEPP_UTF16) && defined(SQLITEPP_TEST)
//!!! TEST only!!!
#include <iostream>
inline std::ostream& operator<<(std::ostream& os, sqlitepp::utf16_string const& str)
{
	return os << sqlitepp::utf8(str);
}
#endif

#endif // SQLITEPP_STRING_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
