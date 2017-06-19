# This is a part of the Active Template Library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Active Template Library Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Active Template Library product.

#############################################################################
# Build the library from the up-to-date objs

!if "$(TARGETTYPE)" == "LIB"

# Build final library
$(D)\$(TARGETNAME).lib: $(OBJS) $(PCH_TARGETS)
	@-if exist $@ erase $@
	@$(LIB32) /out:$@ @<<
$(OBJS)
<<

!endif

!if "$(DELAYLOAD)" != ""
DELAYLOAD=$(DELAYLOAD: =)
DELAYLOAD_FLAGS=/delayload:$(DELAYLOAD:;= /delayload:) delayimp.lib
!else
DELAYLOAD_FLAGS=
!endif

!if "$(TARGETTYPE)" == "DLL"


$(D)\$(TARGETNAME).dll $(D)\$(IMPLIB): $(OBJS) $(DEFFILE) $(PCH_TARGETS) $(RESFILE) $(MANIFESTOBJ)
	link @<<
$(LFLAGS)
$(OBJS)
$(RESFILE)
$(LIBS)
$(CRTLIB)
$(DELAYLOAD_FLAGS)
/def:$(DEFFILE)
/map:$(D)\$(TARGETNAME).map
/out:$(D)\$(TARGETNAME).dll
/implib:$(D)\$(IMPLIB)
<<
!if "$(MANIFESTOBJ)" != ""
	lib $(D)\$(IMPLIB) $(MANIFESTOBJ)
!endif

!endif

!if "$(TARGETTYPE)" == "CONSOLE"
$(D)\$(TARGETNAME).exe: $(OBJS) $(PCH_TARGETS)
	link @<<
$(LFLAGS)
$(OBJS)
$(LIBS)
$(CRTLIB)
$(DELAYLOAD_FLAGS)
/map:$(D)\$(TARGETNAME).map
/out:$(D)\$(TARGETNAME).exe
<<

!endif
