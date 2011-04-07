// ***************************************************************
//  utils   version:  1.0   ? date: 10/28/2006
//  -------------------------------------------------------------
//  -------------------------------------------------------------
//  Copyright (C) 2006 - All Rights Reserved
// ***************************************************************

//this library is from foo-display-lyrics 
// http://code.google.com/p/foo-display-lyrics/


#if !defined(_DEFINED_e391a3a0_61b4_4224_99c8_3d1c526e0823)
#define _DEFINED_e391a3a0_61b4_4224_99c8_3d1c526e0823
#if _MSC_VER > 1000 
    #pragma once  
#endif // _MSC_VER > 1000


#ifdef _WIN32


#include <windows.h>
#else
#include <stdlib.h>
#endif
#include <tchar.h>
#include <assert.h>
#include <process.h>

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#include <vector>
#include <list>
#include <string>
#include <algorithm>

using std::vector;
using std::string;
using std::wstring;


//////////////////////////// Assert/Verify Macros /////////////////////////////

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef VERIFY
#undef VERIFY
#endif

#ifdef _DEBUG
// Put up a message box if an assertion fails in a debug build.
//#define ASSERT(x) if (!(x)) ASSERTFAIL(__FILE__, __LINE__, #x)
#define ASSERT(x) assert(x)
// Assert in debug builds, but don't remove the code in retail builds.
#define VERIFY(x) ASSERT(x)
#define verify(x) ASSERT(x)
#else
#define ASSERT(x) ((void)0)
#define VERIFY(x) (x)
#define verify(x) (x)
#endif

////////////////////////// End of Assert/Verify Macros ///////////////////////////

#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

#define countof(x) (sizeof(x)/sizeof(x[0]))
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))

#define __W(x) L ## x
#define _W(x) __W(x)

#define _TO_STRING(x) #x
#define TO_STRING(x) _TO_STRING(x)
#define TO_WSTRING(x) _W(TO_STRING(x))
#define TO_TSTRING(x) _T(TO_STRING(x))

//typedef
typedef __int64 int64;
typedef unsigned __int64 uint64;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef char sbyte;
typedef TCHAR tchar;
typedef WCHAR wchar;
typedef char * pchar;
typedef wchar * pwchar;
typedef tchar * ptchar;
typedef std::string::size_type size_type;
#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif


//=========================================================================
//					Utilitys namespace
//=========================================================================
namespace util {


namespace CONST_VALUE {

const size_t MAX_ALLOC_SIZE = 0xFFFFFF;

};


// 空类
class null_type;

namespace inner {
// 编译时断言，如果编译时出错行在这里，请检查调用的条件是否满足
template<bool> struct compile_time_assert_fail;
template<> struct compile_time_assert_fail<true> {};
};
#ifdef _DEBUG
#define STATIC_ASSERT(exp) (util::inner::compile_time_assert_fail<(exp) != 0>())
#else
#define STATIC_ASSERT(exp) ((void)0)
#endif


// 禁止副本构造函数的基类
class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:  // emphasize the following members are private
	noncopyable(const noncopyable &);
	const noncopyable & operator=(const noncopyable &);
};
// 在要禁止产生副本构造函数的类内部使用 NONCOPYABLE(类名) 来继承该类
#define NONCOPYABLE(CLASS) private: CLASS(const CLASS &); const CLASS & operator=(const CLASS &)

//type_list
namespace type_lists {

template<typename T, typename U> struct type_list { typedef T head; typedef U tail; };

#define TYPELIST_1(T1) util::type_list<T1, util::null_type>
#define TYPELIST_2(T1, T2) util::type_list<T1, TYPELIST_1(T2) >
#define TYPELIST_3(T1, T2, T3) util::type_list<T1, TYPELIST_2(T2, T3) >
#define TYPELIST_4(T1, T2, T3, T4) util::type_list<T1, TYPELIST_3(T2, T3, T4) >
#define TYPELIST_5(T1, T2, T3, T4, T5) util::type_list<T1, TYPELIST_4(T2, T3, T4, T5) >
#define TYPELIST_6(T1, T2, T3, T4, T5, T6) util::type_list<T1, TYPELIST_5(T2, T3, T4, T5, T6) >
#define TYPELIST_7(T1, T2, T3, T4, T5, T6, T7) util::type_list<T1, TYPELIST_6(T2, T3, T4, T5, T6, T7) >
#define TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8) util::type_list<T1, TYPELIST_7(T2, T3, T4, T5, T6, T7, T8) >
#define TYPELIST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) util::type_list<T1, TYPELIST_8(T2, T3, T4, T5, T6, T7, T8, T9) >
#define TYPELIST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) util::type_list<T1, TYPELIST_9(T2, T3, T4, T5, T6, T7, T8, T9, T10) >
#define TYPELIST_11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) util::type_list<T1, TYPELIST_10(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) >
#define TYPELIST_12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) util::type_list<T1, TYPELIST_11(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) >
#define TYPELIST_13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) util::type_list<T1, TYPELIST_12(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) >
#define TYPELIST_14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) util::type_list<T1, TYPELIST_13(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) >
#define TYPELIST_15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) util::type_list<T1, TYPELIST_14(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) >
#define TYPELIST_16(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) util::type_list<T1, TYPELIST_15(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) >
#define TYPELIST_17(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17) util::type_list<T1, TYPELIST_16(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17) >
#define TYPELIST_18(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18) util::type_list<T1, TYPELIST_17(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18) >
#define TYPELIST_19(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19) util::type_list<T1, TYPELIST_18(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19) >
#define TYPELIST_20(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20) util::type_list<T1, TYPELIST_19(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20) >

template<typename TL> struct type_list_length;
template<> struct type_list_length<null_type> { enum { value = 0 }; };
template<typename T, typename U> struct type_list_length< type_list<T, U> > { enum { value = 1 + type_list_length<U>::value }; };

template<typename TL, typename T> struct type_list_find { enum { value = -1 }; };
template<> struct type_list_find<null_type, null_type> { enum { value = 0 }; };
template<typename T> struct type_list_find<null_type, T> { enum { value = -1 }; };
template<typename T> struct type_list_find<T, T> { enum { value = 0 }; };
template<typename U, typename T> struct type_list_find<type_list<T, U>, T> { enum { value = 0 }; };
template<typename U, typename V, typename T> struct type_list_find<type_list<U, V>, T>
{
private: enum { temp = type_list_find<V, T>::value };
public: enum { value = -1 == temp ? -1 : temp + 1 };
};

}; //namespace type_lists
using namespace type_lists;

