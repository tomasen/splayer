# This is a part of the Active Template Library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Active Template Library Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Active Template Library product.

!if "$(MFC_VER)" == ""
MFC_VER=90
!endif

!if "$(LIBNAME)" != ""
_LIBNAME = LIBNAME=$(LIBNAME)
!endif

!if "$(PLATFORM)" == ""
!message setting PLATFORM=INTEL
PLATFORM=INTEL
!endif

!if "$(CLEAN)" != ""
_CLEAN = CLEAN
!else
_CLEAN =
!endif

!if !exist(atlmfc.mak)
!error Run this makefile from the directory it is loacted in.
!endif


all : ATLSRC MFCSRC

atl : ATLSRC

mfc : MFCSRC

createdir :
# create or clean destination directory
!if "$(CLEAN)" == ""
	-if not exist ..\lib\$(PLATFORM)$(_OD_EXT) md ..\lib\$(PLATFORM)$(_OD_EXT)
!endif

ATLSRC : ATL_ASSMINFO ATL_IDL ATL_STATIC_RELEASE ATL_STATIC_DEBUG ATL_UNICODE_RELEASE ATL_UNICODE_DEBUG ATL_ANSI_RELEASE ATL_ANSI_DEBUG

# build atlXX.dll
# Unicode release
ATL_UNICODE_RELEASE : createdir
	cd atl
	$(MAKE) /$(MAKEFLAGS) /f atldll.mak UNICODE=1 DEBUG=0 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM) _CL_ATL_CHECK_MANIFEST_=$(_CL_ATL_CHECK_MANIFEST_)
!if "$(CLEAN)" == ""
	copy $(PLATFORM)\releaseU$(_OD_EXT)\atl.lib ..\..\lib\$(PLATFORM)$(_OD_EXT)
	copy $(PLATFORM)\releaseU$(_OD_EXT)\atl$(MFC_VER)$(ATL_PDB_VERSION_NAME).pdb ..\..\lib\$(PLATFORM)$(_OD_EXT)
!endif
	cd ..

# Unicode Debug
ATL_UNICODE_DEBUG : createdir
	cd atl
	$(MAKE) /$(MAKEFLAGS) /f atldll.mak UNICODE=1 DEBUG=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM) _CL_ATL_CHECK_MANIFEST_=$(_CL_ATL_CHECK_MANIFEST_)
	cd ..

# Ansi Release
ATL_ANSI_RELEASE : createdir
	cd atl
	$(MAKE) /$(MAKEFLAGS) /f atldll.mak UNICODE=0 DEBUG=0 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM) _CL_ATL_CHECK_MANIFEST_=$(_CL_ATL_CHECK_MANIFEST_)
	cd ..

# Ansi Debug
ATL_ANSI_DEBUG : createdir
	cd atl
	$(MAKE) /$(MAKEFLAGS) /f atldll.mak UNICODE=0 DEBUG=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM) _CL_ATL_CHECK_MANIFEST_=$(_CL_ATL_CHECK_MANIFEST_)
	cd ..

# build Atl static lib
# atls.lib
ATL_STATIC_RELEASE : createdir BUILD_STATIC_RELEASE COPY_STATIC_RELEASE

BUILD_STATIC_RELEASE :
	cd atl\atls
	$(MAKE) /$(MAKEFLAGS) /f atls.mak DEBUG=0 _OD_EXT=$(_OD_EXT) UNICODE= $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..\..

!if "$(CLEAN)" == ""
COPY_STATIC_RELEASE : ..\lib\$(PLATFORM)$(_OD_EXT)\atls.lib

..\lib\$(PLATFORM)$(_OD_EXT)\atls.lib: atl\atls\$(PLATFORM)\release$(_OD_EXT)\atls.lib
	copy atl\atls\$(PLATFORM)\release$(_OD_EXT)\atls.lib ..\lib\$(PLATFORM)$(_OD_EXT)
	copy atl\atls\$(PLATFORM)\release$(_OD_EXT)\atls.pdb ..\lib\$(PLATFORM)$(_OD_EXT)
	
!else
COPY_STATIC_RELEASE : 
!endif
	
# atlsd.lib
ATL_STATIC_DEBUG : createdir BUILD_STATIC_DEBUG COPY_STATIC_DEBUG

BUILD_STATIC_DEBUG :
	cd atl\atls
	$(MAKE) /$(MAKEFLAGS) /f atls.mak DEBUG=1 _OD_EXT=$(_OD_EXT) UNICODE= $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..\..

!if "$(CLEAN)" == ""
COPY_STATIC_DEBUG : ..\lib\$(PLATFORM)$(_OD_EXT)\atlsd.lib

