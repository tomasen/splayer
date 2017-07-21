# Makefile : Builds a Microsoft Foundation Class library variant.
#
# This is a part of the Microsoft Foundation Classes C++ library.
# Copyright (C) Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Microsoft Foundation Classes Reference and related
# electronic documentation provided with the library.
# See these sources for detailed information regarding the
# Microsoft Foundation Classes product.
#
# Usage: NMAKE CLEAN        (removes all intermediary files)
#    or: NMAKE options      (builds one library variant (see below))
# Note that an NMAKE CLEAN should be performed before building a new variant.
#
# 'Options' are one of each of:
#   "DLL"              (defaults to 0)
#           If this item is 0, then a normal library is generated.
#                       DLL=1 is obsolete and not supported by this release.
#           If this item is 2, objects suitable for the shared DLL version
#           of MFC are created.  Note: DLL=2 is to be used only from
#           MFCDLL.MAK, MFCOLE.MAK, or MFCDB.MAK
#
#   "DEBUG"             (defaults to 1)
#           If this item is 1, debugging support is compiled into
#           the library.  If this item is 0, then debugging support
#           is disabled.  Debug support does not include CodeView information.
#
#   "CODEVIEW"          (defaults to 1 for DEBUG=1, 0 for DEBUG=0)
#           If this item is 1 CodeView information is compiled into
#           the library.  You must use the /DEBUG and /DEBUGTYPE:cv link
#           options when linking your executable. A value of 0 indicates that
#           no CodeView information is to be generated.
#
#   "OBJ=.\obj"         (defaults to '$$(MODEL)$(BASE)$(DEBUG)')
#           This optional specification specifies where temporary OBJ files
#           are stored during the build process.  The directory is created or
#           removed as necessary.
#
#   "OPT="              (no default value)
#           This allows additional compiler options to be added to the build.
#           If more than one switch is desired, put double-quotes around the
#           whole OPT= argument, e.g., "OPT=/J /W3".
#
#   "NO_PCH=1"
#           Set this item to override the default use of precompiled headers.
#
#   "NO_PDB=1"
#           Set this item to override the default use of PDB files.
#
#   "BROWSE=1"          (defaults to 0)
#           Set this option to build the browse database for the MFC
#           library.  By setting BROWSE=1, both the .SBRs and the .BSC
#           files will be built along with the .OBJ and .LIB files that
#           are part of the normal build process.
#
#   "BROWSEONLY=1"      (defaults to 0)
#           Set this option to build the browse files without re-building
#           the MFC library itself.  Note: This option is used internally
#           when BROWSE=1 is selected.
#
#   "HOSTPLATFORM=INTEL"(defaults to host)
#           This option chooses the platform on which the library is being
#           built.  Currently INTEL and ALPHA are supported.
#
#   "PLATFORM=INTEL"    (defaults depends on host)
#           This option chooses the platform for which the library is being 
#           built.  Currently INTEL and IA64 are supported for 
#           HOSTPLATFORM=INTEL, and ALPHA and AXP64 are supported for
#           HOSTPLATFORM=ALPHA.
#
#   "INCREMENTAL=1"     (defaults to 0)
#           This option enables incremental/minimal compilation and
#           incremental linking.
#
#   "MANAGED=1"         (defaults to 0)
#           This option enables use of managed extensions (aka /clr).
#
# Advanced Options:
#
#   "MBCS=0"            (defaults to 1)
#           To build an SBCS library instead of the default (MBCS)
#           you can use MBCS=0.  This creates a slightly smaller
#           library, but the code will not work in far-east markets.
#           This option has no effect when UNICODE=1.
#
#   "MT=0"              (defaults to 1)
#           To build a non-multithreaded library instead of the default
#           (which enables multitheading and uses the multithread
#           C-runtimes) you can use MT=0.
#
#############################################################################
!if "$(NTMAKEENV)" != ""
!INCLUDE $(NTMAKEENV)\devdiv.def
!INCLUDE makefile.inc
!else

# Define defaults if not defined

!if "$(MFC_VER)" == ""
MFC_VER=90
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

# Default to DEBUG mode
!ifndef DEBUG
DEBUG=1
!endif

# Default to NOT DLL
!ifndef DLL
DLL=0
!endif

# Default Codeview Info
!ifndef CODEVIEW
!if "$(DEBUG)" == "1"
CODEVIEW=1
!else
CODEVIEW=0
!endif
!endif

# BROWSEONLY is default 0 and implies BROWSE=1 if BROWSEONLY=1
!ifndef BROWSEONLY
BROWSEONLY=0
!endif

!if "$(BROWSEONLY)" != "0"
!undef BROWSE
BROWSE=1
!endif

# Default to no BROWSE info
!ifndef BROWSE
BROWSE=0
!endif

# Default to no INCREMENTAL build
!ifndef DEVBUILD
DEVBUILD=0
!endif
!if "$(DEBUG)" != "0"
!ifndef INCREMENTAL
INCREMENTAL=$(DEVBUILD)
!endif
!endif
!ifndef INCREMENTAL
INCREMENTAL=0
!endif