//type trait
namespace type_trait {

typedef char small_type;
struct big_type { char summy[2]; };

};

namespace inner {

template<typename T>
type_trait::small_type type_trait_test(T);

template<typename T>
type_trait::big_type type_trait_test(...);

};

//type trait
namespace type_trait {


//can_convert<short, long>::value == 1, short can auto convert to long
//can_convert<long, short>::value == 1, long can auto convert to short but may loss data
//can_convert<short *, long>::value == 0, short * do not auto convert to long, unless using reinterrpt_cast<>
template<typename SRC, typename DEST>
class can_convert
{
public:
	enum { value = sizeof(inner::type_trait_test<DEST>(*static_cast<SRC *>(0))) == sizeof(small_type) };
};

//can_convert<short, long>::value == 1, short can safe convert to long
//can_convert<long, short>::value == 0, long can not safe convert to short
//can_convert<short *, long>::value == 0, short * do not auto convert to long, unless using reinterrpt_cast<>
template<typename SRC, typename DEST>
struct can_safe_convert
{
	enum { value = sizeof(SRC) <= sizeof(DEST) && can_convert<SRC, DEST>::value };
};



template<int T> struct int2type { enum { value = T }; };
template<typename T> struct type2type { typedef T type; };

template<typename T> struct is_bool { enum { value = false }; };
template<> struct is_bool<bool> { enum { value = true }; };

template<typename T> struct is_char { enum { value = false }; };
template<> struct is_char<char> { enum { value = true }; };

template<typename T> struct is_uchar { enum { value = false }; };
template<> struct is_uchar<unsigned char> { enum { value = true }; };

template<typename T> struct is_wchar { enum { value = false }; };
template<> struct is_wchar<wchar> { enum { value = true }; };

template<typename T> struct is_short { enum { value = false }; };
template<> struct is_short<short> { enum { value = true }; };

template<typename T> struct is_ushort { enum { value = false }; };
template<> struct is_ushort<unsigned short> { enum { value = true }; };

template<typename T> struct is_int { enum { value = false }; };
template<> struct is_int<int> { enum { value = true }; };

template<typename T> struct is_uint { enum { value = false }; };
template<> struct is_uint<unsigned int> { enum { value = true }; };

template<typename T> struct is_long { enum { value = false }; };
template<> struct is_long<long> { enum { value = true }; };

template<typename T> struct is_ulong { enum { value = false }; };
template<> struct is_ulong<unsigned long> { enum { value = true }; };

template<typename T> struct is_int64 { enum { value = false }; };
template<> struct is_int64<int64> { enum { value = true }; };

template<typename T> struct is_uint64 { enum { value = false }; };
template<> struct is_uint64<uint64> { enum { value = true }; };

template<typename T> struct is_float { enum { value = false }; };
template<> struct is_float<float> { enum { value = true }; };

template<typename T> struct is_double { enum { value = false }; };
template<> struct is_double<double> { enum { value = true }; };

template<typename T> struct is_std_sints { enum { value = is_char<T>::value || is_short<T>::value || is_int<T>::value || is_long<T>::value }; };
template<typename T> struct is_std_uints { enum { value = is_uchar<T>::value || is_wchar<T>::value || is_ushort<T>::value || is_uint<T>::value || is_ulong<T>::value }; };
template<typename T> struct is_std_ints { enum { value = is_std_sints<T>::value || is_std_uints<T>::value }; };

template<typename T>
class is_pointer
{
private:
	template<typename U> struct impl { enum { value = false }; typedef U type; };
	template<typename U> struct impl<U *> { enum { value = true }; typedef U type; };
public:
	enum { value = impl<T>::value };
	typedef typename impl<T>::type type;
};

// 判断 T 类型是否是类成员函数指针, 支持至多 10 个函数参数 (包括带 const 签名的函数, 不包括带 volatile 及 ... 签名的函数)
// class A { public: void f(); };
// is_member_function_pointer<&A::f>::value == true
// is_member_function_pointer<char *>::value == true
template<typename T>
class is_member_function_pointer
{
private:
	template<typename U> struct impl { enum { value = false }; typedef U type; };
	template<typename R, typename U> struct impl<R (U::*)()> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1> struct impl<R (U::*)(P1)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2> struct impl<R (U::*)(P1, P2)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3> struct impl<R (U::*)(P1, P2, P3)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4> struct impl<R (U::*)(P1, P2, P3, P4)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5> struct impl<R (U::*)(P1, P2, P3, P4, P5)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6, P7)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6, P7, P8)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6, P7, P8, P9)> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10)> { enum { value = true }; typedef U type; };
	template<typename R, typename U> struct impl<R (U::*)() const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1> struct impl<R (U::*)(P1) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2> struct impl<R (U::*)(P1, P2) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3> struct impl<R (U::*)(P1, P2, P3) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4> struct impl<R (U::*)(P1, P2, P3, P4) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5> struct impl<R (U::*)(P1, P2, P3, P4, P5) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6, P7) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6, P7, P8) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6, P7, P8, P9) const> { enum { value = true }; typedef U type; };
	template<typename R, typename U, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10> struct impl<R (U::*)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) const> { enum { value = true }; typedef U type; };
