#ifndef __MAIN_H
#define __MAIN_H
#include "stdafx.h"
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <Wininet.h>
#include "utils.h"
#include "MarkupSTL.h"

typedef __int64 t_int64;
typedef unsigned __int64 t_uint64;
typedef __int32 t_int32;
typedef unsigned __int32 t_uint32;
typedef __int16 t_int16;
typedef unsigned __int16 t_uint16;
typedef __int8 t_int8;
typedef unsigned __int8 t_uint8;

namespace pfc {
    void byteswap_raw(void * p_buffer,size_t p_bytes);

    template<typename T> T byteswap_t(T p_source);

    template<> inline char byteswap_t<char>(char p_source) {return p_source;}
    template<> inline unsigned char byteswap_t<unsigned char>(unsigned char p_source) {return p_source;}

#ifdef _MSC_VER//does this even help with performance/size?
    template<> inline wchar_t byteswap_t<wchar_t>(wchar_t p_source) {return _byteswap_ushort(p_source);}

    template<> inline short byteswap_t<short>(short p_source) {return _byteswap_ushort(p_source);}
    template<> inline unsigned short byteswap_t<unsigned short>(unsigned short p_source) {return _byteswap_ushort(p_source);}

    template<> inline int byteswap_t<int>(int p_source) {return _byteswap_ulong(p_source);}
    template<> inline unsigned int byteswap_t<unsigned int>(unsigned int p_source) {return _byteswap_ulong(p_source);}

    template<> inline long byteswap_t<long>(long p_source) {return _byteswap_ulong(p_source);}
    template<> inline unsigned long byteswap_t<unsigned long>(unsigned long p_source) {return _byteswap_ulong(p_source);}

    template<> inline long long byteswap_t<long long>(long long p_source) {return _byteswap_uint64(p_source);}
    template<> inline unsigned long long byteswap_t<unsigned long long>(unsigned long long p_source) {return _byteswap_uint64(p_source);}
#else
    template<> inline t_uint16 byteswap_t<t_uint16>(t_uint16 p_source) {return ((p_source & 0xFF00) >> 8) | ((p_source & 0x00FF) << 8);}
    template<> inline t_int16 byteswap_t<t_int16>(t_int16 p_source) {return byteswap_t<t_uint16>(p_source);}

    template<> inline t_uint32 byteswap_t<t_uint32>(t_uint32 p_source) {return ((p_source & 0xFF000000) >> 24) | ((p_source & 0x00FF0000) >> 8) | ((p_source & 0x0000FF00) << 8) | ((p_source & 0x000000FF) << 24);}
    template<> inline t_int32 byteswap_t<t_int32>(t_int32 p_source) {return byteswap_t<t_uint32>(p_source);}

    template<> inline t_uint64 byteswap_t<t_uint64>(t_uint64 p_source) {
        //optimizeme
        byteswap_raw(&p_source,sizeof(p_source));
        return p_source;		
    }
    template<> inline t_int64 byteswap_t<t_int64>(t_int64 p_source) {return byteswap_t<t_uint64>(p_source);}

    template<> inline wchar_t byteswap_t<wchar_t>(wchar_t p_source) {
        return byteswap_t<pfc::sized_int_t<sizeof(wchar_t)>::t_unsigned>(p_source);
    }
#endif

    template<> inline float byteswap_t<float>(float p_source) {
        float ret;
        *(t_uint32*) &ret = byteswap_t(*(const t_uint32*)&p_source );
        return ret;
    }

    template<> inline double byteswap_t<double>(double p_source) {
        double ret;
        *(t_uint64*) &ret = byteswap_t(*(const t_uint64*)&p_source );
        return ret;
    }

    //blargh at GUID byteswap issue
    template<> inline GUID byteswap_t<GUID>(GUID p_guid) {
        GUID ret;
        ret.Data1 = pfc::byteswap_t(p_guid.Data1);
        ret.Data2 = pfc::byteswap_t(p_guid.Data2);
        ret.Data3 = pfc::byteswap_t(p_guid.Data3);
        ret.Data4[0] = p_guid.Data4[0];
        ret.Data4[1] = p_guid.Data4[1];
        ret.Data4[2] = p_guid.Data4[2];
        ret.Data4[3] = p_guid.Data4[3];
        ret.Data4[4] = p_guid.Data4[4];
        ret.Data4[5] = p_guid.Data4[5];
        ret.Data4[6] = p_guid.Data4[6];
        ret.Data4[7] = p_guid.Data4[7];
        return ret;
    }



};


class window_config
{
public:
    window_config() ;
    int cfg_timeout;
    int cfg_proxy_setting;
    tstring cfg_proxy_address;
    tstring cfg_proxy_username;
    tstring cfg_proxy_password;
    bool cfg_check_assume_charset;
    int cfg_assume_charset;
    int cfg_assume_charset_sel;
};


char* pfc_format_hex(t_uint64 p_val,unsigned int p_width = 0);

bool ReadInternetText(tstring& output, tstring userAgent, tstring host, INTERNET_PORT port, tstring request, tstring postdata, bool isPostRequest, DWORD timeout, window_config *wcfg);
bool ReadInternetTextFromUrl(tstring& output, tstring userAgent, tstring url, DWORD timeout, window_config *wcfg);
tstring ReplaceString(const tstring &stringSearchString, const tstring &stringReplaceString, tstring stringStringToReplace);

#endif