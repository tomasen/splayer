# This is a part of the Active Template Library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Active Template Library Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Active Template Library product.

# Default PLATFORM depending on host environment
!if "$(PLATFORM)" == ""
!if "$(PROCESSOR_ARCHITECTURE)" == ""
!error PLATFORM must be set to intended target
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
PLATFORM=INTEL
!endif
!endif

!ifndef TARGETNAME
!error TARGETNAME must be defined
!endif
!ifndef TARGETTYPE
!error TARGETTYPE must be set to LIB, DLL, EXE, or TLB
!endif

# Default to DEBUG mode
!ifndef DEBUG
DEBUG=1
!endif

# Default Codeview Info
!ifndef CODEVIEW
CODEVIEW=1
!endif

# Default to _MBCS build
!ifndef MBCS
MBCS=1
!endif

!if "$(AFXDLL)" == "1"
USE_CRTDLL=1
!endif

!if "$(USE_CRTDLL)" == "1"

!if "$(DEBUG)" == "1"
TARGOPTS=$(TARGOPTS) /MDd
CRTLIB=msvcrtd.lib
!else
TARGOPTS=$(TARGOPTS) /MD
CRTLIB=msvcrt.lib
!endif

!else

!if "$(DEBUG)" == "1"
TARGOPTS=$(TARGOPTS) /MTd
CRTLIB=libcmtd.lib
!else
TARGOPTS=$(TARGOPTS) /MT
CRTLIB=libcmt.lib
!endif

!endif

#############################################################################
# normalize cases of parameters, or error check

#############################################################################
# Parse options

DEFS=$(DEFS) /D_ATL_NO_DEFAULT_LIBS

#
# DEBUG OPTIONS
#
!if "$(DEBUG)" != "0"

DEBUGSUF=D
DEBDEFS=/D_DEBUG
DEBOPTS=/Od

!endif

#
# NON-DEBUG OPTIONS
#
!if "$(DEBUG)" == "0"

DEBUGSUF=
DEBDEFS=

!if "$(PLATFORM)" == "INTEL"
DEBOPTS=/O1 /Gy
!endif

!if "$(PLATFORM)" == "IA64"
DEBOPTS=/O1 /Gy
!endif

!endif

#
# PLATFORM options
#
CPP=cl
LIB32=lib
LINK32=link
MIDL=midl
RC=rc

!if "$(PLATFORM)" == "INTEL"
CL_MODEL=/D_X86_
!endif

!if "$(PLATFORM)" == "IA64"
CL_MODEL=/D_IA64_
!endif

!if "$(CPP)" == ""
!error PLATFORM must be one of INTEL, IA64
!endif

!if "$(UNICODE)" == "1"
TARGDEFS=$(TARGDEFS) /D_UNICODE /DUNICODE
!else
!if "$(MBCS)" != "0"
TARGDEFS=$(TARGDEFS) /D_MBCS
!endif
!endif

TARGDEFS=$(TARGDEFS) /DWIN32 /D_WINDOWS
!if "$(TARGETTYPE)" == "DLL"
TARGDEFS=$(TARGDEFS) /D_WINDLL
!endif

!if "$(AFXDLL)" == "1"
TARGDEFS=$(TARGDEFS) /D_AFXDLL
!endif

#
# Object File Directory
#
!ifndef D
!if "$(DEBUG)" == "0"
D=$(PLATFORM)\Release$(_OD_EXT)
!else
D=$(PLATFORM)\Debug$(_OD_EXT)
!endif
!if "$(UNICODE)" == "1"
D=$(D)U
!endif
!endif

#
# CODEVIEW options
#
CVOPTS=/Zi /Fd$(D)\$(TARGETNAME).pdb

!if "$(VC6_DEBUGGING)" != ""
CVOPTS=$(CVOPTS) /Zvc6
!endif


#
# COMPILER OPTIONS
#
CL_OPT=/W4 /Wp64 /WX /Zl $(DEBOPTS) $(CVOPTS) $(TARGOPTS)

!if "$(PREPROC)" == "1"
CL_OPT=$(CL_OPT) /P
!endif

!if "$(USE_EH)" == "1"
CL_OPT=$(CL_OPT) /EHsc
!else
CL_OPT=$(CL_OPT) /EHs-c-
!endif

CL_OPT=/Fo$D\ $(CL_OPT)

# REVIEW
CL_OPT=$(CL_OPT) /wd4601

DEFS=$(DEFS) $(DEBDEFS) $(TARGDEFS)

#############################################################################
# Library Components

#############################################################################
# Standard tools

!if "$(VC6_DEBUGGING)" != ""
LFLAGS=$(LFLAGS) /debug /debugtype:vc6 /pdb:$(D)\$(TARGETNAME).pdb /incremental:no
!else
LFLAGS=$(LFLAGS) /debug /debugtype:cv /pdb:$(D)\$(TARGETNAME).pdb /incremental:no
!endif

!if "$(DEBUG)" == "0"
LFLAGS=$(LFLAGS) /opt:ref /opt:icf,32
!endif