public:
	enum { value = impl<T>::value };
	typedef typename impl<T>::type type;
};

template<typename T>
class is_const
{
private:
	template<typename U> struct impl { enum { value = false }; typedef U type; };
	template<typename U> struct impl<const U> { enum { value = true }; typedef U type; };
public:
	enum { value = impl<T>::value };
	typedef typename impl<T>::type type;
};


}; //namespace type_trait
using namespace type_trait;



template<typename T, typename V>
inline T & assign_cast(T & var, const V & value) { var = static_cast<T>(value); return var; }

template<typename T>
inline bool & assign_cast(bool & var, const T & value) { var = value != static_cast<T>(0); return var; }

template<typename T>
inline const T & std_min(const T & a, const T & b) { return a > b ? b : a; }

template<typename T>
inline const T & std_max(const T & a, const T & b) { return a > b ? a : b; }

template<typename T>
inline const T & range(const T & Val, const T & MinVal, const T & MaxVal)
{ return Val < MinVal ? MinVal : (Val > MaxVal ? MaxVal : Val); }

template<typename T>
inline bool tobool(const T & val) { return 0 != val; }


//************************************
// Method:    length
// FullName:  util::length
// Access:    public const 
// Returns:   size_type
// Qualifier: 
// Parameter: elem can be std::string, std::wstring, char, wchar_t, char *, wchar_t *
//************************************

template<typename T>
inline size_type length(const T & elem) { return elem.length(); }
template<> inline size_type length<std::string>(const std::string & elem) { return elem.length(); }
template<> inline size_type length<std::wstring>(const std::wstring & elem) { return elem.length(); }
inline size_type length(const char elem) { return 1; }
inline size_type length(const wchar elem) { return 1; }
inline size_type length(const char * elem) { return strlen(elem); }
inline size_type length(const wchar * elem) { return wcslen(elem); }


//************************************
// Method:    is_number 判断 ch 是否是数字
// FullName:  util::is_number
// Access:    public const 
// Returns:   bool
// Qualifier: 
// Parameter: const T & ch
//************************************

template<typename T>
inline bool is_number(const T & ch) { return ch >= '0' && ch <= '9'; }


//************************************
// Method:    is_hex_number 判断 ch 是否是十六进制数字
// FullName:  util::is_hex_number
// Access:    public const 
// Returns:   bool
// Qualifier: 
// Parameter: const T & ch
//************************************

