!IF 0

Copyright (c) 1990-2003  Microsoft Corporation

Module Name:

    makefile.inc.

Abstract:

    This file specifies build commands for the component in each phase.

!ENDIF

!include $(VCBASE)\makefile.inc
!include $(VCBASE)\vc7libs\makefile.inc

!if defined(NO_AUTOMATIC_MANIFEST) && "$(NO_AUTOMATIC_MANIFEST)" != "0"
_LINK_=/manifest:no
!else
# enable manifest file generation and embedding.
_LINK_=$(_LINK_:/manifest:no=)
_LINK_=$(_LINK_:-manifest:no=)
!endif

BOOT_PHASE SELF_PHASE:

PROD_PHASE:
    set _LINK_=$(_LINK_)
    $(MAKE) /nologo /$(MAKEFLAGS) /f Sproxy.mak UNICODE=0 DEBUG=0 $(_ATLMFC_CLEAN) "SECURE_CRT_DEPRECATE=$(SECURE_CRT_DEPRECATE)"
!ifndef _ATLMFC_CLEAN
    -if not exist $(BINPATH) mkdir $(BINPATH)
    copy $(PLATFORM)\Release\sproxy.exe $(BINPATH)
    copy $(PLATFORM)\Release\sproxy.pdb $(BINPATH)
    -if not exist $(BINPATH)\1033 mkdir $(BINPATH)\1033
	copy $(PLATFORM)\Release\1033\sproxyUI.dll $(BINPATH)\1033

!endif
