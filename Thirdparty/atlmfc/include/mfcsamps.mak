# This is a part of the Microsoft Foundation Classes C++ library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Microsoft Foundation Classes Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Microsoft Foundation Classes product.

# Common include for building MFC Sample programs
#
#  typical usage
#       PROJ=foo
#       OBJS=foo.obj bar.obj ...
#       !INCLUDE ..\..\SAMPLE_.MAK
#
#  ROOT specifies the location of the msdev\samples\mfc directory,
#  relative to the project directory. Because the MFC tutorial samples
#  have an intermediate STEP<n> subdirectory, they use
#       ROOT=..\..\..
#  instead of the default
#       ROOT=..\..
#
# NOTE: do not include 'stdafx.obj' in the OBJS list - the correctly
#    built version will be included for you
#
# Options to NMAKE:
#     "PLATFORM=?"
#       This option chooses the appropriate tools and sources for the
#       different platforms support by Windows/NT.  Currently INTEL,
#       MIPS, ALPHA, PPC are supported. The default is chosen based on
#       the host environment.
#     "DEBUG=0"     use release (default debug)
#     "CODEVIEW=1"  include codeview info (even for release builds)
#         "AFXDLL=1"    to use shared DLL version of MFC
#         "USRDLL=1"    to build a DLL that uses static MFC
#     "UNICODE=1"   to build UNICODE enabled applications
#                   (not all samples support UNICODE)
#     "NO_PCH=1"    do not use precompiled headers (defaults to use pch)
#     "COFF=1"      include COFF symbols

!ifndef PROJ
!ERROR You forgot to define the 'PROJ' symbol!!
!endif


ROOT=.
!ifndef ROOT
!endif

!ifndef OBJS
!ERROR You forgot to define the 'OBJS' symbol!!
!endif

!ifndef DEBUG
DEBUG=1
!endif

!ifndef AFXDLL
AFXDLL=0
!endif

!ifndef UNICODE
UNICODE=0
!endif

!ifndef USRDLL
USRDLL=0
!endif

!if "$(USRDLL)" != "0"
AFXDLL=0
!endif

!ifndef PLATFORM
!ifndef PROCESSOR_ARCHITECTURE
PROCESSOR_ARCHITECTURE=x86
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
PLATFORM=INTEL
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "ALPHA"
PLATFORM=ALPHA
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "MIPS"
PLATFORM=MIPS
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "PPC"
PLATFORM=PPC
!endif
!endif

!ifndef USES_OLE
USES_OLE=0
!endif

!ifndef USES_DB
USES_DB=0
!endif

!ifndef CONSOLE
CONSOLE=0
!endif

!ifndef MBCS
MBCS=1
!endif

!ifndef NO_PCH
NO_PCH=0
!endif

BASE=W

!if "$(UNICODE)" == "0"
!if "$(AFXDLL)" == "0"
!if "$(USRDLL)" != "1"
STDAFX=stdafx
!else
STDAFX=stdusr
!endif
!else
STDAFX=stddll
!endif
!endif

!if "$(UNICODE)" == "1"
!if "$(AFXDLL)" == "0"
!if "$(USRDLL)" != "1"
STDAFX=uniafx
!else
STDAFX=uniusr
!endif
!else
STDAFX=unidll
!endif
!endif

!if "$(DEBUG)" == "1"
STDAFX=$(STDAFX)d
!if "$(COFF)" != "1"
!ifndef CODEVIEW
CODEVIEW=1
!endif
!endif
!endif

!if "$(CODEVIEW)" == "1"
STDAFX=$(STDAFX)v
!endif

!if "$(DEBUG)" == "1"
DEBUG_SUFFIX=d
!endif

!if "$(DEBUG)" != "0"
DEBUGFLAGS=/Od
MFCDEFS=$(MFCDEFS) /D_DEBUG

!endif