# Default to unMANAGED build
!ifndef MANAGED
MANAGED=0
!endif

# Default to _MBCS build
!ifndef MBCS
MBCS=1
!endif

# Default to multithreading support
!ifndef MT
MT=1
!endif

#############################################################################
# normalize cases of parameters, or error check

!if "$(CPU)" == "MIPS"
!if "$(PLATFORM)" != "MIPS"
!error Must set PLATFORM=MIPS for MIPS builds
!endif
!endif

!if "$(CPU)" == "ALPHA"
!if "$(PLATFORM)" != "ALPHA"
!if "$(PLATFORM)" != "AXP64"
!error Must set PLATFORM=ALPHA or PLATFORM=AXP64 for ALPHA builds
!endif
!endif
!endif

BASE=W

#############################################################################
# Parse options

#
# DEBUG OPTIONS
#
!if "$(DEBUG)" != "0"

DEBUGSUF=D
DEBDEFS=/D_DEBUG
DEBOPTS=/Od /Gy
CRTDLL_SWITCH=/MDd
!endif

#
# NON-DEBUG OPTIONS
#
!if "$(DEBUG)" == "0"

DEBUGSUF=
DEBDEFS=
CRTDLL_SWITCH=/MD


!if "$(PLATFORM)" == "INTEL"
DEBOPTS=/O1 /GyF
!endif
!if "$(PLATFORM)" == "AMD64"
DEBOPTS=/O1 /GyF
!endif
!if "$(PLATFORM)" == "MIPS"
DEBOPTS=/O1 /GyF
!endif
!if "$(PLATFORM)" == "ALPHA"
DEBOPTS=/O1 /GyF
!endif
!if "$(PLATFORM)" == "AXP64"
DEBOPTS=/O1 /GyF
!endif
!if "$(PLATFORM)" == "PPC"
DEBOPTS=/O1 /GyF
!endif
!if "$(PLATFORM)" == "IA64"
DEBOPTS=/O1 /GyF
!endif

!endif

#
# PLATFORM options
#

!if "$(PLATFORM)" == "INTEL"
CL_MODEL=/D_X86_
!endif

!if "$(PLATFORM)" == "AMD64"
CL_MODEL=/D_AMD64_
!endif

!if "$(PLATFORM)" == "MIPS"
CL_MODEL=/D_MIPS_
!endif

!if "$(PLATFORM)" == "ALPHA"
CL_MODEL=/D_ALPHA_
!endif

!if "$(PLATFORM)" == "AXP64"
CL_MODEL=/D_ALPHA64_
!endif

!if "$(PLATFORM)" == "PPC"
CL_MODEL=/D_PPC_
!endif

!if "$(PLATFORM)" == "IA64"
CL_MODEL=/D_IA64_
!endif

!if "$(CL_MODEL)" == ""
!error PLATFORM must be one of INTEL, MIPS, ALPHA, AXP64, IA64, AMD64, or PPC.
!endif

# TYPE = Library Type Designator
#       c = normal C library
#       d = DLL library
TYPE=c
DEXT=

#
# Object File Directory
#
!if "$(OBJ)" == ""
D=$$$(MODEL)$(BASE)$(DEBUGSUF)$(DEXT)    # subdirectory specific to variant
!else
D=$(OBJ)                                 # User specified directory
!endif

#
# _AFXDLL DLL Variant
#

!if "$(DLL)" == "2"
# _AFXDLL library
TYPE=e
!if "$(OBJ)" == ""
D=DLL$(DEBUGSUF).$(BASE)
!if "$(UNICODE)" == "1"
D=$(MODEL)$D
!endif
D=$$$D
!endif
TARGOPTS=$(TARGOPTS) $(CRTDLL_SWITCH) /D_DLL /GF
!if "$(MT)" != "0"
TARGOPTS=$(TARGOPTS) /D_MT
!endif
TARGDEFS=$(TARGDEFS) /D_WINDLL /D_AFXDLL
!else
# not _AFXDLL library
!if "$(MD)" == "1"
TARGOPTS=$(TARGOPTS) /MD
!else
!if "$(MT)" != "0"
TARGOPTS=$(TARGOPTS) /MT
!endif
!endif
!endif

D=$(PLATFORM)$(_OD_EXT)\$(D)

!if "$(UNICODE)" == "1"
MODEL=U
TARGDEFS=$(TARGDEFS) /D_UNICODE
!else
MODEL=N
!if "$(MBCS)" != "0"
TARGDEFS=$(TARGDEFS) /D_MBCS
!endif
!endif

!if "$(DLL)" == "2" && "$(BROWSEONLY)" != "1"
!if "$(TARG)" == ""
!error DLL=2 is used only from MFCDLL.MAK, MFCOLE.MAK, or MFCDB.MAK
!endif
GOAL=$(TARG)
!else
GOAL=$(MODEL)afx$(TYPE)$(BASE)$(DEBUGSUF)
!endif

