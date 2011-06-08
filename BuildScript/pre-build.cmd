@echo off

chdir /D %~dp0

IF NOT EXIST "..\src\svplib\shooterclient.key" copy ".\shooterclient_dummy.key" "..\src\svplib\shooterclient.key"


IF NOT EXIST "..\include\shooterclient.key" copy ".\shooterclient_dummy.key" "..\include\shooterclient.key"


IF NOT EXIST "..\include\shooterapi.key" copy ".\shooterapi_dummy.key" "..\include\shooterapi.key"