!if "$(DEBUG)" == "0"
!if "$(PLATFORM)" == "INTEL"
DEBUGFLAGS=/O1 /Gy
!endif
!if "$(PLATFORM)" == "MIPS"
DEBUGFLAGS=/O1 /Gy
!endif
!if "$(PLATFORM)" == "ALPHA"
DEBUGFLAGS=/O1 /Gy
!endif
!if "$(PLATFORM)" == "PPC"
DEBUGFLAGS=/O1 /Gy
!endif
!endif # DEBUG == 0

!if "$(CODEVIEW)" == "1" || "$(COFF)" == "1"
DEBUGFLAGS=$(DEBUGFLAGS) /Z7
!endif

!if "$(UNICODE)" == "1"
DLL_SUFFIX=u
!endif

!if "$(AFXDLL)" == "1"
MFCFLAGS=$(MFCFLAGS) /MD$(DEBUG_SUFFIX)
MFCDEFS=$(MFCDEFS) /D_AFXDLL
!endif # AFXDLL == 1

!if "$(USRDLL)" == "1"
MFCDEFS=$(MFCDEFS) /D_USRDLL /D_WINDLL
!endif

!if "$(AFXDLL)" == "0"
!if "$(MD)" == "1"
MFCFLAGS=$(MFCFLAGS) /MD$(DEBUG_SUFFIX)
!elseif "$(MT)" == "0"
MFCFLAGS=$(MFCFLAGS) /ML$(DEBUG_SUFFIX)
!else
MFCFLAGS=$(MFCFLAGS) /MT$(DEBUG_SUFFIX)
!endif
!endif

!if "$(UNICODE)" == "1"
MFCDEFS=$(MFCDEFS) /D_UNICODE
!else
!if "$(MBCS)" == "1"
MFCDEFS=$(MFCDEFS) /D_MBCS
!endif
!endif

!if "$(PLATFORM)" == "INTEL"
MFCDEFS=$(MFCDEFS) /D_X86_
CPP=cl
CFLAGS=/EHsc /c /W3 $(DEBUGFLAGS) $(MFCFLAGS) $(MFCDEFS)
!endif

!if "$(PLATFORM)" == "MIPS"
MFCDEFS=$(MFCDEFS) /D_MIPS_
CPP=cl
CFLAGS=/EHsc /c /W3 $(DEBUGFLAGS) $(MFCFLAGS) $(MFCDEFS)
!endif

!if "$(PLATFORM)" == "ALPHA"
MFCDEFS=$(MFCDEFS) /D_ALPHA_
CPP=cl
CFLAGS=/EHsc /c /W3 $(DEBUGFLAGS) $(MFCFLAGS) $(MFCDEFS)
!endif

!if "$(PLATFORM)" == "PPC"
MFCDEFS=$(MFCDEFS) /D_PPC_
!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
CPP=mcl
!else
CPP=cl
!endif
CFLAGS=/EHsc /c /W3 $(DEBUGFLAGS) $(MFCFLAGS) $(MFCDEFS)
!endif

CPPMAIN_FLAGS=$(CFLAGS)

!if "$(NO_PCH)" == "1"
CPPFLAGS=$(CPPMAIN_FLAGS)
!else
PCHDIR=.
CPPFLAGS=$(CPPMAIN_FLAGS) /Yustdafx.h /Fp$(PCHDIR)\$(STDAFX).pch
!endif

!if "$(COFF)" == "1"
NO_PDB=1
!if "$(CODEVIEW)" != "1"
LINKDEBUG=/incremental:no /debug /debugtype:coff
!else
LINKDEBUG=/incremental:no /debug /debugtype:both
!endif
!endif

!if "$(COFF)" != "1"
!if "$(CODEVIEW)" == "1"
LINKDEBUG=/incremental:no /debug /debugtype:cv
!else
LINKDEBUG=/incremental:no /debug:none
!endif
!endif

!if "$(NO_PDB)" == "1"
LINKDEBUG=$(LINKDEBUG) /pdb:none
!endif

!if "$(PLATFORM)" == "INTEL"
LINKCMD=link $(LINKDEBUG)
!endif

!if "$(PLATFORM)" == "MIPS"
LINKCMD=link $(LINKDEBUG)
!endif

!if "$(PLATFORM)" == "ALPHA"
LINKCMD=link $(LINKDEBUG)
!endif

