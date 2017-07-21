@echo off


call "%VS90COMNTOOLS%vsvars32.bat"

echo Building sinet project of sinet ...
"%DevEnvDir%/devenv.com" ../trunk/sinet.sln  /build "Release|Win32"

if %ERRORLEVEL% NEQ 0 goto error


goto end

:error
echo ERROR: an error occurred during building 
goto finished

:end
echo sinet-lib built successfully

:finished
pause