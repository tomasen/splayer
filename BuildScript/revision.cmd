@echo off

chdir /D %~dp0

set revfile="..\src\apps\mplayerc\revision.h"

for /f "delims=+ " %%a in ('hg_bin\hg id -n') do @set revnum=%%a 
for /f "delims=+ " %%a in ('hg_bin\hg id -i') do @set revset=%%a 

echo "%revnum%"  "%revset%"

echo #pragma once > %revfile%
echo #define SVP_REV_STR     L"%revnum%" >> %revfile%
echo #define SVP_REV_NUMBER  %revnum% >> %revfile%
echo #define BRANCHVER       L"36" >> %revfile%

REM if anything failed
IF NOT EXIST %revfile% copy ".\revision_dummy.h" %revfile%
