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

!if "$(DEBUG)" == "1"
TARGETNAME=atlsd
!else
TARGETNAME=atls
!endif
TARGETTYPE=LIB

!include ..\atlcommon.mak

OBJS= \
	$(D)\Allocate.obj \
	$(D)\atlbase.obj \
	$(D)\atlcommodule.obj \
	$(D)\ATLComTime.obj \
	$(D)\AtlDebugAPI.obj \
!if "$(PLATFORM)" == "INTEL"
	$(D)\atldebuginterfacesmodule.obj \
!endif
!if "$(PLATFORM)" == "INTEL" || "$(DEBUG)" == "1"
	$(D)\atlfuncs.obj \
!endif
	$(D)\atlimage.obj \
	$(D)\atlimage2.obj \
	$(D)\atlmem.obj \
	$(D)\atlstr.obj \
	$(D)\atltrace.obj \
!if "$(DEBUG)" == "1"
	$(D)\atltime.obj \
	$(D)\atltypes.obj \
!endif
	$(D)\AtlTraceModuleManager.obj \
	$(D)\atlwinmodule.obj \
	$(D)\Externs.obj \
	$(D)\LoadNSave.obj

!if "$(PLATFORM)" == "IA64" || "$(PLATFORM)" == "AMD64"
OBJS= \
	$(OBJS) \
	$(D)\QIThunk.obj \
	$(D)\stdcallthunk.obj
!endif

!if "$(PLATFORM)" == "INTEL" || "$(PLATFORM)" == "AMD64"
OBJS= \
	$(OBJS) \
	$(D)\atlthunk.obj	
!endif

!include ..\atltarg.mak