!if "$(DLL)" == "2"
#  && "$(_VCBUILD)" == "1"
!ifndef DISABLE_MP_BUILD
!ifdef NUMBER_OF_PROCESSORS
!if $(NUMBER_OF_PROCESSORS) > 1
MP_BUILD=1
!endif
!endif
!endif
!endif

#
# CODEVIEW options
#
!if !defined(TARGETPDBNAME)
TARGETPDBNAME=$(GOAL).pdb
!endif
!if "$(CODEVIEW)" == "1"
!if "$(NO_PDB)" == "1"
CVOPTS=/Z7
!if "$(PROFLIB)" != ""
!error Can't build for profiling without PDB files.
!endif
!else
!if "$(MP_BUILD)" == "1"
CVOPTS=/Z7
!else
CVOPTS=/Zi
!endif
!if "$(PROFLIB)" != ""
CVOPTS=$(CVOPTS) /Gh
!endif
!if "$(DLL)" == "2"
PDBOPTS=/Fd$(PLATFORM)\$(TARGETPDBNAME)
!else
PDBOPTS=/Fd..\..\lib\$(PLATFORM)\$(TARGETPDBNAME)
!endif
!endif
!endif

#
# INCREMENTAL options
#
!if "$(MANAGED)" == "0"
!if "$(INCREMENTAL)" == "1"
INCROPTS=/Gm
!else
INCROPTS=/Gm-
!endif
!endif
#
# INCREMENTAL options
#
!if "$(MANAGED)" == "1"
TARGOPTS=$(TARGOPTS) /clr -d1clrNoPoundDefineWhitespaceKeyword
EH_FLAGS=/EHa
!else
EH_FLAGS=/EHsc
!endif

#
# COMPILER OPTIONS
#
!if "$(PLATFORM)" == "INTEL"
CL_OPT=/W4 /WX /Zl $(EH_FLAGS) /GR /GS $(INCROPTS) $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "AMD64"
CL_OPT=/W4 /WX /Zl $(EH_FLAGS) /GR $(INCROPTS) $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "MIPS"
CL_OPT=/W4 /WX /Zl $(EH_FLAGS) /GR $(INCROPTS) $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "ALPHA"
CL_OPT=/W4 /WX /Zl $(EH_FLAGS) /GR $(INCROPTS) $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "AXP64"
CL_OPT=/Ap64 /W4 /WX /Zl $(EH_FLAGS) /GR $(INCROPTS) $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "PPC"
CL_OPT=/W4 /WX /Zl $(EH_FLAGS) /GR $(INCROPTS) $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "IA64"
CL_OPT=/W4 /WX /Zl $(EH_FLAGS) /GR /GS $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(DEVBUILD)" != "0"
CL_OPT=$(CL_OPT) /D_AFX_DEVBUILD
!endif

!if "$(MP_BUILD)" == "1"
MP_OPT=/MP$(NUMBER_OF_PROCESSORS)
# CL_OPT=$(CL_OPT) /MP$(NUMBER_OF_PROCESSORS)
!endif

!if "$(BROWSE)" != "0"
CL_OPT=/FR$D\ $(CL_OPT)
!endif

!if "$(BROWSEONLY)" != "0"
CL_OPT=/Zs $(CL_OPT)
!else
CL_OPT=$(CL_OPT)
CL_OPT_OUTDIR=/Fo$D\ 
!endif

CL_OPT=$(CL_OPT) /Zc:wchar_t

# _SECURE_CRT deprecation mechanism
CL_OPT=$(SECURE_CRT_DEPRECATE) $(CL_OPT)

DEFS=$(DEFS) $(DEBDEFS) $(TARGDEFS) /D_AFX_DISABLE_DEPRECATED /D_MFC_BLD /D_MFC_USER_BUILD
DEFS=$(DEFS) /D_AFX_NOFORCE_MANIFEST /D_ATL_NOFORCE_MANIFEST /D_CRT_NOFORCE_MANIFEST

#Standard pointer to member syntax
DEFS=$(DEFS) /D_ATL_ENABLE_PTM_WARNING

#############################################################################
# Library Components

OBJECT=$D\objcore.obj $D\except.obj \
	$D\validadd.obj $D\dumpcont.obj $D\dumpflt.obj \
	$D\arccore.obj $D\arcobj.obj $D\arcex.obj \
		$D\arcstrm.obj

# non-shared diagnostics
OBJDIAG=$D\dumpinit.obj $D\dumpout.obj \
	$D\afxasert.obj $D\afxmem.obj $D\afxabort.obj

FILES=$D\filecore.obj $D\filetxt.obj $D\filemem.obj $D\fileshrd.obj \
	$D\filex.obj $D\filest.obj

COLL1=$D\array_b.obj $D\array_d.obj $D\array_p.obj $D\array_o.obj \
	$D\array_s.obj $D\array_u.obj $D\array_w.obj \
	$D\list_o.obj $D\list_p.obj $D\list_s.obj