..\lib\$(PLATFORM)$(_OD_EXT)\atlsd.lib: atl\atls\$(PLATFORM)\debug$(_OD_EXT)\atlsd.lib
	copy atl\atls\$(PLATFORM)\debug$(_OD_EXT)\atlsd.lib ..\lib\$(PLATFORM)$(_OD_EXT)
	copy atl\atls\$(PLATFORM)\debug$(_OD_EXT)\atlsd.pdb ..\lib\$(PLATFORM)$(_OD_EXT)
	
!else
COPY_STATIC_DEBUG :
!endif

# buils atl.tlb from atl.idl	
ATL_IDL :
	cd atl
	$(MAKE) /$(MAKEFLAGS) /f atlidl.mak DEBUG=0 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFCSRC : MFC_ASSMINFO MFC_STATIC MFC_DLL MFC_LOC MFCM_DLL

# static libs	
MFC_STATIC: MFC_ANSI_STATIC_RELEASE MFC_ANSI_STATIC_DEBUG MFC_UNICODE_STATIC_RELEASE MFC_UNICODE_STATIC_DEBUG

MFC_ANSI_STATIC_RELEASE: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) DEBUG=0 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFC_ANSI_STATIC_DEBUG: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) DEBUG=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFC_UNICODE_STATIC_RELEASE: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) DEBUG=0 UNICODE=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..

MFC_UNICODE_STATIC_DEBUG: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) DEBUG=1 UNICODE=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	

# dlls
MFC_DLL: MFC_ANSI_RELEASE MFC_ANSI_DEBUG MFC_UNICODE_RELEASE MFC_UNICODE_DEBUG

MFC_ANSI_RELEASE: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) /f mfcdll.mak $(_LIBNAME) debug=0 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFC_ANSI_DEBUG: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) /f mfcdll.mak $(_LIBNAME) debug=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFC_UNICODE_RELEASE: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) /f mfcdll.mak $(_LIBNAME) debug=0 UNICODE=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFC_UNICODE_DEBUG: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) /f mfcdll.mak $(_LIBNAME) debug=1 UNICODE=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..

# managed dlls
MFCM_DLL: MFCM_ANSI_RELEASE MFCM_ANSI_DEBUG MFCM_UNICODE_RELEASE MFCM_UNICODE_DEBUG

MFCM_ANSI_RELEASE: createdir
	cd MFCM
	$(MAKE) /$(MAKEFLAGS) /f mfcmdll.mak $(_LIBNAME) debug=0 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFCM_ANSI_DEBUG: createdir
	cd MFCM
	$(MAKE) /$(MAKEFLAGS) /f mfcmdll.mak $(_LIBNAME) debug=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFCM_UNICODE_RELEASE: createdir
	cd MFCM
	$(MAKE) /$(MAKEFLAGS) /f mfcmdll.mak $(_LIBNAME) debug=0 UNICODE=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..
	
MFCM_UNICODE_DEBUG: createdir
	cd MFCM
	$(MAKE) /$(MAKEFLAGS) /f mfcmdll.mak $(_LIBNAME) debug=1 UNICODE=1 _OD_EXT=$(_OD_EXT) $(_CLEAN) PLATFORM=$(PLATFORM)
	cd ..

# localized dlls
MFC_LOC: createdir
	cd MFC
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak fra
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak deu
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak jpn
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak ita
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak esp
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak chs
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak cht
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak kor
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak rus
	$(MAKE) /$(MAKEFLAGS) /f mfcintl.mak enu
	cd ..

!if !defined(__LIBRARIES_ASSEMBLY_NAME_PREFIX) || "$(__LIBRARIES_ASSEMBLY_NAME_PREFIX)" == ""
__LIBRARIES_ASSEMBLY_NAME_PREFIX=Microsoft.VC90
!endif

!if !defined(_ATL_ASSEMBLY_VER_BUILD) || "$(_ATL_ASSEMBLY_VER_BUILD)" == ""
_ATL_ASSEMBLY_VER_BUILD=0
!endif

!if !defined(_ATL_ASSEMBLY_VER_RBLD) || "$(_ATL_ASSEMBLY_VER_RBLD)" == ""
_ATL_ASSEMBLY_VER_RBLD=0
!endif

!if !defined(_MFC_ASSEMBLY_VER_BUILD) || "$(_MFC_ASSEMBLY_VER_BUILD)" == ""
_MFC_ASSEMBLY_VER_BUILD=0
!endif

!if !defined(_MFC_ASSEMBLY_VER_RBLD) || "$(_MFC_ASSEMBLY_VER_RBLD)" == ""
_MFC_ASSEMBLY_VER_RBLD=0
!endif

