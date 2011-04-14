// lyriclib.h

#pragma once
#include "stdafx.h"

#include "utils.h"
#include <math.h>

//=========================================================================
//					Utilitys namespace
//=========================================================================
namespace util {



const char * ftox(char * out, size_t buf_size, double val, unsigned precision, bool b_sign)
{
	char temp[64];
	size_t outptr;
	size_t temp_len;

	if (buf_size == 0) return out;
	buf_size--; //for null terminator

	outptr = 0;

	if (outptr == buf_size) {out[outptr] = 0; return out;}

	if (val < 0) {out[outptr++] = '-'; val = -val;}
	else if (b_sign) {out[outptr++] = '+';}

	if (outptr == buf_size) {out[outptr] = 0; return out;}


	{
		double powval = pow((double)10.0, (double)precision);
		int64 blargh = (int64)floor(val * powval + 0.5);
		itox(blargh, temp, 10);
	}

	temp_len = strlen(temp);
	if (temp_len <= precision)
	{
		out[outptr++] = '0';
		if (outptr == buf_size) {out[outptr] = 0; return out;}
		out[outptr++] = '.';
		if (outptr == buf_size) {out[outptr] = 0; return out;}
		size_t d;
		for (d = precision - temp_len;d;d--)
		{
			out[outptr++] = '0';
			if (outptr == buf_size) {out[outptr] = 0; return out;}
		}
		for (d = 0;d < temp_len;d++)
		{
			out[outptr++] = temp[d];
			if (outptr == buf_size) {out[outptr] = 0; return out;}
		}
	}
	else
	{
		size_t d = temp_len;
		const char * src = temp;
		while (*src)
		{
			if (d-- == precision)
			{
				out[outptr++] = '.';
				if (outptr == buf_size) {out[outptr] = 0; return out;}
			}
			out[outptr++] = *(src++);
			if (outptr == buf_size) {out[outptr] = 0; return out;}
		}
	}
	out[outptr] = 0;
	return out;
}

const wchar * ftox(wchar * out, size_t buf_size, double val, unsigned precision, bool b_sign)
{
	wchar temp[64];
	size_t outptr;
	size_t temp_len;

	if (buf_size == 0) return out;
	buf_size--; //for null terminator

	outptr = 0;

	if (outptr == buf_size) {out[outptr] = 0; return out;}

	if (val < 0) {out[outptr++] = '-'; val = -val;}
	else if (b_sign) {out[outptr++] = '+';}

	if (outptr == buf_size) {out[outptr] = 0; return out;}


	{
		double powval = pow((double)10.0, (double)precision);
		int64 blargh = (int64)floor(val * powval + 0.5);
		itox(blargh, temp, 10);
	}

	temp_len = _tcslen(temp);
	if (temp_len <= precision)
	{
		out[outptr++] = '0';
		if (outptr == buf_size) {out[outptr] = 0; return out;}
		out[outptr++] = '.';
		if (outptr == buf_size) {out[outptr] = 0; return out;}
		size_t d;
		for (d = precision - temp_len;d;d--)
		{
			out[outptr++] = '0';
			if (outptr == buf_size) {out[outptr] = 0; return out;}
		}
		for (d = 0;d < temp_len;d++)
		{
			out[outptr++] = temp[d];
			if (outptr == buf_size) {out[outptr] = 0; return out;}
		}
	}
	else
	{
		size_t d = temp_len;
		const wchar * src = temp;
		while (*src)
		{
			if (d-- == precision)
			{
				out[outptr++] = '.';
				if (outptr == buf_size) {out[outptr] = 0; return out;}
			}
			out[outptr++] = *(src++);
			if (outptr == buf_size) {out[outptr] = 0; return out;}
		}
	}
	out[outptr] = 0;
	return out;
}


namespace inner {
bool wildcard_match_impl(const wchar * spec, const wchar * rm, wchar separator)
{
	for (;; ++spec, ++rm)
	{
		if ((separator && *rm == separator) || *rm == 0)
			return *spec == 0;
		else if (*rm == '*')
		{
			rm++;
			if (0 == *rm || separator == *rm)
				return true;

			do
			{
				if (wildcard_match_impl(spec, rm, separator))
					return true;
			}
			while (*++spec);
			return false;
		}
		else if (*spec == 0)
			return false;
		else if (*rm != '?' && tolower(*spec) != tolower(*rm))
			return false;
	}
}
};

bool wildcard_match(const wchar * spec, const wchar * pattern, wchar separator)
{
	if (0 == separator)
		return inner::wildcard_match_impl(spec, pattern, 0);
	const wchar * rm = pattern;
	while (*rm)
	{
		if (inner::wildcard_match_impl(spec, rm, separator))
			return true;
		while (*rm && *rm != separator) rm++;
		if (*rm == separator)
		{
			while (*rm == separator) rm++;
			while (*rm == ' ') rm++;
		}
	}

	return false;
}

wchar * a2w_quick(char * src, wchar * dest, size_t len)
{
	wchar * old = dest;
	while (*src && len--)
	{
		ASSERT(*src >= 0 && *src <= 127);
		*dest++ = *src++;
	}
	return old;
}

char * w2a_quick(wchar * src, char * dest, size_t len)
{
	char * old = dest;
	while (*src && len--)
	{
		ASSERT(*src <= 127);
		*dest++ = (char)(*src++);
	}
	return old;
}

double round(double num, int n) //四舍五入
{
	double iFactor = pow((double)10, n); // 计算出因子, 需要 #include <math.h>
	num *= iFactor;
  	num += 0.5;
  	num = (int)num;
  	num /= iFactor;
	return num;
}

namespace inner {
inline int distance_sub(const wchar * x, int xi, const wchar * y, int yi, bool icase) { return icase ? (util::tolower(x[xi]) == util::tolower(y[yi]) ? 0 : 1) : (x[xi] == y[yi] ? 0 : 1); }
inline int distance_ins(const wchar * x, int xi) { return 1; }
inline int distance_del(const wchar * x, int xi) { return 1; }
inline int distance_min(int a, int b, int c) { return min(min(a, b), c); }
};

size_t distance(const wchar * x, size_t xlen, const wchar * y, size_t ylen, bool icase)
{
	
    size_t & m = xlen;
    size_t & n = ylen;
	size_t i, j;
	
	size_t ** T = new size_t *[m+1];
	for(i=0; i <= m; ++i)
		T[i] = new size_t[n+1];
	
    T[0][0] = 0;
    for(i=0; i<n; ++i)
		T[0][i+1] = T[0][i] + inner::distance_ins(y, i);

    for(i=0; i<m; ++i)
	{
		T[i+1][0] = T[i][0] + inner::distance_del(x, i);
		for(j=0; j<n; ++j)
		{
			T[i+1][j+1] =  inner::distance_min(
				T[i][j] + inner::distance_sub(x, i, y, j, icase),
				T[i][j+1] + inner::distance_del(x, i),
				T[i+1][j] + inner::distance_ins(y, j)
				);
		}
    }
	
	size_t ret = T[m][n];
	for(i=0; i<m+1; ++i)
		delete [] T[i];
	delete [] T;
    return ret;
}



tchar * tstrcpy(tchar * dest, const tchar * source, unsigned int count)
{
	tchar * start = dest;
	while (count && (*dest++ = *source++))    // copy string
		count--;
	if (0 == count)
		*dest = '\0';
	return start;
}

int xtoi(const char * in, unsigned int len)
{
	const char * s = in;
	unsigned int v = 0;
	int sign = 1; // sign of positive
	char m = 10; // base of 0
	char t = '9'; // cap top of numbers at 9

	if (*s == '-')
	{
		s++;  //skip over -
		sign = -1; // sign flip
	}

	if (*s == '0')
	{
		s++; // skip over 0
		if (s[0] >= '0' && s[0] <= '7')
		{
			m = 8; // base of 8
			t = '7'; // cap top at 7
		}
		if ((s[0] & ~0x20) == 'X')
		{
			m = 16; // base of 16
			s++; // advance over 'x'
		}
	}

	for (; (unsigned int)(s - in) < len; s++)
	{
		int c = *s;
		if (c >= '0' && c <= t) c-='0';
		else if (m == 16 && (c & ~0x20) >= 'A' && (c & ~0x20) <= 'F') c = (c & 7) + 9;
		else break;
		v *= m;
		v += c;
	}
	return ((int)v) * sign;
}

int xtoi(const wchar * in, unsigned int len)
{
	const wchar * s = in;
	unsigned int v = 0;
	int sign = 1; // sign of positive
	char m = 10; // base of 0
	char t = '9'; // cap top of numbers at 9

	if (*s == '-')
	{
		s++;  //skip over -
		sign = -1; // sign flip
	}

	if (*s == '0')
	{
		s++; // skip over 0
		if (s[0] >= '0' && s[0] <= '7')
		{
			m = 8; // base of 8
			t = '7'; // cap top at 7
		}
		if ((s[0] & ~0x20) == 'X')
		{
			m = 16; // base of 16
			s++; // advance over 'x'
		}
	}

	for (; (unsigned int)(s - in) < len; s++)
	{
		int c = *s;
		if (c >= '0' && c <= t) c-='0';
		else if (m == 16 && (c & ~0x20) >= 'A' && (c & ~0x20) <= 'F') c = (c & 7) + 9;
		else break;
		v *= m;
		v += c;
	}
	return ((int)v) * sign;
}

void bin2string(const byte * data, size_t length, tstring & out)
{
	out.resize(length * 2);
	tchar * p = (tchar *)out.c_str();
	for (; 0 != length; ++data, --length, p += 2)
	{
		if (*data < 0x10)
		{
			*p = '0';
			itox((int)(*data), p + 1, 16);
		}
		else
		{
			itox((int)(*data), p, 16);
		}
	}
}

void string2bin(const tchar * data, vector<byte> & out)
{
	size_t len = _tcslen(data) / 2;
	out.resize(len);
	tchar tmp[5] = _T("0x");
	for (size_t i = 0; i < len; ++i)
	{
		tmp[2] = *data++;
		tmp[3] = *data++;
		tmp[4] = 0;
		out[i] = (byte)util::xtoi(tmp, 4);
	}
}

void split(const wchar * pstr, size_t len, vector<const wchar *> & out)
{
	out.clear();
	const wchar * p = pstr;
	while ((size_t)(p - pstr) < len)
	{
		while (0 == *p && (size_t)(p - pstr) < len)
		{
			++p;
		}
		while (*p && (size_t)(p - pstr) < len)
		{
			++p;
		}
		if (0 == *p)
		{
			out.push_back(p);
		}
	}
}

void split(const tstring str, const tchar * separator, vptstring & out, bool trim_string)
{
	out.clear();
	if (str.length() == 0)
	{
		return;
	}
	string::size_type p1 = 0, p2 = 0;
	while ((p2 = str.find(separator, p1)) != string::npos)
	{
		if (p1 != p2)
		{
			ptstring pstr;
			pstr->assign(str, p1, p2 - p1);
			if (trim_string)
			{
				trim(*pstr, _T(" \t\r\n"));
			}
			out.push_back(pstr);
		}
		p1 = p2 + 1;
		if (p1 >= str.length())
		{
			break;
		}
	}

	if (p1 < str.length())
	{
		ptstring pstr;
		pstr->assign(str, p1, str.length() - p1);
		if (trim_string)
		{
			trim(*pstr, _T(" \t\r\n"));
		}
		out.push_back(pstr);
	}
}


}; //end of namespace util
//=========================================================================
//					Utilitys namespace
//=========================================================================