COLL2=$D\map_pp.obj $D\map_pw.obj $D\map_so.obj \
	$D\map_sp.obj $D\map_ss.obj $D\map_wo.obj $D\map_wp.obj $D\plex.obj

MISC=\
	$D\strcore.obj $D\timecore.obj \
!if "$(UNICODE)" != "1" && "$(MBCS)" == "1"
	$D\afxdbcs.obj \
!endif
	$D\afxstate.obj $D\afxtls.obj $D\fixalloc.obj \
	$D\mtcore.obj $D\mtex.obj

WINDOWS=\
	$D\wincore.obj $D\winfrm.obj $D\winfrm2.obj $D\winfrmx.obj \
	$D\winmdi.obj $D\tooltip.obj $D\winmini.obj $D\winhand.obj \
	$D\winmain.obj $D\barcore.obj $D\bartool.obj $D\bardlg.obj \
	$D\barstat.obj $D\bardock.obj $D\dockcont.obj $D\dockstat.obj \
	$D\dcprev.obj $D\dcmeta.obj $D\trckrect.obj $D\barcool.obj

DIALOG=\
	$D\winctrl1.obj $D\winctrl2.obj $D\winctrl3.obj $D\winctrl4.obj \
	$D\winbtn.obj $D\dlgcore.obj $D\dlgdata.obj $D\dlgfloat.obj \
	$D\dlgprop.obj $D\dlgcomm.obj $D\dlgfile.obj $D\dlgprnt.obj \
	$D\dlgprntx.obj $D\dlgclr.obj $D\dlgfnt.obj $D\dlgfr.obj $D\ccdata.obj \
	$D\dlgtempl.obj $D\winctrl6.obj $D\winctrl7.obj $D\dlgdhtml.obj

