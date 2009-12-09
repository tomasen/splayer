REM http://www.installsite.org/pages/en/msi/articles/embeddedlang/index.htm
msitran -g ..\wix\bin\Release\en-US\SvPlayer.msi ..\wix\bin\ReleaseCHS\zh-CN\SvPlayer.msi chs.mst

rem msidb -d bin\Release\en-US\SvPlayer.msi -r chs.mst

del /F ..\..\..\..\SPlayer.msi
copy /y ..\wix\bin\Release\en-US\SvPlayer.msi ..\..\..\..\SPlayer.msi

scripts\wisubstg.vbs ..\..\..\..\SPlayer.msi chs.mst 2052

scripts\WiLangId.vbs ..\..\..\..\SPlayer.msi Package 1033,2052