!if !defined(ASSEMBLY_VERSION_PART_1_2) || "$(ASSEMBLY_VERSION_PART_1_2)" == ""
ASSEMBLY_VERSION_PART_1_2=9.0
!endif

!if !defined(VC_ASSEMBLY_PUBLICKEYTOKEN) || "$(VC_ASSEMBLY_PUBLICKEYTOKEN)" == ""
VC_ASSEMBLY_PUBLICKEYTOKEN=1fc8b3b9a1e18e3b
!endif

!if "$(CLEAN)" == ""
ATL_ASSMINFO : ..\include\ATLassem.h
!else
ATL_ASSMINFO : 
	if exist ..\include\ATLassem.h del ..\include\ATLassem.h
!endif

..\include\atlassem.h :
	type > nul <<$@
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
#define _VC_ASSEMBLY_PUBLICKEYTOKEN "$(VC_ASSEMBLY_PUBLICKEYTOKEN)"
#endif

#ifndef _ATL_ASSEMBLY_VERSION
#define _ATL_ASSEMBLY_VERSION "$(ASSEMBLY_VERSION_PART_1_2).$(_ATL_ASSEMBLY_VER_BUILD).$(_ATL_ASSEMBLY_VER_RBLD)"
#endif

#ifndef __LIBRARIES_ASSEMBLY_NAME_PREFIX
#define __LIBRARIES_ASSEMBLY_NAME_PREFIX "$(__LIBRARIES_ASSEMBLY_NAME_PREFIX)"
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

<<KEEP

!if "$(CLEAN)" == "" || "$(CLEAN)" == "0"
MFC_ASSMINFO : ..\include\MFCassem.h
!else
MFC_ASSMINFO : 
	if exist ..\include\MFCassem.h del ..\include\MFCassem.h
!endif

..\include\MFCassem.h :
	type > nul <<$@
/***
*MFCassem.h - Libraries Assembly information
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
#define _VC_ASSEMBLY_PUBLICKEYTOKEN "$(VC_ASSEMBLY_PUBLICKEYTOKEN)"
#endif

#ifndef _MFC_ASSEMBLY_VERSION
#define _MFC_ASSEMBLY_VERSION "$(ASSEMBLY_VERSION_PART_1_2).$(_MFC_ASSEMBLY_VER_BUILD).$(_MFC_ASSEMBLY_VER_RBLD)"
#endif

#ifndef __LIBRARIES_ASSEMBLY_NAME_PREFIX
#define __LIBRARIES_ASSEMBLY_NAME_PREFIX "$(__LIBRARIES_ASSEMBLY_NAME_PREFIX)"
#endif

#if _MSC_FULL_VER >= 140040130

#ifdef _DEBUG

#ifdef _M_IX86
    #pragma comment(linker,"/manifestdependency:\"type='win32' "        \
        "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".DebugMFC' "         \
        "version='" _MFC_ASSEMBLY_VERSION "' "                          \
        "processorArchitecture='x86' "                                  \
        "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#ifdef _M_AMD64
    #pragma comment(linker,"/manifestdependency:\"type='win32' "        \
        "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".DebugMFC' "         \
        "version='" _MFC_ASSEMBLY_VERSION "' "                          \
        "processorArchitecture='amd64' "                                \
        "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#ifdef _M_IA64
    #pragma comment(linker,"/manifestdependency:\"type='win32' "        \
        "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".DebugMFC' "         \
        "version='" _MFC_ASSEMBLY_VERSION "' "                          \
        "processorArchitecture='ia64' "                                 \
        "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#else

#ifdef _M_IX86
    #pragma comment(linker,"/manifestdependency:\"type='win32' "        \
        "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".MFC' "              \
        "version='" _MFC_ASSEMBLY_VERSION "' "                          \
        "processorArchitecture='x86' "                                  \
        "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#ifdef _M_AMD64
    #pragma comment(linker,"/manifestdependency:\"type='win32' "        \
        "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".MFC' "              \
        "version='" _MFC_ASSEMBLY_VERSION "' "                          \
        "processorArchitecture='amd64' "                                \
        "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#ifdef _M_IA64
    #pragma comment(linker,"/manifestdependency:\"type='win32' "        \
        "name='" __LIBRARIES_ASSEMBLY_NAME_PREFIX ".MFC' "              \
        "version='" _MFC_ASSEMBLY_VERSION "' "                          \
        "processorArchitecture='ia64' "                                 \
        "publicKeyToken='" _VC_ASSEMBLY_PUBLICKEYTOKEN "'\"")
#endif

#endif

#endif	// _DEBUG

<<KEEP
