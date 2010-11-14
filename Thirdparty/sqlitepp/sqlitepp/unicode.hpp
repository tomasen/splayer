/*
	Copyright 2005-2006 Adobe Systems Incorporated
	Distributed under the MIT License

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
	the Software, and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:
	
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.	
	
*/

/*************************************************************************************************/

#ifndef ADOBE_UNICODE_HPP
#define ADOBE_UNICODE_HPP

/*************************************************************************************************/

//#include <adobe/config.hpp>
//#include <adobe/algorithm.hpp>
//#include <boost/cstdint.hpp>
//#include <boost/utility/enable_if.hpp>

#include <cassert>
#include <stdexcept>

#include "string.hpp"

/*************************************************************************************************/

namespace adobe {

// boost::enable_if
template <bool B, class T = void>
struct enable_if_c { typedef T type; };
template <class T>
struct enable_if_c<false, T> {};

template <class Cond, class T = void> 
struct enable_if : public enable_if_c<Cond::value, T> {};

//using sqlitepp::utf8_char;
typedef unsigned char utf8_char;
using sqlitepp::utf16_char;
using sqlitepp::utf32_char;


/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/

template <typename T>
struct is_utf8_type
{ enum { value = sizeof(T) == 1 }; };

/*************************************************************************************************/

template <typename T>
struct is_utf16_type
{ enum { value = sizeof(T) == 2 }; };

/*************************************************************************************************/

template <typename T>
struct is_utf32_type
{ enum { value = sizeof(T) == 4 }; };

/*************************************************************************************************/

template <typename I>
struct is_utf8_iterator_type
{ enum { value = is_utf8_type<typename std::iterator_traits<I>::value_type>::value }; };

/*************************************************************************************************/

template <typename I>
struct is_utf16_iterator_type
{ enum { value = is_utf16_type<typename std::iterator_traits<I>::value_type>::value }; };

/*************************************************************************************************/

template <typename I>
struct is_utf32_iterator_type
{ enum { value = is_utf32_type<typename std::iterator_traits<I>::value_type>::value }; };

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

const utf8_char	    to_utf32_pivot_1_k(128);
const utf8_char     to_utf32_pivot_2_k(192);
const utf8_char     to_utf32_pivot_3_k(224);
const utf8_char     to_utf32_pivot_4_k(240);
const utf8_char     to_utf32_pivot_5_k(248);
const utf8_char     to_utf32_pivot_6_k(252);
const utf8_char     to_utf32_pivot_7_k(254);

const utf32_char	to_utf8_pivot_1_k(1UL << 7);
const utf32_char	to_utf8_pivot_2_k(1UL << 11);
const utf32_char	to_utf8_pivot_3_k(1UL << 16);
const utf32_char	to_utf8_pivot_4_k(1UL << 21);
const utf32_char	to_utf8_pivot_5_k(1UL << 26);

const utf16_char	to_utf16_surrogate_pivot_k(65535);
const utf16_char	utf16_high_surrogate_front_k(0xd800);
const utf16_char	utf16_high_surrogate_back_k(0xdbff);
const utf16_char	utf16_low_surrogate_front_k(0xdc00);
const utf16_char	utf16_low_surrogate_back_k(0xdfff);

/*************************************************************************************************/

template <std::size_t NumBytes> struct utf8_header_t	{ };
template <>						struct utf8_header_t<0>	{ enum { value = char(0x80) }; }; // nonheader
//template <>					struct utf8_header_t<1>	{ enum { value = char(0x00) }; }; // illegal
template <>						struct utf8_header_t<2>	{ enum { value = char(0xC0) }; };
template <>						struct utf8_header_t<3>	{ enum { value = char(0xE0) }; };
template <>						struct utf8_header_t<4>	{ enum { value = char(0xF0) }; };
template <>						struct utf8_header_t<5>	{ enum { value = char(0xF8) }; };
template <>						struct utf8_header_t<6>	{ enum { value = char(0xFC) }; };

/*************************************************************************************************/

template <char Mask, typename BinaryInteger>
inline char add_mask(BinaryInteger code)
{ return char(code | Mask); }

template <std::size_t NumBytes, bool Header, typename BinaryInteger>
inline char utf8_add_mask(BinaryInteger code)
{ return add_mask<utf8_header_t<Header ? NumBytes : 0>::value>(code); }

/*************************************************************************************************/

template<char Mask, typename BinaryInteger>
inline char strip_mask(BinaryInteger code)
{ return char(code & ~Mask); }

template <std::size_t NumBytes, bool Header, typename BinaryInteger>
inline char utf8_strip_mask(BinaryInteger code)
{ return strip_mask<utf8_header_t<Header ? NumBytes : 0>::value>(code); }

/*************************************************************************************************/

template <std::size_t Position>
inline utf32_char promote_fragment(char fragment)
{ return utf32_char(fragment << ((Position - 1) * 6)); }

template <>
inline utf32_char promote_fragment<1>(char fragment)
{ return utf32_char(fragment); }

template <>
inline utf32_char promote_fragment<0>(char); // unimplemented

/*************************************************************************************************/

template <std::size_t Position>
inline char demote_fragment(utf32_char fragment)
{ return char((fragment >> ((Position - 1) * 6)) & 0x0000003F); }

template <>
inline char demote_fragment<1>(utf32_char fragment)
{ return char(fragment & 0x0000003F); }

template <>
inline char demote_fragment<0>(utf32_char); // unimplemented

/*************************************************************************************************/

template <std::size_t ByteCount, bool Header = true>
struct demotion_engine_t
{
	template <typename OutputIterator>
	inline OutputIterator operator () (utf32_char code, OutputIterator i)
	{
		*i = utf8_add_mask<ByteCount, Header>(demote_fragment<ByteCount>(code));
		return demotion_engine_t<ByteCount - 1, false>()(code, ++i);
	}
};

template <>
struct demotion_engine_t<1, false>
{
	template <typename OutputIterator>
	inline OutputIterator operator () (utf32_char code, OutputIterator i)
	{
		*i = utf8_add_mask<0, false>(demote_fragment<1>(code));
		return ++i;
	}
};

/*************************************************************************************************/

template <std::size_t ByteCount, bool Header = true>
struct promotion_engine_t
{
	template <typename InputIterator>
	inline utf32_char operator () (InputIterator& first, InputIterator last)
	{
		/*
			CodeWarrior 9.4 doesn't like this code composited into one line;
			GCC doesn't seem to have a problem.
		*/

		char			n(*first);
		char			stripped(utf8_strip_mask<ByteCount, Header>(n));
		utf32_char	shifted(promote_fragment<ByteCount>(stripped));

		++first;

		if (first == last) throw std::runtime_error("unicode: utf32 conversion ran out of input");

		return shifted | promotion_engine_t<ByteCount - 1, false>()(first, last);
	}
};

template <>
struct promotion_engine_t<1, false>
{
	template <typename InputIterator>
	inline utf32_char operator () (InputIterator& first, InputIterator)
	{
		utf32_char result(promote_fragment<1>(utf8_strip_mask<0, false>(*first)));

		++first;

		return result;
	}
};

/*************************************************************************************************/

template <typename InputIterator, typename DestInteger>
inline typename enable_if<is_utf16_iterator_type<InputIterator>, InputIterator>::type
	to_utf32 (InputIterator first, InputIterator last, DestInteger& result)
{
	utf16_char code(static_cast<utf16_char>(*first));

	++first;

	if (code >= implementation::utf16_high_surrogate_front_k &&
		code <= implementation::utf16_high_surrogate_back_k)
	{
		result = 0;

		if (first == last)
			throw std::runtime_error("unicode: utf16 high surrogate found without low surrogate"); 

		utf16_char low(static_cast<utf16_char>(*first));

		assert (low >= implementation::utf16_low_surrogate_front_k &&
				low <= implementation::utf16_low_surrogate_back_k);

		++first;

		result =	(code - implementation::utf16_high_surrogate_front_k) * 0x400 +
					(low - implementation::utf16_low_surrogate_front_k) + 0x10000;
	}
	else if (	code >= implementation::utf16_low_surrogate_front_k &&
				code <= implementation::utf16_low_surrogate_back_k)
		{ throw std::runtime_error("unicode: utf16 low surrogate found without high surrogate"); }
	else
		{ result = static_cast<DestInteger>(code); }

	return first;
}

/*************************************************************************************************/

template <typename InputIterator, typename DestInteger>
inline typename enable_if<is_utf8_iterator_type<InputIterator>, InputIterator>::type
	to_utf32 (InputIterator first, InputIterator last, DestInteger& result)
{
	unsigned char n(static_cast<unsigned char>(*first));

	if (n < implementation::to_utf32_pivot_1_k)
		{ result = static_cast<DestInteger>(n); ++first; }
	else if (n < implementation::to_utf32_pivot_2_k)
		{ throw std::runtime_error("unicode: ill-defined utf8 (< 192)"); }
	else if (n < implementation::to_utf32_pivot_3_k)
		result = implementation::promotion_engine_t<2>()(first, last);
	else if (n < implementation::to_utf32_pivot_4_k)
		result = implementation::promotion_engine_t<3>()(first, last);
	else if (n < implementation::to_utf32_pivot_5_k)
		result = implementation::promotion_engine_t<4>()(first, last);
	else if (n < implementation::to_utf32_pivot_6_k)
		result = implementation::promotion_engine_t<5>()(first, last);
	else if (n < implementation::to_utf32_pivot_7_k)
		result = implementation::promotion_engine_t<6>()(first, last);
	else
		{ throw std::runtime_error("unicode: ill-defined utf8 (>= 254)"); }

	return first;
}

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

#endif

/*************************************************************************************************/
/*
		utf32 -> utf8
			- 1 source value
			- n output values
*/

template <	typename I, // I models Integer; I must be a valid UTF32-encoded code point
			typename O>	// O models OutputIterator
inline typename enable_if<is_utf32_type<I>, O>::type
	to_utf8(I code, O output)
{
	if (code < implementation::to_utf8_pivot_1_k) // UTF-8 is 1 byte long
		{ *output = static_cast<char>(code); ++output; }
	else if (code < implementation::to_utf8_pivot_2_k) // UTF-8 is 2 bytes long
		output = implementation::demotion_engine_t<2>()(code, output);
	else if (code < implementation::to_utf8_pivot_3_k) // UTF-8 is 3 bytes long
		output = implementation::demotion_engine_t<3>()(code, output);
	else if (code < implementation::to_utf8_pivot_4_k) // UTF-8 is 4 bytes long
		output = implementation::demotion_engine_t<4>()(code, output);
	else if (code < implementation::to_utf8_pivot_5_k) // UTF-8 is 5 bytes long
		output = implementation::demotion_engine_t<5>()(code, output);
	else // UTF-8 is 6 bytes long
		output = implementation::demotion_engine_t<6>()(code, output);

	return output;
}

/*************************************************************************************************/
/*
		utf16 -> utf8
			- 1 source value
			- n output values
*/

template <	typename I, // I models Integer; I must be a valid UTF32-encoded code point
			typename O>	// O models OutputIterator
inline typename enable_if<is_utf16_type<I>, O>::type
	to_utf8(I code, O output)
{
	return to_utf8(static_cast<utf32_char>(code), output);
}

/*************************************************************************************************/
/*
		utf16 -> utf8
			- n source values
			- m output values
*/

template <	typename I, // I models InputIterator
			typename O> // O models OutputIterator
inline typename enable_if<is_utf16_iterator_type<I>, O>::type
	to_utf8(I first, I last, O output)
{
	while (first != last)
	{
		utf32_char	result;

		first = implementation::to_utf32(first, last, result);

		output = to_utf8(result, output);
	}
	return output;
}

/*************************************************************************************************/
/*
		utf32 -> utf8
			- n source values
			- m output values
*/

template <	typename I, // I models InputIterator
			typename O> // O models OutputIterator
inline typename enable_if<is_utf32_iterator_type<I>, O>::type
	to_utf8(I first, I last, O output)
{
    while ( first != last )
    {
        output = to_utf8(*first, output);
        ++first;
    }
	return output;
}

/*************************************************************************************************/
/*
		utf32 -> utf16
			- 1 source value
			- n output values
*/

template <	typename I, // I models Integer; sizeof(I) must equal 4; code must be valid utf32
			typename O>	// O models OutputIterator
inline typename enable_if<is_utf32_type<I>, O>::type
	to_utf16(I code, O output)
{
	if (code <= implementation::to_utf16_surrogate_pivot_k)
	{
		*output = static_cast<utf16_char>(code);
	}
	else
	{
		*output = static_cast<utf16_char>((code - 0x10000) / 0x400 + implementation::utf16_high_surrogate_front_k);

		++output;

		*output = static_cast<utf16_char>((code - 0x10000) % 0x400 + implementation::utf16_low_surrogate_front_k);
	}
	return ++output;
}

/*************************************************************************************************/
/*
		utf8 -> utf16
			- n source values
			- m output values
*/
template <	typename I, // I models InputIterator
			typename O> // O models OutputIterator
inline typename enable_if<is_utf8_iterator_type<I>, O>::type
	to_utf16(I first, I last, O output)
{
	while (first != last)
	{
		utf32_char	result;

		first = implementation::to_utf32(first, last, result);

		output = to_utf16(result, output);
	}
	return output;
}

/*************************************************************************************************/
/*
		utf32 -> utf16
			- n source values
			- m output values
*/
template <	typename I, // I models InputIterator
			typename O> // O models OutputIterator
inline typename enable_if<is_utf32_iterator_type<I>, O>::type
	to_utf16(I first, I last, O output)
{
	while (first != last)
	{
		output = to_utf16(*first, output);
		++first;
	}
	return output;
}

/*************************************************************************************************/
/*
	Precondition: [ first, last ) must convert to exactly one UTF-16 character
*/

template <typename I>
inline typename enable_if<is_utf8_iterator_type<I>, utf16_char>::type
	to_utf16(I first, I last)
{
	utf32_char result;

	implementation::to_utf32(first, last, result);
	return static_cast<utf16_char>(result);
}

/*************************************************************************************************/
/*
		utf16 -> utf32
			- n source values
			- m output values

		utf8 -> utf32
			- n source values
			- m output values
*/

template <	typename I, // I models InputIterator
			typename O>	// O models OutputIterator
inline O to_utf32(I first, I last, O output)
{
	utf32_char result;

	while (first != last)
	{
		first = implementation::to_utf32(first, last, result);

		*output = result;

		++output;
	}
	return output;
}

/*************************************************************************************************/
/*
	Precondition: [ first, last ) must convert to exactly one UTF-32 character
*/

template <typename I> // I models InputIterator
inline utf32_char to_utf32(I first, I last)
{
	utf32_char result;

	implementation::to_utf32(first, last, result);
	return result;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif
	
/*************************************************************************************************/
