# This is a part of the Microsoft Foundation Classes C++ library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Microsoft Foundation Classes Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Microsoft Foundation Classes product.

# MFCXX[D].DLL is a DLL
#  which exports all the MFC classes
#
# If you need a private build of the MFC DLL, be sure to rename
#  "MFC90.DLL" to something more appropriate for your application.
# Please do not re-distribute a privately built version with the
#  name "MFC90.DLL".
#
# Use nmake /f mfcdll.mak LIBNAME=<your name> to do this.
#
# Note: LIBNAME must be 6 characters or less.

!ifndef LIBNAME
!error LIBNAME is not defined. LIBNAME=MFC90 builds the prebuilt DLL.
!endif

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
!else
CRTDLL=msvcrt.lib
!endif

TARGET=w
DLL=2
TARGDEFS=/D_AFX_CORE_IMPL
#LFLAGS=/nodefaultlib
RCFLAGS=/r

!if "$(UNICODE)" == "1"
TARGTYPE=U
RCDEFINES=$(RCDEFINES) /D_UNICODE
!else
!endif

!if "$(DEBUG)" != "0"
# Debug DLL build
TARGTYPE=$(TARGTYPE)D
RCDEFINES=$(RCDEFINES) /D_DEBUG
LFLAGS=$(LFLAGS)
!ifndef MONOLITHIC
MONOLITHIC=1
!endif
!ELSE
# Release DLL build
RCDEFINES=$(RCDEFINES)
LFLAGS=$(LFLAGS) /LTCG
!ifndef MONOLITHIC
#!if "$(PLATFORM)" == "IA64"
#MONOLITHIC=0
#!else
MONOLITHIC=1
#!endif
!endif
!ENDIF

!if "$(MONOLITHIC)" == "1"
TARGDEFS=$(TARGDEFS) /D_AFX_OLE_IMPL /D_AFX_DB_IMPL /D_AFX_NET_IMPL /D_AFX_MONOLITHIC /D_MFC_DLL_BLD
RCDEFINES=$(RCDEFINES) /D_AFX_MONOLITHIC
!endif

CFNAME=$(LIBNAME)$(TARGTYPE)
TARG=$(LIBNAME)$(PF)$(TARGTYPE)
# if LIBNAME=="mfc90" then static lib is named as in afx.h mfcs90.lib.
# if user chose another LIBNAME, then she will have to modify afx.h in her private build.
!if "$(LIBNAME)" != "MFC$(MFC_VER)" && "$(LIBNAME)" != "mfc$(MFC_VER)"
TARG_STATIC=$(LIBNAME)S$(PF)$(TARGTYPE)
!else
TARG_STATIC=MFCS$(MFC_VER)$(PF)$(TARGTYPE)
!endif

# OPT:noref keeps unreferenced functions (ie. no dead-code elimination)
!if "$(REGEN)" == "0"
!if "$(DEBUG)" != "0"
LFLAGS=$(LFLAGS) /opt:ref
!else
LFLAGS=$(LFLAGS) /opt:ref /opt:icf,32
!endif
!else
LFLAGS=$(LFLAGS) /opt:noref
!endif

DEFFILE=$(PLATFORM)\MFC$(MFC_VER)$(PF)$(TARGTYPE).DEF

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

# If other platforms that support safeseh are added, need to change here. amd64 and ia64 do not 
# support (or require) this switch.
!if "$(PLATFORM)" == "INTEL"
LFLAGS=$(LFLAGS) /SAFESEH
!endif

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

dll_goal: create2.dir \
	$(PLATFORM)\$(TARG).dll ..\..\lib\$(PLATFORM)\$(TARG).lib ..\..\lib\$(PLATFORM)\$(TARG_STATIC).lib

#############################################################################
# import most rules and library files from normal makefile

!include makefile

create2.dir:
	@-if not exist $D\*.* mkdir $D

#############################################################################
# more flags and switches

