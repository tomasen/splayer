REM http://www.installsite.org/pages/en/msi/articles/embeddedlang/index.htm
msitran -g ..\wix\bin\Release\en-US\SvPlayer.msi ..\wix\bin\ReleaseCHS\zh-CN\SvPlayer.msi chs.mst
msitran -g ..\wix\bin\Release\en-US\SvPlayer.msi ..\wix\bin\ReleaseCHT\zh-TW\SvPlayer.msi cht.mst
msitran -g ..\wix\bin\Release\en-US\SvPlayer.msi ..\wix\bin\ReleaseRU\ru-RU\SvPlayer.msi ru.mst

del /F ..\..\..\..\SPlayer.msi
copy /y ..\wix\bin\Release\en-US\SvPlayer.msi ..\..\..\..\SPlayer.msi

scripts\wisubstg.vbs ..\..\..\..\SPlayer.msi chs.mst 2052
scripts\wisubstg.vbs ..\..\..\..\SPlayer.msi cht.mst 1028
scripts\wisubstg.vbs ..\..\..\..\SPlayer.msi ru.mst 1049

scripts\WiLangId.vbs ..\..\..\..\SPlayer.msi Package 1033,2052,1028,1049
