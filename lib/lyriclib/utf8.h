// ***************************************************************
//  utf8   version:  1.0   ? date: 10/28/2006
//  -------------------------------------------------------------
//  mostly code come from foobar2000 SDK and have some modified.
//  -------------------------------------------------------------
//  Copyright (C) 2006 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#ifndef UTF8_UTILITY_H_H
#define UTF8_UTILITY_H_H

#ifdef _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <locale.h>
#endif
#include <string>

#ifdef UNICODE
#define string_os_from_utf8 string_utf16_from_utf8
#define string_utf8_from_os string_utf8_from_utf16
#define string_os_from_utf16(x) x
#define string_utf16_from_os(x) x
#define string_os_from_ansi string_utf16_from_ansi
#define string_ansi_from_os string_ansi_from_utf16

#define estimate_utf8_to_os estimate_utf8_to_utf16
#define estimate_os_to_utf8 estimate_utf16_to_utf8
#define estimate_utf16_to_os estimate_utf16_to_utf16
#define estimate_os_to_utf16 estimate_utf16_to_utf16
#define estimate_ansi_to_os estimate_ansi_to_utf16
#define estimate_os_to_ansi estimate_utf16_to_ansi

#define convert_utf8_to_os convert_utf8_to_utf16
#define convert_os_to_utf8 convert_utf16_to_utf8
#define convert_ansi_to_os convert_ansi_to_utf16
#define convert_os_to_ansi convert_utf16_to_ansi
#define convert_utf16_to_os wcscpy
#define convert_os_to_utf16 wcscpy

#define append_os_from_utf8 append_utf16_from_utf8
#define append_utf8_from_os append_utf8_from_utf16
#define append_os_from_utf16 append_utf16_from_utf16
#define append_utf16_from_os append_utf16_from_utf16
#define append_os_from_ansi append_utf16_from_ansi
#define append_ansi_from_os append_ansi_from_utf16

#define assign_os_from_utf8 assign_utf16_from_utf8
#define assign_utf8_from_os assign_utf8_from_utf16
#define assign_os_from_utf16 assign_utf16_from_utf16
#define assign_utf16_from_os assign_utf16_from_utf16
#define assign_os_from_ansi assign_utf16_from_ansi
#define assign_ansi_from_os assign_ansi_from_utf16
#else
#define string_os_from_utf8 string_ansi_from_utf8
#define string_utf8_from_os string_utf8_from_ansi
#define string_os_from_utf16 string_ansi_from_utf16
#define string_utf16_from_os string_utf16_from_ansi
#define string_os_from_ansi(x) x
#define string_ansi_from_os(x) x

#define estimate_utf8_to_os estimate_utf8_to_ansi
#define estimate_os_to_utf8 estimate_ansi_to_utf8
#define estimate_utf16_to_os estimate_utf16_to_ansi
#define estimate_os_to_utf16 estimate_ansi_to_utf16
#define estimate_ansi_to_os estimate_ansi_to_ansi
#define estimate_os_to_ansi estimate_ansi_to_ansi

#define convert_utf8_to_os convert_utf8_to_ansi
#define convert_os_to_utf8 convert_ansi_to_utf8
#define convert_ansi_to_os strcpy
#define convert_os_to_ansi strcpy
#define convert_utf16_to_os convert_utf16_to_ansi
#define convert_os_to_utf16 convert_ansi_to_utf16

#define append_os_from_utf8 append_ansi_from_utf8
#define append_utf8_from_os append_utf8_from_ansi
#define append_os_from_utf16 append_ansi_from_utf16
#define append_utf16_from_os append_utf16_from_ansi
#define append_os_from_ansi append_ansi_from_ansi
#define append_ansi_from_os append_ansi_from_ansi

#define assign_os_from_utf8 assign_ansi_from_utf8
#define assign_utf8_from_os assign_utf8_from_ansi
#define assign_os_from_utf16 assign_ansi_from_utf16
#define assign_utf16_from_os assign_utf16_from_ansi
#define assign_os_from_ansi assign_ansi_from_ansi
#define assign_ansi_from_os assign_ansi_from_ansi
#endif

#ifndef _WIN32

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#endif

//#define UTF_COUNTOF(x) sizeof(x)/sizeof(x[0])