LFLAGS=$(LFLAGS) /version:9.0
!if "$(UNICODE)" == "1"
!if "$(PLATFORM)" == "INTEL"
LFLAGS=$(LFLAGS) /base:0x78720000
!endif
!if "$(PLATFORM)" == "AMD64"
LFLAGS=$(LFLAGS) /base:0x788a0000
!endif
!if "$(PLATFORM)" == "IA64"
LFLAGS=$(LFLAGS) /base:0x78e50000
!endif
!else
!if "$(PLATFORM)" == "INTEL"
LFLAGS=$(LFLAGS) /base:0x785e0000
!endif
!if "$(PLATFORM)" == "AMD64"
LFLAGS=$(LFLAGS) /base:0x786d0000
!endif
!if "$(PLATFORM)" == "IA64"
LFLAGS=$(LFLAGS) /base:0x78a60000
!endif
!endif

!if "$(PLATFORM)" == "IA64" || "$(PLATFORM)" == "AMD64"
LIBS=$(CRTDLL) kernel32.lib gdi32.lib msimg32.lib user32.lib uuid.lib htmlhelp.lib shlwapi.lib winmm.lib imm32.lib $(PROFLIB)
!else
LIBS=$(CRTDLL) kernel32.lib gdi32.lib msimg32.lib user32.lib uuid.lib htmlhelp.lib shlwapi.lib winmm.lib imm32.lib $(PROFLIB) daouuid.lib
!endif
#############################################################################

STATICLINK_OBJS=$D\stdafx.obj $D\nolib.obj \
	$D\appmodul.obj $D\dumpstak.obj $D\dllmodul.obj $D\dllmodulX.obj $D\rawdllmainproxy.obj $D\oleexp.obj $D\sockexp.obj

# Always compile /Zi (generate PDB) for mfcs90[u][d].lib, since we never get a chance to have the
# linker generate a PDB from /Z7 info

CVOPTS_STATIC=/Zi

CPPFLAGS_STATIC_COMMON=\
	$(CL_MODEL) /D_AFX_MFCS $(CL_OPT:Z7=Zi) $(DEFS:/D_MFC_DLL_BLD=) $(OPT) $(EH) /Gy $(CVOPTS_STATIC) /Fd..\..\lib\$(PLATFORM)\$(TARG_STATIC).pdb \
	-D_AFX_NOFORCE_MANIFEST -D_ATL_NOFORCE_MANIFEST -D_CRT_NOFORCE_MANIFEST

PCH_FLAGS_STATIC=/Yustdafx.h /Fp$D\stdafxs.pch
CPPFLAGS_STATIC = $(CL_OPT_OUTDIR) $(CPPFLAGS_STATIC_COMMON) $(PCH_FLAGS_STATIC)
CPPFLAGS_STATIC_DLLMODUL = $(CL_OPT_OUTDIR) $(CPPFLAGS_STATIC_COMMON)
CPPFLAGS_STATIC_RAWDLLMAINPROXY = $(CL_OPT_OUTDIR) $(CPPFLAGS_STATIC_COMMON)
CPPFLAGS_STATIC_DLLMODULX = $(CL_OPT_OUTDIR)dllmodulX.obj $(CPPFLAGS_STATIC_COMMON) $(PCH_FLAGS_STATIC)

PCH_TARGET=$D\stdafx.obj

$D\stdafx.obj $D\stdafxs.pch: stdafx.cpp stdafx.h
	cl @<<
/c $(CPPFLAGS_STATIC:/Yu=/Yc) stdafx.cpp
<<

$D\appmodul.obj: $D\stdafx.obj appmodul.cpp
	cl @<<
/c $(CPPFLAGS_STATIC) appmodul.cpp
<<

$D\dllmodul.obj: $D\stdafx.obj dllmodul.cpp
	cl @<<
/c $(CPPFLAGS_STATIC_DLLMODUL) /D_AFX_DISABLE_INLINES dllmodul.cpp
<<

$D\rawdllmainproxy.obj: rawdllmainproxy.c
	cl @<<
/c $(CPPFLAGS_STATIC_RAWDLLMAINPROXY) rawdllmainproxy.c
<<

$D\dllmodulX.obj: $D\stdafx.obj dllmodul.cpp
	cl @<<
