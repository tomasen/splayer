@echo off

chdir /D %~dp0

IF NOT EXIST "..\src\svplib\shooterclient.key" copy ".\shooterclient_dummy.key" "..\src\svplib\shooterclient.key"