namespace util
{
namespace utf8
{

size_t strlen_max(const char * ptr, size_t max);
size_t wcslen_max(const WCHAR * ptr, size_t max);

size_t strlen_utf8(const char * s, size_t num = -1); //returns number of characters in utf8 string; num - no. of bytes (optional)
size_t utf8_char_len(const char * s); //returns size of utf8 character pointed by s, in bytes, 0 on error
size_t utf8_chars_to_bytes(const char * str, size_t count);

size_t strcpy_utf8_truncate(const char * src, char * out, size_t maxbytes);

size_t utf8_decode_char(const char * src, unsigned int * out, size_t src_bytes = -1); //returns length in bytes
size_t utf8_encode_char(unsigned int c, char * out); //returns used length in bytes, max 6
size_t utf16_decode_char(const WCHAR * src, unsigned int * out);
size_t utf16_encode_char(unsigned intc, WCHAR * out);

bool is_lower_ascii(const char * param);
bool is_valid_utf8(const char * param);
void swap_utf16_order(WCHAR * ptr, unsigned int len = -1);

inline size_t estimate_utf8_to_utf16(const char * src, size_t utf8_len) { return utf8_len + 1; } //estimates amount of output buffer space that will be sufficient for conversion, including null
inline size_t estimate_utf8_to_utf16(const char * src) { return estimate_utf8_to_utf16(src, strlen(src)); }
inline size_t estimate_utf16_to_utf8(const WCHAR * src, size_t utf16_len) { return utf16_len * 3 + 1; }
inline size_t estimate_utf16_to_utf8(const WCHAR * src) { return estimate_utf16_to_utf8(src, wcslen(src)); }

inline size_t estimate_utf16_to_ansi(const WCHAR * src, size_t utf16_len) { return utf16_len * 2 + 1; }
inline size_t estimate_utf16_to_ansi(const WCHAR * src, size_t utf16_len, unsigned int codepage) { return WideCharToMultiByte(codepage,0,src,(int)utf16_len,0,0,0,FALSE) + 1; }
inline size_t estimate_utf16_to_ansi(const WCHAR * src) { return estimate_utf16_to_ansi(src, wcslen(src)); }
inline size_t estimate_ansi_to_utf16(const char * src, size_t ansi_len) { return ansi_len + 1; }
inline size_t estimate_ansi_to_utf16(const char * src, size_t ansi_len, unsigned int codepage) { return MultiByteToWideChar(codepage,0,src,(int)ansi_len,0,0) + 1; }
inline size_t estimate_ansi_to_utf16(const char * src) { return estimate_ansi_to_utf16(src, strlen(src)); }

inline size_t estimate_utf8_to_ansi(const char * src, size_t utf8_len) { return utf8_len + 1; }
inline size_t estimate_utf8_to_ansi(const char * src) { return estimate_utf8_to_ansi(src, strlen(src)); }
inline size_t estimate_ansi_to_utf8(const char * src, size_t ansi_len) { return ansi_len * 3 + 1; }
inline size_t estimate_ansi_to_utf8(const char * src) { return estimate_ansi_to_utf8(src, strlen(src)); }

inline size_t estimate_ansi_to_ansi(const char * src, size_t ansi_len) { return ansi_len + 1; }
inline size_t estimate_ansi_to_ansi(const char * src) { return estimate_ansi_to_ansi(src, strlen(src)); }
inline size_t estimate_utf16_to_utf16(const WCHAR * src, size_t utf16_len) { return utf16_len + 1; }
inline size_t estimate_utf16_to_utf16(const WCHAR * src) { return estimate_utf16_to_utf16(src, wcslen(src)); }

size_t convert_utf8_to_utf16(const char * src, WCHAR * dst, size_t len = -1); //len - amount of bytes/wchars to convert (will not go past null terminator)
size_t convert_utf16_to_utf8(const WCHAR * src, char * dst, size_t len = -1);
size_t convert_utf8_to_ansi(const char * src, char * dst, size_t len = -1, unsigned int cp = CP_ACP);
size_t convert_ansi_to_utf8(const char * src, char * dst, size_t len = -1, unsigned int cp = CP_ACP);
size_t convert_ansi_to_utf16(const char * src, WCHAR * dst, size_t len = -1, unsigned int cp = CP_ACP);
size_t convert_utf16_to_ansi(const WCHAR * src, char * dst, size_t len = -1, unsigned int cp = CP_ACP);

std::string & append_utf8_from_ansi(std::string & io, const char * src, size_t len = -1, unsigned int cp = CP_ACP);
std::string & append_ansi_from_utf8(std::string & io, const char * src, size_t len = -1, unsigned int cp = CP_ACP);
std::string & append_ansi_from_utf16(std::string & io, const WCHAR * src, size_t len = -1, unsigned int cp = CP_ACP);
std::string & append_utf8_from_utf16(std::string & io, const WCHAR * src, size_t len = -1);
std::wstring & append_utf16_from_ansi(std::wstring & io, const char * src, size_t len = -1, unsigned int cp = CP_ACP);
std::wstring & append_utf16_from_utf8(std::wstring & io, const char * src, size_t len = -1);

inline std::string & append_ansi_from_ansi(std::string & io, const char * src, size_t len = -1, unsigned int cp = CP_ACP)
{ return io.append(src); }
inline std::wstring & append_utf16_from_utf16(std::wstring & io, const WCHAR * src, size_t len = -1, unsigned int cp = CP_ACP)
{ return io.append(src); }

inline std::string & assign_utf8_from_ansi(std::string & io, const char * src, size_t len = -1, unsigned int cp = CP_ACP)
{ io.erase(); return append_utf8_from_ansi(io, src, len, cp); }
inline std::string & assign_ansi_from_utf8(std::string & io, const char * src, size_t len = -1, unsigned int cp = CP_ACP)
{ io.erase(); return append_ansi_from_utf8(io, src, len, cp); }
inline std::string & assign_ansi_from_utf16(std::string & io, const WCHAR * src, size_t len = -1, unsigned int cp = CP_ACP)
{ io.erase(); return append_ansi_from_utf16(io, src, len, cp); }
inline std::string & assign_utf8_from_utf16(std::string & io, const WCHAR * src, size_t len = -1)
{ io.erase(); return append_utf8_from_utf16(io, src, len); }
inline std::wstring & assign_utf16_from_ansi(std::wstring & io, const char * src, size_t len = -1, unsigned int cp = CP_ACP)
{ io.erase(); return append_utf16_from_ansi(io, src, len, cp); }
inline std::wstring & assign_utf16_from_utf8(std::wstring & io, const char * src, size_t len = -1)
{ io.erase(); return append_utf16_from_utf8(io, src, len); }

inline std::string & assign_ansi_from_ansi(std::string & io, const char * src, size_t len = -1, unsigned int cp = CP_ACP)
{ return io.assign(src); }
inline std::wstring & assign_utf16_from_utf16(std::wstring & io, const WCHAR * src, size_t len = -1, unsigned int cp = CP_ACP)
{ return io.assign(src); }

const BYTE UTF16LE_SIGN[] = { 0xFF, 0xFE };
const BYTE UTF16BE_SIGN[] = { 0xFE, 0xFF };
const BYTE UTF16_SIGN[] = { 0xFF, 0xFE };
const BYTE UTF8_SIGN[] = { 0xEF, 0xBB, 0xBF };

const size_t UTF16_SIGN_SIZE = _countof(UTF16_SIGN);
const size_t UTF8_SIGN_SIZE = _countof(UTF8_SIGN);

const char * const XML_UTF8_HEADER = "\xEF\xBB\xBF<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\r\n\r\n";

inline bool is_utf8_sign(const BYTE * header, size_t len = -1)
{
	return len >= UTF8_SIGN_SIZE && !memcmp(header, UTF8_SIGN, UTF8_SIGN_SIZE);
}

inline bool is_utf16le_sign(const BYTE * header, size_t len = -1)
{
	return len >= UTF16_SIGN_SIZE && !memcmp(header, UTF16LE_SIGN, UTF16_SIGN_SIZE);
}

inline bool is_utf16be_sign(const BYTE * header, size_t len = -1)
{
	return len >= UTF16_SIGN_SIZE && !memcmp(header, UTF16BE_SIGN, UTF16_SIGN_SIZE);
}

inline bool is_utf16_sign(const BYTE * header, size_t len = -1)
{
	return is_utf16le_sign(header, len) || is_utf16be_sign(header, len);
}

bool is_text_utf8(const BYTE * s, size_t len);


template <class T>
class string_convert_base
{
protected:
	T * ptr;
	inline void alloc(size_t size)
	{
		ptr = (T *)malloc(size * sizeof(T));
	}