template<typename T>
inline bool is_hex_number(const T & ch) { return is_number(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'); }


//************************************
// Method:    is_wrap_char 判断是否是可以换行的字符
// FullName:  util::is_wrap_char
// Access:    public 
// Returns:   bool
// Qualifier: 
// Parameter: wchar cur_char 当前的字符
// Parameter: wchar next_char 下一个字符
//************************************

inline bool is_wrap_char(wchar cur_char, wchar next_char)
{
	return 0 == next_char || '\r' == cur_char || '\n' == cur_char || ' ' == cur_char || '\t' == cur_char ||
		',' == cur_char || ';' == cur_char || '?' == cur_char || '!' == cur_char || '.' == cur_char || ':' == cur_char
		; //||!(cur_char < 0x80 && next_char < 0x80);
}



namespace inner {

// T == std_int
template<typename T>
inline T & char_lower(T & ch) { return (ch >= 'A' && ch <= 'Z') ? ch += 'a' - 'A' : ch; }

// T == std_int
template<typename T>
inline T & tolower_impl(T & ch, int2type<true> /*summy*/) { return char_lower(ch); }

// T == std::string || std::wstring || std_int *
template<typename T>
inline T & tolower_impl(T & str, int2type<false> /*summy*/)
{ return tolower_ptr_impl(str, int2type<is_pointer<T>::value && is_std_ints<typename is_pointer<T>::type>::value>()); }

// T == pionter
template<typename T>
inline T & tolower_ptr_impl(T & str, int2type<true> /*summy*/)
{ T ptr = str; while (*ptr) char_lower(*ptr++); return str; }

// T == std::string || std::wstring
template<typename T>
inline T & tolower_ptr_impl(T & str, int2type<false> /*summy*/)
{ std::for_each(str.begin(), str.end(), char_lower<T::traits_type::char_type>); return str; }

// T == std_int
template<typename T>
inline T & char_upper(T & ch) { return (ch >= 'a' && ch <= 'z') ? ch -= 'a' - 'A' : ch; }

// T == std_int
template<typename T>
inline T & toupper_impl(T & ch, int2type<true> /*summy*/) { return char_upper(ch); }

// T == std::string || std::wstring || std_int *
template<typename T>
inline T & toupper_impl(T & str, int2type<false> /*summy*/)
{ return toupper_ptr_impl(str, int2type<is_pointer<T>::value && is_std_ints<typename is_pointer<T>::type>::value>()); }

// T == pionter
template<typename T>
inline T & toupper_ptr_impl(T & str, int2type<true> /*summy*/)
{ T ptr = str; while (*ptr) char_upper(*ptr++); return str; }

// T == std::string || std::wstring
template<typename T>
inline T & toupper_ptr_impl(T & str, int2type<false> /*summy*/)
{ std::for_each(str.begin(), str.end(), char_upper<T::traits_type::char_type>); return str; }

///////// for const obj //////////

// T == std_int
template<typename T>
inline T tolower_impl(const T & ch, int2type<true> /*summy*/) { return (ch >= 'A' && ch <= 'Z') ? ch + 'a' - 'A' : ch; }

// T == std::string || std::wstring
template<typename T>
inline T tolower_impl(const T & str, int2type<false> /*summy*/)
{ T new_str(str); std::for_each(new_str.begin(), new_str.end(), char_lower<T::traits_type::char_type>); return new_str; }

// T == std_int
template<typename T>
inline T toupper_impl(const T & ch, int2type<true> /*summy*/) { return (ch >= 'a' && ch <= 'z') ? ch - ('a' - 'A') : ch; }

// T == std::string || std::wstring
template<typename T>
inline T toupper_impl(const T & str, int2type<false> /*summy*/)
{ T new_str(str); std::for_each(new_str.begin(), new_str.end(), char_upper<T::traits_type::char_type>); return new_str; }

}; //namespace inner


//************************************
// Method:    tolower 把字符串转为小写
// FullName:  util::tolower
// Access:    public const 
// Returns:   template<typename T>  T &
// Qualifier: 
// Parameter: T & obj 可以为 char, wchar_t, int, std::string, std:wstring, char *, wchar_t * 等及带 const 修饰符
//************************************

template<typename T>
inline T & tolower(T & obj) { return inner::tolower_impl(obj, int2type<is_std_ints<T>::value>()); }

template<typename T>
inline T & toupper(T & obj) { return inner::toupper_impl(obj, int2type<is_std_ints<T>::value>()); }

template<typename T>
inline T tolower(const T & obj) { return inner::tolower_impl(obj, int2type<is_std_ints<T>::value>()); }

template<typename T>
inline T toupper(const T & obj) { return inner::toupper_impl(obj, int2type<is_std_ints<T>::value>()); }

template<typename T>
inline const T & equal_alter(const T & val, const T & val_test, const T & alter)
{ return val == val_test ? alter : val; }

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T get_before(const T & in, const E & elem)
{ return in.substr(0, util::min(in.length(), in.find(elem))); }

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T & make_before(T & io, const E & elem)
{ return io.erase(util::min(io.find(elem), io.length())); }

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T get_after(const T & in, const E & elem)
{
	return in.substr(equal_alter(util::min(in.length(), in.find(elem) + length(elem)), T::npos, 0U));
}

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T & make_after(T & io, const E & elem)
{
	return io.erase(0, equal_alter(util::min(io.length(), io.find(elem) + length(elem)),
		T::npos, 0U));
}

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T rget_before(const T & in, const E & elem)
{ return in.substr(0, util::min(in.length(), in.rfind(elem))); }

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T & rmake_before(T & io, const E & elem)
{ return io.erase(util::min(io.rfind(elem), io.length())); }

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T rget_after(const T & in, const E & elem)
{
	return in.substr(equal_alter(min(in.length(), in.rfind(elem) + length(elem)),
		T::npos, 0U));
}

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T & rmake_after(T & io, const E & elem)
{
	return io.erase(0, equal_alter(util::min(io.length(), io.rfind(elem) + length(elem)),
		T::npos, 0U));
}

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T & trim_right(T & io, const E & elem)
{ return io.erase(util::std_min(io.find_last_not_of(elem) + 1, io.length())); }

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T & trim_left(T & io, const E & elem)
{ return io.erase(0, io.find_first_not_of(elem)); }

// (T == std::string && (E == char || char * || std::string))
// || (T == std::wstring && (E == wchar_t || wchar_t * || std::wstring))
template<typename T, typename E>
inline T & trim(T & io, const E & elem)
{ return trim_left(trim_right(io, elem), elem); }

namespace inner {
template<typename T, typename E>
inline T & insert(T & in, string::size_type pos, const E & elem) { return in.insert(pos, elem); }
template<typename T>
inline T & insert(T & in, string::size_type pos, const char & elem) { return in.insert(pos, 1, elem); }
};

// (T == std::string && (E1 == char * || std::string) && (E2 == char * || std::string))
// || (T == std::wstring && (E1 == wchar_t * || std::wstring) && (E2 == wchar_t * || std::wstring))
template<typename T, typename E1, typename E2>
inline T & replace(T & io, const E1 & to_find, const E2 & to_replace, string::size_type pos = 0)
{
    string::size_type srclen = length(to_find);
    string::size_type dstlen = length(to_replace);
    while( (pos = io.find(to_find, pos)) != string::npos )
	{
        io.erase(pos, srclen);
		inner::insert(io, pos, to_replace);
        pos += dstlen;
    }
	return io;
}

// (T == std::string && (E1 == char * || std::string) && (E2 == char * || std::string))
// || (T == std::wstring && (E1 == wchar_t * || std::wstring) && (E2 == wchar_t * || std::wstring))
template<typename T, typename E1, typename E2>
inline T & replace_once(T & io, const E1 & to_find, const E2 & to_replace, string::size_type & pos)
{
    if ( (pos = io.find(to_find, pos)) != T::npos )
	{
        io.erase(pos, length(to_find));
        inner::insert(io, pos, to_replace);
        pos += length(to_replace);
    }
	else
		pos = io.length();
	return io;
}

// (T == std::string && (E1 == char * || std::string) && (E2 == char * || std::string))
// || (T == std::wstring && (E1 == wchar_t * || std::wstring) && (E2 == wchar_t * || std::wstring))
template<typename T, typename E1, typename E2>
inline T & replace_once(T & io, const E1 & to_find, const E2 & to_replace)
{
	string::size_type pos;
    if ( (pos = io.find(to_find, 0)) != T::npos )
	{
        io.erase(pos, length(to_find));
        inner::insert(io, pos, to_replace);
    }
	return io;
}

template<typename T, typename Iter>
inline T & get_line(T & str_out, Iter & iter)
{
	Iter start = iter;

	while (*iter && '\r' != *iter && '\n' != *iter)
		++iter;

	if (*iter)
	{
		Iter old = iter;
		if (('\r' == *++iter || '\n' == *iter) && *old != *iter)
			++iter;
	}

	return str_out.assign(start, iter);
}

//template<typename T, typename F>
//class auto_func
//{
//public:
//	auto_func(F FunctionPionter) { f = FunctionPionter; }
//	~auto_func() { std::for_each(data.begin(), data.end(), f); }
//	inline const T & add(const T & Item) { data.push_back(Item); return Item; }
//private:
//	vector<T> data;
//	F f;
//};

template<typename T, typename U>
class auto_func
{
	NONCOPYABLE(auto_func);

public:
	auto_func(const T & t, U * pf) : xm_t(t), xm_pf(pf) {}
	~auto_func() { xm_pf(xm_t); }

	inline T & operator=(const T & t) { return (xm_t = t); }
	inline operator T() const { return xm_t; }

private:
	T xm_t;
	U * xm_pf;
};

template<typename T>
class smart_ptr
{
public:
	typedef T type;
	typedef T * pointer;

public:
	explicit smart_ptr(T * p) : ptr(p), p_ref_count(new uint(0)) {}

	~smart_ptr()
	{
		if (0 == *p_ref_count)
		{
			delete p_ref_count;
			delete ptr;
		}
		else
			--*p_ref_count;
	}

	smart_ptr(const smart_ptr & rhs) : ptr(rhs.ptr), p_ref_count(rhs.p_ref_count)
	{
		++*p_ref_count;
	}

	inline smart_ptr & operator=(T * p) { ~smart_ptr(); ptr = p; p_ref_count = new uint(0); return *this; }
	inline smart_ptr & operator=(const smart_ptr & rhs) { if (this != &rhs) { ptr = rhs.ptr; p_ref_count = rhs.p_ref_count; ++*p_ref_count; } return *this; }

	inline T * get() const { return ptr; }
	inline T & operator*() const { ASSERT(ptr); return *ptr; }
	inline T * operator->() const { ASSERT(ptr); return get(); }

	inline bool operator<(const smart_ptr & rhs) { return ptr->data < rhs.ptr->data; }
	inline bool operator<=(const smart_ptr & rhs) { return ptr->data <= rhs.ptr->data; }
	inline bool operator>(const smart_ptr & rhs) { return ptr->data > rhs.ptr->data; }
	inline bool operator>=(const smart_ptr & rhs) { return ptr->data >= rhs.ptr->data; }
	inline bool operator==(const smart_ptr & rhs) { return ptr->data == rhs.ptr->data; }
	inline bool operator!=(const smart_ptr & rhs) { return ptr->data != rhs.ptr->data; }

#ifdef _DEBUG
	//only for DEBUG!
	inline uint get_ref_count() const { return *p_ref_count; }
#endif

private:
	T * ptr;
	uint * p_ref_count;
};

template<typename T>
class bind_ptr
{
public:
	typedef T type;
	typedef T * pointer;

	struct data_t
	{
		T data;
		uint ref_count;
	};

	// new 一个内部数据结构
	static T * create() { data_t * p = new data_t; p->ref_count = 0; return &p->data; }

public:
	bind_ptr() : ptr(new data_t) { ptr->ref_count = 1; }
	~bind_ptr() { x_release(); }
	explicit bind_ptr(data_t * p) : ptr(p) { ++(ptr->ref_count); }
	bind_ptr(const bind_ptr & rhs) : ptr(rhs.ptr) { ++(ptr->ref_count); }

	inline bind_ptr & operator=(data_t * p) { ptr = p; ++(ptr->ref_count); return *this; }
	inline bind_ptr & operator=(const bind_ptr & rhs) { if (this != &rhs) { ptr = rhs.ptr; ++(ptr->ref_count); } return *this; }

	inline T * get() const { return &ptr->data; }
	inline T & operator*() const { ASSERT(ptr); return ptr->data; }
	inline T * operator->() const { ASSERT(ptr); return get(); }

	inline bool operator<(const bind_ptr & rhs) { return ptr->data < rhs.ptr->data; }
	inline bool operator<=(const bind_ptr & rhs) { return ptr->data <= rhs.ptr->data; }
	inline bool operator>(const bind_ptr & rhs) { return ptr->data > rhs.ptr->data; }
	inline bool operator>=(const bind_ptr & rhs) { return ptr->data >= rhs.ptr->data; }
	inline bool operator==(const bind_ptr & rhs) { return ptr->data == rhs.ptr->data; }
	inline bool operator!=(const bind_ptr & rhs) { return ptr->data != rhs.ptr->data; }

#ifdef _DEBUG
	//only for DEBUG!
	inline uint get_ref_count() const { return ptr->ref_count; }
#endif

private:
	data_t * ptr;

	inline void x_release() const
	{
		ASSERT(ptr);
		ASSERT(ptr->ref_count != 0);
		if (1 == ptr->ref_count)
			delete ptr;
		else
			--(ptr->ref_count);
	}
};

template<typename T>
class owner_ptr
{
	NONCOPYABLE(owner_ptr);

public:
	typedef T type;
	typedef T * pointer;

	owner_ptr() : xm_ptr(0) {}
	explicit owner_ptr(T * ptr) : xm_ptr(ptr) {}
	~owner_ptr() { release(); }

	inline T * operator=(T * ptr) { release(); return (xm_ptr = ptr); }

	inline T * get() const { return xm_ptr; }
	inline T & operator*() const { ASSERT(xm_ptr); return *xm_ptr; }
	inline T * operator->() const { ASSERT(xm_ptr); return get(); }

	inline void release()
	{
		if (xm_ptr)
		{
			delete xm_ptr;
			xm_ptr = 0;
		}
	}

private:
	pointer xm_ptr;
};

// 指针类 string, ANSI
typedef util::bind_ptr<string> pstring;
// 指针类 wstring, Unicode
typedef util::bind_ptr<wstring> pwstring;
// 指针类 tstring, 中间类型
typedef util::bind_ptr<tstring> ptstring;
// 容器指针类 string, ANSI
typedef std::vector<pstring> vpstring;
// 容器指针类 wstring, Unicode
typedef std::vector<pwstring> vpwstring;
// 容器指针类 tstring, 中间类型
typedef std::vector<ptstring> vptstring;


// memory 仅提供对对象内存的分配, 不调用构造函数、析构函数
// MAX_ALLOC_SIZE: 可分配的内存最大值, 默认为 CONST_VALUE::MAX_ALLOC_SIZE
// T: 要分配内存的对象
template<typename T, size_t MAX_ALLOC_SIZE = CONST_VALUE::MAX_ALLOC_SIZE>
class memory
{
public:
	memory() : xm_data(0), xm_size(0) { }
	memory(size_t new_size) : xm_data(0), xm_size(0) { alloc(new_size); }
	memory(const memory & rhs) { set_data(rhs.ptr(), rhs.size()); }
	~memory() { free(); }

	// 分配的内存的容量大小
	inline size_t size() const { return xm_size; }

	inline const T * ptr() const { return xm_data; }
	inline T * ptr() { return xm_data; }

	// 如果尚未分配内存则分配 new_size 的内存, 如果已经分配内存则重新分配 new_size 内存
	T * alloc(size_t new_size)
	{
		ASSERT(new_size > 0 && new_size * sizeof(T) <= MAX_ALLOC_SIZE);
		if (new_size != xm_size)
		{
			if (xm_data == 0)
			{
				xm_data = (T *)malloc(new_size * sizeof(T));
			}
			else
			{
				T * new_data;
				new_data = (T *)realloc(xm_data, new_size * sizeof(T));
				if (new_data == 0)
					free();
				xm_data = new_data;

			}
			xm_size = new_size;
		}
		return xm_data;
	}

	// 确保分配的内存至少有 new_size 大小
	inline T * resize(size_t new_size)
	{
		if (xm_size < new_size)
		{
			return alloc(new_size);
		}
		else
		{
			return ptr();
		}
	}

	inline operator const T * () const { return ptr(); }
	inline operator T * () { return ptr(); }
	inline memory & operator=(const memory & rhs) { assign(rhs.ptr(), rhs.size()); return *this; }
	inline T & at(unsigned int idx) { ASSERT(idx < xm_size); return xm_data[idx]; }
	inline T & operator[] (unsigned int idx) { return at(idx); }
	inline T & operator[] (int idx) { return at(idx); }
	inline T & operator[] (unsigned long idx) { return at(idx); }
	inline T & operator[] (long idx) { return at(idx); }

	inline T * copy(const void * src, size_t bytes, size_t offset = 0)
	{
		ASSERT(src);
		xm_data = resize(bytes / sizeof(T) + offset);
		memcpy((char*)xm_data + offset, src, bytes);
		return ptr();
	}

	inline T * assign(const T * src, size_t count) { return copy(src, count * sizeof(T)); }
	inline T * append(const T * src, size_t count) { return copy(src, count * sizeof(T), xm_size); }
	inline void zero() { memset(ptr(), 0, xm_size * sizeof(T)); }
	inline void free() { if (xm_data) ::free(xm_data); xm_data=0; xm_size=0; }

private:
	T * xm_data;
	size_t xm_size;
};

wchar * a2w_quick(char * src, wchar * dest, size_t len = -1);
char * w2a_quick(wchar * src, char * dest, size_t len = -1);

//inline size_t strlen(const char * str) { return ::strlen(str); }
//inline size_t strlen(const wchar * str) { return ::wcslen(str); }
//inline char * strcpy(char * dest, const char * source) { return ::strcpy(dest, source); }
//inline wchar * strcpy(wchar * dest, const wchar * source) { return ::wcscpy(dest, source); }
//inline char * strncpy(char * dest, const char * source, size_t count) { return ::strncpy(dest, source, count); }
//inline wchar * strncpy(wchar * dest, const wchar * source, size_t count) { return ::wcsncpy(dest, source, count); }
//inline int strcmp(const char * str1, const char * str2) { return ::strcmp(str1, str2); }
//inline int strcmp(const wchar * str1, const wchar * str2) { return ::wcscmp(str1, str2); }
//inline int stricmp(const char * str1, const char * str2) { return ::stricmp(str1, str2); }
//inline int stricmp(const wchar * str1, const wchar * str2) { return ::wcsicmp(str1, str2); }
//inline int strncmp(const char * str1, const char * str2, size_t count) { return ::strncmp(str1, str2, count); }
//inline int strncmp(const wchar * str1, const wchar * str2, size_t count) { return ::wcsncmp(str1, str2, count); }
//inline int strnicmp(const char * str1, const char * str2, size_t count) { return ::strnicmp(str1, str2, count); }
//inline int strnicmp(const wchar * str1, const wchar * str2, size_t count) { return ::wcsnicmp(str1, str2, count); }
//
//inline int sprintf(char * buf, const char * fmt, ...) { return ::vsprintf(buf, fmt, (va_list)(&fmt + 1)); }
//inline int sprintf(wchar * buf, const wchar * fmt, ...) { return ::vswprintf(buf, fmt, (va_list)(&fmt + 1)); }

inline const char * itox(int value, char * buf, int radix = 10) { return _itoa(value, buf, radix); }
inline const char * itox(unsigned int value, char * buf, int radix = 10) { return _ultoa(value, buf, radix); }
inline const char * itox(long value, char * buf, int radix = 10) { return _ltoa(value, buf, radix); }
inline const char * itox(unsigned long value, char * buf, int radix = 10) { return _ultoa(value, buf, radix); }
inline const char * itox(int64 value, char * buf, int radix = 10) { return _i64toa(value, buf, radix); }
inline const char * itox(uint64 value, char * buf, int radix = 10) { return _ui64toa(value, buf, radix); }
inline const wchar * itox(int value, wchar * buf, int radix = 10) { return _itow(value, buf, radix); }
inline const wchar * itox(unsigned int value, wchar * buf, int radix = 10) { return _ultow(value, buf, radix); }
inline const wchar * itox(long value, wchar * buf, int radix = 10) { return _ltow(value, buf, radix); }
inline const wchar * itox(unsigned long value, wchar * buf, int radix = 10) { return _ultow(value, buf, radix); }
inline const wchar * itox(int64 value, wchar * buf, int radix = 10) { return _i64tow(value, buf, radix); }
inline const wchar * itox(uint64 value, wchar * buf, int radix = 10) { return _ui64tow(value, buf, radix); }

const char * ftox(char * out, size_t buf_size, double val, unsigned precision, bool b_sign = false);
const wchar * ftox(wchar * out, size_t buf_size, double val, unsigned precision, bool b_sign = false);
inline const char * itox(double value, char * buf, int digits = 12) { return ftox(buf, -1, value, digits); }
inline const wchar * itox(double value, wchar * buf, int digits = 12) { return ftox(buf, -1, value, digits); }

template<typename T, size_t BUFSIZE = 64U>
class itox_t
{
public:
	inline itox_t(int value) { itox(value, buf); }
	inline itox_t(unsigned int value) { itox(value, buf); }
	inline itox_t(long value) { itox(value, buf); }
	inline itox_t(unsigned long value) { itox(value, buf); }
	inline itox_t(int64 value) { itox(value, buf); }
	inline itox_t(uint64 value) { itox(value, buf); }
	inline itox_t(double value, int digits = 12) { ftox(buf, BUFSIZE, value, digits); }

	inline const T * ptr() const { return buf; }
	inline operator const T * () const { return ptr(); }

private:
	T buf[BUFSIZE];
};

typedef itox_t<tchar> itot;
typedef itox_t<char> itoa;
typedef itox_t<wchar> itow;

namespace inner {

template<typename T> struct is_base_string_int {
	enum { value = is_bool<T>::value || is_short<T>::value || is_ushort<T>::value
		|| is_int<T>::value || is_uint<T>::value
		|| is_long<T>::value || is_ulong<T>::value
		|| is_int64<T>::value || is_uint64<T>::value
		|| is_float<T>::value || is_double<T>::value };
};

template<typename T, typename U>
inline T & base_string_append_impl(T & str, const U & value, int2type<true> /*summy*/)
{
	T::traits_type::char_type buf[64];
	return str.append(itox(value, buf));
}

template<typename T, typename U>
inline T & base_string_append_impl(T & str, const U & value, int2type<false> /*summy*/)
{
	return str.append(value);
}

template<typename T, typename U>
inline T & base_string_assign_impl(T & str, const U & value, int2type<true> /*summy*/)
{
	T::traits_type::char_type buf[64];
	return str.assign(itox(value, buf));
}

template<typename T, typename U>
inline T & base_string_assign_impl(T & str, const U & value, int2type<false> /*summy*/)
{
	return str.assign(value);
}

}; //namespace inner

template<typename T>
inline string & append(string & str, const T & value) { return inner::base_string_append_impl(str, value, int2type<inner::is_base_string_int<T>::value >()); }

template<typename T>
inline string & assign(string & str, const T & value) { return inner::base_string_assign_impl(str, value, int2type<inner::is_base_string_int<T>::value >()); }

template<typename T>
inline wstring & append(wstring & str, const T & value) { return inner::base_string_append_impl(str, value, int2type<inner::is_base_string_int<T>::value >()); }

template<typename T>
inline wstring & assign(wstring & str, const T & value) { return inner::base_string_assign_impl(str, value, int2type<inner::is_base_string_int<T>::value >()); }



template<typename T, size_t BUFSIZE = 64U>
class time2x_t
{
public:
	typedef T type;

	time2x_t(double time)
	{
		if(time < 0.001) {
			buf[0] = '0';
			buf[1] = ':';
			buf[2] = '0';
			buf[3] = '.';
			buf[4] = '0';
			buf[5] = '0';
			buf[6] = '0';
			buf[7] = '\0';
			return;
		}

		//static T fmt[3] = { '%', 'u', '\0' };

		unsigned int itime = (unsigned int)time;
		T * p = buf;
		if (itime >= 3600)
		{
			p += _tcslen(itox(itime / 3600, p));
			//p += sprintf(p, fmt, itime / 3600);
			itime %= 3600;
			*p++ = ':';
		}
		if (itime >= 60 || p > buf)
		{
			if (itime < 600 && p > buf)
				*p++ = '0';
			p += tcslen(itox(itime / 60, p));
			//p += sprintf(p, fmt, itime / 60);
			itime %= 60;
			*p++ = ':';
		}
		if (itime < 10)
			*p++ = '0';
		p += tcslen(itox(itime, p));
		//p += sprintf(p, fmt, itime);
		*p++ = '.';
		itime = (unsigned int)(time * 1000) % 1000;
		if (itime < 100)
			*p++ = '0';
		if (itime < 10)
			*p++ = '0';
		itox(itime, p);
		//sprintf(p, fmt, itime);
	}

	inline const T * ptr() const { return buf; }
	inline operator const T * () const { return ptr(); }

private:
	T buf[BUFSIZE];
};

typedef time2x_t<tchar> time2x;
typedef time2x_t<char>  time2a;
typedef time2x_t<wchar> time2w;



// accept decimal(12345)/hexadecimal(0xFFFF)/octal(012345)
int xtoi(const char * in, unsigned int len = -1);
int xtoi(const wchar * in, unsigned int len = -1);
// tstrcpy == _tcscpy && _tcsncpy
tchar * tstrcpy(tchar * dest, const tchar * source, unsigned int count = -1);


class format
{
public:
	explicit format(const TCHAR * Fmt, ...) : buf(0)
	{
#ifdef _vsctprintf
		int size = _vsctprintf(Fmt, (va_list)(&Fmt + 1));
		buf = new TCHAR[size + 1];
		_vstprintf(buf, Fmt, (va_list)(&Fmt + 1));
#else
		int size = 0, res = 0;
		do {
			size += 256;
			if (buf)
				delete [] buf;
			buf = new TCHAR[size + 1];
			res = _vsntprintf(buf, size, Fmt, (va_list)(&Fmt + 1));
		} while (size - res <= sizeof(TCHAR) * 5);
		buf[res] = 0;
#endif
	}

	~format() { delete [] buf; }
	format(const format & rhs) { _copy(rhs); }
	inline format & operator= (const format & rhs) { return _copy(rhs); }
	inline bool operator== (const format & rhs) { return !_tcscmp(buf, rhs.buf); }
	inline bool operator!= (const format & rhs) { return !(*this == rhs); }
	inline operator const TCHAR * () const { return buf; }
	inline const TCHAR * ptr() { return buf; }

private:
	format & _copy(const format & rhs)
	{
		if (this != &rhs)
		{
			delete [] buf;
			buf = new TCHAR[_tcslen(rhs.buf) + 1];
			_tcscpy(buf, rhs.buf);
		}
		return *this;
	}

private:
	TCHAR * buf;
};




size_t distance(const wchar * x, size_t xlen, const wchar * y, size_t ylen, bool icase = true);
inline size_t distance(const wchar * x, const wchar * y, bool icase = true) { return distance(x, wcslen(x), y, wcslen(y), icase); }
double round(double num, int n); //四舍五入
bool wildcard_match(const wchar * spec, const wchar * pattern, wchar separator = ';');
// 把一组二进制数据转换为十六进制字符串的形式
void bin2string(const byte * data, size_t length, tstring & out);
// 把一组十六进制字符串转换为二进制数据
void string2bin(const tchar * data, vector<byte> & out);


//************************************
// Method:    alignment 计算对齐
// Returns:   template<typename T>  T
// Parameter: T data 要对齐的数据
// Parameter: T align 对齐粒度
//************************************

template<typename T>
inline T alignment(T data, T align) { return 0 != data % align ? (data / align + 1) * align : data; }

//************************************
// Method:    split 把包含多个字串的块分解成字串数组的形式
// Returns:   void
// Parameter: const wchar * pstr 块的地址
// Parameter: size_t len 块的长度
// Parameter: vector<const wchar *> & out 输出
//************************************

void split(const wchar * pstr, size_t len, vector<const wchar *> & out);
void split(const tstring str, const tchar * separator, vptstring & out, bool trim_string = true);
inline void split(const tstring str, const tstring & separator, vptstring & out, bool trim_string = true)
{
	split(str, separator.c_str(), out, trim_string);
}

inline void split(const tstring str, tchar separator, vptstring & out, bool trim_string = true)
{
	tchar separators[] = { separator, '\0' };
	split(str, separators, out, trim_string);
}





} // end of namespace
//=========================================================================
//					Utilitys namespace
//=========================================================================


template<typename T>
inline std::string & operator<<(std::string & str, const T & value) { return util::append(str, value); }

template<typename T>
inline std::wstring & operator<<(std::wstring & str, const T & value) { return util::append(str, value); }







#endif // _DEFINED_uniqueheadername
