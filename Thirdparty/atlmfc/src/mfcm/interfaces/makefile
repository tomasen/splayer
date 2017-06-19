
!ifndef LIBNAME
#!error LIBNAME is not defined. LIBNAME=MFC90 builds the prebuilt DLL.
LIBNAME=MFC90
!endif

# mfcmifc80 is unchanged in MFC 9.0
PROJNAME=$(LIBNAME:MFC90=MFCMIFC80)

TARGET=$(PROJNAME).dll
TARGET_PDB=$(PROJNAME).pdb

!ifndef DEBUG
DEBUG=0
!endif

# Default PLATFORM depending on host environment
!ifndef PLATFORM
!ifndef PROCESSOR_ARCHITECTURE
PROCESSOR_ARCHITECTURE=x86
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
PLATFORM=INTEL
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "MIPS"
PLATFORM=MIPS
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "ALPHA"
PLATFORM=ALPHA
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "PPC"
PLATFORM=PPC
!endif
!endif

!if "$(DEBUG)" == "1"
OUTDIR=$(PLATFORM)\$$udlld.w
CL_DEFS=$(CL_DEFS) /D DEBUG
CL_OPT=$(CL_OPT) /MDd /Od /Zi
LINK_OPT=/DEBUG /ASSEMBLYDEBUG
RCDEFINES=$(RCDEFINES) /D_DEBUG
!else
OUTDIR=$(PLATFORM)\$$udll.w
CL_DEFS=$(CL_DEFS) /D NDEBUG
CL_OPT=$(CL_OPT) /MD /O2 /Zi
RCDEFINES=$(RCDEFINES)
!endif

MFC_BINDIR=..\..\mfc\$(PLATFORM)

CL_DEFS=$(CL_DEFS) /D WIN32 /D _WINDOWS /D _USRDLL \
	/D _WINDLL /D _UNICODE /D UNICODE /D MFCM80_EXPORTS
CL_OPT=/nologo /FD /EHac /GS /W4 /c /clr:safe /TP \
	/AI $(OUTDIR) /Fo$(OUTDIR)/ /Fd$(OUTDIR)/vc80.pdb

CL_PCH_USER_OPT=/Yu"stdafx.h" /Fp$(OUTDIR)/$(PROJNAME).pch
CL_PCH_CREATOR_OPT=/Yc"stdafx.h" /Fp$(OUTDIR)/$(PROJNAME).pch

LINK_OPT=/INCREMENTAL:NO /nologo /DLL /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /FIXED:No \
	/OUT:$(OUTDIR)\$(PROJNAME).dll /PDB:$(OUTDIR)\$(TARGET_PDB)  \
	/IMPLIB:$(OUTDIR)\$(PROJNAME).lib \
	/IDLOUT:$(PROJNAME).idl \
	/DEBUG /DEBUGTYPE:cv,fixup
	
LINK_LIBS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib

RCFLAGS=/r

DLL_RESOURCES=$(OUTDIR)\mfcm80.res

OBJS=\
	$(OUTDIR)\AssemblyInfo.obj \
	$(OUTDIR)\stdafx.obj \
	$(OUTDIR)\mfcm80.obj

!if "$(CLEAN)" == ""
all: createdir $(OUTDIR)\$(TARGET) deploy_at_mfc_binary_dir
!else
all: _clean
!endif

createdir:
	if not exist $(OUTDIR) mkdir $(OUTDIR)

$(OUTDIR)\stdafx.obj: stdafx.cpp stdafx.h
	cl.exe stdafx.cpp @<<
	$(CL_DEFS) $(CL_OPT) $(CL_PCH_CREATOR_OPT)
<<

$(OUTDIR)\mfcm80.obj: $(OUTDIR)\stdafx.obj mfcm80.cpp
	cl.exe  mfcm80.cpp @<<
	$(CL_DEFS) $(CL_OPT) $(CL_PCH_USER_OPT)
<<

$(OUTDIR)\AssemblyInfo.obj: $(OUTDIR)\stdafx.obj AssemblyInfo.cpp
	cl.exe AssemblyInfo.cpp @<<
	$(CL_DEFS) $(CL_OPT) $(CL_PCH_USER_OPT)
!if "$(LIBNAME)" == "MFC90"
	/DBUILD_PREBUILT
!endif
<<


$(DLL_RESOURCES) : mfcm80.rc ..\..\..\include\atlbuild.h	
	rc $(RCFLAGS) $(RCDEFINES) /fo $(DLL_RESOURCES) mfcm80.rc

$(OUTDIR)\$(TARGET) : $(OBJS) $(DLL_RESOURCES)
	link.exe @<<
	$(LINK_OPT) $(LINK_LIBS) $(OBJS) $(DLL_RESOURCES)
<<


# The following rule deploys MFCM DLLs at the MFC DLL directory.
# It is off by default, and could be turned on by setting the
#  environment variable MFCM_DEPLOY to 1.

deploy_at_mfc_binary_dir:
!if "$(MFCM_DEPLOY)" == "1"
	copy $(OUTDIR)\$(TARGET) $(MFC_BINDIR)
	copy $(OUTDIR)\$(TARGET_PDB) $(MFC_BINDIR)
!else

!endif

_clean:
	-del /q $(OUTDIR)\*.*
	-rd $(OUTDIR)
