# This is a part of the Microsoft Foundation Classes C++ library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Microsoft Foundation Classes Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Microsoft Foundation Classes product.

# MFCMXX[D].DLL is a DLL
#  which exports managed MFC classes
#
# If you need a private build of the MFC DLL, be sure to rename
#  "MFCXX.DLL" to something more appropriate for your application.
# Please do not re-distribute a privately built version with the
#  name "MFCMXX.DLL".
#
# Use nmake /f mfcmdll.mak LIBNAME=<your name> to do this.
#
# Note: LIBNAME must be 6 characters or less.

!ifndef LIBNAME
!error LIBNAME is not defined. LIBNAME=MFC90 builds the prebuilt DLL.
!endif
MLIBNAME=$(LIBNAME:MFC90=MFCM90)

!ifndef PLATFORM
!message ******** setting PLATFORM=INTEL ********
PLATFORM=INTEL
!endif

!if "$(MFC_VER)" == ""
MFC_VER=90
!endif

!ifndef REGEN
REGEN=0
!endif

!if "$(DEBUG)" != "0"
CRTDLL=msvcrtd.lib
CRTMDLL=msvcmrtd.lib
!else
CRTDLL=msvcrt.lib
CRTMDLL=msvcmrt.lib
!endif

TARGET=w
DLL=2
MANAGED=1
TARGDEFS=/D_AFXEXT /DBUILD_MFCM
TARGOPTS=/AI $D /AI ../mfc/$(PLATFORM) /I../../include /I../mfc /I.
#LFLAGS=/nodefaultlib
RCFLAGS=/r

!ifndef PREBUILT
PREBUILT=0
!endif

!if "$(UNICODE)" == "1"
TARGTYPE=U
RCDEFINES=$(RCDEFINES) /D_UNICODE
!else
!endif

!IF "$(DEBUG)" != "0"
# Debug DLL build
TARGTYPE=$(TARGTYPE)D
RCDEFINES=$(RCDEFINES) /D_DEBUG
LFLAGS=$(LFLAGS)
!ELSE
# Release DLL build
RCDEFINES=$(RCDEFINES)
LFLAGS=$(LFLAGS)
!ENDIF

CFNAME=$(MLIBNAME)$(TARGTYPE)
TARG=$(MLIBNAME)$(PF)$(TARGTYPE)
MFCLIB=$(LIBNAME)$(PF)$(TARGTYPE).lib

# OPT:noref keeps unreferenced functions (ie. no dead-code elimination)
!if "$(REGEN)" == "0"
LFLAGS=$(LFLAGS) /opt:ref /opt:icf,32
!else
LFLAGS=$(LFLAGS) /opt:noref
!endif

DEFFILE=$(PLATFORM)\MFCM$(MFC_VER)$(PF)$(TARGTYPE).DEF

!if "$(DEBUGTYPE)" == ""
DEBUGTYPE=cv
!endif

TARGETPDBNAME=$(TARG)$(MFC_PDB_VERSION_NAME).pdb
!if "$(CODEVIEW)" != "0"
!if "$(REGEN)" != "1"
LFLAGS=$(LFLAGS) /debug /debugtype:$(DEBUGTYPE)
!else
LFLAGS=$(LFLAGS) /debug:none
!endif
!if "$(NO_PDB)" != "1" && "$(REGEN)" != "1"
LFLAGS=$(LFLAGS) /pdb:$(PLATFORM)\$(TARGETPDBNAME)
# !else
#LFLAGS=$(LFLAGS) /pdb:none
!endif
!else
LFLAGS=$(LFLAGS) /debug:none
!if "$(INCREMENTAL)" != "1"
LFLAGS=$(LFLAGS) /incremental:no
!endif
!endif

!ifdef RELEASE # Release VERSION info
RCDEFINES=$(RCDEFINES) /DRELEASE
LFLAGS=$(LFLAGS) /release
!endif

LFLAGS=$(LFLAGS) /dll