CONTROLBARS=\
	$D\afxautohidebutton.obj $D\afxautohidedocksite.obj $D\afxautohidebar.obj $D\afxpanecontainer.obj \
	$D\afxpanecontainermanager.obj $D\afxbasepane.obj $D\afxbaseribbonelement.obj $D\afxbasetabbedpane.obj \
	$D\afxbasetabctrl.obj $D\afxbasetoolbar.obj $D\afxtoolbarbuttoncustomizedialog.obj $D\afxtoolbarbuttonslistbutton.obj \
	$D\afxtoolbarscommandslistbox.obj $D\afxcaptionbar.obj $D\afxcaptionbutton.obj $D\afxcaptionmenubutton.obj \
	$D\afxcmdusagecount.obj $D\afxcolorbar.obj $D\afxcolorbutton.obj $D\afxcolormenubutton.obj \
	$D\afxstandardcolorspropertypage.obj $D\afxcustomcolorspropertypage.obj $D\afxcolorpickerctrl.obj $D\afxcolorpopupmenu.obj \
	$D\afxcommandmanager.obj $D\afxcontextmenumanager.obj $D\afxcontrolrenderer.obj \
	$D\afxcustomizebutton.obj $D\afxcustomizemenubutton.obj $D\afxdialogex.obj $D\afxdialogimpl.obj \
	$D\afxdocksite.obj $D\afxdockingpanesrow.obj $D\afxdockablepane.obj $D\afxdockablepaneadapter.obj \
	$D\afxpanedialog.obj $D\afxdockingmanager.obj $D\afxdragframeimpl.obj $D\afxdrawmanager.obj \
	$D\afxdropdownlistbox.obj $D\afxdropdowntoolbar.obj $D\afxvslistbox.obj $D\afxfontcombobox.obj \
	$D\afxframeimpl.obj $D\afxframewndex.obj $D\afxfullscreenimpl.obj $D\afxglobals.obj \
	$D\afxglobalutils.obj $D\afximageeditordialog.obj $D\afximagepaintarea.obj $D\afxpropertygridtooltipctrl.obj \
	$D\afxacceleratorkeyassignctrl.obj $D\afxkeyboardmanager.obj $D\afxtoolbarskeyboardpropertypage.obj $D\afxacceleratorkey.obj \
	$D\afxkeymapdialog.obj $D\afxmdiclientareawnd.obj $D\afxmaskededit.obj \
	$D\afxmenubar.obj $D\afxmenubutton.obj $D\afxmenuhash.obj $D\afxmenuimages.obj \
	$D\afxtoolbarsmenupropertypage.obj $D\afxpaneframewnd.obj $D\afxmousemanager.obj $D\afxmousepropertypage.obj \
	$D\afxmultipaneframewnd.obj $D\afxolecntrframewndex.obj $D\afxoledocipframewndex.obj $D\afxoleipframewndex.obj \
	$D\afxoleserverdocex.obj $D\afxtoolbarsoptionspropertypage.obj $D\afxoutlookbar.obj $D\afxoutlookbarpaneadapter.obj \
	$D\afxoutlookbarpane.obj $D\afxoutlookbarpanebutton.obj $D\afxoutlookbartabctrl.obj $D\afxdesktopalertdialog.obj \
	$D\afxpopupmenu.obj $D\afxpopupmenubar.obj $D\afxdesktopalertwnd.obj $D\afxpreviewviewex.obj \
	$D\afxpropertygridctrl.obj $D\afxrecentdocksiteinfo.obj $D\afxsettingsstore.obj $D\afxregpath.obj \
	$D\afxribbonbar.obj $D\afxribbonbutton.obj $D\afxribbonbuttonsgroup.obj $D\afxribboncategory.obj \
	$D\afxribboncheckbox.obj $D\afxribboncolorbutton.obj $D\afxribboncombobox.obj $D\afxribboncommandslistbox.obj \
	$D\afxpane.obj $D\afxcontrolbarimpl.obj $D\afxribboncustomizedialog.obj $D\afxribbonedit.obj \
	$D\afxribbonminitoolbar.obj $D\afxribbonlinkctrl.obj $D\afxribbonkeyboardcustomizedialog.obj $D\afxribbonkeytip.obj \
	$D\afxribbonlabel.obj $D\afxribbonmainpanel.obj $D\afxribbonpalettegallery.obj $D\afxribbonpanel.obj \
	$D\afxribbonpanelmenu.obj $D\afxribbonprogressbar.obj $D\afxribbonquickaccesstoolbar.obj $D\afxribbonslider.obj \
	$D\afxribbonstatusbar.obj $D\afxribbonstatusbarpane.obj $D\afxtoolbar.obj $D\afxribbonundobutton.obj \
	$D\afxshelllistctrl.obj $D\afxshellmanager.obj $D\afxshelltreectrl.obj $D\afxshowallbutton.obj \
	$D\afxsmartdockingmanager.obj $D\afxsmartdockingguide.obj $D\afxsmartdockinghighlighterwnd.obj $D\afxsound.obj \
	$D\afxsplitterwndex.obj $D\afxtabbedpane.obj $D\afxmdichildwndex.obj $D\afxmdiframewndex.obj \
	$D\afxtabview.obj $D\afxtabctrl.obj $D\afxtagmanager.obj $D\afxtaskspaneframewnd.obj \
	$D\afxtaskspane.obj $D\afxmenutearoffmanager.obj $D\afxbutton.obj $D\afxtoolbarslistcheckbox.obj \
	$D\afxcolordialog.obj $D\afxeditbrowsectrl.obj $D\afxheaderctrl.obj \
	$D\afxlistctrl.obj $D\afxpropertypage.obj $D\afxpropertysheet.obj $D\afxcolorpropertysheet.obj \
	$D\afxrebar.obj $D\afxrebarstate.obj $D\afxpanedivider.obj $D\afxspinbuttonctrl.obj \
	$D\afxstatusbar.obj $D\afxtooltipctrl.obj $D\afxtoolbarbutton.obj $D\afxtoolbarcomboboxbutton.obj \
	$D\afxtoolbarscustomizedialog.obj $D\afxtoolbarslistpropertypage.obj $D\afxtoolbardatetimectrl.obj $D\afxtoolbardropsource.obj \
	$D\afxtoolbardroptarget.obj $D\afxtoolbareditboxbutton.obj $D\afxtoolbarfontcombobox.obj $D\afxtoolbarimages.obj \
	$D\afxtoolbarmenubutton.obj $D\afxtoolbarmenubuttonsbutton.obj $D\afxtoolbarnamedialog.obj $D\afxtoolbarspineditboxbutton.obj \
	$D\afxtoolbarsystemmenubutton.obj $D\afxtoolbarstoolspropertypage.obj $D\afxtooltipmanager.obj $D\afxtrackmouse.obj \
	$D\afxlinkctrl.obj $D\afxusertool.obj $D\afxusertoolsmanager.obj $D\afxvisualmanager.obj \
	$D\afxvisualmanageroffice2003.obj $D\afxvisualmanageroffice2007.obj $D\afxvisualmanagervs2005.obj $D\afxvisualmanagerofficexp.obj \
	$D\afxwindowsmanagerdialog.obj $D\afxvisualmanagerwindows.obj $D\afxwinappex.obj \

WINMISC=\
	$D\wingdi.obj $D\wingdix.obj $D\winstr.obj $D\winmenu.obj \
	$D\auxdata.obj $D\afxcrit.obj $D\afxtrace.obj $D\winutil.obj \
	$D\winocc.obj $D\themehelper.obj

DOCVIEW=\
	$D\cmdtarg.obj $D\doccore.obj $D\doctempl.obj \
	$D\docsingl.obj $D\docmulti.obj $D\docmgr.obj \
	$D\viewcore.obj $D\viewprnt.obj $D\winsplit.obj $D\viewscrl.obj \
	$D\viewform.obj $D\viewedit.obj $D\viewprev.obj $D\viewcmn.obj \
	$D\docmapi.obj