!if "$(TARGETTYPE)" == "DLL"
LFLAGS=$(LFLAGS) /dll
!endif

!if "$(TARGETTYPE)" == "EXE"
LFLAGS=$(LFLAGS) /subsystem:windows
!endif

!if "$(TARGETTYPE)" == "CONSOLE"
LFLAGS=$(LFLAGS) /subsystem:console
!endif

MIDL_FLAGS=$(MIDL_FLAGS) /out $(D)

!if "$(PLATFORM)" == "INTEL"
MIDL_FLAGS=$(MIDL_FLAGS)
!endif

!if "$(PLATFORM)" == "IA64"
MIDL_FLAGS=$(MIDL_FLAGS)
!endif

RCFLAGS=$(RCFLAGS) /l 0x409 $(DEFS)
!ifdef RCINCLUDES
RCFLAGS=$(RCFLAGS) /i $(RCINCLUDES: =/i )
!endif

#############################################################################
# Goals to build

GOALS=create.dir

!if "$(TARGETTYPE)" == "CONSOLE"
GOALS=$(GOALS) $(D)\$(TARGETNAME).exe
!endif

!if "$(TARGETTYPE)" == "EXE"
GOALS=$(GOALS) $(D)\$(TARGETNAME).exe
!endif

!if "$(TARGETTYPE)" == "LIB"
GOALS=$(GOALS) $(D)\$(TARGETNAME).lib
!endif

!if "$(TARGETTYPE)" == "DLL"
!ifndef IMPLIB
IMPLIB=$(TARGETNAME).lib
!endif
GOALS=$(GOALS) $(D)\$(TARGETNAME).dll $(D)\$(IMPLIB)
!endif

!if "$(TARGETTYPE)" == "TLB"
MIDL_TARGETS=$(D)\$(TARGETNAME).tlb $(D)\$(TARGETNAME).H
GOALS=$(GOALS) $(MIDL_TARGETS)
!endif

!ifdef HELPFILE
HLPFILE=$(D)\$(TARGETNAME).hlp
GOALS=$(GOALS) $(HLPFILE)
!endif

goal: $(GOALS)

create.dir:
	@-if not exist $D\*.* mkdir $D

clean:
	-if exist $D\*.obj erase $D\*.obj
	-if exist $D\*.pch erase $D\*.pch
	-if exist $D\*.res erase $D\*.res
	-if exist $D\*.rsc erase $D\*.rsc
	-if exist $D\*.map erase $D\*.map
	-if exist $D\*.pdb erase $D\*.pdb
	-if exist $D\*.tlb erase $D\*.tlb
	-if exist $D\*.h erase $D\*.h
	-if not exist $D\*.* rmdir $D

#############################################################################
# Set CPPFLAGS for use with .cpp.obj and .c.obj rules
# Define rule for use with OBJ directory
# C++ uses a PCH file

!if "$(NO_PCH)" != "1"
PCH_FILE=stdafx
PCH_TARGETS=$(D)\$(PCH_FILE).pch $(D)\$(PCH_FILE).obj

PCH_USE_OPT=/Yu$(PCH_FILE).h /Fp$(D)\$(PCH_FILE).pch
!else
PCH_USE_OPT=
!endif

CPPFLAGS=$(CPPFLAGS) $(CL_MODEL) $(CL_OPT) $(DEFS) $(OPT)

.SUFFIXES: .cpp .c .s

!if "$(NO_PCH)" != "1"
$(PCH_TARGETS):: $(PCH_FILE).cpp $(PCH_FILE).h
	$(CPP) @<<
/Yc$(PCH_FILE).h /Fp$(D)\$(PCH_FILE).pch $(CPPFLAGS) /c $(PCH_FILE).cpp
<<
!endif

.cpp{$D}.obj::
	$(CPP) @<<
$(CPPFLAGS) $(PCH_USE_OPT) /c $<
<<

.cpp.i::
	$(CPP) @<<
$(CPPFLAGS) $(PCH_USE_OPT) /c /P $<
<<

.c{$D}.obj::
	$(CPP) @<<
$(CPPFLAGS)  /c $<
<<

.c.i::
	$(CPP) @<<
$(CPPFLAGS) /c /P $<
<<

!if "$(PLATFORM)" == "IA64"

{$(PLATFORM)}.s{$D}.obj:
	ias -o $@ $<

!endif

!ifdef RCFILE
RESFILE=$(D)\$(RCFILE:.rc=.res)

$(RESFILE): $(RCFILE)
	$(RC) $(RCFLAGS) /fo$(RESFILE) $(RCFILE)
!endif

!ifdef HELPFILE
$(HLPFILE): $(HELPFILE)
	makehelp.bat
!endif

!if "$(TARGETTYPE)" == "TLB"
$(MIDL_TARGETS):: $(TARGETNAME).idl
	$(MIDL) @<<
$(MIDL_FLAGS) /h $(TARGETNAME).H $(TARGETNAME).idl
<<
!endif