!if "$(ORDER)" == "1"
!if exist($(PLATFORM)\$(TARG).prf)
DEFS=$(DEFS) /D_AFX_FUNCTION_ORDER
LFLAGS=$(LFLAGS) /order:@$(PLATFORM)\$(TARG).prf
!endif
!endif

!if "$(PLATFORM)" == "PPC"
LIBS=$(LIBS) int64.lib /force:multiple
!endif

!if "$(PLATFORM)" == "AXP64"
LFLAGS=$(LFLAGS) /machine:ALPHA64
!endif

!if "$(CLEAN)" == ""

dll_goal: create2.dir \
	interfaces_goal \
	pch_goal \
	$(PLATFORM)\$(TARG).dll ..\..\lib\$(PLATFORM)\$(TARG).lib \
	create_alias_objs \
	deploy_at_mfc_binary_dir

cleanall:
	$(MAKE) /f mfcmdll.mak LIBNAME=$(LIBNAME) DEBUG=0 UNICODE=0 clean
	$(MAKE) /f mfcmdll.mak LIBNAME=$(LIBNAME) DEBUG=1 UNICODE=0 clean
	$(MAKE) /f mfcmdll.mak LIBNAME=$(LIBNAME) DEBUG=0 UNICODE=1 clean
	$(MAKE) /f mfcmdll.mak LIBNAME=$(LIBNAME) DEBUG=1 UNICODE=1 clean
	cd interfaces
	$(MAKE) DEBUG=0 CLEAN=1
	cd ..

!else

dll_goal: clean
	cd interfaces
	$(MAKE) DEBUG=0 CLEAN=1
	$(MAKE) DEBUG=1 CLEAN=1
	cd ..

!endif

# The following rule deploys MFCM DLLs at the MFC DLL directory.
# It is off by default, and could be turned on by setting the
#  environment variable MFCM_DEPLOY to 1.

deploy_at_mfc_binary_dir:
!if "$(MFCM_DEPLOY)" == "1"
	copy $(PLATFORM)\$(TARG).dll ..\mfc\$(PLATFORM)	
	copy $(PLATFORM)\$(TARGETPDBNAME) ..\mfc\$(PLATFORM)
!else

!endif

#############################################################################
# WinFormDesigner Helper Library

interfaces_goal:
	cd interfaces
	$(MAKE) DEBUG=0 MFCM_DEPLOY=$(MFCM_DEPLOY)
	cd ..

#############################################################################
# import most rules and library files from normal makefile

PCH_CPP=stdafx
DN=$(D)\N

!include ..\mfc\makefile

create2.dir:
	@-if not exist $D\*.* mkdir $D
	@-if not exist $(DN)\*.* mkdir $(DN)

pch_goal: $(PCH_TARGETS)

#############################################################################
# more flags and switches

LFLAGS=$(LFLAGS) /version:9.0
!if "$(UNICODE)" == "1"
!if "$(PLATFORM)" == "INTEL"
LFLAGS=$(LFLAGS) /base:0x78880000
!endif
!if "$(PLATFORM)" == "AMD64"
LFLAGS=$(LFLAGS) /base:0x78a90000
!endif
!if "$(PLATFORM)" == "IA64"
LFLAGS=$(LFLAGS) /base:0x79270000
!endif
!else
!if "$(PLATFORM)" == "INTEL"
LFLAGS=$(LFLAGS) /base:0x78860000
!endif
!if "$(PLATFORM)" == "AMD64"
LFLAGS=$(LFLAGS) /base:0x78a70000
!endif
!if "$(PLATFORM)" == "IA64"
LFLAGS=$(LFLAGS) /base:0x79240000
!endif
!endif

!if "$(PLATFORM)" == "IA64" || "$(PLATFORM)" == "AMD64"
LIBS=$(CRTDLL) $(CRTMDLL) mscoree.lib kernel32.lib gdi32.lib msimg32.lib user32.lib \
      uuid.lib htmlhelp.lib shlwapi.lib $(PROFLIB)
!else
LIBS=$(CRTDLL) $(CRTMDLL) mscoree.lib kernel32.lib gdi32.lib msimg32.lib user32.lib \
      uuid.lib daouuid.lib htmlhelp.lib shlwapi.lib $(PROFLIB)