INTERNET=$D\inet.obj $D\filefind.obj

APPLICATION=\
	$D\thrdcore.obj $D\appcore.obj $D\appinit.obj $D\appterm.obj \
	$D\appui.obj $D\appui1.obj $D\appui2.obj $D\appui3.obj \
	$D\appdlg.obj $D\appprnt.obj $D\apphelp.obj $D\apphelpx.obj \
	$D\filelist.obj

!if "$(DLL)" != "2"
APPLICATION=$(APPLICATION) $D\nolib.obj \
	$D\appmodul.obj $D\dumpstak.obj $D\dllmodul.obj $D\oleexp.obj
!endif

DB=\
	$D\dbcore.obj $D\dbrfx.obj $D\dbview.obj $D\dbflt.obj \
	$D\dblong.obj $D\dbvar.obj \
	$(DB) $D\viewoled.obj

!if "$(PLATFORM)" == "INTEL"
DAO=\
	$D\daocore.obj $D\daodfx.obj $D\daoview.obj
!else
DAO=
!endif

SOCKETS=$D\sockcore.obj

OLEREQ=$D\olelock.obj

OLE=\
	$D\oleinit.obj $D\olecli1.obj $D\olecli2.obj \
	$D\olecli3.obj $D\olecnvrt.obj $D\oledobj1.obj $D\oledobj2.obj \
	$D\oledisp1.obj $D\oledisp2.obj $D\oledlgs1.obj $D\oledlgs2.obj \
	$D\oledlgs3.obj $D\oledata.obj $D\olevar.obj $D\olevar1.obj \
	$D\oledoc1.obj $D\oledoc2.obj $D\oledrop1.obj $D\oledrop2.obj \
	$D\olemsgf.obj $D\oleenum.obj $D\olefact.obj $D\oleipfrm.obj \
	$D\olelink.obj $D\olemisc.obj $D\olestrm.obj $D\olesvr1.obj \
	$D\olesvr2.obj $D\olereg.obj $D\oletsvr.obj $D\oleui1.obj \
	$D\oleui2.obj $D\oleunk.obj $D\oleverb.obj \
!if "$(PLATFORM)" == "INTEL" || "$(PLATFORM)" == "MIPS"
	$D\olecall.obj \
!endif
	$D\viewrich.obj $D\oledll.obj $D\oletyplb.obj \
	$D\olemon.obj $D\winctrl5.obj $D\viewhtml.obj \
!if 0
	$D\winctrl8.obj \
!endif
    $D\occmgr.obj $D\occevent.obj $D\occcont.obj $D\occsite.obj \
	$D\occlock.obj $D\occddx.obj $D\occddxf.obj $D\occdlg.obj \
	$D\oledocvw.obj $D\oledocob.obj $D\oledoctg.obj $D\oledocip.obj \
	$D\oledoccl.obj $D\oleasmon.obj $D\olebar.obj

OLECTL=\
	$D\ctlcache.obj $D\ctlcore.obj $D\ctlconn.obj \
	$D\ctldata.obj $D\ctlevent.obj $D\ctlmodul.obj \
!if "$(DLL)" == "2" 
	$D\ctlframe.obj \
!endif
	$D\ctlfont.obj $D\ctlinplc.obj \
	$D\ctllic.obj $D\oleconn.obj $D\ctlobj.obj $D\ctlpict.obj \
	$D\ctlpropx.obj $D\ctlppg.obj $D\ctlprop.obj \
	$D\ctlpset.obj $D\ctlpstg.obj $D\ctlpstm.obj \
	$D\ctlrefl.obj $D\ctlreg.obj $D\ctltrack.obj \
	$D\ctlview.obj $D\olepset.obj $D\ctlpbag.obj \
	$D\ctlquick.obj $D\ctlnownd.obj

!if "$(DEBUG)" == "1"
OLECTL=$(OLECTL) $D\ctlinl.obj
!endif

!if "$(PLATFORM)" == "ALPHA"
!if "$(DEBUG)" == "1"
OLEASM=.\alpha\olecalld.obj
!else
OLEASM=.\alpha\olecalln.obj
!endif
!endif

!if "$(PLATFORM)" == "AXP64"
!if "$(DEBUG)" == "1"
OLEASM=.\axp64\olecalld.obj
!else
OLEASM=.\axp64\olecalln.obj
!endif
!endif

!if "$(PLATFORM)" == "AMD64"
!if "$(DEBUG)" == "1"
OLEASM=$(D)\olecalld.obj
!else
OLEASM=$(D)\olecalln.obj
!endif
!endif

!if "$(PLATFORM)" == "IA64"
!if "$(DEBUG)" == "1"
OLEASM=$(D)\olecalld.obj
!else
OLEASM=$(D)\olecalln.obj
!endif
!endif

!if "$(PLATFORM)" == "PPC"
!if "$(DEBUG)" == "1"
OLEASM=.\ppc\olecalld.obj
!else
OLEASM=.\ppc\olecalln.obj
!endif
!endif

