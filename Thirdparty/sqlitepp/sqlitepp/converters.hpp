//////////////////////////////////////////////////////////////////////////////
// $Id: converters.hpp 64 2008-01-24 11:15:36Z pmed $
//
// Copyright (c) 2005 Pavel Medvedev
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SQLITEPP_CONVERTERS_HPP_INCLUDED
#define SQLITEPP_CONVERTERS_HPP_INCLUDED

#ifdef SQLITEPP_ENUM_CONVERTER
	#include <boost/utility/enable_if.hpp>
	#include <boost/type_traits/is_enum.hpp>
#endif

#include "string.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace sqlitepp {

//////////////////////////////////////////////////////////////////////////////

template<typename T, typename Enable = void>
struct converter;

template<typename T, typename U>
struct converter_base
{
	typedef U base_type;
	static T to(U const& u) { return static_cast<T>(u); }
	static U from(T const& t) { return static_cast<U>(t); }
};

template<> struct converter<bool> : converter_base<bool, int> {};
template<> struct converter<char> : converter_base<char, int> {};
template<> struct converter<signed char> : converter_base<signed char, int> {};
template<> struct converter<unsigned char> : converter_base<unsigned char, int> {};
template<> struct converter<wchar_t> : converter_base<wchar_t, int> {};
template<> struct converter<short> : converter_base<short, int> {};
template<> struct converter<unsigned short> : converter_base<unsigned short, int> {};
template<> struct converter<int> : converter_base<int, int> {};
template<> struct converter<unsigned int> : converter_base<unsigned int, int> {};
template<> struct converter<long> : converter_base<long, int> {};
template<> struct converter<unsigned long> : converter_base<unsigned long, int> {};
template<> struct converter<long long> : converter_base<long long, long long> {};
template<> struct converter<unsigned long long> : converter_base<unsigned long long, long long> {};
template<> struct converter<float> : converter_base<float, double> {};
template<> struct converter<double> : converter_base<double, double> {};

#ifdef SQLITEPP_ENUM_CONVERTER
template<typename T>
struct converter<T, typename boost::enable_if<boost::is_enum<T> >::type> : converter_base<T, int> {};
#endif

template<>
struct converter<string_t>
{
	typedef string_t base_type;
	static string_t const& to(string_t const& b)
	{
		return b;
	}
	static string_t const& from(string_t const& t)
	{
		return t;
	}
};

template<>
struct converter<blob>
{
	typedef blob base_type;
	static blob const& to(blob const& b)
	{
		return b;
	}
	static blob const& from(blob const& t)
	{
		return t;
	}
};

template<typename T>
struct converter<std::vector<T> >
{
	typedef blob base_type;
	static std::vector<T> to(blob const& b)
	{
		T const* f = reinterpret_cast<T const*>(b.data);
		T const* l = f + b.size / sizeof(T);
		return std::vector<T>(f, l);
	}
	static blob from(std::vector<T> const& t)
	{
		blob b;
		b.data = t.empty()? 0 : &t[0];
		b.size = t.size() * sizeof(T);
		return b;
	}
};

//////////////////////////////////////////////////////////////////////////////

} //namespace sqlitepp

//////////////////////////////////////////////////////////////////////////////

#endif // SQLITEPP_CONVERTERS_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