!endif
#############################################################################

MANAGED_OBJS=$D\stdafx.obj $D\wfrmsite.obj $D\wfrmview.obj $D\wfrmcmd.obj $D\afxwfrminl.obj 

NATIVE_OBJS=$D\mfcm.obj

IMPLIB_MANAGED_OBJS=$D\postdllmain.obj $D\postrawdllmain.obj

IMPLIB_NATIVE_OBJS=

CPPFLAGS_NATIVE=$(CPPFLAGS:-d1clrNoPoundDefineWhitespaceKeyword=)
CPPFLAGS_NATIVE=$(CPPFLAGS_NATIVE:/clr=)

$(D)\mfcm.obj: mfcm.cpp
	cl $(CPPFLAGS_NATIVE) /clr- /Y- /c mfcm.cpp

$(D)\postdllmain.obj: postdllmain.cpp
	cl $(CPPFLAGS) /Y- /c postdllmain.cpp

$(D)\postrawdllmain.obj: postrawdllmain.cpp
	cl $(CPPFLAGS) /Y- /c postrawdllmain.cpp

#############################################################################
# Build target

$D\$(TARG).res: mfcm.rc ..\..\include\atlbuild.h
	rc $(RCFLAGS) $(RCDEFINES) /fo $D\$(TARG).res mfcm.rc

DLL_OBJS=$(MANAGED_OBJS) $(NATIVE_OBJS)

IMPLIB_OBJS=$(IMPLIB_MANAGED_OBJS) $(IMPLIB_NATIVE_OBJS)

DLL_RESOURCES=$D\$(TARG).res

!if "$(PLATFORM)" == "AXP64" || "$(PLATFORM)" == "IA64"
$(PLATFORM)\$(TARG).dll ..\..\lib\$(PLATFORM)\$(TARG).lib: $(DLL_OBJS) $(DLL_RESOURCES) $(IMPLIB_OBJS)
	link @<<
$(LFLAGS) /nologo
$(LIBS)
$(DLL_OBJS)
$(DLL_RESOURCES)
/def:$(DEFFILE)
/out:$(PLATFORM)\$(TARG).DLL
/map:$D\$(TARG).MAP
/implib:..\..\lib\$(PLATFORM)\$(TARG).LIB
/filealign:4096
/delayload:oleaut32.dll
delayimp.lib
mfcdload.lib
/NOD:olepro32.lib
/NOD:libcmt.lib
<<
	lib /nologo ..\..\lib\$(PLATFORM)\$(TARG).LIB $(IMPLIB_OBJS)
!else
$(PLATFORM)\$(TARG).dll ..\..\lib\$(PLATFORM)\$(TARG).lib: $(DLL_OBJS) $(DLL_RESOURCES) $(IMPLIB_OBJS)
	link @<<
$(LFLAGS)
$(LIBS)
$(DLL_OBJS)
$(DLL_RESOURCES)
/def:$(DEFFILE)
/out:$(PLATFORM)\$(TARG).DLL
/map:$D\$(TARG).MAP
/implib:..\..\lib\$(PLATFORM)\$(TARG).LIB
/ignore:4037
/ignore:4065
/ignore:4199
/merge:.rdata=.text
/delayload:oleaut32.dll
delayimp.lib
mfcdload.lib
<<
	lib /nologo ..\..\lib\$(PLATFORM)\$(TARG).LIB $(IMPLIB_OBJS)
!endif

create_alias_objs:
!if "$(REGEN)" != "1" && "$(PREBUILT)" == "1"
	cd $D
	dumpbin /exports $(MAKEDIR)\..\..\lib\$(PLATFORM)\$(TARG).LIB > mfcm.exp
	type mfcm.exp | perl $(_LIBSROOT)\nonship\build\mfcmdll\wcharthunk.pl -importlib  -platform $(PLATFORM)
	lib $(MAKEDIR)\..\..\lib\$(PLATFORM)\$(TARG).LIB _alias*.obj
	del _alias*.obj 2> nul
	cd $(MAKEDIR)
!else

!endif

#############################################################################