	virtual ~string_convert_base() { if (ptr) free(ptr); }
public:
	inline operator const T * () const { return ptr;}
	inline const T * get_ptr() const { return ptr;}

	inline size_t length()
	{
		size_t ret = 0;
		const T * p = ptr;
		while (*p) { ret++; p++; }
		return ret;
	}
};

class string_utf8_from_utf16 : public string_convert_base<char>
{
public:
	explicit string_utf8_from_utf16(const WCHAR * src, size_t len = -1)
	{
		len = wcslen_max(src, len);
		alloc(estimate_utf16_to_utf8(src, len));
		convert_utf16_to_utf8(src, ptr, len);
	}
};

class string_utf16_from_utf8 : public string_convert_base<WCHAR>
{
public:
	explicit string_utf16_from_utf8(const char * src, size_t len = -1)
	{
		len = strlen_max(src, len);
		alloc(estimate_utf8_to_utf16(src, len));
		convert_utf8_to_utf16(src, ptr, len);
	}
};

class string_ansi_from_utf16 : public string_convert_base<char>
{
public:
	explicit string_ansi_from_utf16(const WCHAR * src, size_t len = -1, unsigned int cp = CP_ACP)
	{
		len = wcslen_max(src, len);
		alloc(estimate_utf16_to_ansi(src, len));
		convert_utf16_to_ansi(src, ptr, len, cp);
	}
};

class string_utf16_from_ansi : public string_convert_base<WCHAR>
{
public:
	explicit string_utf16_from_ansi(const char * src, size_t len = -1, unsigned int cp = CP_ACP)
	{
		len = strlen_max(src, len);
		alloc(estimate_ansi_to_utf16(src, len));
		convert_ansi_to_utf16(src, ptr, len, cp);
	}
};

class string_utf8_from_ansi : public string_convert_base<char>
{
public:
	explicit string_utf8_from_ansi(const char * src, size_t len = -1, unsigned int cp = CP_ACP)
	{
		len = strlen_max(src, len);
		alloc(estimate_ansi_to_utf8(src, len));
		convert_ansi_to_utf8(src, ptr, len, cp);
	}
};

class string_ansi_from_utf8 : public string_convert_base<char>
{
public:
	explicit string_ansi_from_utf8(const char * src, size_t len = -1, unsigned int cp = CP_ACP)
	{
		len = strlen_max(src, len);
		alloc(estimate_utf8_to_ansi(src, len));
		convert_utf8_to_ansi(src, ptr, len, cp);
	}
};


typedef string_utf8_from_utf16 w2u;
typedef string_utf16_from_utf8 u2w;
typedef string_ansi_from_utf16 w2a;
typedef string_utf16_from_ansi a2w;
typedef string_utf8_from_ansi a2u;
typedef string_ansi_from_utf8 u2a;

#ifdef UNICODE
typedef w2u t2u;
typedef w2a t2a;
#define t2w(x) x
typedef u2w u2t;
typedef a2w a2t;
#define w2t(x) x
#else
typedef a2u t2u;
typedef a2w t2w;
#define t2a(x) x
typedef u2a u2t;
typedef w2a w2t;
#define a2t(x) x
#endif









} //namespace utf8
} //namespace util


inline std::string & operator<<(std::string & str, const std::wstring & value) { return util::utf8::append_ansi_from_utf16(str, value.c_str(), value.length()); }
inline std::string & operator<<(std::string & str, const WCHAR * value) { return util::utf8::append_ansi_from_utf16(str, value); }
inline std::wstring & operator<<(std::wstring & str, const std::string & value) { return util::utf8::append_utf16_from_ansi(str, value.c_str(), value.length()); }
inline std::wstring & operator<<(std::wstring & str, const char * value) { return util::utf8::append_utf16_from_ansi(str, value); }






#endif