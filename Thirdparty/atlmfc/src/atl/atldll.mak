# This is a part of the Active Template Library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Active Template Library Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Active Template Library product.

!ifndef DEBUG
DEBUG=1
!endif

TARGETNAME=atl$(MFC_VER)
IMPLIB=atl.lib
TARGETTYPE=DLL
DEFFILE=atl.def
RCFILE=atl.rc
RCINCLUDES=$(PLATFORM)\TLB
PDBNAME=$(TARGETNAME)$(ATL_PDB_VERSION_NAME).pdb

# base address for atl$(MFC_VER).dll. Should change if the name of the DLL changes

!if "$(PLATFORM)" == "INTEL"
LFLAGS=/base:0x788a0000
!endif
!if "$(PLATFORM)" == "AMD64"
LFLAGS=/base:0x78ab0000
!endif
!if "$(PLATFORM)" == "IA64"
LFLAGS=/base:0x792a0000
!endif

USE_CRTDLL=1

!include atlcommon.mak

OBJS=$(D)\RegObj.obj $(D)\atl.obj $(D)\stdafx.obj
LIBS=$(LIBS) atldload.lib ole32.lib oleaut32.lib advapi32.lib user32.lib gdi32.lib kernel32.lib shlwapi.lib
MANIFESTOBJ=$(D)\atlmanifest.obj

!if "$(DEBUG)" == "1"
LIBS=$(LIBS) atlsd.lib
!else
LIBS=$(LIBS) atls.lib
!endif

DELAYLOAD=ole32.dll;oleaut32.dll;advapi32.dll;user32.dll;gdi32.dll

$(D)\atlmanifest.obj : atlmanifest.cpp
	$(CPP) $(CPPFLAGS:/Zi=/Z7) /c atlmanifest.cpp

!include atltarg.mak