!if "$(PLATFORM)" == "PPC"
LINKCMD=link $(LINKDEBUG)
!endif

# link flags - must be specified after $(LINKCMD)
#
# conflags : creating a character based console application
# guiflags : creating a GUI based "Windows" application

CONFLAGS=/subsystem:console
GUIFLAGS=/subsystem:windows

!if "$(UNICODE)" == "1"
CONFLAGS=$(CONFLAGS) /entry:wmainCRTStartup
GUIFLAGS=$(GUIFLAGS) /entry:wWinMainCRTStartup
!endif

PROJRESFILE=$(PROJ).res
RESFILE=$(PROJRESFILE)

.SUFFIXES: .rcm .rc
.SUFFIXES:: .c .cpp

.cpp.obj::
	$(CPP) @<<
$(CPPFLAGS) $<
<<

.c.obj::
	$(CPP) @<<
$(CFLAGS) $(CVARS) $<
<<

.rc.res:
	rc /r $(MFCDEFS) $<

#############################################################################

!if "$(NO_PCH)" == "0"
LINK_OBJS=$(OBJS) $(PCHDIR)\$(STDAFX).obj
!else
LINK_OBJS=$(OBJS)
!endif

#
# Build CONSOLE Win32 application
#
!if "$(CONSOLE)" == "1"

$(PROJ).exe: $(LINK_OBJS)
	$(LINKCMD) @<<
$(CONFLAGS) /out:$(PROJ).exe /map:$(PROJ).map
$(LINK_OBJS) $(EXTRA_LIBS)
<<

!endif  # CONSOLE=1

#
# Build Win32 application
#
!if "$(CONSOLE)" == "0"

!if "$(USRDLL)" == "1"
$(PROJ).dll: $(LINK_OBJS) $(PROJRESFILE)
	$(LINKCMD) @<<
$(GUIFLAGS) /out:$(PROJ).dll /map:$(PROJ).map
/dll /def:$(PROJ).def
$(LINK_OBJS) $(RESFILE) $(EXTRA_LIBS)
<<

$(PROJ).res:  resource.h
$(PROJ).rsc:  resource.h
!endif

!if "$(SIMPLE_APP)" != "1"
$(PROJ).exe: $(LINK_OBJS) $(PROJRESFILE)
	$(LINKCMD) @<<
$(GUIFLAGS) /out:$(PROJ).exe /map:$(PROJ).map
$(LINK_OBJS) $(RESFILE) $(EXTRA_LIBS)
<<

$(PROJ).res:  resource.h
$(PROJ).rsc:  resource.h
!endif

!if "$(SIMPLE_APP)" == "1"

$(PROJ).exe: $(LINK_OBJS)
	$(LINKCMD) @<<
$(GUIFLAGS) /out:$(PROJ).exe /map:$(PROJ).map
$(LINK_OBJS) $(EXTRA_LIBS)
<<

!endif

!if "$(NO_PCH)" == "0"
$(PCHDIR)\$(STDAFX).obj $(PCHDIR)\$(STDAFX).pch: stdafx.h stdafx.cpp
	echo "BUILDING SHARED PCH and PCT files"
	$(CPP) @<<
$(CPPMAIN_FLAGS) /Ycstdafx.h /Fp$(PCHDIR)\$(STDAFX).pch /Fo$(PCHDIR)\$(STDAFX).obj /c $(ROOT)\stdafx.cpp
<<

$(OBJS): $(PCHDIR)\$(STDAFX).pch
!endif

!endif  # CONSOLE=0

clean::
	if exist $(PROJ).exe erase $(PROJ).exe
	if exist *.aps erase *.aps
	if exist *.pch erase *.pch
	if exist *.map erase *.map
	if exist *.obj erase *.obj
	if exist *.exp erase *.exp
	if exist *.pdb erase *.pdb
	if exist *.map erase *.map
	if exist *.lib erase *.lib
	if exist *.res erase *.res
	if exist *.rsc erase *.rsc
	if exist *.pef erase *.pef

#############################################################################