OLEDLL=$(OLE) $(OLECTL) $(OLEASM)

!if "$(DEBUG)" == "1"
INLINES = $D\afxinl1.obj $D\afxinl2.obj $D\afxinl3.obj $D\afxinl4.obj
!else
INLINES =
!endif

CPP_OBJS=$(OBJECT) $(OBJDIAG) $(INLINES) $(FILES) $(COLL1) $(COLL2) $(MISC) \
	$(WINDOWS) $(DIALOG) $(CONTROLBARS) $(WINMISC) $(DOCVIEW) $(APPLICATION) \
	$(SOCKETS) $(OLEREQ) $(OLE) $(DAO) $(DB) $(INTERNET) $(OLECTL)

!if "$(BROWSEONLY)" == "1" && "$(DLL)" == "2"
TARGDEFS=$(TARGDEFS) /D_AFX_CORE_IMPL /D_AFX_OLE_IMPL /D_AFX_DB_IMPL /D_AFX_NET_IMPL
!endif

OBJS=$(CPP_OBJS) $(OLEASM)


#############################################################################
# Standard tools

#############################################################################
# Set CPPFLAGS for use with .cpp.obj and .c.obj rules
# Define rule for use with OBJ directory
# C++ uses a PCH file

CPPFLAGS=$(CPPFLAGS) $(CL_MODEL) $(CL_OPT_OUTDIR) $(CL_OPT) $(PDBOPTS) $(DEFS) $(OPT)
# Add compiler support for Whole Program Optimizations (linker /LTCG)
# .cpp files that are grouped into static libs should not be built with /GL
!if "$(DLL)" == "2" && "$(MANAGED)" == "0" && "$(DEBUG)" == "0"
CL_GLOB_OPT=/GL
!endif
CPPFLAGS=$(CPPFLAGS) $(CL_GLOB_OPT)


!ifndef NO_PCH
!ifndef PCH_FILE
PCH_FILE=$D\stdafx
!if "$(BROWSE)" != "0"
PCH_FILE=$(PCH_FILE)b
!endif
PCH_FILE=$(PCH_FILE).pch
!endif
!ifndef PCH_CPP
PCH_CPP=objcore
!endif

CPPFLAGS=$(CPPFLAGS) /Yustdafx.h /Fp$(PCH_FILE) $(MP_OPT)
!else
PCH_FILE=
!endif

#CPPFLAGS=$(CPPFLAGS) /FAas

.SUFFIXES:: .cpp

!if "$(BROWSE)" != "0"
.cpp{$D}.obj:
	cl @<<
$(CPPFLAGS) /c $<
<<
!else
!ifdef PREPROC
.cpp{$D}.obj::
	cl @<<
$(CPPFLAGS) /c /P $<
<<
	cl @<<
$(CPPFLAGS) /c $<
<<
!else
.cpp{$D}.obj::
	cl @<<
$(CPPFLAGS) /c $<
<<
!endif
!endif

.cpp{$D}.sbr::
	cl @<<
$(CPPFLAGS) /c $<
<<

#############################################################################
# Goals to build

GOALS=create.dir
!if "$(BROWSEONLY)" == "0"
GOALS=$(GOALS) ..\..\lib\$(PLATFORM)$(_OD_EXT)\$(GOAL).lib
!endif
!if "$(BROWSE)" != "0"
GOALS=$(GOALS) $(PLATFORM)\$(GOAL).bsc
!endif

goal: $(GOALS)

create.dir:
	@-if not exist $D\*.* mkdir $D
	@-if not exist ..\..\lib\$(PLATFORM)$(_OD_EXT)\*.* mkdir ..\..\lib\$(PLATFORM)

clean:
	-if exist $D\*.obj erase $D\*.obj
	-if exist $D\*.pch erase $D\*.pch
	-if exist $D\*.res erase $D\*.res
	-if exist $D\*.rsc erase $D\*.rsc
	-if exist $D\*.map erase $D\*.map
	-if exist $D\*.sbr erase $D\*.sbr
	-if exist $D\*.tlh erase $D\*.tlh
	-if exist $D\*.tli erase $D\*.tli
!if "$(BROWSE)" != "0"
    -erase $(PLATFORM)\$(GOAL).bsc
!endif
	-if not exist $D\*.* rmdir $D
	-if exist ..\..\lib\$(PLATFORM)$(_OD_EXT)\$(TARGETPDBNAME) del ..\..\lib\$(PLATFORM)$(_OD_EXT)\$(TARGETPDBNAME)
	-if exist ..\..\lib\$(PLATFORM)$(_OD_EXT)\$(GOAL).idb del ..\..\lib\$(PLATFORM)$(_OD_EXT)\$(GOAL).idb
	-if exist ..\..\lib\$(PLATFORM)$(_OD_EXT)\$(GOAL).rep del ..\..\lib\$(PLATFORM)$(_OD_EXT)\$(GOAL).rep
	-if exist $(PLATFORM)$(_OD_EXT)\$(TARGETPDBNAME) del $(PLATFORM)$(_OD_EXT)\$(TARGETPDBNAME)
	-if exist $(PLATFORM)$(_OD_EXT)\$(GOAL).idb del $(PLATFORM)$(_OD_EXT)\$(GOAL).idb
	-if exist $(PLATFORM)$(_OD_EXT)\$(GOAL).rep del $(PLATFORM)$(_OD_EXT)\$(GOAL).rep

