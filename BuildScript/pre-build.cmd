@echo off

chdir /D %~dp0

IF NOT EXIST "..\src\svplib\shooterclient.key" copy ".\shooterclient_dummy.key" "..\src\svplib\shooterclient.key"

call rev.cmd

IF NOT EXIST "..\src\apps\mplayerc\revision.h" copy ".\revision_dummy.h" "..\src\apps\mplayerc\revision.h"
