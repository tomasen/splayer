del /F /S /Q bin
"%programfiles%\TortoiseSVN\bin\SubWCRev.exe" ..\..\.. .\SvPlayerver.wxi.template .\SvPlayerver.wxi
msbuild svplayer.wixproj /property:Configuration=Release /l:FileLogger,Microsoft.Build.Engine;logfile=svplayer.log;verbosity=detailed /t:Clean,Build 
msbuild svplayer.wixproj /property:Configuration=ReleaseCHT /l:FileLogger,Microsoft.Build.Engine;logfile=svplayer.cht.log;verbosity=detailed /t:Clean,Build 
msbuild svplayer.wixproj /property:Configuration=ReleaseCHS /l:FileLogger,Microsoft.Build.Engine;logfile=svplayer.ch.log;verbosity=detailed /t:Clean,Build 
msbuild svplayer.wixproj /property:Configuration=ReleaseRU /l:FileLogger,Microsoft.Build.Engine;logfile=svplayer.ru.log;verbosity=detailed /t:Clean,Build 