#############################################################################
# Precompiled header file

!ifndef NO_PCH

!if "$(DEBUG)" == "1"
HDRS =..\..\include\*.h
!else
HDRS =..\..\include\*.h ..\..\include\*.inl
!endif

PCH_TARGETS=$(PCH_FILE) $D\$(PCH_CPP).obj
!if "$(BROWSEONLY)" != "0"
PCH_TARGETS=$(PCH_TARGETS) $D\$(PCH_CPP).sbr
!endif

$(PCH_TARGETS):: $(PCH_CPP).cpp $(HDRS)
	cl @<<
/Ycstdafx.h /Fp$(PCH_FILE) $(CL_MODEL) $(CL_OPT_OUTDIR) $(CL_OPT) $(CL_GLOB_OPT) $(PDBOPTS) $(DEFS) $(OPT) /c $(PCH_CPP).cpp
<<

!if "$(BROWSEONLY)" == "1"
$D\$(PCH_CPP).sbr:: $(PCH_CPP).cpp $(PCH_FILE)
!endif

!endif # NO_PCH

#############################################################################
## PLATFORM=ALPHA specific target(s)

!if "$(PLATFORM)" == "ALPHA"

!if "$(DEBUG)" == "1"
ASMOPT=$(ASMOPT) /D_DEBUG /g2
!else
ASMOPT=$(ASMOPT)
!endif

$(OLEASM) : alpha\olecall_.s
	asaxp $(ASMOPT) -o $@ alpha\olecall_.s

!endif

#############################################################################
## PLATFORM=AXP64 specific target(s)

!if "$(PLATFORM)" == "AXP64"

!if "$(DEBUG)" == "1"
ASMOPT=$(ASMOPT) -Ap64 /D_DEBUG /g2
!else
ASMOPT=$(ASMOPT) -Ap64
!endif

$(OLEASM) : axp64\olecall_.s
	asaxp $(ASMOPT) -o $@ axp64\olecall_.s

!endif

#############################################################################
## PLATFORM=PPC specific target(s)

!if "$(PLATFORM)" == "PPC"

!if "$(DEBUG)" == "1"
ASMOPT=$(ASMOPT)
!else
ASMOPT=$(ASMOPT)
!endif

$(OLEASM) : ppc\olecall_.s
	pas $(ASMOPT) -o $@ ppc\olecall_.s

!endif

#############################################################################
## PLATFORM=IA64 specific target(s)

!if "$(PLATFORM)" == "IA64"

!if "$(DEBUG)" == "1"
ASMOPT=$(ASMOPT)
!else
ASMOPT=$(ASMOPT)
!endif

$(OLEASM) : ia64\olecall_.s
	ias $(ASMOPT) -o $@ ia64\olecall_.s

!endif

#############################################################################
## PLATFORM=AMD64 specific target(s)

!if "$(PLATFORM)" == "AMD64"

!if "$(DEBUG)" == "1"
ASMOPT=$(ASMOPT)
!else
ASMOPT=$(ASMOPT)
!endif

$(OLEASM) : amd64\olecall_.s
	ml64 $(ASMOPT) -c -Fo$@ amd64\olecall_.s

!endif

#############################################################################
# Build the library from the up-to-date objs

SBRS=$(CPP_OBJS:.obj=.sbr)

!if "$(BROWSEONLY)" != "0"

# Build final browse database
$(PLATFORM)\$(GOAL).bsc: $(PCH_TARGETS) $(SBRS)
	bscmake /n /Iu /El /o$@ @<<
$(SBRS)
<<

!else #BROWSEONLY

!if "$(DLL)" != "2"
# Build final library
..\..\lib\$(PLATFORM)$(_OD_EXT)\$(GOAL).lib: $(PCH_TARGETS) $(OBJS)
	@-if exist $@ erase $@
	@lib /out:$@ @<<
$(OBJS)
<<

# Recurse to build browse database
$(PLATFORM)\$(GOAL).bsc: $(PCH_TARGETS) $(SBRS)
	$(MAKE) /f makefile. @<<
BROWSEONLY=1 PLATFORM=$(PLATFORM) DEBUG=$(DEBUG) CODEVIEW=$(CODEVIEW) \
DLL=$(DLL) NO_PCH=$(NO_PCH) OBJ=$(OBJ) OPT=$(OPT)
<<
!endif #DLL!=2

!endif #!BROWSEONLY

#############################################################################

!endif  # NTMAKEENV


