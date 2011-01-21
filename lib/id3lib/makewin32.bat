REM $Id: makewin32.bat,v 1.2 2001/12/16 11:11:52 shadrack Exp $
@echo off

REM Comments, bugs go to johan@linkdata.se
REM Mail me on how to run the command line compiler, and I'll flame you. RTFM.

if defined INCLUDE goto ok
if not exist "\program files\microsoft visual studio\VC98\bin\vcvars32.bat" goto altloc1
call "\program files\microsoft visual studio\VC98\bin\vcvars32.bat"
goto ok

:altloc1
if not exist "\programs\microsoft visual studio\VC98\bin\vcvars32.bat" goto altloc2
call "\programs\microsoft visual studio\VC98\bin\vcvars32.bat"
goto ok

:altloc2
if not exist "\programs\msvs\VC98\bin\vcvars32.bat" goto runvc
call "\programs\msvs\VC98\bin\vcvars32.bat"
goto ok

:ok
if not defined INCLUDE goto runvc
if not exist config.h copy config.h.win32 config.h
REM nmake DEBUG=1 -f makefile.win32
nmake -f makefile.win32
goto done

:runvc
echo You need to run the VCVARS32.BAT batch file.
echo You'll find it in the Visual C++ binaries directory.
echo Also, when installing Visual C++, you are asked if to automatically
echo run this file when starting a command shell.

:done
