@echo off

chdir /D %~dp0

set revfile="..\src\apps\mplayerc\revision.h"
set tmpfile=".\temp.h"

for /f "delims=+ " %%a in ('hg_bin\hg id -n') do @set revnum=%%a 
for /f "delims=+ " %%a in ('hg_bin\hg id -i') do @set revset=%%a 
set revnum=%revnum:~0,-1%
set revset=%revset:~0,-1%
echo "%revnum%"  "%revset%"

echo #pragma once > %tmpfile%
echo #define SVP_REV_STR     L"%revnum%" >> %tmpfile%
echo #define SVP_REV_NUMBER  %revnum% >> %tmpfile%
echo #define BRANCHVER       L"36" >> %tmpfile%

IF NOT EXIST %tmpfile% copy ".\revision_dummy.h" %revfile%

FC %tmpfile% %revfile% | FIND "FC: no dif" > nul 
IF ERRORLEVEL 1 goto s_files_are_different

goto end

:s_files_are_different
copy %tmpfile% %revfile%

:end
REM if anything failed
IF NOT EXIST %revfile% copy ".\revision_dummy.h" %revfile%
