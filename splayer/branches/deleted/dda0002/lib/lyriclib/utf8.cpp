#include "stdafx.h"
#include <assert.h>
#include "utf8.h"

using std::string;


namespace util {
namespace utf8 {


static const BYTE mask_tab[6]={0x80,0xE0,0xF0,0xF8,0xFC,0xFE};

static const BYTE val_tab[6]={0,0xC0,0xE0,0xF0,0xF8,0xFC};

size_t strlen_max(const char * ptr, size_t max)
{
	if (ptr==0) return 0;
	size_t n = 0;
	while(ptr[n] && n<max) n++;
	return n;
}

size_t wcslen_max(const WCHAR * ptr, size_t max)
{
	if (ptr==0) return 0;
	size_t n = 0;
	while(ptr[n] && n<max) n++;
	return n;
}

size_t utf8_decode_char(const char *p_utf8, unsigned * wide, size_t max)
{
	const BYTE * utf8 = (const BYTE*)p_utf8;

	if (wide) *wide = 0;

	if (max==0)
		return 0;
	else if (max>6) max = 6;

	if (utf8[0]<0x80)
	{
		if (wide) *wide = utf8[0];
		return utf8[0]>0 ? 1 : 0;
	}

	unsigned res=0;
	unsigned n;
	unsigned cnt=0;
	while(1)
	{
		if ((*utf8&mask_tab[cnt])==val_tab[cnt]) break;
		if (++cnt>=max) return 0;
	}
	cnt++;

	for(n=0;n<cnt;n++)
		if (utf8[n]==0) return 0;


	if (cnt==2 && !(*utf8&0x1E)) return 0;

	if (cnt==1)
		res=*utf8;
	else
		res=(0xFF>>(cnt+1))&*utf8;

	for (n=1;n<cnt;n++)
	{
		if ((utf8[n]&0xC0) != 0x80)
			return 0;
		if (!res && n==2 && !((utf8[n]&0x7F) >> (7 - cnt)))
			return 0;

		res=(res<<6)|(utf8[n]&0x3F);
	}

	if (wide)
		*wide=res;

	return cnt;
}


size_t utf8_encode_char(unsigned wide, char * target)
{
	size_t count;

	if (wide < 0x80)
		count = 1;
	else if (wide < 0x800)
		count = 2;
	else if (wide < 0x10000)
		count = 3;
	else if (wide < 0x200000)
		count = 4;
	else if (wide < 0x4000000)
		count = 5;
	else if (wide <= 0x7FFFFFFF)
		count = 6;
	else
		return 0;
	//if (count>max) return 0;

	if (target == 0)
		return count;

	switch (count)
	{
    case 6:
		target[5] = 0x80 | (wide & 0x3F);
		wide = wide >> 6;
		wide |= 0x4000000;
    case 5:
		target[4] = 0x80 | (wide & 0x3F);
		wide = wide >> 6;
		wide |= 0x200000;
    case 4:
		target[3] = 0x80 | (wide & 0x3F);
		wide = wide >> 6;
		wide |= 0x10000;
    case 3:
		target[2] = 0x80 | (wide & 0x3F);
		wide = wide >> 6;
		wide |= 0x800;
    case 2:
		target[1] = 0x80 | (wide & 0x3F);
		wide = wide >> 6;
		wide |= 0xC0;
	case 1:
		target[0] = wide;
	}

	return count;
}

size_t utf16_encode_char(unsigned cur_wchar, WCHAR * out)
{
	if (cur_wchar>0 && cur_wchar<(1<<20))
	{
		if (cur_wchar>=0x10000)
		{
			unsigned c = cur_wchar - 0x10000;
			//MSDN:
			//The first (high) surrogate is a 16-bit code value in the range U+D800 to U+DBFF. The second (low) surrogate is a 16-bit code value in the range U+DC00 to U+DFFF. Using surrogates, Unicode can support over one million characters. For more details about surrogates, refer to The Unicode Standard, version 2.0.
			out[0] = (WCHAR)(0xD800 | (0x3FF & (c>>10)) );
			out[1] = (WCHAR)(0xDC00 | (0x3FF & c) ) ;
			return 2;
		}
		else
		{
			*out = (WCHAR)cur_wchar;
			return 1;
		}
	}
	return 0;
}

size_t utf16_decode_char(const WCHAR * src, unsigned * out)
{
	size_t rv = 0;
	unsigned int cur_wchar = *(src++);
	if (cur_wchar)
	{
		rv = 1;
		if ((cur_wchar & 0xFC00) == 0xD800)
		{
			unsigned int low = *src;
			if ((low & 0xFC00) == 0xDC00)
			{
				src++;
				cur_wchar = 0x10000 + ( ((cur_wchar & 0x3FF) << 10) | (low & 0x3FF) );
				rv = 2;
			}
		}
	}
	*out = cur_wchar;
	return rv;
}


UINT utf8_get_char(const char * src)
{
	UINT rv = 0;
	utf8_decode_char(src,&rv);
	return rv;
}


size_t utf8_char_len(const char * s)
{
	return utf8_decode_char(s,0);
}

int skip_utf8_chars(const char * ptr,int count)
{
	int num = 0;
	for(;count && ptr[num];count--)
	{
		int d = (int)utf8_char_len(ptr+num);
		if (d<=0) break;
		num+=d;
	}
	return num;
}

size_t convert_utf8_to_utf16(const char * src, WCHAR * dst, size_t len)
{
	size_t rv = 0;
	while(*src && len)
	{
		unsigned int c;
		size_t d = utf8_decode_char(src,&c,len);
		if (d==0 || d>len) break;
		src += d;
		len -= d;
		d = utf16_encode_char(c,dst);
		if (d==0) break;
		dst += d;
		rv += d;
	}
	*dst = 0;
	return rv;
}

size_t convert_utf16_to_utf8(const WCHAR * src, char * dst, size_t len)
{
	size_t rv = 0;
	while(*src && len)
	{
		unsigned int c;
		size_t d = utf16_decode_char(src,&c);
		if (d==0 || d>len) break;
		src += d;
		len -= d;
		d = utf8_encode_char(c,dst);
		if (d==0) break;
		dst += d;
		rv += d;
	}
	*dst = 0;
	return rv;
}

size_t convert_ansi_to_utf16(const char * src, WCHAR * dst, size_t len, unsigned int cp)
{
	len = strlen_max(src,len);
	size_t rv;
#ifdef _WIN32
	rv = MultiByteToWideChar(cp,0,src,(int)len,dst,(int)estimate_ansi_to_utf16(src));
#else
	setlocale(LC_CTYPE,"");
	rv = mbstowcs(dst,src,len);
#endif
	if ((signed)rv<0) rv = 0;
	dst[rv]=0;
	return rv;
}

size_t convert_utf16_to_ansi(const WCHAR * src, char * dst, size_t len, unsigned int cp)
{
	len = wcslen_max(src,len);
	size_t rv;
#ifdef _WIN32
	rv = WideCharToMultiByte(cp,0,src,(int)len,dst,(int)estimate_utf16_to_ansi(src),0,0);
#else
	setlocale(LC_CTYPE,"");
	rv = wcstombs(dst,src,len);
#endif
	if ((signed)rv<0) rv = 0;
	dst[rv]=0;
	return rv;
}

size_t convert_utf8_to_ansi(const char * src, char * dst, size_t len, unsigned int cp)
{//meh
	len = strlen_max(src,len);

	size_t temp_len = estimate_utf8_to_utf16(src,len);
	WCHAR * temp = new WCHAR[temp_len];

	len = convert_utf8_to_utf16(src,temp,len);
	size_t rv = convert_utf16_to_ansi(temp,dst,len,cp);
	delete[] temp;
	return rv;
}

size_t convert_ansi_to_utf8(const char * src, char * dst, size_t len, unsigned int cp)
{//meh
	len = strlen_max(src,len);

	size_t temp_len = estimate_ansi_to_utf16(src,len);
	WCHAR * temp = new WCHAR[temp_len];

	len = convert_ansi_to_utf16(src,temp,len,cp);
	return convert_utf16_to_utf8(temp,dst,len);
}

bool is_valid_utf8(const char * param)
{
#ifdef _MSC_VER
	__try {
#endif
		while(*param)
		{
			size_t d = utf8_decode_char(param,0);
			if (d==0) return false;
			param += d;
		}
		return true;
#ifdef _MSC_VER
	}
	__except(1)
	{
		return false;
	}
#endif
}

bool is_lower_ascii(const char * param)
{
	while(*param)
	{
		if (*param<0) return false;
		param++;
	}
	return true;
}

static bool check_end_of_string(const char * ptr)
{
#ifdef _MSC_VER
	__try {
#endif
		return !*ptr;
#ifdef _MSC_VER
	}
	__except(1) {return true;}
#endif
}

size_t strcpy_utf8_truncate(const char * src, char * out, size_t maxbytes)
{
	size_t rv = 0 , ptr = 0;
	if (maxbytes>0)
	{
		maxbytes--;//for null
		while(!check_end_of_string(src) && maxbytes>0)
		{
#ifdef _MSC_VER
			__try {
#endif
				size_t delta = utf8_char_len(src);
				if (delta>maxbytes || delta==0) break;
				do
				{
					out[ptr++] = *(src++);
				} while(--delta);
#ifdef _MSC_VER
			} __except(1) { break; }
#endif
			rv = ptr;
		}
		out[rv]=0;
	}
	return rv;
}

void recover_invalid_utf8(const char * src,char * out,unsigned replace)
{
	while(!check_end_of_string(src))
	{
		unsigned int c;
		size_t d;
#ifdef _MSC_VER
		__try {
#endif
			d = utf8_decode_char(src,&c);
#ifdef _MSC_VER
		} __except(1) {d = 0;}
#endif
		if (d==0) c = replace;
		out += utf8_encode_char(c,out);
	}
	*out = 0;
}

size_t strlen_utf8(const char * p, size_t num)
{
	unsigned w;
	size_t d;
	size_t ret = 0;
	for(;num;)
	{
		d = utf8_decode_char(p,&w);
		if (w==0 || d<=0) break;
		ret++;
		p+=d;
		num-=d;
	}
	return ret;
}

size_t utf8_chars_to_bytes(const char * str, size_t count)
{
	size_t bytes = 0;
	while(count)
	{
		size_t delta = utf8_decode_char(str+bytes,0);
		if (delta==0) break;
		bytes += delta;
		count--;
	}
	return bytes;
}

void swap_utf16_order(WCHAR * ptr, unsigned int len)
{
	for (unsigned int i=0; i<len; i++)
	{
		*ptr = ((*ptr & 0xFF) << 8) | (*ptr >> 8);
		if (0 == *ptr) break;
		ptr++;
	}
}

std::string & append_utf8_from_ansi(std::string & io, const char * src, size_t len, unsigned int cp)
{
	len = strlen_max(src, len);
	std::string::size_type str_len = io.length();
	io.append(estimate_ansi_to_utf8(src, len), 0);
	len = convert_ansi_to_utf8(src, (char *)io.c_str() + str_len, len, cp);
	return io.erase(str_len + len);
}

std::string & append_ansi_from_utf8(std::string & io, const char * src, size_t len, unsigned int cp)
{
	len = strlen_max(src, len);
	std::string::size_type str_len = io.length();
	io.append(estimate_utf8_to_ansi(src, len), 0);
	len = convert_utf8_to_ansi(src, (char *)io.c_str() + str_len, len, cp);
	return io.erase(str_len + len);
}

std::string & append_ansi_from_utf16(std::string & io, const WCHAR * src, size_t len, unsigned int cp)
{
	len = wcslen_max(src, len);
	std::string::size_type str_len = io.length();
	io.append(estimate_utf16_to_ansi(src, len), 0);
	len = convert_utf16_to_ansi(src, (char *)io.c_str() + str_len, len, cp);
	return io.erase(str_len + len);
}

std::string & append_utf8_from_utf16(std::string & io, const WCHAR * src, size_t len)
{
	len = wcslen_max(src, len);
	std::string::size_type str_len = io.length();
	io.append(estimate_utf16_to_utf8(src, len), 0);
	len = convert_utf16_to_utf8(src, (char *)io.c_str() + str_len, len);
	return io.erase(str_len + len);
}

std::wstring & append_utf16_from_ansi(std::wstring & io, const char * src, size_t len, unsigned int cp)
{
	len = strlen_max(src, len);
	std::string::size_type str_len = io.length();
	io.append(estimate_ansi_to_utf16(src, len), 0);
	len = convert_ansi_to_utf16(src, (WCHAR *)io.c_str() + str_len, len, cp);
	return io.erase(str_len + len);
}

std::wstring & append_utf16_from_utf8(std::wstring & io, const char * src, size_t len)
{
	len = strlen_max(src, len);
	std::string::size_type str_len = io.length();
	io.append(estimate_utf8_to_utf16(src, len), 0);
	len = convert_utf8_to_utf16(src, (WCHAR *)io.c_str() + str_len, len);
	return io.erase(str_len + len);
}

bool is_text_utf8(const BYTE * s, size_t len)
{
	unsigned long octets = 0;  // octets to go in this UTF-8 encoded character
	BYTE chr;
	bool is_all_ascii = true;

	for (size_t i = 0; i < len; ++i )
	{
		chr= *(s + i);

		if ((chr & 0x80) != 0)
			is_all_ascii= FALSE;

		if (octets == 0 )
		{
			//
			// 7 bit ascii after 7 bit ascii is just fine.  Handle start of encoding case.
			//
			if (chr >= 0x80)
			{
				//
				// count of the leading 1 bits is the number of characters encoded
				//
				do
				{
					chr <<= 1;
					++octets;
				}
				while ((chr&0x80) != 0);

				--octets;                        // count includes this character

				if (octets == 0)
					return false;  // must start with 11xxxxxx
			}
		}
		else
		{
			// non-leading bytes must start as 10xxxxxx
			if ((chr & 0xC0) != 0x80)
			{
				return false;
			}
			
			--octets;                           // processed another octet in encoding
		}
	}

	//
	// End of text.  Check for consistency.
	//
	if (octets > 0)
	{
		// anything left over at the end is an error
		return false;
	}

	if (is_all_ascii)
	{
		// Not utf-8 if all ascii.  Forces caller to use code pages for conversion
		return false;
	}

	return true;
}



} //namespace utf8
} //namespace util


