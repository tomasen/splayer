=======================================================================
This is a part of the Microsoft Foundation Classes C++ library.
Copyright (C) Microsoft Corporation
All rights reserved.

This source code is only intended as a supplement to the
Microsoft Foundation Classes Reference and related
electronic documentation provided with the library.
See these sources for detailed information regarding the
Microsoft Foundation Classes product.
=======================================================================


=======================================================================
					 MICROSOFT (R) FOUNDATION CLASSES
							  Version 9.0
						   Source Code ReadMe
=======================================================================

This directory contains the source code to the MFC library.

These files have been included for reference purposes, in
conjunction with the Class Library Reference manual and tutorial.

These files are compiled to form the Microsoft Foundation Classes 9.0
(MFC 9.0).  The library may be built in a number of configurations,
character sets (UNICODE or ANSI), and whether or not debugging and
diagnostic aids are to be included in applications which link with
the library.

============================================
 1.  BUILDING A STATIC LINK LIBRARY VARIANT
============================================

The makefile provided can be used to build the static link library
variants of MFC 9.0.  See section 2 for instructions on building
the shared DLL variant.  The makefile can be used from either the
MS-DOS command line (in a Windows NT cmd shell) or as an External
Project file in the Microsoft Developer Studio.

To build a library in a particular configuration, use the NMAKE tool
and the makefile which is in this directory.  The following arguments
can be given to NMAKE to successfully build a specific library variant.

  NMAKE {DEBUG=[0|1]} {BROWSE=[0|1]} {BROWSEONLY={0|1} \
		{CODEVIEW=[0|1|2]} {OBJ=path} \
		{PLATFORM=INTEL} \
		{UNICODE=[0|1]} \
		{OPT=<CL command line switches>}

Previous versions of MFC supported MODEL and TARGET options to control the
memory model and to support DOS/CONSOLE mode targetting. These options are
no longer supported (your Win32 console application can still be linked to
the GUI MFC library, NAFXCW[D].LIB).

For example to build default model with browser information in the debug
build use the following settings:

	Debug Build:  NMAKE DEBUG=1 CODEVIEW=1 BROWSE=1
	Release Build: NMAKE DEBUG=0

DEBUG=[0|1]
	The "DEBUG" argument specifies whether or not to include diagnostic
	support code for the library.  This may be 0 (for no diagnostics)
	or 1 (for full diagnostics).

CODEVIEW=[0|1]
	The "CODEVIEW" argument specifies whether to compile the library with
	CodeView information or not.  You need to compile the library with
	CodeView information if you want to trace into MFC code using the
	Visual C++ debugger.    You should also compile your application files
	with the /Zi option, and link your executable with the /debug
	and /debugtype:cv options.

	Setting CODEVIEW does not affect the DEBUG argument, although the
	value of the DEBUG argument does affect the default value of CODEVIEW
	(discussed below).  A value of 0 indicates that no CodeView
	information is to be compiled into the library.  A value of 1 indicates
	to compile in full CodeView information for all modules of the library.

	The default value depends on the setting of the DEBUG argument.
	If DEBUG=1, CODEVIEW defaults to 1.  If DEBUG=0, CODEVIEW defaults
	to 0.  The installed libraries have been built with CODEVIEW=1 for
	maximum CodeView information.

UNICODE=[0|1]
	The "UNICODE" argument specifies whether to create an MBCS
	or UNICODE aware library.  If UNICODE is set to 1, the UNICODE
	variant of the specified library will be built.  The default,
	UNICODE=0, specifies that an MBCS aware (or ANSI) library is
	to be built.

OBJ=[path]
	We recommend storing .OBJ files in a separate directory so that you
	may compile different versions of the MFC library concurrently.
	The "OBJ" argument allows you to specify where these files are stored
	during the build process.  The directory specified is created and
	removed automatically as required.  This defaults to a combination
	of the target, model, and debug status, preceded by a '$' (i.e. $NWD).

OPT=[switches]
	If your library needs to be built with custom compiler switches, then
	these may be included in the "OPT" argument.  Note that switches need
	to be separated by spaces, so when including more than one extra
	compiler switch, enclose the whole OPT= argument in double-quotes.
	This is an advanced feature; read the makefile and the details on each
	of the switches concerned in the Microsoft C/C++ Compiler User Manual
	before using this option.

Defaults
	The default is:
	nmake DEBUG=1 CODEVIEW=1 BROWSE=0 BROWSEONLY=0 UNICODE=0 OBJ=$NWD

To set these options from MS Developer Studio, from the Build menu choose
the Settings menu command and set them in the associated build sections.

=============================
 2.  BUILDING THE SHARED DLL
=============================

Building the shared DLL is very similar to the static link variants.  You
must, however, use the MFCDLL.MAK which is also an External Project file.

You must provide a unique name for your DLL(s) and you must provide a DEF
file with a matching name.  Typically, you can make a copy of the DEF
files provided in the \msdev\mfc\src\<platform> directory.

Once you rename this DEF file to be the same base name as the DLL you're
building you then need to add the additional exports (if any).

Technical note TN033 explains in detail how to build the shared DLL and
how to build an application that uses the shared DLL.


===============================
 3. AFTER BUILDING THE LIBRARY
===============================

Once the library has been built successfully, you may want to delete object
files with:

	NMAKE CLEAN OBJ=[path]

Note that if you used the "OBJ" argument while building the library, specify
the same sub directory in the cleanup command.

This will remove all of the temporary .OBJ files created by building the
library, and remove the directory where they were stored.

Always perform a cleanup before building a new variant of the library,
or use different object paths for each variant.  Note that the OBJ files
are only necessary during the building process.


===========================================
 4. BUILDING AND USING THE BROWSE DATABASE
===========================================

A prebuilt MFC browser database is included on the Visual C++ CD-ROM.
It is located in the same directory as the MFC source files, and can be
accessed by opening <cd-drive>:\MSDEV\MFC\SRC\MFC.BSC.  You may want to
build a browse database if you wish to merge the MFC browse information
with your own project or you wish to build a different browse variant than
the one provided (MFC.BSC is the browser database for NAFXCWD.LIB).
The following instructions describe how to build the browse database using
MFC's makefile and how to integrate the resulting .SBR files into your
project.

By building the library with either the BROWSE=1 or BROWSEONLY=1
options you can create the browse database files for the MFC source code.
The output browse file (.BSC) will be placed in the source directory
(this allows the browser to find the source files). The browse
database files share a common naming convention with the libraries.

Usually, it is sufficient to build only one browse database, although
you can build a browse variant for each library variant that MFC
supports.  In addition, it is possible to build only the browse database,
instead of building all the object files and library file.  To build
the NAFXCWD.BSC file, for example:

	NMAKE DEBUG=1 BROWSEONLY=1

The output will be placed in NAFXCWD.BSC (in the MFC source directory).
In addition, all of the SBR files (the individual files, which when combined,
form the .BSC) will be preserved in the OBJ directory (in this case $NWD).
You can add these SBR files to your project's browse database, enabling you to
browse the MFC source as well as your application source code at the same time.

If you are using an external makefile, simply include the MFC SBRs in your
BSCMAKE command.  For example:

	BSCMAKE /o myproj.sbr myfile1.sbr myfile2.sbr \msdev\mfc\src\$nwd\*.sbr

If you are using a Visual C++ project file, you can add it to your project
settings.  To do so, load your project (.MDP or .MAK) file:

	- From the Build menu, choose Settings to edit your project settings
	- Select the "Browse Info" tab in the settings dialog
	- Choose the target you wish to add the MFC browse information
		(ie. Win32 Debug or Win32 Retail)
	- Add the browse files (.SBR files) to "Project Options" edit box:
		\msdev\mfc\src\$nwd\*.sbr

	  Note: Substitute the appropriate path if you built a different variant
		of the library or installed VC++ in a different directory than the
		default.

Even if you are using the incremental build option of BSCMAKE (the default),
these files will not be truncated to zero length (the files are marked in a
special way to prevent the normal truncation that occurs when doing incremental
builds).

Note: The SBR files are protected from truncation by adding a special record
(PCHMARK) that bscmake uses to protect SBR files that are refered to from
pre-compiled header files (PCH).  This record consists of a single byte with
the value 0x10.  The PCHMARK.BIN file contains this single byte -- it is
appended to the SBR files during the build process.

You can also open the resulting browse database file directly and use it
to browse MFC source code.


======================================
 5. SOURCE CODE FORMATTING CONVENTION
======================================

All MFC source code has been formatted such that leading whitespace
on a line is made up of physical tabs, while embedded whitespace is
physical spaces.  MFC source code assumes that your editor is set to
display a physical tab as four blanks.

For example:

int FormatExample()
{
// Statements below should start in column 5 if tabs are set correctly
// Comment should start in column 20
/*
0        1         2
12345678901234567890
*/
	int i;
	i = 5;         // whitespace between statement and comment is spaces

	return i;

}

More information on MFC coding and commenting conventions can be found
in Technical Note #46.
