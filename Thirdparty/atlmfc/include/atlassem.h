/***
*atlassem.h - Libraries Assembly information
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file has information about Libraries Assembly version.
*
*
****/

#pragma once

#ifndef _VC_ASSEMBLY_PUBLICKEYTOKEN
#define _VC_ASSEMBLY_PUBLICKEYTOKEN "1fc8b3b9a1e18e3b"
#endif

#if !defined(_BIND_TO_CURRENT_VCLIBS_VERSION)
  #define _BIND_TO_CURRENT_VCLIBS_VERSION 0
#endif

#if !defined(_BIND_TO_CURRENT_ATL_VERSION)
  #if _BIND_TO_CURRENT_VCLIBS_VERSION
    #define _BIND_TO_CURRENT_ATL_VERSION 1
  #else
    #define _BIND_TO_CURRENT_ATL_VERSION 0
  #endif
#endif

#ifndef _ATL_ASSEMBLY_VERSION
#if _BIND_TO_CURRENT_ATL_VERSION
#define _ATL_ASSEMBLY_VERSION "9.0.30729.4148"
#else
#define _ATL_ASSEMBLY_VERSION "9.0.21022.8"
#endif
#endif

#ifndef __LIBRARIES_ASSEMBLY_NAME_PREFIX
#define __LIBRARIES_ASSEMBLY_NAME_PREFIX "Microsoft.VC90"
#endif

#if _MSC_FULL_VER >= 140040130

#ifdef _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' "        \
"name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".ATL' "              \
"version='" _ATL_ASSEMBLY_VERSION "' "                          \
"processorArchitecture='x86' "                                  \
"publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#ifdef _M_AMD64
#pragma comment(linker,"/manifestdependency:\"type='win32' "        \
"name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".ATL' "              \
"version='" _ATL_ASSEMBLY_VERSION "' "                          \
"processorArchitecture='amd64' "                                \
"publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#ifdef _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' "        \
"name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".ATL' "              \
"version='" _ATL_ASSEMBLY_VERSION "' "                          \
"processorArchitecture='ia64' "                                 \
"publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#endif
