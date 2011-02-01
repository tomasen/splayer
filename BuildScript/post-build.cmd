@echo off

chdir /D %~dp0

for /f "delims=+ " %%a in ('hg_bin\hg id -n -R ../') do @set revnum=%%a 

StampVer.exe -f"3.6.0.%revnum%" -p"3.6.0.%revnum%" ../../splayer.exe