/c $(CPPFLAGS_STATIC_DLLMODULX) /D_AFX_DLLMODULE_HELPER dllmodul.cpp
<<

$D\oleexp.obj: $D\stdafx.obj oleexp.cpp
	cl @<<
/c $(CPPFLAGS_STATIC) oleexp.cpp
<<

$D\nolib.obj: $D\stdafx.obj nolib.cpp
	cl @<<
/c $(CPPFLAGS_STATIC) nolib.cpp
<<

$D\dumpstak.obj: $D\stdafx.obj dumpstak.cpp
	cl @<<
/c $(CPPFLAGS_STATIC) dumpstak.cpp
<<

$D\sockexp.obj: $D\stdafx.obj sockexp.cpp
	cl @<<
/c $(CPPFLAGS_STATIC) sockexp.cpp
<<

#############################################################################
# Build target

$D\$(TARG).res: mfcdll.rc ..\..\include\atlbuild.h
	rc $(RCFLAGS) $(RCDEFINES) /fo $D\$(TARG).res mfcdll.rc

DLL_OBJS=$(OBJECT) $(OBJDIAG) $(INLINES) $(FILES) $(COLL1) $(COLL2) $(MISC) \
	$(WINDOWS) $(DIALOG) $(CONTROLBARS) $(WINMISC) $(DOCVIEW) $(APPLICATION) $(OLEREQ) \
	$(INTERNET) $(DAO)

!if "$(MONOLITHIC)" == "1"
DLL_OBJS=$(DLL_OBJS) $(OLEDLL) $(SOCKETS) $(DB)
!else
!if "$(DEBUG)" == "0"
!if "$(PLATFORM)" != "IA64"
DLL_OBJS=$(DLL_OBJS) $(OLEDLL) $(SOCKETS)
!endif
!endif
!endif

DLL_OBJS=$(DLL_OBJS) $D\dllinit.obj

DLL_RESOURCES=$D\$(TARG).res

!if "$(PLATFORM)" == "AXP64" || "$(PLATFORM)" == "IA64"
$(PLATFORM)\$(TARG).dll ..\..\lib\$(PLATFORM)\$(TARG).lib: $(DLL_OBJS) $(DEFFILE) $(DLL_RESOURCES)
	link @<<
$(LFLAGS)
$(LIBS)
$(DLL_OBJS)
$(DLL_RESOURCES)
/def:$(DEFFILE)
/out:$(PLATFORM)\$(TARG).DLL
/map:$D\$(TARG).MAP
/implib:..\..\lib\$(PLATFORM)\$(TARG).LIB
/filealign:4096
/delayload:comdlg32.dll
/delayload:shell32.dll
/delayload:advapi32.dll
/delayload:wininet.dll
/delayload:WS2_32.dll
/delayload:ole32.dll
/delayload:oleaut32.dll
/delayload:oledlg.dll
/delayload:urlmon.dll
/delayload:odbc32.dll
/delayload:winspool.drv
!if "$(DEBUG)" != "0"
/delayload:msimg32.dll
!endif
/delayload:oleacc.dll
delayimp.lib
mfcdload.lib
/NOD:olepro32.lib
/NOD:libcmt.lib
<<
!else
$(PLATFORM)\$(TARG).dll ..\..\lib\$(PLATFORM)\$(TARG).lib: $(DLL_OBJS) $(DEFFILE) $(DLL_RESOURCES)
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
/delayload:comdlg32.dll
/delayload:comctl32.dll
/delayload:shell32.dll
/delayload:advapi32.dll
/delayload:wininet.dll
/delayload:wsock32.dll
/delayload:WS2_32.dll
/delayload:ole32.dll
/delayload:oleaut32.dll
/delayload:oledlg.dll
/delayload:urlmon.dll
/delayload:odbc32.dll
/delayload:winspool.drv
/delayload:hhctrl.ocx
/delayload:msimg32.dll
/delayload:oleacc.dll
delayimp.lib
mfcdload.lib
<<
!endif

..\..\lib\$(PLATFORM)\$(TARG_STATIC).lib: $(STATICLINK_OBJS)
	lib /out:$@ $(STATICLINK_OBJS)

#############################################################################
